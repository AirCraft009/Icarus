//
// Created by cocon on 11.10.2026.
//

#include "kernel/Drivers/PS2/PS2_Controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "kernel_helper.h"
#include "kernel/Memory/DMA/ACPI_helpers.h"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_REG 0x64         // on reads
#define PS2_COMMAND_REGISTER 0x64   // on writes

#define PS2_STATUS_OUT_BUFF     0
#define PS2_STATUS_IN_BUFF      1
#define PS2_STATUS_SYS_FLAG     2
#define PS2_STATUS_DIRECTION    3       // (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command)
#define PS2_STATUS_UNKNOWN_1    4       //chipset
#define PS2_STATUS_UNKNOWN_2    5       //chipset
#define PS2_STATUS_TIMEOUT_ERR  6
#define PS2_STATUS_PARITY_ERR   7

#define PS2_TIMEOUT 100000
PS2_info_t ps2_info = {0};

/**
 * write a single byte command to the ps2 controller (not device)
 */
void write_ps2_bcommand(uint8_t command) {
    outportb(PS2_COMMAND_REGISTER, command);
}

/**
 * write a single byte command to the ps2 controller (not device)
 * can hang (if the output buffer never sets)
 */
int write_ps2_bcommand_ret(uint8_t command) {
    outportb(PS2_COMMAND_REGISTER, command);
    // poll the output buffer flag until a val is there to read
    for (int i = 0; i < PS2_TIMEOUT; ++i) {
        if (inportb(PS2_STATUS_REG) >> PS2_STATUS_OUT_BUFF & 0x1)
            return inportb(PS2_DATA_PORT);
    }
    return -1;
}

/**
 * write a two byte command to the ps2 controller (not device)
 * returns 0 on success or -1 on fail
 */
int write_ps2_2bcommand(uint16_t command) {
    outportb(PS2_COMMAND_REGISTER, (uint8_t) command);
    // wait till data port input is empty
    for (int i = 0; i < PS2_TIMEOUT; ++i) {
        if (!(inportb(PS2_STATUS_REG) >> PS2_STATUS_IN_BUFF & 0x1)) {
            goto write_b_2;
        }
    }
    goto timeout;

    write_b_2:
        outportb(PS2_DATA_PORT, (uint8_t)(command >> 8));
        return 0;

    timeout:
        return -1;
}

/**
 * write a two byte command to the ps2 controller (not device)
 * returns value of the return byte or -1 on fail
 */
int write_ps2_2bcommand_ret(uint16_t command) {
    if (write_ps2_2bcommand(command) != -1) {
        goto w_byte_2;
    }
    goto error;

    w_byte_2:
        // poll the output buffer flag until a val is there to read
        for (int i = 0; i < PS2_TIMEOUT; ++i) {
            if (inportb(PS2_STATUS_REG) >> PS2_STATUS_OUT_BUFF & 0x1)
                return inportb(PS2_DATA_PORT);

        }

    error:
        return -1;
}

void flush_out_buff() {
    inportb(PS2_DATA_PORT);
}

/**
 *
 */
static int ps2_read_timeout() {
    for (int i = 0; i < PS2_TIMEOUT; i++)
        if (inportb(PS2_STATUS_REG) & 1)
            return inportb(PS2_DATA_PORT);
    return -1;
}

static int ps2_write_timeout(uint8_t b) {
    for (int i = 0; i < PS2_TIMEOUT; i++)
        if (!(inportb(PS2_STATUS_REG) & 2)) {
            outportb(PS2_DATA_PORT, b);
            return 0;
        }
    return -1;
}

/**
 * send a byte to device on port 1 or 2
 */
static int ps2_send_dev(int port, uint8_t b) {
    if (port == 2) write_ps2_bcommand(0xD4);
    return ps2_write_timeout(b);
}

// 0 = reset ok, -1 = self test failed, -2 = nothing there
static int ps2_reset_dev(int port) {
    if (ps2_send_dev(port, 0xFF) < 0) return -2;
    int r1 = ps2_read_timeout();
    if (r1 < 0)    return -2;
    if (r1 == 0xFC) return -1;
    int r2 = ps2_read_timeout();
    if (r2 < 0)    return -2;
    // 0xFA and 0xAA can arrive in either order
    if (!((r1 == 0xFA && r2 == 0xAA) || (r1 == 0xAA && r2 == 0xFA))) return -1;
    int id = ps2_read_timeout();
    (void)id; // TODO: store for device type detection
    return 0;
}


/**
 * responsible for checking the ps2 controllers functionality
 * & disabling ps2 ports to avoid errors during setup
 */
int init_ps2_controller() {

    //TODO: actually check the config byte and make sure everything is running accordingly

    // !!!!!!! NOT RELIABLE !!!!!!!
    // // https://uefi.org/specs/ACPI/6.5/05_ACPI_Software_Programming_Model.html#ia-pc-boot-architecture-flags
    // if ((FADT.BootArchitectureFlags >> 1) & 0x1) {
    //     return -1;
    // };


    write_ps2_bcommand(0xAD);       // Disable first PS/2 port
    write_ps2_bcommand(0xA7 );      // Disable second PS/2 port (only if 2 PS/2 ports supported)
    flush_out_buff();
    uint8_t conf_byte = write_ps2_bcommand_ret(0x20);
    //clear bits 0, 4, 6, to disable IRQ and translation to port 1  &  enabling the clock sig w/ bit 4
    uint8_t conf_mask =  ~(1 | 1 << 4 | 1 << 6);
    conf_byte &= conf_mask;
    // write the new config back
    write_ps2_2bcommand(0x60 | (uint16_t) conf_byte << 8);
    // do a selftest on a known good config state
    if (write_ps2_bcommand_ret(0xAA) != 0x55)       //any val other than 0x55 is invalid
        return -1;

    // write back the good config as the self test can mess up the conf
    write_ps2_2bcommand(0x60 | (uint16_t) conf_byte << 8);


    //check for a dual channel
    int dual = 0;
    write_ps2_bcommand(0xA8);                       // enable second port
    conf_byte = write_ps2_bcommand_ret(0x20);
    if (!(conf_byte & (1 << 5))) {                  // bit 5 clear -> dual channel
        dual = 1;
        write_ps2_bcommand(0xA7);                   // disable second port again
        conf_byte &= ~(1 | 1 << 1 | 1 << 5);        // IRQs off, clock on for port 2
        write_ps2_2bcommand(0x60 | (uint16_t) conf_byte << 8);
    }

    // test (both) interfaces
    int port1_ok = (write_ps2_bcommand_ret(0xAB) == 0x00);
    int port2_ok = dual && (write_ps2_bcommand_ret(0xA9) == 0x00);
    if (!port1_ok && !port2_ok) {
        // TODO: log error
        return -1;
    }

    // enable working ports + their IRQs
    if (port1_ok) {
        write_ps2_bcommand(0xAE);
        conf_byte |= 1;
    }
    if (port2_ok) {
        write_ps2_bcommand(0xA8);
        conf_byte |= 1 << 1;
    }
    write_ps2_2bcommand(0x60 | (uint16_t) conf_byte << 8);

    // reset devices
    if (port1_ok) {
        int r = ps2_reset_dev(1);
        if (r != 0) port1_ok = 0;
    }
    if (port2_ok) {
        int r = ps2_reset_dev(2);
        if (r != 0) port2_ok = 0;
    }

    ps2_info.config = conf_byte;
    ps2_info.enabled = true;
    ps2_info.exists = true;
    ps2_info.is_dual_channel = dual;

    return 0;
}


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


/**
 * write a single byte command to the ps2 controller (not device)
 * is blocking (can't hang except on io failure)
 */
void write_ps2_bcommand(uint8_t command) {
    outportb(PS2_COMMAND_REGISTER, command);
}

/**
 * write a single byte command to the ps2 controller (not device)
 * is blocking & can hang (if the output buffer never sets)
 */
uint8_t write_ps2_bcommand_ret(uint8_t command) {
    outportb(PS2_COMMAND_REGISTER, command);
    // poll the output buffer flag until a val is there to read
    while (!inportb(PS2_STATUS_REG) >> PS2_STATUS_OUT_BUFF)
        ;
    return inportb(PS2_DATA_PORT);
}

/**
 * write a two byte command to the ps2 controller (not device)
 * is blocking & can hang (if the write reg is never empty)
 */
void write_ps2_2bcommand(uint16_t command) {
    outportb(PS2_COMMAND_REGISTER, (uint8_t) command);
    // wait till data port input is empty
    while (inportb(PS2_STATUS_REG) >> PS2_STATUS_IN_BUFF)
        ;

    outportb(PS2_DATA_PORT, (uint8_t)(command >> 8));
}

/**
 * write a two byte command to the ps2 controller (not device)
 * is blocking & can hang (if the data input is never clear | the output buffer never sets)
 */
uint8_t write_ps2_2bcommand_ret(uint16_t command) {
    outportb(PS2_COMMAND_REGISTER, (uint8_t) command);
    // wait till data port input is empty
    while (inportb(PS2_STATUS_REG) >> PS2_STATUS_IN_BUFF)
        ;

    outportb(PS2_DATA_PORT, (uint8_t)(command >> 8));

    // poll the output buffer flag until a val is there to read
    while (!inportb(PS2_STATUS_REG) >> PS2_STATUS_OUT_BUFF)
        ;
    return inportb(PS2_DATA_PORT);
}

void flush_out_buff() {
    inportb(PS2_DATA_PORT);
}


/**
 * responsible for checking the ps2 controllers functionality
 * & disabling ps2 ports to avoid errors during setup
 */
int init_ps2_controller() {
    //TODO: actually check the config byte and make sure everything is running accordingly

    write_ps2_bcommand(0xAD);       // Disable first PS/2 port
    write_ps2_bcommand(0xA7 );      // Disable second PS/2 port (only if 2 PS/2 ports supported)

    // currently only checks to see if the ps2 controller even exists
    // https://uefi.org/specs/ACPI/6.5/05_ACPI_Software_Programming_Model.html#ia-pc-boot-architecture-flags
    if ((FADT.BootArchitectureFlags >> 1) & 0x1) {
        // this flag is not reliable so we do more aggressive testing (if the controller doesn't exist it won't matter, if it does at least we'll know)
        return -1;
    };
    return 0;
}


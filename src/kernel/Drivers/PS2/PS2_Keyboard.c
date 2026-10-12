//
// Created by cocon on 11.10.2026.
//

#include "kernel/Drivers/PS2/PS2_Keyboard.h"

#include "kernel/Drivers/PS2/PS2_Controller.h"
#include "kernel/interrupts/APIC.h"
#include "./PS2_Scancode.h"
#include "kernel/util/shellio.h"

typedef struct {
    bool shift;
    bool caps;
    bool release;
    bool extended;
} ps2_kb_state;

static ps2_kb_state ps2_state = {0};


/**
 * translates a ps2 scancode into ASCII
 * takes shift, caps, and extended (arrows, right ctrl etc.)
 */
char kbd_translate(uint8_t sc) {
    //Iprintf("sc: %x", sc);
    if (sc == 0xE0) {
        ps2_state.extended = true;
        return 0;
    }   // prefix for arrows, right ctrl, etc.

    if (ps2_state.extended) {
        ps2_state.extended = false;
        return 0;
    } // ignore ps2_state.extended keys for now; TODO: might extend later

    if (sc == 0xF0) {
        ps2_state.release = true;
        return 0;
    }

    bool release = ps2_state.release;
    ps2_state.release = false;

    if (sc == 0x12 || sc == 0x59) {
        ps2_state.shift = !release;
        //Iprintf("SHIFTY: %i", ps2_state.shift);
        return 0;
    }

    if (sc == 0x58 && release)  {
        ps2_state.caps = ! ps2_state.caps;
        return 0;
    }

    if (release || sc >= sizeof sc2_normal) {
        return 0;
    }


    char c = ( ps2_state.shift ? sc2_shift : sc2_normal)[sc];
    if ( ps2_state.caps && c >= 'a' && c <= 'z')
        c -= 32;       // caps lock only affects letters
    else if ( ps2_state.caps && c >= 'A' && c <= 'Z' &&  ps2_state.shift)
        c += 32;
    return c;
}


void init_keyboard_ps2() {
    ioapic_route(
        ioapic_infos->apics[0].base_address,
        1,
        33,
        0,
        false,
        false
        );
}

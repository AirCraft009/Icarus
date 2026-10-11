//
// Created by cocon on 11.10.2026.
//

#include "kernel/Drivers/PS2/PS2_Keyboard.h"

#include "kernel/Drivers/PS2/PS2_Controller.h"
#include "kernel/interrupts/APIC.h"

// !!! AI - GENERATED !!!!!!!!
static const char sc_normal[0x3A] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']', '\n',
    0,    'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,    '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' '
};

static const char sc_shift[0x3A] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t', 'Q','W','E','R','T','Y','U','I','O','P','{','}', '\n',
    0,    'A','S','D','F','G','H','J','K','L',':','"','~',
    0,    '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' '
};
// !!! AI - GENERATED END !!!!!!

typedef struct {
    bool shift;
    bool caps;
    bool extended;
} ps2_kb_state;

static ps2_kb_state ps2_state = {0};


/**
 * translates a ps2 scancode into ASCII
 * takes shift, caps, and extended (arrows, right ctrl etc.)
 */
char kbd_translate(uint8_t sc) {
    if (sc == 0xE0) {
        ps2_state.extended = true;
        return 0;
    }   // prefix for arrows, right ctrl, etc.

    if (ps2_state.extended) {
        ps2_state.extended = false;
        return 0;
    } // ignore ps2_state.extended keys for now; TODO: might extend later

    bool released = sc & 0x80;
    sc &= 0x7F;

    if (sc == 0x2A || sc == 0x36) {
        ps2_state.shift = !released;
        return 0;
    }

    if (sc == 0x3A && !released)  {
        ps2_state.caps = ! ps2_state.caps;
        return 0;
    }

    if (released || sc >= sizeof sc_normal)
        return 0;

    char c = ( ps2_state.shift ? sc_shift : sc_normal)[sc];
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

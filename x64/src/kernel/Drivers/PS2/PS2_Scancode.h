//
// Created by Mxsxll on 12.10.2026.
//

#ifndef ICARUS_PS2_SCANCODE_H
#define ICARUS_PS2_SCANCODE_H


// !!!!!!!!!!!!!! AI - GENERATED !!!!!!!!!!!!!!

/* PS/2 scancode set 2 (make codes 0x00-0x7F) -> ASCII.
 * 0 = no printable character (modifiers, function keys, etc.).
 * Extended keys (prefixed by 0xE0) are NOT covered by these tables. */

#define SC2_BREAK_PREFIX 0xF0
#define SC2_EXT_PREFIX   0xE0

#define SC2_LSHIFT 0x12
#define SC2_RSHIFT 0x59
#define SC2_LCTRL  0x14
#define SC2_LALT   0x11
#define SC2_CAPS   0x58


// !!! AI - GENERATED !!!!!!!!
static const char sc2_normal[128] = {
    [0x0D] = '\t', [0x0E] = '`',
    [0x15] = 'q',  [0x16] = '1',
    [0x1A] = 'z',  [0x1B] = 's',  [0x1C] = 'a',  [0x1D] = 'w',  [0x1E] = '2',
    [0x21] = 'c',  [0x22] = 'x',  [0x23] = 'd',  [0x24] = 'e',  [0x25] = '4',  [0x26] = '3',
    [0x29] = ' ',  [0x2A] = 'v',  [0x2B] = 'f',  [0x2C] = 't',  [0x2D] = 'r',  [0x2E] = '5',
    [0x31] = 'n',  [0x32] = 'b',  [0x33] = 'h',  [0x34] = 'g',  [0x35] = 'y',  [0x36] = '6',
    [0x3A] = 'm',  [0x3B] = 'j',  [0x3C] = 'u',  [0x3D] = '7',  [0x3E] = '8',
    [0x41] = ',',  [0x42] = 'k',  [0x43] = 'i',  [0x44] = 'o',  [0x45] = '0',  [0x46] = '9',
    [0x49] = '.',  [0x4A] = '/',  [0x4B] = 'l',  [0x4C] = ';',  [0x4D] = 'p',  [0x4E] = '-',
    [0x52] = '\'', [0x54] = '[',  [0x55] = '=',
    [0x5A] = '\n', [0x5B] = ']',  [0x5D] = '\\',
    [0x66] = '\b', [0x76] = 27,
    /* keypad (NumLock on) */
    [0x69] = '1',  [0x6B] = '4',  [0x6C] = '7',
    [0x70] = '0',  [0x71] = '.',  [0x72] = '2',  [0x73] = '5',  [0x74] = '6',  [0x75] = '8',
    [0x79] = '+',  [0x7A] = '3',  [0x7B] = '-',  [0x7C] = '*',  [0x7D] = '9',
};

static const char sc2_shift[128] = {
    [0x0D] = '\t', [0x0E] = '~',
    [0x15] = 'Q',  [0x16] = '!',
    [0x1A] = 'Z',  [0x1B] = 'S',  [0x1C] = 'A',  [0x1D] = 'W',  [0x1E] = '@',
    [0x21] = 'C',  [0x22] = 'X',  [0x23] = 'D',  [0x24] = 'E',  [0x25] = '$',  [0x26] = '#',
    [0x29] = ' ',  [0x2A] = 'V',  [0x2B] = 'F',  [0x2C] = 'T',  [0x2D] = 'R',  [0x2E] = '%',
    [0x31] = 'N',  [0x32] = 'B',  [0x33] = 'H',  [0x34] = 'G',  [0x35] = 'Y',  [0x36] = '^',
    [0x3A] = 'M',  [0x3B] = 'J',  [0x3C] = 'U',  [0x3D] = '&',  [0x3E] = '*',
    [0x41] = '<',  [0x42] = 'K',  [0x43] = 'I',  [0x44] = 'O',  [0x45] = ')',  [0x46] = '(',
    [0x49] = '>',  [0x4A] = '?',  [0x4B] = 'L',  [0x4C] = ':',  [0x4D] = 'P',  [0x4E] = '_',
    [0x52] = '"',  [0x54] = '{',  [0x55] = '+',
    [0x5A] = '\n', [0x5B] = '}',  [0x5D] = '|',
    [0x66] = '\b', [0x76] = 27,
    [0x69] = '1',  [0x6B] = '4',  [0x6C] = '7',
    [0x70] = '0',  [0x71] = '.',  [0x72] = '2',  [0x73] = '5',  [0x74] = '6',  [0x75] = '8',
    [0x79] = '+',  [0x7A] = '3',  [0x7B] = '-',  [0x7C] = '*',  [0x7D] = '9',
};

/* Usage sketch (state machine for one byte at a time):
 *
 *   static int release = 0, shift = 0, caps = 0;
 *
 *   int sc2_feed(uint8_t b) {            // returns ASCII, or 0 if none
 *       if (b == SC2_EXT_PREFIX) return 0;       // handle separately if needed
 *       if (b == SC2_BREAK_PREFIX) { release = 1; return 0; }
 *       if (b == SC2_LSHIFT || b == SC2_RSHIFT) { shift = !release; release = 0; return 0; }
 *       if (b == SC2_CAPS && !release) caps ^= 1;
 *       if (release || b > 0x7F) { release = 0; return 0; }
 *       char c = shift ? sc2_shift[b] : sc2_normal[b];
 *       if (caps && c >= 'a' && c <= 'z') c -= 32;
 *       else if (caps && c >= 'A' && c <= 'Z' && shift) c += 32;
 *       return c;
 *   }
 */
#endif //ICARUS_PS2_SCANCODE_H

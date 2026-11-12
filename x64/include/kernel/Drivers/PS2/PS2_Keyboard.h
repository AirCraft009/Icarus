//
// Created by cocon on 11.10.2026.
//

#ifndef ICARUS_KEYBOARD_H
#define ICARUS_KEYBOARD_H
#pragma once
#include <stdint.h>

void keyboard_ps2_init();
char kbd_translate(uint8_t sc);

#endif //ICARUS_KEYBOARD_H

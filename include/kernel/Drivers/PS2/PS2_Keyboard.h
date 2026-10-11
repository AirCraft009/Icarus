//
// Created by cocon on 11.10.2026.
//

#ifndef ICARUS_KEYBOARD_H
#define ICARUS_KEYBOARD_H
#pragma once
#include <stdint.h>

void init_keyboard_ps2();
char kbd_translate(uint8_t sc);

#endif //ICARUS_KEYBOARD_H

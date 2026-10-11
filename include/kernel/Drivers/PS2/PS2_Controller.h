//
// Created by cocon on 11.10.2026.
//

#ifndef ICARUS_PS2_CONTROLLER_H
#define ICARUS_PS2_CONTROLLER_H
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct PS2_INFO_STRUCT {
    bool exists;
    bool enabled;
    bool is_dual_channel;
    uint8_t config;
} PS2_info_t;

extern PS2_info_t PS2_info;

int init_ps2_controller();
int ps2_read_timeout();

#endif //ICARUS_PS2_CONTROLLER_H

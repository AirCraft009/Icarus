//
// Created by Mxsxll on 21.09.2026.
//

#ifndef ICARUS_TSS_H
#define ICARUS_TSS_H
#pragma once
#include <stdint.h>

typedef struct TSS_Entry {
    uint8_t l_w1;
    uint8_t l_w2;
    uint8_t h_w1;
    uint8_t h_w2;
} __attribute__((packed)) tss_entry_t;

__attribute__((aligned(0x10)))
static tss_entry_t tss[26];

void init_tss();
#endif //ICARUS_TSS_H

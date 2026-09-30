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

typedef struct __attribute__((packed)) {
    uint32_t reserved0;
    uint64_t rsp0, rsp1, rsp2;
    uint64_t reserved1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss_t;


void init_tss(tss_t *tss);
#endif //ICARUS_TSS_H

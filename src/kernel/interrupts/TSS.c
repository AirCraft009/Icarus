//
// Created by Mxsxll on 21.09.2026.
//

#include "TSS.h"

#include <stdint.h>


typedef struct TSS_Entry {
    uint8_t l_w1;
    uint8_t l_w2;
    uint8_t h_w1;
    uint8_t h_w2;
} __attribute__((packed)) tss_entry_t;

__attribute__((aligned(0x10)))
static tss_entry_t tss[25];

void init_tss() {
    for (int i = 0; i < 25; i++) {

    }
}


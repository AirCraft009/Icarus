//
// Created by cocon on 20.09.2026.
//

#include "memory_mapping.h"

#include <stdint.h>

/*
 *  handles paging in and out
*/

extern uint64_t pml4[512], pdpt[512], pd[512], pt[512];

int page_in(uint64_t physical_address, uint64_t virtual_address, uint8_t pageT) {
    uint64_t size;
    switch(pageT) {
        case NPage:
            size = PageS;
            break;
        case HPage:
            size = HugePS;
            break;
        case SPage:
            size = SuperPs;
            break;
        default:
            return -1;
    }


}


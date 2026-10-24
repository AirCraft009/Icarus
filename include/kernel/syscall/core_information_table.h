//
// Created by cocon on 24.10.2026.
//

#ifndef ICARUS_CORE_LOCAL_STORAGE_H
#define ICARUS_CORE_LOCAL_STORAGE_H
#pragma once
#include <stdint.h>

typedef struct LOCAL_CORE_KERNEL_INFORMATION_TABLE {
     uint64_t selfPointer;        // offset 0: address of this structure itself
     uint64_t kernelStackTop;     // offset 8
     uint64_t savedUserStack;     // offset 16
     uint64_t scratchSpace;       // offset 24
}lcore_ktable_t;


#endif //ICARUS_CORE_LOCAL_STORAGE_H

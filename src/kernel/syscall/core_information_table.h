//
// Created by cocon on 24.10.2026.
//

#ifndef ICARUS_CORE_LOCAL_STORAGE_H
#define ICARUS_CORE_LOCAL_STORAGE_H
#pragma once
#include <stdint.h>

typedef struct LOCAL_CORE_KERNEL_INFORMATION_TABLE {
     uint64_t rsp_scratch_space;             // during syscall into kernel space store rsp here
     uint16_t rsp_proper;                    // store kernel rsp here
}lcore_ktable_t;

typedef struct LOCAL_CORE_USR_INFORMATION_TABLE {

}lcore_utable_t;

#endif //ICARUS_CORE_LOCAL_STORAGE_H

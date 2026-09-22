//
// Created by cocon on 21.09.2026.
//

#ifndef ICARUS_MULTIBOOT_H
#define ICARUS_MULTIBOOT_H
#pragma once


#define MULTIBOOT2_MAGIC 920085129
#include <stdint.h>

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_info {
    uint32_t total_size;
    uint32_t reserved;
    struct multiboot_tag tags[];
};

typedef struct multiboot_memory_info {
    
}mem_info;

typedef struct multiboot_sys_info {
    uint64_t memory_size;

}sys_info;

sys_info handle_multiboot_info(struct multiboot_info *mboot);

#endif //ICARUS_MULTIBOOT_H

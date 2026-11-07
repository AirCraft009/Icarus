//
// Created by cocon on 29.10.2026.
//

#ifndef ICARUS_TAR_PARSE_H
#define ICARUS_TAR_PARSE_H
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/multiboot2.h"

unsigned int parse(virt_addr_t address);
int tar_mb2_init(struct multiboot_tag_module * mb2_module);
extern struct tar_header **headers;
#endif //ICARUS_TAR_PARSE_H

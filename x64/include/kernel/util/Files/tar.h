//
// Created by cocon on 29.10.2026.
//

#ifndef ICARUS_TAR_PARSE_H
#define ICARUS_TAR_PARSE_H
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/multiboot2.h"


struct tar_header
{
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

unsigned int tar_parse(virt_addr_t address);
int tar_mb2_init(struct multiboot_tag_module * mb2_module);
extern struct tar_header **files;
#endif //ICARUS_TAR_PARSE_H

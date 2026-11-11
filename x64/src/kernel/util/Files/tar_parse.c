//
// Created by cocon on 29.10.2026.
//

#include "kernel/util/Files/tar_parse.h"

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/paging.h"
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/shellio.h"

// Snippets from osdev wiki
// errors in source on osdev in parse_tar
//https://wiki.osdev.org/Tar

#define  TAR_BLOCK_SIZE 512

struct tar_header **files;

unsigned int tar_getsize(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

static int tar_is_zero_block(const void *ptr)
{
    const unsigned char *p = ptr;

    for (unsigned int i = 0; i < 512; i++)
    {
        if (p[i] != 0)
            return 0;
    }

    return 1;
}

unsigned int tar_parse(virt_addr_t address)
{

    unsigned int i;

    for (i = 0; ; i++)
    {
        struct tar_header *header = (struct tar_header *)address;

        // End-of-archive marker
        if (tar_is_zero_block(header))
            break;

        unsigned int size = tar_getsize(header->size);


        if (header->typeflag[0] == '0' || header->typeflag[0] == '\0')
            files[i] = header;

        // Move to the next 512-byte-aligned header
        address += ((size / 512) + 1) * 512;

        if (size % 512)
            address += 512;
    }

    return i;
}

static uint64_t parse_octal(const char *text, uint64_t length) {
    uint64_t value = 0;
    for (uint64_t index = 0; index < length && text[index] >= '0' && text[index] <= '7'; index++) {
        value = value * 8 + (uint64_t)(text[index] - '0');
    }
    return value;
}

uint64_t tar_count_files(const void *archive) {

    unsigned int i;

    for (i = 0; ; i++)
    {
        struct tar_header *header = (struct tar_header *)archive;

        // End-of-archive marker
        if (header->filename[0] == '\0')
            break;

        if (header->typeflag[0] != '0' && header->typeflag[0] != '\0')
            i--;        // only count files. so decrement if type is not file

        unsigned int size = tar_getsize(header->size);


        archive += ((size / 512) + 1) * 512;

        if (size % 512)
            archive += 512;
    }

    return i;
}

int tar_mb2_init(struct multiboot_tag_module * mb2_module) {
    Iprintf("tar_mb2_init\n");
    uint64_t count = tar_count_files((void *) KERNEL_PHYS_TO_VIRT(mb2_module->mod_start));
    Iprintf("tar_mb2_init %i\n", count);
    files = (struct tar_header **) Imalloc(sizeof (struct tar_header *) * count);
    int check_count = tar_parse(KERNEL_PHYS_TO_VIRT(mb2_module->mod_start));
    char * data = (void *) files[3] + 512;
    Iprintf("%s, %c%c%c\n",files[3]->filename, data[1], data[2], data[3]);
    if (check_count != count) {return -1;}
    return check_count;
}
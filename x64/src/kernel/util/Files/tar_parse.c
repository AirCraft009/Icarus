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
//https://wiki.osdev.org/Tar

#define  TAR_BLOCK_SIZE 512

struct tar_header **headers;

unsigned int tar_getsize(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

unsigned int tar_parse(virt_addr_t address)
{

    unsigned int i;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (header->filename[i] == '\0')
            break;

        Iprintf("%s\n", header->filename);

        unsigned int size = tar_getsize(header->size);

        headers[i] = header;

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

uint64_t tar_count_files(const void *archive_start, uint32_t archive_length) {
    const uint8_t *cursor = archive_start;
    const uint8_t *archive_end = cursor + archive_length;
    uint64_t file_count = 0;
    while (cursor + TAR_BLOCK_SIZE <= archive_end) {
        const struct tar_header *header = (const struct tar_header *)cursor;

        if (header->filename[0] == '\0') {
            break; // zero block = end of archive
        }

        uint64_t file_size = parse_octal(header->size, sizeof(header->size));

        if (header->typeflag[0] == '0' || header->typeflag[0] == '\0') {
            file_count++;
        }

        uint64_t data_blocks = (file_size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
        cursor += TAR_BLOCK_SIZE + data_blocks * TAR_BLOCK_SIZE;
    }

    return file_count;
}

int tar_mb2_init(struct multiboot_tag_module * mb2_module) {
    //!!!! set size to the in memory size of the module instead of the size of the tag!!!!!!!!
    uint64_t size = mb2_module->mod_end - mb2_module->mod_start;
    uint64_t count = tar_count_files((void *) KERNEL_PHYS_TO_VIRT(mb2_module->mod_start), size);
    headers = (struct tar_header **) Imalloc(sizeof (struct tar_header *) * count);
    int check_count = tar_parse(KERNEL_PHYS_TO_VIRT(mb2_module->mod_start));
    char * data = (void *) headers[3] + 0x800;
    Iprintf("%s, %c%c%c\n",headers[3]->filename, data, data[1], data[2]);
    if (check_count != count) {return -1;}
    return check_count;
}
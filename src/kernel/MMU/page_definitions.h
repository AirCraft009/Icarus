//
// Created by Mxsxll on 21.09.2026.
//

#ifndef ICARUS_PAGE_DEFINITIONS_H
#define ICARUS_PAGE_DEFINITIONS_H
#pragma once
#include <stdint.h>

/**
 * represents a pml4 table entry
 */
typedef struct PageMapL4Entry {
    uint64_t present                   :1;
    uint64_t writeable                 :1;
    uint64_t user_access               :1;
    uint64_t write_through             :1;
    uint64_t cache_disabled            :1;
    uint64_t accessed                  :1;
    uint64_t ignored_3                 :1;
    uint64_t size                      :1; // must be 0
    uint64_t ignored_2                 :4;
    uint64_t page_ppn                  :28;
    uint64_t reserved_1                :12; // must be 0
    uint64_t ignored_1                 :11;
    uint64_t execution_disabled        :1;
}__attribute__((__packed__)) page_map_l4_entry;

typedef struct PageDirPointerTablePageDirEntry
{
    uint64_t present                   :1;
    uint64_t writeable                 :1;
    uint64_t user_access               :1;
    uint64_t write_through             :1;
    uint64_t cache_disabled            :1;
    uint64_t accessed                  :1;
    uint64_t ignored_3                 :1;
    uint64_t huge                      :1; // 0 means page directory mapped
    uint64_t ignored_2                 :4;
    uint64_t page_ppn                  :28;
    uint64_t reserved_1                :12; // must be 0
    uint64_t ignored_1                 :11;
    uint64_t execution_disabled        :1;
} __attribute__((__packed__)) pdpt_entry_t;

typedef struct PageDirPageTableEntry
{
    uint64_t present                   :1;
    uint64_t writeable                 :1;
    uint64_t user_access               :1;
    uint64_t write_through             :1;
    uint64_t cache_disabled            :1;
    uint64_t accessed                  :1;
    uint64_t ignored_3                 :1;
    uint64_t huge                      :1; // 0 means page table mapped
    uint64_t ignored_2                 :4;
    uint64_t page_ppn                  :28;
    uint64_t reserved_1                :12; // must be 0
    uint64_t ignored_1                 :11;
    uint64_t execution_disabled        :1;
} __attribute__((__packed__)) pd_entry_t;

typedef struct
{
    uint64_t present                   :1;
    uint64_t writeable                 :1;
    uint64_t user_access               :1;
    uint64_t write_through             :1;
    uint64_t cache_disabled            :1;
    uint64_t accessed                  :1;
    uint64_t dirty                     :1;
    uint64_t size                      :1;
    uint64_t global                    :1;
    uint64_t ignored_2                 :3;
    uint64_t page_ppn                  :28;
    uint64_t reserved_1                :12; // must be 0
    uint64_t ignored_1                 :11;
    uint64_t execution_disabled        :1;
} __attribute__((__packed__)) PageTableEntry;

#endif //ICARUS_PAGE_DEFINITIONS_H

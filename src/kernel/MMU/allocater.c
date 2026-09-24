//
// Created by cocon on 23.09.2026.
//

#include "allocater.h"
#include <stddef.h>
#include "../kernel_info.h"

void * alloc_frame(mem_map *mmap, uint64_t page_size) {
    // default page size = 4096 while the page size param could be a huge or super page
    int cont_units = DEFAULT_PAGE_SIZE / page_size;
    int count = 0;
    for (uint64_t i = 0; i < mmap->size; i++) {
        if (mmap->data[i]) {
            count++;
        }
        if (count == cont_units) {
            return (void *) (i - (cont_units - 1));
        }
    }
    return NULL;
}


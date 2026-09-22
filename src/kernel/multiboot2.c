//
// Created by cocon on 21.09.2026.
//

#include "multiboot2.h"

#include <stdbool.h>
#include <stdint.h>

sys_info handle_multiboot_info(struct multiboot_info *mboot) {
    sys_info info = (sys_info) {};

    uint8_t *p = (uint8_t *)mboot->tags;

    while (true) {
        struct multiboot_tag *tag = (struct multiboot_tag *)p;

        if (tag->type == 0)
            break;

        // use tag...

        p += (tag->size + 7) & ~7;
    }
    return info;
}

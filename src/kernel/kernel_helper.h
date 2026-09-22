//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_KERNEL_HELPER_H
#define ICARUS_KERNEL_HELPER_H
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#endif //ICARUS_KERNEL_HELPER_H

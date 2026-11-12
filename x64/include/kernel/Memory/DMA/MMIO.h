//
// Created by cocon on 02.10.2026.
//

#ifndef ICARUS_MMIO_H
#define ICARUS_MMIO_H
#pragma once
#include <stdint.h>

#include "kernel/Memory/PMM/mem_map.h"

virt_addr_t map_mmio(phys_addr_t phys_addr, uint64_t size, uint64_t flags);

#endif //ICARUS_MMIO_H

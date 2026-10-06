//
// Created by cocon on 06.10.2026.
//

#ifndef ICARUS_MADT_H
#define ICARUS_MADT_H
#pragma once
#include "kernel/Memory/DMA/ACPI.h"

int handle_MADT(struct ACPISDTHeader *header);

#endif //ICARUS_MADT_H

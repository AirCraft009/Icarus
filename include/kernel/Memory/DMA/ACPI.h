//
// Created by cocon on 02.10.2026.
//

#ifndef ICARUS_ACPI_H
#define ICARUS_ACPI_H
#pragma once
#include "kernel/util/multiboot2.h"



int handle_new_acpi(struct XSDP_t * rsdp);
int handle_old_acpi(struct RSDP_t * rsdp);

#endif //ICARUS_ACPI_H


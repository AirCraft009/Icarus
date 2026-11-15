//
// Created by cocon on 29.10.2026.
//

#include "kernel/util/Files/elf.h"

#include <stdint.h>

#include "string.h"
#include "kernel/Processes/Process.h"

/**
 *
 * validate an elf header
 *
 * @param file The ELF in memory
 * @return 0 (OK), -1(FAIL)
 */
int elf_validate_headers(elf_header_t * header) {

    if (strncmp(&header->e_ident_c[EI_MAG0], ELF_MAGIC, 4) != 0)
        return -1;

    // only allow statically linked exec
    //TODO: expand to dynamic
    if (header->e_type != ET_EXEC)
        return -1;

    if (header->e_version == 0)
        return -1;

    if (header->e_machine != EM_X86_64)
        return -1;

    if (header->e_ehsize != 64)
        return -1;




    return 0;
}

int load_elf(elf_header_t * elf_file, PID_t current_pid, bool is_privileged, uint16_t priority) {
    if (elf_validate_headers(elf_file))
        return -1;

    elf_program_header_t * p_headers = (elf_program_header_t *)elf_file->e_phoff;

    for (int i = 0; i < elf_file->e_phnum; i++) {
        // only handle the static LOAD types
        //TODO: expand to dynamic
        if (p_headers[i].p_type != PT_LOAD)
            continue;

        // load section into mem
        create_process(
            current_pid,
            is_privileged,
            priority,
            elf_file->e_entry,

        )
    }
}

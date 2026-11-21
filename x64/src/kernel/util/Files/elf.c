//
// Created by cocon on 29.10.2026.
//

#include "kernel/util/Files/elf.h"

#include <stdint.h>

#include "kernel_helper.h"
#include "string.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/paging.h"
#include "kernel/Processes/Process.h"
#include "kernel/util/shellio.h"
#include "kernel/util/Files/tar.h"

/**
 *
 * validate an elf header
 *
 * @param header The ELF in memory
 * @return 0 (OK), -1(FAIL)
 */
int elf_validate_headers(elf_header_t * header) {
    if (strncmp(&header->e_ident_c[EI_MAG0], ELF_MAGIC, 4) != 0)
        return -1;

    // only allow statically linked exec
    //TODO: expand to dynamic
    if (header->e_type != ET_EXEC)
        return -2;

    if (header->e_version == 0)
        return -3;

    if (header->e_machine != EM_X86_64)
        return -4;

    if (header->e_ehsize != 64)
        return -5;

    return 0;
}

/**
 *
 * loads an ELF as an executable
 *  - spawns process
 *  - maps regions into memory
 *  - side effect: cli, interrupts are disabled
 *
 * @param elf_file ptr to the elf file in memory
 * @param current_pid process id of the parent
 * @param is_privileged wether the process is spawned priviledged or not ( only a priv proc can spawn a priv )
 * @param priority the scheduling priority (floor(parent_prio))
 * @return 0 on sucess -1 on fail (invalid header, allocation failure, etc...)
 */
int load_elf(elf_header_t * elf_file, PID_t current_pid, bool is_privileged, uint16_t priority) {
    int validate = elf_validate_headers(elf_file);
    if (validate != 0) {
        Iprintf("ELF validation failed: %i\n", validate);
        return validate;
    }

    elf_program_header_t * p_headers = (elf_program_header_t *) ((void *) elf_file + elf_file->e_phoff);
    pcb_t * pcb = create_process(current_pid, is_privileged, priority, STANDARD_STACK_PAGES * DEFAULT_PAGE_SIZE, elf_file->e_entry);

    for (int i = 0; i < elf_file->e_phnum; i++) {
        // only handle the static LOAD types
        //TODO: expand to dynamic
        if (p_headers[i].p_type != PT_LOAD)
            continue;

        //Iprintf("p_header(%x)\n", p_headers[i].p_vaddr);

        // load section into mem
        uint64_t offset_in_page   = p_headers[i].p_vaddr & (DEFAULT_PAGE_SIZE - 1);
        uint64_t segment_base     = p_headers[i].p_vaddr - offset_in_page;
        uint64_t segment_pages    = ALIGN_UP(offset_in_page + p_headers[i].p_memsz, DEFAULT_PAGE_SIZE) / DEFAULT_PAGE_SIZE;

        struct_page_t * sp = ialloc_frames(segment_pages, DEFAULT_PAGE);
        if (sp == (void *) FRAME_ALLOC_FAILED)
            return -1;

        // TODO: replace with mapping functions from Process.h
        for (uint64_t j = 0; j < sp->page_count; j ++) {
            if (page_in
            (
                (void *) KERNEL_PHYS_TO_VIRT(pcb->head_thread->page_table),
                (void *) p_headers[i].p_vaddr + DEFAULT_PAGE_SIZE * j,
                (void *) sp->page_addr[j],
                DEFAULT_PAGE_SIZE,
                PT_USER_ACCESS | PT_WRITEABLE
            )!= 0)
                return -1;
        }

        uint8_t * frame_start = (uint8_t *) KERNEL_PHYS_TO_VIRT(sp->page_addr[0]);
        memset(frame_start, 0, segment_pages * DEFAULT_PAGE_SIZE);
        memcpy(frame_start + offset_in_page,
               (uint8_t *) elf_file + p_headers[i].p_offset,
               p_headers[i].p_filesz);
    }
    return 0;
}

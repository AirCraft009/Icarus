//
// Created by cocon on 13.10.2026.
//

#include "kernel/Processes/processes.h"

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/mem_utils.h"
static PID_t PID_GEN = 0;

tcb_t * create_proc(PID_t parent, bool is_priviledged,  uint64_t initial_pages, PAGE_SIZES page_type) {
    tcb_t * pcb = (tcb_t *) Imalloc(sizeof(tcb_t));
    Imemset(pcb, 0, sizeof(tcb_t));
    pcb->pid = PID_GEN ++;
    pcb->parent_pid = parent;

    struct_page_t * start_frame = ialloc_frames(initial_pages, page_type);
    page_map_l4_entry * proc_pml4 = Imalloc(sizeof(page_map_l4_entry) * 512);
    // current pml4 js has the higher mapped kernel, by copying its zeroing lower and higher mapping the kernel
    page_map_l4_entry * cur_pml4 = (page_map_l4_entry *) read_cr3();
    Imemccpy(proc_pml4, cur_pml4, sizeof(page_map_l4_entry) * 512);
    pcb->page_table = proc_pml4;
    pcb->state = PROCESS_NEW;
    pcb->privileged = is_priviledged;
    return pcb;
}


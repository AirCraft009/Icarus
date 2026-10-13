//
// Created by cocon on 13.10.2026.
//

#include "scheduler.h"

#include "kernel/Memory/alloc.h"
#include "kernel/Processes/process_control_block.h"

static PID_t PID_GEN = 0;

pcb_t * create_proc(PID_t parent, uint64_t initial_pages, uint64_t page_size) {
    pcb_t * pcb = (pcb_t *) Imalloc(sizeof(pcb_t));
    pcb->pid = PID_GEN ++;
    pcb->parent_pid = parent;

    struct_page_t * start_frame = ialloc_frames(initial_pages, page_size);

    return pcb;
}



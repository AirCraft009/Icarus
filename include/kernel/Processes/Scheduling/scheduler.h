//
// Created by cocon on 13.10.2026.
//

#ifndef ICARUS_SCHEDULER_H
#define ICARUS_SCHEDULER_H
#pragma once
#include "kernel/interrupts/ISR.h"
#include "kernel/Processes/thread.h"



void schedule(struct interrupt_frame *frame, INTERRUPT_SRC src);
void scheduler_add_thread(tcb_t *tcb);
void scheduler_init(tcb_t * tcb);
tcb_t * spawn_alloc_thread(
    PID_t parent,
    bool is_priviledged,
    uint16_t priority,
    thread_memory_allocation_info_t *alloc_info,
    thread_memory_permission_info_t *perms,
    thread_memory_supply_block_t *supply
);

#endif //ICARUS_SCHEDULER_H

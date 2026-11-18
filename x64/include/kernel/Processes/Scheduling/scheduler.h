//
// Created by cocon on 13.10.2026.
//

#ifndef ICARUS_SCHEDULER_H
#define ICARUS_SCHEDULER_H
#pragma once
#include "kernel/interrupts/ISR.h"
#include "kernel/Processes/Process.h"
#include "kernel/Processes/thread.h"



void schedule(struct interrupt_frame *frame, INTERRUPT_SRC src);
void scheduler_add_thread(tcb_t *tcb);
void scheduler_init(tcb_t * tcb);
tcb_t * spawn_thread(pcb_t *parent, virt_addr_t p_virt);
int scheduler_stop_thread(tcb_t * thread);
void scheduler_start(uint64_t ms_tick);

#endif //ICARUS_SCHEDULER_H

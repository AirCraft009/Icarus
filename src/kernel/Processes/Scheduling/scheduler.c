//
// Created by cocon on 13.10.2026.
//

#include "scheduler.h"

#include "kernel/Processes/processes.h"

// holds the pcb currently being switched to
tcb_t *cur_pcb;
// holds the pcb that is last in the waiting queue
tcb_t *last_pcb;






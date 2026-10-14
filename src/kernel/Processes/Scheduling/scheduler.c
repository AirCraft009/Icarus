//
// Created by cocon on 13.10.2026.
//

#include "scheduler.h"

#include "kernel/Processes/process_control_block.h"

// holds the pcb currently being switched to
pcb_t *cur_pcb;
// holds the pcb that is last in the waiting queue
pcb_t *last_pcb;






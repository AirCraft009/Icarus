//
// Created by cocon on 20.10.2026.
//

#ifndef ICARUS_PROCESS_H
#define ICARUS_PROCESS_H
#include "thread.h"
typedef uint64_t PID_t;

typedef struct {
    tcb_t * tcb;
    tcb_t * next_tcb;
} thread_descriptor_t;

typedef struct PROCESS_CONTROL_BLOCK {
    PID_t parent_pid;
    PID_t pid;
    uint16_t init_priority;
    thread_descriptor_t * process_threads;      // linked list of threads belonging to this proc
}pcb_t;

pcb_t * create_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    const void * code_location,
    uint64_t code_size,
    thread_memory_allocation_info_t *allocation_info
);

pcb_t * create_initial_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    const void * code_location,
    uint64_t code_size,
    thread_memory_allocation_info_t *allocation_info
);

#endif //ICARUS_PROCESS_H

//
// Created by cocon on 20.10.2026.
//

#ifndef ICARUS_PROCESS_H
#define ICARUS_PROCESS_H
#include "thread.h"
#include "stdatomic.h"

typedef uint64_t PID_t;

typedef struct THREAD_DESCRIPTOR{
    tcb_t * tcb;
    struct THREAD_DESCRIPTOR * next_tcb;
} thread_descriptor_t;

typedef struct PROCESS_CONTROL_BLOCK {
    PID_t parent_pid;
    PID_t pid;
    uint16_t init_priority;
    bool blocked;
    tcb_t * head_thread;
    thread_descriptor_t * process_threads;      // linked list of threads belonging to this proc
}pcb_t;

pcb_t * create_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t code_location,
    const void * src_code,
    uint64_t code_size,
    uint64_t stack_size
);

pcb_t * create_initial_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t code_location,
    const void * src_code,
    uint64_t code_size,
    uint64_t stack_size
);

#endif //ICARUS_PROCESS_H

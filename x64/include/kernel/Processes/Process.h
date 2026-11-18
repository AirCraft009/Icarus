//
// Created by cocon on 20.10.2026.
//

#ifndef ICARUS_PROCESS_H
#define ICARUS_PROCESS_H
#include "thread.h"
#define STANDARD_STACK_PAGES 10

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
    // process_threads has head thread last
    thread_descriptor_t * process_threads;      // linked list of threads belonging to this proc
}pcb_t;

pcb_t * create_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t stack_size,
    virt_addr_t exec_start
);

pcb_t * create_initial_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t stack_size,
    virt_addr_t exec_start
);

struct_page_t * process_map_stack
(
    pcb_t * process,
    uint64_t pages,
    PAGE_TYPE p_size,
    uint64_t permissions
);

struct_page_t * process_map
(
    pcb_t * process,
    virt_addr_t virt_addr,
    uint64_t pages,
    PAGE_TYPE p_size,
    uint64_t permissions
);
#endif //ICARUS_PROCESS_H

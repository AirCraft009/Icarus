//
// Created by cocon on 12.10.2026.
//

#ifndef ICARUS_PROCESS_CONTROL_BLOCK_H
#define ICARUS_PROCESS_CONTROL_BLOCK_H
#include "page_definitions.h"
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/cpu_regs.h"

// Define possible process states
typedef enum {
    PROCESS_NEW,        // signifies that the thread doesn't yet have any memory backing it
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;

typedef uint64_t PID_t;
typedef uint64_t TID_t;

/**
 * in virt addr space every tasks mem space starts at the same position
 */
#define PROCESS_VIRT_ADDR 0x0000000000400000

typedef struct THREAD_CONTROL_BLOCK {
    PID_t pid;                   // Process ID
    PID_t parent_pid;            // Parent Process ID
    TID_t tid;                   // thread ID
    cpu_reg_frame regsiters;
    process_state_t state;       // Current state of the process
    uint16_t priority;           // constant priority
    uint16_t active_priority;    // priority in the active switch cycle
    bool privileged;


    // Memory management information
    page_map_l4_entry *page_table;  // physical addr of pml4 table
    uint64_t memory_limit;          // size of memory allocated

    // Accounting
    uint64_t cpu_time_used;
    uint64_t arrival_time;     // Time process was created

    // Pointer to link TCBs in a scheduler queue
    struct THREAD_CONTROL_BLOCK* next;
} tcb_t;

#endif //ICARUS_PROCESS_CONTROL_BLOCK_H

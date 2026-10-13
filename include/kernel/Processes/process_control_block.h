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
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;

typedef uint64_t PID_t;


typedef struct PROCESS_CONTROL_BLOCK {
    PID_t pid;                   // Process ID
    PID_t parent_pid;            // Parent Process ID
    cpu_reg_frame registers;     // Saved CPU registers for context switching
    process_state_t state;             // Current state of the process
    uint8_t priority;               // Scheduling priority

    // Memory management information
    page_map_l4_entry *page_table;  // physical addr of pml4 table
    virt_addr_t memory_base;        // Base address in memory
    uint64_t memory_limit;          // size of memory allocated

    // Accounting
    uint64_t cpu_time_used;
    uint64_t arrival_time;     // Time process was created

    // Pointer to link PCBs in a scheduler queue
    struct PROCESS_CONTROL_BLOCK* next;
} pcb_t;

#endif //ICARUS_PROCESS_CONTROL_BLOCK_H

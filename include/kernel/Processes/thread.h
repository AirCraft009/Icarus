//
// Created by cocon on 12.10.2026.
//

#ifndef ICARUS_PROCESS_CONTROL_BLOCK_H
#define ICARUS_PROCESS_CONTROL_BLOCK_H
#include "page_definitions.h"
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/cpu_regs.h"

// Define possible process states
typedef enum PROCESS_STATE {
    PROCESS_NEW,        // signifies that the thread doesn't yet have any memory backing it
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED,
    PROCESS_DENIED
} process_state_t;

typedef uint64_t PID_t;
typedef uint64_t TID_t;

/**
 * in virt addr space every tasks code segment starts at the same position
 */
#define PROCESS_CODE_VIRT_ADDR 0x0000048000000000            // entry 9 in the pml4 table bc I is the 9th letter in the alphabet
#define PROCESS_MAPPINGS_VIRT_ADDR 0x0000090000000000            // entry 18 in the pml4 table bc I is the 9th letter in the alphabet
#define PROCESS_STACK_VIRT_ADDR 0x00007C8000000000

typedef enum {
    SUBSPACE_CODE,         // encompasses .text, .bss ... + heap
    SUBSPACE_MAPPING,      // common mappings of libraries io buffers etc.
    SUBSPACE_STACK,        // stack addr and down
}ADDR_SUB_SPACE;

#define SUB_SPACE_TO_ADDR(sub_space) ((sub_space == SUBSPACE_CODE)? PROCESS_CODE_VIRT_ADDR : (sub_space == SUBSPACE_MAPPING)? PROCESS_MAPPINGS_VIRT_ADDR : PROCESS_STACK_VIRT_ADDR)


/*
 * represents the sizes of each memory subspace
 * union so that I can index into the fields w/ the ADDR_SUB_SPACE ENUM
 */
typedef union THREAD_MEMORY {
    struct {
        uint64_t code_size;
        uint64_t mapping_size;
        uint64_t stack_size;
        uint64_t memory_limit;
    };
    uint64_t fields[4];
} thread_memory_t;

typedef struct THREAD_MEMORY_SUPPLY_BLOCK {
    struct_page_t * code_memory;
    struct_page_t * mapping_memory;
    struct_page_t * stack_memory;
}thread_memory_supply_block_t;

typedef struct THREAD_MEMORY_ALLOCATION_INFO {
    uint64_t code_pages;
    uint64_t code_page_size;
    uint64_t mapping_pages;
    uint64_t mapping_page_size;
    uint64_t stack_pages;
    uint64_t stack_page_size;
}thread_memory_allocation_info_t;

typedef struct THREAD_MEMORY_PERMISSION_INFO {
    uint64_t code_perms;
    uint64_t mapping_perms;
    uint64_t stack_perms;
}thread_memory_permission_info_t;

typedef struct THREAD_CONTROL_BLOCK {
    PID_t pid;                   // Process ID
    PID_t parent_pid;            // Parent Process ID
    TID_t tid;                   // thread ID
    cpu_regs regsiters;
    // _Atomic bc it is a flag for the scheduler
    // e.g the transition from New -> Ready
    process_state_t state;       // Current state of the process
    uint16_t priority;           // constant priority
    uint16_t active_priority;    // priority in the active switch cycle
    bool privileged;



    // Memory management information
    page_map_l4_entry *page_table;  // physical addr of pml4 table
    thread_memory_t memory;

    // Accounting
    uint64_t cpu_time_used;
    uint64_t arrival_time;     // Time process was created

    // Pointer to link TCBs in a scheduler queue
    struct THREAD_CONTROL_BLOCK* next;
} tcb_t;

tcb_t * create_thread(PID_t parent, bool is_priviledged, uint16_t priority);
int thread_supply_mem_region(tcb_t *thread, ADDR_SUB_SPACE sub_space, struct_page_t * info_page, uint64_t permissions);
struct_page_t * thread_supply_new_mem_region(tcb_t *thread, ADDR_SUB_SPACE sub_space,  uint64_t pages, PAGE_TYPE p_size, uint64_t permissions);
int * thread_supply_mem(tcb_t * thread, thread_memory_supply_block_t supply_block, thread_memory_permission_info_t perms);
thread_memory_supply_block_t * thread_supply_new_mem(
    tcb_t *thread,
    thread_memory_allocation_info_t allocation_info,
    thread_memory_permission_info_t perm_info);

#endif //ICARUS_PROCESS_CONTROL_BLOCK_H

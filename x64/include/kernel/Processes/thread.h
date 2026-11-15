//
// Created by cocon on 12.10.2026.
//

#ifndef ICARUS_THREAD_H
#define ICARUS_THREAD_H
#include "page_definitions.h"
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/cpu_regs.h"
typedef uint64_t PID_t;
typedef uint64_t TID_t;

// Define possible process states
typedef enum THREAD_STATE {
    THREAD_NEW,        // signifies that the thread doesn't yet have any memory backing it
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_WAITING,
    THREAD_TERMINATED,
    THREAD_DENIED
} THREAD_state_t;


/**
 * in virt addr space every tasks code segment starts at the same position
 */
#define THREAD_MAPPINGS_VIRT_ADDR 0x00006FFFFFFFFFFF
#define THREAD_STACK_VIRT_ADDR 0x00007FFFFFFFFFFF

typedef enum {
    SUBSPACE_CODE,         // encompasses .text, .bss ... + heap
    SUBSPACE_MAPPING,      // common mappings of libraries io buffers etc.
    SUBSPACE_STACK,        // stack addr and down
}ADDR_SUB_SPACE;

#define SUB_SPACE_TO_ADDR(sub_space, code_start) ((sub_space == SUBSPACE_CODE)? code_start : (sub_space == SUBSPACE_MAPPING)? THREAD_MAPPINGS_VIRT_ADDR : THREAD_STACK_VIRT_ADDR)

typedef struct THREAD_MEM_ENTRY {
    void * start;
    uint64_t size;
    uint8_t permissions;
} thread_memory_entry_t;

/*
 * contains all allocations for program header
 */
typedef union THREAD_MEMORY {
    uint32_t entry_count;
    thread_memory_entry_t *entry;
} thread_memory_t;

typedef struct THREAD_MEMORY_INFORMATION {
    union {
        struct {
            uint64_t code_size;
            uint64_t stack_size;
            uint64_t mapping_size;
        };
        uint64_t fields[3];
    };
    uint64_t total_size;
} thread_mem_info;

typedef struct THREAD_CONTROL_BLOCK {
    PID_t pid;                   // Process ID
    TID_t tid;                   // thread ID
    cpu_regs registers;          // registers for restoring cpu state
    uint64_t code_start;
    // _Atomic bc it is a flag for the scheduler
    // e.g the transition from New -> Ready
    THREAD_state_t state;        // Current state of the process
    uint16_t priority;           // constant priority
    uint16_t active_priority;    // priority in the active switch cycle
    bool privileged;



    // Memory management information
    // TODO: add bookkeeping for the UML (unified mapping layer)
    // extract mapped / owned mem from here
    page_map_l4_entry *page_table;  // physical addr of pml4 table
    void * fs_base;                 // TODO: implement fs for per thread local storage
    thread_mem_info mem_info;

    // Accounting
    uint64_t cpu_time_used;
    uint64_t arrival_time;     // Time process was created

    // Pointer to link TCBs in a scheduler queue
    struct THREAD_CONTROL_BLOCK* next;
    struct THREAD_CONTROL_BLOCK* prev;
} tcb_t;

tcb_t * create_thread(PID_t owner_id, bool is_priviledged, uint16_t priority, uint64_t code_start);
int thread_supply_mem_region(tcb_t *thread, virt_addr_t virt_addr, ADDR_SUB_SPACE sub_space, struct_page_t * info_page, uint64_t permissions);
struct_page_t * thread_supply_new_mem_region(tcb_t *thread, virt_addr_t virt_addr, ADDR_SUB_SPACE sub_space, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions);
struct_page_t * thread_map(tcb_t * thread, virt_addr_t virt_addr, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions);
struct_page_t * thread_map_stack(tcb_t * thread, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions);
struct_page_t * thread_map_mapping(tcb_t * thread, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions);
int thread_exit(tcb_t * thread);

#endif //ICARUS_THREAD_H

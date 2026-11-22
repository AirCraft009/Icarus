//
// Created by cocon on 20.10.2026.
//

#include "kernel/Processes/Process.h"

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Processes/Scheduling/scheduler.h"
#include <string.h>

#include "kernel/Memory/paging.h"
#include "kernel/util/shellio.h"

static PID_t PID_GEN = 1;

//TODO: find a process via it's PID
//TODO: fix logic issues when stopping the head-thread
//          - possible issue with use after free
//          - possible problem with general decommissioning


pcb_t * init_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t stack_size,
    virt_addr_t code_virt
) {
    //TODO: implement unified mapping layer and replace mappings w/ a better struct

    PID_t pid = PID_GEN++;

    tcb_t * thread = create_thread(pid, is_priviledged, priority, code_virt);

    if (!thread) {
        return NULL;
    }

    page_map_l4_entry * proc_pml4 = (page_map_l4_entry *) KERNEL_VIRT_TO_PHYS(ialloc_frames(1, DEFAULT_PAGE));      // get physical addr struct page is mapped into k_space
    // current pml4 js has the higher mapped kernel, by copying its zeroing lower and higher mapping the kernel
    page_map_l4_entry * cur_pml4 = (page_map_l4_entry *) read_cr3();
    memcpy( (void *) KERNEL_PHYS_TO_VIRT(proc_pml4), (void *) KERNEL_PHYS_TO_VIRT(cur_pml4), sizeof(page_map_l4_entry) * 512);
    thread->page_table = proc_pml4;
    thread_map_stack
    (
        thread,
        MEM_SIZE_TO_PAGE_COUNT(stack_size, DEFAULT_PAGE),
        DEFAULT_PAGE ,
        PT_WRITEABLE
    );

    pcb_t * pcb = Imalloc(sizeof(pcb_t));
    if (!pcb) {
        free(thread);
        return NULL;
    }
    pcb->init_priority = priority;
    pcb->pid = pid;
    pcb->parent_pid = parent_pid;
    thread_descriptor_t * thread_ptr = Imalloc(sizeof(thread_descriptor_t));
    if (!thread_ptr) {
        free(thread);
        free(pcb);
        return NULL;
    }
    thread_ptr->tcb = thread;
    thread_ptr->next_tcb = NULL;
    pcb->process_threads = thread_ptr;
    pcb->head_thread = thread;

    return pcb;
}

/**
 *
 * creates a process (scheduled collection of threads) with a single thread
 * side effect: disables maskable interrupts
 *
 * @param is_priviledged is the process kernel or userspace
 * @param priority initial priority of the process
 * @param exec_start start of execution (virt addr)
 * @return NULL on failure (if any allocation fails all is freed and the thread is never dispatched)
 */
pcb_t * create_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t stack_size,
    virt_addr_t exec_start
) {
    pcb_t * pcb = init_process(
        parent_pid,
        is_priviledged,
        priority,
        stack_size,
        exec_start
        );

    if (!pcb) return NULL;

    tcb_t * tcb = pcb->process_threads->tcb;
    if (!tcb) return NULL;
    scheduler_add_thread(tcb);
    return pcb;
}

/**
 *
 * creates the initial process for the scheduler
 * use for the first process and spawn every other separately
 *
 * @param is_priviledged is the process kernel or userspace
 * @param priority initial priority of the process
 * @param exec_start virt addr of the execution start
 * @return NULL on failure (if any allocation fails all is freed and the thread is never dispatched)
 */
pcb_t * create_initial_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t stack_size,
    virt_addr_t exec_start
) {
    pcb_t * pcb = init_process(
    parent_pid,
    is_priviledged,
    priority,
    stack_size,
    exec_start
    );

    if (!pcb) return NULL;

    tcb_t * tcb = pcb->head_thread;
    if (!tcb) return NULL;
    scheduler_init(tcb);
    return pcb;
}

void process_exit(pcb_t * pcb) {
    thread_descriptor_t * thread_ptr = pcb->process_threads;
    while (thread_ptr) {
        scheduler_stop_thread(thread_ptr->tcb);
        thread_ptr = thread_ptr->next_tcb;
    }

}


/**
 *
 * maps new frames into code space
 *
 * @param process process to map into
 * @param virt_addr start virtual addr
 * @param pages number of pages
 * @param p_size size per page
 * @param permissions the page perms
 * @return FRAME_ALLOC_FAILED or the struct_page of the alloc mem
 */
struct_page_t * process_map
(
    pcb_t * process,
    virt_addr_t virt_addr,
    uint64_t pages,
    PAGE_TYPE p_size,
    uint64_t permissions
)
{
    struct_page_t * sp = thread_map(process->head_thread, virt_addr, pages, p_size, permissions);
    if ((virt_addr_t) sp == FRAME_ALLOC_FAILED) {
        return sp;
    };

    // head_thread is the last thread in this linked list
    Iprintf("ON LIST\n");
    thread_descriptor_t * thread_ptr = process->process_threads;
    while (thread_ptr->tcb != process->head_thread && thread_ptr->tcb != NULL) {
        Iprintf("ON LIST 1 %x == %x\n", &process->head_thread, &thread_ptr->tcb);
        thread_ptr->tcb->mem_info.code_size += pages * PAGE_TYPE_TO_SIZE(p_size);
        thread_ptr = thread_ptr->next_tcb;
    }
    return sp;
}


/**
 *
 * maps new frames into the stack_space
 *
 * @param process process to map into
 * @param pages number of pages
 * @param p_size size per page
 * @param permissions the page perms
 * @return -1 on failed alloc or map 0 on success
 */
struct_page_t * process_map_stack
(
    pcb_t * process,
    uint64_t pages,
    PAGE_TYPE p_size,
    uint64_t permissions
)
{
    struct_page_t * sp =thread_map_stack(process->head_thread, pages, p_size, permissions);
    if ((virt_addr_t) sp == FRAME_ALLOC_FAILED) {
        return sp;
    };

    // head_thread is the last thread in this linked list
    thread_descriptor_t * thread_ptr = process->process_threads;
    while (&thread_ptr->tcb != &process->head_thread) {
        thread_ptr->tcb->mem_info.stack_size += pages * PAGE_TYPE_TO_SIZE(p_size);
    }
    return sp;
}
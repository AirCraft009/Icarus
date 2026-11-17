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

//find a process via it's PID

pcb_t * init_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t code_virt,
    const void * src_code,
    uint64_t code_size,
    uint64_t stack_size
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

    struct_page_t * code_p = thread_map
    (
        thread,
        code_virt,
        MEM_SIZE_TO_PAGE_COUNT(code_size, DEFAULT_PAGE),
        DEFAULT_PAGE,
        0
    );

    // TODO: add mappings


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

    // struct page is overridden here and no longer contains freeing info
    memcpy((void *) KERNEL_PHYS_TO_VIRT(code_p->page_addr[0]),(void *) src_code, code_size);

    return pcb;
}

/**
 *
 * creates a process (scheduled collection of threads) with a single thread
 * side effect: disables maskable interrupts
 *
 * @param is_priviledged is the process kernel or userspace
 * @param priority initial priority of the process
 * @param src_code location of copy to execute in mem TODO: expand / replace w/ disk access
 * @param code_size size of code in bytes
 * @return NULL on failure (if any allocation fails all is freed and the thread is never dispatched)
 */
pcb_t * create_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t code_location,
    const void * src_code,
    uint64_t code_size,
    uint64_t stack_size
) {
    pcb_t * pcb = init_process(
        parent_pid,
        is_priviledged,
        priority,
        code_location,
        src_code,
        code_size,
        stack_size
        );

    if (!pcb) return NULL;

    tcb_t * tcb = pcb->process_threads->tcb;
    if (!tcb) return NULL;
    scheduler_add_thread(tcb);
    tcb->state = THREAD_READY;
    return pcb;
}

/**
 *
 * creates the initial process for the scheduler
 * use for the first process and spawn every other separately
 *
 * @param is_priviledged is the process kernel or userspace
 * @param priority initial priority of the process
 * @param code_location location of copy to execute in mem TODO: expand / replace w/ disk access
 * @param code_size size of code in bytes
 * @return NULL on failure (if any allocation fails all is freed and the thread is never dispatched)
 */
pcb_t * create_initial_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    uint64_t code_location,
    const void * code_src,
    uint64_t code_size,
    uint64_t stack_size
) {
    pcb_t * pcb = init_process(
    parent_pid,
    is_priviledged,
    priority,
    code_location,
    code_src,
    code_size,
    stack_size
    );

    if (!pcb) return NULL;

    tcb_t * tcb = pcb->process_threads->tcb;
    if (!tcb) return NULL;
    scheduler_init(tcb);
    tcb->state = THREAD_READY;
    return pcb;
}

void process_exit(pcb_t * pcb) {
    thread_descriptor_t * thread_ptr = pcb->process_threads;
    while (thread_ptr) {
        scheduler_stop_thread(thread_ptr->tcb);
        thread_ptr = thread_ptr->next_tcb;
    }

}
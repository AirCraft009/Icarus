//
// Created by cocon on 20.10.2026.
//

#include "kernel/Processes/Process.h"

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Processes/Scheduling/scheduler.h"
#include "kernel/util/mem_utils.h"

static PID_t PID_GEN = 1;

pcb_t * init_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    const void * code_location,
    uint64_t code_size,
    thread_memory_allocation_info_t *allocation_info
) {
    //TODO: implement unified mapping layer and replace mappings w/ a better struct

    PID_t pid = PID_GEN++;

    //set default permissions for memory in processes
    thread_memory_permission_info_t permissions = {
        WRITEABLE,
        WRITEABLE | CACHE_DISABLED,
        WRITEABLE,
    };
    thread_memory_supply_block_t * sp = Imalloc(sizeof (thread_memory_supply_block_t));
    if (!sp) return NULL;

    tcb_t * thread = spawn_alloc_thread(pid, is_priviledged, priority, allocation_info, &permissions, sp);

    if (!thread) {
        free(sp);
        return NULL;
    }

    pcb_t * pcb = Imalloc(sizeof(pcb_t));
    if (!pcb) {
        free(sp);
        free(thread);
        return NULL;
    }
    pcb->init_priority = priority;
    pcb->pid = pid;
    pcb->parent_pid = parent_pid;
    thread_descriptor_t * thread_ptr = Imalloc(sizeof(thread_descriptor_t));
    if (!thread_ptr) {
        free(sp);
        free(thread);
        free(pcb);
        return NULL;
    }
    thread_ptr->tcb = thread;
    thread_ptr->next_tcb = NULL;
    pcb->process_threads = thread_ptr;


    // dealloc all if one failed
    if (
        (uint64_t) sp->code_memory == FRAME_ALLOC_FAILED ||
        (uint64_t) sp->mapping_memory == FRAME_ALLOC_FAILED ||
        (uint64_t) sp->stack_memory == FRAME_ALLOC_FAILED
        )
    {
        if ((uint64_t) sp->code_memory != FRAME_ALLOC_FAILED) idealloc_frames(sp->code_memory);
        if ((uint64_t) sp->mapping_memory != FRAME_ALLOC_FAILED) idealloc_frames(sp->mapping_memory);
        if ((uint64_t) sp->stack_memory != FRAME_ALLOC_FAILED) idealloc_frames(sp->stack_memory);
        free(sp);
        return NULL;
    }

    // struct page is overridden here and no longer contains freeing info
    Imemcpy((void *)sp->code_memory,(void *) code_location, code_size);
    //__asm__ volatile ("cli; hlt"); // Completely hangs the computer
    return pcb;
}

/**
 *
 * creates a process (scheduled collection of threads) with a single thread
 * side effect: disables maskable interrupts
 *
 * @param is_priviledged is the process kernel or userspace
 * @param priority initial priority of the process
 * @param code_location location of copy to execute in mem TODO: expand / replace w/ disk access
 * @param code_size size of code in bytes
 * @return NULL on failure (if any allocation fails all is freed and the thread is never dispatched)
 */
pcb_t * create_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    const void * code_location,
    uint64_t code_size,
    thread_memory_allocation_info_t *allocation_info
) {
    pcb_t * pcb = init_process(
        parent_pid,
        is_priviledged,
        priority,
        code_location,
        code_size,
        allocation_info
        );

    if (!pcb) return NULL;

    tcb_t * tcb = pcb->process_threads->tcb;
    if (!tcb) return NULL;
    tcb->state = THREAD_READY;
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
 * @param code_location location of copy to execute in mem TODO: expand / replace w/ disk access
 * @param code_size size of code in bytes
 * @return NULL on failure (if any allocation fails all is freed and the thread is never dispatched)
 */
pcb_t * create_initial_process
(
    PID_t parent_pid,
    bool is_priviledged,
    uint16_t priority,
    const void * code_location,
    uint64_t code_size,
    thread_memory_allocation_info_t *allocation_info
) {
    pcb_t * pcb = init_process(
    parent_pid,
    is_priviledged,
    priority,
    code_location,
    code_size,
    allocation_info
    );
    //__asm__ volatile ("cli; hlt"); // Completely hangs the computer

    if (!pcb) return NULL;

    tcb_t * tcb = pcb->process_threads->tcb;
    if (!tcb) return NULL;
    scheduler_init(tcb);
    tcb->state = THREAD_READY;
    return pcb;
}
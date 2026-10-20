//
// Created by cocon on 20.10.2026.
//

#include "kernel/Processes/Process.h"

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Processes/Scheduling/scheduler.h"
#include "kernel/util/mem_utils.h"

static PID_t PID_GEN = 0;

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
    //TODO: implement unified mapping layer and replace mappings w/ a better struct

    PID_t pid = PID_GEN++;

    //set default permissions for memory in processes
    thread_memory_permission_info_t permissions = {
        WRITEABLE,
        WRITEABLE | CACHE_DISABLED,
        WRITEABLE,
    };
    thread_memory_supply_block_t * sp = Imalloc(sizeof (thread_memory_supply_block_t));

    tcb_t * thread = spawn_alloc_thread(pid, is_priviledged, priority, allocation_info, &permissions, sp);

    if (!thread) return NULL;
    pcb_t * pcb = Imalloc(sizeof(pcb_t));
    pcb->init_priority = priority;
    pcb->pid = pid;
    pcb->parent_pid = parent_pid;
    pcb->process_threads[0] = thread;


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
    Imemcpy((void *)KERNEL_PHYS_TO_VIRT(sp->code_memory),(void *) code_location, code_size);
    thread->state = THREAD_READY;
    scheduler_add_thread(thread);
    return pcb;
}
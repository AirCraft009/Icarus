//
// Created by cocon on 13.10.2026.
//

#include "kernel/Processes/thread.h"

#include <stdatomic.h>
#include <string.h>

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/paging.h"
#include <string.h>
// TID 0 is kernel
static TID_t TID_GEN = 1;

/**
 *
 * creates a thread control block.
 * pml4 table needs to be supplied separately
 *
 * @param owner_id the PID of the process owning the thread
 * @param is_priviledged privilege lvl user vs. kernel
 * @param priority priority for scheduling 0 - (UINT16_MAX - 1) (0 is most aggressive)
 * @return address of the thread of NULL
 */
tcb_t * create_thread(PID_t owner_id, bool is_priviledged, uint16_t priority, uint64_t code_start) {
    if (priority == UINT16_MAX)     // the active prio is priority + 1 so this would overflow to 0
        return NULL;

    tcb_t * tcb = (tcb_t *) Imalloc(sizeof(tcb_t));
    memset(tcb, 0, sizeof(tcb_t));
    tcb->tid = TID_GEN ++;
    tcb->pid = owner_id;

    // set the cpu registers
    tcb->registers.psp.rflags = 0x202;              // default value bit 1 reserved always(1); bit 9 allow interrupts
    tcb->registers.psp.rip = code_start;
    tcb->code_start = code_start;
    tcb->registers.psp.rsp = THREAD_STACK_VIRT_ADDR;
    tcb->registers.psp.cs = is_priviledged? 0x08 : 0x23;
    tcb->registers.psp.ss = is_priviledged? 0x10 : 0x1b;

    atomic_store(&tcb->state, THREAD_NEW);
    tcb->privileged = is_priviledged;
    tcb->priority = priority;
    tcb->active_priority = priority;
    return tcb;
}


/**
 * supplies a thread w/ existing memory
 * it appends to memory already owned by the thread / maps it contiguously
 * doesn't change the state
 *
 * @param thread the tcb of the thread in question
 * @param permissions permission for mapped pages (user access is defined by the type of thread)
 * @return 0 on success -1 if paging failed
 */
int thread_supply_mem_region(tcb_t *thread, virt_addr_t virt_addr, ADDR_SUB_SPACE sub_space, struct_page_t * info_page, uint64_t permissions) {
    for (uint64_t i = 0; i < info_page->page_count; i++) {
        if (page_in(
            (void *) KERNEL_PHYS_TO_VIRT(thread->page_table),
            (void *) SUB_SPACE_TO_ADDR(sub_space, virt_addr) + info_page->page_size * i,
            (void *) info_page->page_addr[i],
            info_page->page_size,
            permissions ^ ((-!thread->privileged ^ permissions) & (1 << PT_USER_ACCESS))
        ) != 0)
            return -1;
        thread->mem_info.fields[sub_space] += info_page->page_size;
        thread->mem_info.total_size += info_page->page_size;
    }
    return 0;
}

/**
 * supplies a thread w/ newly allocated memory
 * it appends to memory already owned by the thread / maps it contiguously
 * doesn't change the state
 *
 * @param thread the tcb of the thread in question
 * @param sub_space what space the memory should be supplied to / mapped into
 * @param permissions permission for mapped pages (user access is defined by the type of thread)
 * @return the addr of the struct page (direct kernel mapping) or FRAME_ALLOC_FAILED
 */
struct_page_t * thread_supply_new_mem_region(
    tcb_t *thread,
    virt_addr_t virt_addr,
    ADDR_SUB_SPACE sub_space,
    uint64_t pages,
    PAGE_TYPE p_size,
    uint64_t permissions
) {
    uint64_t size = PAGE_TYPE_TO_SIZE(p_size);
    struct_page_t * info = ialloc_frames(pages, size);
    if ((uint64_t)(info) == FRAME_ALLOC_FAILED)
        return (void *) FRAME_ALLOC_FAILED;
    if (thread_supply_mem_region(
        thread,
        virt_addr,
        sub_space,
        info,
        permissions
    ) != 0)
        return (void *) FRAME_ALLOC_FAILED;

    return info;
}

struct_page_t * thread_map(tcb_t * thread, virt_addr_t virt_addr, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions) {
    return thread_supply_new_mem_region
    (
        thread,
        virt_addr,
        SUBSPACE_CODE,
        pages,
        p_size,
        permissions
    );
}

struct_page_t * thread_map_stack(tcb_t * thread, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions)
{
    return thread_supply_new_mem_region
(
    thread,
    THREAD_MAPPINGS_VIRT_ADDR,
    SUBSPACE_MAPPING,
    pages,
    p_size,
    permissions
);
}


struct_page_t * thread_map_mapping(tcb_t * thread, uint64_t pages, PAGE_TYPE p_size, uint64_t permissions)
{
    return thread_supply_new_mem_region
(
    thread,
    THREAD_STACK_VIRT_ADDR,
    SUBSPACE_STACK,
    pages,
    p_size,
    permissions
);
}

int thread_exit(tcb_t * thread) {
    thread->state = THREAD_TERMINATED;
    struct_page_t * page = {};
    // TODO: clean up memory
    // remove thread from schedule list
    free(thread);
    return 0;
}

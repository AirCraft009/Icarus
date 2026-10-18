//
// Created by cocon on 13.10.2026.
//

#include "kernel/Processes/thread.h"

#include <stdatomic.h>

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/paging.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"
// TID 0 is kernel
static TID_t TID_GEN = 1;

/**
 *
 * creates a thread control block.
 * if it needs memory use the thread_supply_mem & thread_supply_new_mem
 * depending on whether it should be alr existing memory or newly allocated
 *
 * @param parent
 * @param is_priviledged
 * @param priority
 * @return
 */
tcb_t * create_thread(PID_t parent, bool is_priviledged, uint16_t priority) {
    if (priority == UINT16_MAX)     // the active prio is priority + 1 so this would overflow to 0
        return NULL;

    tcb_t * tcb = (tcb_t *) Imalloc(sizeof(tcb_t));
    Imemset(tcb, 0, sizeof(tcb_t));
    tcb->tid = TID_GEN ++;
    tcb->parent_pid = parent;

    // set the cpu registers
    tcb->regsiters.psp.rflags = 0x202;              // default value bit 1 reserved always(1); bit 9 allow interrupts
    tcb->regsiters.psp.rip = PROCESS_CODE_VIRT_ADDR;
    tcb->regsiters.psp.rsp = PROCESS_STACK_VIRT_ADDR;
    tcb->regsiters.psp.cs = is_priviledged? 0x08 : 0x23;
    tcb->regsiters.psp.ss = is_priviledged? 0x10 : 0x1b;

    page_map_l4_entry * proc_pml4 = (page_map_l4_entry *) KERNEL_VIRT_TO_PHYS(ialloc_frames(1, DEFAULT_PAGE, "ialloc + create_thread"));      // get physical addr struct page is mapped into k_space
    // current pml4 js has the higher mapped kernel, by copying its zeroing lower and higher mapping the kernel
    page_map_l4_entry * cur_pml4 = (page_map_l4_entry *) read_cr3();
    Imemcpy( (void *) KERNEL_PHYS_TO_VIRT(proc_pml4), (void *) KERNEL_PHYS_TO_VIRT(cur_pml4), sizeof(page_map_l4_entry) * 512);
    tcb->page_table = proc_pml4;
    //test_walk_table((void *)KERNEL_PHYS_TO_VIRT(tcb->page_table), (void *)KERNEL_PHYS_TO_VIRT(2 * HUGE_PS));
    atomic_store(&tcb->state, PROCESS_NEW);
    tcb->privileged = is_priviledged;
    tcb->priority = priority;
    tcb->active_priority = priority + 1;
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
int thread_supply_mem(tcb_t *thread, ADDR_SUB_SPACE sub_space, struct_page_t * info_page, uint64_t permissions) {
    for (uint64_t i = 0; i < info_page->page_count; i++) {
        if (page_in(
            (void *) KERNEL_PHYS_TO_VIRT(thread->page_table),
            (void *) (SUB_SPACE_TO_ADDR(sub_space) + thread->memory_limit),
            (void *) info_page->page_addr[i],
            info_page->page_size,
            permissions | (!thread->privileged << USER_ACCESS_SHIFT)
        ) != 0)
            return -1;
        thread->memory_limit += info_page->page_size;
    }
    Iprintf("table: (%x)\n", ((page_map_l4_entry *)KERNEL_PHYS_TO_VIRT(thread->page_table))[511].present);
    test_walk_table(((page_map_l4_entry *)KERNEL_PHYS_TO_VIRT(thread->page_table)),PROCESS_CODE_VIRT_ADDR);
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
struct_page_t * thread_supply_new_mem(tcb_t *thread, ADDR_SUB_SPACE sub_space,  uint64_t pages, PAGE_TYPE p_size, uint64_t permissions) {
    uint64_t size = PAGE_TYPE_TO_SIZE(p_size);
    struct_page_t * info = ialloc_frames(pages, size, "ialloc + thread_mem");
    if ((uint64_t)(info) == FRAME_ALLOC_FAILED)
        return (void *) FRAME_ALLOC_FAILED;
    if (thread_supply_mem(
        thread,
        sub_space,
        info,
        permissions
    ) != 0)
        return (void *) FRAME_ALLOC_FAILED;

    return info;
}

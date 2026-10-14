//
// Created by cocon on 13.10.2026.
//

#include "kernel/Processes/processes.h"

#include "kernel_helper.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/paging.h"
#include "kernel/util/mem_utils.h"
static PID_t PID_GEN = 0;

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

    tcb_t * pcb = (tcb_t *) Imalloc(sizeof(tcb_t));
    Imemset(pcb, 0, sizeof(tcb_t));
    pcb->pid = PID_GEN ++;
    pcb->parent_pid = parent;

    page_map_l4_entry * proc_pml4 = Imalloc(sizeof(page_map_l4_entry) * 512);
    // current pml4 js has the higher mapped kernel, by copying its zeroing lower and higher mapping the kernel
    page_map_l4_entry * cur_pml4 = (page_map_l4_entry *) read_cr3();
    Imemccpy(proc_pml4, cur_pml4, sizeof(page_map_l4_entry) * 512);
    pcb->page_table = proc_pml4;
    pcb->state = PROCESS_NEW;
    pcb->privileged = is_priviledged;
    pcb->priority = priority;
    pcb->active_priority = priority + 1;
    return pcb;
}


/**
 * supplies a thread w/ memory
 * it appends to memory already owned by the thread / maps it contiguously
 * doesn't change the state
 *
 * @param thread the tcb of the thread in question
 * @param permissions permission for mapped pages (user-accessible etc...)
 * @return 0 on success -1 if paging failed
 */
int thread_supply_mem(tcb_t thread, struct_page_t * info_page, uint64_t permissions) {
    for (int i = 0; i < info_page->page_count; i++) {
        if (page_in(
            thread.page_table,
            (void *) (PROCESS_VIRT_ADDR + thread.memory_limit),
            (void *) info_page->page_addr[i],
            info_page->page_size,
            permissions
        ) != 0)
            return -1;
        thread.memory_limit += info_page->page_size;
    }

    return 0;
}


/* create.c : create a process
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <pcb.h>

/* Declarations */

pcb_t* getFreePCBSlot(void); /* helper function declaration */

#define INTERRUPT_FLAG 0x00003200; /* the eflags to handle interrupt */

////////////////////////////////////////////////////////////
int create(void (*func)(void), int stack) {
    pcb_t* pcb; /* The pcb pointer to modify */
    unsigned long addr, addr_end; /* The address of dataStart[0], and end of size */
    cf_t* cf;

    // get free pcb from table
    pcb = getFreePCBSlot();
    if (pcb == NULL) {
        kprintf("There is no free PCB in the table.\n");
        return FAILED;
    }

    // allocate stack size
    addr = (unsigned long) kmalloc(stack);
    addr_end = addr + stack;
    if (addr == NULL) {
        kprintf("There is no space in the memory to allocated anymore. \n");
        return FAILED;
    }

    // set cf values
    cf = (cf_t *) (addr_end - sizeof(cf_t) - 4);
    cf->esp = cf->eflags;
    cf->ebp = cf->esp;
    cf->iret_eip = (unsigned long) func;
    cf->iret_cs = getCS();
    cf->eflags = INTERRUPT_FLAG;

    // set return address to sysstop
    cf->free_slots[0] = (unsigned long) sysstop;

    // set pcb values
    memset(pcb, 0, sizeof(pcb)); // clear any values a stopped process could have
    pcb->pid = pcb_id++;
    pcb->esp = (unsigned long) cf;
    pcb->addr_start = (unsigned long) addr;
    pcb->cpuTime = 0;
    pcb->priority = 3;
    pcb->semNo = -1;
    pcb->sig_mask = 0xFFFFFFFF;
    pcb->sigTable[SIG_TABLE_SIZE-1] = (sighandler_t) sysstop; // set last signal the signal to kill
    PRINT("PCB Stats, pid: %d, pstate: %d, pesp: %d, addr_start:%d, max_addr: %d\n",
        pcb->pid, pcb->state, pcb->esp, pcb->addr_start, addr_end);

    // add to ready queue
    ready(pcb);

    return pcb->pid;
}

/**
 * @brief Returns the first index of a free pcb in the table.
 * 
 * @returns The first index of a stopped pcb in the pcbTable stack.
 */
pcb_t* getFreePCBSlot(void) {
    int i;
    for (i = 0; i < PCB_TABLE_SIZE; i++) {
        if (pcbTable[i].state == STATE_STOPPED) {
            // PRINT("The current index of table to get: %d\n", i);
            return &pcbTable[i];
        }
    }
    return NULL;
}
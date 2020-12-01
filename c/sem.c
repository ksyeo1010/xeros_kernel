/* sem.c : messaging system 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>

#define NUM_SEMAPHORES 2    /* Number of semaphores in our system. */

/* semaphore typedef */
typedef struct semaphore {
    int value;
    pcb_t *head[PQ_SIZE];
    pcb_t *tail[PQ_SIZE];
} semaphore_t;

/* The semaphores in our system. */
static semaphore_t semaphores[NUM_SEMAPHORES];

/* semaphore helper functions declarations */
void blockProcess(semaphore_t *sp, pcb_t *pcb);
pcb_t *unblockProcess(semaphore_t *sp);

////////////////////////////////////////////////////////////
void seminit() {
    semaphore_t *sp;
    int i, j;
    for (i = 0; i < NUM_SEMAPHORES; i++) {
        sp = &semaphores[i];
        sp->value = 0;
        for (j = 0; j < PQ_SIZE; j++) {
            sp->head[j] = NULL;
            sp->tail[j] = NULL;
        }
    }
}

////////////////////////////////////////////////////////////
int P_kern(int semNo, pcb_t *pcb) {
    semaphore_t *sp;

    // check semNo
    if (semNo < 0 || semNo > NUM_SEMAPHORES) {
        return FAILED;
    }

    sp = &semaphores[semNo];

    PRINT("PCB pid: %d, Sem #: %d, value: %d.\n", pcb->pid, semNo, sp->value);

    if (sp->value == 0) {
        blockProcess(sp, pcb);
        return BLOCKED;
    } else if (sp->value > 0) {
        sp->value--;
    } else {
        FAIL("Semaphore values should not be negative. Number: %d.\n", semNo);
        return FAILED;
    }

    return SUCCEED;
}

////////////////////////////////////////////////////////////
int V_kern(int semNo) {
    semaphore_t *sp;
    pcb_t *pcb;

    // check semNo
    if (semNo < 0 || semNo >= NUM_SEMAPHORES) {
        return FAILED;
    }

    sp = &semaphores[semNo];
    pcb = unblockProcess(sp);

    PRINT("PCB pid: %d, Sem #: %d, value: %d.\n", (pcb==NULL)?0:pcb->pid, semNo, sp->value);

    if (pcb == NULL) {
        sp->value++;
    } else {
        ready(pcb);
    } 

    return SUCCEED;
}

////////////////////////////////////////////////////////////
void removeFromBlockQueue(pcb_t *pcb) {
    semaphore_t *sp;
    pcb_t *p;
    int i;

    for (i = 0; i < NUM_SEMAPHORES; i++) {
        sp = &semaphores[i];
        p = sp->head[pcb->priority];

        // if its first in queue
        if (p->pid == pcb->pid) {
            sp->head[pcb->priority] = sp->head[pcb->priority]->next;
            return;
        }

        while (p->next != NULL) {
            if (p->next->pid == pcb->pid) {
                p->next = pcb->next;
                // if tail is the one to kill
                if (sp->tail[pcb->priority]->pid == pcb->pid) {
                    sp->tail[pcb->priority] = p;
                }
                return;
            }
            p = p->next;
        }
    }
}

/**
 * @brief Helper function to add a process to the blocked list given
 *        the semaphore.
 * 
 * @param {sp} The semaphore to block the process on.
 * @param {pcb} The process to block.
 */
void blockProcess(semaphore_t *sp, pcb_t *pcb) {
    int priority;

    priority = pcb->priority;
    pcb->next = NULL;
    pcb->state = STATE_BLOCKED;

    PRINT("PCB stats, pid: %d, next: %d\n", pcb->pid, pcb->next);

    if (sp->head[priority] == NULL) {
        sp->head[priority] = pcb;
        sp->tail[priority] = sp->head[priority];
    } else {
        sp->tail[priority]->next = pcb;
        sp->tail[priority] = sp->tail[priority]->next;
    }
}

/**
 * @brief Helper function to unblock the process from the blocked list.
 *        Unblocks highest priority process.
 * 
 * @param {sp} The semaphore to unblock the process on.
 * @returns The process that was unblocked, NULL if no process was found.
 */
////////////////////////////////////////////////////////////
pcb_t *unblockProcess(semaphore_t *sp) {
    pcb_t *pcb;
    int i;

    // start with highest priority
    for (i = PQ_SIZE - 1; i >= 0; i--) {
        pcb = sp->head[i];

        if (pcb != NULL) {
            PRINT("PCB stats, pid: %d, next: %d\n", pcb->pid, pcb->next);

            // check if pcb was in state blocked
            if (pcb->state != STATE_BLOCKED) {
                FAIL("PCB was not in blocked state. pid: %d\n", pcb->pid);
            }
            // move head ptrs
            sp->head[i] = sp->head[i]->next;
            return pcb;
        }
    }
    return NULL;
}
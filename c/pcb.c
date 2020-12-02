/* pcb.c */

#include <xeroskernel.h>
#include <xeroslib.h>

/* priority queues declarations */
static pcb_t *head[PQ_SIZE];
static pcb_t *tail[PQ_SIZE];

/* Pointer to the idle process */
static pcb_t *idle_pcb;

/* init pcb_id with 0 */
pid_t pcb_id = 0;

////////////////////////////////////////////////////////////
void initReadyQueue(void) {
    PRINT("Starting ready queue.\n");

    for (int i=0; i < PQ_SIZE; i++) {
        head[i] = NULL;
        tail[i] = NULL;
    }
}

////////////////////////////////////////////////////////////
void initPcbTable(void) {
    memset(pcbTable, 0, sizeof( pcb_t ) * PCB_TABLE_SIZE);
    idle_pcb = &pcbTable[0]; // set idle process to 0
}

////////////////////////////////////////////////////////////
pcb_t *next() {
    pcb_t *pcb;
    int i;

    // get the highest priority process first
    for (i = PQ_SIZE - 1; i >= 0; i--) {
        pcb = head[i];
        if (pcb != NULL) {
            break;
        }
    }

    // just return idle process if there is no more process to run.
    if (pcb == NULL) {
        idle_pcb->state = STATE_RUNNING;
        return idle_pcb;
    }

    // check if it was not ready state something must have occurred.
    if (pcb->state != STATE_READY) {
        FAIL("Process was not in ready state. PID: %d.\n", pcb->pid);
    }

    head[pcb->priority] = head[pcb->priority]->next;
    pcb->state = STATE_RUNNING;
    pcb->next = NULL;
    pcb->prev = NULL;

    return pcb;
}

////////////////////////////////////////////////////////////
void ready(pcb_t *pcb) {
    int priority;

    // ignore idle process
    if (pcb->pid == 0) {
        idle_pcb->state = STATE_READY;
        return;
    }

    priority = pcb->priority;
    pcb->next = NULL;
    pcb->prev = NULL;
    pcb->state = STATE_READY;

    if (head[priority] == NULL) {
        head[priority] = pcb;
        tail[priority] = head[priority];
    } else {
        tail[priority]->next = pcb;
        pcb->prev = tail[priority];
        tail[priority] = tail[priority]->next;
    }
}

////////////////////////////////////////////////////////////
void cleanup(pcb_t *pcb) {
    pcb->state = STATE_STOPPED;
    pcb->next = NULL;
    pcb->prev = NULL;
    PRINT("Freeing pid: %d, addr: %d.\n", pcb->pid, pcb->addr_start);
    kfree((void*) pcb->addr_start);
}

////////////////////////////////////////////////////////////
pcb_t *getPCB(pid_t pid) {
    for (int i = 0; i < PCB_TABLE_SIZE; i++) {
        if (pcbTable[i].pid == pid) {
            return &pcbTable[i];
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////
void setPriority(pid_t pid, int priority) {
    pcb_t *pcb = getPCB(pid);
    pcb->priority = priority;
}

////////////////////////////////////////////////////////////
void removeFromReadyQueue(pcb_t *pcb) {
    // adjust pointers

    if (pcb->prev == NULL) {
        // if prev is NULL, then it is in front
        head[pcb->priority] = pcb->next;
    } else if (pcb->next == NULL) {
        // if next is NULL, then it is last
        pcb->prev->next = NULL;
        tail[pcb->priority] = pcb->prev;
    } else {
        pcb->prev->next = pcb->next;
    }
    // pcb_t *p = head[pcb->priority];

    // // if its first in queue
    // if (p->pid == pcb->pid) {
    //     head[pcb->priority] = head[pcb->priority]->next;
    //     return;
    // }

    // while (p->next != NULL) {
    //     if (p->next->pid == pcb->pid) {
    //         p->next = pcb->next;
    //         // if tail is the one to kill
    //         if (tail[pcb->priority]->pid == pcb->pid) {
    //             tail[pcb->priority] = p;
    //         }
    //         return;
    //     }
    //     p = p->next;
    // }
}

////////////////////////////////////////////////////////////
int removeFromQueue(pcb_t *pcb) {
    int state = pcb->state;

    PRINT("PCB stats, pid: %d, next: %d, addr: %d\n", pcb->pid, pcb->next, pcb);

    switch (state) {
        case STATE_SLEEP:
            removeFromSleepQueue(pcb);
            break;
        case STATE_READY:
            removeFromReadyQueue(pcb);
            break;
        case STATE_BLOCKED:
            removeFromBlockQueue(pcb);
            break;
        default:
            PRINT("Should be either READY, BLOCKED, SLEEP state.\n");
            return SYSERR;
    }

    return SYSOK;
}
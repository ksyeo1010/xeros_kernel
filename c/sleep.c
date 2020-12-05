/* sleep.c : sleep device 
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <pcb.h>

static pcb_t *dl = NULL;

////////////////////////////////////////////////////////////
void sleep(pcb_t *pcb, unsigned int milliseconds) {
    unsigned int tick;
    pcb_t *p;

    // set tick
    tick = (milliseconds/TICK_SPLIT) + ((milliseconds%TICK_SPLIT)?1:0);
    pcb->tick = tick;
    pcb->state = STATE_SLEEP;
    pcb->next = NULL;
    pcb->prev = NULL;

    PRINT("Sleeping process, pid: %d, tick: %d.\n", pcb->pid, pcb->tick);

    // initial case
    if (dl == NULL) {
        dl = pcb;
        return;
    }

    // find where to put pcb in
    p = dl;
    while (p != NULL) {
        PRINT("P stats, pid: %d, tick: %d.\n", p->pid, p->tick);
        // if pcb tick is smaller than current pos tick
        // add in front
        if (pcb->tick < p->tick) {
            p->tick = p->tick - pcb->tick;
            pcb->next = p;
            if (p->prev == NULL) {
                dl = pcb;
                p->prev = pcb;
            } else {
                pcb->prev = p->prev;
            }
            return;
        }

        // if its bigger substract the tick
        pcb->tick = pcb->tick - p->tick;

        // if next is null, pcb is last
        if (p->next == NULL) {
            p->next = pcb;
            pcb->prev = p;
            return;
        }

        p = p->next;
    }
}

////////////////////////////////////////////////////////////
void tick() {
    pcb_t *next;

    // if we have nothing just return;
    if (dl == NULL) {
        return;
    }

    // decrease count of tick
    dl->tick--;
    // for every tick that has reached 0 add to ready queue
    while ((dl != NULL)) {
        if (dl->tick < 0) {
            FAIL("dl tick was less than 0, pid: %d.\n", dl->pid);
        }

        if (dl->tick != 0) {
            return;
        }

        PRINT("PID stats: pid: %d, tick: %d.\n", dl->pid, dl->tick);
        dl->rc = dl->tick;
        next = dl->next;
        ready(dl);
        dl = next;
        dl->prev = NULL;
    }
}

///////////////////////////////////////////////////////////
void removeFromSleepQueue(pcb_t *pcb) {
    pcb_t *p;
    int tick = pcb->tick;

    // get sum of all ticks
    for (p = pcb->prev; p != NULL; p = p->prev) {
        pcb->tick += p->tick;
    }

    if (pcb->prev == NULL && pcb->next == NULL) {
        dl = NULL;
        return;
    }

    // adjust pointers
    if (pcb->prev == NULL) {
        // if prev is NULL, then it is in front
        dl = pcb->next;
        dl->tick += tick;
    } else if (pcb->next == NULL) {
        // if next is NULL, then it is last
        pcb->prev->next = NULL;
    } else {
        pcb->next->tick += tick;
        pcb->prev->next = pcb->next;
    }
}
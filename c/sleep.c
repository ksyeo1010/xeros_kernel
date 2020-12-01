/* sleep.c : sleep device 
 */

#include <xeroskernel.h>
#include <xeroslib.h>

static pcb_t *dl = NULL;

////////////////////////////////////////////////////////////
void sleep(pcb_t *pcb, unsigned int milliseconds) {
    unsigned int tick;
    pcb_t *p;
    pcb_t *prev;

    // set tick
    tick = (milliseconds/TICK_SPLIT) + ((milliseconds%TICK_SPLIT)?1:0);
    pcb->tick = tick;
    pcb->state = STATE_SLEEP;
    pcb->next = NULL;

    PRINT("Sleeping process, pid: %d, tick: %d.\n", pcb->pid, pcb->tick);

    // initial case
    if (dl == NULL) {
        dl = pcb;
        return;
    }

    // find where to put pcb in
    p = dl;
    prev = NULL;

    while (p != NULL) {
        PRINT("P stats, pid: %d, tick: %d.\n", p->pid, p->tick);
        // if pcb tick is smaller than current pos tick
        // add in front
        if (pcb->tick < p->tick) {
            p->tick = p->tick - pcb->tick;
            pcb->next = p;
            if (prev == NULL) {
                dl = pcb;
            } else {
                prev->next = pcb;
                
            }
            return;
        }

        // if its bigger substract the tick
        pcb->tick = pcb->tick - p->tick;

        // if next is null, pcb is last
        if (p->next == NULL) {
            p->next = pcb;
            return;
        }

        prev = p;
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
    }
}

///////////////////////////////////////////////////////////
void removeFromSleepQueue(pcb_t *pcb) {
    pcb_t *p = dl;

    // if its first in queue
    if (p->pid == pcb->pid) {
        dl = dl->next;
        if (dl != NULL) {
            dl->tick += p->tick;
        }
        return;
    }

    while (p->next != NULL) {
        if (p->next->pid == pcb->pid) {
            p->next = pcb->next;
            // increment tick if needed
            if (p->next != NULL) {
                p->next->tick += pcb->tick;
            }
            return;
        }
        p = p->next;
    }
}
/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <i386.h>
#include <pcb.h>

extern char * maxaddr;

////////////////////////////////////////////////////////////
sighandler_t set_signal(pcb_t *pcb, int signum, sighandler_t handler) {
    // check if signum within limits
    if (signum < 0 || signum >= (SIG_TABLE_SIZE - 1)) {
        return FAILED;
    }

    // check if addr is valid
    if (((unsigned long) handler) >= HOLESTART && ((unsigned long) handler <= HOLEEND)) 
        return FAILED;

    //Check if address of the data structure is beyone the end of main memory
    if ((((char * ) handler) + sizeof(processStatuses)) > maxaddr)  
        return FAILED;

    PRINT("pid: %d, signum: %d, handler: %d.\n", pcb->pid, signum, (unsigned long) handler);

    sighandler_t prev = pcb->sigTable[signum];

    // set the handler
    pcb->sigTable[signum] = handler;

    return prev;
}

////////////////////////////////////////////////////////////
void sigtramp(void (*handler)(void *), void *cntxPtr) {
    PRINT("handler: %d, ctxptr: %d\n", (unsigned long) handler, (unsigned long) cntxPtr);
    handler(cntxPtr);
    syssigreturn(cntxPtr);
}

////////////////////////////////////////////////////////////
int signal(pid_t pid, int signum) {
    pcb_t *target = getPCB(pid);

    PRINT("Pid: %d, signum: %d\n", pid, signum);

    // if target does not exists
    if (target == NULL) return SIG_NOT_EXISTS;
    // if sig is no between 0~31
    if (signum < 0 || signum > (SIG_TABLE_SIZE - 1)) return SIG_FAIL;
    // if signal does not exists
    if (target->sigTable[signum] == NULL) return SIG_FAIL;

    if ((target->sig_mask & (0x1 << signum)) == 0) {
        target->sig_ignored = target->sig_ignored | (0x1 << signum);
        PRINT("Signal ignored pid: %d, signum: %d\n", target->pid, signum);
    } else {
        // remove if it is anything that is not running/ready and add back to ready
        if ((target->state != STATE_READY) && (target->state != STATE_RUNNING)) {
            removeFromQueue(target);
            if (target->state == STATE_SLEEP) {
                target->rc = target->tick * TICK_SPLIT;
            } else if (target->state != DEV_BLOCK) {
                target->rc = SIG_EINTR;
            }
            ready(target); 
        }

        // set signal context frame
        sigframe_t *sig_frame = (sigframe_t *) (target->esp - sizeof(sigframe_t));

        sig_frame->ctx.iret_eip = (unsigned long) &sigtramp;
        sig_frame->ctx.iret_cs = getCS();
        sig_frame->ctx.eflags = 0x3200;
        sig_frame->ctx.esp = (unsigned long) &(sig_frame->returnAddr);

        sig_frame->handlerAddr = (unsigned long) target->sigTable[signum];
        sig_frame->contextFramePtr = target->esp;
        sig_frame->prevSigNum = signum;
        sig_frame->returnValue = target->rc;

        target->esp = (unsigned long) sig_frame;
        target->sig_mask = target->sig_mask << signum;

        PRINT("sig frame stats, addr: %d, ctx frame ptrs: %d, signum: %d, sig mask: 0x%x, handler: %d, ret addr: %d\n", 
            target->esp, sig_frame->contextFramePtr, signum, target->sig_mask, sig_frame->handlerAddr, sig_frame->ctx.esp);

        // toggle down ignored if this was ignored signal
        if (target->sig_ignored & (0x1 << signum)) {
            target->sig_ignored = target->sig_ignored ^ (0x1 << signum);
        }
    }

    return SIG_OK;
}

////////////////////////////////////////////////////////////
int addToWaitQueue(pcb_t *pcb, pid_t pid) {
    pcb_t *target = getPCB(pid);

    if (pid == 0) return SIG_FAIL;

    if (target == NULL) return SIG_FAIL;

    PRINT("PCB pid: %d, Target pid: %d, target state: %d\n", pcb->pid, target->pid, target->state);

    if (target->state == STATE_STOPPED) return SIG_FAIL;

    pcb->next = NULL;
    pcb->prev = NULL;
    pcb->state = STATE_WAITING;
    pcb->waiting_on = target;

    if (target->wait_head == NULL) {
        target->wait_head = pcb;
        target->wait_tail = target->wait_head;
    } else {
        target->wait_tail->next = pcb;
        pcb->prev = target->wait_tail;
        target->wait_tail = target->wait_tail;
    }

    return SIG_OK;
}

////////////////////////////////////////////////////////////
void removeFromWaitQueue(pcb_t *pcb) {
    if (pcb->state != STATE_WAITING || pcb->waiting_on == NULL) {
        FAIL("PCB should be in waiting and waiting cpu should not be null.\n");
    }

    pcb_t *target = pcb->waiting_on;
    if (pcb->prev == NULL) {
        // if prev is NULL, then it is in front
        target->wait_head = pcb->next;
    } else if (pcb->next == NULL) {
        // if next is NULL, then it is last
        pcb->prev->next = NULL;
        target->wait_tail = pcb->prev;
    } else {
        pcb->prev->next = pcb->next;
    }
}

////////////////////////////////////////////////////////////
void restoreToReadyQueue(pcb_t *pcb) {
    pcb_t *p;

    for (p = pcb->wait_head; p != NULL; p = p->next) {
        ready(p);
    }
}

int getsig(int bits) {
    int i;
    for (i = SIG_TABLE_SIZE - 1; i >=0; i++) {
        if (bits & 0x80000000) {
            break;
        }
        bits = bits << 1;
    }
    return i;
}
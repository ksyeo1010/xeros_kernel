/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <stdarg.h>
#include <i386.h>
#include <pcb.h>
#include <di_calls.h>

/* declarations for getting CPU times */
extern char * maxaddr;
int getCPUtimes(pcb_t *p, processStatuses *ps);

////////////////////////////////////////////////////////////
void dispInit() {
    initPcbTable();             /* initialize pcb table */
    initReadyQueue();           /* initialize ready queue */
}

////////////////////////////////////////////////////////////
void dispatch() {
    pcb_t *pcb;                     /* the pcb pointer */
    int request;                    /* request from ctsw */
    va_list ap;                     /* va_list struct */
    void *func;                     /* function pointer to pass when create */
    int stack;                      /* size to pass when create */
    char *str;                      /* store string to print to screen */
    pid_t pid;                      /* the pid */
    int signum;                     /* signal number */
    int priority;                   /* the priority to set */
    int fd;                         /* file description argument */
    void *buffer;                   /* buffer argument */
    int bufferlen;                  /* the length of the buffer argument */
    unsigned long command;          /* the command argument */

    // the next process. Should be the idle process.
    pcb = next();

    for (;;) {
        request = contextswitch(pcb);

        switch(request) {
            case SYS_CREATE:
                ap = (va_list) pcb->args;
                func = va_arg(ap, void*);
                stack = va_arg(ap, int);
                pcb->rc = create(func, stack);
                break;
            case SYS_YIELD:
                ready(pcb);
                pcb = next();
                break;
            case SYS_STOP:
                restoreToReadyQueue(pcb);
                cleanup(pcb);
                pcb = next();
                break;
            case SYS_GETPID:
                pcb->rc = pcb->pid;
                break;
            case SYS_PUTS:
                ap = (va_list) pcb->args;
                str = va_arg(ap, char*);
                kprintf("%s", str);
                break;
            case SYS_KILL:
                ap = (va_list) pcb->args;
                pid = va_arg(ap, pid_t);
                pcb->rc = signal(pid, va_arg(ap, int));
                break;
            case SYS_SETPRIO:
                ap = (va_list) pcb->args;
                priority = va_arg(ap, int);
                if (priority > 3 || priority < -1) {
                    pcb->rc = SYSERR;
                } else {
                    pcb->rc = pcb->priority;
                    if (priority != -1) {
                        pcb->priority = priority;
                    }
                }
                break;
            case SYS_P:
                ap = (va_list) pcb->args;
                pcb->rc = P_kern(va_arg(ap, int), pcb);
                if (pcb->rc == BLOCKED) {
                    pcb->rc = SUCCEED;
                    pcb = next();
                }
                break;
            case SYS_V:
                ap = (va_list) pcb->args;
                pcb->rc = V_kern(va_arg(ap, int));
                break;
            case TIMER_INT:
                tick();
                pcb->cpuTime++;
                ready(pcb);
                pcb = next();
                end_of_intr();
                break;
            case SYS_SLEEP:
                ap = (va_list) pcb->args;
                sleep(pcb, va_arg(ap, unsigned int));
                pcb = next();
                break;
            case SYS_CPUTIMES:
                ap = (va_list) pcb->args;
                pcb->rc = getCPUtimes(pcb, va_arg(ap, processStatuses *));
                break;
            case SYS_SIGNAL:
                ap = (va_list) pcb->args;
                signum = va_arg(ap, int);
                pcb->rc = (int) set_signal(pcb, signum, va_arg(ap, sighandler_t));
                break;
            case SYS_SIGRETURN:
                ap = (va_list) pcb->args;
                pcb->esp = (unsigned long) va_arg(ap, void *);
                // shift back
                pcb->sig_mask = (pcb->sig_mask >> *(int *)(pcb->esp - 2*sizeof(int)));
                // get return value before signal
                pcb->rc = *(int *)(pcb->esp - sizeof(int));
                PRINT("pid %d, sig mask: 0x%x\n", pcb->pid, pcb->sig_mask);
                // signal again if we have ignored signals
                if (pcb->sig_ignored != 0) {
                    signal(pcb->pid, getsig(pcb->sig_ignored));
                }
                break;
            case SYS_WAIT:
                ap = (va_list) pcb->args;
                pcb->rc = addToWaitQueue(pcb, va_arg(ap, pid_t));
                if (pcb->rc != SIG_FAIL) {
                    pcb = next();
                }
                break;
            case SYS_OPEN:
                ap = (va_list) pcb->args;
                pcb->rc = di_open(pcb, va_arg(ap, int));
                break;
            case SYS_CLOSE:
                ap = (va_list) pcb->args;
                pcb->rc = di_close(pcb, va_arg(ap, int));
                break;
            case SYS_WRITE:
                ap = (va_list) pcb->args;
                fd = va_arg(ap, int);
                buffer = va_arg(ap, void*);
                bufferlen = va_arg(ap, int);
                pcb->rc = di_write(pcb, fd, buffer, bufferlen);
                break;
            case SYS_READ:
                ap = (va_list) pcb->args;
                fd = va_arg(ap, int);
                buffer = va_arg(ap, void*);
                bufferlen = va_arg(ap, int);
                pcb->rc = di_read(pcb, fd, buffer, bufferlen);
                if (pcb->state == DEV_BLOCK) {
                    pcb = next();
                }
                break;
            case SYS_IOCTL:
                ap = (va_list) pcb->args;
                fd = va_arg(ap, int);
                command = va_arg(ap, unsigned long);
                pcb->rc = di_ioctl(pcb, fd, command, va_arg(ap, void *));
                break;
            case KBD_INT:
                kbdint_handler();
                end_of_intr();
                break;
        }
    }
}

////////////////////////////////////////////////////////////
void idleproc(void) {
    for (;;) {
        __asm __volatile("hlt":::);
    }
}

////////////////////////////////////////////////////////////
int getCPUtimes(pcb_t *p, processStatuses *ps) {
  
  int i, currentSlot;
  currentSlot = -1;

  // Check if address is in the hole
  if (((unsigned long) ps) >= HOLESTART && ((unsigned long) ps <= HOLEEND)) 
    return -1;

  //Check if address of the data structure is beyone the end of main memory
  if ((((char * ) ps) + sizeof(processStatuses)) > maxaddr)  
    return -2;

  // There are probably other address checks that can be done, but this is OK for now


  for (i=0; i < PCB_TABLE_SIZE; i++) {
    if (pcbTable[i].state != STATE_STOPPED) {
      // fill in the table entry
      currentSlot++;
      ps->pid[currentSlot] = pcbTable[i].pid;
      ps->status[currentSlot] = p->pid == pcbTable[i].pid ? STATE_RUNNING: pcbTable[i].state;
      ps->cpuTime[currentSlot] = pcbTable[i].cpuTime * TICK_SPLIT;
    }
  }

  return currentSlot;
}
/* ctsw.c : context switcher
 */

#include <i386.h>
#include <xeroskernel.h>

void _ISREntryPoint(void);      /* code to run when interrupt happens */
void _TimerEntryPoint(void);
static void *k_stack;           /* the k_stack pointer */
static unsigned long *esp;      /* the esp pointer */
static int rc;
static int interrupt;
static int calltype;
static unsigned long *args;     /* the args pointer */

////////////////////////////////////////////////////////////
void contextinit() {
    set_evec(SYS_CALL, (unsigned long) _ISREntryPoint);
    set_evec(IRQBASE, (unsigned long) _TimerEntryPoint);

    initPIT(PIT_VALUE);
}

////////////////////////////////////////////////////////////
int contextswitch(pcb_t* pcb) {
    esp = (unsigned long *) pcb->esp;
    rc = pcb->rc;
    __asm __volatile("          \
        pushf                   \n\
        pusha                   \n\
        movl rc, %%eax          \n\
        movl %%esp, k_stack     \n\
        movl esp, %%esp         \n\
        movl %%eax, 28(%%esp)   \n\
        popa                    \n\
        iret                    \n\
    _TimerEntryPoint:           \n\
        cli                     \n\
        pusha                   \n\
        movl $1, %%ecx          \n\
        jmp _CommonJump         \n\
    _ISREntryPoint:             \n\
        cli                     \n\
        pusha                   \n\
        movl $0, %%ecx          \n\
    _CommonJump:                \n\
        movl %%esp, esp         \n\
        movl k_stack, %%esp     \n\
        movl %%eax, rc          \n\
        movl %%ecx, interrupt   \n\
        movl %%edx, args        \n\
        popa                    \n\
        popf                    \n\
        "
        : 
        :
        : "%eax", "%ecx", "%edx"
    );

    // if it is system time out
    if (interrupt == 1) {
        calltype = TIMER_INT;
        pcb->rc = rc;
    } else {
        calltype = rc;
    }

    (void)k_stack; // ignore compiler warning
    pcb->esp = (unsigned long) esp;
    pcb->args = (unsigned long) args;
    return calltype;
}
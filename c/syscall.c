/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>

////////////////////////////////////////////////////////////
int syscall(int call, ...) {
    va_list ap;
    int rc;

    va_start(ap, call);

    __asm __volatile("  \
        movl %1, %%eax  \n\
        movl %2, %%edx  \n\
        int  %3         \n\
        movl %%eax, %0  \n\
        "
        : "=g" (rc)
        : "g" (call), "g" (ap), "i" (SYS_CALL)
        : "eax", "edx"
    );

    va_end(ap);

    return rc;
}

////////////////////////////////////////////////////////////
unsigned int syscreate(void (*func)(void), int stack) {
    return syscall(SYS_CREATE, func, stack);
}

////////////////////////////////////////////////////////////
void sysyield(void) {
    syscall(SYS_YIELD);
}

////////////////////////////////////////////////////////////
void sysstop(void) {
    syscall(SYS_STOP);
}

////////////////////////////////////////////////////////////
pid_t sysgetpid(void) {
    return syscall(SYS_GETPID);
}

////////////////////////////////////////////////////////////
void sysputs(char *str) {
    syscall(SYS_PUTS, str);
}

////////////////////////////////////////////////////////////
int syskill(pid_t pid) {
    return syscall(SYS_KILL, pid);
}

////////////////////////////////////////////////////////////
int syssetprio(int priority) {
    return syscall(SYS_SETPRIO, priority);
}

////////////////////////////////////////////////////////////
int sysP(int semNo) {
    return syscall(SYS_P, semNo);
}

////////////////////////////////////////////////////////////
int sysV(int semNo) {
    return syscall(SYS_V, semNo);
}

////////////////////////////////////////////////////////////
unsigned int syssleep(unsigned int milliseconds) {
    return syscall(SYS_SLEEP, milliseconds);
}

////////////////////////////////////////////////////////////
int sysgetcputimes(processStatuses *ps) {
    return syscall(SYS_CPUTIMES, ps);
}
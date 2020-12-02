/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

void printA(void) {
    sysputs("A\n");
}

void printB(void) {
    sysputs("B\n");
}

void producer(void) {
    int pid;
    char buf[1024];
    int sleep;

    pid = sysgetpid();

    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    syssignal(5, &printA);

    sleep = syssleep(5000);

    sprintf(buf, "Interrupted pid %d at %d.\n", pid, sleep);
    sysputs(buf);

    for (;;) {
        sysyield();
    }
}

void root(void) {

    sysputs("Welcome to Xeros 415 - A not very secure Kernel.\n");

    syscreate(producer, STACKSIZE);

    syssleep(1000);

    syskill(2, 5);

}
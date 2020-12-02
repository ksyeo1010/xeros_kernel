/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

void printA(void) {
    sysputs("A\n");
}

void producer(void) {
    int pid;
    char buf[1024];

    pid = sysgetpid();

    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    syssignal(3, &printA);

    sysputs("sleeping for 2000.\n");
    syssleep(2000);
}

void root(void) {

    sysputs("Welcome to Xeros 415 - A not very secure Kernel.\n");

    syscreate(producer, STACKSIZE);

    syssleep(1000);

    syskill(2, 3);
    sysputs("waiting.\n");
    syswait(2);
    sysputs("done waiting.\n");

}
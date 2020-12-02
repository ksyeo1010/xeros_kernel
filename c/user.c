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
    int fd;
    int len;
    char out[1024];
    char buf[1024] = {'a', 'b', 'c', 'd', 'e'};
    sysputs("Welcome to Xeros 415 - A not very secure Kernel.\n");

    fd = sysopen(0);
    sysputs(buf);
    sysread(fd, buf, 5);
    sysputs(buf);
    sysclose(fd);

    fd = sysopen(1);
    len = sysread(fd, buf, 3);
    sprintf(out, "Length: %d, val: %d\n", len, buf);
    sysputs(out);

    sysioctl(fd, 32, 22);
    len = sysread(fd, buf, 3);
    sprintf(out, "Length: %d, val: %d\n", len, buf);
    sysputs(out);

    sysclose(fd);
}
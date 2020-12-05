/* test.c */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <test.h>

char *test_case = NULL;
unsigned int pid_g;

void sig3(void) {
    for (int i = 0; i < 3; i++) {
        sysputs("Signal 3.\n");
        syssleep(1000);
    }
}

void sig10(void) {
    sysputs("Signal 10.\n");
    syssleep(1000);
}

void sig22(void) {
    for (int i = 0; i < 5; i++) {
        sysputs("Signal 22.\n");
        syssleep(1000);
    }
}

void sig23(void) {
    sysputs("Signal 23.\n");
    syssleep(1000);
}

/*===========================================================================*/
/* Test case 1 */
/*===========================================================================*/
void test_trigger_signal(void) {
    // trigger signals
    syskill(pid_g, 3);
    syssleep(1000);
    syskill(pid_g, 22);
    syskill(pid_g, 10);
    syssleep(2000);
    syskill(pid_g, 23);
}

void test_signal_priority(void) {
    unsigned int pid;
    char buf[128];
    pid_g = sysgetpid();

    sprintf(buf, "My pid is %d.\n", pid_g);
    sysputs(buf);

    // create 4 signal handlers
    syssignal(3, (sighandler_t) sig3);
    syssignal(10, (sighandler_t) sig10);
    syssignal(22, (sighandler_t) sig22);
    syssignal(23, (sighandler_t) sig23);

    // create and wait
    pid = syscreate(test_trigger_signal, STACKSIZE);
    syswait(pid);
}

/*===========================================================================*/
/* Test case 2 */
/*===========================================================================*/
void test_signal_handler(void) {
    unsigned int pid;
    char buf[128];
    unsigned long ret;

    pid = sysgetpid();

    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // try to set sig number 31
    ret = (unsigned long) syssignal(31, (sighandler_t) sig3);
    sprintf(buf, "Tried to set signal 31. ret code: %d.\n", ret);
    sysputs(buf);

    // try to set sig number -1
    ret = (unsigned long) syssignal(-1, (sighandler_t) sig3);
    sprintf(buf, "Tried to set signal -1. ret code: %d.\n", ret);
    sysputs(buf);

    // create a signal handler
    ret = (unsigned long) syssignal(3, (sighandler_t) sig3);
    sprintf(buf, "Previous sig handler: %d.\n", ret);
    sysputs(buf);

    // modify the same handler
    ret = (unsigned long) syssignal(3, (sighandler_t) sig23);
    sprintf(buf,"Previous sighandler: %d, sig3 sighandler: %d.\n", ret, sig3);
    sysputs(buf);

    // run
    syskill(pid, 3);
}

/*===========================================================================*/
/* Test case 3 */
/*===========================================================================*/
void test_sleep(void){
    int ret;
    char buf[128];

    // set signal 10
    syssignal(10, (sighandler_t) sig10);

    ret = syssleep(10000);
    sprintf(buf, "Sleep was interrupted, remaining: %d.\n", ret);
    sysputs(buf);
}

void test_sys_kill(void) {
    unsigned int pid;
    char buf[128];
    int ret;

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // set something to 3
    syssignal(3, (sighandler_t) sig3);

    // try to signal idle process
    ret = syskill(0, 3);
    sprintf(buf, "Tried to signal idle process. ret code %d.\n", ret);
    sysputs(buf);

    // try to signal non existant process
    ret = syskill(999, 3);
    sprintf(buf, "Tried to signal process 999(non-existing). ret code %d.\n", ret);
    sysputs(buf);

    // try to signal valid signal but theres no handler
    ret = syskill(pid, 4);
    sprintf(buf, "Tried to signal unmapped signal 4. ret code %d.\n", ret);
    sysputs(buf);

    // try to signal sleeping process
    pid = syscreate(test_sleep, STACKSIZE);
    syssleep(1000); // time for sleep to setup
    ret = syskill(pid, 10);
    sprintf(buf, "Tried to signal sleeping process. ret code %d.\n", ret);
    sysputs(buf);

    syswait(pid);
} 

/*===========================================================================*/
/* Test case 4 */
/*===========================================================================*/
void test_wait_sleep(void) {
    syssleep(10000);
}

void test_wait_int(void) {
    int pid;
    int ret;
    char buf[128];

    // set signal 10
    syssignal(10, (sighandler_t) sig10);

    pid = syscreate(test_wait_sleep, STACKSIZE);
    ret = syswait(pid);

    sprintf(buf, "Wait interrupted: %d.\n", ret);
    sysputs(buf);
}

void test_sys_wait(void) {
    unsigned int pid;
    char buf[128];
    int ret;

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // try to wait on idle process
    ret = syswait(0);
    sprintf(buf, "Tried to wait on idle process. ret code: %d.\n", ret);
    sysputs(buf);

    // try to wait on non-existant process
    ret = syswait(999);
    sprintf(buf, "Tried to wait on process 999(non-existing). ret code: %d.\n", ret);
    sysputs(buf);

    // try to wait on itself
    ret = syswait(pid);
    sprintf(buf, "Tried to wait on itself. ret code: %d.\n", ret);
    sysputs(buf);

    // set up a process thats on wait and signal it.
    pid = syscreate(test_wait_int, STACKSIZE);
    syssleep(1000);
    syskill(pid, 10);
    syswait(pid);
}

/*===========================================================================*/
/* Test case 5 */
/*===========================================================================*/
void test_sys_open(void) {
    unsigned int pid;
    char buf[128];
    int ret;
    int fds[5];

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // try on negative numbers
    ret = sysopen(-1);
    sprintf(buf, "Tried to open device no -1. ret code: %d.\n", ret);
    sysputs(buf);

    // try on num > 2
    ret = sysopen(3);
    sprintf(buf, "Tried to open device no 3. ret code: %d.\n", ret);
    sysputs(buf);

    // try to open num 2 again. this should be already open from the shell
    ret = sysopen(2);
    sprintf(buf, "Tried to open keyboard while it is open. ret code: %d.\n", ret);
    sysputs(buf);

    // try to open more that 4 fd
    for (int i = 0; i < 5; i++) {
        fds[i] = sysopen(0);
        sprintf(buf, "Opening zero device. fd: %d.\n", fds[i]);
        sysputs(buf);
    }

    // close all
    for (int i = 0; i < 5; i++) {
        ret = sysclose(fds[i]);
        sprintf(buf, "Closing zero device. fd: %d.\n", ret);
        sysputs(buf);
    }
}

/*===========================================================================*/
/* Test case 6 */
/*===========================================================================*/
void test_sys_write(void) {
    unsigned int pid;
    char buf[128];
    int ret;
    int fds[6];
    char dev_buf[128];

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // try to write to unset fd
    fds[0] = 0;
    ret = syswrite(fds[0], dev_buf, 128);
    sprintf(buf, "Tried to write to unset fd. ret code: %d.\n", ret);
    sysputs(buf);

    // fill in fds
    for (int i = 0; i <= 5; i++) {
        sysopen(0);
        fds[i] = i-1;
    }

    // fd should be from -1 to 4, try to write to each of them
    for (int i = 0; i <= 5; i++) {
        ret = syswrite(fds[i], dev_buf, 128);
        sprintf(buf, "Writing to fd %d, ret code: %d.\n", fds[i], ret);
        sysputs(buf);
    }
}

/*===========================================================================*/
/* Test case 7 */
/*===========================================================================*/
void test_sys_ioctl(void) {
    unsigned int pid;
    char buf[128];
    int ret;
    int fd;

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // test ioctl on zero device
    fd = sysopen(0);
    ret = sysioctl(fd, 23);
    sprintf(buf, "Tried ioctl on zero device. ret code: %d.\n", ret);
    sysputs(buf);
    sysclose(fd);

    // test ioctl on rand device invalid command
    fd = sysopen(1);
    ret = sysioctl(fd, 12);
    sprintf(buf, "Tried ioctl with command 12 on rand device. ret code: %d.\n", ret);
    sysputs(buf);

    // test with valid command
    ret = sysioctl(fd, 23, 2);
    sprintf(buf, "Tried ioctl with command 23 on rand device. ret code: %d.\n", ret);
    sysputs(buf);
    sysclose(fd);
}

/*===========================================================================*/
/* Test case 8 */
/*===========================================================================*/
void test_sys_read(void) {
    unsigned int pid;
    char buf[128];
    char kbd_buf[128];
    int fd;

    memset(kbd_buf, 0, sizeof(kbd_buf));

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // open the keyboard
    fd = sysopen(2);

    // start interactive testing, kbd is open should be listening to kbd commands
    sysputs("Starting interactive testing.\n");
    sysputs("Sleeping for 5 seconds, type anything.\n");
    syssleep(5000);

    sysputs("Typed characters: ");
    for (int i = 0; i < 4; i++) {
        sysread(fd, kbd_buf, 1);
        sprintf(buf, "%c ", *kbd_buf);
        sysputs(buf);
    }
    sysputs("\n");

    sysputs("Sleeping for 5 seconds, type anything.\n");
    syssleep(5000);
    sysread(fd, kbd_buf, 2);
    sprintf(buf, "Two characters read: %s.\n", kbd_buf);
    sysputs(buf);

    sysclose(fd);
}

/*===========================================================================*/
/* Test case 9 */
/*===========================================================================*/
void test_zero_device(void) {
    unsigned int pid;
    char buf[128];
    char z_buf[128];
    int fd, ret;

    memset(z_buf, 0, sizeof(z_buf));

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // open the 0 device
    fd = sysopen(0);

    // a read of negative number
    ret = sysread(fd, z_buf, -1);
    sprintf(buf, "Tried to read negative number of bytes. ret code: %d.\n", ret);
    sysputs(buf);

    // read 7 bytes
    sprintf(z_buf, "12345678");
    sprintf(buf, "Current values in z_buf: %s.\n", z_buf);
    sysputs(buf);
    ret = sysread(fd, (z_buf+1), 7);
    sprintf(buf, "Read of 7 bytes. length: %d, z_buf: %s.\n", ret, z_buf);
    sysputs(buf);

    // a write of negative number
    ret = syswrite(fd, z_buf, -1);
    sprintf(buf, "Tried to write negative number of bytes. ret code: %d.\n", ret);
    sysputs(buf);

    // write 10 bytes
    ret = syswrite(fd, z_buf, 10);
    sprintf(buf, "Write of 10 bytes. length: %d.\n", ret);
    sysputs(buf);
}

/*===========================================================================*/
/* Test case 10 */
/*===========================================================================*/
void test_rand_device(void) {
    unsigned int pid;
    char buf[128];
    char r_buf[128];
    int fd, ret;

    memset(r_buf, 0, sizeof(r_buf));

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // open rand device
    fd = sysopen(1);

    // set ioctl to current pid
    ret = sysioctl(fd, 23, pid);

    // syswrite should fail
    ret = syswrite(fd, r_buf, 1);
    sprintf(buf, "Tried to write to rand device. ret code: %d.\n", ret);
    sysputs(buf);

    // read one byte
    ret = sysread(fd, r_buf, 1);
    sprintf(buf, "Reading rand device. length: %d, number: %d.\n", ret, *(unsigned char *)r_buf);
    sysputs(buf);

    // read 4 byte
    ret = sysread(fd, r_buf, 4);
    sprintf(buf, "Reading rand device. length: %d, number: %d.\n", ret, *(int *)r_buf);
    sysputs(buf);

    // read 6 byte
    ret = sysread(fd, r_buf, 6);
    sprintf(buf, "Reading rand device. length: %d, number 1: %d, number 2: %d.\n", ret, *(int *)r_buf, *(unsigned short *)(r_buf + 4));
    sysputs(buf);

    sysclose(fd);
}

/*===========================================================================*/
/* Test case 11 */
/*===========================================================================*/
void test_kbd_eof(void) {
    unsigned int pid;
    char buf[128];
    char k_buf[128];
    int fd, ret;

    memset(k_buf, 0, sizeof(k_buf));

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    // interactive test for kbd eof
    fd = sysopen(2);
    // echo on for testing purposes
    ret = sysioctl(fd, 74);

    // set oef to +
    ret = sysioctl(fd, 18, 43);

    sysputs("Type up to 4 characters including one ctrl-d.\n");
    ret = sysread(fd, k_buf, 4);
    sprintf(buf, "\nRead characters. length: %d, string: %s.\n", ret, k_buf);
    sysputs(buf);

    sysclose(fd);
}

/*===========================================================================*/
/* Test case runner */
/*===========================================================================*/
void run_test(void) {
    int pid;
    if (strcmp(test_case, "signalpriority") == 0) {
        pid = syscreate(test_signal_priority, STACKSIZE);
    } else if (strcmp(test_case, "signalhandler") == 0) {
        pid = syscreate(test_signal_handler, STACKSIZE);
    } else if (strcmp(test_case, "syskill") == 0) {
        pid = syscreate(test_sys_kill, STACKSIZE);
    } else if (strcmp(test_case, "syswait") == 0) {
        pid = syscreate(test_sys_wait, STACKSIZE);
    } else if (strcmp(test_case, "sysopen") == 0) {
        pid = syscreate(test_sys_open, STACKSIZE);   
    } else if (strcmp(test_case, "syswrite") == 0) {
        pid = syscreate(test_sys_write, STACKSIZE);
    } else if (strcmp(test_case, "sysioctl") == 0) {
        pid = syscreate(test_sys_ioctl, STACKSIZE);
    } else if (strcmp(test_case, "sysread") == 0) {
        pid = syscreate(test_sys_read, STACKSIZE);
    } else if (strcmp(test_case, "zerodevice") == 0) {
        pid = syscreate(test_zero_device, STACKSIZE);
    } else if (strcmp(test_case, "randdevice") == 0) {
        pid = syscreate(test_rand_device, STACKSIZE);
    } else if (strcmp(test_case, "kbdeof") == 0) {
        pid = syscreate(test_kbd_eof, STACKSIZE);
    } else {
        kprintf("Unknown test given.\n");
        return;
    }

    syswait(pid);
}
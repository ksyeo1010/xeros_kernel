/* test.c */

#include <xeroskernel.h>
#include <xeroslib.h>


/* Set the test case here. */
#define TEST_CASE 1

void run_test(void);                /* declaration so we can this on init.c */

void test_1(void);                  /* decalration of test 1 */
void test_1_helper_1(void);         /* decalration of test 1 helper 1 */
void test_1_helper_2(void);         /* decalration of test 1 helper 2*/

void test_2(void);                  /* decalration of test 2 */
void test_2_helper(void);           /* decalration of test 2 helper */

void test_3(void);                  /* decalration of test 3 */
void test_3_helper1(void);          /* decalration of test 3 helper 1 */
void test_3_helper2(void);          /* decalration of test 3 helper 2 */

static int g_priority;              /* decalration g_priority global var */
void test_4(void);                  /* decalration of test 4 */
void test_4_helper(void);           /* decalration of test 4 helper*/

void test_5(void);                  /* decalration of test 5 */
void test_5_helper_1(void);         /* decalration of test 5 helper 1 */
void test_5_helper_2(void);         /* decalration of test 5 helper 2 */

void test_6(void);                  /* decalration of test 6 */
void test_6_helper(void);           /* decalration of test 6 helper */

/* the main function that will run a test set */
void run_test() {
    #if (TEST_CASE == 1)
        create(test_1, STACKSIZE);
    #elif (TEST_CASE == 2)
        create(test_2, STACKSIZE);
    #elif (TEST_CASE == 3)
        create(test_3, STACKSIZE);
    #elif (TEST_CASE == 4)
        create(test_4, STACKSIZE);
    #elif (TEST_CASE == 5)
        create(test_5, STACKSIZE);
    #elif (TEST_CASE == 6)
        create(test_6, STACKSIZE);
    #endif
}

/////////////////////////////////////////////////////////////////////////////////////

/* test case 1 */
void test_1() {
    // create two processes
    syscreate(test_1_helper_1, STACKSIZE);
    syscreate(test_1_helper_2, STACKSIZE);
}

void test_1_helper_1() {
    unsigned int pid;
    int i;
    char s[64];

    pid = sysgetpid();

    // run loop
    for (i = 0; i < 20; i++) {
        sprintf(s, "Process %d is running.\n", pid);
        sysputs(s);
    }
}

void test_1_helper_2() {
    unsigned int pid;
    int i;
    char s[64];

    pid = sysgetpid();

    // run loop
    for (i = 0; i < 20; i++) {
        sprintf(s, "Process %d is running.\n", pid);
        sysputs(s);
    }
}

/////////////////////////////////////////////////////////////////////////////////////

/* test case 2 */
void test_2() {
    unsigned int pid, rc;
    int i;
    char s[64];

    pid = sysgetpid();

    // create process
    rc = syscreate(test_2_helper, STACKSIZE);

    // block on sem 0
    sysP(0);

    // kill process
    i = syskill(rc);
    sprintf(s, "Process %d: test kill successful, RC: %d.\n", pid, i);
    sysputs(s);

    // try to kill again
    i = syskill(rc);
    sprintf(s, "Process %d: test kill unsuccessful, RC: %d.\n", pid, i);
    sysputs(s);

    // kill myself
    sprintf(s, "Process %d: killing itself.\n", pid);
    sysputs(s);

    syskill(pid);

    sysputs("Should not print.\n");
    
}

void test_2_helper() {
    int i;
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    for (i = 0; i < 5; i++) {
        sprintf(s, "Process %d: printing loop: %d.\n", pid, i);
        sysputs(s);
        // unblock on sem 0
        sysV(0);
        sysyield();
        sprintf(s, "Process %d: should have been killed.\n", pid);
        sysputs(s);
    }
}

/////////////////////////////////////////////////////////////////////////////////////

/* test case 3 */
void test_3() {
    unsigned int rc, pid;
    char s[64];

    pid = sysgetpid();

    rc = syscreate(test_3_helper1, STACKSIZE);

    // block on semaphore 0
    sysP(0);

    // kill process that was blocked
    sprintf(s, "Process %d: killing process %d.\n", pid, rc);
    sysputs(s);

    syskill(rc);

    // release semaphore 1, at time point there is nothing on semaphore 1
    // releaseing increases value +1, blocking should continue.
    sysV(1);
    sysP(1);

    rc = syscreate(test_3_helper2, STACKSIZE);

    // block on semaphore 0 again
    sysP(0);

    // kill process that was sleeping
    sprintf(s, "Process %d: killing process %d.\n", pid, rc);
    sysputs(s);

    syskill(rc);
}

void test_3_helper1() {
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    sprintf(s, "Process %d: starting.\n", pid);
    sysputs(s);

    sprintf(s, "Process %d: blocking.\n", pid);
    sysputs(s);

    // release semaphore 0, block on semaphore 1
    sysV(0);
    sysP(1);

    sprintf(s, "Process %d: should have been killed.\n", pid);
    sysputs(s);
}

void test_3_helper2() {
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    sprintf(s, "Process %d: starting.\n", pid);
    sysputs(s);

    sprintf(s, "Process %d: sleeping.\n", pid);
    sysputs(s);

    // release semaphore 0
    sysV(0);

    // sleep
    syssleep(3000);

    sprintf(s, "Process %d: should have been killed.\n", pid);
    sysputs(s);
}

/////////////////////////////////////////////////////////////////////////////////////

/* test case 4 */
void test_4() {
    int i, prio;
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    g_priority = 1;

    syssetprio(0);

    for (i = 0; i < 3; i++) {
        syscreate(test_4_helper, STACKSIZE);

        // block on sem 0
        sysP(0);

        g_priority++;
    }

    // create new process with priority 2
    syscreate(test_4_helper, STACKSIZE);
    g_priority = 2;
    sysP(0);

    // update priority to 2
    syssetprio(2);
    sprintf(s, "Process %d: setting priority: %d.\n", pid, 2);
    sysputs(s);

    sprintf(s, "Process %d: printing high priority first.\n", pid);
    sysputs(s);

    // unblock sem 1
    sysV(1);

    // block on sem 1
    sysP(1);

    prio = syssetprio(-1);

    sprintf(s, "Process %d: priority: %d.\n", pid, prio);
    sysputs(s);

    sysV(1);
}

void test_4_helper() {
    unsigned int pid;
    int prio;
    char s[64];

    pid = sysgetpid();

    sprintf(s, "Process %d: starting.\n", pid);
    sysputs(s);

    sprintf(s, "Process %d: setting priority: %d.\n", pid, g_priority);
    sysputs(s);

    syssetprio(g_priority);

    sprintf(s, "Process %d: blocking.\n", pid);
    sysputs(s);

    // unblock on sem 0, block on sem 1
    sysV(0);
    sysP(1);

    prio = syssetprio(-1);

    sprintf(s, "Process %d: priority: %d.\n", pid, prio);
    sysputs(s);

    // unblock sem 1
    sysV(1);
}

/////////////////////////////////////////////////////////////////////////////////////

/* test case 5 */
void test_5() {
    int i;
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    // create two process
    syscreate(test_5_helper_1, STACKSIZE);
    syscreate(test_5_helper_2, STACKSIZE);

    sysP(0);
    sysP(0);

    sysV(1);
    sysV(1);

    // sleep for 3 ticks
    sprintf(s, "Process %d: sleeping.\n", pid);
    sysputs(s);

    syssleep(30);

    for (i = 0; i < 10; i++) {
        sprintf(s, "Process %d: loop: %d.\n", pid, i);
        sysputs(s);
    }

}

void test_5_helper_1() {
    int i;
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    sprintf(s, "Process %d: starting.\n", pid);
    sysputs(s);

    // set priority to 1
    sprintf(s, "Process %d: setting priority to 1.\n", pid);
    sysputs(s);

    syssetprio(1);
    sysV(0);
    sysP(1);

    for (i = 0; i < 20; i++) {
        sprintf(s, "Process %d: loop: %d.\n", pid, i);
        sysputs(s);
    }
}

void test_5_helper_2() {
    int i;
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    sprintf(s, "Process %d: starting.\n", pid);
    sysputs(s);

    // set priority to 2
    sprintf(s, "Process %d: setting priority to 2.\n", pid);
    sysputs(s);

    syssetprio(2);
    sysV(0);
    sysP(1);

    sprintf(s, "Process %d: sleeping.\n", pid);
    sysputs(s);

    // sleep for 1 tick
    syssleep(10);

    for (i = 0; i < 20; i++) {
        sprintf(s, "Process %d: loop: %d.\n", pid, i);
        sysputs(s);
    }
}

/////////////////////////////////////////////////////////////////////////////////////

/* test case 6 */
void test_6() {
    int i;
    unsigned int pid;
    char s[64];

    pid = sysgetpid();

    // create process
    for (i = 0; i < 2; i++) {
        syscreate(test_6_helper, STACKSIZE);
    }

    sprintf(s, "Process %d: setting priority to lowest.\n", pid);
    sysputs(s);

    syssetprio(0);

    // loop for ever
    for (;;) {
        sprintf(s, "Process %d: running loop.\n", pid);
        sysputs(s);
    }
}

void test_6_helper() {
    unsigned int pid;
    char s[64];

    pid = sysgetpid();
    
    // loop for ever
    for (;;) {
        sprintf(s, "Process %d: running loop.\n", pid);
        sysputs(s);
    }
}
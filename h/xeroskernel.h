/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

#include <constants.h>

/* defines for testing */

// #define TEST /* Comment/Uncomment this line for testing. */
#ifdef TEST

/* run the test based on test case */
extern void run_test(void);

#endif

/* Console Printing */
#define CONSOLE_PRINTING /* Comment/Uncomment this line to print on console */

/* For Debugging ONLY */
// #define IS_DEBUG /* Comment/Uncomment this line to print DEBUG messages */
#ifdef IS_DEBUG
// from https://piazza.com/class/keulh3m6vuj47c?cid=46
#define PRINT(...) do {\
    __asm __volatile("cli":::);\
    kprintf("    %s \n\t%s:%d: ",\
        __func__, __FILE__, __LINE__);\
    kprintf(__VA_ARGS__);\
} while(0)
#define FAIL(...) do {\
    __asm __volatile("cli":::);\
    kprintf("    %s failed:\n\t%s:%d: ",\
        __func__, __FILE__, __LINE__);\
    kprintf(__VA_ARGS__);\
    __asm __volatile("hlt":::);\
} while(0)
#else
    #define PRINT(...) ((void)0)
    #define FAIL(...) ((void)0)
#endif

/* Symbolic constants used throughout gutted Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */


/* Universal return constants */

#define	OK            1         /* system call ok               */
#define SYSOK         0         /* system ok                    */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */
#define LASTCONST    -5

/* Succeed and Failed */
#define FAILED          0       /* Failed to do a function */
#define SUCCEED         1       /* Succeed in doing a function */
#define BLOCKED        -1       /* Blocked state for semaphore */

/* Tick values */
#define PIT_VALUE       100
#define TICK_SPLIT      (1000/PIT_VALUE)

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);
void           set_evec(unsigned int xnum, unsigned long handler);

#define STACKSIZE 8192 /* Just for testing part. */

/* ========================================================================== */
/* Memory function declarations. */
/* ========================================================================== */

/**
 * @brief Initializes the free memory list.
 *        Sets free memory from kernel end to HOLSTART, and HOLEEND to maxaddr.
 */
extern void kmeminit(void);

/**
 * @brief Allocates space to a process.
 *        Searches the free memlist, gets a memslot from the memlist, updates
 *        memlist with changes.
 * 
 * @param {size} The size to allocate.
 * @returns The start of the memslot allocated (dataStart[0]) if availabe slot was found
 *          0 otherwise.
 */
extern void *kmalloc(size_t size);

/**
 * @brief Puts a memslot back to the memlist.
 *        Does a sanity check, puts the memslot in the correct position and
 *        merges adjacent free list blocks.
 * 
 * @param {ptr} The start of the address. (dataStart[0])
 * @returns 1 if correctly freed, 0 othersize.
 */
extern int kfree(void *ptr);

/* ========================================================================== */
/* Dispatcher function declarations  */
/* ========================================================================== */

/**
 * @brief Initializes the pcbTable with size PCB_TABLE_SIZE and
 *        also initializes the readyQueue.
 *        Creates the first process, which is the idle process.
 */
extern void dispInit(void);

/**
 * @brief Manages the syscall events for our xeros kernel. 
 *        Supported syscall types:
 *        - 1. CREATE: creates the process and adds to readyQueue.
 *        - 2. YIELD:  puts the process to the readyqueue.
 *        - 3. STOP:   removes the process from readyqueue and puts the pcb in the
 *                     pcb table ready to use again. (STATE = STOPPED).
 */
extern void dispatch(void);

/* ========================================================================== */
/* Context switcher function declarations */
/* ========================================================================== */

/**
 * @brief Initializes the context switcher.
 *        Creates and sets the SYS_CALL event number to the IDT table.
 */
extern void contextinit(void);

/**
 * @brief Switches the context from kernel to user and back and forth.
 *        The INT will call the method, save the registers and return the
 *        type of request back to the dispatcher.
 * 
 * @param {pcb} The current process to context switch.
 * @returns The request type.
 */
extern int contextswitch(pcb_t *pcb);

/* ========================================================================== */
/* Create function declaration */
/* ========================================================================== */

/**
 * @brief Creates a process and adds it to the process readyQueue.
 *        Calls kmalloc to get a free memslot, gets a stopped pcb from the
 *        pcb table, sets the values of the pcb. Also creates a context_frame
 *        to save register values onto the stack for the context_switcher.
 * 
 * @param {func} The function pointer to create.
 * @param {stack} The stack size to allocate.
 * @returns SUCCEED if no errors occur, FAILED otherwise.
 */ 
extern int create(void (*func)(void), int stack);

/* ========================================================================== */
/* Semaphore function declarations */
/* ========================================================================== */

/**
 * @brief Initiates the semaphores as well as the blocked
 *        lists.
 * 
 */
extern void seminit(void);


/**
 * @brief Handles the blocking part of a semaphore.
 *        Blocks if semaphore value = 0, otherwise decrements the value
 *        and keeps running.
 * 
 * @param {semNo} The number of semaphore.
 * @param {pcb} The process to block, if it should.
 * @returns SUCCEED if no errors occur, FAILED otherwise.
 */
extern int P_kern(int semNo, pcb_t *pcb);

/**
 * @brief Handles the unblocking part of a semaphore.
 *        Unblocks if semaphore value is 0, otherwise increments the value.
 *        The process always returns to the dispatcher.
 * 
 * @param {semNo} The number of semaphore.
 * @returns SUCCEED if no errors occur, FAILED otherwise.
 */
extern int V_kern(int semNo);

/* Sleep function declarations */

/**
 * @brief Coverts milliseconds into tick time and adds the process
 *        into a sleep list.
 * 
 * @param {pcb} The process to add to sleep list.
 * @param {milliseconds} The time to sleep the amount for.
 */
extern void sleep(pcb_t *pcb, unsigned int milliseconds);

/**
 * @brief Each time a TIMER_INT occurs, decrements the tick.
 *        If the tick is 0, add all process whose tick time = 0 back
 *        to ready queue.
 * 
 */
extern void tick(void);

/* ========================================================================== */
/* Syscall function declaration */
/* ========================================================================== */

/**
 * @brief Calls interrupt, to make context switch.
 *        Sets the request type and pointer to function arguments so that
 *        the dispatcher can handle the request type. Uses va_list to get
 *        the pointer to the initial part of the function arguments.
 * 
 * @param {call} The request type.
 * @param {...} The parameter(s) depending on the function call.
 * @return The return value if any.
 */
extern int syscall(int call, ...);

/**
 * @brief Sends an int to create the current process.
 * 
 * @param {func} The function to create.
 * @param {stack} The size of the stack.
 * 
 * @returns The return value from the function.
 */
extern unsigned int syscreate(void (*func)(void), int stack);

/**
 * @brief Sends an int to yield the current process.
 */
extern void sysyield(void);

/**
 * @brief Sends an int to stop the current process.
 */
extern void sysstop(void);

/**
 * @brief Sends an int to get current process id.
 * 
 * @returns The pid of the current running process.
 */
extern pid_t sysgetpid(void);

/**
 * @brief Displays a null terminated string to the screen.
 * 
 * @param {str} The null terminated character array.
 */
extern void sysputs(char *str);

/**
 * @brief Requests the signal to be delivered to a specific process.
 * 
 * @param {pid} The pid of the process to send the signal.
 * @param {signalNumber} The signal number to send.
 * @returns 0 if succeed, -1 if signal number is invalid, -999 if process does not exist.
 */
extern int syskill(pid_t pid, int signalNumber);

/**
 * @brief Sets the priority of a process. If a -1 is given
 *        just returns the current priority.
 * 
 * @param {priority} The priority in the range of -1 to 3.
 * @returns The priority it had if succeed, -1 otherwise.
 */
extern int syssetprio(int priority);

/**
 * @brief Sends a system call to handle the P_kern in the kernel.
 *        The process is blocked if semaphore value = 0;
 * 
 * @param {semNo} The number of semaphore to block the process.
 * @returns 1 if succeed, 0 otherwise.
 */
extern int sysP(int semNo);

/**
 * @brief Sends a system call to handle the V_kern in the kernel.
 *        The process with highest priority is added to the ready queue.
 *        If there are no process, semaphore value is increased by 1.
 * 
 * @param {semNo} The number of semaphore to unblock the process.
 * @returns 1 if succeed, 0 otherwise.
 */
extern int sysV(int semNo);

/**
 * @brief Puts the process to sleep given in milliseconds.
 * 
 * @param {milliseconds} The time to sleep.
 * @returns 0 if 
 */
extern unsigned int syssleep(unsigned int milliseconds);

/**
 * @brief For each process, record the pid, current state and milliseconds
 *        charged to the process.
 *        IDLE process is always recorded.
 * 
 * @param {ps} The process statuses.
 * @return -1 if address is in the hole, -2 if the structure goes beyond maxaddr,
 *         otherwise the last element starting from 0.
 */
extern int sysgetcputimes(processStatuses *ps);

/**
 * @brief Sets the signal handler given the signal number of the process.
 *        signum must be between 0 and 30.
 * 
 * @param {signum} The signal number to set the handler
 * @param {handler} The handler itself
 * @return 0 if an error occurs, the previous otherwise.
 */
extern sighandler_t syssignal(int signum, sighandler_t handler);

/**
 * @brief Call to return to previous context used by sigtramp only.
 *        It also enables ignored signals.
 * 
 * @param {cntxPtr} The context to return to.
 */
extern void syssigreturn(void *cntxPtr);

/**
 * @brief Call to wait for a process to terminate.
 * 
 * @param {pid} The process pid to wait for.
 * @return 0 if terminates normally, -1 otherwise.
 */
extern int syswait(pid_t pid);

/**
 * @brief Call to open a device.
 * 
 * @param {device_no} The device number to open.
 * @returns -1 if an error occurred, 0 otherwise.
 */
extern int sysopen(int device_no);

/**
 * @brief Call to close a device.
 * 
 * @param {fd} The fd number a device is associated with.
 * @returns -1 if an error occurred, 0 otherwise.
 */
extern int sysclose(int fd);

/**
 * @brief Call to write to a device.
 * 
 * @param {fd} The fd number a device is associated with.
 * @param {buff} The buffer containing data to write to a device.
 * @param {bufflen} The length of the buffer.
 * @returns -1 if an error occurred, the bytes written to a device otherwise.
 */
extern int syswrite(int fd, void *buff, int bufflen);

/**
 * @brief Call to read from a device.
 * 
 * @param {fd} The fd number a device is associated with.
 * @param {buff} The buffer containing data to read from a device.
 * @param {bufflen} The length of the buffer.
 * @returns -1 if an error occurred, the bytes read from a device otherwise.
 */
extern int sysread(int fd, void *buff, int bufflen);

/**
 * @brief Call to make special control functions to a device.
 * 
 * @param {fd} The fd number a device is associated with.
 * @param {command} The command number to run.
 * @param {...} Extra parameters needed by the command.
 * @returns -1 is failed, 0 otherwise.
 */
extern int sysioctl(int fd, unsigned long command, ...);

/* ========================================================================== */
/* root function declarations */
/* ========================================================================== */

/**
 * @brief The root process to run when the kernel is initalized.
 *        Other syscalls are done through this function.
 */
extern void root(void);

/* idle process declaration */

/**
 * @brief The idle process. Does infinite amount of hlt executions.
 */
extern void idleproc(void);

/* ========================================================================== */
/* device function declarations */
/* ========================================================================== */

/**
 * @brief Initiates the devices currently supported by the kernel.
 *        ZERO, RAND, KBD are the devices that setup during the function call.
 */
extern void devinit(void);

/**
 * @brief Function to run once a keyboard interrupt occurs. Updated the KBD
 *        buffer, writes to a FD buffer if its currently blocked by 
 *        the keyboard device.
 */
extern void kbdint_handler(void);

/* Anything you add must be between the #define and this comment */
#endif

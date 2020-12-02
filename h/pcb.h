/* pcb.h - struct pcb, process state */

#ifndef PCB_H
#define PCB_H

// kernel calls
#define SYS_CALL        0x31        /* general call to idt */

// syscall identifiers
#define SYS_CREATE          0x10        /* create process block */
#define SYS_YIELD           0x11        /* set current process to the end of queue */
#define SYS_STOP            0x12        /* remove process from ready queue */
#define SYS_GETPID          0x13        /* gets the pid of the current process */
#define SYS_PUTS            0x14        /* displays null terminated string to screen */
#define SYS_KILL            0x15        /* kills the process given pid */
#define SYS_SETPRIO         0x16        /* sets the priority of the process */
#define SYS_P               0x17        /* blocks process in a semaphore */
#define SYS_V               0x18        /* unblocks process in a semaphore */
#define SYS_SLEEP           0x19        /* sleeps the process given amount of time */
#define TIMER_INT           0x20        /* The timer int value */
#define SYS_CPUTIMES        0x21
#define SYS_SIGNAL          0x60
#define SYS_SIGRETURN       0x61
#define SYS_WAIT            0x62

// process states
#define STATE_STOPPED       0       /* process stopped state */
#define STATE_RUNNING       1       /* process running state */
#define STATE_READY         2       /* process ready state */
#define STATE_BLOCKED       3       /* process blocked state */
#define STATE_SLEEP         4       /* process sleep state */
#define STATE_WAITING       5

// signal return values
#define SIG_OK               0
#define SIG_FAIL            -1
#define SIG_NOT_EXISTS      -999
#define SIG_EINTR           -777

// process table size
#define PCB_TABLE_SIZE      64

// priority queue size
#define PQ_SIZE             4

// signal table size
#define SIG_TABLE_SIZE      32

// define pid type
typedef unsigned int pid_t;

// signal handler type
typedef void (*sighandler_t)(void *);

// Process control block
typedef struct process_control_block {
    pid_t pid;                              /* The pid of the process */ 
    int state;                              /* The state of the process */
    int rc;                                 /* The return value */
    int priority;                           /* The priority of the process */
    int semNo;
    int sig_mask;
    int sig_ignored;
    int          bufferlen;
    void        *buffer;
    long         cpuTime;                   /* CPU time consumed                     */
    unsigned int otherpid;
    unsigned int tick;                      /* The current tick of a process */
    unsigned long esp;                      /* The stack pointer of the process */
    unsigned long addr_start;               /* The initial address of the process */
    unsigned long args;                     /* The pointer to the arguments in the stack */
    struct process_control_block *next;     /* The next pcb in the queue if any */
    struct process_control_block *prev;
    struct process_control_block *wait_head;
    struct process_control_block *wait_tail;
    struct process_control_block *waiting_on;
    sighandler_t sigTable[SIG_TABLE_SIZE];
} pcb_t;

typedef struct struct_ps processStatuses;
struct struct_ps {
  int     entries;                  // Last entry used in the table
  pid_t   pid[PCB_TABLE_SIZE];      // The process ID
  int     status[PCB_TABLE_SIZE];   // The process status
  long    cpuTime[PCB_TABLE_SIZE];  // CPU time used in milliseconds
};

// Context frame structure
typedef struct context_frame {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
    unsigned long iret_eip;
    unsigned long iret_cs;
    unsigned long eflags;
    unsigned long free_slots[];
} cf_t;

// signal context frame
typedef struct signal_context_frame {
    struct context_frame ctx;
    unsigned long returnAddr;
    unsigned long handlerAddr;
    unsigned long contextFramePtr;
    unsigned long prevSigNum;
} sigframe_t;

// global variable for pcb id;
extern pid_t pcb_id;

// process table
pcb_t pcbTable[PCB_TABLE_SIZE];

/* helper function declarations */

/**
 * @brief Initializes the ready queue.
 *        readyqueue just starts with NULL.
 */
extern void initReadyQueue(void);

/**
 * @brief Initializes the pcb table.
 *        Sets everything to {0, STATE_STOPPED, NULL, NULL, NULL, NULL}
 */
extern void initPcbTable(void);

/**
 * @brief Gets the next process from the ready queue.
 *        Sets the state of the process as STATE_RUNNING.
 *        Moves pointer of readyQueue to readyQueue->next.
 * 
 * @return next process in the ready queue.
 */ 
extern pcb_t *next(void);

/**
 * @brief Removes the process from the ready queue.
 *        Sets the state of process to STOPPED so it can be used by other
 *        processes.
 * 
 * @param {process} The process to remove.
 */ 
extern void cleanup(pcb_t *process);

/**
 * @brief Adds a process to the ready queue.
 *        Moves tail pointer.
 *        When it is first called, readyQueue = tail = process
 * 
 * @param {process} The current process to add to ready queue.
 */ 
extern void ready(pcb_t *process);

/**
 * @brief Gets the pid with corresponding pid.
 *        Searches through the pcb table to find corresponding
 *        pid.
 * 
 * @param {pid} The pid of a process.
 * @return pcb with corresponding pid, NULL if not found.
 */ 
extern pcb_t *getPCB(pid_t pid);

/**
 * @brief Helper function to set the priority given a pid. NOTE: not PCB
 * 
 * @param {pid} The pid of the process.
 * @param {priority} The priority to set.
 */ 
extern void setPriority(pid_t pid, int priority);

/**
 * @brief Helper function to remove a process from the ready queue.
 * 
 * @param {pcb} The process to remove.
 */ 
extern void removeFromReadyQueue(pcb_t *pcb);

/**
 * @brief Helper function to remove a process from the block queue.
 * 
 * @param {pcb} The process to remove.
 */ 
extern void removeFromBlockQueue(pcb_t *pcb);

/**
 * @brief Helper function to remove a process from the sleep queue.
 * 
 * @param {pcb} The process to remove.
 */ 
extern void removeFromSleepQueue(pcb_t *pcb);

/**
 * @brief Helper function to remove a process from a queue.
 * 
 * @param {pcb} The process to remove.
 * @returns SYSOK is successful, SYSERR otherwise.
 */ 
extern int removeFromQueue(pcb_t *pcb);

extern sighandler_t set_signal(pcb_t *pcb, int signum, sighandler_t handler);

extern void sigtramp(void (*handler)(void *), void *cntxPtr);

extern int signal(pid_t pid, int signum);

extern int addToWaitQueue(pcb_t *pcb, pid_t pid);

extern void removeFromWaitQueue(pcb_t *pcb);

extern void restoreToReadyQueue(pcb_t *pcb);

#endif
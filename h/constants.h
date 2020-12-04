/* constants.h */

#ifndef CONSTANTS_H
#define CONSTANTS_H

// kernel calls
#define TIMER_INT       0x20        /* The timer int value */
#define KBD_INT         0x21
#define SYS_CALL        0x31        /* general call to idt */

// syscall identifiers
#define SYS_CREATE          0x60        /* create process block */
#define SYS_YIELD           0x61        /* set current process to the end of queue */
#define SYS_STOP            0x62        /* remove process from ready queue */
#define SYS_GETPID          0x63        /* gets the pid of the current process */
#define SYS_PUTS            0x64        /* displays null terminated string to screen */
#define SYS_KILL            0x65        /* kills the process given pid */
#define SYS_SETPRIO         0x66        /* sets the priority of the process */
#define SYS_P               0x67        /* blocks process in a semaphore */
#define SYS_V               0x68        /* unblocks process in a semaphore */
#define SYS_SLEEP           0x69        /* sleeps the process given amount of time */
#define SYS_CPUTIMES        0x6A        /* gets the cpu running time of each process alive */
#define SYS_SIGNAL          0x70        /* set the signal of a process */
#define SYS_SIGRETURN       0x71        /* sig return call for the sig tramp */
#define SYS_WAIT            0x72        /* wait call for a process to terminate */
#define SYS_OPEN            0x80        /* open call of a device */
#define SYS_CLOSE           0x81        /* close call of a device */
#define SYS_WRITE           0x82        /* write call of a device */
#define SYS_READ            0x83        /* read call of a device */
#define SYS_IOCTL           0x84        /* ioctl call of a device */

// process states
#define STATE_STOPPED       0       /* process stopped state */
#define STATE_RUNNING       1       /* process running state */
#define STATE_READY         2       /* process ready state */
#define STATE_BLOCKED       3       /* process blocked state */
#define STATE_SLEEP         4       /* process sleep state */
#define STATE_WAITING       5       /* the waiting state */

// signal return values
#define SIG_OK               0
#define SIG_FAIL            -1
#define SIG_EINTR           -777
#define SIG_NOT_EXISTS      -999

// device return values
#define DEV_BLOCK           -2
#define DEV_FAIL            -1
#define DEV_OK              0

// devices
#define ZERO                0
#define RAND                1
#define KEYBOARD            2

// ioctl commands
#define RESET_SEED          23
#define COMMAND_EOF         18
#define COMMAND_ECHO_OFF    73
#define COMMAND_ECHO_ON     74

// process table size
#define PCB_TABLE_SIZE      64

// priority queue size
#define PQ_SIZE             4

// signal table size
#define SIG_TABLE_SIZE      32

// fd table size
#define FD_TABLE_SIZE       4

// define pid type
typedef unsigned int pid_t;

// signal handler type
typedef void (*sighandler_t)(void *);

// FD structure
typedef struct fd_struct {
    struct devsw_struct *dev;
} fd_t;

// Process control block
typedef struct process_control_block {
    pid_t pid;                                      /* The pid of the process */ 
    int state;                                      /* The state of the process */
    int rc;                                         /* The return value */
    int priority;                                   /* The priority of the process */
    int semNo;                                      /* The semaphore number the process is currently on */
    int sig_mask;                                   /* The sigmask bits */
    int sig_ignored;                                /* The ignored sigmasks */
    int          bufflen;                           /* The bufferlen currently the process is holding */
    void        *buf;                               /* The buffer the process is currently holding */
    long         cpuTime;                           /* CPU time consumed                     */
    unsigned int tick;                              /* The current tick of a process */
    unsigned long esp;                              /* The stack pointer of the process */
    unsigned long addr_start;                       /* The initial address of the process */
    unsigned long args;                             /* The pointer to the arguments in the stack */
    struct process_control_block *next;             /* The next pcb in the queue if any */
    struct process_control_block *prev;             /* The prev pointer */
    struct process_control_block *wait_head;        /* The head pointer of the waiting processes */
    struct process_control_block *wait_tail;        /* The tail pointer of the waiting processes */
    struct process_control_block *waiting_on;       /* The process currently waiting on another process */
    sighandler_t sigTable[SIG_TABLE_SIZE];          /* The signal table of a process */
    fd_t fdt[FD_TABLE_SIZE];                        /* The file descriptor table of a process */
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
    unsigned long returnValue;
} sigframe_t;

// device structure
typedef struct devsw_struct {
    int (*dvopen)(pcb_t *);
    int (*dvclose)(pcb_t *);
    int (*dvread)(pcb_t *, void *, int);
    int (*dvwrite)(pcb_t *, void *, int);
    int (*dvioctl)(pcb_t *, unsigned long, void *);
} devsw;

#endif
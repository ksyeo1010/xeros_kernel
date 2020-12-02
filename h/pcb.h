/* pcb.h - struct pcb, process state */

#ifndef PCB_H
#define PCB_H

#include <constants.h>

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

/**
 * @brief Sets the a signal and the handler given the process.
 * 
 * @param {pcb} The process to set the handler.
 * @param {signum} The signal number to set.
 * @param {handler} The handler itself.
 * @returns SYS_FAIL if an error occurs, otherwise the previous sighandler is returned.
 */
extern sighandler_t set_signal(pcb_t *pcb, int signum, sighandler_t handler);

/**
 * @brief Handles the trampoline code for a signal.
 * 
 * @param {handler} The handler to run given a signal.
 * @param {cntxPtr} The context to return once it is finished.
 */
extern void sigtramp(void (*handler)(void *), void *cntxPtr);

/**
 * @brief Handles the signal case in the kernel. Allocates the signal context frame
 *        and also handles the delivery.
 * 
 * @param {pid} The pid to send the signal to.
 * @param {signum} The signal number to send.
 * @returns SIG_FAIL if signal doesn't exists, SIG_NO_EXISTS if process doesn't exists
 *          SIG_OK otherwise.
 */
extern int signal(pid_t pid, int signum);

/**
 * @brief Adds a process the waiting queue of another process.
 * 
 * @param {pcb} The process currently waiting.
 * @param {pid} The process pid to wait for.
 * @returns SIG_FAIL if pid is 0 or no available pid exists, SIG_OK otherwise.
 */
extern int addToWaitQueue(pcb_t *pcb, pid_t pid);

/**
 * @brief Removes a process from its waiting queue.
 * 
 * @param {pcb} The process to remove from the waiting queue.
 */
extern void removeFromWaitQueue(pcb_t *pcb);

/**
 * @brief Restores all waiting process back to the ready queue
 *        if the process terminates.
 * 
 * @param {pcb} The process that terminates.
 */
extern void restoreToReadyQueue(pcb_t *pcb);

/**
 * @brief Given a 32-bit integer, gets the highest bit turned on.
 * 
 * @param {bits} The 32-bit integer.
 * @returns The highest bit turned on.
 */
extern int getsig(int bits);

#endif
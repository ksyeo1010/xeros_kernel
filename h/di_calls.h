/* di_calls.h  */

#ifndef DI_CALLS_H
#define DI_CALLS_H

#include <constants.h>

/* Device table size */
#define DEV_TABLE_SIZE      3

/* Device table array */
extern devsw devtab[];

/**
 * @brief Abstract call to open a device.
 *        Checks for valid device number.
 * 
 * @param {pcb} The process control block calling the function.
 * @param {device_no} The device number to open
 * @returns DEV_FAIL if no device fails to open, a file descriptor for
 *          the device otherwise.
 */
extern int di_open(pcb_t *pcb, int device_no);

/**
 * @brief Abstract call to close a open device.
 * 
 * @param {pcb} The process control block calling the function.
 * @param {fd} The file description the device is open at.
 * @returns DEV_FAIL if fd is wrong, DEV_OK otherwise.
 */
extern int di_close(pcb_t *pcb, int fd);

/**
 * @brief Abstract call to write to a open device.
 * 
 * @param {pcb} The process control block calling the function.
 * @param {fd} The file description the device is open at.
 * @param {buff} The buffer which contains data to write to the device
 * @param {bufflen} The length of the buffer.
 * @returns DEV_FAIL if it fails, the length written to the device otherwise.
 */
extern int di_write(pcb_t *pcb, int fd, void *buff, int bufflen);

/**
 * @brief Abstract call to read from a open device.
 * 
 * @param {pcb} The process control block calling the function.
 * @param {fd} The file description the device is open at.
 * @param {buff} The buffer which contains data to read from the device.
 * @param {bufflen} The length of the buffer.
 * @returns DEV_FAIL if it fails, the length read from the device otherwise.
 */
extern int di_read(pcb_t *pcb, int fd, void *buff, int bufflen);

/**
 * @brief Abstract call to make special control functions to a open device.
 * 
 * @param {pcb} The process control block calling the function.
 * @param {fd} The file description the device is open at.
 * @param {command} The command number to run.
 * @param {ap} The pointer to the start of the argument.
 * @returns DEV_FAIL if it fails, DEV_OK otherwise.
 */
extern int di_ioctl(pcb_t *pcb, int fd, unsigned long command, void *ap);

#endif
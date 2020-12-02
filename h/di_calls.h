/* di_calls.h  */

#ifndef DI_CALLS_H
#define DI_CALLS_H

#include <constants.h>

#define DEV_TABLE_SIZE      3

#define ZERO                0
#define RAND                1
#define KEYBOARD            2

extern devsw devtab[];

extern int di_open(pcb_t *pcb, int device_no);

extern int di_close(pcb_t *pcb, int fd);

extern int di_write(pcb_t *pcb, int fd, void *buff, int bufflen);

extern int di_read(pcb_t *pcb, int fd, void *buff, int bufflen);

extern int di_ioctl(pcb_t *pcb, int fd, unsigned long command, void *ap);

#endif
/* zerorand.h */

#ifndef ZERORAND_H
#define ZERORAND_H

#include <constants.h>

#define RESET_SEED      23

/* zero device functions */

extern void zero_init(devsw *dev);
extern int zero_open(pcb_t *pcb);
extern int zero_close(pcb_t *pcb);
extern int zero_read(pcb_t *pcb, void *buf, int bufflen);
extern int zero_write(pcb_t *pcb, void *buf, int bufflen);
extern int zero_ioctl(pcb_t *pcb, unsigned long command, void *args);

/* rand device functions */

extern void rand_init(devsw *dev);
extern int rand_open(pcb_t *pcb);
extern int rand_close(pcb_t *pcb);
extern int rand_read(pcb_t *pcb, void *buf, int bufflen);
extern int rand_write(pcb_t *pcb, void *buf, int bufflen);
extern int rand_ioctl(pcb_t *pcb, unsigned long command, void *args);

#endif
/* kbd.h */

#ifndef KBD_H
#define KBD_H

#include <constants.h>

/* KBD IRQ number */
#define KBD_IRQ             1

/* The buffer size of the KBD */
#define BUF_LEN             4

/* TRUE/FALSE for KBD functions */
#define KBD_ON              1
#define KBD_OFF             0

/* PORTS to read the KBD from */
#define PORT_READ           0x60
#define PORT_PRESENT        0x64

/* ctrl-d ASCII number */
#define CTRL_D              4

/* kbd struct */
typedef struct kbd_struct {
    int state;
    int echo;
    char eof;
    int eof_flag;
    int buff_ind;
    char buf[BUF_LEN];
    pcb_t *pcb;
} kbd_t;

/* kbd device functions */

extern void kbd_init(devsw *dev);
extern int kbd_open(pcb_t *pcb);
extern int kbd_close(pcb_t *pcb);
extern int kbd_read(pcb_t *pcb, void *buf, int bufflen);
extern int kbd_write(pcb_t *pcb, void *buf, int bufflen);
extern int kbd_ioctl(pcb_t *pcb, unsigned long command, void *args);


#endif
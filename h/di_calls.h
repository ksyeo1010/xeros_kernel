/* di_calls.h  */

#ifndef DI_CALLS_H
#define DI_CALLS_H

extern int di_open(int device_no);

extern int di_close(int fd);

extern int di_write(int fd, void *buff, int bufflen);

extern int di_read(int fd, void *buff, int bufflen);

extern int di_ioctl(int fd, unsigned long command, ...);

#endif
#include <xeroskernel.h>
#include <di_calls.h>
#include <zerorand.h>
#include <kbd.h>
#include <i386.h>

/* device table */
devsw devtab[DEV_TABLE_SIZE];

extern char * maxaddr;

////////////////////////////////////////////////////////////
void devinit() {
    zero_init(&devtab[ZERO]);
    rand_init(&devtab[RAND]);
    kbd_init(&devtab[KEYBOARD]);
}

////////////////////////////////////////////////////////////
int di_open(pcb_t *pcb, int device_no) {
    int fd;
    devsw *dev;

    PRINT("OPEN pid: %d, device no: %d\n", pcb->pid, device_no);

    // check device_no
    if (device_no < 0 || device_no >= DEV_TABLE_SIZE) {
        return DEV_FAIL;
    }

    // check for empty fd
    fd = -1;
    for (int i = 0; i < FD_TABLE_SIZE; i++) {
        if (pcb->fdt[i].dev == NULL) {
            fd = i;
            break;
        }
    }
    if (fd < 0) return DEV_FAIL;

    // check if device could open successfully
    dev = &devtab[device_no];
    if (dev->dvopen(pcb) < 0) {
        return DEV_FAIL;
    }

    // remember ptr to device
    pcb->fdt[fd].dev = dev;
    return fd;
}

////////////////////////////////////////////////////////////
int di_close(pcb_t *pcb, int fd) {
    devsw *dev;

    PRINT("CLOSE pid: %d, fd: %d\n", pcb->pid, fd);

    // check if fd is valid
    if (fd < 0 || fd >= FD_TABLE_SIZE) return DEV_FAIL;

    // check for device 
    dev = pcb->fdt[fd].dev;
    if (dev == NULL) return DEV_FAIL;
    if (dev->dvclose(pcb) < 0) return DEV_FAIL;

    // clean up
    pcb->fdt[fd].dev = NULL;
    return DEV_OK;
}

////////////////////////////////////////////////////////////
int di_write(pcb_t *pcb, int fd, void *buff, int bufflen) {
    devsw *dev;

    PRINT("WRITE pid: %d, fd: %d, bufflen: %d\n", pcb->pid, fd, bufflen);

    // bufflen should not be negative
    if (bufflen < 0) {
        return DEV_FAIL;
    }

    // Check if address is in the hole
    if (((unsigned long) buff) >= HOLESTART && ((unsigned long) buff <= HOLEEND)) 
        return DEV_FAIL;

    //Check if address of the data structure is beyond the end of main memory
    if ((((char * ) buff) + sizeof(processStatuses)) > maxaddr)  
        return DEV_FAIL;

    // check if fd is valid
    if (fd < 0 || fd >= FD_TABLE_SIZE) return DEV_FAIL;

    // check for device 
    dev = pcb->fdt[fd].dev;
    if (dev == NULL) return DEV_FAIL;

    return dev->dvwrite(pcb, buff, bufflen);
}

////////////////////////////////////////////////////////////
int di_read(pcb_t *pcb, int fd, void *buff, int bufflen) {
    devsw *dev;

    PRINT("READ pid: %d, fd: %d, bufflen: %d\n", pcb->pid, fd, bufflen);

    // bufflen should not be negative
    if (bufflen < 0) {
        return DEV_FAIL;
    }

    // Check if address is in the hole
    if (((unsigned long) buff) >= HOLESTART && ((unsigned long) buff <= HOLEEND)) 
        return DEV_FAIL;

    //Check if address of the data structure is beyond the end of main memory
    if ((((char * ) buff) + sizeof(processStatuses)) > maxaddr)  
        return DEV_FAIL;

    // check if fd is valid
    if (fd < 0 || fd >= FD_TABLE_SIZE) return DEV_FAIL;

    // check for device 
    dev = pcb->fdt[fd].dev;
    if (dev == NULL) return DEV_FAIL;

    return dev->dvread(pcb, buff, bufflen);
}

////////////////////////////////////////////////////////////
int di_ioctl(pcb_t *pcb, int fd, unsigned long command, void *ap) {
    devsw *dev;

    PRINT("IOCTL pid: %d, fd: %d, command: %d\n", pcb->pid, fd, command);

    // check if fd is valid
    if (fd < 0 || fd >= FD_TABLE_SIZE) return DEV_FAIL;

    if (ap == NULL) return DEV_FAIL;

     // Check if address is in the hole
    if (((unsigned long) ap) >= HOLESTART && ((unsigned long) ap <= HOLEEND)) 
        return DEV_FAIL;

    //Check if address of the data structure is beyond the end of main memory
    if ((((char * ) ap) + sizeof(processStatuses)) > maxaddr)  
        return DEV_FAIL;

    // check for device 
    dev = pcb->fdt[fd].dev;
    if (dev == NULL) return DEV_FAIL;

    return dev->dvioctl(pcb, command, ap);
}
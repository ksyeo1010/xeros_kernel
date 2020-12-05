#include <xeroskernel.h>
#include <xeroslib.h>
#include <zerorand.h>
#include <stdarg.h>

// Implement the upper and lower half of the device driver code for the zero device here
// The zero device is an infinite stream of 0s. Any read of the device for any number of bytes
// always returns the requested number of bytes, but each byte is 0 (i.e. the bits are all 0s)

////////////////////////////////////////////////////////////
void zero_init(devsw *dev) {
    dev->dvopen = zero_open;
    dev->dvclose = zero_close;
    dev->dvread = zero_read;
    dev->dvwrite = zero_write;
    dev->dvioctl = zero_ioctl;
}

////////////////////////////////////////////////////////////
int zero_open(pcb_t *pcb) {
    return DEV_OK;
}

////////////////////////////////////////////////////////////
int zero_close(pcb_t *pcb) {
    return DEV_OK;
}

////////////////////////////////////////////////////////////
int zero_read(pcb_t *pcb, void *buf, int bufflen) {
    memset(buf, 0, bufflen);
    return bufflen;
}

////////////////////////////////////////////////////////////
int zero_write(pcb_t *pcb, void *buf, int bufflen) {
    return bufflen;
}

////////////////////////////////////////////////////////////
int zero_ioctl(pcb_t *pcb, unsigned long command, void *args) {
    return DEV_FAIL;
}


// Implement the upper and lower half of the device driver code for the random device here 
// The random device is an infinite stream of random bytes. Any read of the device for any number of bytes 
// alwasy returns the requested number of bytes, but each byte is a random value. Use the rand() and srand()
// functions from the xeroslib. The srand() function can be used to set a seed value for the rand() function

////////////////////////////////////////////////////////////
void rand_init(devsw *dev) {
    dev->dvopen = rand_open;
    dev->dvclose = rand_close;
    dev->dvread = rand_read;
    dev->dvwrite = rand_write;
    dev->dvioctl = rand_ioctl;
}

////////////////////////////////////////////////////////////
int rand_open(pcb_t *pcb) {
    return DEV_OK;
}

////////////////////////////////////////////////////////////
int rand_close(pcb_t *pcb) {
    return DEV_OK;
}

////////////////////////////////////////////////////////////
int rand_read(pcb_t *pcb, void *buf, int bufflen) {
    int i, r;
    int *dup = (int *) buf;

    // write 4 bytes
    for (i = 0; i < (bufflen/4); i++) {
        r = rand();
        *(dup + i) = r;
    }

    // write remaining bytes
    if ((bufflen % 4) > 0) {
        r = rand();
        char *dup2 = (char *) (dup + i);
        for (i = 0; i < (bufflen % 4); i++) {
            *(dup2 + i) = (char) (r >> (8*i));
        }
    }

    PRINT("r: %d, rand buf: %d\n", r, *(int *) buf);
    return bufflen;
}

////////////////////////////////////////////////////////////
int rand_write(pcb_t *pcb, void *buf, int bufflen) {
    return DEV_FAIL;
}

////////////////////////////////////////////////////////////
int rand_ioctl(pcb_t *pcb, unsigned long command, void *args) {
    int seed;
    va_list ap;

    if (command != RESET_SEED) return DEV_FAIL;

    ap = (va_list) args;
    seed = va_arg(ap, unsigned int);
    srand(seed);

    return DEV_OK;
}

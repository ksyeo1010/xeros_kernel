#include <xeroskernel.h>
#include <xeroslib.h>
#include <kbd.h>
#include <stdarg.h>
#include <i386.h>

#define KEY_UP 0x80 /* If this bit is on then it is a key   */
                    /* up event instead of a key down event */

/* Control code */
#define LSHIFT 0x2a
#define RSHIFT 0x36
#define LMETA 0x38

#define LCTL 0x1d
#define CAPSL 0x3a

/* scan state flags */
#define INCTL 0x01    /* control key is down          */
#define INSHIFT 0x02  /* shift key is down            */
#define CAPSLOCK 0x04 /* caps lock mode               */
#define INMETA 0x08   /* meta (alt) key is down       */
#define EXTENDED 0x10 /* in extended character mode   */

#define EXTESC 0xe0 /* extended character escape    */
#define NOCHAR 256

static int state; /* the state of the keyboard */

/*  Normal table to translate scan code  */
unsigned char kbcode[] = {0,
                          27, '1', '2', '3', '4', '5', '6', '7', '8', '9',
                          '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't',
                          'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a',
                          's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
                          '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
                          ',', '.', '/', 0, 0, 0, ' '};

/* captialized ascii code table to tranlate scan code */
unsigned char kbshift[] = {0,
                           0, '!', '@', '#', '$', '%', '^', '&', '*', '(',
                           ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T',
                           'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A',
                           'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
                           '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
                           '<', '>', '?', 0, 0, 0, ' '};
/* extended ascii code table to translate scan code */
unsigned char kbctl[] = {0,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 31, 0, '\b', '\t', 17, 23, 5, 18, 20,
                         25, 21, 9, 15, 16, 27, 29, '\n', 0, 1,
                         19, 4, 6, 7, 8, 10, 11, 12, 0, 0,
                         0, 0, 28, 26, 24, 3, 22, 2, 14, 13};

/* kb to ascii function declaration */
unsigned int kbtoa(unsigned char code);

/* enable/disable ireq function declaration */
void enable_irq(unsigned int, int);

/* helper function declaration */
int buf_copy(void *buf, int bufflen);

/* The struct containing current state of device */
static kbd_t kbd;

////////////////////////////////////////////////////////////
void kbd_init(devsw *dev) {
    dev->dvopen = kbd_open;
    dev->dvclose = kbd_close;
    dev->dvread = kbd_read;
    dev->dvwrite = kbd_write;
    dev->dvioctl = kbd_ioctl;

    // initialize kbd state
    memset(&kbd, 0, sizeof(kbd_t));
}

////////////////////////////////////////////////////////////
int kbd_open(pcb_t *pcb) {
    if (kbd.state) return DEV_FAIL;

    // enable irq
    enable_irq(KBD_IRQ, 0);

    // set state open
    kbd.state = KBD_ON;

    // store ptr to process
    kbd.pcb = pcb;

    return DEV_OK;
}

////////////////////////////////////////////////////////////
int kbd_close(pcb_t *pcb) {
    if (!kbd.state) {
        FAIL("Keyboard close called when KBD was off.\n");
        return DEV_FAIL;
    }

    // disable irq
    enable_irq(KBD_IRQ, 1);

    // reset kbd state
    memset(&kbd, 0, sizeof(kbd_t));

    return DEV_OK;
}

////////////////////////////////////////////////////////////
int kbd_read(pcb_t *pcb, void *buf, int bufflen) {
    int ret_len = 0;
    char *dup = (char *) buf;

    if (bufflen < 0) return DEV_FAIL;
    if (bufflen == 0) return ret_len;
    if (kbd.eof_flag && (kbd.buff_ind == 0)) return ret_len;

    ret_len = buf_copy(buf, bufflen);

    PRINT("%s\n", dup);

    // check return condtions
    if (ret_len == bufflen) return ret_len;
    if (ret_len != 0) {
        if (dup[ret_len-1] == '\n') return ret_len;
    }
    if (kbd.eof_flag && (kbd.buff_ind == 0)) return ret_len;

    // save states so we can unblock later
    pcb->state = DEV_BLOCK;
    pcb->buf = buf;
    pcb->bufflen = bufflen;

    return ret_len;
}

////////////////////////////////////////////////////////////
int kbd_write(pcb_t *pcb, void *buf, int bufflen) {
    return DEV_FAIL;
}

////////////////////////////////////////////////////////////
int kbd_ioctl(pcb_t *pcb, unsigned long command, void *args) {
    va_list ap;
    ap = (va_list)args;

    if (!kbd.state) return DEV_FAIL;

    switch (command) {
        case COMMAND_EOF:
            kbd.eof = (char)va_arg(ap, int);
            break;
        case COMMAND_ECHO_ON:
            kbd.echo = KBD_ON;
            break;
        case COMMAND_ECHO_OFF:
            kbd.echo = KBD_OFF;
            break;
        default:
            return DEV_FAIL;
    }

    return DEV_OK;
}

////////////////////////////////////////////////////////////
void kbdint_handler() {
    unsigned char b;
    unsigned char c;

    if (!kbd.state) {
        FAIL("Receiving ints when keyboard was off.\n");
        return;
    }

    b = inb(PORT_PRESENT);
    if (b & 0x1) {
        b = inb(PORT_READ);
        c = kbtoa(b);

        PRINT("c char: %c, c code: %d\n", b, c, c);

        // add to buffer
        if (kbd.buff_ind < BUF_LEN) {
            if (c == CTRL_D) {
                kbd.buf[kbd.buff_ind] = kbd.eof;
                kbd.eof_flag = KBD_ON;
                kbd.buff_ind++;
                enable_irq(KBD_IRQ, 1);
            } else if (c != 0) {
                kbd.buf[kbd.buff_ind] = c;
                kbd.buff_ind++;

                // print if echo is on
                if (kbd.echo) kprintf("%c", c);
            }
        }
    }

    pcb_t *pcb = kbd.pcb;
    if (pcb->state == DEV_BLOCK) {
        int done = 0;
        char *dup = (char *) pcb->buf;

        int ret_len = buf_copy(&dup[pcb->rc], pcb->bufflen - pcb->rc);
        pcb->rc += ret_len;

        PRINT("Buf stats, len: %d, dup: %s\n", pcb->rc, dup);

        if (pcb->rc == 0) return;
        if (pcb->rc == pcb->bufflen) done = 1;
        if (dup[pcb->rc-1] == '\n') done = 1;
        if (kbd.eof_flag && (kbd.buff_ind == 0)) done = 1;

        if (done) {
            ready(pcb);
            PRINT("Ready.\n");
        }
    }
}

/**
 * @brief Helper function to copy from kbd buffer to pointer buffer.
 *        Shifts the kbd after if bufflen is less than the kbd buffer length.
 * 
 * @param {buf} The start of the buffer position to copy data over.
 * @param {bufflen} The amount of data to copy. NOTE: copies max(bufflen, kbd.buff_ind)
 * @returns The total number bytes copied. max(bufflen, kbd.buff_ind)
 */
int buf_copy(void *buf, int bufflen) {
    int i, len = 0;
    char *dup = (char *) buf;

    while ((len<bufflen) && (len<kbd.buff_ind)) {
        *(dup+len) = *(kbd.buf + len);
        len++;
        if (dup[len-1] == '\n') break;
        if ((dup[len-1] == kbd.eof) && kbd.eof_flag) break;
    }

    // shift kbd buf bytes
    int j = 0;
    for (i=len; i < kbd.buff_ind; i++) {
        kbd.buf[j] = kbd.buf[i];
        kbd.buf[i] = NULL;
        j++;
    }
    kbd.buff_ind = j;

    // return num bytes copied
    return len;
}

/*======================================================================*/
/* Start of functions to translate scan code to ascii */
/*======================================================================*/

static int extchar(unsigned char code) {
    return state &= ~EXTENDED;
}

unsigned int kbtoa(unsigned char code) {
    unsigned int ch;

    if (state & EXTENDED) return extchar(code);

    if (code & KEY_UP) {
        switch (code & 0x7f) {
        case LSHIFT:
        case RSHIFT:
            state &= ~INSHIFT;
            break;
        case CAPSL:
            // printf("Capslock off detected\n");
            state &= ~CAPSLOCK;
            break;
        case LCTL:
            state &= ~INCTL;
            break;
        case LMETA:
            state &= ~INMETA;
            break;
        }

        return NOCHAR;
    }

    /* check for special keys */
    switch (code) {
        case LSHIFT:
        case RSHIFT:
            state |= INSHIFT;
            // printf("shift detected!\n");
            return NOCHAR;
        case CAPSL:
            state |= CAPSLOCK;
            // printf("Capslock ON detected!\n");
            return NOCHAR;
        case LCTL:
            state |= INCTL;
            return NOCHAR;
        case LMETA:
            state |= INMETA;
            return NOCHAR;
        case EXTESC:
            state |= EXTENDED;
            return NOCHAR;
    }

    ch = NOCHAR;

    if (code < sizeof(kbcode)) {
        if (state & CAPSLOCK)
            ch = kbshift[code];
        else
            ch = kbcode[code];
    }
    if (state & INSHIFT) {
        if (code >= sizeof(kbshift))
            return NOCHAR;
        if (state & CAPSLOCK)
            ch = kbcode[code];
        else
            ch = kbshift[code];
    }
    if (state & INCTL) {
        if (code >= sizeof(kbctl))
            return NOCHAR;
        ch = kbctl[code];
    }
    if (state & INMETA)
        ch += 0x80;
    return ch;
}
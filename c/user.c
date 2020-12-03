/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

#define USER "cs415"
#define PASSWORD "pw"

#define BUFFERSIZE 128

#define KBD_EOF     '\0'
#define DETACH      '&'

#define P_CMD       "p"
#define X_CMD       "x"
#define K_CMD       'k'
#define A_CMD       'a'
#define T_CMD       't'

/* commands */

void p(void);
void x(void);
void k(void);
void a(void);
void t(void);

/* command helper */

void mapstate(int state, char *s);

/**
 * @brief The shell function of our application.
 *        Runs various commands indefinitely until "x" or CTRL-D
 *        is given.
 */
void shell(void) {
    int fd, buf_len, detach, pid, close;
    char buf[BUFFERSIZE];

    for (;;) {
        // reset state
        buf_len = 0;
        detach = 0;
        close = 0;
        memset(buf, 0, sizeof(buf));

        sysputs("> ");
        fd = sysopen(KEYBOARD);
        sysioctl(fd, COMMAND_ECHO_ON);

        buf_len += sysread(fd, &buf[buf_len], BUFFERSIZE);
        kprintf("buf stats, len: %d, buf: %s", buf_len, buf);

        // either \n or \0 case
        if (buf_len == 1) {
            if (buf[buf_len-1] == '\n') {
                sysclose(fd);
                continue;
            }
            if (buf[buf_len-1] == KBD_EOF) {
                sysputs("\n");
                sysclose(fd);
                return;
            }
        }

        if (buf[buf_len-2] == DETACH) detach = 1;

        if (buf[buf_len-1] == KBD_EOF) {
            sysputs("\n");
            close = 1;
        }

        // x command
        if ((strncmp(X_CMD, buf, buf_len-1-detach) == 0)) {
            sysclose(fd);
            return;
        }

        // p command
        if ((strncmp(P_CMD, buf, buf_len-1-detach)) == 0) {
            pid = syscreate(p, STACKSIZE);
            if (!detach) syswait(pid);
        }

        sysclose(fd);
        if (close) return;
    }
}

////////////////////////////////////////////////////////////
void root(void) {
    int fd, pid;

    int user_len;
    char user_buf[BUFFERSIZE];
    
    int pw_len;
    char pw_buf[BUFFERSIZE];

    sysputs("Welcome to Xeros 415 - A not very secure Kernel.\n");
    
    for (;;) {
        // reset state
        memset(user_buf, 0, sizeof(user_buf));
        memset(pw_buf, 0, sizeof(pw_buf));
        user_len = 0;
        pw_len = 0;

        sysputs("Username: ");
        fd = sysopen(KEYBOARD);
        sysioctl(fd, COMMAND_ECHO_ON);

        // read until we get \n
        while(user_len < BUFFERSIZE) {
            user_len += sysread(fd, &user_buf[user_len], 4);
            if (user_buf[user_len-1] == '\n') {
                break;
            }
        }

        sysioctl(fd, COMMAND_ECHO_OFF);
        sysputs("Password: ");

        // read until we get \n
        while(pw_len < BUFFERSIZE) {
            pw_len += sysread(fd, &pw_buf[pw_len], 4);
            if (pw_buf[pw_len-1] == '\n') {
                break;
            }
        }

        // verify
        if ((strncmp(USER, user_buf, user_len-1) != 0) || (strncmp(PASSWORD, pw_buf, pw_len-1) != 0)) {
            sysputs("Invalid credentials.\n");
            sysclose(fd);
            continue;
        }

        sysclose(fd);
        sysputs("\n");
        pid = syscreate(shell, STACKSIZE);
        syswait(pid);
    }
}

void p(void) {
    int procs;
    processStatuses psTab;
    char buf[128];
    char status[64];

    procs = sysgetcputimes(&psTab);
    sprintf(buf, "%4s    %10s    %10s\n", "pid", "state", "time (ms)");
    kprintf(buf);
    for (int i = 0; i <= procs; i++) {
        mapstate(psTab.status[i], status);
        sprintf(buf, "%4d    %10s    %10d\n", psTab.pid[i], status, 
	        psTab.cpuTime[i]);
        kprintf(buf);
    }
}

void x(void) {

}

void k(void) {

}

void a(void) {

}

void t(void) {

}

void mapstate(int state, char *s) {
    switch(state) {
        case STATE_RUNNING:
            sprintf(s, "%s", "RUNNING");
            break;
        case STATE_READY:
            sprintf(s, "%s", "READY");
            break;
        case DEV_BLOCK:
        case STATE_BLOCKED:
            sprintf(s, "%s", "BLOCKED");
            break;
        case STATE_WAITING:
            sprintf(s, "%s", "WAITING");
            break;
        case STATE_SLEEP:
            sprintf(s, "%s", "SLEEPING");
            break;
        default:
            sprintf(s, "%s", "UNKNOWN");
    }
}
/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>


/* user and pass */
#define USER            "cs415"
#define PASSWORD        "pw"

/* shell's max buffer size */
#define BUFFERSIZE      128

/* list of default commands in STRING not CHAR */
#define P_CMD           "p"
#define X_CMD           "x"
#define K_CMD           "k"
#define A_CMD           "a"
#define T_CMD           "t"

/* The detach character */
#define DETACH          '&'

/* States of the tokenizer */
#define TOK_NORMAL      0
#define TOK_ENTER       1
#define TOK_EOF         2

/* tok structure */
typedef struct tok_struct {
    int ind;
    int state;
    int detach;
    char *dup;
    char str[BUFFERSIZE];
} tok_t;

/* commands */

void p(void);
void k(char *p_str);
void a(void);
void t(void);

/* sig21 */

void sig21(void);

/* command helpers */

void mapstate(int state, char *s);
int token(char *s, tok_t *tok);

/* some global variables */

static char kbd_eof = '\0';
static unsigned int milliseconds;
static unsigned int shellpid;

/**
 * @brief The shell function of our application.
 *        Runs various commands indefinitely until "x" or CTRL-D
 *        is given.
 */
void shell(void) {
    int fd, buf_len, len;
    char buf[BUFFERSIZE];
    tok_t tok;
    unsigned int pid;

    for (;;) {
        buf_len = 0;
        memset(buf, 0, sizeof(buf));

        sysputs("> ");
        fd = sysopen(KEYBOARD);
        sysioctl(fd, COMMAND_ECHO_ON);

        buf_len = sysread(fd, &buf[buf_len], BUFFERSIZE);
        if (buf_len < 1) {
            // that means something occurred while reading
            // either it failed or was interrupted by a signal before return buffer was filled
            // based on our model we should have gotten at least size of 1
            sysclose(fd);
            continue;
        }
        if (buf_len == BUFFERSIZE) {
            // we filled up out buffersize causing a ret from read.
            // just print command too long and retake command.
            sysputs("\nCommand too long.\n");
            sysclose(fd);
            continue;
        }
        if ((buf[buf_len-1] != kbd_eof) && (buf[buf_len-1] != '\n')) {
            // if buffer doesn't actually end with \n or \0 then that means
            // the read to the device got interrupted. For sake of not triggering
            // an eof, we just reset the shell.
            sysclose(fd);
            continue;
        }

        len = token(buf, &tok);

        // kprintf("buf stats, len: %d, buf: %s", buf_len, buf);
        // kprintf("tok stats len: %d, ind: %d, state: %d, detach: %d, str: %s\n", len, tok.ind, tok.state, tok.detach, tok.str);

        if (tok.state == TOK_EOF) {
            kprintf("\n");
        }

        // initial cases
        if (len > 0) {
            if (strcmp(X_CMD, tok.str) == 0) { 
                // x command
                sysclose(fd);
                return;
            } else if (strcmp(P_CMD, tok.str) == 0) {
                // p command
                p();
            } else if (strcmp(K_CMD, tok.str) == 0) {
                // k command
                token(NULL, &tok);
                if ((tok.state == TOK_ENTER) || (tok.state == TOK_EOF)) {
                    k(tok.str);
                } else {
                    sysputs("Wrong number of paramters.\n");
                }
            } else if (strcmp(A_CMD, tok.str) == 0) {
                // a command
                token(NULL, &tok);
                if ((tok.state == TOK_ENTER) || (tok.state == TOK_EOF)) {
                    shellpid = sysgetpid();
                    milliseconds = atoi(tok.str);
                    syssignal(21, (sighandler_t) sig21);
                    pid = syscreate(a, STACKSIZE);
                    if (!tok.detach) syswait(pid);
                } else {
                    sysputs("Wrong number of parameters.\n");
                }
            } else if (strcmp(T_CMD, tok.str) == 0) {
                // t command
                pid = syscreate(t, STACKSIZE);
                if (!tok.detach) syswait(pid);
            } else {
                sysputs("Invalid command specified.\n");
            }
        }

        sysclose(fd);
        if (tok.state == TOK_EOF) {
            return;
        }
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
            if ((user_buf[user_len-1] == '\n') || (user_buf[user_len-1] == kbd_eof)) {
                break;
            }
        }

        if (user_buf[user_len-1] == kbd_eof) {
            sysclose(fd);
            sysputs("\n");
            continue;
        }

        sysioctl(fd, COMMAND_ECHO_OFF);
        sysputs("Password: ");

        // read until we get \n
        while(pw_len < BUFFERSIZE) {
            pw_len += sysread(fd, &pw_buf[pw_len], 4);
            if ((pw_buf[pw_len-1] == '\n') || (pw_buf[pw_len-1] == kbd_eof)) {
                break;
            }
        }
        if (pw_buf[pw_len-1] == kbd_eof) {
            sysclose(fd);
            sysputs("\n");
            continue;
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

/**
 * @brief Prints the process pid, state and cputime consumed.
 *        This is not spawned by a system call.
 */
void p(void) {
    int procs;
    processStatuses psTab;
    char buf[128];
    char status[64];

    procs = sysgetcputimes(&psTab);
    sprintf(buf, "%10s    %10s    %10s\n", "pid", "state", "time (ms)");
    kprintf(buf);
    for (int i = 0; i <= procs; i++) {
        mapstate(psTab.status[i], status);
        sprintf(buf, "%10d    %10s    %10d\n", psTab.pid[i], status, 
	        psTab.cpuTime[i]);
        kprintf(buf);
    }
}

/**
 * @brief Calls the syskill with signum 31 (which kills the process).
 * 
 * @param {p_str} The character having the pid.
 */
void k(char *p_str) {
    int ret;
    unsigned int pid = atoi(p_str);
    ret = syskill(pid, 31);
    if (ret == -999) {
        sysputs("No such process.\n");
    }
}

/**
 * @brief The a command for the shell. Sleeps and then calls SIG 21.
 *        This is created by a syscreate.
 */
void a(void) {
    syssleep(milliseconds);
    syskill(shellpid, 21);
}

/**
 * @brief The t command for the shell.
 *        Prints its pid and prints T every 10 seconds.
 */
void t(void) {
    unsigned int pid;
    char buf[BUFFERSIZE];

    pid = sysgetpid();
    sprintf(buf, "My pid is %d.\n", pid);
    sysputs(buf);

    for (;;) {
        sysputs("T\n");
        syssleep(10000);
    }
}

/**
 * @brief The sig 21 implementation needed by the shell process.
 *        Just prints `SIG21 SIG21` and then removes the signal.
 */
void sig21(void) {
    sysputs("\nSIG21 SIG21\n");
    syssignal(21, NULL);
}

/**
 * @brief Helper function mapping state(in numbers) to actual string representation.
 * 
 * @param {state} The state in integer.
 * @param {s} The pointer to the character array to write the state into.
 */
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

/**
 * @brief The tokenizer for the shell process. Each call will give a set of string
 *        without any of these: ' ', '\t', '\n', 'kbd_eof'. If the parameter 's' is not NULL
 *        we start tokenizing from the given char array. If the parameter 's' is NULL we get the next
 *        token until we reach a TOK_ENTER or TOK_EOF state. Modifies the tok state accordingly.
 *        NOTE: The variable use `tok.str` will give the next token. `tok.state` gives the token state.
 *              Everything else should not be important and only used for this function.
 * 
 * @param {s} The character array to start tokenizing.
 * @param {tok} The token state.
 * @returns The length of the token.
 */
int token(char *s, tok_t *tok) {
    int len = 0;
    int space = 0;
    if (s != NULL) {
        // refreshing token
        memset(tok, 0, sizeof(tok_t));
        tok->dup = s;
    } else {
        memset(tok->str, 0, sizeof(tok->str));
    }
    // if there was a \n or \0, then we know there is no more
    if (tok->state != TOK_NORMAL) {
        return len;
    }
    // limit how many times we loop in case there's something wrong.
    for (int i = 0; i < sizeof(tok->str); i++) {
        if ((tok->dup[tok->ind] == '\t') || (tok->dup[tok->ind] == ' ')) {
            do {
                tok->ind++;
            } while ((tok->dup[tok->ind] == '\t') || (tok->dup[tok->ind] == ' '));
            space = 1;
        }
        if (tok->dup[tok->ind] == '\n') {
            tok->ind++;
            tok->state = TOK_ENTER;
            break;
        }
        if (tok->dup[tok->ind] == kbd_eof) {
            tok->ind++;
            tok->state = TOK_EOF;
            break;
        }
        if (tok->dup[tok->ind] == DETACH) {
            // only ignore if its last
            tok->ind++;
            tok->detach = 1;
            continue;
        }
        if (space) break;
        // set detach to 0, we know theres something else after &
        if (tok->detach) tok->detach = 0;
        tok->str[len] = tok->dup[tok->ind];
        tok->ind++;
        len++;
    }

    return len;
}
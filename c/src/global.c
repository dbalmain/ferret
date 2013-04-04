#include "global.h"
#include "symbol.h"
#include "hash.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "internal.h"

const char *EMPTY_STRING = "";

bool  x_do_logging = false;
bool  x_abort_on_exception = true;
bool  x_has_aborted = false;
FILE *x_exception_stream = NULL;

INLINE int min3(int a, int b, int c)
{
    return MIN3(a, b, c);
}

INLINE int min2(int a, int b)
{
    return MIN(a, b);
}

INLINE int max3(int a, int b, int c)
{
    return MAX3(a, b, c);
}

INLINE int max2(int a, int b)
{
    return MAX(a, b);
}

int scmp(const void *p1, const void *p2)
{
    return strcmp(*(char **) p1, *(char **) p2);
}

void frt_strsort(char **str_array, int size)
{
    qsort(str_array, size, sizeof(char *), &scmp);
}

int icmp(const void *p1, const void *p2)
{
    int i1 = *(int *) p1;
    int i2 = *(int *) p2;

    if (i1 > i2) {
        return 1;
    }
    else if (i1 < i2) {
        return -1;
    }
    return 0;
}

int icmp_risky(const void *p1, const void *p2)
{
  return (*(int *)p1) - *((int *)p2);
}

unsigned int *imalloc(unsigned int value)
{
  unsigned int *p = ALLOC(unsigned int);
  *p = value;
  return p;
}

unsigned long *lmalloc(unsigned long value)
{
  unsigned long *p = ALLOC(unsigned long);
  *p = value;
  return p;
}

u32 *u32malloc(u32 value)
{
  u32 *p = ALLOC(u32);
  *p = value;
  return p;
}

u64 *u64malloc(u64 value)
{
  u64 *p = ALLOC(u64);
  *p = value;
  return p;
}

/* concatenate two strings freeing the second */
char *estrcat(char *str1, char *str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    REALLOC_N(str1, char, len1 + len2 + 3);     /* leave room for <CR> */
    memcpy(str1 + len1, str2, len2 + 1);        /* make sure '\0' copied too */
    free(str2);
    return str1;
}

/* epstrdup: duplicate a string with a format, report if error */
char *epstrdup(const char *fmt, int len, ...)
{
    char *string;
    va_list args;
    len += (int) strlen(fmt);

    string = ALLOC_N(char, len + 1);
    va_start(args, len);
    vsprintf(string, fmt, args);
    va_end(args);

    return string;
}

/* estrdup: duplicate a string, report if error */
char *estrdup(const char *s)
{
    char *t = ALLOC_N(char, strlen(s) + 1);
    strcpy(t, s);
    return t;
}

/* Pretty print a float to the buffer. The buffer should have at least 32
 * bytes available.
 */
char *dbl_to_s(char *buf, double num)
{
    char *p, *e;

#ifdef FRT_IS_C99
    if (isinf(num)) {
        return estrdup(num < 0 ? "-Infinity" : "Infinity");
    }
    else if (isnan(num)) {
        return estrdup("NaN");
    }
#endif

    sprintf(buf, DBL2S, num);
    if (!(e = strchr(buf, 'e'))) {
        e = buf + strlen(buf);
    }
    if (!isdigit(e[-1])) {
        /* reformat if ended with decimal point (ex 111111111111111.) */
        sprintf(buf, "%#.6e", num);
        if (!(e = strchr(buf, 'e'))) { e = buf + strlen(buf); }
    }
    p = e;
    while (p[-1] == '0' && isdigit(p[-2])) {
        p--;
    }

    memmove(p, e, strlen(e) + 1);
    return buf;
}

/* strfmt: like sprintf except that it allocates memory for the string */
char *vstrfmt(const char *fmt, va_list args)
{
    char *string;
    char *p = (char *) fmt, *q;
    int len = (int) strlen(fmt) + 1;
    int slen, curlen;
    char *s;
    long l;
    double d;

    q = string = ALLOC_N(char, len);

    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
            case 's':
                p++;
                s = va_arg(args, char *);
                /* to be consistent with printf print (null) for NULL */
                if (!s) {
                    s = "(null)";
                }
                slen = (int) strlen(s);
                len += slen;
                curlen = q - string;
                REALLOC_N(string, char, len);
                q = string + curlen;
                memcpy(q, s, slen);
                q += slen;
                continue;
            case 'f':
                p++;
                len += 32;
                *q = 0;
                REALLOC_N(string, char, len);
                q = string + strlen(string);
                d = va_arg(args, double);
                dbl_to_s(q, d);
                q += strlen(q);
                continue;
            case 'd':
                p++;
                len += 20;
                *q = 0;
                REALLOC_N(string, char, len);
                q = string + strlen(string);
                l = va_arg(args, long);
                q += sprintf(q, "%ld", l);
                continue;
            default:
                break;
            }
        }
        *q = *p;
        p++;
        q++;
    }
    *q = 0;

    return string;
}

char *strfmt(const char *fmt, ...)
{
    va_list args;
    char *str;
    va_start(args, fmt);
    str = vstrfmt(fmt, args);
    va_end(args);
    return str;
}

void dummy_free(void *p)
{
    (void)p; /* suppress unused argument warning */
}

#ifdef HAVE_GDB
#define CMD_BUF_SIZE (128 + FILENAME_MAX)
/* need to declare this as it is masked by default in linux */

static char *build_shell_command(const char *gdb_filename)
{
    int   pid = getpid();
    char *buf = ALLOC_N(char, CMD_BUF_SIZE);
    char *command =
        "gdb -quiet -command=%s %s %d 2>/dev/null | grep '^[ #]'";

    snprintf(buf, CMD_BUF_SIZE, command, gdb_filename, progname(), pid);
    return buf;
}

/* Returns the fd to the tempfile */
static int build_tempfile(char *name, size_t max_size)
{
    char *tmpdir = getenv("TMPDIR");

    snprintf(name, max_size, "%s/frt.XXXXXXXXXX", tmpdir ? tmpdir : "/tmp");
    return mkstemp(name);
}

static char *build_gdb_commandfile()
{
    const char *commands = "bt\nquit\n";
    char *filename = ALLOC_N(char, FILENAME_MAX);
    int fd = build_tempfile(filename, FILENAME_MAX);
    if (fd < 0) { return NULL; }
    write(fd, commands, strlen(commands));
    close(fd);
    return filename;
}
#endif

/**
 * Call out to gdb to get our stacktrace.
 */
char *get_stacktrace()
{
#ifdef HAVE_GDB
    FILE *stream;
    char *gdb_filename = NULL, *buf = NULL, *stack = NULL;
    int   offset = -BUFFER_SIZE;

    if ( !(gdb_filename = build_gdb_commandfile()) ) {
        fprintf(EXCEPTION_STREAM,
                "Unable to build gdb command file\n");
        goto cleanup;
    }
    if ( !(buf = build_shell_command(gdb_filename)) ) {
        fprintf(EXCEPTION_STREAM,
                "Unable to build stacktrace shell command\n");
        goto cleanup;
    }

    if ( !(stream = popen(buf, "r")) ) {
        fprintf(EXCEPTION_STREAM,
                "Unable to exec stacktrace shell command: '%s'\n", buf);
        goto cleanup;
    }

    do {
        offset += BUFFER_SIZE;
        REALLOC_N(stack, char, offset + BUFFER_SIZE);
        ZEROSET_N(stack + offset, char, BUFFER_SIZE);
    } while(fread(stack + offset, 1, BUFFER_SIZE, stream) == BUFFER_SIZE);

    pclose(stream);

 cleanup:
    if (gdb_filename) free(gdb_filename);
    if (buf) free(buf);
    return stack;
#else
    return NULL;
#endif
}

void print_stacktrace()
{
    char *stack = get_stacktrace();

    fprintf(EXCEPTION_STREAM, "Stack trace:\n%s",
            stack ? stack : "Not available\n");
    if (stack) free(stack);
}

typedef struct FreeMe
{
    void *p;
    free_ft free_func;
} FreeMe;

static FreeMe *free_mes = NULL;
static int free_mes_size = 0;
static int free_mes_capa = 0;

void register_for_cleanup(void *p, free_ft free_func)
{
    FreeMe *free_me;
    if (free_mes_capa == 0) {
        free_mes_capa = 16;
        free_mes = ALLOC_N(FreeMe, free_mes_capa);
    }
    else if (free_mes_capa <= free_mes_size) {
        free_mes_capa *= 2;
        REALLOC_N(free_mes, FreeMe, free_mes_capa);
    }
    free_me = free_mes + free_mes_size++;
    free_me->p = p;
    free_me->free_func = free_func;
}

void clean_up()
{
    int i;
    for (i = 0; i < free_mes_size; i++) {
        FreeMe *free_me = free_mes + i;
        free_me->free_func(free_me->p);
    }
    free(free_mes);
    free_mes = NULL;
    free_mes_size = free_mes_capa = 0;
}

/* weprintf: print error message and don't exit */
void weprintf(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);
    fprintf(stderr, "%s: ", progname());

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':')
        fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");
}

#define MAX_PROG_NAME 200
static char name[MAX_PROG_NAME]; /* program name for error msgs */

/* setprogname: set stored name of program */
void setprogname(const char *str)
{
    strncpy(name, str, sizeof(name) - 1);
}

const char *progname()
{
    return name;
}

static const char *signal_to_string(int signum)
{
    switch (signum)
    {
        case SIGILL:  return "SIGILL";
        case SIGABRT: return "SIGABRT";
        case SIGFPE:  return "SIGFPE";
        case SIGBUS:  return "SIGBUS";
        case SIGSEGV: return "SIGSEGV";
    }

    return "Unknown Signal";
}

static void sighandler_crash(int signum)
{
    print_stacktrace();
    XEXIT("Signal", "Exiting on signal %s (%d)",
             signal_to_string(signum), signum);
}

#define SETSIG_IF_UNSET(sig, new) do {  \
    struct sigaction __old;             \
    sigaction(sig, NULL, &__old);       \
    if (__old.sa_handler != SIG_IGN) {  \
        sigaction(sig, &new, NULL);     \
    }                                   \
} while(0)

void init(int argc, const char *const argv[])
{
    struct sigaction action;

    if (argc > 0) {
        setprogname(argv[0]);
    }

    action.sa_handler = sighandler_crash;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    SETSIG_IF_UNSET(SIGILL , action);
    SETSIG_IF_UNSET(SIGABRT, action);
    SETSIG_IF_UNSET(SIGFPE , action);
    SETSIG_IF_UNSET(SIGBUS , action);
    SETSIG_IF_UNSET(SIGSEGV, action);

    symbol_init();

    atexit(&hash_finalize);
}

/**
 * For general use when testing
 *
 * TODO wrap in #ifdef
 */

static bool p_switch = false;
static bool p_switch_tmp = false;

void p(const char *format, ...)
{
    va_list args;

    if (!p_switch) return;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void p_on()
{
    fprintf(stderr, "> > > > > STARTING PRINT\n");
    p_switch = true;
}

void p_off()
{
    fprintf(stderr, "< < < < < STOPPING PRINT\n");
    p_switch = false;
}

void p_pause()
{
    p_switch_tmp = p_switch;
    p_switch = false;
}

void p_resume()
{
    p_switch = p_switch_tmp;
}

#include "global.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

const char *EMPTY_STRING = "";

int min3(int a, int b, int c)
{
    return MIN3(a, b, c);
}

int min2(int a, int b)
{
    return MIN(a, b);
}

int max3(int a, int b, int c)
{
    return MAX3(a, b, c);
}

int max2(int a, int b)
{
    return MAX(a, b);
}

int scmp(const void *p1, const void *p2)
{
    return strcmp(*(char **) p1, *(char **) p2);
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

/* raise: print error message and exit */
#ifdef FRT_HAS_VARARGS
void raise_pos(char *file, int line_num, const char *func,
               const char *etype, const char *fmt, ...)
#else
extern void raise(const char *etype, const char *fmt, ...)
#endif
{
    va_list args;

    fflush(stdout);
    fprintf(stderr, "\n");
    if (progname() != NULL) {
        fprintf(stderr, "%s: ", progname());
    }

#ifdef FRT_HAS_VARARGS
    fprintf(stderr, "%s occured at <%s>:%d in %s\n",
            etype, file, line_num, func);
#else
    fprintf(stderr, "%s occured:\n", etype);
#endif
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':') {
        fprintf(stderr, " %s", strerror(errno));
    }

    fprintf(stderr, "\n");
    exit(2);                    /* conventional value for failed execution */
}

/* weprintf: print error message and don't exit */
void weprintf(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);
    if (progname() != NULL)
        fprintf(stderr, "%s: ", progname());

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':')
        fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");
}

static char name[200];          /* program name for error msgs */

/* setprogname: set stored name of program */
void setprogname(const char *str)
{
    strcpy(name, str);
}

char *progname()
{
    return name;
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
    char *t = (char *) malloc(strlen(s) + 1);

    if (t == NULL) {
        raise(MEM_ERROR, "Memory allocation error");
    }

    strcpy(t, s);
    return t;
}

/* emalloc: malloc and report if error */
void *emalloc(size_t size)
{
    void *p = malloc(size);

    if (p == NULL) {
        raise(MEM_ERROR, "malloc failed");
    }

    return p;
}

/* erealloc: realloc and report if error */
void *erealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);

    if (p == NULL) {
        raise(MEM_ERROR, "malloc failed");
    }

    return p;
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

    sprintf(buf, "%#.7g", num);
    if (!(e = strchr(buf, 'e'))) {
        e = buf + strlen(buf);
    }
    if (!isdigit(e[-1])) {
        /* reformat if ended with decimal point (ex 111111111111111.) */
        sprintf(buf, "%#.6e", num);
        if (!(e = strchr(buf, 'e'))) {
            e = buf + strlen(buf);
        }
    }
    p = e;
    while (p[-1] == '0' && isdigit(p[-2])) {
        p--;
    }

    memmove(p, e, strlen(e) + 1);
    return buf;
}

/* strfmt: like sprintf except that it allocates memory for the string */
char *strfmt(const char *fmt, ...)
{
    char *string;
    char *p = (char *) fmt, *q;
    va_list args;
    int len = (int) strlen(fmt) + 1;
    int slen;
    char *s;
    long i;
    double d;

    q = string = ALLOC_N(char, len);

    va_start(args, fmt);
    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
            case 's':
                p++;
                s = va_arg(args, char *);
                if (s) {
                    slen = (int) strlen(s);
                    len += slen;
                    *q = 0;
                    REALLOC_N(string, char, len);
                    q = string + strlen(string);
                    sprintf(q, s);
                    q += slen;
                }
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
                i = va_arg(args, long);
                sprintf(q, "%ld", i);
                q += strlen(q);
                continue;
            default:
                break;
            }
        }
        *q = *p;
        p++;
        q++;
    }
    va_end(args);
    *q = 0;

    return string;
}

void dummy_free(void *p)
{
    (void)p; /* suppress unused argument warning */
}

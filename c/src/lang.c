#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lang.h"
#include "except.h"
#include "global.h"


/* frt_malloc: malloc and report if error */
extern void *frt_malloc(size_t size)
{
    void *p = malloc(size);

    if (p == NULL) {
        RAISE(MEM_ERROR, "failed to allocate %d bytes", (int)size);
    }

    return p;
}

/* frt_calloc: malloc, zeroset and report if error */
extern void *frt_calloc(size_t size)
{
    void *p = calloc(1, size);

    if (p == NULL) {
        RAISE(MEM_ERROR, "failed to allocate %d bytes", (int)size);
    }

    return p;
}

/* frt_realloc: realloc and report if error */
extern void *frt_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);

    if (p == NULL) {
        RAISE(MEM_ERROR, "failed to reallocate %d bytes", (int)size);
    }

    return p;
}

#ifdef FRT_IS_C99
extern void usleep(unsigned long usec);
extern int unlink(const char *path);
#else
#  include <unistd.h>
#endif

extern void frt_micro_sleep(const int micro_seconds)
{
#ifdef POSH_OS_WIN32
    Sleep(micro_seconds / 1000);
#else
    usleep(micro_seconds);
#endif
}

/* frt_exit: print error message and exit */
# ifdef FRT_HAS_VARARGS
extern void frt_vexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args)
# else
extern void FRT_VEXIT(const char *err_type, const char *fmt, va_list args)
# endif
{
    fflush(stdout);
    fprintf(EXCEPTION_STREAM, "\n%s: ", frt_progname());

# ifdef FRT_HAS_VARARGS
    fprintf(EXCEPTION_STREAM, "%s occured at <%s>:%d in %s\n",
            err_type, file, line_num, func);
# else
    fprintf(EXCEPTION_STREAM, "%s occured:\n", err_type);
# endif
    vfprintf(EXCEPTION_STREAM, fmt, args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':') {
        fprintf(EXCEPTION_STREAM, " %s", strerror(errno));
    }

    fprintf(EXCEPTION_STREAM, "\n");
    if (x_abort_on_exception) {
        exit(2);                 /* conventional value for failed execution */
    }
    else {
        x_has_aborted = true;
    }
}


# ifdef FRT_HAS_VARARGS
extern void frt_exit(const char *file, int line_num, const char *func,
              const char *err_type, const char *fmt, ...)
# else
extern void FRT_EXIT(const char *err_type, const char *fmt, ...)
# endif
{
    va_list args;
    va_start(args, fmt);
# ifdef FRT_HAS_VARARGS
    frt_vexit(file, line_num, func, err_type, fmt, args);
# else
    FRT_VEXIT(err_type, fmt, args);
# endif
    va_end(args);
}

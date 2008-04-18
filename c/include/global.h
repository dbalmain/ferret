#ifndef FRT_GLOBAL_H
#define FRT_GLOBAL_H

#include "config.h"
#include "except.h"
#include "lang.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define FRT_MAX_WORD_SIZE 255
#define FRT_MAX_FILE_PATH 1024

#if defined(__GNUC__)
# define FRT_INLINE __inline__
#else
# define FRT_INLINE
#endif

typedef void (*frt_free_ft)(void *key);

#define FRT_NELEMS(array) ((int)(sizeof(array)/sizeof(array[0])))


#define FRT_ZEROSET(ptr, type) memset(ptr, 0, sizeof(type))
#define FRT_ZEROSET_N(ptr, type, n) memset(ptr, 0, sizeof(type)*(n))

#define FRT_ALLOC_AND_ZERO(type) (type*)frt_ecalloc(sizeof(type))
#define FRT_ALLOC_AND_ZERO_N(type,n) (type*)frt_ecalloc(sizeof(type)*(n))

#define FRT_REF(a) (a)->ref_cnt++
#define FRT_DEREF(a) (a)->ref_cnt--

#define FRT_NEXT_NUM(index, size) (((index) + 1) % (size))
#define FRT_PREV_NUM(index, size) (((index) + (size) - 1) % (size))

#define FRT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FRT_MAX(a, b) ((a) > (b) ? (a) : (b))

#define FRT_MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define FRT_MAX3(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

#define FRT_ABS(n) ((n >= 0) ? n : -n)

#define FRT_RECAPA(self, len, capa, ptr, type) \
  do {\
    if (self->len >= self->capa) {\
      if (self->capa > 0) {\
        self->capa <<= 1;\
      } else {\
        self->capa = 4;\
      }\
      self->ptr = frt_erealloc(self->ptr, sizeof(type) * self->capa);\
    }\
  } while (0)

#ifdef POSH_OS_WIN32
# define Jx fprintf(stderr,"%s, %d\n", __FILE__, __LINE__);
# define Xj fprintf(stdout,"%s, %d\n", __FILE__, __LINE__);
#else
# define Jx fprintf(stderr,"%s, %d: %s\n", __FILE__, __LINE__, __func__);
# define Xj fprintf(stdout,"%s, %d: %s\n", __FILE__, __LINE__, __func__);
#endif

extern unsigned int *frt_imalloc(unsigned int value);
extern unsigned long *frt_lmalloc(unsigned long value);
extern f_u32 *frt_u32malloc(f_u32 value);
extern f_u64 *frt_u64malloc(f_u64 value);

extern char *frt_estrdup(const char *s);
extern char *frt_estrcat(char *str, char *str_cat);
extern void frt_weprintf(const char *fmt, ...);
extern char *frt_epstrdup(const char *fmt, int len, ...);

extern const char *FRT_EMPTY_STRING;

extern int frt_scmp(const void *p1, const void *p2);
extern int frt_icmp(const void *p1, const void *p2);
extern int frt_icmp_risky(const void *p1, const void *p2);

extern int frt_min2(int a, int b);
extern int frt_min3(int a, int b, int c);
extern int frt_max2(int a, int b);
extern int frt_max3(int a, int b, int c);

extern char *frt_dbl_to_s(char *buf, double num);
extern char *frt_strfmt(const char *fmt, ...);
extern char *frt_vstrfmt(const char *fmt, va_list args);

extern char *frt_get_stacktrace();
extern void  frt_print_stacktrace();

extern void frt_register_for_cleanup(void *p, frt_free_ft free_func);
extern void frt_do_clean_up();

/**
 * A dummy function which can be passed to functions which expect a free
 * function such as h_new() if you don't want the free functions to do anything.
 * This function will do nothing.
 *
 * @param p the object which this function will be called on.
 */
extern void frt_dummy_free(void *p);

/**
 * For coverage, we don't want FRT_XEXIT to actually exit on uncaught
 * exceptions.  +frt_x_abort_on_exception+ is +true+ by default, set it to
 * +false+, and +frt_x_has_aborted+ will be set as appropriate.  We also
 * don't want spurious errors to be printed out to stderr, so we give
 * the option to set where errors go to with +frt_x_exception_stream+.
 */

extern bool  frt_x_abort_on_exception;
extern bool  frt_x_has_aborted;
extern FILE *frt_x_exception_stream;

/**
 * The convenience macro +EXCEPTION_STREAM+ returns stderr when
 * +frt_x_exception_stream+ isn't explicitely set.
 */
#define EXCEPTION_STREAM (frt_x_exception_stream ? frt_x_exception_stream : stderr)

#ifdef DEBUG
extern bool frt_x_do_logging;
#define xlog if (frt_x_do_logging) printf
#else
#define xlog()
#endif

extern void frt_init(int arc, const char *const argv[]);
extern void frt_setprogname(const char *str);
extern const char *frt_progname();
extern void frt_micro_sleep(const int micro_seconds);
extern void frt_clean_up();
#endif

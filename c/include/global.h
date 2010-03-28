#ifndef FRT_GLOBAL_H
#define FRT_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "except.h"
#include "lang.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#define FRT_MAX_WORD_SIZE 255
#define FRT_MAX_FILE_PATH 1024
#define FRT_BUFFER_SIZE 1024

#define FRT_DBL2S "%#.7g"

#if defined(__GNUC__) && !defined(__cplusplus)
#  define FRT_INLINE
#else
#  define FRT_INLINE
#endif

#if __GNUC__ >= 3
#  define FRT_ATTR_ALWAYS_INLINE inline __attribute__ ((always_inline))
#  define FRT_ATTR_MALLOC               __attribute__ ((malloc))
#  define FRT_ATTR_PURE                 __attribute__ ((pure))
#  define FRT_ATTR_CONST                __attribute__ ((const))
#  define likely(x)   __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else
#  define FRT_ATTR_ALWAYS_INLINE
#  define FRT_ATTR_MALLOC
#  define FRT_ATTR_PURE
#  define FRT_ATTR_CONST
#  define likely(x)   (x)
#  define unlikely(x) (x)
#endif

#ifdef __cplusplus
#define FRT_EXTERNC extern "C"
#else
#define FRT_EXTERNC
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
#define FRT_TO_WORD(n) (((n - 1) >> 5) + 1)

#define FRT_RECAPA(self, len, capa, ptr, type) \
  do {\
    if (self->len >= self->capa) {\
      if (self->capa > 0) {\
        self->capa <<= 1;\
      } else {\
        self->capa = 4;\
      }\
      self->ptr = (type *)frt_erealloc(self->ptr, sizeof(type) * self->capa);\
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
extern frt_u32 *frt_u32malloc(frt_u32 value);
extern frt_u64 *frt_u64malloc(frt_u64 value);

extern char *frt_estrdup(const char *s);
extern char *frt_estrcat(char *str, char *str_cat);
extern void frt_weprintf(const char *fmt, ...);
extern char *frt_epstrdup(const char *fmt, int len, ...);

extern const char *FRT_EMPTY_STRING;

extern int frt_scmp(const void *p1, const void *p2);
extern int frt_icmp(const void *p1, const void *p2);
extern int frt_icmp_risky(const void *p1, const void *p2);
extern void frt_strsort(char **string_array, int size);

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
 * Returns the count of leading [MSB] 0 bits in +word+.
 */
static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_count_leading_zeros(frt_u32 word)
{
#ifdef __GNUC__
    if (word)
        return __builtin_clz(word);
    return 32;
#else
    static const int count_leading_zeros[] = {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
                if (word & 0xff) return count_leading_zeros[word & 0xff];
    word >>= 8; if (word & 0xff) return count_leading_zeros[word & 0xff] + 8;
    word >>= 8; if (word & 0xff) return count_leading_zeros[word & 0xff] + 16;
    word >>= 8;                  return count_leading_zeros[word & 0xff] + 24;
#endif
}

static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_count_leading_ones(frt_u32 word)
{
    return frt_count_leading_zeros(~word);
}

/**
 * Return the count of trailing [LSB] 0 bits in +word+.
 */

static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_count_trailing_zeros(frt_u32 word)
{
#ifdef __GNUC__
    if (word)
        return __builtin_ctz(word);
    return 32;
#else
    static const int count_trailing_zeros[] = {
        8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
    };
                if (word & 0xff) return count_trailing_zeros[word & 0xff];
    word >>= 8; if (word & 0xff) return count_trailing_zeros[word & 0xff] + 8;
    word >>= 8; if (word & 0xff) return count_trailing_zeros[word & 0xff] + 16;
    word >>= 8;                  return count_trailing_zeros[word & 0xff] + 24;
#endif
}

static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_count_trailing_ones(frt_u32 word)
{
    return frt_count_trailing_zeros(~word);
}

static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_count_ones(frt_u32 word)
{
#ifdef __GNUC__
    return __builtin_popcount(word);
#else
    static const frt_uchar count_ones[] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };
    return count_ones[(word      ) & 0xff]
        +  count_ones[(word >> 8 ) & 0xff]
        +  count_ones[(word >> 16) & 0xff]
        +  count_ones[(word >> 24) & 0xff];
#endif
}

static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_count_zeros(frt_u32 word)
{
    return frt_count_ones(~word);
}

/**
 * Round up to the next power of 2
 */
static FRT_ATTR_ALWAYS_INLINE FRT_ATTR_CONST
int frt_round2(frt_u32 word)
{
    return 1 << (32 - frt_count_leading_zeros(word));
}

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

#ifdef __cplusplus
} // extern "C"
#endif


/**
 * For general use during testing. Switch this on and off for print statements
 * to only print when p_on is called and not after p_off is called
 */
extern void p(const char *format, ...);
extern void p_on();
extern void p_off();
extern void p_pause();
extern void p_resume();

#endif

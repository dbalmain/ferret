#ifndef FRT_GLOBAL_H
#define FRT_GLOBAL_H

#include "defines.h"
#include "except.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_WORD_SIZE 255
#define MAX_FILE_PATH 1024

typedef void (*free_ft)(void *key);

#define NELEMS(array) ((int)(sizeof(array)/sizeof(array[0])))

#define ALLOC_N(type,n) (type *)emalloc(sizeof(type)*(n))
#define ALLOC(type) (type *)emalloc(sizeof(type))
#define REALLOC_N(ptr, type, n)\
    (ptr)=(type *)erealloc((ptr),sizeof(type)*(n))

#define ZEROSET(ptr, type) memset(ptr, 0, sizeof(type))
#define ZEROSET_N(ptr, type, n) memset(ptr, 0, sizeof(type)*(n))

#define ALLOC_AND_ZERO(type) (type*)ecalloc(sizeof(type))
#define ALLOC_AND_ZERO_N(type,n) (type*)ecalloc(sizeof(type)*(n))

#define REF(a) (a)->ref_cnt++
#define DEREF(a) (a)->ref_cnt--

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define MAX3(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

#define RECAPA(self, len, capa, ptr, type) \
  do {\
    if (self->len >= self->capa) {\
      if (self->capa > 0) {\
        self->capa <<= 1;\
      } else {\
        self->capa = 4;\
      }\
      REALLOC_N(self->ptr, type, self->capa);\
    }\
  } while (0)

#define Jx fprintf(stderr,"%s, %d: %s\n", __FILE__, __LINE__, __func__);
#define Xj fprintf(stdout,"%s, %d: %s\n", __FILE__, __LINE__, __func__);


#ifdef FRT_HAS_ISO_VARARGS
  /* C99-compliant compiler */

# define FRT_EXIT(...) frt_exit(__FILE__, __LINE__, __func__, __VA_ARGS__)
extern void frt_exit(const char *file, int line_num, const char *func,
                     const char *err_type, const char *fmt, ...);

# define V_FRT_EXIT(err_type, fmt, args) \
    vfrt_exit(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void vfrt_exit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args);

#elif defined(FRT_HAS_GNUC_VARARGS)
  /* gcc has an extension */
# define FRT_EXIT(args...) frt_exit(__FILE__, __LINE__, __func__, ##args)
extern void frt_exit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, ...);

# define V_FRT_EXIT(err_type, fmt, args) \
    vfrt_exit(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void vfrt_exit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args);
#else
  /* Can't do VARARGS */
extern void FRT_EXIT(const char *err_type, const char *fmt, ...);
extern void V_FRT_EXIT(const char *err_type, const char *fmt, va_list args);
#endif

extern char *progname();
extern void setprogname(const char *str);

extern void *emalloc(size_t n);
extern void *ecalloc(size_t n);
extern void *erealloc(void *ptr, size_t n);
extern char *estrdup(const char *s);
extern char *estrcat(char *str, char *str_cat);

extern const char *EMPTY_STRING;

extern int scmp(const void *p1, const void *p2);
extern int icmp(const void *p1, const void *p2);
extern int icmp_risky(const void *p1, const void *p2);

extern int min2(int a, int b);
extern int min3(int a, int b, int c);
extern int max2(int a, int b);
extern int max3(int a, int b, int c);

extern char *dbl_to_s(char *buf, double num);
extern char *strfmt(const char *fmt, ...);

/**
 * A dummy function which can be passed to functions which expect a free
 * function such as h_new() if you don't want the free functions to do anything.
 * This function will do nothing.
 *
 * @param p the object which this function will be called on.
 */
extern void dummy_free(void *p);

#endif

#ifndef FRT_LANG_H
#define FRT_LANG_H

#include <ruby.h>
#include "hash.h"

#define FERRET_EXT

#define MAX_ERROR_LEN 2048

typedef LONG_LONG llong;
typedef unsigned LONG_LONG ullong;

#ifdef WIN32
extern void eprintf(VALUE etype, const char *fmt, ...);
#else
# define eprintf(...) ft_raise(__FILE__, __LINE__, __VA_ARGS__)
#endif
extern void ft_raise(char *file, int line_num, VALUE etype, const char *fmt, ...);
extern void weprintf(const char *fmt, ...);
extern char *progname(void);
extern void setprogname(const char *str);

extern VALUE cQueryParseException;

#define EXCEPTION_CODE rb_eException
//#define IO_ERROR rb_eIOError
//#define ARG_ERROR rb_eArgError
//#define EOF_ERROR rb_eEOFError
//#define UNSUPPORTED_ERROR rb_eNotImpError
//#define STATE_ERROR rb_eException
//#define PARSE_ERROR cQueryParseException
//#define MEM_ERROR rb_eNoMemError

typedef void * mutex_t;
typedef struct HshTable * thread_key_t;
typedef int thread_once_t;
#define MUTEX_INITIALIZER NULL
#define MUTEX_RECURSIVE_INITIALIZER NULL
#define THREAD_ONCE_INIT 1;
#define mutex_init(a, b)
#define mutex_lock(a)
#define mutex_trylock(a)
#define mutex_unlock(a)
#define mutex_destroy(a)
#define thread_key_create(a, b) frt_thread_key_create(a, b)
#define thread_key_delete(a) frt_thread_key_delete(a)
#define thread_setspecific(a, b) frt_thread_setspecific(a, b)
#define thread_getspecific(a) frt_thread_getspecific(a)
#define thread_exit(a)
#define thread_once(a, b) frt_thread_once(a, b)

void frt_thread_once(int *once_control, void (*init_routine) (void));
void frt_thread_key_create(thread_key_t *key, void (*destr_function) (void *));
void frt_thread_key_delete(thread_key_t key);
void frt_thread_setspecific(thread_key_t key, const void *pointer);
void *frt_thread_getspecific(thread_key_t key);

#endif

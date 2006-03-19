#ifndef FRT_LANG_H
#define FRT_LANG_H

#include <ruby.h>

#define FERRET_EXT

#define MAX_ERROR_LEN 2048
#define eprintf(...) ft_raise(__FILE__, __LINE__, __VA_ARGS__)
extern void ft_raise(char *file, int line_num, VALUE etype, const char *fmt, ...);
extern void weprintf(const char *fmt, ...);
extern char *progname(void);
extern void setprogname(const char *str);

extern VALUE cQueryParseException;

#define ERROR rb_eException
#define IO_ERROR rb_eIOError
#define ARG_ERROR rb_eArgError
#define EOF_ERROR rb_eEOFError
#define UNSUPPORTED_ERROR rb_eNotImpError
#define STATE_ERROR rb_eException
#define PARSE_ERROR cQueryParseException
#define MEM_ERROR rb_eNoMemError

typedef void * mutex_t;
typedef void * thread_key_t;
#define MUTEX_INITIALIZER NULL
#define MUTEX_RECURSIVE_INITIALIZER NULL
#define mutex_init(a, b)
#define mutex_lock(a)
#define mutex_trylock(a)
#define mutex_unlock(a)
#define mutex_destroy(a)
#define thread_key_create(a, b)
#define thread_key_delete(a)
#define thread_setspecific(a, b)
#define thread_getspecific(a) NULL
#define thread_exit(a)

#endif

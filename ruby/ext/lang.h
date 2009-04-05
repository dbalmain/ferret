#ifndef FRT_LANG_H
#define FRT_LANG_H

#define RUBY_BINDINGS 1

#include <stdarg.h>
#include <ruby.h>

#undef close
#undef rename
#undef read

#define frt_emalloc xmalloc
#define frt_ecalloc(n) xcalloc(n, 1)
#define frt_erealloc xrealloc
/* FIXME: should eventually delete this */
#define FRT_REALLOC_N REALLOC_N


#ifdef FRT_HAS_ISO_VARARGS
/* C99-compliant compiler */

# define FRT_XEXIT(...) frb_rb_raise(__FILE__, __LINE__, __func__, __VA_ARGS__)
extern void frb_rb_raise(const char *file, int line_num, const char *func,
                         const char *err_type, const char *fmt, ...);

# define FRT_VEXIT(err_type, fmt, args) \
    frb_vrb_raise(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void frb_vrb_raise(const char *file, int line_num, const char *func,
                          const char *err_type, const char *fmt, va_list args);

#elif defined(FRT_HAS_GNUC_VARARGS)
/* gcc has an extension */

# define FRT_XEXIT(args...) frb_rb_raise(__FILE__, __LINE__, __func__, ##args)
extern void frb_rb_raise(const char *file, int line_num, const char *func,
                         const char *err_type, const char *fmt, ...);

# define FRT_VEXIT(err_type, fmt, args) \
    frb_vrb_raise(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void frb_vrb_raise(const char *file, int line_num, const char *func,
                          const char *err_type, const char *fmt, va_list args);
#else
/* Can't do VARARGS */

extern void FRT_XEXIT(const char *err_type, const char *fmt, ...);
extern void FRT_VEXIT(const char *err_type, const char *fmt, va_list args);
#endif

#ifdef RUBY_RUBY_H
#  define FRT_RUBY_VERSION_1_9
#endif

// ruby 1.8 compat with 1.9 to avoid ifdefs
#if !defined RSTRING_LEN
#define RSTRING_LEN(a) RSTRING(a)->len
#endif
#if !defined RSTRING_PTR
#define RSTRING_PTR(a) RSTRING(a)->ptr
#endif
#if !defined RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif
#if !defined RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

#endif

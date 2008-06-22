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

#endif

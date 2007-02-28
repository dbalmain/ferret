#ifndef FRT_LANG_H
#define FRT_LANG_H

#include <stdarg.h>

#define frt_malloc emalloc
#define frt_calloc ecalloc
#define frt_realloc erealloc

#define ALLOC(type) (type *)frt_malloc(sizeof(type))
#define ALLOC_N(type,n) (type *)frt_malloc(sizeof(type)*(n))
#define REALLOC_N(ptr, type, n)\
    (ptr)=(type *)frt_realloc((ptr),sizeof(type)*(n))

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

#endif

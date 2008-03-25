#ifndef FRT_LANG_H
#define FRT_LANG_H

#include <stdarg.h>

extern void *frt_malloc(size_t n);
extern void *frt_calloc(size_t n);
extern void *frt_realloc(void *ptr, size_t n);

#define ALLOC(type) (type *)frt_malloc(sizeof(type))
#define ALLOC_N(type,n) (type *)frt_malloc(sizeof(type)*(n))
#define REALLOC_N(ptr, type, n)\
    (ptr)=(type *)frt_realloc((ptr),sizeof(type)*(n))

#ifdef FRT_HAS_ISO_VARARGS
  /* C99-compliant compiler */

# define FRT_EXIT(...) frt_exit(__FILE__, __LINE__, __func__, __VA_ARGS__)
extern void frt_exit(const char *file, int line_num, const char *func,
                     const char *err_type, const char *fmt, ...);

# define FRT_VEXIT(err_type, fmt, args) \
    frt_vexit(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void frt_vexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args);

#elif defined(FRT_HAS_GNUC_VARARGS)
  /* gcc has an extension */
# define FRT_EXIT(args...) frt_exit(__FILE__, __LINE__, __func__, ##args)
extern void frt_exit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, ...);

# define FRT_VEXIT(err_type, fmt, args) \
    frt_vexit(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void frt_vexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args);
#else
  /* Can't do VARARGS */
extern void FRT_EXIT(const char *err_type, const char *fmt, ...);
extern void FRT_VEXIT(const char *err_type, const char *fmt, va_list args);
#endif
#endif

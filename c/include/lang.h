#ifndef FRT_LANG_H
#define FRT_LANG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

extern void *frt_emalloc(size_t n);
extern void *frt_ecalloc(size_t n);
extern void *frt_erealloc(void *ptr, size_t n);

#define FRT_ALLOC(type) (type *)frt_emalloc(sizeof(type))
#define FRT_ALLOC_N(type,n) (type *)frt_emalloc(sizeof(type)*(n))
#define FRT_REALLOC_N(ptr, type, n)\
    (ptr)=(type *)frt_erealloc((ptr),sizeof(type)*(n))

#ifdef FRT_HAS_ISO_VARARGS
  /* C99-compliant compiler */

# define FRT_XEXIT(...) frt_xexit(__FILE__, __LINE__, __func__, __VA_ARGS__)
extern void frt_xexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, ...);

# define FRT_VEXIT(err_type, fmt, args) \
    frt_vexit(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void frt_vexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args);

#elif defined(FRT_HAS_GNUC_VARARGS)
  /* gcc has an extension */
# define FRT_XEXIT(args...) frt_xexit(__FILE__, __LINE__, __func__, ##args)
extern void frt_xexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, ...);

# define FRT_VEXIT(err_type, fmt, args) \
    frt_vexit(__FILE__, __LINE__, __func__, err_type, fmt, args)
extern void frt_vexit(const char *file, int line_num, const char *func,
                      const char *err_type, const char *fmt, va_list args);
#else
  /* Can't do VARARGS */
extern void FRT_XEXIT(const char *err_type, const char *fmt, ...);
extern void FRT_VEXIT(const char *err_type, const char *fmt, va_list args);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif

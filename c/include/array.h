#ifndef FRT_ARRAY_H
#define FRT_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

#if defined POSH_OS_SOLARIS || defined POSH_OS_SUNOS
# define FRT_ARY_META_CNT 4
#else
# define FRT_ARY_META_CNT 3
#endif

#define FRT_ARY_INIT_CAPA 8
#define frt_ary_size(ary)      frt_ary_sz(ary)
#define frt_ary_sz(ary)        (((int *)ary)[-1])
#define frt_ary_capa(ary)      (((int *)ary)[-2])
#define frt_ary_type_size(ary) (((int *)ary)[-3])
#define frt_ary_start(ary)     ((void **)&(((int *)ary)[-FRT_ARY_META_CNT]))
#define frt_ary_free(ary)      free(frt_ary_start(ary))

#define frt_ary_new_type_capa(type, init_capa)\
                                (type *)frt_ary_new_i(sizeof(type), init_capa)
#define frt_ary_new_type(type)      (type *)frt_ary_new_i(sizeof(type), 0)
#define frt_ary_new_capa(init_capa) frt_ary_new_i(sizeof(void *), init_capa)
#define frt_ary_new()               frt_ary_new_i(sizeof(void *), 0)
#define frt_ary_resize(ary, size)   frt_ary_resize_i(((void ***)(void *)&ary), size)
#define frt_ary_set(ary, i, val)    frt_ary_set_i(((void ***)(void *)&ary), i, val)
#define frt_ary_get(ary, i)         frt_ary_get_i(((void **)ary), i)
#define frt_ary_push(ary, val)      frt_ary_push_i(((void ***)(void *)&ary), val)
#define frt_ary_pop(ary)            frt_ary_pop_i(((void **)ary))
#define frt_ary_unshift(ary, val)   frt_ary_unshift_i(((void ***)(void *)&ary), val)
#define frt_ary_shift(ary)          frt_ary_shift_i(((void **)ary))
#define frt_ary_remove(ary, i)      frt_ary_remove_i(((void **)ary), i)
#define frt_ary_delete(ary, i, f)   frt_ary_delete_i(((void **)ary), i, (free_ft)f)
#define frt_ary_destroy(ary, f)     frt_ary_destroy_i(((void **)ary), (free_ft)f)
#define frt_ary_rsz(ary, size)      frt_ary_resize(ary, size)
#define frt_ary_grow(ary)           frt_ary_resize(ary, frt_ary_sz(ary))
#define frt_ary_last(ary)           ary[frt_ary_sz(ary) - 1]
#define frt_ary_sort(ary, cmp)      qsort(ary, frt_ary_size(ary), frt_ary_type_size(ary), cmp)
#define frt_ary_each_rev(ary, i)    for (i = frt_ary_size(ary) - 1; i >= 0; i--)
#define frt_ary_each(ary, i)        for (i = 0; i < frt_ary_size(ary); i++)

extern void   frt_ary_resize_i(void ***ary, int size);
extern void **frt_ary_new_i(int type_size, int init_capa);
extern void   frt_ary_set_i(void ***ary, int index, void *value);
extern void  *frt_ary_get_i(void **ary, int index);
extern void   frt_ary_push_i(void ***ary, void *value);
extern void  *frt_ary_pop_i(void **ary);
extern void   frt_ary_unshift_i(void ***ary, void *value);
extern void  *frt_ary_shift_i(void **ary);
extern void  *frt_ary_remove_i(void **ary, int index);
extern void   frt_ary_delete_i(void **ary, int index, frt_free_ft p);
extern void   frt_ary_destroy_i(void **ary, frt_free_ft p);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

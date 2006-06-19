#ifndef FRT_ARRAY_H
#define FRT_ARRAY_H

#define ARY_INIT_CAPA 8
#define ary_sz(ary)        (((int *)ary)[-1])
#define ary_capa(ary)      (((int *)ary)[-2])
#define ary_type_size(ary) (((int *)ary)[-3])
#define ary_start(ary)     ((void **)&(((int *)ary)[-3]))
#define ary_free(ary)      free(ary_start(ary))

#define ary_new_type_capa(type, init_capa)\
                                (type *)ary_new_i(sizeof(type), init_capa)
#define ary_new_type(type)      (type *)ary_new_i(sizeof(type), 0)
#define ary_new_capa(init_capa) ary_new_i(sizeof(void *), init_capa)
#define ary_new()               ary_new_i(sizeof(void *), 0)
#define ary_resize(ary, size)   ary_set_i(((void ***)&ary), size)
#define ary_set(ary, i, val)    ary_set_i(((void ***)&ary), i, val)
#define ary_push(ary, val)      ary_push_i(((void ***)&ary), val)
#define ary_unshift(ary, val)   ary_unshift_i(((void ***)&ary), val)


extern inline void ary_resize_i(void ***ary, int size);
extern void **ary_new_i(int type_size, int init_capa);
extern void   ary_set_i(void ***ary, int index, void *value);
extern void  *ary_get(void **ary, int index);
extern void   ary_push_i(void ***ary, void *value);
extern void  *ary_pop(void **ary);
extern void   ary_unshift_i(void ***ary, void *value);
extern void  *ary_shift(void **ary);
extern void  *ary_remove(void **ary, int index);
extern void   ary_delete(void **ary, int index, void (*free_elem)(void *p));
extern void   ary_destroy(void **ary, void (*free_elem)(void *p));

#endif

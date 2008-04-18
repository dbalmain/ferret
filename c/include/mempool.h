#ifndef FRT_MEM_POOL_H
#define FRT_MEM_POOL_H

#define MP_BUF_SIZE 65536
#define MP_INIT_CAPA 4

typedef struct FrtMemoryPool {
    int buf_alloc;
    int buf_capa;
    int buf_pointer;
    int pointer;
    int chunk_size;
    char *curr_buffer;
    char **buffers;
} FrtMemoryPool;

extern FrtMemoryPool *mp_new();
extern FrtMemoryPool *mp_new_capa(int chunk_size, int init_capa);
extern FRT_INLINE void *mp_alloc(FrtMemoryPool *mp, int size);
extern void mp_reset(FrtMemoryPool *mp);
extern void mp_destroy(FrtMemoryPool *mp);
extern char *mp_strdup(FrtMemoryPool *mp, const char *str);
extern char *mp_strndup(FrtMemoryPool *mp, const char *str, int len);
extern void *mp_memdup(FrtMemoryPool *mp, const void *p, int len);
extern int mp_used(FrtMemoryPool *mp);

#define MP_ALLOC_N(mp,type,n) (type *)mp_alloc(mp, sizeof(type)*(n))
#define MP_ALLOC(mp,type) (type *)mp_alloc(mp, sizeof(type))

#define MP_ALLOC_AND_ZERO(mp,type)\
    (type*)memset(mp_alloc(mp, sizeof(type)), 0, sizeof(type))
#define MP_ALLOC_AND_ZERO_N(mp,type,n)\
    (type*)FRT_ZEROSET_N(mp_alloc(mp, sizeof(type)*(n)), type, n)

#endif

#ifndef FRT_MEM_POOL_H
#define FRT_MEM_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#define FRT_MP_BUF_SIZE 65536
#define FRT_MP_INIT_CAPA 4

typedef struct FrtMemoryPool {
    int buf_alloc;
    int buf_capa;
    int buf_pointer;
    int pointer;
    int chunk_size;
    char *curr_buffer;
    char **buffers;
} FrtMemoryPool;

extern FrtMemoryPool *frt_mp_new();
extern FrtMemoryPool *frt_mp_new_capa(int chunk_size, int init_capa);
extern FRT_INLINE void *frt_mp_alloc(FrtMemoryPool *mp, int size);
extern void frt_mp_reset(FrtMemoryPool *mp);
extern void frt_mp_destroy(FrtMemoryPool *mp);
extern char *frt_mp_strdup(FrtMemoryPool *mp, const char *str);
extern char *frt_mp_strndup(FrtMemoryPool *mp, const char *str, int len);
extern void *frt_mp_memdup(FrtMemoryPool *mp, const void *p, int len);
extern int frt_mp_used(FrtMemoryPool *mp);

#define FRT_MP_ALLOC_N(mp,type,n) (type *)frt_mp_alloc(mp, sizeof(type)*(n))
#define FRT_MP_ALLOC(mp,type) (type *)frt_mp_alloc(mp, sizeof(type))

#define FRT_MP_ALLOC_AND_ZERO(mp,type)\
    (type*)memset(frt_mp_alloc(mp, sizeof(type)), 0, sizeof(type))
#define FRT_MP_ALLOC_AND_ZERO_N(mp,type,n)\
    (type*)FRT_ZEROSET_N(frt_mp_alloc(mp, sizeof(type)*(n)), type, n)

#ifdef __cplusplus
} // extern "C"
#endif

#endif

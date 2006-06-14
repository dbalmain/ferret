#include "global.h"
#include "mem_pool.h"
#include <string.h>

MemoryPool *mp_new_capa(int init_buf_capa)
{
    MemoryPool *mp = ALLOC(MemoryPool);
    mp->buf_capa = init_buf_capa;
    mp->buffers = ALLOC_N(char *, init_buf_capa);

    mp->buffers[0] = mp->curr_buffer = emalloc(MP_BUF_SIZE);
    mp->buf_alloc = 1;
    mp->buf_pointer = 0;
    mp->pointer = 0;
    return mp;
}

MemoryPool *mp_new()
{
    return mp_new_capa(MP_INIT_CAPA);
}

inline void *mp_alloc(MemoryPool *mp, int size)
{
    char *p;
    p = mp->curr_buffer + mp->pointer;
    mp->pointer += size;

    if (mp->pointer > MP_BUF_SIZE) {
        mp->buf_pointer++;
        if (mp->buf_pointer >= mp->buf_alloc) {
            mp->buf_alloc++;
            if (mp->buf_alloc >= mp->buf_capa) {
                mp->buf_capa <<= 1;
                REALLOC_N(mp->buffers, char *, mp->buf_capa);
            }
            mp->buffers[mp->buf_pointer] = emalloc(MP_BUF_SIZE);
        }
        p = mp->curr_buffer = mp->buffers[mp->buf_pointer];
        mp->pointer = size;
    }
    return p;
}

char *mp_strdup(MemoryPool *mp, char *str)
{
    int len = strlen(str) + 1;
    return memcpy(mp_alloc(mp, len), str, len);
}

void *mp_memdup(MemoryPool *mp, void *p, int len)
{
    return memcpy(mp_alloc(mp, len), p, len);
}

void mp_reset(MemoryPool *mp)
{
    mp->buf_pointer = 0;
    mp->pointer = 0;
    mp->curr_buffer = mp->buffers[0];
}

void mp_destroy(MemoryPool *mp)
{
    int i;
    for (i = 0; i < mp->buf_alloc; i++) {
        free(mp->buffers[i]);
    }
    free(mp->buffers);
    free(mp);
}

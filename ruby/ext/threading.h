#ifndef FRT_THREADING_H
#define FRT_THREADING_H

#include "hash.h"
#define UNTHREADED 1

typedef void * frt_mutex_t;
typedef struct FrtHash *frt_thread_key_t;
typedef int frt_thread_once_t;
#define FRT_MUTEX_INITIALIZER NULL
#define FRT_MUTEX_RECURSIVE_INITIALIZER NULL
#define FRT_THREAD_ONCE_INIT 1;
#define frt_mutex_init(a, b)
#define frt_mutex_lock(a)
#define frt_mutex_trylock(a)
#define frt_mutex_unlock(a)
#define frt_mutex_destroy(a)
#define frt_thread_key_create(a, b) frb_thread_key_create(a, b)
#define frt_thread_key_delete(a) frb_thread_key_delete(a)
#define frt_thread_setspecific(a, b) frb_thread_setspecific(a, b)
#define frt_thread_getspecific(a) frb_thread_getspecific(a)
#define frt_thread_exit(a)
#define frt_thread_once(a, b) frb_thread_once(a, b)

void frb_thread_once(int *once_control, void (*init_routine)(void));
void frb_thread_key_create(frt_thread_key_t *key, frt_free_ft destroy);
void frb_thread_key_delete(frt_thread_key_t key);
void frb_thread_setspecific(frt_thread_key_t key, const void *pointer);
void *frb_thread_getspecific(frt_thread_key_t key);

#endif

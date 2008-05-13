#ifndef FRT_THREADING_H
#define FRT_THREADING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef pthread_mutex_t frt_mutex_t;
typedef pthread_key_t frt_thread_key_t;
typedef pthread_once_t frt_thread_once_t;
#define FRT_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define FRT_MUTEX_RECURSIVE_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#define FRT_THREAD_ONCE_INIT PTHREAD_ONCE_INIT
#define frt_mutex_init(a, b) pthread_mutex_init(a, b)
#define frt_mutex_lock(a) pthread_mutex_lock(a)
#define frt_mutex_trylock(a) pthread_mutex_trylock(a)
#define frt_mutex_unlock(a) pthread_mutex_unlock(a)
#define frt_mutex_destroy(a) pthread_mutex_destroy(a)
#define frt_thread_key_create(a, b) pthread_key_create(a, b)
#define frt_thread_key_delete(a) pthread_key_delete(a)
#define frt_thread_setspecific(a, b) pthread_setspecific(a, b)
#define frt_thread_getspecific(a) pthread_getspecific(a)
#define frt_thread_exit(a) pthread_exit(a)
#define frt_thread_once(a, b) pthread_once(a, b)

#ifdef __cplusplus
} // extern "C"
#endif

#endif

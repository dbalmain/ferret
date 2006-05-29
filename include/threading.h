#ifndef FRT_THREADING_H
#define FRT_THREADING_H

#include <pthread.h>

typedef pthread_mutex_t mutex_t;
typedef pthread_key_t thread_key_t;
typedef pthread_once_t thread_once_t;
#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define MUTEX_RECURSIVE_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#define THREAD_ONCE_INIT PTHREAD_ONCE_INIT;
#define mutex_init(a, b) pthread_mutex_init(a, b)
#define mutex_lock(a) pthread_mutex_lock(a)
#define mutex_trylock(a) pthread_mutex_trylock(a)
#define mutex_unlock(a) pthread_mutex_unlock(a)
#define mutex_destroy(a) pthread_mutex_destroy(a)
#define thread_key_create(a, b) pthread_key_create(a, b)
#define thread_key_delete(a) pthread_key_delete(a)
#define thread_setspecific(a, b) pthread_setspecific(a, b)
#define thread_getspecific(a) pthread_getspecific(a)
#define thread_exit(a) pthread_exit(a)
#define thread_once(a, b) pthread_once(a, b)

#endif

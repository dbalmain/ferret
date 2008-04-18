#include <stdlib.h>
#include "test.h"
#include "mempool.h"


static void test_mp_default_capa(TestCase *tc, void *data)
{
    MemoryPool *mp = mp_new();
    (void)data;
    Aiequal(MP_INIT_CAPA, mp->buf_capa);
    mp_destroy(mp);
}

struct MemChecker {
    int size;
    char vals[1];
};

#define NUM_ALLOCS 10000
#define MAX_SIZE 100

static void do_mp_test(TestCase *tc, MemoryPool *mp)
{
    int i, j;
    int max_necessary;
    int total_bytes = 0;
    struct MemChecker *mem_checkers[NUM_ALLOCS];

    for (i = 0; i < NUM_ALLOCS; i++) {
        int size = rand() % MAX_SIZE;
        total_bytes += size + sizeof(int);
        mem_checkers[i] = (struct MemChecker *)mp_alloc(mp, size + sizeof(int));
        mem_checkers[i]->size =  size;
    }
    for (i = 0; i < NUM_ALLOCS; i++) {
        for (j = 0; j < mem_checkers[i]->size; j++) {
            mem_checkers[i]->vals[j] = (char)(i & 0xFF);
        }
    }
    for (i = 0; i < NUM_ALLOCS; i++) {
        for (j = 0; j < mem_checkers[i]->size; j++) {
            if (mem_checkers[i]->vals[j] != (char)(i & 0xFF)) {
                Aiequal(i & 0xFF, mem_checkers[i]->vals[j]);
            }
        }
    }
    if (!Atrue(total_bytes < (mp->buf_alloc * mp->chunk_size))) {
        Tmsg("total bytes allocated <%d> > memory used <%d>",
             total_bytes, mp->buf_alloc * mp->chunk_size);
    }

    max_necessary =
        (mp->buf_alloc - 1) * (mp->chunk_size - (MAX_SIZE+sizeof(int)));
    if (!Atrue(total_bytes > max_necessary)) {
        Tmsg("total bytes allocated <%d> < max memory needed <%d>",
             total_bytes, max_necessary);
    }
}

static void test_mp_dup(TestCase *tc, void *data)
{
    MemoryPool *mp = mp_new_capa(2000, 16);
    (void)data;

    do_mp_test(tc, mp);
    mp_reset(mp);
    do_mp_test(tc, mp);
    mp_destroy(mp);
}

static void test_mp_alloc(TestCase *tc, void *data)
{
    MemoryPool *mp = mp_new_capa(20, 16);
    char *t;
    (void)data;

    t = mp_strdup(mp, "012345678901234");

    Asequal("012345678901234", t);
    Aiequal(strlen(t) + 1, mp_used(mp));

    t = (char *)mp_memdup(mp, "012345678901234", 10);
    Asnequal("012345678901234", t, 10);
    Aiequal(30, mp_used(mp));

    t = (char *)mp_strndup(mp, "012345678", 9);
    Asequal("012345678", t);
    Aiequal(40, mp_used(mp)); /* Stays in the same chunk */

    mp_destroy(mp);
}

TestSuite *ts_mem_pool(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_mp_default_capa, NULL);
    tst_run_test(suite, test_mp_alloc, NULL);
    tst_run_test(suite, test_mp_dup, NULL);

    return suite;
}

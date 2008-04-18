#include <assert.h>
#include "bitvector.h"
#include "benchmark.h"

#define N 5
#define DENSE_SCAN_SIZE 2000000
#define SCAN_INC 97
#define SCAN_SIZE DENSE_SCAN_SIZE * SCAN_INC

static BitVector *bv;
static BitVector *not_bv;

static void setup()
{
    bv = bv_new_capa(SCAN_SIZE);
}

static void teardown()
{
    bv_destroy(bv);
    bv_destroy(not_bv);
}

static void ferret_bv_set_sparse()
{
    int i;

    for (i = SCAN_INC; i < SCAN_SIZE; i += SCAN_INC) {
        bv_set_fast(bv, i);
    }

    not_bv = bv_not(bv);
}

static void ferret_bv_scan_sparse()
{
    int i, j;

    for (i = 0; i < N; i++) {
        bv_scan_reset(bv);
        bv_scan_reset(not_bv);
        for (j = SCAN_INC; j < SCAN_SIZE; j += SCAN_INC) {
            assert(j == bv_scan_next(bv));
            assert(j == bv_scan_next_unset(not_bv));
        }
        assert(-1 == bv_scan_next(bv));
        assert(-1 == bv_scan_next_unset(not_bv));
    }
}

BENCH(bitvector_implementations)
{
    BM_SETUP(setup);
    BM_ADD(ferret_bv_set_sparse);
    BM_ADD(ferret_bv_scan_sparse);
    BM_TEARDOWN(teardown);
}

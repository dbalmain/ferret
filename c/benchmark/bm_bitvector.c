#include <assert.h>
#include "bitvector.h"
#include "benchmark.h"

#define N 1
#define DENSE_SCAN_SIZE 20000000
#define SCAN_INC 97
#define SCAN_SIZE DENSE_SCAN_SIZE * SCAN_INC

#ifdef __cplusplus
#include <bitset>
std::bitset<SCAN_SIZE> bs;
static void cpp_bs_set_sparse()
{
    int i;

    for (i = SCAN_INC; i < SCAN_SIZE; i += SCAN_INC) {
        bs[i] = 1;
    }
}

static void cpp_bs_scan_sparse()
{
    int i;

    for (i = 0; i < N; i++) {
        int bit, j;
        //for (j = SCAN_INC, bit = bs.find_first(); j < SCAN_SIZE; j += SCAN_INC, bit = bs.find_next(bit)) {
        //    assert(j == bit);
        //}
        //assert(-1 == bv_scan_next(bv));
    }
}

static void cpp_bs_set_dense()
{
    int i;
    bs.reset();
    for (i = 0; i < DENSE_SCAN_SIZE; i++) {
        bs[i] = 1;
    }
}

//static void cpp_bs_scan_dense()
//{
//    int i, j;
//
//    for (i = 0; i < N; i++) {
//        bv_scan_reset(bv);
//        for (j = 0; j < DENSE_SCAN_SIZE; j++) {
//            assert(j == bv_scan_next(bv));
//        }
//        assert(-1 == bv_scan_next(bv));
//    }
//}
#endif

static BitVector *bv;

static void setup()
{
    bv = bv_new_capa(SCAN_SIZE);
}

static void teardown()
{
    bv_destroy(bv);
}

static void ferret_bv_set_sparse()
{
    int i;

    for (i = SCAN_INC; i < SCAN_SIZE; i += SCAN_INC) {
        bv_set_fast(bv, i);
        if (bv_get(bv, i) == 1) {printf("%d\n", bv_get(bv, i));}
    }
}

static void ferret_bv_scan_sparse()
{
    int i, j;

    for (i = 0; i < N; i++) {
        bv_scan_reset(bv);
        for (j = SCAN_INC; j < SCAN_SIZE; j += SCAN_INC) {
            assert(j == bv_scan_next(bv));
        }
        assert(-1 == bv_scan_next(bv));
    }
}

static void ferret_bv_set_dense()
{
    int i;
    bv_clear(bv);
    for (i = 0; i < DENSE_SCAN_SIZE; i++) {
        bv_set(bv, i);
    }
}

static void ferret_bv_scan_dense()
{
    int i, j;

    for (i = 0; i < N; i++) {
        bv_scan_reset(bv);
        for (j = 0; j < DENSE_SCAN_SIZE; j++) {
            assert(j == bv_scan_next(bv));
        }
        assert(-1 == bv_scan_next(bv));
    }
}

BENCH(bitvector_implementations)
{
    BM_SETUP(setup);
    BM_ADD(ferret_bv_set_sparse);
    BM_ADD(ferret_bv_scan_sparse);
    BM_ADD(ferret_bv_set_dense);
    BM_ADD(ferret_bv_scan_dense);
#ifdef __cplusplus
    BM_ADD(cpp_bs_set_sparse);
    BM_ADD(cpp_bs_scan_sparse);
    BM_ADD(cpp_bs_set_dense);
    //BM_ADD(cpp_bs_scan_dense);
#endif
    BM_TEARDOWN(teardown);
}

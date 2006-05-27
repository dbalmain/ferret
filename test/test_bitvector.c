#include <time.h>
#include "test.h"
#include "bitvector.h"

#define BV_SIZE 1000
#define BV_INT 33

/**
 * Test basic BitVector get/set/unset operations
 */
static void test_bv(tst_case *tc, void *data)
{
    int i;
    BitVector *bv = bv_create();
    (void)data; /* suppress unused argument warning */

    Aiequal(0, bv->size);
    Aiequal(0, bv->count);
    Aiequal(0, bv_recount(bv));

    bv_set(bv, 10);
    Aiequal(1, bv_get(bv, 10));
    Aiequal(11, bv->size);
    Aiequal(1, bv->count);
    Aiequal(1, bv_recount(bv));

    bv_set(bv, 10);
    Aiequal(1, bv_get(bv, 10));
    Aiequal(11, bv->size);
    Aiequal(1, bv->count);
    Aiequal(1, bv_recount(bv));

    bv_set(bv, 20);
    Aiequal(1, bv_get(bv, 20));
    Aiequal(21, bv->size);
    Aiequal(2, bv->count);
    Aiequal(2, bv_recount(bv));

    bv_unset(bv, 21);
    Aiequal(0, bv_get(bv, 21));
    Aiequal(21, bv->size);
    Aiequal(2, bv->count);
    Aiequal(2, bv_recount(bv));

    bv_unset(bv, 20);
    Aiequal(0, bv_get(bv, 20));
    Aiequal(21, bv->size);
    Aiequal(1, bv->count);
    Aiequal(1, bv_recount(bv));
    Aiequal(1, bv_get(bv, 10));

    bv_set(bv, 100);
    Aiequal(1, bv_get(bv, 100));
    Aiequal(101, bv->size);
    Aiequal(2, bv->count);
    Aiequal(2, bv_recount(bv));
    Aiequal(1, bv_get(bv, 10));

    bv_clear(bv);
    Aiequal(0, bv_get(bv, 10));
    Aiequal(0, bv->size);
    Aiequal(0, bv->count);
    Aiequal(0, bv_recount(bv));

    /* test setting bits at intervals for a large number of bits */
    bv_clear(bv);
    for (i = BV_INT; i < BV_SIZE; i += BV_INT) {
        bv_set(bv, i);
    }
    for (i = BV_INT; i < BV_SIZE; i += BV_INT) {
        Aiequal(1, bv_get(bv, i));
        Aiequal(0, bv_get(bv, i - 1));
        Aiequal(0, bv_get(bv, i + 1));
    }

    bv_destroy(bv);
}

/**
 * Test simple BitVector scanning
 */
static void test_bv_scan(tst_case *tc, void *data)
{
    int i;
    BitVector *bv = bv_create();
    (void)data; /* suppress unused argument warning */

    for (i = 6; i <= 10; i++) {
        bv_set(bv, i);
    }
    for (i = 6; i <= 10; i++) {
        Aiequal(i, bv_scan_next(bv));
    }
    Aiequal(-1, bv_scan_next(bv));
    bv_destroy(bv);
}

#define BV_DENSE_SCAN_SIZE 2000
#define BV_SCAN_INC 97
#define BV_SCAN_SIZE BV_DENSE_SCAN_SIZE * BV_SCAN_INC
/**
 * Stress test BitVector Scanning as well as bv_set_fast. This test has been
 * run successfully with BV_DENSE_SCAN_SIZE set to 20000000 and BV_SCAN_INC
 * set to 97. When running this test with high numbers, be sure use -q on the
 * command line or the test will take a very long time.
 */
static void test_bv_scan_stress(tst_case * tc, void *data)
{
    int i;
    BitVector *bv = bv_create_capa(BV_SCAN_SIZE);
    (void)data; /* suppress unused argument warning */

    /*
    clock_t t = clock(), total_t = 0;
    */

    for (i = BV_SCAN_INC; i < BV_SCAN_SIZE; i += BV_SCAN_INC) {
        bv_set_fast(bv, i);
    }

    /*
    t = clock() - t;
    total_t += t;
    printf("bv_set took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    t = clock();
    */

    for (i = BV_SCAN_INC; i < BV_SCAN_SIZE; i += BV_SCAN_INC) {
        Aiequal(i, bv_scan_next_from(bv, i - BV_SCAN_INC / 2));
    }
    Aiequal(-1, bv_scan_next_from(bv, i - BV_SCAN_INC / 2));

    /*
    t = clock() - t;
    total_t += t;
    printf("bv_scan_next_from took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    t = clock();
    */

    bv_scan_reset(bv);
    for (i = BV_SCAN_INC; i < BV_SCAN_SIZE; i += BV_SCAN_INC) {
        Aiequal(i, bv_scan_next(bv));
    }
    Aiequal(-1, bv_scan_next(bv));

    /*
    t = clock() - t;
    total_t += t;
    printf("bv_scan_next took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    */

    bv_clear(bv);
    for (i = 0; i < BV_DENSE_SCAN_SIZE; i++) {
        bv_set(bv, i);
    }

    /*
    t = clock();
    */

    for (i = 0; i < BV_DENSE_SCAN_SIZE; i++) {
        Aiequal(i, bv_scan_next_from(bv, i));
    }
    Aiequal(-1, bv_scan_next_from(bv, i));

    /*
    t = clock() - t;
    total_t += t;
    printf("dense bv_scan_next_from took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    t = clock();
    */

    bv_scan_reset(bv);
    for (i = 0; i < BV_DENSE_SCAN_SIZE; i++) {
        Aiequal(i, bv_scan_next(bv));
    }
    Aiequal(-1, bv_scan_next(bv));

    /*
    t = clock() - t;
    total_t += t;
    printf("dense bv_scan_next took %0.3f secs\n", (double) t / CLOCKS_PER_SEC);
    printf("total time %0.3f seconds\n",
           (double) total_t / CLOCKS_PER_SEC);
    */

    bv_destroy(bv);
}


tst_suite *ts_bitvector(tst_suite * suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_bv, NULL);
    tst_run_test(suite, test_bv_scan, NULL);
    tst_run_test(suite, test_bv_scan_stress, NULL);

    return suite;
}

#include <time.h>
#include "test.h"
#include "testhelper.h"
#include "bitvector.h"

#define BV_SIZE 1000
#define BV_INT 33

#define SET_BITS_MAX_CNT 100
static BitVector *set_bits(BitVector *bv, char *bits)
{
    static int bit_array[SET_BITS_MAX_CNT];
    const int bit_cnt = s2l(bits, bit_array);
    int i;
    for (i = 0; i < bit_cnt; i++) {
        bv_set(bv, bit_array[i]);
    }
    return bv;
}

/*
static BitVector *unset_bits(BitVector *bv, char *bits)
{
    static int bit_array[SET_BITS_MAX_CNT];
    const int bit_cnt = s2l(bits, bit_array);
    int i;
    for (i = 0; i < bit_cnt; i++) {
        bv_unset(bv, bit_array[i]);
    }
    return bv;
}
*/

/**
 * Test basic BitVector get/set/unset operations
 */
static void test_bv(tst_case *tc, void *data)
{
    int i;
    BitVector *bv = bv_new();
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
    Aiequal(22, bv->size);
    Aiequal(2, bv->count);
    Aiequal(2, bv_recount(bv));

    bv_unset(bv, 20);
    Aiequal(0, bv_get(bv, 20));
    Aiequal(22, bv->size);
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
    bv_unset(bv, 20);
    Aiequal(21, bv->size);

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

    /* test setting all bits */
    bv_clear(bv);
    for (i = 0; i < BV_SIZE; i++) {
        bv_set(bv, i);
    }
    for (i = 0; i < BV_SIZE; i++) {
        Aiequal(1, bv_get(bv, i));
    }

    /* test random bits */
    bv_clear(bv);
    for (i = 0; i < BV_SIZE; i++) {
        if ((rand() % 2) == 0) {
            bv_set(bv, i);
            Aiequal(1, bv_get(bv, i));
        }
    }

    bv_destroy(bv);
}

/**
 * Test simple BitVector scanning
 */
static void test_bv_scan(tst_case *tc, void *data)
{
    int i;
    BitVector *bv = bv_new();
    BitVector *not_bv;
    (void)data; /* suppress unused argument warning */

    for (i = 6; i <= 10; i++) {
        bv_set(bv, i);
    }
    not_bv = bv_not(bv);
    for (i = 6; i <= 10; i++) {
        Aiequal(i, bv_scan_next(bv));
        Aiequal(i, bv_scan_next_unset(not_bv));
    }
    Aiequal(-1, bv_scan_next(bv));
    Aiequal(-1, bv_scan_next_unset(bv));
    bv_destroy(bv);
    bv_destroy(not_bv);
}

static void test_bv_eq_hash(tst_case *tc, void *data)
{
    static int const COUNT = 1000;
    int i;
    BitVector *bv1 = bv_new();
    BitVector *bv2 = bv_new();
    (void)data;

    Assert(bv_eq(bv1, bv2), "BitVectors are both empty");
    Assert(bv_eq(bv1, bv1), "bv_eq on self should work");
    Aiequal(bv_hash(bv1), bv_hash(bv2));

    bv_set(bv1, 1);
    Assert(!bv_eq(bv1, bv2), "BitVectors are not equal");
    Assert(bv_hash(bv1) != bv_hash(bv2), "BitVectors are not equal");

    bv_set(bv2, 11);
    Assert(!bv_eq(bv1, bv2), "BitVectors are not equal");
    Assert(bv_hash(bv1) != bv_hash(bv2), "BitVectors are not equal");

    bv_set(bv1, 11);
    bv_set(bv2, 1);
    Assert(bv_eq(bv1, bv2), "BitVectors are equal");
    Aiequal(bv_hash(bv1), bv_hash(bv2));

    bv_unset(bv1, 1000);
    Assert(bv_eq(bv1, bv2), "BitVectors are equal");
    Aiequal(bv_hash(bv1), bv_hash(bv2));

    for (i = 0; i < COUNT; i++) {
        int bit = rand() % COUNT;
        bv_set(bv1, bit);
        bv_set(bv2, bit);
    }

    Assert(bv_eq(bv1, bv2), "BitVectors are equal");
    Aiequal(bv_hash(bv1), bv_hash(bv2));
    bv_destroy(bv1);
    bv_destroy(bv2);

    bv1 = set_bits(bv_new(), "1, 3, 5");
    bv2 = bv_not_x(set_bits(bv_new(), "0, 2, 4"));
    bv_set(bv2, 5);
    Assert(!bv_eq(bv1, bv2), "BitVectors have different extension");
    Assert(bv_hash(bv1) != bv_hash(bv2), "BitVectors have different extension");
    bv_destroy(bv1);
    bv_destroy(bv2);
}

static void test_bv_and(tst_case *tc, void *data)
{
#   define AND_SIZE 1000
    static const int and_cnt = 500;
    BitVector *and_bv;
    BitVector *bv1 = bv_new();
    BitVector *bv2 = bv_new();
    char set1[AND_SIZE];
    char set2[AND_SIZE];
    int i;
    int count = 0;
    (void)data;

    memset(set1, 0, AND_SIZE);
    memset(set2, 0, AND_SIZE);
    for (i = 0; i < and_cnt; i++) {
        int bit = rand() % AND_SIZE;
        bv_set(bv1, bit);
        set1[bit] = 1;
    }
    for (i = 0; i < and_cnt; i++) {
        int bit = rand() % AND_SIZE;
        bv_set(bv2, bit);
        if (1 == set1[bit] && !set2[bit]) {
            count++;
            set2[bit] = 1;
        }
    }

    and_bv = bv_and(bv1, bv2);

    Aiequal(count, and_bv->count);
    for (i = 0; i < AND_SIZE; i++) {
        Aiequal(set2[i], bv_get(and_bv, i));
    }

    bv1 = bv_and_x(bv1, bv2);
    Assert(bv_eq(bv1, and_bv), "BitVectors should be equal");

    bv_destroy(bv2);
    bv_destroy(and_bv);

    bv2 = bv_new();
    and_bv = bv_and(bv1, bv2);

    Assert(bv_eq(bv2, and_bv), "ANDed BitVector should be empty");


    bv_destroy(bv1);
    bv_destroy(bv2);
    bv_destroy(and_bv);

    bv1 = bv_new();
    bv_not_x(bv1);
    bv2 = bv_new();
    bv_set(bv2, 10);
    bv_set(bv2, 11);
    bv_set(bv2, 20);
    and_bv = bv_and(bv1, bv2);

    Assert(bv_eq(bv2, and_bv), "ANDed BitVector should be equal");

    bv_destroy(bv1);
    bv_destroy(bv2);
    bv_destroy(and_bv);
}

static void test_bv_or(tst_case *tc, void *data)
{
#   define OR_SIZE 1000
    static const int or_cnt = 500;
    BitVector *or_bv;
    BitVector *bv1 = bv_new();
    BitVector *bv2 = bv_new();
    char set[OR_SIZE];
    int i;
    int count = 0;
    (void)data;

    memset(set, 0, OR_SIZE);
    for (i = 0; i < or_cnt; i++) {
        int bit = rand() % OR_SIZE;
        bv_set(bv1, bit);
        if (0 == set[bit]) {
            count++;
            set[bit] = 1;
        }
    }
    for (i = 0; i < or_cnt; i++) {
        int bit = rand() % OR_SIZE;
        bv_set(bv2, bit);
        if (0 == set[bit]) {
            count++;
            set[bit] = 1;
        }
    }

    or_bv = bv_or(bv1, bv2);

    Aiequal(count, or_bv->count);
    for (i = 0; i < OR_SIZE; i++) {
        Aiequal(set[i], bv_get(or_bv, i));
    }

    bv1 = bv_or_x(bv1, bv2);
    Assert(bv_eq(bv1, or_bv), "BitVectors should be equal");

    bv_destroy(bv2);
    bv_destroy(or_bv);

    bv2 = bv_new();
    or_bv = bv_or(bv1, bv2);

    Assert(bv_eq(bv1, or_bv), "ORed BitVector equals bv1");

    bv_destroy(bv1);
    bv_destroy(bv2);
    bv_destroy(or_bv);
}

static void test_bv_xor(tst_case *tc, void *data)
{
#   define XOR_SIZE 1000
    static const int xor_cnt = 500;
    BitVector *xor_bv;
    BitVector *bv1 = bv_new();
    BitVector *bv2 = bv_new();
    char set[XOR_SIZE];
    char set1[XOR_SIZE];
    char set2[XOR_SIZE];
    int i;
    int count = 0;
    (void)data;

    memset(set, 0, XOR_SIZE);
    memset(set1, 0, XOR_SIZE);
    memset(set2, 0, XOR_SIZE);
    for (i = 0; i < xor_cnt; i++) {
        int bit = rand() % XOR_SIZE;
        bv_set(bv1, bit);
        set1[bit] = 1;
    }
    for (i = 0; i < xor_cnt; i++) {
        int bit = rand() % XOR_SIZE;
        bv_set(bv2, bit);
        set2[bit] = 1;
    }
    for (i = 0; i < XOR_SIZE; i++) {
        if (set1[i] != set2[i]) {
            set[i] = 1;
            count++;
        }
    }

    xor_bv = bv_xor(bv1, bv2);

    Aiequal(count, xor_bv->count);
    for (i = 0; i < XOR_SIZE; i++) {
        if (!Aiequal(set[i], bv_get(xor_bv, i))) {
            Tmsg("At position %d, bv1 is %d and bv2 is %d\n", i,
                 bv_get(bv1, i), bv_get(bv2, i));
        }
    }

    bv1 = bv_xor_x(bv1, bv2);
    Assert(bv_eq(bv1, xor_bv), "BitVectors should be equal");

    bv_destroy(bv2);
    bv_destroy(xor_bv);

    bv2 = bv_new();
    xor_bv = bv_xor(bv1, bv2);

    Assert(bv_eq(bv1, xor_bv), "XORed BitVector equals bv1");

    bv_destroy(bv1);
    bv_destroy(bv2);
    bv_destroy(xor_bv);
}

static void test_bv_not(tst_case *tc, void *data)
{
    BitVector *bv = bv_new(), *not_bv;
    int i;
    (void)data;
    set_bits(bv, "1, 5, 25, 41, 97, 185");
    Aiequal(186, bv->size);

    not_bv = bv_not(bv);
    Aiequal(bv->count, not_bv->count);
    for (i = 0; i < 300; i++) {
        Aiequal(1 - bv_get(bv, i), bv_get(not_bv, i));
    }

    bv_not_x(bv);
    Assert(bv_eq(bv, not_bv), "BitVectors equal");

    bv_destroy(bv);
    bv_destroy(not_bv);
}

static void test_bv_combined_boolean_ops(tst_case *tc, void *data)
{
    BitVector *bv1 = bv_new();
    BitVector *bv2 = bv_new();
    BitVector *bv3;
    BitVector *bv4;
    BitVector *bv5;
    BitVector *bv_empty = bv_new();
    (void)data;

    set_bits(bv1, "1, 5, 7");
    set_bits(bv2, "1, 8, 20");

    bv3 = bv_not(bv1);
    Aiequal(bv3->size, bv1->size);

    bv4 = bv_and(bv1, bv3);
    Assert(bv_eq(bv4, bv_empty), "bv & ~bv == empty BitVector");
    bv_destroy(bv4);

    bv4 = bv_and(bv2, bv3);
    bv5 = set_bits(bv_new(), "8, 20");
    Assert(bv_eq(bv4, bv5), "~[1,5,7] & [1,8,20] == [8,20]");
    bv_destroy(bv4);
    bv_destroy(bv5);

    bv4 = bv_or(bv1, bv3);
    bv5 = bv_not(bv_empty);
    Assert(bv_eq(bv4, bv5), "bv | ~bv == all 1s");
    bv_destroy(bv4);
    bv_destroy(bv5);

    bv4 = bv_or(bv2, bv3);
    bv5 = bv_not_x(set_bits(bv_new(), "5, 7"));
    Assert(bv_eq(bv4, bv5), "~[1,5,7] | [1,8,20] == ~[5, 7]");
    bv_destroy(bv4);
    bv_destroy(bv5);

    bv4 = bv_xor(bv1, bv3);
    bv5 = bv_not(bv_empty);
    Assert(bv_eq(bv4, bv5), "bv ^ ~bv == full BitVector");
    bv_destroy(bv4);
    bv_destroy(bv5);

    bv4 = bv_xor(bv2, bv3);
    bv5 = bv_not_x(set_bits(bv_new(), "5, 7, 8, 20"));
    Assert(bv_eq(bv4, bv5), "~[1,5,7] ^ [1,8,20] == ~[5, 7, 8, 20]");
    bv_destroy(bv4);
    bv_destroy(bv5);

    bv_destroy(bv1);
    bv_destroy(bv2);
    bv_destroy(bv3);
    bv_destroy(bv_empty);
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
    BitVector *bv = bv_new_capa(BV_SCAN_SIZE);
    BitVector *not_bv;
    (void)data; /* suppress unused argument warning */

    /*
    clock_t t = clock(), total_t = 0;
    */

    for (i = BV_SCAN_INC; i < BV_SCAN_SIZE; i += BV_SCAN_INC) {
        bv_set_fast(bv, i);
    }

    not_bv = bv_not(bv);
    /*
    t = clock() - t;
    total_t += t;
    printf("bv_set took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    t = clock();
    */

    for (i = BV_SCAN_INC; i < BV_SCAN_SIZE; i += BV_SCAN_INC) {
        Aiequal(i, bv_scan_next_from(bv, i - BV_SCAN_INC / 2));
        Aiequal(i, bv_scan_next_unset_from(not_bv, i - BV_SCAN_INC / 2));
    }
    Aiequal(-1, bv_scan_next_from(bv, i - BV_SCAN_INC / 2));
    Aiequal(-1, bv_scan_next_unset_from(not_bv, i - BV_SCAN_INC / 2));

    /*
    t = clock() - t;
    total_t += t;
    printf("bv_scan_next_from took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    t = clock();
    */

    bv_scan_reset(bv);
    bv_scan_reset(not_bv);
    for (i = BV_SCAN_INC; i < BV_SCAN_SIZE; i += BV_SCAN_INC) {
        Aiequal(i, bv_scan_next(bv));
        Aiequal(i, bv_scan_next_unset(not_bv));
    }
    Aiequal(-1, bv_scan_next(bv));
    Aiequal(-1, bv_scan_next_unset(not_bv));

    /*
    t = clock() - t;
    total_t += t;
    printf("bv_scan_next took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    */

    bv_clear(bv);
    bv_destroy(not_bv);
    for (i = 0; i < BV_DENSE_SCAN_SIZE; i++) {
        bv_set(bv, i);
    }
    not_bv = bv_not(bv);

    /*
    t = clock();
    */

    for (i = 0; i < BV_DENSE_SCAN_SIZE; i++) {
        Aiequal(i, bv_scan_next_from(bv, i));
        Aiequal(i, bv_scan_next_unset_from(not_bv, i));
    }
    Aiequal(-1, bv_scan_next_from(bv, i));
    Aiequal(-1, bv_scan_next_unset_from(not_bv, i));

    /*
    t = clock() - t;
    total_t += t;
    printf("dense bv_scan_next_from took %0.3f secs\n", (double) (t) / CLOCKS_PER_SEC);
    t = clock();
    */

    bv_scan_reset(bv);
    bv_scan_reset(not_bv);
    for (i = 0; i < BV_DENSE_SCAN_SIZE; i++) {
        Aiequal(i, bv_scan_next(bv));
        Aiequal(i, bv_scan_next_unset(not_bv));
    }
    Aiequal(-1, bv_scan_next(bv));
    Aiequal(-1, bv_scan_next_unset(not_bv));

    /*
    t = clock() - t;
    total_t += t;
    printf("dense bv_scan_next took %0.3f secs\n", (double) t / CLOCKS_PER_SEC);
    printf("total time %0.3f seconds\n",
           (double) total_t / CLOCKS_PER_SEC);
    */

    bv_destroy(bv);
    bv_destroy(not_bv);
}


tst_suite *ts_bitvector(tst_suite * suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_bv, NULL);
    tst_run_test(suite, test_bv_eq_hash, NULL);
    tst_run_test(suite, test_bv_and, NULL);
    tst_run_test(suite, test_bv_or, NULL);
    tst_run_test(suite, test_bv_xor, NULL);
    tst_run_test(suite, test_bv_not, NULL);
    tst_run_test(suite, test_bv_combined_boolean_ops, NULL);
    tst_run_test(suite, test_bv_scan, NULL);
    tst_run_test(suite, test_bv_scan_stress, NULL);

    return suite;
}

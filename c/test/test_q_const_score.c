#include "search.h"
#include "test.h"

static Symbol num;

extern void prepare_filter_index(Store *store);

extern void check_hits(TestCase *tc, Searcher *searcher, Query *query,
                       char *expected_hits, int top);

static void test_const_score_query(TestCase *tc, void *data)
{
    Searcher *searcher = (Searcher *)data;
    Query *q;
    q = csq_new_nr(rfilt_new(num, "2", "6", true, true));
    check_hits(tc, searcher, q, "2,3,4,5,6", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, "2", "6", true, false));
    check_hits(tc, searcher, q, "2,3,4,5", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, "2", "6", false, true));
    check_hits(tc, searcher, q, "3,4,5,6", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, "2", "6", false, false));
    check_hits(tc, searcher, q, "3,4,5", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, "6", NULL, true, false));
    check_hits(tc, searcher, q, "6,7,8,9", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, "6", NULL, false, false));
    check_hits(tc, searcher, q, "7,8,9", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, NULL, "2", false, true));
    check_hits(tc, searcher, q, "0,1,2", -1);
    q_deref(q);
    q = csq_new_nr(rfilt_new(num, NULL, "2", false, false));
    check_hits(tc, searcher, q, "0,1", -1);
    q_deref(q);
}

static void test_const_score_query_hash(TestCase *tc, void *data)
{
    Filter *f;
    Query *q1, *q2;
    (void)data;
    f = rfilt_new(num, "2", "6", true, true);
    q1 = csq_new_nr(f);
    q2 = csq_new(f);

    Assert(q_eq(q1, q1), "Test same queries are equal");
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    q_deref(q2);

    q2 = csq_new_nr(rfilt_new(num, "2", "6", true, true));
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    q_deref(q2);

    q2 = csq_new_nr(rfilt_new(num, "3", "6", true, true));
    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
    Assert(!q_eq(q1, q2), "Queries are not equal");
    q_deref(q2);
    q_deref(q1);
}

TestSuite *ts_q_const_score(TestSuite *suite)
{
    Store *store = open_ram_store();
    IndexReader *ir;
    Searcher *searcher;

    num = intern("num");

    suite = ADD_SUITE(suite);

    prepare_filter_index(store);
    ir = ir_open(store);
    searcher = isea_new(ir);

    tst_run_test(suite, test_const_score_query, (void *)searcher);
    tst_run_test(suite, test_const_score_query_hash, NULL);

    store_deref(store);
    searcher_close(searcher);
    return suite;
}

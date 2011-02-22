#include "search.h"
#include "test.h"

static Symbol num, flipflop;

extern void prepare_filter_index(Store *store);

extern void check_hits(TestCase *tc, Searcher *searcher, Query *query,
                       char *expected_hits, int top);

static void test_filtered_query(TestCase *tc, void *data)
{
    Searcher *searcher = (Searcher *)data;
    Query *q;
    q = fq_new(maq_new(), rfilt_new(num, "2", "6", true, true));
    check_hits(tc, searcher, q, "2,3,4,5,6", -1);
    q_deref(q);
    q = fq_new(tq_new(flipflop, "on"),
                  rfilt_new(num, "2", "6", true, true));
    check_hits(tc, searcher, q, "2,4,6", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, "2", "6", true, false));
    check_hits(tc, searcher, q, "2,3,4,5", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, "2", "6", false, true));
    check_hits(tc, searcher, q, "3,4,5,6", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, "2", "6", false, false));
    check_hits(tc, searcher, q, "3,4,5", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, "6", NULL, true, false));
    check_hits(tc, searcher, q, "6,7,8,9", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, "6", NULL, false, false));
    check_hits(tc, searcher, q, "7,8,9", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, NULL, "2", false, true));
    check_hits(tc, searcher, q, "0,1,2", -1);
    q_deref(q);
    q = fq_new(maq_new(), rfilt_new(num, NULL, "2", false, false));
    check_hits(tc, searcher, q, "0,1", -1);
    q_deref(q);
}

TestSuite *ts_q_filtered(TestSuite *suite)
{
    Store *store = open_ram_store();
    IndexReader *ir;
    Searcher *searcher;

    num      = intern("num");
    flipflop = intern("flipflop");

    suite = ADD_SUITE(suite);

    prepare_filter_index(store);
    ir = ir_open(store);
    searcher = isea_new(ir);

    tst_run_test(suite, test_filtered_query, (void *)searcher);

    store_deref(store);
    searcher->close(searcher);
    return suite;
}

#include "test.h"
#include "search.h"

#define ARRAY_SIZE 20

static const char *field = "field";

static void add_doc(char *text, IndexWriter *iw)
{
    Document *doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(field), text));
    iw_add_doc(iw, doc);
    doc_destroy(doc);
}

extern void check_hits(tst_case *tc, Searcher *searcher, Query *query,
                       char *expected_hits, char top);

static void do_prefix_test(tst_case *tc, Searcher *searcher, char *qstr,
                    char *expected_hits, int pre_len, float min_sim)
{
    Query *fq = fuzq_new_conf(field, qstr, min_sim, pre_len, 10);
    check_hits(tc, searcher, fq, expected_hits, -1);
    q_deref(fq);
}

static void test_fuzziness(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw;
    IndexReader *ir;
    Searcher *sea;
    TopDocs *top_docs;
    Query *q;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    index_create(store, fis);
    fis_deref(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    add_doc("aaaaa", iw);
    add_doc("aaaab", iw);
    add_doc("aaabb", iw);
    add_doc("aabbb", iw);
    add_doc("abbbb", iw);
    add_doc("bbbbb", iw);
    add_doc("ddddd", iw);
    add_doc("ddddddddddddddddddddd", iw);   /* test max_distances problem */
    add_doc("aaaaaaaaaaaaaaaaaaaaaaa", iw); /* test max_distances problem */
    iw_close(iw);


    ir = ir_open(store);
    sea = isea_new(ir);

    q = fuzq_new_conf(field, "aaaaa", 0.0, 5, 10);
    q_deref(q);

    do_prefix_test(tc, sea, "aaaaaaaaaaaaaaaaaaaaaa", "8", 1, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 0, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 1, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 2, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 3, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1", 4, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0", 5, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0", 6, 0.0);

    do_prefix_test(tc, sea, "xxxxx", "", 0, 0.0);

    do_prefix_test(tc, sea, "aaccc", "", 0, 0.0);

    do_prefix_test(tc, sea, "aaaac", "0,1,2", 0, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 1, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 2, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 3, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1", 4, 0.0);
    do_prefix_test(tc, sea, "aaaac", "", 5, 0.0);

    do_prefix_test(tc, sea, "ddddX", "6", 0, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 1, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 2, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 3, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 4, 0.0);
    do_prefix_test(tc, sea, "ddddX", "", 5, 0.0);

    q = fuzq_new_conf("anotherfield", "ddddX", 0.0, 10, 100);
    top_docs = searcher_search(sea, q, 0, 1, NULL, NULL, NULL);
    q_deref(q);
    Aiequal(0, top_docs->total_hits);
    td_destroy(top_docs);

    searcher_close(sea);
}

static void test_fuzziness_long(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw;
    Searcher *sea;
    IndexReader *ir;
    TopDocs *top_docs;
    Query *q;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    index_create(store, fis);
    fis_deref(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    add_doc("aaaaaaa", iw);
    add_doc("segment", iw);
    iw_close(iw);
    ir = ir_open(store);
    sea = isea_new(ir);

    /* not similar enough: */
    do_prefix_test(tc, sea, "xxxxx", "", 0, 0.0);

    /* edit distance to "aaaaaaa" = 3, this matches because the string is longer than
     * in testDefaultFuzziness so a bigger difference is allowed: */
    do_prefix_test(tc, sea, "aaaaccc", "0", 0, 0.0);

    /* now with prefix */
    do_prefix_test(tc, sea, "aaaaccc", "0", 1, 0.0);
    do_prefix_test(tc, sea, "aaaaccc", "0", 4, 0.0);
    do_prefix_test(tc, sea, "aaaaccc", "", 5, 0.0);

    /* no match, more than half of the characters is wrong: */
    do_prefix_test(tc, sea, "aaacccc", "", 0, 0.0);

    /* now with prefix */
    do_prefix_test(tc, sea, "aaacccc", "", 1, 0.0);

    /* "student" and "stellent" are indeed similar to "segment" by default: */
    do_prefix_test(tc, sea, "student", "1", 0, 0.0);

    /* now with prefix */
    do_prefix_test(tc, sea, "student", "", 2, 0.0);
    do_prefix_test(tc, sea, "stellent", "", 2, 0.0);

    /* "student" doesn't match anymore thanks to increased min-similarity: */
    q = fuzq_new_conf(field, "student", (float)0.6, 0, 100);
    top_docs = searcher_search(sea, q, 0, 1, NULL, NULL, NULL);
    q_deref(q);
    Aiequal(0, top_docs->total_hits);
    td_destroy(top_docs);

    searcher_close(sea);
}

static void test_fuzzy_query_hash(tst_case *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = fuzq_new_conf("A", "a", (float)0.4, 2, 100);
    q2 = fuzq_new_conf("A", "a", (float)0.4, 2, 100);

    Assert(q_eq(q1, q1), "Test same queries are equal");
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    q_deref(q2);

    q2 = fuzq_new_conf("A", "a", (float)0.4, 0, 100);
    Assert(q_hash(q1) != q_hash(q2), "prelen differs");
    Assert(!q_eq(q1, q2), "prelen differs");
    q_deref(q2);

    q2 = fuzq_new_conf("A", "a", (float)0.5, 2, 100);
    Assert(q_hash(q1) != q_hash(q2), "similarity differs");
    Assert(!q_eq(q1, q2), "similarity differs");
    q_deref(q2);

    q2 = fuzq_new_conf("A", "b", (float)0.4, 2, 100);
    Assert(q_hash(q1) != q_hash(q2), "term differs");
    Assert(!q_eq(q1, q2), "term differs");
    q_deref(q2);

    q2 = fuzq_new_conf("B", "a", (float)0.4, 2, 100);
    Assert(q_hash(q1) != q_hash(q2), "field differs");
    Assert(!q_eq(q1, q2), "field differs");
    q_deref(q2);

    q_deref(q1);
}

tst_suite *ts_q_fuzzy(tst_suite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_fuzziness, (void *)store);
    tst_run_test(suite, test_fuzziness_long, (void *)store);
    tst_run_test(suite, test_fuzzy_query_hash, (void *)store);

    store_deref(store);
    return suite;
}

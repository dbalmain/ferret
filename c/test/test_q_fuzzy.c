#include "search.h"
#include "test.h"

#define ARRAY_SIZE 20

static Symbol field;

static void add_doc(char *text, IndexWriter *iw)
{
    Document *doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(field), text));
    iw_add_doc(iw, doc);
    doc_destroy(doc);
}

void check_hits(TestCase *tc, Searcher *searcher, Query *query,
                char *expected_hits, int top);
void check_to_s(TestCase *tc, Query *query, Symbol field, char *q_str);

static void do_prefix_test(TestCase *tc, Searcher *searcher, char *qstr,
                    char *expected_hits, int pre_len, float min_sim)
{
    Query *fq = fuzq_new_conf(field, qstr, min_sim, pre_len, 10);
    check_hits(tc, searcher, fq, expected_hits, -1);
    q_deref(fq);
}

static void test_fuzziness(TestCase *tc, void *data)
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
    check_hits(tc, sea, q, "0", -1);
    q_deref(q);

    q = fuzq_new(I("not a field"), "aaaaa");
    check_hits(tc, sea, q, "", -1);
    q_deref(q);

    /* test prefix length */
    do_prefix_test(tc, sea, "aaaaaaaaaaaaaaaaaaaaaa", "8", 1, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 0, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 1, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 2, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 3, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0,1", 4, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0", 5, 0.0);
    do_prefix_test(tc, sea, "aaaaa", "0", 6, 0.0);
    /* test where term will equal prefix but not whole query string */
    do_prefix_test(tc, sea, "aaaaaaa", "0", 5, 0.0);

    /* test minimum similarity */
    do_prefix_test(tc, sea, "aaaaa", "0,1,2,3", 0, 0.2);
    do_prefix_test(tc, sea, "aaaaa", "0,1,2", 1, 0.4);
    do_prefix_test(tc, sea, "aaaaa", "0,1", 1, 0.6);
    do_prefix_test(tc, sea, "aaaaa", "0", 1, 0.8);

    /* test where no terms will have any similarity */
    do_prefix_test(tc, sea, "xxxxx", "", 0, 0.0);

    /* test where no terms will have enough similarity to match */
    do_prefix_test(tc, sea, "aaccc", "", 0, 0.0);

    /* test prefix length but with non-matching term (aaaac does not exit in
     * the index) */
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 0, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 1, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 2, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1,2", 3, 0.0);
    do_prefix_test(tc, sea, "aaaac", "0,1", 4, 0.0);
    do_prefix_test(tc, sea, "aaaac", "", 5, 0.0);

    /* test really long string never matches */
    do_prefix_test(tc, sea, "ddddX", "6", 0, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 1, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 2, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 3, 0.0);
    do_prefix_test(tc, sea, "ddddX", "6", 4, 0.0);
    do_prefix_test(tc, sea, "ddddX", "", 5, 0.0);

    /* test non-existing field doesn't break search */
    q = fuzq_new_conf(I("anotherfield"), "ddddX", 0.0, 10, 100);
    top_docs = searcher_search(sea, q, 0, 1, NULL, NULL, NULL);
    q_deref(q);
    Aiequal(0, top_docs->total_hits);
    td_destroy(top_docs);

    searcher_close(sea);
}

static void test_fuzziness_long(TestCase *tc, void *data)
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

/**
 * Test query->to_s functionality
 */
static void test_fuzzy_query_to_s(TestCase *tc, void *data)
{
    Query *q;
    (void)data;

    q = fuzq_new_conf(I("A"), "a", 0.4f, 2, 100);
    check_to_s(tc, q, I("A"), "a~0.4");
    check_to_s(tc, q, I("B"), "A:a~0.4");
    q_deref(q);

    q = fuzq_new_conf(I("field"), "mispell", 0.5f, 2, 100);
    check_to_s(tc, q, I("field"), "mispell~");
    check_to_s(tc, q, I("notfield"), "field:mispell~");
    q_deref(q);

}

/**
 * Test query hashing functionality
 */
static void test_fuzzy_query_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = fuzq_new_conf(I("A"), "a", 0.4f, 2, 100);
    q2 = fuzq_new_conf(I("A"), "a", 0.4f, 2, 100);

    Assert(q_eq(q1, q1), "Test same queries are equal");
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    q_deref(q2);

    q2 = fuzq_new_conf(I("A"), "a", 0.4f, 0, 100);
    Assert(q_hash(q1) != q_hash(q2), "prelen differs");
    Assert(!q_eq(q1, q2), "prelen differs");
    q_deref(q2);

    q2 = fuzq_new_conf(I("A"), "a", 0.5f, 2, 100);
    Assert(q_hash(q1) != q_hash(q2), "similarity differs");
    Assert(!q_eq(q1, q2), "similarity differs");
    q_deref(q2);

    q2 = fuzq_new_conf(I("A"), "b", 0.4f, 2, 100);
    Assert(q_hash(q1) != q_hash(q2), "term differs");
    Assert(!q_eq(q1, q2), "term differs");
    q_deref(q2);

    q2 = fuzq_new_conf(I("B"), "a", 0.4f, 2, 100);
    Assert(q_hash(q1) != q_hash(q2), "field differs");
    Assert(!q_eq(q1, q2), "field differs");
    q_deref(q2);

    q_deref(q1);
}

TestSuite *ts_q_fuzzy(TestSuite *suite)
{
    Store *store = open_ram_store();

    field = intern("field");

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_fuzziness, (void *)store);
    tst_run_test(suite, test_fuzziness_long, (void *)store);
    tst_run_test(suite, test_fuzzy_query_hash, (void *)store);
    tst_run_test(suite, test_fuzzy_query_to_s, (void *)store);

    store_deref(store);
    return suite;
}

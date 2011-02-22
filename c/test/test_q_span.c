#include "search.h"
#include "test.h"

#define ARRAY_SIZE 20
#define TEST_SE(query, ir, expected) do { \
    SpanEnum *__se = ((SpanQuery *)query)->get_spans(query, ir); \
    char *__tmp = __se->to_s(__se);                              \
    Asequal(expected, __tmp);                                     \
    __se->destroy(__se);                                          \
    free(__tmp);                                                  \
} while(0)

static Symbol field;

static void add_doc(char *text, IndexWriter *iw)
{
    Document *doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(field), text));
    iw_add_doc(iw, doc);
    doc_destroy(doc);
}


static void span_test_setup(Store *store)
{
    char **d;
    IndexWriter *iw;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    char *data[] = {
        "start finish one two three four five six seven",
        "start one finish two three four five six seven",
        "start one two finish three four five six seven flip",
        "start one two three finish four five six seven",
        "start one two three four finish five six seven flip",
        "start one two three four five finish six seven",
        "start one two three four five six finish seven eight",
        "start one two three four five six seven finish eight nine",
        "start one two three four five six finish seven eight",
        "start one two three four five finish six seven",
        "start one two three four finish five six seven",
        "start one two three finish four five six seven",
        "start one two finish three four five six seven flop",
        "start one finish two three four five six seven",
        "start finish one two three four five six seven toot",
        "start start  one two three four five six seven",
        "finish start one two three four five six seven flip flop",
        "finish one start two three four five six seven",
        "finish one two start three four five six seven",
        "finish one two three start four five six seven flip",
        "finish one two three four start five six seven",
        "finish one two three four five start six seven flip flop",
        "finish one two three four five six start seven eight",
        "finish one two three four five six seven start eight nine",
        "finish one two three four five six start seven eight",
        "finish one two three four five start six seven",
        "finish one two three four start five six seven",
        "finish one two three start four five six seven flop",
        "finish one two start three four five six seven",
        "finish one start two three four five six seven flip",
        "finish start one two three four five six seven",
        NULL
    };
    index_create(store, fis);
    fis_deref(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    for (d = data; *d != NULL; d++) {
        add_doc(*d, iw);
    }
    iw_close(iw);
}

extern void check_hits(TestCase *tc, Searcher *searcher, Query *query,
                       char *expected_hits, int top);

static void test_span_term(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *tq;

    ir = ir_open(store);
    sea = isea_new(ir);

    tq = spantq_new(I("notafield"), "nine");
    check_hits(tc, sea, tq, "", -1);
    TEST_SE(tq, ir, "SpanTermEnum(span_terms(notafield:nine))@START");
    q_deref(tq);

    tq = spantq_new(field, "nine");
    check_hits(tc, sea, tq, "7,23", -1);
    TEST_SE(tq, ir, "SpanTermEnum(span_terms(field:nine))@START");
    q_deref(tq);

    tq = spantq_new(field, "eight");
    check_hits(tc, sea, tq, "6,7,8,22,23,24", -1);
    TEST_SE(tq, ir, "SpanTermEnum(span_terms(field:eight))@START");
    q_deref(tq);

    searcher_close(sea);
}

static void test_span_term_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = spantq_new(I("A"), "a");

    q2 = spantq_new(I("A"), "a");
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are equal");
    q_deref(q2);

    q2 = spantq_new(I("A"), "b");
    Assert(q_hash(q1) != q_hash(q2), "Terms differ");
    Assert(!q_eq(q1, q2), "Terms differ");
    q_deref(q2);

    q2 = spantq_new(I("B"), "a");
    Assert(q_hash(q1) != q_hash(q2), "Fields differ");
    Assert(!q_eq(q1, q2), "Fields differ");
    q_deref(q2);

    q_deref(q1);
}

static void test_span_multi_term(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *mtq;

    ir = ir_open(store);
    sea = isea_new(ir);

    mtq = spanmtq_new(I("notafield"));
    check_hits(tc, sea, mtq, "", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(notafield:[]))@START");

    spanmtq_add_term(mtq, "nine");
    check_hits(tc, sea, mtq, "", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(notafield:[nine]))@START");

    spanmtq_add_term(mtq, "finish");
    check_hits(tc, sea, mtq, "", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(notafield:[nine,finish]))@START");
    q_deref(mtq);

    mtq = spanmtq_new_conf(field, 4);
    check_hits(tc, sea, mtq, "", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(field:[]))@START");

    spanmtq_add_term(mtq, "nine");
    check_hits(tc, sea, mtq, "7, 23", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(field:[nine]))@START");

    spanmtq_add_term(mtq, "flop");
    check_hits(tc, sea, mtq, "7, 12, 16, 21, 23, 27", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(field:[nine,flop]))@START");

    spanmtq_add_term(mtq, "toot");
    check_hits(tc, sea, mtq, "7, 12, 14, 16, 21, 23, 27", -1);
    TEST_SE(mtq, ir, "SpanTermEnum(span_terms(field:[nine,flop,toot]))@START");
    q_deref(mtq);

    searcher_close(sea);
}

static void test_span_multi_term_hash(TestCase *tc, void *data)
{
    Query *q1 = spanmtq_new_conf(field, 100);
    Query *q2 = spanmtq_new(field);
    (void)data, (void)tc;

    Assert(q_hash(q1) == q_hash(q2), "Queries should be equal");
    Assert(q_eq(q1, q1), "Same queries should be equal");
    Assert(q_eq(q1, q2), "Queries should be equal");

    spanmtq_add_term(q1, "word1");
    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
    Assert(!q_eq(q1, q2), "Queries should not be equal");

    spanmtq_add_term(q2, "word1");
    Assert(q_hash(q1) == q_hash(q2), "Queries should be equal");
    Assert(q_eq(q1, q2), "Queries should be equal");

    spanmtq_add_term(q1, "word2");
    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
    Assert(!q_eq(q1, q2), "Queries should not be equal");

    spanmtq_add_term(q2, "word2");
    Assert(q_hash(q1) == q_hash(q2), "Queries should be equal");
    Assert(q_eq(q1, q2), "Queries should be equal");

    q_deref(q1);
    q_deref(q2);
}

static void test_span_prefix(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *prq;
    char *tmp;

    ir = ir_open(store);
    sea = isea_new(ir);

    prq = spanprq_new(I("notafield"), "fl");
    tmp = prq->to_s(prq, I("notafield"));
    Asequal("fl*", tmp);
    free(tmp);
    tmp = prq->to_s(prq, I("foo"));
    Asequal("notafield:fl*", tmp);
    free(tmp);
    check_hits(tc, sea, prq, "", -1);
    q_deref(prq);

    prq = spanprq_new(field, "fl");
    tmp = prq->to_s(prq, I("field"));
    Asequal("fl*", tmp);
    free(tmp);
    check_hits(tc, sea, prq, "2, 4, 12, 16, 19, 21, 27, 29", -1);
    q_deref(prq);

    searcher_close(sea);
}

static void test_span_prefix_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;
    q1 = spanprq_new(I("A"), "a");

    q2 = spanprq_new(I("A"), "a");
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "SpanPrefixQueries are equal");
    Assert(q_eq(q1, q1), "SpanPrefixQueries are same");
    q_deref(q2);

    q2 = spanprq_new(I("A"), "b");
    Assert(q_hash(q1) != q_hash(q2), "SpanPrefixQueries are not equal");
    Assert(!q_eq(q1, q2), "SpanPrefixQueries are not equal");
    q_deref(q2);

    q2 = spanprq_new(I("B"), "a");
    Assert(q_hash(q1) != q_hash(q2), "SpanPrefixQueries are not equal");
    Assert(!q_eq(q1, q2), "SpanPrefixQueries are not equal");
    q_deref(q2);

    q_deref(q1);
}

static void test_span_first(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *q;

    ir = ir_open(store);
    sea = isea_new(ir);

    q = spanfq_new_nr(spantq_new(field, "finish"), 1);
    check_hits(tc, sea, q, "16,17,18,19,20,21,22,23,24,25,26,27,28,29,30", -1);
    TEST_SE(q, ir, "SpanFirstEnum(span_first(span_terms(field:finish), 1))");
    q_deref(q);

    q = spanfq_new_nr(spantq_new(field, "finish"), 5);
    check_hits(tc, sea, q, "0,1,2,3,11,12,13,14,16,17,18,19,20,21,22,23,24,25,"
               "26,27,28,29,30", -1);
    TEST_SE(q, ir, "SpanFirstEnum(span_first(span_terms(field:finish), 5))");
    q_deref(q);

    searcher_close(sea);
}

static void test_span_first_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = spanfq_new_nr(spantq_new(I("A"), "a"), 5);

    q2 = spanfq_new_nr(spantq_new(I("A"), "a"), 5);
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are equal");
    q_deref(q2);

    q2 = spanfq_new_nr(spantq_new(I("A"), "a"), 3);
    Assert(q_hash(q1) != q_hash(q2), "Ends differ");
    Assert(!q_eq(q1, q2), "Ends differ");
    q_deref(q2);

    q2 = spanfq_new_nr(spantq_new(I("A"), "b"), 5);
    Assert(q_hash(q1) != q_hash(q2), "Terms differ");
    Assert(!q_eq(q1, q2), "Terms differ");
    q_deref(q2);

    q_deref(q1);
}

static void test_span_or(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *q;

    ir = ir_open(store);
    sea = isea_new(ir);

    q = spanoq_new();
    check_hits(tc, sea, q, "", -1);
    TEST_SE(q, ir, "SpanOrEnum(span_or[])@START");

    spanoq_add_clause_nr(q, spantq_new(field, "flip"));
    check_hits(tc, sea, q, "2, 4, 16, 19, 21, 29", -1);
    TEST_SE(q, ir, "SpanTermEnum(span_terms(field:flip))@START");

    spanoq_add_clause_nr(q, spantq_new(field, "flop"));
    check_hits(tc, sea, q, "2, 4, 12, 16, 19, 21, 27, 29", -1);
    TEST_SE(q, ir, "SpanOrEnum(span_or[span_terms(field:flip),span_terms(field:flop)])@START");
    q_deref(q);

    searcher_close(sea);
}

static void test_span_or_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = spanoq_new();
    q2 = spanoq_new();
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are same");

    spanoq_add_clause_nr(q1, spantq_new(field, "a"));
    Assert(q_hash(q1) != q_hash(q2), "Clause counts differ");
    Assert(!q_eq(q1, q2), "Clause counts differ");

    spanoq_add_clause_nr(q2, spantq_new(field, "a"));
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are same");

    spanoq_add_clause_nr(q1, spantq_new(field, "b"));
    Assert(q_hash(q1) != q_hash(q2), "Clause counts differ");
    Assert(!q_eq(q1, q2), "Clause counts differ");

    spanoq_add_clause_nr(q2, spantq_new(field, "c"));
    Assert(q_hash(q1) != q_hash(q2), "2nd clause differs");
    Assert(!q_eq(q1, q2), "2nd clause differs");

    q_deref(q2);
    q_deref(q1);
}

static void test_span_near(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *q;

    ir = ir_open(store);
    sea = isea_new(ir);

    q = spannq_new(0, true);
    TEST_SE(q, ir, "SpanNearEnum(span_near[])@START");
    spannq_add_clause_nr(q, spantq_new(field, "start"));
    TEST_SE(q, ir, "SpanTermEnum(span_terms(field:start))@START");
    spannq_add_clause_nr(q, spantq_new(field, "finish"));
    TEST_SE(q, ir, "SpanNearEnum(span_near[span_terms(field:start),span_terms(field:finish)])@START");
    check_hits(tc, sea, q, "0, 14", -1);

    ((SpanNearQuery *)q)->in_order = false;
    check_hits(tc, sea, q, "0,14,16,30", -1);

    ((SpanNearQuery *)q)->in_order = true;
    ((SpanNearQuery *)q)->slop = 1;
    check_hits(tc, sea, q, "0,1,13,14", -1);

    ((SpanNearQuery *)q)->in_order = false;
    check_hits(tc, sea, q, "0,1,13,14,16,17,29,30", -1);

    ((SpanNearQuery *)q)->in_order = true;
    ((SpanNearQuery *)q)->slop = 4;
    check_hits(tc, sea, q, "0,1,2,3,4,10,11,12,13,14", -1);

    ((SpanNearQuery *)q)->in_order = false;
    check_hits(tc, sea, q, "0,1,2,3,4,10,11,12,13,14,16,17,18,19,20,26,27,"
               "28,29,30", -1);

    q_deref(q);

    q = spannq_new(0, true);
    spannq_add_clause_nr(q, spanprq_new(field, "fi"));
    spannq_add_clause_nr(q, spanprq_new(field, "fin"));
    spannq_add_clause_nr(q, spanprq_new(field, "si"));
    check_hits(tc, sea, q, "5, 9, 4, 10", -1);
    q_deref(q);

    searcher_close(sea);
}

static void test_span_near_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = spannq_new(0, false);
    q2 = spannq_new(0, false);
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are same");

    spanoq_add_clause_nr(q1, spantq_new(field, "a"));
    Assert(q_hash(q1) != q_hash(q2), "Clause counts differ");
    Assert(!q_eq(q1, q2), "Clause counts differ");

    spanoq_add_clause_nr(q2, spantq_new(field, "a"));
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are same");

    spanoq_add_clause_nr(q1, spantq_new(field, "b"));
    spanoq_add_clause_nr(q2, spantq_new(field, "b"));
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are same");

    ((SpanNearQuery *)q1)->in_order = true;
    Assert(q_hash(q1) != q_hash(q2), "%d == %d, in_order differs",
           q_hash(q1), q_hash(q2));
    Assert(!q_eq(q1, q2), "in_order differs");

    ((SpanNearQuery *)q2)->in_order = true;
    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");

    ((SpanNearQuery *)q2)->slop = 4;
    Assert(q_hash(q1) != q_hash(q2), "slop differs");
    Assert(!q_eq(q1, q2), "slop differs");

    ((SpanNearQuery *)q2)->slop = 0;
    spanoq_add_clause_nr(q1, spantq_new(field, "c"));
    Assert(q_hash(q1) != q_hash(q2), "Clause counts differ");
    Assert(!q_eq(q1, q2), "Clause counts differ");

    spanoq_add_clause_nr(q2, spantq_new(field, "d"));
    Assert(q_hash(q1) != q_hash(q2), "3rd clause differs");
    Assert(!q_eq(q1, q2), "3rd clause differs");

    q_deref(q2);

    q_deref(q1);
}

static void test_span_not(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *q, *nearq0, *nearq1;

    ir = ir_open(store);
    sea = isea_new(ir);

    nearq0 = spannq_new(4, true);
    TEST_SE(nearq0, ir, "SpanNearEnum(span_near[])@START");
    spannq_add_clause_nr(nearq0, spantq_new(field, "start"));
    TEST_SE(nearq0, ir, "SpanTermEnum(span_terms(field:start))@START");
    spannq_add_clause_nr(nearq0, spantq_new(field, "finish"));
    TEST_SE(nearq0, ir, "SpanNearEnum(span_near[span_terms(field:start),span_terms(field:finish)])@START");

    nearq1 = spannq_new(4, true);
    TEST_SE(nearq1, ir, "SpanNearEnum(span_near[])@START");
    spannq_add_clause_nr(nearq1, spantq_new(field, "two"));
    TEST_SE(nearq1, ir, "SpanTermEnum(span_terms(field:two))@START");
    spannq_add_clause_nr(nearq1, spantq_new(field, "five"));
    TEST_SE(nearq1, ir, "SpanNearEnum(span_near[span_terms(field:two),span_terms(field:five)])@START");


    q = spanxq_new(nearq0, nearq1);
    TEST_SE(q, ir, "SpanNotEnum(span_not(inc:<span_near[span_terms(field:start),span_terms(field:finish)]>, exc:<span_near[span_terms(field:two),span_terms(field:five)]>))");
    check_hits(tc, sea, q, "0,1,13,14", -1);
    q_deref(q);
    q_deref(nearq0);

    nearq0 = spannq_new(4, false);
    spannq_add_clause_nr(nearq0, spantq_new(field, "start"));
    spannq_add_clause_nr(nearq0, spantq_new(field, "finish"));

    q = spanxq_new_nr(nearq0, nearq1);
    check_hits(tc, sea, q, "0,1,13,14,16,17,29,30", -1);
    q_deref(q);

    nearq0 = spannq_new(4, true);
    spannq_add_clause_nr(nearq0, spantq_new(field, "start"));
    spannq_add_clause_nr(nearq0, spantq_new(field, "two"));

    nearq1 = spannq_new(8, false);
    spannq_add_clause_nr(nearq1, spantq_new(field, "finish"));
    spannq_add_clause_nr(nearq1, spantq_new(field, "five"));

    q = spanxq_new_nr(nearq0, nearq1);
    check_hits(tc, sea, q, "2,3,4,5,6,7,8,9,10,11,12,15", -1);
    q_deref(q);

    searcher_close(sea);
}

static void test_span_not_hash(TestCase *tc, void *data)
{
    Query *q1, *q2;
    (void)data;

    q1 = spanxq_new_nr(spantq_new(I("A"), "a"),
                       spantq_new(I("A"), "b"));
    q2 = spanxq_new_nr(spantq_new(I("A"), "a"),
                       spantq_new(I("A"), "b"));

    Aiequal(q_hash(q1), q_hash(q2));
    Assert(q_eq(q1, q2), "Queries are equal");
    Assert(q_eq(q1, q1), "Queries are equal");
    q_deref(q2);

    q2 = spanxq_new_nr(spantq_new(I("A"), "a"),
                       spantq_new(I("A"), "c"));
    Assert(q_hash(q1) != q_hash(q2), "exclude queries differ");
    Assert(!q_eq(q1, q2), "exclude queries differ");
    q_deref(q2);

    q2 = spanxq_new_nr(spantq_new(I("A"), "x"),
                       spantq_new(I("A"), "b"));
    Assert(q_hash(q1) != q_hash(q2), "include queries differ");
    Assert(!q_eq(q1, q2), "include queries differ");
    q_deref(q2);

    q2 = spanxq_new_nr(spantq_new(I("B"), "a"),
                       spantq_new(I("B"), "b"));
    Assert(q_hash(q1) != q_hash(q2), "fields differ");
    Assert(!q_eq(q1, q2), "fields differ");
    q_deref(q2);

    q_deref(q1);
}

TestSuite *ts_q_span(TestSuite *suite)
{
    Store *store = open_ram_store();
    field = intern("field");
    span_test_setup(store);

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_span_term, (void *)store);
    tst_run_test(suite, test_span_term_hash, NULL);

    tst_run_test(suite, test_span_multi_term, (void *)store);
    tst_run_test(suite, test_span_multi_term_hash, NULL);

    tst_run_test(suite, test_span_prefix, (void *)store);
    tst_run_test(suite, test_span_prefix_hash, NULL);

    tst_run_test(suite, test_span_first, (void *)store);
    tst_run_test(suite, test_span_first_hash, NULL);

    tst_run_test(suite, test_span_or, (void *)store);
    tst_run_test(suite, test_span_or_hash, NULL);

    tst_run_test(suite, test_span_near, (void *)store);
    tst_run_test(suite, test_span_near_hash, NULL);

    tst_run_test(suite, test_span_not, (void *)store);
    tst_run_test(suite, test_span_not_hash, NULL);

    store_deref(store);
    return suite;
}

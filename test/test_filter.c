#include "test.h"
#include "testhelper.h"
#include "search.h"

#define FILTER_DOCS_SIZE 10
#define ARRAY_SIZE 20

struct FilterData {
    char *num;
    char *date;
    char *flipflop;
};

static const char *num = "num";
static const char *date = "date";
static const char *flipflop = "flipflop";

void prepare_filter_index(Store *store)
{
    int i;
    IndexWriter *iw;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_NO);

    struct FilterData data[FILTER_DOCS_SIZE] = {
        {"0", "20040601", "on"},
        {"1", "20041001", "off"},
        {"2", "20051101", "on"},
        {"3", "20041201", "off"},
        {"4", "20051101", "on"},
        {"5", "20041201", "off"},
        {"6", "20050101", "on"},
        {"7", "20040701", "off"},
        {"8", "20050301", "on"},
        {"9", "20050401", "off"}
    };

    index_create(store, fis);
    fis_destroy(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), NULL);
    for (i = 0; i < FILTER_DOCS_SIZE; i++) {
        Document *doc = doc_new();
        doc->boost = (float)(i+1);
        doc_add_field(doc, df_add_data(df_new(num), data[i].num));
        doc_add_field(doc, df_add_data(df_new(date), data[i].date));
        doc_add_field(doc, df_add_data(df_new(flipflop), data[i].flipflop));
        iw_add_doc(iw, doc);
        doc_destroy(doc);
    }
    iw_close(iw);
    return;
}

static void check_filtered_hits(tst_case *tc, Searcher *searcher, Query *query,
                                Filter *f, char *expected_hits, char top)
{
    static int num_array[ARRAY_SIZE];
    int i;
    int total_hits = s2l(expected_hits, num_array);
    TopDocs *top_docs = searcher_search(searcher, query, 0, total_hits+1, f, NULL);
    Aiequal(total_hits, top_docs->total_hits);
    Aiequal(total_hits, top_docs->size);

    if ((top >= 0) && top_docs->size) {
        Aiequal(top, top_docs->hits[0]->doc);
    }

    /* printf("top_docs->size = %d\n", top_docs->size); */
    for (i = 0; i < top_docs->size; i++) {
        Hit *hit = top_docs->hits[i];
        char buf[1000];
        sprintf(buf, "doc %d was found unexpectedly", hit->doc);
        Assert(ary_includes(num_array, total_hits, hit->doc), buf);
        /* only check the explanation if we got the correct docs. Obviously we
         * might want to remove this to visually check the explanations */
        if (total_hits == top_docs->total_hits) {
            Explanation *e = searcher->explain(searcher, query, hit->doc);
            /* char *t; printf("%s\n", t = expl_to_s(e, 0)); free(t); */
            Afequal(hit->score, e->value);
            expl_destroy(e);
        }
    }
    td_destroy(top_docs);
}

extern void check_hits(tst_case *tc, Searcher *searcher, Query *query,
                       char *expected_hits, char top);

#define TEST_TO_S(mstr, mfilt) \
    do {\
        char *fstr = mfilt->to_s(mfilt);\
        Asequal(mstr, fstr);\
        free(fstr);\
    } while (0)

static void test_range_filter(tst_case *tc, void *data)
{
    Searcher *searcher = (Searcher *)data;
    Query *q = maq_new();
    Filter *rf = rfilt_new(num, "2", "6", true, true);
    check_filtered_hits(tc, searcher, q, rf, "2,3,4,5,6", -1);
    TEST_TO_S("RangeFilter< num:[2 6] >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, "2", "6", true, false);
    check_filtered_hits(tc, searcher, q, rf, "2,3,4,5", -1);
    TEST_TO_S("RangeFilter< num:[2 6} >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, "2", "6", false, true);
    check_filtered_hits(tc, searcher, q, rf, "3,4,5,6", -1);
    TEST_TO_S("RangeFilter< num:{2 6] >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, "2", "6", false, false);
    check_filtered_hits(tc, searcher, q, rf, "3,4,5", -1);
    TEST_TO_S("RangeFilter< num:{2 6} >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, "6", NULL, true, false);
    check_filtered_hits(tc, searcher, q, rf, "6,7,8,9", -1);
    TEST_TO_S("RangeFilter< num:[6> >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, "6", NULL, false, false);
    check_filtered_hits(tc, searcher, q, rf, "7,8,9", -1);
    TEST_TO_S("RangeFilter< num:{6> >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, NULL, "2", false, true);
    check_filtered_hits(tc, searcher, q, rf, "0,1,2", -1);
    TEST_TO_S("RangeFilter< num:<2] >", rf);
    filt_deref(rf);
    rf = rfilt_new(num, NULL, "2", false, false);
    check_filtered_hits(tc, searcher, q, rf, "0,1", -1);
    TEST_TO_S("RangeFilter< num:<2} >", rf);
    filt_deref(rf);
    q_deref(q);
}

static void test_range_filter_hash(tst_case *tc, void *data)
{
    Filter *f1, *f2;
    (void)data;
    f1 = rfilt_new(date, "20051006", "20051010", true, true);
    f2 = rfilt_new(date, "20051006", "20051010", true, true);

    Assert(filt_eq(f1, f1), "Test same queries are equal");
    Aiequal(filt_hash(f1), filt_hash(f2));
    Assert(filt_eq(f1, f2), "Queries are equal");
    filt_deref(f2);

    f2 = rfilt_new(date, "20051006", "20051010", true, false);
    Assert(filt_hash(f1) != filt_hash(f2), "Upper bound include differs");
    Assert(!filt_eq(f1, f2), "Upper bound include differs");
    filt_deref(f2);

    f2 = rfilt_new(date, "20051006", "20051010", false, true);
    Assert(filt_hash(f1) != filt_hash(f2), "Lower bound include differs");
    Assert(!filt_eq(f1, f2), "Lower bound include differs");
    filt_deref(f2);

    f2 = rfilt_new(date, "20051006", "20051011", true, true);
    Assert(filt_hash(f1) != filt_hash(f2), "Upper bound differs");
    Assert(!filt_eq(f1, f2), "Upper bound differs");
    filt_deref(f2);

    f2 = rfilt_new(date, "20051005", "20051010", true, true);
    Assert(filt_hash(f1) != filt_hash(f2), "Lower bound differs");
    Assert(!filt_eq(f1, f2), "Lower bound differs");
    filt_deref(f2);

    f2 = rfilt_new(date, "20051006", NULL, true, false);
    Assert(filt_hash(f1) != filt_hash(f2), "Upper bound is NULL");
    Assert(!filt_eq(f1, f2), "Upper bound is NULL");
    filt_deref(f2);

    f2 = rfilt_new(date, NULL, "20051010", false, true);
    Assert(filt_hash(f1) != filt_hash(f2), "Lower bound is NULL");
    Assert(!filt_eq(f1, f2), "Lower bound is NULL");
    filt_deref(f2);

    f2 = rfilt_new(flipflop, "20051006", "20051010", true, true);
    Assert(filt_hash(f1) != filt_hash(f2), "Field differs");
    Assert(!filt_eq(f1, f2), "Field differs");
    filt_deref(f2);
    filt_deref(f1);

    f1 = rfilt_new(date, NULL, "20051010", false, true);
    f2 = rfilt_new(date, NULL, "20051010", false, true);
    Aiequal(filt_hash(f1), filt_hash(f2));
    Assert(filt_eq(f1, f2), "Queries are equal");
    filt_deref(f2);
    filt_deref(f1);
}

static void test_query_filter(tst_case *tc, void *data)
{
    Searcher *searcher = (Searcher *)data;
    Query *bq;
    Filter *qf;
    Query *q = maq_new();

    qf = qfilt_new_nr(tq_new(flipflop, "on"));
    TEST_TO_S("QueryFilter< flipflop:on >", qf);
    check_filtered_hits(tc, searcher, q, qf, "0,2,4,6,8", -1);
    filt_deref(qf);

    bq = bq_new(false);
    bq_add_query_nr(bq, tq_new(date, "20051101"), BC_SHOULD);
    bq_add_query_nr(bq, tq_new(date, "20041201"), BC_SHOULD);
    qf = qfilt_new_nr(bq);
    check_filtered_hits(tc, searcher, q, qf, "2,3,4,5", -1);
    TEST_TO_S("QueryFilter< date:20051101 date:20041201 >", qf);
    filt_deref(qf);

    q_deref(q);
}

static void test_query_filter_hash(tst_case *tc, void *data)
{
    Filter *f1, *f2;
    (void)data;
    f1 = qfilt_new_nr(tq_new("A", "a"));
    f2 = qfilt_new_nr(tq_new("A", "a"));

    Aiequal(filt_hash(f1), filt_hash(f2));
    Assert(filt_eq(f1, f2), "Queries are equal");
    Assert(filt_eq(f1, f1), "Queries are equal");
    filt_deref(f2);

    f2 = qfilt_new_nr(tq_new("A", "b"));
    Assert(filt_hash(f1) != filt_hash(f2), "texts differ");
    Assert(!filt_eq(f1, f2), "texts differ");
    filt_deref(f2);

    f2 = qfilt_new_nr(tq_new("B", "a"));
    Assert(filt_hash(f1) != filt_hash(f2), "fields differ");
    Assert(!filt_eq(f1, f2), "fields differ");
    filt_deref(f2);

    filt_deref(f1);
}

tst_suite *ts_filter(tst_suite *suite)
{
    Store *store;
    IndexReader *ir;
    Searcher *searcher;

    suite = ADD_SUITE(suite);

    store = open_ram_store();
    prepare_filter_index(store);
    ir = ir_open(store);
    searcher = stdsea_new(ir);

    tst_run_test(suite, test_range_filter, (void *)searcher);
    tst_run_test(suite, test_range_filter_hash, NULL);
    tst_run_test(suite, test_query_filter, (void *)searcher);
    tst_run_test(suite, test_query_filter_hash, NULL);

    store_deref(store);
    searcher->close(searcher);
    return suite;
}

#include "test.h"
#include "testhelper.h"
#include "search.h"

#define ARRAY_SIZE 20

static char *search = "search";
static char *string = "string";
static char *integer = "integer";
static char *flt = "flt";

typedef struct SortTestData {
    char *search;
    char *string;
    char *integer;
    char *flt;
} SortTestData;

static void add_sort_test_data(SortTestData *std, IndexWriter *iw)
{
    Document *doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(search), std->search));
    doc_add_field(doc, df_add_data(df_new(string), std->string));
    doc_add_field(doc, df_add_data(df_new(integer), std->integer));
    doc_add_field(doc, df_add_data(df_new(flt), std->flt));

    sscanf(std->flt, "%f", &doc->boost);

    iw_add_doc(iw, doc);
    doc_destroy(doc);
}

static void sort_test_setup(Store *store)
{
    int i;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    SortTestData data[] = {            /* len mod */
        {"findall","a","6","0.01"},    /*  4   0  */
        {"findall","c","5","0.1"},     /*  3   3  */
        {"findall","e","2","0.001"},   /*  5   1  */
        {"findall","g","1","1.0"},     /*  3   3  */
        {"findall","i","3","0.0001"},  /*  6   2  */
        {"findall","j","4","10.0"},    /*  4   0  */
        {"findall","h","5","0.00001"}, /*  7   3  */
        {"findall","f","2","100.0"},   /*  5   1  */
        {"findall","d","3","1000.0"},  /*  6   2  */
        {"findall","b","4","0.000001"} /*  8   0  */
    };

    index_create(store, fis);
    fis_destroy(fis);

    IndexWriter *iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    for (i = 0; i < NELEMS(data); i++) {
        add_sort_test_data(&data[i], iw);
    }
    iw_close(iw);
}

#define R_START 3
#define R_END 6
static void do_test_top_docs(tst_case *tc, Searcher *searcher, Query *query,
                      char *expected_hits, Sort *sort)
{
    static int num_array[ARRAY_SIZE];
    int i;
    int total_hits = s2l(expected_hits, num_array);
    TopDocs *top_docs = searcher->search(searcher, query, 0, total_hits, NULL, sort);
    Aiequal(total_hits, top_docs->total_hits);
    Aiequal(total_hits, top_docs->size);

    for (i = 0; i < top_docs->size; i++) {
        Hit *hit = top_docs->hits[i];
        Aiequal(num_array[i], hit->doc);
    }
    td_destroy(top_docs);

    if (total_hits >= R_END) {
        top_docs = searcher->search(searcher, query, 3, 3, NULL, sort);
        for (i = R_START; i < R_END; i++) {
            Hit *hit = top_docs->hits[i - R_START];
            Aiequal(num_array[i], hit->doc);
        }
        td_destroy(top_docs);
    }
}

#define TEST_SF_TO_S(mstr, msf) \
    do {\
        SortField *msf_p = msf;\
        char *fstr = sort_field_to_s(msf_p);\
        Asequal(mstr, fstr);\
        free(fstr);\
        sort_field_destroy(msf_p);\
    } while (0)


static void test_sort_field_to_s(tst_case *tc, void *data)
{
    (void)data;
    TEST_SF_TO_S("<SCORE>", sort_field_score_new(false));
    TEST_SF_TO_S("<SCORE>!", sort_field_score_new(true));
    TEST_SF_TO_S("<DOC>", sort_field_doc_new(false));
    TEST_SF_TO_S("<DOC>!", sort_field_doc_new(true));
    TEST_SF_TO_S("date:<integer>", sort_field_int_new("date", false));
    TEST_SF_TO_S("date:<integer>!", sort_field_int_new("date", true));
    TEST_SF_TO_S("price:<float>", sort_field_float_new("price", false));
    TEST_SF_TO_S("price:<float>!", sort_field_float_new("price", true));
    TEST_SF_TO_S("content:<string>", sort_field_string_new("content", false));
    TEST_SF_TO_S("content:<string>!", sort_field_string_new("content", true));
    TEST_SF_TO_S("auto_field:<auto>", sort_field_auto_new("auto_field", false));
    TEST_SF_TO_S("auto_field:<auto>!", sort_field_auto_new("auto_field", true));
}

#define TEST_SORT_TO_S(mstr, msort) \
    do {\
        char *fstr = sort_to_s(msort);\
        Asequal(mstr, fstr);\
        free(fstr);\
    } while (0)

static void test_sort_to_s(tst_case *tc, void *data)
{
    Sort *sort = sort_new();
    (void)data;

    TEST_SORT_TO_S("Sort[]", sort);
    sort_add_sort_field(sort, sort_field_score_new(false));
    TEST_SORT_TO_S("Sort[<SCORE>]", sort);
    sort_add_sort_field(sort, sort_field_doc_new(true));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!]", sort);
    sort_add_sort_field(sort, sort_field_int_new("date", true));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!]", sort);
    sort_add_sort_field(sort, sort_field_float_new("price", false));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!, price:<float>]", sort);
    sort_add_sort_field(sort, sort_field_string_new("content", true));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!, price:<float>, content:<string>!]", sort);
    sort_add_sort_field(sort, sort_field_auto_new("auto_field", false));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!, price:<float>, content:<string>!, auto_field:<auto>]", sort);
    sort_clear(sort);
    sort_add_sort_field(sort, sort_field_string_new("content", true));
    TEST_SORT_TO_S("Sort[content:<string>!]", sort);
    sort_add_sort_field(sort, sort_field_auto_new("auto_field", false));
    TEST_SORT_TO_S("Sort[content:<string>!, auto_field:<auto>]", sort);
    sort_destroy(sort);
}

static void test_sorts(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    Searcher *sea;
    Query *q;
    Sort *sort = NULL;

    ir = ir_open(store);
    sea = stdsea_new(ir);

    q = tq_new(search, "findall");
    do_test_top_docs(tc, sea, q, "8,7,5,3,1,0,2,4,6,9", NULL);

    sort = sort_new();

    sort_add_sort_field(sort, sort_field_score_new(false));
    do_test_top_docs(tc, sea, q, "8,7,5,3,1,0,2,4,6,9", sort);
    sort->sort_fields[0]->reverse = true;
    do_test_top_docs(tc, sea, q, "9,6,4,2,0,1,3,5,7,8", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_doc_new(false));
    do_test_top_docs(tc, sea, q, "0,1,2,3,4,5,6,7,8,9", sort);
    sort->sort_fields[0]->reverse = true;
    do_test_top_docs(tc, sea, q, "9,8,7,6,5,4,3,2,1,0", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_int_new(integer, true));
    do_test_top_docs(tc, sea, q, "0,1,6,5,9,4,8,2,7,3", sort);
    sort_add_sort_field(sort, sort_field_score_new(false));
    do_test_top_docs(tc, sea, q, "0,1,6,5,9,8,4,7,2,3", sort);
    sort->sf_cnt = 1; /* remove score sort_field */
    sort->sort_fields[0]->reverse = false;
    do_test_top_docs(tc, sea, q, "3,2,7,4,8,5,9,1,6,0", sort);
    sort->sf_cnt = 2; /* re-add score sort_field */
    do_test_top_docs(tc, sea, q, "3,7,2,8,4,5,9,1,6,0", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_float_new(flt, false));
    do_test_top_docs(tc, sea, q, "9,6,4,2,0,1,3,5,7,8", sort);
    sort->sort_fields[0]->reverse = true;
    do_test_top_docs(tc, sea, q, "8,7,5,3,1,0,2,4,6,9", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_string_new(string, false));
    do_test_top_docs(tc, sea, q, "0,9,1,8,2,7,3,6,4,5", sort);
    sort->sort_fields[0]->reverse = true;
    do_test_top_docs(tc, sea, q, "5,4,6,3,7,2,8,1,9,0", sort);
    sort_clear(sort);

    /* Test Auto Sort */
    sort_add_sort_field(sort, sort_field_auto_new(string, false));
    do_test_top_docs(tc, sea, q, "0,9,1,8,2,7,3,6,4,5", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_auto_new(integer, false));
    do_test_top_docs(tc, sea, q, "3,2,7,4,8,5,9,1,6,0", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_auto_new(flt, false));
    do_test_top_docs(tc, sea, q, "9,6,4,2,0,1,3,5,7,8", sort);
    sort->sort_fields[0]->reverse = true;
    do_test_top_docs(tc, sea, q, "8,7,5,3,1,0,2,4,6,9", sort);
    sort_clear(sort);

    sort_add_sort_field(sort, sort_field_auto_new(integer, false));
    sort_add_sort_field(sort, sort_field_auto_new(string, false));
    do_test_top_docs(tc, sea, q, "3,2,7,8,4,9,5,1,6,0", sort);

    sort_destroy(sort);
    q_deref(q);

    searcher_close(sea);
}

tst_suite *ts_sort(tst_suite *suite)
{
    Store *store = open_ram_store();
    sort_test_setup(store);

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_sort_field_to_s, NULL);
    tst_run_test(suite, test_sort_to_s, NULL);
    tst_run_test(suite, test_sorts, (void *)store);

    store_deref(store);
    return suite;
}

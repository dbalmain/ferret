#include "testhelper.h"
#include "symbol.h"
#include "search.h"
#include "test.h"

#define ARRAY_SIZE 20

static Symbol search, string, integer, flt;

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

static SortTestData data[] = {     /* len mod */
    {"findall","a","6","0.01"},    /*  4   0  */
    {"findall","c","5","0.1"},     /*  3   3  */
    {"findall","e","2","0.001"},   /*  5   1  */
    {"findall","g","1","1.0"},     /*  3   3  */
    {"findall","i","3","0.0001"},  /*  6   2  */
    {"findall","", "4","10.0"},    /*  4   0  */
    {"findall","h","5","0.00001"}, /*  7   3  */
    {"findall","f","2","100.0"},   /*  5   1  */
    {"findall","d","3","1000.0"},  /*  6   2  */
    {"findall","b","4","0.000001"} /*  8   0  */
};

static void sort_test_setup(Store *store)
{
    int i;
    IndexWriter *iw;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    index_create(store, fis);
    fis_deref(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    for (i = 0; i < NELEMS(data); i++) {
        add_sort_test_data(&data[i], iw);
    }
    iw_close(iw);
}

static void sort_multi_test_setup(Store *store1, Store *store2)
{
    int i;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    IndexWriter *iw;
    SortTestData data[] = {            /* len mod */
        {"findall","a","6","0.01"},    /*  4   0  */
        {"findall","c","5","0.1"},     /*  3   3  */
        {"findall","e","2","0.001"},   /*  5   1  */
        {"findall","g","1","1.0"},     /*  3   3  */
        {"findall","i","3","0.0001"},  /*  6   2  */
        {"findall","", "4","10.0"},    /*  4   0  */
        {"findall","h","5","0.00001"}, /*  7   3  */
        {"findall","f","2","100.0"},   /*  5   1  */
        {"findall","d","3","1000.0"},  /*  6   2  */
        {"findall","b","4","0.000001"} /*  8   0  */
    };

    index_create(store1, fis);
    index_create(store2, fis);
    fis_deref(fis);

    iw = iw_open(store1, whitespace_analyzer_new(false), NULL);

    for (i = 0; i < NELEMS(data)/2; i++) {
        add_sort_test_data(&data[i], iw);
    }
    iw_close(iw);

    iw = iw_open(store2, whitespace_analyzer_new(false), NULL);

    for (i = NELEMS(data)/2; i < NELEMS(data); i++) {
        add_sort_test_data(&data[i], iw);
    }
    iw_close(iw);
}

#define R_START 3
#define R_END 6
static void do_test_top_docs(TestCase *tc, Searcher *searcher, Query *query,
                      char *expected_hits, Sort *sort)
{
    static int num_array[ARRAY_SIZE];
    int i;
    int total_hits = s2l(expected_hits, num_array);
    TopDocs *top_docs = searcher_search(searcher, query, 0,
                                        total_hits, NULL, sort, NULL);
    Aiequal(total_hits, top_docs->total_hits);
    Aiequal(total_hits, top_docs->size);

    for (i = 0; i < top_docs->size; i++) {
        Hit *hit = top_docs->hits[i];
        if (false && sort && searcher->doc_freq != isea_doc_freq) {
            FieldDoc *fd = (FieldDoc *)hit;
            int j;
            printf("%d == %d:%f ", num_array[i], hit->doc, hit->score);
            for (j = 0; j < fd->size; j++) {
                switch (fd->comparables[j].type) {
                    case SORT_TYPE_SCORE:
                        printf("sc:%f ", fd->comparables[j].val.f); break;
                    case SORT_TYPE_FLOAT:
                        printf("f:%f ", fd->comparables[j].val.f); break;
                    case SORT_TYPE_DOC:
                        printf("d:%ld ", fd->comparables[j].val.l); break;
                    case SORT_TYPE_INTEGER:
                        printf("i:%ld ", fd->comparables[j].val.l); break;
                    case SORT_TYPE_STRING:
                        printf("s:%s ", fd->comparables[j].val.s); break;
                    default:
                        printf("NA "); break;
                }
            }
            printf("\n");
        }
        Aiequal(num_array[i], hit->doc);
    }
    td_destroy(top_docs);

    if (total_hits >= R_END) {
        top_docs = searcher_search(searcher, query, R_START, R_END - R_START,
                                   NULL, sort, NULL);
        for (i = R_START; i < R_END; i++) {
            Hit *hit = top_docs->hits[i - R_START];
            Aiequal(num_array[i], hit->doc);
            /*
            printf("%d == %d\n", num_array[i], hit->doc);
            */
        }
        td_destroy(top_docs);
    }
}

#define TEST_SF_TO_S(_str, _sf) \
    do {\
        SortField *_sf_p = _sf;\
        char *_field = sort_field_to_s(_sf_p);\
        Asequal(_str, _field);\
        free(_field);\
        sort_field_destroy(_sf_p);\
    } while (0)


static void test_sort_field_to_s(TestCase *tc, void *data)
{
    (void)data;
    TEST_SF_TO_S("<SCORE>", sort_field_score_new(false));
    TEST_SF_TO_S("<SCORE>!", sort_field_score_new(true));
    TEST_SF_TO_S("<DOC>", sort_field_doc_new(false));
    TEST_SF_TO_S("<DOC>!", sort_field_doc_new(true));
    TEST_SF_TO_S("date:<integer>",
                 sort_field_int_new(I("date"), false));
    TEST_SF_TO_S("date:<integer>!",
                 sort_field_int_new(I("date"), true));
    TEST_SF_TO_S("price:<float>",
                 sort_field_float_new(I("price"), false));
    TEST_SF_TO_S("price:<float>!",
                 sort_field_float_new(I("price"), true));
    TEST_SF_TO_S("content:<string>",
                 sort_field_string_new(I("content"), false));
    TEST_SF_TO_S("content:<string>!",
                 sort_field_string_new(I("content"), true));
    TEST_SF_TO_S("auto_field:<auto>",
                 sort_field_auto_new(I("auto_field"), false));
    TEST_SF_TO_S("auto_field:<auto>!",
                 sort_field_auto_new(I("auto_field"), true));
}

#define TEST_SORT_TO_S(_expected_str, _sort) \
    do {\
        char *_str = sort_to_s(_sort);\
        Asequal(_expected_str, _str);\
        free(_str);\
    } while (0)

static void test_sort_to_s(TestCase *tc, void *data)
{
    Sort *sort = sort_new();
    (void)data;

    TEST_SORT_TO_S("Sort[]", sort);
    sort_add_sort_field(sort, sort_field_score_new(false));
    TEST_SORT_TO_S("Sort[<SCORE>]", sort);
    sort_add_sort_field(sort, sort_field_doc_new(true));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!]", sort);
    sort_add_sort_field(sort, sort_field_int_new(I("date"), true));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!]", sort);
    sort_add_sort_field(sort, sort_field_float_new(I("price"), false));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!, price:<float>]", sort);
    sort_add_sort_field(sort, sort_field_string_new(I("content"), true));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!, price:<float>, content:<string>!]", sort);
    sort_add_sort_field(sort, sort_field_auto_new(I("auto_field"), false));
    TEST_SORT_TO_S("Sort[<SCORE>, <DOC>!, date:<integer>!, price:<float>, content:<string>!, auto_field:<auto>]", sort);
    sort_clear(sort);
    sort_add_sort_field(sort, sort_field_string_new(I("content"), true));
    TEST_SORT_TO_S("Sort[content:<string>!]", sort);
    sort_add_sort_field(sort, sort_field_auto_new(I("auto_field"), false));
    TEST_SORT_TO_S("Sort[content:<string>!, auto_field:<auto>]", sort);
    sort_destroy(sort);
}

static bool do_byte_test = true;
static void test_sorts(TestCase *tc, void *data)
{
    Searcher *sea = (Searcher *)data;
    Query *q;
    Sort *sort = NULL;

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
    sort->size = 1; /* remove score sort_field */
    sort->sort_fields[0]->reverse = false;
    do_test_top_docs(tc, sea, q, "3,2,7,4,8,5,9,1,6,0", sort);
    sort->size = 2; /* re-add score sort_field */
    do_test_top_docs(tc, sea, q, "3,7,2,8,4,5,9,1,6,0", sort);
    sort_clear(sort);

    if (do_byte_test) {
        sort_add_sort_field(sort, sort_field_byte_new(integer, true));
        do_test_top_docs(tc, sea, q, "0,1,6,5,9,4,8,2,7,3", sort);
        sort_add_sort_field(sort, sort_field_score_new(false));
        do_test_top_docs(tc, sea, q, "0,1,6,5,9,8,4,7,2,3", sort);
        sort->size = 1; /* remove score sort_field */
        sort->sort_fields[0]->reverse = false;
        do_test_top_docs(tc, sea, q, "3,2,7,4,8,5,9,1,6,0", sort);
        sort->size = 2; /* re-add score sort_field */
        do_test_top_docs(tc, sea, q, "3,7,2,8,4,5,9,1,6,0", sort);
        sort_clear(sort);
    }

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

    if (do_byte_test) {
        sort_add_sort_field(sort, sort_field_byte_new(string, false));
        do_test_top_docs(tc, sea, q, "5,0,9,1,8,2,7,3,6,4", sort);
        sort->sort_fields[0]->reverse = true;
        do_test_top_docs(tc, sea, q, "4,6,3,7,2,8,1,9,0,5", sort);
        sort_clear(sort);
    }

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
}

TestSuite *ts_sort(TestSuite *suite)
{
    Searcher *sea, **searchers;
    Store *store = open_ram_store(), *fs_store;

    search = intern("search");
    string = intern("string");
    integer = intern("integer");
    flt = intern("flt");

    sort_test_setup(store);

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_sort_field_to_s, NULL);
    tst_run_test(suite, test_sort_to_s, NULL);

    sea = isea_new(ir_open(store));

    tst_run_test(suite, test_sorts, (void *)sea);

    searcher_close(sea);

    do_byte_test = false;

#ifdef POSH_OS_WIN32
    fs_store = open_fs_store(".\\test\\testdir\\store");
#else
    fs_store = open_fs_store("./test/testdir/store");
#endif
    sort_multi_test_setup(store, fs_store);

    searchers = ALLOC_N(Searcher *, 2);

    searchers[0] = isea_new(ir_open(store));
    searchers[1] = isea_new(ir_open(fs_store));

    sea = msea_new(searchers, 2, true);
    tst_run_test(suite, test_sorts, (void *)sea);
    searcher_close(sea);

    store_deref(store);
    store_deref(fs_store);

    return suite;
}

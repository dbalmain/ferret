#include "search.h"
#include "array.h"
#include "testhelper.h"
#include "test.h"

#define ARRAY_SIZE 100

static void test_match_vector(TestCase *tc, void *data)
{
    const int match_test_count = 100;
    int i;
    MatchVector *mv = matchv_new();
    (void)data; /* suppress unused argument warning */
    srand(5);

    matchv_add(mv, 0, 10); 
    matchv_add(mv, 200, 220); 
    matchv_add(mv, 50, 56); 
    matchv_add(mv, 50, 55); 
    matchv_add(mv, 57, 63); 

    Aiequal(0  , mv->matches[0].start);
    Aiequal(10 , mv->matches[0].end);
    Aiequal(200, mv->matches[1].start);
    Aiequal(220, mv->matches[1].end);
    Aiequal(50 , mv->matches[2].start);
    Aiequal(56 , mv->matches[2].end);
    Aiequal(50 , mv->matches[3].start);
    Aiequal(55 , mv->matches[3].end);
    Aiequal(57 , mv->matches[4].start);
    Aiequal(63 , mv->matches[4].end);

    matchv_sort(mv);

    Aiequal(0  , mv->matches[0].start);
    Aiequal(10 , mv->matches[0].end);
    Aiequal(50 , mv->matches[1].start);
    Aiequal(56 , mv->matches[1].end);
    Aiequal(50 , mv->matches[2].start);
    Aiequal(55 , mv->matches[2].end);
    Aiequal(57 , mv->matches[3].start);
    Aiequal(63 , mv->matches[3].end);
    Aiequal(200, mv->matches[4].start);
    Aiequal(220, mv->matches[4].end);

    matchv_compact(mv);

    Aiequal(3  , mv->size);
    Aiequal(0  , mv->matches[0].start);
    Aiequal(10 , mv->matches[0].end);
    Aiequal(50 , mv->matches[1].start);
    Aiequal(63 , mv->matches[1].end);
    Aiequal(200, mv->matches[2].start);
    Aiequal(220, mv->matches[2].end);

    matchv_destroy(mv);

    mv = matchv_new();

    for (i = 0; i < match_test_count; i++)
    {
        int start = rand() % 10000000;
        int end = start + rand() % 100;
        matchv_add(mv, start, end);
    }

    matchv_sort(mv);

    for (i = 1; i < match_test_count; i++)
    {
        Assert(mv->matches[i].start > mv->matches[i-1].start
               || mv->matches[i].end > mv->matches[i-1].end,
               "Offset(%d:%d) < Offset(%d:%d)",
               mv->matches[i].start, mv->matches[i].end,
               mv->matches[i-1].start, mv->matches[i-1].end);
    }
    matchv_destroy(mv);
}

static void make_index(Store *store)
{
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    index_create(store, fis);
    fis_deref(fis);
}

static void add_string_docs(Store *store, const char *string[])
{
    IndexWriter *iw = iw_open(store, whitespace_analyzer_new(true), NULL);
    
    while (*string) {
        Document *doc = doc_new();
        doc_add_field(doc, df_add_data(df_new(I("field")), (char *)*string));
        iw_add_doc(iw, doc);
        doc_destroy(doc);
        string++;
    }
    iw_close(iw);
}

#define Chk_sea_mv(query, doc_num, expected)\
    check_searcher_match_vector(tc, store, query, doc_num, expected)
static void check_searcher_match_vector(TestCase *tc, Store *store,
                                        Query *query, int doc_num,
                                        char *expected)
{
    IndexReader *ir = ir_open(store);
    Searcher *sea = isea_new(ir);
    MatchVector *mv = searcher_get_match_vector(sea, query, doc_num, I("field"));
    static int offset_array[ARRAY_SIZE];
    int matchv_size = s2l(expected, offset_array) / 2;
    int i;

    Aiequal(matchv_size, mv->size);
    matchv_sort(mv);
    for (i = 0; i < matchv_size; i++) {
        Aiequal(offset_array[i<<1], mv->matches[i].start);
        Aiequal(offset_array[(i<<1)+1], mv->matches[i].end);
    }
    matchv_destroy(mv);
    searcher_close(sea);
}

#define Chk_mv(query, doc_num, expected)\
    check_match_vector(tc, store, query, doc_num, expected)
static void check_match_vector(TestCase *tc, Store *store, Query *query, int doc_num,
                               char *expected)
{
    IndexReader *ir = ir_open(store);
    MatchVector *mv = matchv_new();
    TermVector *term_vector = ir->term_vector(ir, doc_num, I("field"));
    static int offset_array[ARRAY_SIZE];
    int matchv_size = s2l(expected, offset_array) / 2;
    int i;

    mv = query->get_matchv_i(query, mv, term_vector);
    Aiequal(matchv_size, mv->size);
    matchv_sort(mv);
    for (i = 0; i < matchv_size; i++) {
        Aiequal(offset_array[i<<1], mv->matches[i].start);
        Aiequal(offset_array[(i<<1)+1], mv->matches[i].end);
    }
    matchv_destroy(mv);
    ir_close(ir);
    tv_destroy(term_vector);
    check_searcher_match_vector(tc, store, query, doc_num, expected);
}

static void test_term_query(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q;
    const char *docs[] = {
        "the phrase has the word rabbit once",
        "rabbit one rabbit two rabbit three rabbit four",
        "Term doesn't appear in this sentence",
        NULL
    };
    make_index(store);
    add_string_docs(store, docs);
    q = tq_new(I("field"), "rabbit");
    Chk_mv(q, 0, "5:5");
    Chk_mv(q, 1, "0:0 2:2 4:4 6:6");
    Chk_mv(q, 2, "");
    q_deref(q);
    q = tq_new(I("diff_field"), "rabbit");
    Chk_mv(q, 0, "");
    Chk_mv(q, 1, "");
    q_deref(q);
}

static void test_phrase_query(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q;
    const char *docs[] = {
        "the magic phrase of the day is one two three not three "
            "two one one too three",
        "one two three and again one two three and maybe one more for good "
            "luck one two three",
        "phrase doesn't appear in this sentence",
        "multi phrase quick brown fox fast white wolf agile red fox quick "
            "pink hound",
        "multi phrase with slop brown quick fox the agile beautful and "
            "cunning white wolf",
        NULL
    };
    make_index(store);
    add_string_docs(store, docs);
    q = phq_new(I("field"));
    phq_add_term(q, "one", 1); 
    phq_add_term(q, "two", 1); 
    phq_add_term(q, "three", 1); 
    Chk_mv(q, 0, "7:9");
    Chk_mv(q, 1, "0:2 5:7 15:17");
    Chk_mv(q, 2, "");
    ((PhraseQuery *)q)->slop = 3;
    Chk_mv(q, 0, "7:9 12:16");
    ((PhraseQuery *)q)->slop = 4;
    Chk_mv(q, 0, "7:9 11:13 12:16");
    q_deref(q);
    
    /* test that it only works for the correct field */
    q = phq_new(I("wrong_field"));
    phq_add_term(q, "one", 1); 
    phq_add_term(q, "two", 1); 
    phq_add_term(q, "three", 1); 
    Chk_mv(q, 0, "");
    Chk_mv(q, 1, "");
    Chk_mv(q, 2, "");
    ((PhraseQuery *)q)->slop = 4;
    Chk_mv(q, 0, "");
    q_deref(q);

    q = phq_new(I("field"));
    phq_add_term(q, "quick", 1);
    phq_append_multi_term(q, "fast");
    phq_append_multi_term(q, "agile");
    phq_add_term(q, "brown", 1);
    phq_append_multi_term(q, "pink");
    phq_append_multi_term(q, "red");
    phq_append_multi_term(q, "white");
    phq_add_term(q, "fox", 1);
    phq_append_multi_term(q, "wolf");
    phq_append_multi_term(q, "hound");
    Chk_mv(q, 3, "2:4 5:7 8:10 11:13");
    Chk_mv(q, 4, "");
    ((PhraseQuery *)q)->slop = 2;
    Chk_mv(q, 4, "4:6");
    ((PhraseQuery *)q)->slop = 5;
    Chk_mv(q, 4, "4:6 8:13");
    q_deref(q);
}

static void test_boolean_query(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q, *phq;
    const char *docs[] = {
        "one and some words and two and three and some more words one two",
        NULL
    };
    make_index(store);
    add_string_docs(store, docs);
    q = bq_new(false);
    bq_add_query_nr(q, tq_new(I("field"), "one"), BC_SHOULD);
    Chk_mv(q, 0, "0:0 12:12");
    bq_add_query_nr(q, tq_new(I("field"), "two"), BC_MUST);
    Chk_mv(q, 0, "0:0 5:5 12:12 13:13");
    phq = phq_new(I("field"));
    phq_add_term(phq, "one", 1);
    phq_add_term(phq, "two", 1);
    Chk_mv(phq, 0, "12:13");
    bq_add_query_nr(q, phq, BC_SHOULD);
    Chk_mv(q, 0, "0:0 5:5 12:13 12:12 13:13");
    q_deref(q);
}

static void test_multi_term_query(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q;
    const char *docs[] = {
        "one and some words and two and three and some more words one two",
        NULL
    };
    make_index(store);
    add_string_docs(store, docs);
    q = multi_tq_new(I("field"));
    multi_tq_add_term(q, "one");
    Chk_mv(q, 0, "0:0 12:12");
    multi_tq_add_term(q, "two");
    Chk_mv(q, 0, "0:0 5:5 12:12 13:13");
    multi_tq_add_term(q, "and");
    Chk_mv(q, 0, "0:0 1:1 4:4 5:5 6:6 8:8 12:12 13:13");
    q_deref(q);
}

static void test_span_queries(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q, *oq;
    const char *docs[] = {
        "one and some words an two and three words and some more words one two",
        "worda one wordb one worda one 2 wordb one 2 worda one two three wordb",
        NULL
    };
    make_index(store);
    add_string_docs(store, docs);
    q = spantq_new(I("wrong_field"), "words");
    Chk_mv(q, 0, "");
    q_deref(q);

    q = spantq_new(I("field"), "words");
    Chk_mv(q, 0, "3:3 8:8 12:12");
    q = spanfq_new_nr(q, 4);
    Chk_mv(q, 0, "3:3");
    ((SpanFirstQuery *)q)->end = 8;
    Chk_mv(q, 0, "3:3");
    ((SpanFirstQuery *)q)->end = 9;
    Chk_mv(q, 0, "3:3 8:8");
    ((SpanFirstQuery *)q)->end = 12;
    Chk_mv(q, 0, "3:3 8:8");
    ((SpanFirstQuery *)q)->end = 13;
    Chk_mv(q, 0, "3:3 8:8 12:12");
    
    oq = spanoq_new();
    spanoq_add_clause_nr(oq, q);
    Chk_mv(oq, 0, "3:3 8:8 12:12");
    spanoq_add_clause_nr(oq, spantq_new(I("field"), "one"));
    Chk_mv(oq, 0, "0:0 3:3 8:8 12:12 13:13");
    spanoq_add_clause_nr(oq, spantq_new(I("field"), "two"));
    Chk_mv(oq, 0, "0:0 3:3 5:5 8:8 12:12 13:13 14:14");
    q_deref(oq);

    q = spannq_new(1, true);
    spannq_add_clause_nr(q, spantq_new(I("field"), "worda"));
    Chk_mv(q, 0, "");
    Chk_mv(q, 1, "0:0 4:4 10:10");
    spannq_add_clause_nr(q, spantq_new(I("field"), "wordb"));
    Chk_mv(q, 1, "0:0 2:2");
    ((SpanNearQuery *)q)->in_order = false;
    Chk_mv(q, 1, "0:0 2:2 4:4");
    ((SpanNearQuery *)q)->slop = 2;
    Chk_mv(q, 1, "0:0 2:2 4:4 7:7 10:10");
    ((SpanNearQuery *)q)->slop = 3;
    Chk_mv(q, 1, "0:0 2:2 4:4 7:7 10:10 14:14");

    q = spanxq_new_nr(q, spantq_new(I("field"), "2"));
    Chk_mv(q, 1, "0:0 2:2 4:4 10:10 14:14");
    q_deref(q);
}

static void test_searcher_get_match_vector(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q;
    const char *docs[] = {
        "funnyword funniward funyword funywod funnywerd funniword finnywood",
        NULL
    };
    make_index(store);
    add_string_docs(store, docs);
    q = fuzq_new_conf(I("field"), "funnyword", 0.9f, 0, 512);
    Chk_sea_mv(q, 0, "0:0");
    q_deref(q);

    q = fuzq_new_conf(I("field"), "funnyword", 0.8f, 0, 512);
    Chk_sea_mv(q, 0, "0:0 2:2 4:4 5:5");
    q_deref(q);

    q = fuzq_new_conf(I("field"), "funnyword", 0.5f, 0, 512);
    Chk_sea_mv(q, 0, "0:0 1:1 2:2 3:3 4:4 5:5 6:6");
    q_deref(q);
}

static void test_searcher_highlight(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Query *q, *phq;
    IndexWriter *iw;
    IndexReader *ir;
    Searcher *sea;
    char **highlights;
    const char *docs[] = {
        "the words we are searching for are one and two also sometimes "
            "looking for them as a phrase like this; one two lets see "
            "how it goes",
        NULL
    };
    Document *doc = doc_new();

    make_index(store);
    add_string_docs(store, docs);

    iw = iw_open(store, letter_analyzer_new(true), NULL);
    doc_add_field(doc, df_add_data(df_new(I("field")), "That's how it goes now."));
    iw_add_doc(iw, doc);
    doc_destroy(doc);
    iw_close(iw);

    ir = ir_open(store);
    sea = isea_new(ir);

    q = tq_new(I("field"), "one");
    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 1,
                                    "<b>", "</b>", "...");
    Aiequal(1, ary_size(highlights));
    Asequal("...are <b>one</b>...", highlights[0]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 2,
                                    "<b>", "</b>", "...");
    Aiequal(2, ary_size(highlights));
    Asequal("...are <b>one</b>...", highlights[0]);
    Asequal("...this; <b>one</b>...", highlights[1]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 3,
                                    "<b>", "</b>", "...");
    Aiequal(3, ary_size(highlights));
    Asequal("the words...", highlights[0]);
    Asequal("...are <b>one</b>...", highlights[1]);
    Asequal("...this; <b>one</b>...", highlights[2]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 4,
                                    "<b>", "</b>", "...");
    Aiequal(3, ary_size(highlights));
    Asequal("the words we are...", highlights[0]);
    Asequal("...are <b>one</b>...", highlights[1]);
    Asequal("...this; <b>one</b>...", highlights[2]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 5,
                                    "<b>", "</b>", "...");
    Aiequal(2, ary_size(highlights));
    Asequal("the words we are searching for are <b>one</b>...", highlights[0]);
    Asequal("...this; <b>one</b>...", highlights[1]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 20,
                                    "<b>", "</b>", "...");
    Aiequal(1, ary_size(highlights));
    Asequal("the words we are searching for are <b>one</b> and two also "
            "sometimes looking for them as a phrase like this; <b>one</b> "
            "two lets see how it goes", highlights[0]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 1000, 1,
                                    "<b>", "</b>", "...");
    Aiequal(1, ary_size(highlights));
    Asequal("the words we are searching for are <b>one</b> and two also "
            "sometimes looking for them as a phrase like this; <b>one</b> "
            "two lets see how it goes", highlights[0]);
    ary_destroy(highlights, &free);

    q_deref(q);

    q = bq_new(false);
    bq_add_query_nr(q, tq_new(I("field"), "one"), BC_SHOULD);
    bq_add_query_nr(q, tq_new(I("field"), "two"), BC_SHOULD);

    highlights = searcher_highlight(sea, q, 0, I("field"), 15, 2,
                                    "<b>", "</b>", "...");
    Aiequal(2, ary_size(highlights));
    Asequal("...<b>one</b> and <b>two</b>...", highlights[0]);
    Asequal("...this; <b>one</b> <b>two</b>...", highlights[1]);
    ary_destroy(highlights, &free);

    phq = phq_new(I("field"));
    phq_add_term(phq, "one", 1);
    phq_add_term(phq, "two", 1);

    bq_add_query_nr(q, phq, BC_SHOULD);

    highlights = searcher_highlight(sea, q, 0, I("field"), 15, 2,
                                    "<b>", "</b>", "...");
    Aiequal(2, ary_size(highlights));
    Asequal("...<b>one</b> and <b>two</b>...", highlights[0]);
    Asequal("...this; <b>one two</b>...", highlights[1]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 15, 1,
                                    "<b>", "</b>", "...");
    Aiequal(1, ary_size(highlights));
    /* should have a higher priority since it the merger of three matches */
    Asequal("...this; <b>one two</b>...", highlights[0]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("not_a_field"), 15, 1,
                                    "<b>", "</b>", "...");
    Apnull(highlights);

    q_deref(q);

    q = tq_new(I("wrong_field"), "one");
    highlights = searcher_highlight(sea, q, 0, I("not_a_field"), 15, 1,
                                    "<b>", "</b>", "...");
    Apnull(highlights);

    q_deref(q);

    q = bq_new(false);
    phq = phq_new(I("field"));
    phq_add_term(phq, "the", 1);
    phq_add_term(phq, "words", 1);
    bq_add_query_nr(q, phq, BC_SHOULD);
    phq = phq_new(I("field"));
    phq_add_term(phq, "for", 1);
    phq_add_term(phq, "are", 1);
    phq_add_term(phq, "one", 1);
    phq_add_term(phq, "and", 1);
    phq_add_term(phq, "two", 1);
    bq_add_query_nr(q, phq, BC_SHOULD);
    bq_add_query_nr(q, tq_new(I("field"), "words"), BC_SHOULD);
    bq_add_query_nr(q, tq_new(I("field"), "one"), BC_SHOULD);
    bq_add_query_nr(q, tq_new(I("field"), "two"), BC_SHOULD);
    bq_add_query_nr(q, tq_new(I("field"), "UnKnOwNfIeLd"), BC_SHOULD);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 1,
                                    "<b>", "</b>", "...");
    Aiequal(1, ary_size(highlights));
    Asequal("<b>the words</b>...", highlights[0]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 0, I("field"), 10, 2,
                                    "<b>", "</b>", "...");
    Aiequal(2, ary_size(highlights));
    Asequal("<b>the words</b>...", highlights[0]);
    Asequal("...<b>one</b> <b>two</b>...", highlights[1]);
    ary_destroy(highlights, &free);

    q_deref(q);

    q = tq_new(I("field"), "goes");
    highlights = searcher_highlight(sea, q, 0, I("field"), 13, 2,
                                    "<b>", "</b>", "...");
    Aiequal(2, ary_size(highlights));
    Asequal("the words we...", highlights[0]);
    Asequal("...how it <b>goes</b>", highlights[1]);
    ary_destroy(highlights, &free);

    highlights = searcher_highlight(sea, q, 1, I("field"), 16, 1,
                                    "<b>", "</b>", "...");
    Aiequal(1, ary_size(highlights));
    Asequal("...how it <b>goes</b> now.", highlights[0]);
    ary_destroy(highlights, &free);
    q_deref(q);

    searcher_close(sea);
}

TestSuite *ts_highlighter(TestSuite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_match_vector, NULL);
    tst_run_test(suite, test_term_query, store);
    tst_run_test(suite, test_phrase_query, store);
    tst_run_test(suite, test_boolean_query, store);
    tst_run_test(suite, test_multi_term_query, store);
    tst_run_test(suite, test_span_queries, store);

    tst_run_test(suite, test_searcher_get_match_vector, store);
    tst_run_test(suite, test_searcher_highlight, store);

    store_deref(store);
    return suite;
}

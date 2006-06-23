#include "test.h"
#include "search.h"
#include "helper.h"

#define ARRAY_SIZE 20

void test_byte_float_conversion(tst_case *tc, void *data)
{
    int i;
    (void)data;

    for (i = 0; i < 256; i++) {
        Aiequal(i, float2byte(byte2float((char)i)));
    }
}

//static int my_doc_freq(Searcher *searcher, Term *term) { return 9; }
//static int my_max_doc(Searcher *searcher) { return 10; }
//
//void test_default_similarity(tst_case *tc, void *data)
//{
//    Term *terms[4];
//    Term term = {"field", "text"};
//    Searcher searcher;
//    Similarity *dsim = sim_create_default();
//    Term t1 = {"field1", "text1"};
//    Term t2 = {"field2", "text2"};
//    Term t3 = {"field3", "text3"};
//    Term t4 = {"field4", "text4"};
//
//    terms[0] = &t1;
//    terms[1] = &t2;
//    terms[2] = &t3;
//    terms[3] = &t4;
//
//    Afequal(1.0/4, sim_length_norm(dsim, "field", 16));
//    Afequal(1.0/4, sim_query_norm(dsim, 16));
//    Afequal(3.0, sim_tf(dsim, 9));
//    Afequal(1.0/10, sim_sloppy_freq(dsim, 9));
//    Afequal(1.0, sim_idf(dsim, 9, 10));
//    Afequal(4.0, sim_coord(dsim, 12, 3));
//    searcher.doc_freq = &my_doc_freq;
//    searcher.max_doc = &my_max_doc;
//    Afequal(1.0, sim_idf_term(dsim, &term, &searcher));
//    Afequal(4.0, sim_idf_phrase(dsim, terms, 4, &searcher));
//}
//
//struct Data {
//    char *date;
//    char *field;
//    char *cat;
//};
//
//static const char *date = "date";
//static const char *field = "field";
//static const char *cat = "cat";
//
//#define SEARCH_DOCS_SIZE 18
//static void prepare_search_index(Store *store)
//{
//    int i;
//    IndexWriter *iw;
//    struct Data data[SEARCH_DOCS_SIZE] = {
//        {"20050930", "word1",
//            "cat1/"},
//        {"20051001", "word1 word2 the quick brown fox",
//            "cat1/sub1"},
//        {"20051002", "word1 word3",
//            "cat1/sub1/subsub1"},
//        {"20051003", "word1 word3",
//            "cat1/sub2"},
//        {"20051004", "word1 word2",
//            "cat1/sub2/subsub2"},
//        {"20051005", "word1",
//            "cat2/sub1"},
//        {"20051006", "word1 word3",
//            "cat2/sub1"},
//        {"20051007", "word1",
//            "cat2/sub1"},
//        {"20051008", "word1 word2 word3 the fast brown fox",
//            "cat2/sub1"},
//        {"20051009", "word1",
//            "cat3/sub1"},
//        {"20051010", "word1",
//            "cat3/sub1"},
//        {"20051011", "word1 word3 the quick red fox",
//            "cat3/sub1"},
//        {"20051012", "word1",
//            "cat3/sub1"},
//        {"20051013", "word1",
//            "cat1/sub2"},
//        {"20051014", "word1 word3 the quick hairy fox",
//            "cat1/sub1"},
//        {"20051015", "word1",
//            "cat1/sub2/subsub1"},
//        {"20051016", "word1 the quick fox is brown and hairy and a little red",
//            "cat1/sub1/subsub2"},
//        {"20051017", "word1 the brown fox is quick and red",
//            "cat1/"}
//    };
//    iw = iw_open(store, whitespace_analyzer_create(false), true);
//    for (i = 0; i < SEARCH_DOCS_SIZE; i++) {
//        Document *doc = doc_create();
//        doc->boost = (float)(i+1);
//        doc_add_field(doc, df_create(date, estrdup(data[i].date), DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
//        doc_add_field(doc, df_create(field, estrdup(data[i].field), DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
//        doc_add_field(doc, df_create(cat, estrdup(data[i].cat), DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
//        iw_add_doc(iw, doc);
//        doc_destroy(doc);
//    }
//    iw_close(iw);
//}
//
//void test_get_doc(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    Document *doc;
//    Aiequal(SEARCH_DOCS_SIZE, sea_max_doc(searcher));
//    doc = sea_get_doc(searcher, 0);
//    Asequal("20050930", doc_get_field(doc, date)->data);
//    doc_destroy(doc);
//    doc = sea_get_doc(searcher, 4);
//    Asequal("cat1/sub2/subsub2", doc_get_field(doc, cat)->data);
//    doc_destroy(doc);
//    doc = sea_get_doc(searcher, 12);
//    Asequal("20051012", doc_get_field(doc, date)->data);
//    doc_destroy(doc);
//}
//
//void check_to_s(tst_case *tc, Query *query, char *field, char *q_str)
//{
//    char *q_res = query->to_s(query, field);
//    Asequal(q_str, q_res);
//    free(q_res);
//}
//
//void check_hits(tst_case *tc, Searcher *searcher, Query *query,
//                char *expected_hits, char top)
//{
//    static int num_array[ARRAY_SIZE];
//    int i;
//    int total_hits = s2l(expected_hits, num_array);
//    TopDocs *top_docs = sea_search(searcher, query, 0, total_hits+1, NULL, NULL);
//    Aiequal(total_hits, top_docs->total_hits);
//    Aiequal(total_hits, top_docs->size);
//
//    if ((top >= 0) && top_docs->size)
//        Aiequal(top, top_docs->hits[0]->doc);
//
//    /* printf("top_docs->size = %d\n", top_docs->size); */
//    for (i = 0; i < top_docs->size; i++) {
//        Hit *hit = top_docs->hits[i];
//        char buf[1000];
//        sprintf(buf, "doc %d was found unexpectedly", hit->doc);
//        Assert(ary_includes(num_array, total_hits, hit->doc), buf);
//        /* only check the explanation if we got the correct docs. Obviously we
//         * might want to remove this to visually check the explanations */
//        if (total_hits == top_docs->total_hits) {
//            Explanation *e = sea_explain(searcher, query, hit->doc);
//            /*
//               char *t;
//               printf("\n%d>>\n%f\n%s\n", hit->doc, hit->score, t = expl_to_s(e, 0));
//               free(t);
//               */
//            Afequal(hit->score, e->value);
//            expl_destoy(e);
//        }
//    }
//    td_destroy(top_docs);
//}
//
//void test_term_query(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    TopDocs *top_docs;
//    Query *tq = tq_create(term_create(field, "word2"));
//    check_to_s(tc, tq, (char *)field, "word2");
//    check_to_s(tc, tq, "", "field:word2");
//    tq->boost = 100;
//    check_hits(tc, searcher, tq, "4, 8, 1", -1);
//    check_to_s(tc, tq, (char *)field, "word2^100.0");
//    check_to_s(tc, tq, "", "field:word2^100.0");
//    q_deref(tq);
//
//    tq = tq_create(term_create(field, "2342"));
//    check_hits(tc, searcher, tq, "", -1);
//    q_deref(tq);
//
//    tq = tq_create(term_create(field, ""));
//    check_hits(tc, searcher, tq, "", -1);
//    q_deref(tq);
//
//    tq = tq_create(term_create(field, "word1"));
//    top_docs = sea_search(searcher, tq, 0, 10, NULL, NULL);
//    Aiequal(SEARCH_DOCS_SIZE, top_docs->total_hits);
//    Aiequal(10, top_docs->size);
//    td_destroy(top_docs);
//
//    top_docs = sea_search(searcher, tq, 0, 20, NULL, NULL);
//    Aiequal(SEARCH_DOCS_SIZE, top_docs->total_hits);
//    Aiequal(SEARCH_DOCS_SIZE, top_docs->size);
//    td_destroy(top_docs);
//
//    top_docs = sea_search(searcher, tq, 10, 20, NULL, NULL);
//    Aiequal(SEARCH_DOCS_SIZE, top_docs->total_hits);
//    Aiequal(SEARCH_DOCS_SIZE - 10, top_docs->size);
//    td_destroy(top_docs);
//    q_deref(tq);
//}
//
//void test_term_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    q1 = tq_create(term_create("A", "a"));
//
//    q2 = tq_create(term_create("A", "a"));
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    Assert(q_eq(q1, q1), "Queries are equal");
//    q_deref(q2);
//
//    q2 = tq_create(term_create("A", "b"));
//    Assert(q_hash(q1) != q_hash(q2), "texts differ");
//    Assert(!q_eq(q1, q2), "texts differ");
//    q_deref(q2);
//
//    q2 = tq_create(term_create("B", "a"));
//    Assert(q_hash(q1) != q_hash(q2), "fields differ");
//    Assert(!q_eq(q1, q2), "fields differ");
//    q_deref(q2);
//
//    q_deref(q1);
//}
//
//void test_boolean_query(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    Query *bq = bq_create(false);
//    Query *tq1 = tq_create(term_create(field, "word1"));
//    Query *tq2 = tq_create(term_create(field, "word3"));
//    Query *tq3 = tq_create(term_create(field, "word2"));
//    bq_add_query(bq, tq1, BC_MUST);
//    bq_add_query(bq, tq2, BC_MUST);
//    check_hits(tc, searcher, bq, "2, 3, 6, 8, 11, 14", 14);
//
//    bq_add_query(bq, tq3, BC_SHOULD);
//    check_hits(tc, searcher, bq, "2, 3, 6, 8, 11, 14", 8);
//    q_deref(bq);
//
//    tq2 = tq_create(term_create(field, "word3"));
//    tq3 = tq_create(term_create(field, "word2"));
//    bq = bq_create(false);
//    bq_add_query(bq, tq2, BC_MUST);
//    bq_add_query(bq, tq3, BC_MUST_NOT);
//    check_hits(tc, searcher, bq, "2, 3, 6, 11, 14", -1);
//    q_deref(bq);
//
//    tq2 = tq_create(term_create(field, "word3"));
//    bq = bq_create(false);
//    bq_add_query(bq, tq2, BC_MUST_NOT);
//    check_hits(tc, searcher, bq, "", -1);
//    q_deref(bq);
//
//    tq2 = tq_create(term_create(field, "word3"));
//    bq = bq_create(false);
//    bq_add_query(bq, tq2, BC_SHOULD);
//    check_hits(tc, searcher, bq, "2, 3, 6, 8, 11, 14", 14);
//    q_deref(bq);
//
//    tq2 = tq_create(term_create(field, "word3"));
//    tq3 = tq_create(term_create(field, "word2"));
//    bq = bq_create(false);
//    bq_add_query(bq, tq2, BC_SHOULD);
//    bq_add_query(bq, tq3, BC_SHOULD);
//    check_hits(tc, searcher, bq, "1, 2, 3, 4, 6, 8, 11, 14", -1);
//    q_deref(bq);
//}
//
//void test_boolean_query_hash(tst_case *tc, void *data)
//{
//    Query *tq1, *tq2, *tq3, *q1, *q2;
//    tq1 = tq_create(term_create("A", "1"));
//    tq2 = tq_create(term_create("B", "2"));
//    tq3 = tq_create(term_create("C", "3"));
//    q1 = bq_create(false);
//    q1->destroy_all = false;
//    bq_add_query(q1, tq1, BC_MUST);
//    bq_add_query(q1, tq2, BC_MUST);
//
//    q2 = bq_create(false);
//    q2->destroy_all = false;
//    bq_add_query(q2, tq1, BC_MUST);
//    bq_add_query(q2, tq2, BC_MUST);
//
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q1), "Queries are equal");
//    Assert(q_eq(q1, q2), "Queries are equal");
//    Assert(q_hash(q1) != q_hash(tq1), "Queries are not equal");
//    Assert(!q_eq(q1, tq1), "Queries are not equal");
//    Assert(!q_eq(tq1, q1), "Queries are not equal");
//    q_deref(q2);
//
//    q2 = bq_create(true);
//    q2->destroy_all = false;
//    bq_add_query(q2, tq1, BC_MUST);
//    bq_add_query(q2, tq2, BC_MUST);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
//    Assert(!q_eq(q1, q2), "Queries are not equal");
//    q_deref(q2);
//
//    q2 = bq_create(false);
//    q2->destroy_all = false;
//    bq_add_query(q2, tq1, BC_SHOULD);
//    bq_add_query(q2, tq2, BC_MUST_NOT);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
//    Assert(!q_eq(q1, q2), "Queries are not equal");
//    q_deref(q2);
//
//    q2 = bq_create(false);
//    q2->destroy_all = false;
//    bq_add_query(q2, tq1, BC_MUST);
//    bq_add_query(q2, tq2, BC_MUST);
//    bq_add_query(q2, tq3, BC_MUST);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
//    Assert(!q_eq(q1, q2), "Queries are not equal");
//
//    bq_add_query(q1, tq3, BC_MUST);
//
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    q_deref(q2);
//
//    q_deref(q1);
//    q_deref(tq1);
//    q_deref(tq2);
//    q_deref(tq3);
//}
//
//void test_phrase_query(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    Query *q;
//
//    Query *phq = phq_create();
//    Term *t1 = term_create(field, "quick");
//    Term *t2 = term_create(field, "brown");
//    Term *t3 = term_create(field, "fox");
//
//    phq_add_term(phq, t1, 1);
//    phq_add_term(phq, t2, 1);
//    phq_add_term(phq, t3, 1);
//    check_to_s(tc, phq, (char *)field, "\"quick brown fox\"");
//    check_to_s(tc, phq, "", "field:\"quick brown fox\"");
//    check_hits(tc, searcher, phq, "1", 1);
//
//    ((PhraseQuery *)phq->data)->slop = 4;
//    check_hits(tc, searcher, phq, "1, 16, 17", 17);
//    q_deref(phq);
//
//    phq = phq_create();
//    t1 = term_create(field, "quick");
//    t3 = term_create(field, "fox");
//    phq_add_term(phq, t1, 1);
//    phq_add_term(phq, t3, 2);
//    check_to_s(tc, phq, (char *)field, "\"quick <> fox\"");
//    check_to_s(tc, phq, "", "field:\"quick <> fox\"");
//    check_hits(tc, searcher, phq, "1, 11, 14", 14);
//
//    ((PhraseQuery *)phq->data)->slop = 1;
//    check_hits(tc, searcher, phq, "1, 11, 14, 16", 14);
//
//    ((PhraseQuery *)phq->data)->slop = 4;
//    check_hits(tc, searcher, phq, "1, 11, 14, 16, 17", 14);
//    q_deref(phq);
//
//    /* test single term case, query is rewritten to TermQuery */
//    phq = phq_create();
//    t1 = term_create(field, "word2");
//    phq_add_term(phq, t1, 1);
//    check_hits(tc, searcher, phq, "4, 8, 1", -1);
//    q = sea_rewrite(searcher, phq);
//    Aiequal(q->type, TERM_QUERY);
//    q_deref(phq);
//    q_deref(q);
//}
//
//void test_phrase_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    Term *t1 = term_create(field, "quick");
//    Term *t2 = term_create(field, "brown");
//    Term *t3 = term_create(field, "fox");
//
//    q1 = phq_create();
//    phq_add_term(q1, t1, 1);
//    phq_add_term(q1, t2, 2);
//    phq_add_term(q1, t3, 3);
//
//    q2 = phq_create();
//    q2->destroy_all = false;
//    phq_add_term(q2, t1, 1);
//    phq_add_term(q2, t2, 2);
//    phq_add_term(q2, t3, 3);
//
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q1), "Test query equals itself");
//    Assert(q_eq(q1, q2), "Queries should be equal");
//
//    ((PhraseQuery *)q2->data)->slop = 5;
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//
//    q2 = phq_create();
//    q2->destroy_all = false;
//    phq_add_term(q2, t1, 1);
//    phq_add_term(q2, t2, 1);
//    phq_add_term(q2, t3, 1);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//
//    q2 = phq_create();
//    q2->destroy_all = false;
//    phq_add_term(q2, t3, 1);
//    phq_add_term(q2, t2, 2);
//    phq_add_term(q2, t1, 3);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//    q_deref(q1);
//}
//
//void test_multi_phrase_query(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    Query *mphq;
//    Term **t1 = ALLOC_N(Term *, 2);
//    Term **t2 = ALLOC_N(Term *, 3);
//    Term **t3 = ALLOC_N(Term *, 1);
//    t1[0] = term_create(field, "quick");
//    t1[1] = term_create(field, "fast");
//    t2[0] = term_create(field, "brown");
//    t2[1] = term_create(field, "red");
//    t2[2] = term_create(field, "hairy");
//    t3[0] = term_create(field, "fox");
//
//    mphq = mphq_create();
//    mphq_add_terms(mphq, t1, 2, 1);
//    check_hits(tc, searcher, mphq, "1, 8, 11, 14, 16, 17", -1);
//
//    mphq_add_terms(mphq, t2, 3, 1);
//    mphq_add_terms(mphq, t3, 1, 1);
//    check_to_s(tc, mphq, (char *)field, "\"quick|fast brown|red|hairy fox\"");
//    check_to_s(tc, mphq, "", "field:\"quick|fast brown|red|hairy fox\"");
//    check_hits(tc, searcher, mphq, "1, 8, 11, 14", -1);
//
//    ((MultiPhraseQuery *)mphq->data)->slop = 4;
//    check_hits(tc, searcher, mphq, "1, 8, 11, 14, 16, 17", -1);
//    q_deref(mphq);
//}
//
//void test_multi_phrase_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    Term **t1 = ALLOC_N(Term *, 2);
//    Term **t2 = ALLOC_N(Term *, 3);
//    Term **t3 = ALLOC_N(Term *, 1);
//    t1[0] = term_create(field, "quick");
//    t1[1] = term_create(field, "fast");
//    t2[0] = term_create(field, "brown");
//    t2[1] = term_create(field, "red");
//    t2[2] = term_create(field, "hairy");
//    t3[0] = term_create(field, "fox");
//
//    q1 = mphq_create();
//    mphq_add_terms(q1, t1, 2, 1);
//    mphq_add_terms(q1, t2, 3, 1);
//    mphq_add_terms(q1, t3, 1, 1);
//
//    q2 = mphq_create();
//    q2->destroy_all = false;
//    mphq_add_terms(q2, t1, 2, 1);
//    mphq_add_terms(q2, t2, 3, 1);
//    mphq_add_terms(q2, t3, 1, 1);
//
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q1), "Test query equals itself");
//    Assert(q_eq(q1, q2), "Queries should be equal");
//
//    ((MultiPhraseQuery *)q2->data)->slop = 5;
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//
//    q2 = mphq_create();
//    q2->destroy_all = false;
//    mphq_add_terms(q2, t1, 2, 1);
//    mphq_add_terms(q2, t2, 3, 2);
//    mphq_add_terms(q2, t3, 1, 3);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//
//    q2 = mphq_create();
//    q2->destroy_all = false;
//    mphq_add_terms(q2, t1, 2, 1);
//    mphq_add_terms(q2, t2, 2, 1);
//    mphq_add_terms(q2, t3, 1, 1);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//
//    q2 = mphq_create();
//    q2->destroy_all = false;
//    mphq_add_terms(q2, t2, 3, 1);
//    mphq_add_terms(q2, t1, 2, 1);
//    mphq_add_terms(q2, t3, 1, 1);
//
//    Assert(q_hash(q1) != q_hash(q2), "Queries should not be equal");
//    Assert(!q_eq(q1, q2), "Queries should not be equal");
//    q_deref(q2);
//
//    q_deref(q1);
//}
//
//void test_prefix_query(tst_case *tc, void *data)
//{
//    Term *t = term_create(cat, "cat1");
//    Searcher *searcher = (Searcher *)data;
//    Query *prq = prefixq_create(t);
//    check_hits(tc, searcher, prq, "0, 1, 2, 3, 4, 13, 14, 15, 16, 17", -1);
//
//    q_deref(prq);
//    t = term_create(cat, "cat1/sub2");
//    prq = prefixq_create(t);
//    check_hits(tc, searcher, prq, "3, 4, 13, 15", -1);
//    q_deref(prq);
//}
//
//void test_prefix_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    q1 = prefixq_create(term_create("A", "a"));
//
//    q2 = prefixq_create(term_create("A", "a"));
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "TermQueries are equal");
//    Assert(q_eq(q1, q1), "TermQueries are equal");
//    q_deref(q2);
//
//    q2 = prefixq_create(term_create("A", "b"));
//    Assert(q_hash(q1) != q_hash(q2), "TermQueries are not equal");
//    Assert(!q_eq(q1, q2), "TermQueries are not equal");
//    q_deref(q2);
//
//    q2 = prefixq_create(term_create("B", "a"));
//    Assert(q_hash(q1) != q_hash(q2), "TermQueries are not equal");
//    Assert(!q_eq(q1, q2), "TermQueries are not equal");
//    q_deref(q2);
//
//    q_deref(q1);
//}
//
//void test_range_query(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    Query *rq;
//    rq = rq_create(date, "20051006", "20051010", true, true);
//    check_hits(tc, searcher, rq, "6,7,8,9,10", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, "20051006", "20051010", false, true);
//    check_hits(tc, searcher, rq, "7,8,9,10", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, "20051006", "20051010", true, false);
//    check_hits(tc, searcher, rq, "6,7,8,9", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, "20051006", "20051010", false, false);
//    check_hits(tc, searcher, rq, "7,8,9", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, NULL, "20051003", false, true);
//    check_hits(tc, searcher, rq, "0,1,2,3", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, NULL, "20051003", false, false);
//    check_hits(tc, searcher, rq, "0,1,2", -1);
//    q_deref(rq);
//
//    rq = rq_create_less(date, "20051003", true);
//    check_hits(tc, searcher, rq, "0,1,2,3", -1);
//    q_deref(rq);
//
//    rq = rq_create_less(date, "20051003", false);
//    check_hits(tc, searcher, rq, "0,1,2", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, "20051014", NULL, true, false);
//    check_hits(tc, searcher, rq, "14,15,16,17", -1);
//    q_deref(rq);
//
//    rq = rq_create(date, "20051014", NULL, false, false);
//    check_hits(tc, searcher, rq, "15,16,17", -1);
//    q_deref(rq);
//
//    rq = rq_create_more(date, "20051014", true);
//    check_hits(tc, searcher, rq, "14,15,16,17", -1);
//    q_deref(rq);
//
//    rq = rq_create_more(date, "20051014", false);
//    check_hits(tc, searcher, rq, "15,16,17", -1);
//    q_deref(rq);
//}
//
//void test_range_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    q1 = rq_create(date, "20051006", "20051010", true, true);
//    q2 = rq_create(date, "20051006", "20051010", true, true);
//
//    Assert(q_eq(q1, q1), "Test same queries are equal");
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    q_deref(q2);
//
//    q2 = rq_create(date, "20051006", "20051010", true, false);
//    Assert(q_hash(q1) != q_hash(q2), "Upper bound include differs");
//    Assert(!q_eq(q1, q2), "Upper bound include differs");
//    q_deref(q2);
//
//    q2 = rq_create(date, "20051006", "20051010", false, true);
//    Assert(q_hash(q1) != q_hash(q2), "Lower bound include differs");
//    Assert(!q_eq(q1, q2), "Lower bound include differs");
//    q_deref(q2);
//
//    q2 = rq_create(date, "20051006", "20051011", true, true);
//    Assert(q_hash(q1) != q_hash(q2), "Upper bound differs");
//    Assert(!q_eq(q1, q2), "Upper bound differs");
//    q_deref(q2);
//
//    q2 = rq_create(date, "20051005", "20051010", true, true);
//    Assert(q_hash(q1) != q_hash(q2), "Lower bound differs");
//    Assert(!q_eq(q1, q2), "Lower bound differs");
//    q_deref(q2);
//
//    q2 = rq_create(date, "20051006", NULL, true, false);
//    Assert(q_hash(q1) != q_hash(q2), "Upper bound is NULL");
//    Assert(!q_eq(q1, q2), "Upper bound is NULL");
//    q_deref(q2);
//
//    q2 = rq_create(date, NULL, "20051010", false, true);
//    Assert(q_hash(q1) != q_hash(q2), "Lower bound is NULL");
//    Assert(!q_eq(q1, q2), "Lower bound is NULL");
//    q_deref(q2);
//
//    q2 = rq_create(field, "20051006", "20051010", true, true);
//    Assert(q_hash(q1) != q_hash(q2), "Field differs");
//    Assert(!q_eq(q1, q2), "Field differs");
//    q_deref(q2);
//    q_deref(q1);
//
//    q1 = rq_create(date, NULL, "20051010", false, true);
//    q2 = rq_create(date, NULL, "20051010", false, true);
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    q_deref(q2);
//    q_deref(q1);
//
//    q1 = rq_create(date, "20051010", NULL, true, false);
//    q2 = rq_create(date, "20051010", NULL, true, false);
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    q_deref(q2);
//    q_deref(q1);
//}
//
//void test_wildcard_match(tst_case *tc, void *data)
//{
//    Assert(wc_match("*", "asdasdg"), "Star matches everything");
//    Assert(wc_match("asd*", "asdasdg"), "Star matches everything after");
//    Assert(wc_match("*dg", "asdasdg"), "Star matches everything before");
//    Assert(wc_match("a?d*", "asdasdg"), "Q-mark matchs one char");
//    Assert(wc_match("?sd*", "asdasdg"), "Q-mark can come first");
//    Assert(wc_match("asd?", "asdg"), "Q-mark can come last");
//    Assert(wc_match("asdg", "asdg"), "No special chars");
//    Assert(!wc_match("asdf", "asdi"), "Do not match");
//    Assert(!wc_match("asd??", "asdg"), "Q-mark must match");
//    Assert(wc_match("as?g", "asdg"), "Q-mark matches in");
//    Assert(!wc_match("as??g", "asdg"), "Q-mark must match");
//    Assert(wc_match("a*?f", "asdf"), "Q-mark and star can appear together");
//    Assert(wc_match("a?*f", "asdf"), "Q-mark and star can appear together");
//    Assert(wc_match("a*?df", "asdf"), "Q-mark and star can appear together");
//    Assert(wc_match("a?*df", "asdf"), "Q-mark and star can appear together");
//    Assert(!wc_match("as*?df", "asdf"), "Q-mark must match");
//    Assert(!wc_match("as?*df", "asdf"), "Q-mark must match");
//    Assert(wc_match("asdf*", "asdf"), "Star can match nothing");
//    Assert(wc_match("asd*f", "asdf"), "Star can match nothing");
//    Assert(wc_match("*asdf*", "asdf"), "Star can match nothing");
//    Assert(wc_match("asd?*****", "asdf"), "Can have multiple stars");
//    Assert(wc_match("as?*****g", "asdg"), "Can have multiple stars");
//    Assert(!wc_match("*asdf", "asdi"), "Do not match");
//    Assert(!wc_match("asdf*", "asdi"), "Do not match");
//    Assert(!wc_match("*asdf*", "asdi"), "Do not match");
//    Assert(!wc_match("cat1*", "cat2/sub1"), "Do not match");
//}
//
//void test_wildcard_query(tst_case *tc, void *data)
//{
//    Searcher *searcher = (Searcher *)data;
//    Query *wq = wcq_create(term_create(cat, "cat1*"));
//    check_hits(tc, searcher, wq, "0, 1, 2, 3, 4, 13, 14, 15, 16, 17", -1);
//
//    q_deref(wq);
//    wq = wcq_create(term_create(cat, "cat1*/su??ub2"));
//    check_hits(tc, searcher, wq, "4, 16", -1);
//    q_deref(wq);
//
//    wq = wcq_create(term_create(cat, "cat1/"));
//    check_hits(tc, searcher, wq, "0, 17", -1);
//    q_deref(wq);
//}
//
//void test_wildcard_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    q1 = wcq_create(term_create("A", "a*"));
//
//    q2 = wcq_create(term_create("A", "a*"));
//    Assert(q_eq(q1, q1), "Test same queries are equal");
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    q_deref(q2);
//
//    q2 = wcq_create(term_create("A", "a?"));
//    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
//    Assert(!q_eq(q1, q2), "Queries are not equal");
//    q_deref(q2);
//
//    q2 = wcq_create(term_create("B", "a?"));
//    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
//    Assert(!q_eq(q1, q2), "Queries are not equal");
//    q_deref(q2);
//    q_deref(q1);
//}
//
//void test_match_all_query_hash(tst_case *tc, void *data)
//{
//    Query *q1, *q2;
//    q1 = maq_create();
//    q2 = maq_create();
//
//    Assert(q_eq(q1, q1), "Test same queries are equal");
//    Aiequal(q_hash(q1), q_hash(q2));
//    Assert(q_eq(q1, q2), "Queries are equal");
//    q_deref(q2);
//
//    q2 = wcq_create(term_create("A", "a*"));
//    Assert(q_hash(q1) != q_hash(q2), "Queries are not equal");
//    Assert(!q_eq(q1, q2), "Queries are not equal");
//    q_deref(q2);
//
//    q_deref(q1);
//}

void test_explanation(tst_case *tc, void *data)
{
    Explanation *expl = expl_new(1.6, "short description");
    char *str = expl_to_s(expl);
    (void)data;
    Asequal("1.6 = short description\n", str);
    free(str);
    expl_add_detail(expl, expl_new(0.8, "half the score"));
    expl_add_detail(expl, expl_new(2.0, "to make the difference"));
    expl_add_detail(expl->details[1], expl_new(0.5, "sub-sub"));
    expl_add_detail(expl->details[1], expl_new(4.0, "another sub-sub"));
    expl_add_detail(expl->details[0], expl_new(0.8, "and sub-sub for 1st sub"));

    str = expl_to_s(expl);
    Asequal("1.6 = short description\n"
            "  0.8 = half the score\n"
            "    0.8 = and sub-sub for 1st sub\n"
            "  2.0 = to make the difference\n"
            "    0.5 = sub-sub\n"
            "    4.0 = another sub-sub\n", str);
    free(str);
}

tst_suite *ts_search(tst_suite *suite)
{
    Store *store = open_ram_store();
    IndexReader *ir;
    //Searcher *searcher;

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_explanation, NULL);
    tst_run_test(suite, test_byte_float_conversion, NULL);
    //tst_run_test(suite, test_default_similarity, NULL);

    //prepare_search_index(store);
    //ir = ir_open(store);
    //searcher = sea_create(ir);

    //tst_run_test(suite, test_get_doc, (void *)searcher);

    //tst_run_test(suite, test_term_query, (void *)searcher);
    //tst_run_test(suite, test_term_query_hash, NULL);

    //tst_run_test(suite, test_boolean_query, (void *)searcher);
    //tst_run_test(suite, test_boolean_query_hash, NULL);

    //tst_run_test(suite, test_phrase_query, (void *)searcher);
    //tst_run_test(suite, test_phrase_query_hash, NULL);

    //tst_run_test(suite, test_multi_phrase_query, (void *)searcher);
    //tst_run_test(suite, test_multi_phrase_query_hash, NULL);

    //tst_run_test(suite, test_prefix_query, (void *)searcher);
    //tst_run_test(suite, test_prefix_query_hash, NULL);

    //tst_run_test(suite, test_range_query, (void *)searcher);
    //tst_run_test(suite, test_range_query_hash, NULL);

    //tst_run_test(suite, test_wildcard_match, (void *)searcher);
    //tst_run_test(suite, test_wildcard_query, (void *)searcher);
    //tst_run_test(suite, test_wildcard_query_hash, NULL);

    //tst_run_test(suite, test_match_all_query_hash, NULL);

    //store_deref(store);
    //sea_close(searcher);
    return suite;
}

#include "search.h"
#include "symbol.h"
#include "internal.h"
#include "ind.h"
#include "testhelper.h"
#include "test.h"

extern char *num_to_str(int num);

#define test_num(n, expected) num = num_to_str(n); Asequal(expected, num); free(num)

static void test_number_to_str(TestCase *tc, void *data)
{
    char *num;
    (void)data;
    test_num(0, "zero");
    test_num(9, "nine");
    test_num(10, "ten");
    test_num(13, "thirteen");
    test_num(19, "nineteen");
    test_num(20, "twenty");
    test_num(21, "twenty one");
    test_num(99, "ninety nine");
    test_num(100, "one hundred");
    test_num(101, "one hundred and one");
    test_num(111, "one hundred and eleven");
    test_num(1111, "one thousand one hundred and eleven");
    test_num(22222, "twenty two thousand two hundred and twenty two");
    test_num(333333, "three hundred and thirty three thousand three hundred and thirty three");
    test_num(8712387, "eight million seven hundred and twelve thousand three hundred and eighty seven");
    test_num(1000000000, "one billion");
    test_num(-8712387, "negative eight million seven hundred and twelve thousand three hundred and eighty seven");

}

void dummy_log(const void *fmt, ...) {(void)fmt;}
#define ITERATIONS 10
#define NTHREADS 10
#ifdef FRT_HAS_VARARGS
#define tlog(...)
#else
#define tlog dummy_log
#endif
/*#define tlog printf */

static void do_optimize(Index *index)
{
    tlog("Optimizing the index\n");
    index_optimize(index);
}

static void do_delete_doc(Index *index)
{
    int size;
    if ((size=index_size(index)) > 0) {
        int doc_num = rand() % size;
        tlog("Deleting %d from index which has%s deletions\n",
             doc_num, (index_has_del(index) ? "" : " no"));
        if (index_is_deleted(index, doc_num)) {
            tlog("document was already deleted\n");
        }
        else {
            index_delete(index, doc_num);
        }
    }
}

static char *id = "id";
static char *contents = "contents";

static void do_add_doc(Index *index)
{
    Document *doc = doc_new();
    int n = rand();

    doc_add_field(doc, df_add_data(df_new(I(id)), strfmt("%d", n)))->destroy_data = true;
    doc_add_field(doc, df_add_data(df_new(I(contents)), num_to_str(n)))->destroy_data = true;
    tlog("Adding %d\n", n);
    index_add_doc(index, doc);
    doc_destroy(doc);
}

static void do_search(Index *index)
{
    int n = rand(), i;
    char *query = num_to_str(n);
    TopDocs *td;

    tlog("Searching for %d\n", n);

    mutex_lock(&index->mutex);
    td = index_search_str(index, query, 0, 3, NULL, NULL, NULL);
    free(query);
    for (i = 0; i < td->size; i++) {
        Hit *hit = td->hits[i];
        Document *doc = index_get_doc(index, hit->doc);
        tlog("Hit for %d: %s - %f\n",
             hit->doc, doc_get_field(doc, id)->data[0], hit->score);
        doc_destroy(doc);
    }
    tlog("Searched for %d: total = %d\n", n, td->total_hits);
    mutex_unlock(&index->mutex);

    td_destroy(td);
}

static void *indexing_thread(void *p)
{
    int i, choice;
    Index *index = (Index *)p;

    for (i = 0; i < ITERATIONS; i++) {
        choice = rand() % 1000;

        if (choice > 999) {
            do_optimize(index);
        } else if (choice > 900) {
            do_delete_doc(index);
        } else if (choice > 700) {
            do_search(index);
        } else {
            do_add_doc(index);
        }
    }
    return NULL;
}

static void test_threading_test(TestCase *tc, void *data)
{
    Index *index = (Index *)data;
    (void)data;
    (void)tc;
    indexing_thread(index);
}

static void test_threading(TestCase *tc, void *data)
{
    int i;
    pthread_t thread_id[NTHREADS];
    Index *index = (Index *)data;
    (void)data;
    (void)tc;

    for(i=0; i < NTHREADS; i++) {
        pthread_create(&thread_id[i], NULL, &indexing_thread, index );
    }

    for(i=0; i < NTHREADS; i++) {
        pthread_join(thread_id[i], NULL); 
    }
}

TestSuite *ts_threading(TestSuite *suite)
{
    Analyzer *a = letter_analyzer_new(true);
    Store *store = open_fs_store("./test/testdir/store");
    Index *index;
    HashSet *def_fields = hs_new_str(NULL);
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_new(I(id), STORE_YES, INDEX_UNTOKENIZED,
                              TERM_VECTOR_YES));
    index_create(store, fis);
    fis_deref(fis);

    hs_add(def_fields, (char *)I(contents));
    store->clear_all(store);
    index = index_new(store, a, def_fields, true);
    hs_destroy(def_fields);

    suite = ADD_SUITE(suite);

    store_deref(store);
    a_deref(a);

    tst_run_test(suite, test_number_to_str, NULL);
    tst_run_test(suite, test_threading_test, index);
    tst_run_test(suite, test_threading, index);

    index_destroy(index);

    store = open_fs_store("./test/testdir/store");
    store->clear_all(store);
    store_deref(store);

    return suite;
}

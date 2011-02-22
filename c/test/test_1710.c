#include "index.h"
#include "test.h"
#include "testhelper.h"
#include <stdio.h>

static FieldInfos *create_fis()
{
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    return fis;
}

static IndexWriter *create_iw(Store *store)
{
    FieldInfos *fis = create_fis();
    index_create(store, fis);
    fis_deref(fis);
    return iw_open(store, standard_analyzer_new(true), &default_config);
}

static Document *prep_doc()
{
    Document *doc = doc_new();
    doc_add_field(
        doc,
        df_add_data(
            df_new(I("content")),
            estrdup("http://_____________________________________________________")
            )
        )->destroy_data = true;
    return doc;

}

static void test_problem_text(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw = create_iw(store);
    Document *problem_text = prep_doc();

    iw_add_doc(iw, problem_text);
    Aiequal(1, iw_doc_count(iw));
    Assert(!store->exists(store, "_0.cfs"),
           "data shouldn't have been written yet");
    iw_commit(iw);
    Assert(store->exists(store, "_0.cfs"), "data should now be written");
    iw_close(iw);
    Assert(store->exists(store, "_0.cfs"), "data should still be there");
}

TestSuite *ts_1710(TestSuite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_problem_text, store);

    store_deref(store);

    return suite;
}


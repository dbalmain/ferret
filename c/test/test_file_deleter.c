#include "index.h"
#include "testhelper.h"
#include <limits.h>
#include "test.h"

static char *content_f = "content";
static char *id_f = "id";
const Config lucene_config = {
    0x100000,       /* chunk size is 1Mb */
    0x1000000,      /* Max memory used for buffer is 16 Mb */
    INDEX_INTERVAL, /* index interval */
    SKIP_INTERVAL,  /* skip interval */
    10,             /* default merge factor */
    10,             /* max_buffered_docs */
    INT_MAX,        /* max_merged_docs */
    10000,          /* maximum field length (number of terms) */
    true            /* use compound file by default */
};


static FieldInfos *prep_fis()
{
    return fis_new(STORE_NO, INDEX_YES, TERM_VECTOR_NO);
}

static void create_index(Store *store)
{
    FieldInfos *fis = prep_fis();
    index_create(store, fis);
    fis_deref(fis);
}

/*
static IndexWriter *create_iw(Store *store)
{
    create_index(store);
    return iw_open(store, whitespace_analyzer_new(false), &default_config);
}

static IndexWriter *create_iw_conf(Store *store, int max_buffered_docs,
                                   int merge_factor)
{
    Config config = default_config;
    config.max_buffered_docs = max_buffered_docs;
    config.merge_factor = merge_factor;
    create_index(store);
    return iw_open(store, whitespace_analyzer_new(false), &config);
}
*/

static IndexWriter *create_iw_lucene(Store *store)
{
    create_index(store);
    return iw_open(store, whitespace_analyzer_new(false), &lucene_config);
}

static void add_doc(IndexWriter *iw, int id)
{
    Document *doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(I(content_f)),
                                   estrdup("aaa")))->destroy_data = true;
    doc_add_field(doc, df_add_data(df_new(I(id_f)),
                                   strfmt("%d", id)))->destroy_data = true;
    iw_add_doc(iw, doc);
    doc_destroy(doc);
}

static void add_docs(IndexWriter *iw, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        add_doc(iw, i);
    }
}

static void copy_file(Store *store, char *src, char *dest)
{
    InStream *is = store->open_input(store, src);
    OutStream *os = store->new_output(store, dest);
    is2os_copy_bytes(is, os, is_length(is));
    is_close(is);
    os_close(os);
}

/*
 * Verify we can read the pre-XXX file format, do searches
 * against it, and add documents to it.
 */
static void test_delete_leftover_files(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw = create_iw_lucene(store);
    IndexReader *ir;
    char *store_before, *store_after;
    add_docs(iw, 35);
    iw_close(iw);

    /* Delete one doc so we get a .del file: */
    ir = ir_open(store);
    ir_delete_doc(ir, 7);
    Aiequal(1, ir->max_doc(ir) - ir->num_docs(ir));

    /* Set one norm so we get a .s0 file: */
    ir_set_norm(ir, 21, I(content_f), 12);
    ir_close(ir);
    store_before = store_to_s(store);

    /* Create a bogus separate norms file for a
     * segment/field that actually has a separate norms file
     * already: */
    copy_file(store, "_2_1.s0", "_2_2.s0");

    /* Create a bogus separate norms file for a
     * segment/field that actually has a separate norms file
     * already, using the "not compound file" extension: */
    copy_file(store, "_2_1.s0", "_2_2.f0");

    /* Create a bogus separate norms file for a
     * segment/field that does not have a separate norms
     * file already: */
    copy_file(store, "_2_1.s0", "_1_1.s0");

    /* Create a bogus separate norms file for a
     * segment/field that does not have a separate norms
     * file already using the "not compound file" extension: */
    copy_file(store, "_2_1.s0", "_1_1.f0");

    /* Create a bogus separate del file for a
     * segment that already has a separate del file:  */
    copy_file(store, "_0_0.del", "_0_1.del");

    /* Create a bogus separate del file for a
     * segment that does not yet have a separate del file: */
    copy_file(store, "_0_0.del", "_1_1.del");

    /* Create a bogus separate del file for a
     * non-existent segment: */
    copy_file(store, "_0_0.del", "_188_1.del");

    /* Create a bogus segment file: */
    copy_file(store, "_0.cfs", "_188.cfs");

    /* Create a bogus frq file when the CFS already exists: */
    copy_file(store, "_0.cfs", "_0.frq");

    /* Create a bogus frq file when the CFS already exists: */
    copy_file(store, "_0.cfs", "_0.frq");
    copy_file(store, "_0.cfs", "_0.prx");
    copy_file(store, "_0.cfs", "_0.fdx");
    copy_file(store, "_0.cfs", "_0.fdt");
    copy_file(store, "_0.cfs", "_0.tfx");
    copy_file(store, "_0.cfs", "_0.tix");
    copy_file(store, "_0.cfs", "_0.tis");

    /* Create some old segments file: */
    copy_file(store, "segments_5", "segments");
    copy_file(store, "segments_5", "segments_2");


    /* Open & close a writer: should delete the above files and nothing more: */
    iw_close(iw_open(store, whitespace_analyzer_new(false), &lucene_config));

    store_after = store_to_s(store);

    Asequal(store_before, store_after);
    free(store_before);
    free(store_after);
}

/***************************************************************************
 *
 * IndexFileDeleterSuite
 *
 ***************************************************************************/

TestSuite *ts_file_deleter(TestSuite *suite)
{
    Store *store = open_ram_store();
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_delete_leftover_files, store);

    store_deref(store);
    return suite;
}

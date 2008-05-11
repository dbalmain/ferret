#include "index.h"
#include "test.h"

#define do_field_prop_test(tc, fi, name, boost, is_stored,\
                           is_compressed, is_indexed, is_tokenized, omit_norms,\
                           store_term_vector, store_positions, store_offsets)\
        field_prop_test(tc, __LINE__, fi, I(name), boost, is_stored,\
                        is_compressed, is_indexed, is_tokenized, omit_norms,\
                        store_term_vector, store_positions, store_offsets)
#define T 1
#define F 0

void field_prop_test(TestCase *tc,
                     int line_num,
                     FieldInfo *fi,
                     Symbol name,
                     float boost,
                     bool is_stored,
                     bool is_compressed,
                     bool is_indexed,
                     bool is_tokenized,
                     bool omit_norms,
                     bool store_term_vector,
                     bool store_positions,
                     bool store_offsets)
{
    tst_ptr_equal(line_num, tc, name, fi->name);
    tst_flt_equal(line_num, tc, boost, fi->boost);
    tst_int_equal(line_num, tc, is_stored,          fi_is_stored(fi));
    tst_int_equal(line_num, tc, is_compressed,      fi_is_compressed(fi));
    tst_int_equal(line_num, tc, is_indexed,         fi_is_indexed(fi));
    tst_int_equal(line_num, tc, is_tokenized,       fi_is_tokenized(fi));
    tst_int_equal(line_num, tc, omit_norms,         fi_omit_norms(fi));
    tst_int_equal(line_num, tc, store_term_vector,  fi_store_term_vector(fi));
    tst_int_equal(line_num, tc, store_positions,    fi_store_positions(fi));
    tst_int_equal(line_num, tc, store_offsets,      fi_store_offsets(fi));
}

/****************************************************************************
 *
 * FieldInfo
 *
 ****************************************************************************/

static void test_fi_new(TestCase *tc, void *data)
{
    FieldInfo *fi;
    (void)data; /* suppress unused argument warning */

    fi = fi_new(I("name"), STORE_NO, INDEX_NO, TERM_VECTOR_NO);
    do_field_prop_test(tc, fi, "name", 1.0, F, F, F, F, F, F, F, F);
    fi_deref(fi);
    fi = fi_new(I("name"), STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    do_field_prop_test(tc, fi, "name", 1.0, T, F, T, T, F, T, F, F);
    fi_deref(fi);
    fi = fi_new(I("name"), STORE_COMPRESS, INDEX_UNTOKENIZED,
                   TERM_VECTOR_WITH_POSITIONS);
    do_field_prop_test(tc, fi, "name", 1.0, T, T, T, F, F, T, T, F);
    fi_deref(fi);
    fi = fi_new(I("name"), STORE_NO, INDEX_YES_OMIT_NORMS,
                   TERM_VECTOR_WITH_OFFSETS);
    do_field_prop_test(tc, fi, "name", 1.0, F, F, T, T, T, T, F, T);
    fi_deref(fi);
    fi = fi_new(I("name"), STORE_NO, INDEX_UNTOKENIZED_OMIT_NORMS,
                   TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fi->boost = 1000.0;
    do_field_prop_test(tc, fi, "name", 1000.0, F, F, T, F, T, T, T, T);
    fi_deref(fi);
}

/****************************************************************************
 *
 * FieldInfos
 *
 ****************************************************************************/

static void test_fis_basic(TestCase *tc, void *data)
{
    FieldInfos *fis;
    FieldInfo *fi;
    volatile bool arg_error = false;
    (void)data; /* suppress unused argument warning */

    fis = fis_new(STORE_NO, INDEX_NO, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new(I("FFFFFFFF"), STORE_NO, INDEX_NO,
                                 TERM_VECTOR_NO));
    fis_add_field(fis, fi_new(I("TFTTFTFF"), STORE_YES, INDEX_YES,
                                 TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("TTTFFTTF"), STORE_COMPRESS, INDEX_UNTOKENIZED,
                                 TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_new(I("FFTTTTFT"), STORE_NO, INDEX_YES_OMIT_NORMS,
                                 TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new(I("FFTFTTTT"), STORE_NO,
                                 INDEX_UNTOKENIZED_OMIT_NORMS,
                                 TERM_VECTOR_WITH_POSITIONS_OFFSETS));

    fi = fi_new(I("FFTFTTTT"), STORE_NO, INDEX_UNTOKENIZED_OMIT_NORMS,
                   TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    TRY
        Apnull(fis_add_field(fis, fi));
    case ARG_ERROR:
        arg_error = true;
        HANDLED();
    XENDTRY
    Assert(arg_error, "exception should have been thrown");

    fi_deref(fi);

    Apequal(fis_get_field(fis, I("FFFFFFFF")), fis->fields[0]);
    Apequal(fis_get_field(fis, I("TFTTFTFF")), fis->fields[1]);
    Apequal(fis_get_field(fis, I("TTTFFTTF")), fis->fields[2]);
    Apequal(fis_get_field(fis, I("FFTTTTFT")), fis->fields[3]);
    Apequal(fis_get_field(fis, I("FFTFTTTT")), fis->fields[4]);

    Aiequal(0, fis_get_field(fis, I("FFFFFFFF"))->number);
    Aiequal(1, fis_get_field(fis, I("TFTTFTFF"))->number);
    Aiequal(2, fis_get_field(fis, I("TTTFFTTF"))->number);
    Aiequal(3, fis_get_field(fis, I("FFTTTTFT"))->number);
    Aiequal(4, fis_get_field(fis, I("FFTFTTTT"))->number);

    Asequal("FFFFFFFF", fis->fields[0]->name);
    Asequal("TFTTFTFF", fis->fields[1]->name);
    Asequal("TTTFFTTF", fis->fields[2]->name);
    Asequal("FFTTTTFT", fis->fields[3]->name);
    Asequal("FFTFTTTT", fis->fields[4]->name);

    fis->fields[1]->boost = 2.0;
    fis->fields[2]->boost = 3.0;
    fis->fields[3]->boost = 4.0;
    fis->fields[4]->boost = 5.0;

    do_field_prop_test(tc, fis->fields[0], "FFFFFFFF", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[1], "TFTTFTFF", 2.0,
                       T, F, T, T, F, T, F, F);
    do_field_prop_test(tc, fis->fields[2], "TTTFFTTF", 3.0,
                       T, T, T, F, F, T, T, F);
    do_field_prop_test(tc, fis->fields[3], "FFTTTTFT", 4.0,
                       F, F, T, T, T, T, F, T);
    do_field_prop_test(tc, fis->fields[4], "FFTFTTTT", 5.0,
                       F, F, T, F, T, T, T, T);

    fis_deref(fis);
}

static void test_fis_with_default(TestCase *tc, void *data)
{
    FieldInfos *fis;
    (void)data; /* suppress unused argument warning */

    fis = fis_new(STORE_NO, INDEX_NO, TERM_VECTOR_NO);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("name")), "name", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("dave")), "dave", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("wert")), "wert", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[0], "name", 1.0, F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[1], "dave", 1.0, F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[2], "wert", 1.0, F, F, F, F, F, F, F, F);
    Apnull(fis_get_field(fis, I("random")));
    fis_deref(fis);

    fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("name")), "name", 1.0,
                       T, F, T, T, F, T, F, F);
    fis_deref(fis);
    fis = fis_new(STORE_COMPRESS, INDEX_UNTOKENIZED,
                     TERM_VECTOR_WITH_POSITIONS);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("name")), "name", 1.0,
                       T, T, T, F, F, T, T, F);
    fis_deref(fis);
    fis = fis_new(STORE_NO, INDEX_YES_OMIT_NORMS,
                     TERM_VECTOR_WITH_OFFSETS);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("name")), "name", 1.0,
                       F, F, T, T, T, T, F, T);
    fis_deref(fis);
    fis = fis_new(STORE_NO, INDEX_UNTOKENIZED_OMIT_NORMS,
                     TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("name")), "name", 1.0,
                       F, F, T, F, T, T, T, T);
    fis_deref(fis);
}

static void test_fis_rw(TestCase *tc, void *data)
{
    char *str;
    FieldInfos *fis;
    Store *store = open_ram_store();
    InStream *is;
    OutStream *os;
    (void)data; /* suppress unused argument warning */

    fis = fis_new(STORE_YES, INDEX_UNTOKENIZED_OMIT_NORMS, 
                  TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_new(I("FFFFFFFF"), STORE_NO, INDEX_NO,
                                 TERM_VECTOR_NO));
    fis_add_field(fis, fi_new(I("TFTTFTFF"), STORE_YES, INDEX_YES,
                                 TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("TTTFFTTF"), STORE_COMPRESS, INDEX_UNTOKENIZED,
                                 TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_new(I("FFTTTTFT"), STORE_NO, INDEX_YES_OMIT_NORMS,
                                 TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new(I("FFTFTTTT"), STORE_NO,
                                 INDEX_UNTOKENIZED_OMIT_NORMS,
                                 TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    fis->fields[1]->boost = 2.0;
    fis->fields[2]->boost = 3.0;
    fis->fields[3]->boost = 4.0;
    fis->fields[4]->boost = 5.0;

    os = store->new_output(store, "fields");
    fis_write(fis, os);
    os_close(os);


    /* these fields won't be saved be will added again later */
    Aiequal(5, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("new_field")),
                       "new_field", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(6, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("another")),
                       "another", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(7, fis->size);

    fis_deref(fis);

    is = store->open_input(store, "fields");
    fis = fis_read(is);
    is_close(is);
    Aiequal(STORE_YES, fis->store);
    Aiequal(INDEX_UNTOKENIZED_OMIT_NORMS, fis->index);
    Aiequal(TERM_VECTOR_WITH_POSITIONS_OFFSETS, fis->term_vector);

    do_field_prop_test(tc, fis->fields[0], "FFFFFFFF", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[1], "TFTTFTFF", 2.0,
                       T, F, T, T, F, T, F, F);
    do_field_prop_test(tc, fis->fields[2], "TTTFFTTF", 3.0,
                       T, T, T, F, F, T, T, F);
    do_field_prop_test(tc, fis->fields[3], "FFTTTTFT", 4.0,
                       F, F, T, T, T, T, F, T);
    do_field_prop_test(tc, fis->fields[4], "FFTFTTTT", 5.0,
                       F, F, T, F, T, T, T, T);
    Aiequal(5, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("new_field")),
                       "new_field", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(6, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, I("another")),
                       "another", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(7, fis->size);
    str=fis_to_s(fis);
    Asequal("default:\n"
            "  store: :yes\n"
            "  index: :untokenized_omit_norms\n"
            "  term_vector: :with_positions_offsets\n"
            "fields:\n"
            "  FFFFFFFF:\n"
            "    boost: 1.000000\n"
            "    store: :no\n"
            "    index: :no\n"
            "    term_vector: :no\n"
            "  TFTTFTFF:\n"
            "    boost: 2.000000\n"
            "    store: :yes\n"
            "    index: :yes\n"
            "    term_vector: :yes\n"
            "  TTTFFTTF:\n"
            "    boost: 3.000000\n"
            "    store: :compressed\n"
            "    index: :untokenized\n"
            "    term_vector: :with_positions\n"
            "  FFTTTTFT:\n"
            "    boost: 4.000000\n"
            "    store: :no\n"
            "    index: :omit_norms\n"
            "    term_vector: :with_offsets\n"
            "  FFTFTTTT:\n"
            "    boost: 5.000000\n"
            "    store: :no\n"
            "    index: :untokenized_omit_norms\n"
            "    term_vector: :with_positions_offsets\n"
            "  new_field:\n"
            "    boost: 1.000000\n"
            "    store: :yes\n"
            "    index: :untokenized_omit_norms\n"
            "    term_vector: :with_positions_offsets\n"
            "  another:\n"
            "    boost: 1.000000\n"
            "    store: :yes\n"
            "    index: :untokenized_omit_norms\n"
            "    term_vector: :with_positions_offsets\n", str);
    free(str);

    fis_deref(fis);
    store_deref(store);
}

/****************************************************************************
 *
 * FieldsReader/FieldsWriter
 *
 ****************************************************************************/

#define BIN_DATA_LEN 1234

static char *prepare_bin_data(int len)
{
  int i;
  char *bin_data = ALLOC_N(char, len);
  for (i = 0; i < len; i++) {
    bin_data[i] = i;
  }
  return bin_data;
}

#define check_df_data(df, index, mdata)\
    do {\
        Aiequal(strlen(mdata), df->lengths[index]);\
        Asequal(mdata, df->data[index]);\
    } while (0)

#define check_df_bin_data(df, index, mdata, mlen)\
    do {\
        Aiequal(mlen, df->lengths[index]);\
        Assert(memcmp(mdata, df->data[index], mlen) == 0, "Data should be equal");\
    } while (0)

static Document *prepare_doc()
{
    Document *doc = doc_new();
    DocField *df;
    char *bin_data = prepare_bin_data(BIN_DATA_LEN);

    doc_add_field(doc, df_add_data(df_new(I("ignored")), "this fld's ignored"));
    doc_add_field(doc, df_add_data(df_new(I("unstored")), "unstored ignored"));
    doc_add_field(doc, df_add_data(df_new(I("stored")), "Yay, a stored field"));
    df = doc_add_field(doc, df_add_data(df_new(I("stored_array")), "one"));
    df->destroy_data = false;
    df_add_data(df, "two");
    df_add_data(df, "three");
    df_add_data(df, "four");
    df_add_data_len(df, bin_data, BIN_DATA_LEN);
    doc_add_field(doc, df_add_data_len(df_new(I("binary")), bin_data,
                                       BIN_DATA_LEN))->destroy_data = true;
    df = doc_add_field(doc, df_add_data(df_new(I("array")), "ichi"));
    df_add_data(df, "ni");
    df_add_data(df, "san");
    df_add_data(df, "yon");
    df_add_data(df, "go");

    return doc;
}

static FieldInfos *prepare_fis()
{
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new(I("ignored"), STORE_NO, INDEX_NO,
                                 TERM_VECTOR_NO));
    fis_add_field(fis, fi_new(I("unstored"), STORE_NO, INDEX_YES,
                                 TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    fis_add_field(fis, fi_new(I("stored"), STORE_YES, INDEX_YES,
                                 TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("stored_array"), STORE_COMPRESS,
                                 INDEX_UNTOKENIZED, TERM_VECTOR_NO));
    return fis;
}

static void test_fields_rw_single(TestCase *tc, void *data)
{
    Store *store = open_ram_store();
    char *bin_data = prepare_bin_data(BIN_DATA_LEN);
    Document *doc = prepare_doc();
    FieldInfos *fis = prepare_fis();
    FieldsWriter *fw;
    FieldsReader *fr;
    DocField *df;
    (void)data;

    Aiequal(4, fis->size);
    Aiequal(6, doc->size);

    fw = fw_open(store, "_0", fis);
    fw_add_doc(fw, doc);
    fw_write_tv_index(fw);
    fw_close(fw);
    doc_destroy(doc);

    Aiequal(6, fis->size);
    do_field_prop_test(tc, fis_get_field(fis, I("binary")), "binary", 1.0,
                       T, F, T, T, F, F, F, F);
    do_field_prop_test(tc, fis_get_field(fis, I("array")), "array", 1.0,
                       T, F, T, T, F, F, F, F);

    fr = fr_open(store, "_0", fis);
    doc = fr_get_doc(fr, 0);
    fr_close(fr);

    Aiequal(4, doc->size);

    Apnull(doc_get_field(doc, I("ignored")));
    Apnull(doc_get_field(doc, I("unstored")));

    df = doc_get_field(doc, I("stored"));
    Aiequal(1, df->size);
    check_df_data(df, 0, "Yay, a stored field");

    df = doc_get_field(doc, I("stored_array"));
    Aiequal(5, df->size);
    check_df_data(df, 0, "one");
    check_df_data(df, 1, "two");
    check_df_data(df, 2, "three");
    check_df_data(df, 3, "four");
    check_df_bin_data(df, 4, bin_data, BIN_DATA_LEN);

    df = doc_get_field(doc, I("binary"));
    Aiequal(1, df->size);
    check_df_bin_data(df, 0, bin_data, BIN_DATA_LEN);

    df = doc_get_field(doc, I("array"));
    Aiequal(5, df->size);
    check_df_data(df, 0, "ichi");
    check_df_data(df, 1, "ni");
    check_df_data(df, 2, "san");
    check_df_data(df, 3, "yon");
    check_df_data(df, 4, "go");

    free(bin_data);
    store_deref(store);
    doc_destroy(doc);
    fis_deref(fis);
}

static void test_fields_rw_multi(TestCase *tc, void *data)
{
    int i;
    Store *store = open_ram_store();
    char *bin_data = prepare_bin_data(BIN_DATA_LEN);
    Document *doc;
    FieldInfos *fis = prepare_fis();
    FieldsWriter *fw;
    FieldsReader *fr;
    DocField *df;
    (void)data;
    
    fw = fw_open(store, "_as3", fis);
    for (i = 0; i < 100; i++) {
        char buf[100];
        sprintf(buf, "<<%d>>", i);
        doc = doc_new();
        doc_add_field(doc, df_add_data(df_new(I(buf)), buf));
        fw_add_doc(fw, doc);
        fw_write_tv_index(fw);
        doc_destroy(doc);
    }

    doc = prepare_doc();
    fw_add_doc(fw, doc);
    fw_write_tv_index(fw);
    doc_destroy(doc);
    fw_close(fw);

    Aiequal(106, fis->size);
    do_field_prop_test(tc, fis_get_field(fis, I("binary")), "binary", 1.0,
                       T, F, T, T, F, F, F, F);
    do_field_prop_test(tc, fis_get_field(fis, I("array")), "array", 1.0,
                       T, F, T, T, F, F, F, F);
    for (i = 0; i < 100; i++) {
        char buf[100];
        sprintf(buf, "<<%d>>", i);
        do_field_prop_test(tc, fis_get_field(fis, I(buf)), buf, 1.0,
                           T, F, T, T, F, F, F, F);
    }

    fr = fr_open(store, "_as3", fis);
    doc = fr_get_doc(fr, 100);
    fr_close(fr);

    Aiequal(4, doc->size);

    Apnull(doc_get_field(doc, I("ignored")));
    Apnull(doc_get_field(doc, I("unstored")));

    df = doc_get_field(doc, I("stored"));
    Aiequal(1, df->size);
    check_df_data(df, 0, "Yay, a stored field");

    df = doc_get_field(doc, I("stored_array"));
    Aiequal(5, df->size);
    check_df_data(df, 0, "one");
    check_df_data(df, 1, "two");
    check_df_data(df, 2, "three");
    check_df_data(df, 3, "four");
    check_df_bin_data(df, 4, bin_data, BIN_DATA_LEN);

    df = doc_get_field(doc, I("binary"));
    Aiequal(1, df->size);
    check_df_bin_data(df, 0, bin_data, BIN_DATA_LEN);

    df = doc_get_field(doc, I("array"));
    Aiequal(5, df->size);
    check_df_data(df, 0, "ichi");
    check_df_data(df, 1, "ni");
    check_df_data(df, 2, "san");
    check_df_data(df, 3, "yon");
    check_df_data(df, 4, "go");

    free(bin_data);
    store_deref(store);
    doc_destroy(doc);
    fis_deref(fis);
}

static void test_lazy_field_loading(TestCase *tc, void *data)
{
    Store *store = open_ram_store();
    Document *doc;
    FieldInfos *fis = prepare_fis();
    FieldsWriter *fw;
    FieldsReader *fr;
    DocField *df;
    LazyDoc *lazy_doc;
    LazyDocField *lazy_df;
    char *text, buf[1000];
    (void)data;
    
    fw = fw_open(store, "_as3", fis);
    doc = doc_new();
    df = df_new(I("stored"));
    df_add_data(df, "this is a stored field");
    df_add_data(df, "to be or not to be");
    df_add_data(df, "a stitch in time, saves nine");
    df_add_data(df, "the quick brown fox jumped over the lazy dog");
    df_add_data(df, "that's it folks");
    doc_add_field(doc, df);
    fw_add_doc(fw, doc);
    fw_write_tv_index(fw);
    doc_destroy(doc);
    fw_close(fw);

    fr = fr_open(store, "_as3", fis);
    lazy_doc = fr_get_lazy_doc(fr, 0);
    fr_close(fr);
    fis_deref(fis);
    store_deref(store);

    lazy_df = lazy_doc_get(lazy_doc, I("stored"));
    Apnull(lazy_doc->fields[0]->data[0].text);
    Asequal("this is a stored field", text=lazy_df_get_data(lazy_df, 0));
    Asequal("this is a stored field", lazy_doc->fields[0]->data[0].text);
    Apequal(text, lazy_df_get_data(lazy_df, 0));

    Apnull(lazy_doc->fields[0]->data[4].text);
    Asequal("that's it folks", text=lazy_df_get_data(lazy_df, 4));
    Asequal("that's it folks", lazy_doc->fields[0]->data[4].text);
    Apequal(text, lazy_df_get_data(lazy_df, 4));

    lazy_df_get_bytes(lazy_df, buf, 17, 8);
    buf[8] = 0;
    Asequal("field to", buf);
    lazy_df_get_bytes(lazy_df, buf, 126, 5);
    buf[5] = 0;
    Asequal("folks", buf);
    lazy_df_get_bytes(lazy_df, buf, 0, 131);
    buf[131] = 0;
    Asequal("this is a stored field to be or not to be a stitch in time, "
             "saves nine the quick brown fox jumped over the lazy dog "
             "that's it folks", buf);

    lazy_doc_close(lazy_doc);
}

TestSuite *ts_fields(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_fi_new, NULL);
    tst_run_test(suite, test_fis_basic, NULL);
    tst_run_test(suite, test_fis_with_default, NULL);
    tst_run_test(suite, test_fis_rw, NULL);
    tst_run_test(suite, test_fields_rw_single, NULL);
    tst_run_test(suite, test_fields_rw_multi, NULL);
    tst_run_test(suite, test_lazy_field_loading, NULL);

    return suite;
}

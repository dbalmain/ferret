#include "test.h"
#include "fields.h"

#define do_field_prop_test(tc, fi, name, boost, is_stored,\
                           is_compressed, is_indexed, is_tokenized, omit_norms,\
                           store_term_vector, store_positions, store_offsets)\
        field_prop_test(tc, __LINE__, fi, name, boost, is_stored,\
                        is_compressed, is_indexed, is_tokenized, omit_norms,\
                        store_term_vector, store_positions, store_offsets)
#define T 1
#define F 0

static void field_prop_test(tst_case *tc,
                            int line_num,
                            FieldInfo *fi,
                            char *name,
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
    tst_str_equal(line_num, tc, name, fi->name);
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

/**
 * Test fields
 */
static void test_fi_create(tst_case *tc, void *data)
{
    FieldInfo *fi;
    (void)data; /* suppress unused argument warning */

    fi = fi_create("name", STORE_NO, INDEX_NO, TERM_VECTOR_NO);
    do_field_prop_test(tc, fi, "name", 1.0, F, F, F, F, F, F, F, F);
    fi_destroy(fi);
    fi = fi_create("name", STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    do_field_prop_test(tc, fi, "name", 1.0, T, F, T, T, F, T, F, F);
    fi_destroy(fi);
    fi = fi_create("name", STORE_COMPRESS, INDEX_UNTOKENIZED,
                   TERM_VECTOR_WITH_POSITIONS);
    do_field_prop_test(tc, fi, "name", 1.0, T, T, T, F, F, T, T, F);
    fi_destroy(fi);
    fi = fi_create("name", STORE_NO, INDEX_YES_OMIT_NORMS,
                   TERM_VECTOR_WITH_OFFSETS);
    do_field_prop_test(tc, fi, "name", 1.0, F, F, T, T, T, T, F, T);
    fi_destroy(fi);
    fi = fi_create("name", STORE_NO, INDEX_UNTOKENIZED_OMIT_NORMS,
                   TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fi->boost = 1000.0;
    do_field_prop_test(tc, fi, "name", 1000.0, F, F, T, F, T, T, T, T);
    fi_destroy(fi);
}

static void test_fis_basic(tst_case *tc, void *data)
{
    FieldInfos *fis;
    (void)data; /* suppress unused argument warning */

    fis = fis_create(STORE_NO, INDEX_NO, TERM_VECTOR_NO);
    fis_add_field(fis, fi_create("FFFFFFFF", STORE_NO, INDEX_NO,
                                 TERM_VECTOR_NO));
    fis_add_field(fis, fi_create("TFTTFTFF", STORE_YES, INDEX_YES,
                                 TERM_VECTOR_YES));
    fis_add_field(fis, fi_create("TTTFFTTF", STORE_COMPRESS, INDEX_UNTOKENIZED,
                                 TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_create("FFTTTTFT", STORE_NO, INDEX_YES_OMIT_NORMS,
                                 TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_create("FFTFTTTT", STORE_NO,
                                 INDEX_UNTOKENIZED_OMIT_NORMS,
                                 TERM_VECTOR_WITH_POSITIONS_OFFSETS));

    Apequal(fis_get_field(fis, "FFFFFFFF"), fis->fields[0]);
    Apequal(fis_get_field(fis, "TFTTFTFF"), fis->fields[1]);
    Apequal(fis_get_field(fis, "TTTFFTTF"), fis->fields[2]);
    Apequal(fis_get_field(fis, "FFTTTTFT"), fis->fields[3]);
    Apequal(fis_get_field(fis, "FFTFTTTT"), fis->fields[4]);

    Aiequal(0, fis_get_field(fis, "FFFFFFFF")->number);
    Aiequal(1, fis_get_field(fis, "TFTTFTFF")->number);
    Aiequal(2, fis_get_field(fis, "TTTFFTTF")->number);
    Aiequal(3, fis_get_field(fis, "FFTTTTFT")->number);
    Aiequal(4, fis_get_field(fis, "FFTFTTTT")->number);

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

    fis_destroy(fis);
}

static void test_fis_with_default(tst_case *tc, void *data)
{
    FieldInfos *fis;
    (void)data; /* suppress unused argument warning */

    fis = fis_create(STORE_NO, INDEX_NO, TERM_VECTOR_NO);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "name"), "name", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "dave"), "dave", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "wert"), "wert", 1.0,
                       F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[0], "name", 1.0, F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[1], "dave", 1.0, F, F, F, F, F, F, F, F);
    do_field_prop_test(tc, fis->fields[2], "wert", 1.0, F, F, F, F, F, F, F, F);
    Apnull(fis_get_field(fis, "random"));
    fis_destroy(fis);

    fis = fis_create(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "name"), "name", 1.0,
                       T, F, T, T, F, T, F, F);
    fis_destroy(fis);
    fis = fis_create(STORE_COMPRESS, INDEX_UNTOKENIZED,
                     TERM_VECTOR_WITH_POSITIONS);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "name"), "name", 1.0,
                       T, T, T, F, F, T, T, F);
    fis_destroy(fis);
    fis = fis_create(STORE_NO, INDEX_YES_OMIT_NORMS,
                     TERM_VECTOR_WITH_OFFSETS);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "name"), "name", 1.0,
                       F, F, T, T, T, T, F, T);
    fis_destroy(fis);
    fis = fis_create(STORE_NO, INDEX_UNTOKENIZED_OMIT_NORMS,
                     TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "name"), "name", 1.0,
                       F, F, T, F, T, T, T, T);
    fis_destroy(fis);
}

static void test_fis_rw(tst_case *tc, void *data)
{
    char *str;
    FieldInfos *fis;
    Store *store = open_ram_store();
    OutStream *os;
    InStream *is;
    (void)data; /* suppress unused argument warning */

    fis = fis_create(STORE_YES, INDEX_UNTOKENIZED_OMIT_NORMS, 
                     TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_create("FFFFFFFF", STORE_NO, INDEX_NO,
                                 TERM_VECTOR_NO));
    fis_add_field(fis, fi_create("TFTTFTFF", STORE_YES, INDEX_YES,
                                 TERM_VECTOR_YES));
    fis_add_field(fis, fi_create("TTTFFTTF", STORE_COMPRESS, INDEX_UNTOKENIZED,
                                 TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_create("FFTTTTFT", STORE_NO, INDEX_YES_OMIT_NORMS,
                                 TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_create("FFTFTTTT", STORE_NO,
                                 INDEX_UNTOKENIZED_OMIT_NORMS,
                                 TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    fis->fields[1]->boost = 2.0;
    fis->fields[2]->boost = 3.0;
    fis->fields[3]->boost = 4.0;
    fis->fields[4]->boost = 5.0;

    os = store->create_output(store, "fields");
    fis_write(fis, os);
    os_close(os);

    /* these fields won't be saved be will added again later */
    Aiequal(5, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "new_field"),
                       "new_field", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(6, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "another"),
                       "another", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(7, fis->size);

    fis_destroy(fis);

    is = store->open_input(store, "fields");
    fis = fis_read(is);
    Aiequal(STORE_YES, fis->store);
    Aiequal(INDEX_UNTOKENIZED_OMIT_NORMS, fis->index);
    Aiequal(TERM_VECTOR_WITH_POSITIONS_OFFSETS, fis->term_vector);
    is_close(is);

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
    do_field_prop_test(tc, fis_get_or_add_field(fis, "new_field"),
                       "new_field", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(6, fis->size);
    do_field_prop_test(tc, fis_get_or_add_field(fis, "another"),
                       "another", 1.0, T, F, T, F, T, T, T, T);
    Aiequal(7, fis->size);
    str=fis_to_s(fis);
    Asequal("default:\n"
            "  store: :yes\n"
            "  index: :yes_omit_norms\n"
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
            "    term_vector: :with_positions\n"
            "  TTTFFTTF:\n"
            "    boost: 3.000000\n"
            "    store: :compressed\n"
            "    index: :untokenized\n"
            "    term_vector: :yes\n"
            "  FFTTTTFT:\n"
            "    boost: 4.000000\n"
            "    store: :no\n"
            "    index: :yes_omit_norms\n"
            "    term_vector: :with_positions_offsets\n"
            "  FFTFTTTT:\n"
            "    boost: 5.000000\n"
            "    store: :no\n"
            "    index: :untokenized_omit_norms\n"
            "    term_vector: :with_offsets\n"
            "  new_field:\n"
            "    boost: 1.000000\n"
            "    store: :yes\n"
            "    index: :untokenized_omit_norms\n"
            "    term_vector: :with_offsets\n"
            "  another:\n"
            "    boost: 1.000000\n"
            "    store: :yes\n"
            "    index: :untokenized_omit_norms\n"
            "    term_vector: :with_offsets\n", str);
    free(str);

    fis_destroy(fis);
    store_deref(store);
}

tst_suite *ts_fields(tst_suite * suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_fi_create, NULL);
    tst_run_test(suite, test_fis_basic, NULL);
    tst_run_test(suite, test_fis_with_default, NULL);
    tst_run_test(suite, test_fis_rw, NULL);

    return suite;
}

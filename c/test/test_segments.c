#include "index.h"
#include "test.h"

#define T 1
#define F 0

/***************************************************************************
 *
 * SegmentInfo
 *
 ***************************************************************************/

#define Asi_equal(si1, si2)\
    tst_si_eql(__LINE__, tc, si2, si1->name, si1->doc_cnt, si1->store)

#define Asi_has_vals(si, name, doc_cnt, store)\
    tst_si_eql(__LINE__, tc, si, name, doc_cnt, store)
static FieldInfos dummy_fis = {
    STORE_NO,       /* store */
    INDEX_NO,       /* index */
    TERM_VECTOR_NO, /* term_vector */
    0,              /* size */
    0,              /* capa */
    NULL,           /* fields */
    NULL,           /* field_dict */
    1000000         /* ref_cnt so that it is never destroyed */
};

static int tst_si_eql(int line_num,
                      TestCase *tc,
                      SegmentInfo *si,
                      const char *name,
                      int doc_cnt,
                      Store *store)
{
    if (tst_str_equal(line_num, tc, name, si->name) &&
        tst_int_equal(line_num, tc, doc_cnt, si->doc_cnt) &&
        tst_ptr_equal(line_num, tc, store, si->store)) {
        return true;
    }
    else {
        return false;
    }

}

static void test_si(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    SegmentInfo *si = si_new(estrdup("_1"), 10, store);
    Asi_has_vals(si, "_1", 10, store);
    store = open_fs_store("./test/testdir/store");
    si->name[1] = '2';
    si->doc_cnt += 2;
    si->store = store;
    Asi_has_vals(si, "_2", 12, store);
    Assert(!si_has_separate_norms(si), "doesn't use compound file/have norms");
    si->use_compound_file = true;
    Assert(!si_has_separate_norms(si), "doesn't have norms");
    si_advance_norm_gen(si, 3);
    si->use_compound_file = false;
    Assert(!si_has_separate_norms(si), "doesn't use compound file");
    si->use_compound_file = true;
    Assert(!si_has_separate_norms(si), "has norms in compound file");
    si_advance_norm_gen(si, 3);
    Assert(si_has_separate_norms(si), "has seperate norms");
    si_deref(si);
    store_deref(store);
}

/***************************************************************************
 *
 * SegmentInfos
 *
 ***************************************************************************/


void test_sis_add_del(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    SegmentInfos *sis = sis_new(&dummy_fis);
    SegmentInfo *seg0 = sis_new_segment(sis, 123, store);
    SegmentInfo *seg1 = sis_new_segment(sis,  98, store);
    SegmentInfo *seg2 = sis_new_segment(sis,   3, store);
    SegmentInfo *seg3 = sis_new_segment(sis,  87, store);
    SegmentInfo *seg4 = sis_new_segment(sis,  12, store);

    Asi_has_vals(seg0, "_0", 123, store);
    Asi_has_vals(seg1, "_1",  98, store);
    Asi_has_vals(seg2, "_2",   3, store);
    Asi_has_vals(seg3, "_3",  87, store);
    Asi_has_vals(seg4, "_4",  12, store);

    Apequal(seg0, sis->segs[0]);
    Apequal(seg1, sis->segs[1]);
    Apequal(seg2, sis->segs[2]);
    Apequal(seg3, sis->segs[3]);
    Apequal(seg4, sis->segs[4]);

    sis_del_from_to(sis, 1, 4);
    Apequal(seg0, sis->segs[0]);
    Apequal(seg4, sis->segs[1]);
    sis_del_at(sis, 0);

    Apequal(seg4, sis->segs[0]);
    sis_destroy(sis);
}

void test_sis_rw(TestCase *tc, void *data)
{
    u64 version;
    Store *store = (Store *)data;
    SegmentInfos *sis = sis_new(&dummy_fis);
    SegmentInfo *seg0 = sis_new_segment(sis, 51, store);
    SegmentInfo *seg1 = sis_new_segment(sis, 213, store);
    SegmentInfo *seg2 = sis_new_segment(sis, 23, store);
    SegmentInfo *seg3 = si_new(estrdup("_3"), 9, store);
    SegmentInfos *sis2, *sis3;

    TRY
        sis_read_current_version(store);
        Afail("Should have failed trying to read index");
    XCATCHALL
        HANDLED();
    XENDTRY

    Aiequal(3, sis->size);
    Apequal(seg0, sis->segs[0]);
    Apequal(seg2, sis->segs[2]);
    sis_write(sis, store, NULL);
    version = sis_read_current_version(store);
    Assert(store->exists(store, "segments_0"),
           "segments file should have been created");
    sis2 = sis_read(store);
    Aiequal(3, sis2->size);
    Asi_equal(seg0, sis2->segs[0]);
    Asi_equal(seg1, sis2->segs[1]);
    Asi_equal(seg2, sis2->segs[2]);

    sis_add_si(sis2, seg3);
    Aiequal(4, sis2->size);
    sis_write(sis2, store, NULL);
    Aiequal(version + 1, sis_read_current_version(store));
    sis3 = sis_read(store);
    Aiequal(version + 1, sis3->version);
    Aiequal(4, sis3->size);
    Asi_equal(seg0, sis3->segs[0]);
    Asi_equal(seg3, sis3->segs[3]);

    sis_destroy(sis);
    sis_destroy(sis2);
    sis_destroy(sis3);
}

TestSuite *ts_segments(TestSuite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_si, store);
    tst_run_test(suite, test_sis_add_del, store);
    tst_run_test(suite, test_sis_rw, store);

    store_deref(store);
    return suite;
}

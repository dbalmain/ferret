#include "index.h"
#include "test.h"

#define NUM_TERMS 100
#define TERM_LEN 10


static void test_posting(TestCase *tc, void *data)
{
    MemoryPool *mp = (MemoryPool *)data;
    PostingList *pl;
    Posting *p = p_new(mp, 0, 10);
    Aiequal(0, p->doc_num);
    Aiequal(1, p->freq);
    Aiequal(10, p->first_occ->pos);
    Apnull(p->first_occ->next);

    pl = pl_new(mp, "seven", 5, p);
    Aiequal(5, pl->term_len);
    Asequal("seven", pl->term);
    Apequal(p->first_occ, pl->last_occ);

    pl_add_occ(mp, pl, 50);
    Apequal(pl->last_occ, p->first_occ->next);
    Aiequal(2, p->freq);
    Aiequal(50,  pl->last_occ->pos);
    Apnull(pl->last_occ->next);

    pl_add_occ(mp, pl, 345);
    Apequal(pl->last_occ, p->first_occ->next->next);
    Aiequal(3, p->freq);
    Aiequal(345, pl->last_occ->pos);
    Apnull(pl->last_occ->next);
}

static FieldInfos *create_tv_fis()
{
    FieldInfos *fis = fis_new(FRT_STORE_NO, FRT_INDEX_UNTOKENIZED,
                              TERM_VECTOR_NO);
    fis_add_field(fis, fi_new(I("tv"), FRT_STORE_NO, FRT_INDEX_UNTOKENIZED,
                              TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("tv2"), FRT_STORE_NO, FRT_INDEX_UNTOKENIZED,
                              TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("tv_with_positions"), FRT_STORE_NO,
                              FRT_INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_new(I("tv_with_offsets"), FRT_STORE_NO,
                              FRT_INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new(I("tv_with_positions_offsets"), FRT_STORE_NO,
                              FRT_INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    return fis;

}

static char **create_tv_terms(MemoryPool *mp)
{
    int i;
    char term_buf[10];
    char **terms = MP_ALLOC_N(mp, char *, NUM_TERMS);
    for (i = 0; i < NUM_TERMS; i++) {
        sprintf(term_buf, "%09d", i);
        terms[i] = mp_strdup(mp, term_buf);
    }
    return terms;
}

static PostingList **create_tv_plists(MemoryPool *mp, char **terms)
{
    int i, j;
    PostingList **plists, *pl;
    plists = MP_ALLOC_N(mp, PostingList *, NUM_TERMS);
    for (i = 0; i < NUM_TERMS; i++) {
        pl = plists[i] =
            pl_new(mp, terms[i], 9, p_new(mp, 0, 0));
        for (j = 1; j <= i; j++) {
            pl_add_occ(mp, pl, j);
        }
    }
    return plists;
}

static Offset *create_tv_offsets(MemoryPool *mp)
{
    int i;
    Offset *offsets = MP_ALLOC_N(mp, Offset, NUM_TERMS);
    for (i = 0; i < NUM_TERMS; i++) {
        offsets[i].start = 5 * i;
        offsets[i].end = 5 * i + 4;
    }
    return offsets;
}

static void test_tv_single_doc(TestCase *tc, void *data)
{
    int i, j;
    Store *store = open_ram_store();
    MemoryPool *mp = (MemoryPool *)data;
    FieldsReader *fr; 
    FieldsWriter *fw;
    TermVector *tv;
    Hash *tvs;
    FieldInfos *fis = create_tv_fis();
    char **terms = create_tv_terms(mp);
    PostingList **plists = create_tv_plists(mp, terms);
    Offset *offsets = create_tv_offsets(mp);
    Document *doc = doc_new();

    fw = fw_open(store, "_0", fis);
    fw_close(fw);

    fr = fr_open(store, "_0", fis);
    Aiequal(0, fr->size);
    fr_close(fr);


    fw = fw_open(store, "_0", fis);
    fw_add_doc(fw, doc);
    fw_add_postings(fw, fis_get_field(fis, I("tv"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_add_postings(fw, fis_get_field(fis, I("tv_with_positions"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_add_postings(fw, fis_get_field(fis, I("tv_with_offsets"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_add_postings(fw,
                    fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_write_tv_index(fw);
    fw_close(fw);
    doc_destroy(doc);

    fr = fr_open(store, "_0", fis);
    Aiequal(1, fr->size);

    /* test individual field's term vectors */
    tv = fr_get_field_tv(fr, 0, fis_get_field(fis, I("tv"))->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv"))->number, tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(0, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            Apnull(tv->terms[i].positions);
        }
        Apnull(tv->offsets);
    }
    if (tv) tv_destroy(tv);
    
    tv = fr_get_field_tv(fr, 0,
                         fis_get_field(fis, I("tv_with_positions"))->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv_with_positions"))->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(0, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 0; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
            }
        }
        Apnull(tv->offsets);
    }
    if (tv) tv_destroy(tv);
    
    tv = fr_get_field_tv(fr, 0,
                         fis_get_field(fis, I("tv_with_offsets"))->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv_with_offsets"))->number, tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(NUM_TERMS, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            Apnull(tv->terms[i].positions);
        }
        for (i = 0; i < NUM_TERMS; i++) {
            Aiequal(i * 5, tv->offsets[i].start);
            Aiequal(i * 5 + 4, tv->offsets[i].end);
        }
    }
    if (tv) tv_destroy(tv);
    
    tv = fr_get_field_tv(fr, 0,
                         fis_get_field(fis, I("tv_with_positions_offsets")
                                       )->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(NUM_TERMS, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
            }
        }
        for (i = 0; i < NUM_TERMS; i++) {
            Aiequal(i * 5, tv->offsets[i].start);
            Aiequal(i * 5 + 4, tv->offsets[i].end);
        }
    }
    if (tv) tv_destroy(tv);

    tv = fr_get_field_tv(fr, 0, fis_get_or_add_field(fis, I("tv2"))->number);
    Apnull(tv);
    tv = fr_get_field_tv(fr, 0, fis_get_or_add_field(fis, I("new"))->number);
    Apnull(tv);
    
    /* test document's term vectors */
    tvs = fr_get_tv(fr, 0);
    Aiequal(4, tvs->size);
    tv = (TermVector*)h_get(tvs, I("tv2"));
    Apnull(tv);
    tv = (TermVector*)h_get(tvs, I("other"));
    Apnull(tv);

    tv = (TermVector*)h_get(tvs, I("tv_with_positions_offsets"));
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(NUM_TERMS, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
            }
        }
        for (i = 0; i < NUM_TERMS; i++) {
            Aiequal(i * 5, tv->offsets[i].start);
            Aiequal(i * 5 + 4, tv->offsets[i].end);
        }
    }
    h_destroy(tvs);

    fr_close(fr);
    fis_deref(fis);
    store_deref(store);
}

static void test_tv_multi_doc(TestCase *tc, void *data)
{
    int i, j;
    Store *store = open_ram_store();
    MemoryPool *mp = (MemoryPool *)data;
    FieldsReader *fr; 
    FieldsWriter *fw;
    TermVector *tv;
    Hash *tvs;
    FieldInfos *fis = create_tv_fis();
    char **terms = create_tv_terms(mp);
    PostingList **plists = create_tv_plists(mp, terms);
    Offset *offsets = create_tv_offsets(mp);
    Document *doc = doc_new();

    fw = fw_open(store, "_0", fis);
    fw_add_doc(fw, doc);
    fw_add_postings(fw, fis_get_field(fis, I("tv"))->number,
                     plists, NUM_TERMS, offsets, NUM_TERMS);

    fw_write_tv_index(fw); fw_add_doc(fw, doc);

    fw_add_postings(fw, fis_get_field(fis, I("tv_with_positions"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);

    fw_write_tv_index(fw); fw_add_doc(fw, doc);

    fw_add_postings(fw, fis_get_field(fis, I("tv_with_offsets"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);

    fw_write_tv_index(fw); fw_add_doc(fw, doc);

    fw_add_postings(fw,
                    fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);

    fw_write_tv_index(fw); fw_add_doc(fw, doc);

    fw_add_postings(fw, fis_get_field(fis, I("tv"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_add_postings(fw, fis_get_field(fis, I("tv_with_positions"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_add_postings(fw, fis_get_field(fis, I("tv_with_offsets"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);
    fw_add_postings(fw,
                    fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                    plists, NUM_TERMS, offsets, NUM_TERMS);

    fw_write_tv_index(fw);
    fw_close(fw);
    doc_destroy(doc);

    fr = fr_open(store, "_0", fis);
    Aiequal(5, fr->size);

    tv = fr_get_field_tv(fr, 0, fis_get_field(fis, I("tv"))->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv"))->number, tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(0, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            Apnull(tv->terms[i].positions);
        }
        Apnull(tv->offsets);
    }
    Apnull(fr_get_field_tv(fr, 0,
                           fis_get_field(fis, I("tv_with_positions"))->number));
    Apnull(fr_get_field_tv(fr, 0,
                           fis_get_field(fis, I("tv_with_offsets"))->number));
    Apnull(fr_get_field_tv(fr, 0,
                           fis_get_field(fis, I("tv_with_positions_offsets")
                                         )->number));
    tv_destroy(tv);

    tv = fr_get_field_tv(fr, 3,
                         fis_get_field(fis, I("tv_with_positions_offsets")
                                       )->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(NUM_TERMS, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
            }
        }
        for (i = 0; i < NUM_TERMS; i++) {
            Aiequal(i * 5, tv->offsets[i].start);
            Aiequal(i * 5 + 4, tv->offsets[i].end);
        }
    }
    tv_destroy(tv);

    /* test document's term vector */
    tvs = fr_get_tv(fr, 0);
    Aiequal(1, tvs->size);
    h_destroy(tvs);

    tvs = fr_get_tv(fr, 4);
    Aiequal(4, tvs->size);
    tv = (TermVector*)h_get(tvs, I("tv2"));
    Apnull(tv);
    tv = (TermVector*)h_get(tvs, I("other"));
    Apnull(tv);

    tv = (TermVector*)h_get(tvs, I("tv_with_positions_offsets"));
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, I("tv_with_positions_offsets"))->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->term_cnt);
        Aiequal(NUM_TERMS, tv->offset_cnt);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
            }
        }
        for (i = 0; i < NUM_TERMS; i++) {
            Aiequal(i * 5, tv->offsets[i].start);
            Aiequal(i * 5 + 4, tv->offsets[i].end);
        }
    }
    for (i = 0; i < NUM_TERMS; i++) {
        char buf[100];
        int len = sprintf(buf, "%s", tv->terms[i].text);
        assert(strlen(tv->terms[i].text) < 100);

        Aiequal(i, tv_get_term_index(tv, buf));

        /* make the word lexically less than it was but greater than any other
         * word in the index that originally came before it. */
        buf[len - 1]--;
        buf[len    ] = '~';
        buf[len + 1] = '\0';
        Aiequal(-1, tv_get_term_index(tv, buf));
        Aiequal(i, tv_scan_to_term_index(tv, buf));

        /* make the word lexically more than it was by less than any other
         * word in the index that originally came after it. */
        buf[len - 1]++;
        buf[len    ] = '.';
        Aiequal(-1, tv_get_term_index(tv, buf));
        Aiequal(i + 1, tv_scan_to_term_index(tv, buf));
    }
    Aiequal(-1, tv_get_term_index(tv, "UnKnOwN TeRm"));
    h_destroy(tvs);

    fr_close(fr);
    fis_deref(fis);
    store_deref(store);
}


TestSuite *ts_term_vectors(TestSuite *suite)
{
    MemoryPool *mp = mp_new();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_posting, mp);
    mp_reset(mp);
    tst_run_test(suite, test_tv_single_doc, mp);
    mp_reset(mp);
    tst_run_test(suite, test_tv_multi_doc, mp);

    mp_destroy(mp);
    return suite;
}

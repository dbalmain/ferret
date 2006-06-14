#include "test.h"
#include "index.h"

#define NUM_TERMS 100
#define TERM_LEN 10


static void test_posting(tst_case *tc, void *data)
{
    MemoryPool *mp = (MemoryPool *)data;
    Posting *p = p_new_with_offsets(mp, "term", 4, 0, 0, 5); 
    Aiequal(1, p->freq);
    Aiequal(4, p->term_len);
    Asequal("term", p->term);
    Apequal(p->first_occ, p->last_occ);
    Aiequal(0, p->first_occ->position);
    Aiequal(0, p->first_occ->offset.start);
    Aiequal(5, p->first_occ->offset.end);
    Apnull(p->first_occ->next);

    p_add_occurence_with_offsets(mp, p, 50, 154, 158);
    Apequal(p->last_occ, p->first_occ->next);
    Aiequal(2, p->freq);
    Aiequal(50,  p->last_occ->position);
    Aiequal(154, p->last_occ->offset.start);
    Aiequal(158, p->last_occ->offset.end);
    Apnull(p->last_occ->next);

    p_add_occurence_with_offsets(mp, p, 345, 912, 916);
    Apequal(p->last_occ, p->first_occ->next->next);
    Aiequal(3, p->freq);
    Aiequal(345, p->last_occ->position);
    Aiequal(912, p->last_occ->offset.start);
    Aiequal(916, p->last_occ->offset.end);
    Apnull(p->last_occ->next);

    mp_reset(mp);
    p = p_new(mp, "term", 4, 10);
    Aiequal(1, p->freq);
    Aiequal(4, p->term_len);
    Asequal("term", p->term);
    Apequal(p->first_occ, p->last_occ);
    Aiequal(10, p->first_occ->position);
    Apnull(p->last_occ->next);

    p_add_occurence(mp, p, 50);
    Apequal(p->last_occ, p->first_occ->next);
    Aiequal(2, p->freq);
    Aiequal(50,  p->last_occ->position);
    Apnull(p->last_occ->next);

    p_add_occurence(mp, p, 345);
    Apequal(p->last_occ, p->first_occ->next->next);
    Aiequal(3, p->freq);
    Aiequal(345, p->last_occ->position);
    Apnull(p->last_occ->next);
}

static FieldInfos *create_tv_fis()
{
    FieldInfos *fis = fis_new(0, 1, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new("tv", 0, 1, TERM_VECTOR_YES));
    fis_add_field(fis, fi_new("tv2", 0, 1, TERM_VECTOR_YES));
    fis_add_field(fis, fi_new("tv_with_positions", 0, 1,
                                 TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_new("tv_with_offsets", 0, 1,
                                 TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new("tv_with_positions_offsets", 0, 1,
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

static Posting **create_tv_postings(MemoryPool *mp, char **terms)
{
    int i, j;
    Posting **postings, *p;
    postings = MP_ALLOC_N(mp, Posting *, NUM_TERMS);
    for (i = 0; i < NUM_TERMS; i++) {
        p = postings[i] =
            p_new_with_offsets(mp, terms[i], 9, 0, 0, 5);
        for (j = 1; j <= i; j++) {
            p_add_occurence_with_offsets(mp, p, j, j * 6, j * 6 + 5);
        }
    }
    return postings;
}

static void test_tv_single_doc(tst_case *tc, void *data)
{
    int i, j;
    Store *store = open_ram_store();
    MemoryPool *mp = (MemoryPool *)data;
    TermVectorsReader *tvr; 
    TermVectorsWriter *tvw;
    TermVector *tv;
    HashTable *tvs;
    FieldInfos *fis = create_tv_fis();
    char **terms = create_tv_terms(mp);
    Posting **postings = create_tv_postings(mp, terms);

    tvw = tvw_open(store, "_0", fis);
    tvw_close(tvw);

    tvr = tvr_open(store, "_0", fis);
    Aiequal(0, tvr->size);
    tvr_close(tvr);


    tvw = tvw_open(store, "_0", fis);
    tvw_open_doc(tvw);
    tvw_add_postings(tvw, fis_get_field(fis, "tv")->number,
                     postings, NUM_TERMS);
    tvw_add_postings(tvw, fis_get_field(fis, "tv_with_positions")->number,
                     postings, NUM_TERMS);
    tvw_add_postings(tvw, fis_get_field(fis, "tv_with_offsets")->number,
                     postings, NUM_TERMS);
    tvw_add_postings(tvw,
                     fis_get_field(fis, "tv_with_positions_offsets")->number,
                     postings, NUM_TERMS);
    tvw_close_doc(tvw);
    tvw_close(tvw);

    tvr = tvr_open(store, "_0", fis);
    Aiequal(1, tvr->size);

    /* test individual field's term vectors */
    tv = tvr_get_field_tv(tvr, 0, fis_get_field(fis, "tv")->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv")->number, tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            Apnull(tv->terms[i].positions);
            Apnull(tv->terms[i].offsets);
        }
    }
    tv_destroy(tv);
    
    tv = tvr_get_field_tv(tvr, 0,
                          fis_get_field(fis, "tv_with_positions")->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv_with_positions")->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 0; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
            }
            Apnull(tv->terms[i].offsets);
        }
    }
    tv_destroy(tv);
    
    tv = tvr_get_field_tv(tvr, 0,
                          fis_get_field(fis, "tv_with_offsets")->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv_with_offsets")->number, tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j * 6, tv->terms[i].offsets[j].start);
                Aiequal(j * 6 + 5, tv->terms[i].offsets[j].end);
            }
            Apnull(tv->terms[i].positions);
        }
    }
    tv_destroy(tv);
    
    tv = tvr_get_field_tv(tvr, 0,
                          fis_get_field(fis, "tv_with_positions_offsets"
                                       )->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv_with_positions_offsets")->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
                Aiequal(j * 6, tv->terms[i].offsets[j].start);
                Aiequal(j * 6 + 5, tv->terms[i].offsets[j].end);
            }
        }
    }
    tv_destroy(tv);

    tv = tvr_get_field_tv(tvr, 0, fis_get_or_add_field(fis, "tv2")->number);
    Apnull(tv);
    tv = tvr_get_field_tv(tvr, 0, fis_get_or_add_field(fis, "new")->number);
    Apnull(tv);
    
    /* test document's term vectors */
    tvs = tvr_get_tv(tvr, 0);
    Aiequal(4, tvs->size);
    tv = h_get_int(tvs, fis_get_or_add_field(fis, "tv2")->number);
    Apnull(tv);
    tv = h_get_int(tvs, fis_get_or_add_field(fis, "other")->number);
    Apnull(tv);

    tv = h_get_int(tvs, fis_get_field(fis, "tv_with_positions_offsets"
                                     )->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv_with_positions_offsets")->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
                Aiequal(j * 6, tv->terms[i].offsets[j].start);
                Aiequal(j * 6 + 5, tv->terms[i].offsets[j].end);
            }
        }
    }
    h_destroy(tvs);

    tvr_close(tvr);
    fis_destroy(fis);
    store_deref(store);
}

static void test_tv_multi_doc(tst_case *tc, void *data)
{
    int i, j;
    Store *store = open_ram_store();
    MemoryPool *mp = (MemoryPool *)data;
    TermVectorsReader *tvr; 
    TermVectorsWriter *tvw;
    TermVector *tv;
    HashTable *tvs;
    FieldInfos *fis = create_tv_fis();
    char **terms = create_tv_terms(mp);
    Posting **postings = create_tv_postings(mp, terms);

    tvw = tvw_open(store, "_0", fis);
    tvw_open_doc(tvw);
    tvw_add_postings(tvw, fis_get_field(fis, "tv")->number,
                     postings, NUM_TERMS);
    tvw_close_doc(tvw);
    tvw_open_doc(tvw);
    tvw_add_postings(tvw, fis_get_field(fis, "tv_with_positions")->number,
                     postings, NUM_TERMS);
    tvw_close_doc(tvw);
    tvw_open_doc(tvw);
    tvw_add_postings(tvw, fis_get_field(fis, "tv_with_offsets")->number,
                     postings, NUM_TERMS);
    tvw_close_doc(tvw);
    tvw_open_doc(tvw);
    tvw_add_postings(tvw,
                     fis_get_field(fis, "tv_with_positions_offsets")->number,
                     postings, NUM_TERMS);
    tvw_close_doc(tvw);
    tvw_open_doc(tvw);
    tvw_add_postings(tvw, fis_get_field(fis, "tv")->number,
                     postings, NUM_TERMS);
    tvw_add_postings(tvw, fis_get_field(fis, "tv_with_positions")->number,
                     postings, NUM_TERMS);
    tvw_add_postings(tvw, fis_get_field(fis, "tv_with_offsets")->number,
                     postings, NUM_TERMS);
    tvw_add_postings(tvw,
                     fis_get_field(fis, "tv_with_positions_offsets")->number,
                     postings, NUM_TERMS);
    tvw_close_doc(tvw);
    tvw_close(tvw);

    tvr = tvr_open(store, "_0", fis);
    Aiequal(5, tvr->size);

    tv = tvr_get_field_tv(tvr, 0, fis_get_field(fis, "tv")->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv")->number, tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            Apnull(tv->terms[i].positions);
            Apnull(tv->terms[i].offsets);
        }
    }
    Apnull(tvr_get_field_tv(tvr, 0,
                            fis_get_field(fis, "tv_with_positions")->number));
    Apnull(tvr_get_field_tv(tvr, 0,
                            fis_get_field(fis, "tv_with_offsets")->number));
    Apnull(tvr_get_field_tv(tvr, 0,
                            fis_get_field(fis, "tv_with_positions_offsets"
                                          )->number));
    tv_destroy(tv);

    tv = tvr_get_field_tv(tvr, 3,
                          fis_get_field(fis, "tv_with_positions_offsets"
                                       )->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv_with_positions_offsets")->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
                Aiequal(j * 6, tv->terms[i].offsets[j].start);
                Aiequal(j * 6 + 5, tv->terms[i].offsets[j].end);
            }
        }
    }
    tv_destroy(tv);

    /* test document's term vector */
    tvs = tvr_get_tv(tvr, 0);
    Aiequal(1, tvs->size);
    h_destroy(tvs);

    tvs = tvr_get_tv(tvr, 4);
    Aiequal(4, tvs->size);
    tv = h_get_int(tvs, fis_get_or_add_field(fis, "tv2")->number);
    Apnull(tv);
    tv = h_get_int(tvs, fis_get_or_add_field(fis, "other")->number);
    Apnull(tv);

    tv = h_get_int(tvs, fis_get_field(fis, "tv_with_positions_offsets"
                                     )->number);
    if (Apnotnull(tv)) {
        Aiequal(fis_get_field(fis, "tv_with_positions_offsets")->number,
                tv->field_num);
        Aiequal(NUM_TERMS, tv->size);
        for (i = 0; i < NUM_TERMS; i++) {
            Asequal(terms[i], tv->terms[i].text);
            Aiequal(i + 1, tv->terms[i].freq);
            for (j = 1; j <= i; j++) {
                Aiequal(j, tv->terms[i].positions[j]);
                Aiequal(j * 6, tv->terms[i].offsets[j].start);
                Aiequal(j * 6 + 5, tv->terms[i].offsets[j].end);
            }
        }
    }
    h_destroy(tvs);

    tvr_close(tvr);
    fis_destroy(fis);
    store_deref(store);
}


tst_suite *ts_term_vectors(tst_suite *suite)
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

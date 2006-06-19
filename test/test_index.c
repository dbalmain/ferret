#include "index.h"
#include "test.h"
#include "testhelper.h"

extern HashTable *dw_invert_field(DocWriter *dw,
                                  FieldInverter *fld_inv,
                                  DocField *df);
extern FieldInverter *dw_get_fld_inv(DocWriter *dw, const char *fld_name);

static FieldInfos *create_all_fis()
{
    FieldInfos *fis = fis_new(0, INDEX_YES, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new("tv", 0, INDEX_YES, TERM_VECTOR_YES));
    fis_add_field(fis, fi_new("tv un-t", 0, INDEX_UNTOKENIZED,
                              TERM_VECTOR_YES));
    fis_add_field(fis, fi_new("tv+offsets", 0, INDEX_YES,
                              TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new("tv+offsets un-t", 0, INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_OFFSETS));
    return fis;

}

extern void dw_reset_postings(HashTable *postings);

static void test_fld_inverter(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    Analyzer *a = whitespace_analyzer_new(true);
    FieldInfos *fis = create_all_fis();
    HashTable *plists;
    HashTable *curr_plists;
    Posting *p;
    PostingList *pl;
    DocWriter *dw;
    IndexWriter *iw;

    index_create(store, fis);
    iw = iw_open(store, a, NULL);
    dw = dw_open(iw, "_0");

    DocField *df = df_new("no tv");
    df_add_data(df, "one two three four five two three four five three "
                "four five four five");
    df_add_data(df, "ichi ni san yon go ni san yon go san yon go yon go go");
    df_add_data(df, "The quick brown fox jumped over five lazy dogs");

    curr_plists = dw_invert_field(dw, dw_get_fld_inv(dw, df->name), df);

    Aiequal(18, curr_plists->size);
    
    plists = ((FieldInverter *)h_get(dw->fields, df->name))->plists;


    pl = h_get(curr_plists, "one");
    if (Apnotnull(pl)) {
        Asequal("one", pl->term);
        Aiequal(3, pl->term_len);

        p = pl->last;
        Aiequal(1, p->freq);
        Apequal(p->first_occ, pl->last_occ);
        Apnull(p->first_occ->next);
        Aiequal(0, p->first_occ->pos);
        Apequal(pl, ((PostingList *)h_get(plists, "one")));
    }

    pl = h_get(curr_plists, "five");
    if (Apnotnull(pl)) {
        Asequal("five", pl->term);
        Aiequal(4, pl->term_len);
        Apnull(pl->last_occ->next);
        p = pl->last;
        Aiequal(5, p->freq);
        Aiequal(4, p->first_occ->pos);
        Aiequal(8, p->first_occ->next->pos);
        Aiequal(11, p->first_occ->next->next->pos);
        Aiequal(13, p->first_occ->next->next->next->pos);
        Aiequal(35, p->first_occ->next->next->next->next->pos);
        Apequal(pl, ((PostingList *)h_get(plists, "five")));
    }

    df_destroy(df);

    df = df_new("no tv");
    df_add_data(df, "seven new words and six old ones");
    df_add_data(df, "ichi ni one two quick dogs");

    dw->doc_num++;
    dw_reset_postings(dw->curr_plists);

    curr_plists = dw_invert_field(dw, dw_get_fld_inv(dw, df->name), df);

    Aiequal(13, curr_plists->size);

    pl = h_get(curr_plists, "one");
    if (Apnotnull(pl)) {
        Asequal("one", pl->term);
        Aiequal(3, pl->term_len);

        p = pl->first;
        Aiequal(1, p->freq);
        Apnull(p->first_occ->next);
        Aiequal(0, p->first_occ->pos);

        p = pl->last;
        Aiequal(1, p->freq);
        Apequal(p->first_occ, pl->last_occ);
        Apnull(p->first_occ->next);
        Aiequal(9, p->first_occ->pos);
        Apequal(pl, ((PostingList *)h_get(plists, "one")));
    }

    df_destroy(df);

    a_deref(a);
    dw_close(dw);
    iw_close(iw);
    fis_destroy(fis);
}

extern int pl_cmp(Posting *p1, Posting *p2);

#define NUM_POSTINGS TEST_WORD_LIST_SIZE
static void test_postings_sorter(tst_case *tc, void *data)
{
    int i;
    PostingList plists[NUM_POSTINGS], *p_ptr[NUM_POSTINGS];
    (void)data;
    for (i = 0; i < NUM_POSTINGS; i++) {
        plists[i].term = test_word_list[i];
        p_ptr[i] = &plists[i];
    }

    qsort(p_ptr, NUM_POSTINGS, sizeof(PostingList *),
          (int (*)(const void *, const void *))&pl_cmp);

    for (i = 1; i < NUM_POSTINGS; i++) {
        Assert(strcmp(p_ptr[i - 1]->term, p_ptr[i]->term) <= 0,
               "\"%s\" > \"%s\"", p_ptr[i - 1]->term, p_ptr[i]->term);
    }
}

#define NUM_TEST_DOCS 50
#define MAX_TEST_WORDS 1000

static void test_segment_term_doc_enum(tst_case *tc, void *data)
{
    int i, j;
    Store *store = (Store *)data;
    Analyzer *a = whitespace_analyzer_new(false);
    char buf[MAX_TEST_WORDS * (TEST_WORD_LIST_MAX_LEN + 1)];
    FieldInfos *fis = create_all_fis();
    FieldInfo *fi;
    DocWriter *dw;
    Document *docs[NUM_TEST_DOCS], *doc;
    DocField *df;
    IndexWriter *iw;
    SegmentFieldIndex *sfi;
    TermInfosReader *tir;
    InStream *frq_in, *prx_in;
    BitVector *bv = NULL;
    TermDocEnum *tde, *tde_reader, *tde_skip_to;

    index_create(store, fis);
    iw = iw_open(store, a, NULL);
    dw = dw_open(iw, "_0");

    for (i = 0; i < NUM_TEST_DOCS; i++) {
        docs[i] = doc_new();
        for (j = 0; j < fis->size; j++) {
            if ((rand() % 2) == 0) {
                DocField *df = df_new(fis->fields[j]->name);
                df_add_data(df, estrdup(make_random_string(buf, MAX_TEST_WORDS)));
                df->destroy_data = true;
                doc_add_field(docs[i], df);
            }
        }
        dw_add_doc(dw, docs[i]);
    }
    Aiequal(NUM_TEST_DOCS, dw->doc_num);

    a_deref(a);
    dw_close(dw);
    iw_close(iw);

    sfi = sfi_open(store, "_0");
    tir = tir_open(store, sfi, "_0");
    frq_in = store->open_input(store, "_0.frq");
    prx_in = store->open_input(store, "_0.prx");
    tde = stde_new(tir, frq_in, bv);
    tde_reader = stde_new(tir, frq_in, bv);
    tde_skip_to = stde_new(tir, frq_in, bv);

    fi = fis_get_field(fis, "tv");
    for (i = 0; i < 300; i++) {
        int cnt = 0, ind = 0, doc_nums[3], freqs[3];
        const char *word = test_word_list[rand()%TEST_WORD_LIST_SIZE];
        tde->seek(tde, fi->number, word);
        tde_reader->seek(tde_reader, fi->number, word);
        while (tde->next(tde)) {
            if (cnt == ind) {
                cnt = tde_reader->read(tde_reader, doc_nums, freqs, 3);
                ind = 0;
            }
            Aiequal(doc_nums[ind], tde->doc_num(tde));
            Aiequal(freqs[ind], tde->freq(tde));
            ind++;

            doc = docs[tde->doc_num(tde)];
            df = doc_get_field(doc, fi->name);
            if (Apnotnull(df)) {
                Assert(strstr((char *)df->data[0], word) != NULL,
                       "%s not found in doc[%d]\n\"\"\"\n%s\n\"\"\"\n",
                       word, tde->doc_num(tde), df->data[0]);
            }
            tde_skip_to->seek(tde_skip_to, fi->number, word);
            Atrue(tde_skip_to->skip_to(tde_skip_to, tde->doc_num(tde)));
            Aiequal(tde->doc_num(tde), tde_skip_to->doc_num(tde_skip_to));
            Aiequal(tde->freq(tde), tde_skip_to->freq(tde_skip_to));
        }
        Aiequal(ind, cnt);
    }
    tde->close(tde);
    tde_reader->close(tde_reader);
    tde_skip_to->close(tde_skip_to);


    tde = stpe_new(tir, frq_in, prx_in, bv);
    tde_skip_to = stpe_new(tir, frq_in, prx_in, bv);

    fi = fis_get_field(fis, "tv+offsets");
    for (i = 0; i < 200; i++) {
        const char *word = test_word_list[rand()%TEST_WORD_LIST_SIZE];
        tde->seek(tde, fi->number, word);
        while (tde->next(tde)) {
            tde_skip_to->seek(tde_skip_to, fi->number, word);
            Atrue(tde_skip_to->skip_to(tde_skip_to, tde->doc_num(tde)));
            Aiequal(tde->doc_num(tde), tde_skip_to->doc_num(tde_skip_to));
            Aiequal(tde->freq(tde), tde_skip_to->freq(tde_skip_to));

            doc = docs[tde->doc_num(tde)];
            df = doc_get_field(doc, fi->name);
            if (Apnotnull(df)) {
                Assert(strstr((char *)df->data[0], word) != NULL,
                       "%s not found in doc[%d]\n\"\"\"\n%s\n\"\"\"\n",
                       word, tde->doc_num(tde), df->data[0]);
                for (j = tde->freq(tde); j > 0; j--) {
                    int pos = tde->next_position(tde), t;
                    Aiequal(pos, tde_skip_to->next_position(tde_skip_to));
                    Asequal(word, get_nth_word(df->data[0], buf, pos, &t, &t));
                }
            }
        }

    }
    tde->close(tde);
    tde_skip_to->close(tde_skip_to);

    for (i = 0; i < NUM_TEST_DOCS; i++) {
        doc_destroy(docs[i]);
    }
    fis_destroy(fis);
    is_close(frq_in);
    is_close(prx_in);
    tir_close(tir);
    sfi_close(sfi);
}

tst_suite *ts_index(tst_suite * suite)
{
    Store *store = open_ram_store();
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_fld_inverter, store);
    tst_run_test(suite, test_postings_sorter, NULL);
    tst_run_test(suite, test_segment_term_doc_enum, store);

    store_deref(store);
    return suite;
}

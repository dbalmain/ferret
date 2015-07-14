#include "index.h"
#include "test.h"

/***************************************************************************
 *
 * TermInfosWriter
 * TermInfosReader
 *
 ***************************************************************************/

#define DICT_LEN 282
static const char *DICT[DICT_LEN] = {
    "duad", "dual", "dualism", "dualist", "duality", "dualize", "duan",
    "duarchy", "dub", "dubber", "dubbin", "dubbing", "dubiety", "dubiosity",
    "dubious", "dubiously", "dubiousness", "dubitate", "dubitation",
    "dubnium", "dubonnet", "ducal", "ducat", "ducatoon", "duce", "duchess",
    "duchesse", "duchy", "duck", "duckbill", "duckboard", "ducker", "duckie",
    "ducking", "duckling", "duckpin", "duckshove", "duckshover", "ducktail",
    "duckwalk", "duckweed", "ducky", "duct", "ductile", "ductileness",
    "ductility", "ducting", "ductless", "ductule", "ductulus", "ductwork",
    "dud", "dudder", "duddery", "duddie", "duddy", "dude", "dudeen",
    "dudgeon", "due", "duecento", "duel", "dueler", "dueling", "duelist",
    "dueller", "duelling", "duellist", "duello", "duende", "dueness",
    "duenna", "duennaship", "duet", "duette", "duettino", "duettist",
    "duetto", "duff", "duffel", "duffer", "duffle", "dufus", "dug", "dugong",
    "dugout", "duiker", "duit", "duke", "dukedom", "dukeling", "dukery",
    "dukeship", "dulcamara", "dulcet", "dulcian", "dulciana", "dulcification",
    "dulcify", "dulcimer", "dulcimore", "dulcinea", "dulcitone", "dulcorate",
    "dule", "dulfer", "dulia", "dull", "dullard", "dullness", "dullsville",
    "dully", "dulness", "dulocracy", "dulosis", "dulse", "duly", "duma",
    "dumaist", "dumb", "dumbass", "dumbbell", "dumbcane", "dumbfound",
    "dumbfounder", "dumbhead", "dumbledore", "dumbly", "dumbness", "dumbo",
    "dumbstruck", "dumbwaiter", "dumdum", "dumfound", "dummerer", "dummkopf",
    "dummy", "dumortierite", "dump", "dumpbin", "dumpcart", "dumper",
    "dumpiness", "dumping", "dumpling", "dumplings", "dumpsite", "dumpster",
    "dumpy", "dun", "dunam", "dunce", "dunch", "dunder", "dunderhead",
    "dunderheadedness", "dunderpate", "dune", "duneland", "dunfish", "dung",
    "dungaree", "dungeon", "dungeoner", "dungheap", "dunghill", "dungy",
    "dunite", "duniwassal", "dunk", "dunker", "dunlin", "dunnage", "dunnakin",
    "dunness", "dunnite", "dunnock", "dunny", "dunt", "duo", "duodecillion",
    "duodecimal", "duodecimo", "duodenectomy", "duodenum", "duolog",
    "duologue", "duomo", "duopoly", "duopsony", "duotone", "dup",
    "dupability", "dupatta", "dupe", "duper", "dupery", "dupion", "duple",
    "duplet", "duplex", "duplexer", "duplexity", "duplicability", "duplicand",
    "duplicate", "duplication", "duplicator", "duplicature",
    "duplicitousness", "duplicity", "dupondius", "duppy", "dura",
    "durability", "durable", "durableness", "durably", "dural", "duralumin",
    "duramen", "durance", "duration", "durative", "durbar", "dure", "dures",
    "duress", "durgan", "durian", "durion", "durmast", "durn", "durned",
    "duro", "duroc", "durometer", "durr", "durra", "durrie", "durukuli",
    "durum", "durzi", "dusk", "duskiness", "dusky", "dust", "dustbin",
    "dustcart", "dustcloth", "dustcover", "duster", "dustheap", "dustiness",
    "dusting", "dustless", "dustman", "dustmop", "dustoff", "dustpan",
    "dustpanful", "dustrag", "dustsheet", "dustup", "dusty", "dutch",
    "dutchman", "duteous", "duteously", "duteousness", "dutiability",
    "dutiable", "dutifulness", "duty", "duumvir", "duumvirate", "duvet",
    "duvetine", "duvetyn", "duvetyne", "dux", "duyker"
};

/*
static char *rev(char *str)
{
    char *rev = estrdup(str);
    int i, j;
    for (i = 0, j = (int)strlen(str)-1; i < j; i++, j--) {
        rev[i] = str[j];
        rev[j] = str[i];
    }
    return rev;
}
*/

/*****************************************************************************
 *
 * SegmentFieldIndex
 *
 *****************************************************************************/

static void test_segment_field_index_single_field(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    SegmentFieldIndex *sfi;
    SegmentTermIndex *sti;
    TermInfosWriter *tiw = tiw_open(store, "_0", 32, SKIP_INTERVAL);

    tiw_start_field(tiw, 0);

    for (i = 0; i < DICT_LEN; i++) {
        TermInfo term_info = {(i % 20) + 1, i, i, 0};
        tiw_add(tiw, DICT[i], strlen(DICT[i]), &term_info);
    }
    tiw_close(tiw);

    sfi = sfi_open(store, "_0");
    Aiequal(32, sfi->index_interval);
    Aiequal(SKIP_INTERVAL, sfi->skip_interval);
    Aiequal(1, sfi->field_dict->size);

    sti = (SegmentTermIndex *)h_get_int(sfi->field_dict, 0);
    Aiequal(DICT_LEN, sti->size);
    Aiequal(DICT_LEN / 32 + 1, sti->index_cnt);
    Aiequal(0, sti->ptr);
    Aiequal(0, sti->index_ptr);
    sfi_close(sfi);
}

static void add_multi_field_terms(Store *store)
{
    int i;
    int field_num = 0;
    TermInfosWriter *tiw = tiw_open(store, "_0", 8, 8);

    for (i = 0; i < DICT_LEN; i++) {
        TermInfo term_info = {(i % 20) + 1, i, i, i};
        if (i % 40 == 0) {
            tiw_start_field(tiw, field_num);
            field_num += 2;
        }
        tiw_add(tiw, DICT[i], strlen(DICT[i]), &term_info);
    }
    tiw_close(tiw);
}

static void test_segment_field_index_multi_field(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    SegmentFieldIndex *sfi;
    SegmentTermIndex *sti;
    add_multi_field_terms(store);


    sfi = sfi_open(store, "_0");
    Aiequal(8, sfi->index_interval);
    Aiequal(8, sfi->skip_interval);
    Aiequal(DICT_LEN / 40 + 1, sfi->field_dict->size);

    sti = (SegmentTermIndex *)h_get_int(sfi->field_dict, 0);
    Aiequal(40, sti->size);
    Aiequal(5, sti->index_cnt);
    Aiequal(0, sti->ptr);
    Aiequal(0, sti->index_ptr);

    sti = (SegmentTermIndex *)h_get_int(sfi->field_dict, 2);
    Aiequal(40, sti->size);
    Aiequal(5, sti->index_cnt);

    sti = (SegmentTermIndex *)h_get_int(sfi->field_dict, 2 * (DICT_LEN / 40));
    Aiequal(DICT_LEN % 40, sti->size);
    Aiequal((DICT_LEN % 40) / 8 + 1, sti->index_cnt);
    sfi_close(sfi);
}

void test_segment_term_enum(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    SegmentFieldIndex *sfi;
    TermEnum *te;
    TermEnum *te_clone;
    add_multi_field_terms(store);

    sfi = sfi_open(store, "_0");
    te = ste_new(store->open_input(store, "_0.tis"), sfi);
    te->set_field(te, 0);
    for (i = 0; i < 40; i++) {
        int doc_count = i % 20 + 1;
        Asequal(DICT[i], te->next(te));
        Aiequal(doc_count, te->curr_ti.doc_freq);
        Aiequal(i, te->curr_ti.frq_ptr);
        Aiequal(i, te->curr_ti.prx_ptr);
        if (doc_count > 8) {
            Aiequal(i, te->curr_ti.skip_offset);
        }
    }
    /* go back to start */
    te->set_field(te, 0);
    for (i = 0; i < 40; i++) {
        int doc_count = i % 20 + 1;
        Asequal(DICT[i], te->next(te));
        Aiequal(doc_count, te->curr_ti.doc_freq);
        Aiequal(i, te->curr_ti.frq_ptr);
        Aiequal(i, te->curr_ti.prx_ptr);
        if (doc_count > 8) {
            Aiequal(i, te->curr_ti.skip_offset);
        }
    }

    te->set_field(te, 4);
    for (i = 80; i < 120; i++) {
        int doc_count = i % 20 + 1;
        Asequal(DICT[i], te->next(te));
        Aiequal(doc_count, te->curr_ti.doc_freq);
        Aiequal(i, te->curr_ti.frq_ptr);
        Aiequal(i, te->curr_ti.prx_ptr);
        if (doc_count > 8) {
            Aiequal(i, te->curr_ti.skip_offset);
        }
    }

    /* last segment */
    te->set_field(te, (DICT_LEN / 40) * 2);
    for (i = DICT_LEN - (DICT_LEN % 40); i < DICT_LEN; i++) {
        int doc_count = i % 20 + 1;
        Asequal(DICT[i], te->next(te));
        Aiequal(doc_count, te->curr_ti.doc_freq);
        Aiequal(i, te->curr_ti.frq_ptr);
        Aiequal(i, te->curr_ti.prx_ptr);
        if (doc_count > 8) {
            Aiequal(i, te->curr_ti.skip_offset);
        }
    }

    te->set_field(te, 4);
    for (i = 80; i < 120; i++) {
        int doc_count = i % 20 + 1;
        Asequal(DICT[i], te->next(te));
        Aiequal(doc_count, te->curr_ti.doc_freq);
        Aiequal(i, te->curr_ti.frq_ptr);
        Aiequal(i, te->curr_ti.prx_ptr);
        if (doc_count > 8) {
            Aiequal(i, te->curr_ti.skip_offset);
        }
    }

    te_clone = ste_clone(te);
    Asequal("dulcet", te->skip_to(te, "dulcet"));
    Aiequal(15, te->curr_ti.doc_freq);
    Aiequal(94, te->curr_ti.frq_ptr);
    Aiequal(94, te->curr_ti.prx_ptr);
    Aiequal(94, te->curr_ti.skip_offset);
    Asequal("duffer", te->skip_to(te, "duf"));
    Aiequal(1, te->curr_ti.doc_freq);
    Aiequal(80, te->curr_ti.frq_ptr);
    Aiequal(80, te->curr_ti.prx_ptr);
    Aiequal(0, te->curr_ti.skip_offset);
    Asequal("dugong", te->skip_to(te, "dugo"));
    Aiequal(5, te->curr_ti.doc_freq);
    Aiequal(84, te->curr_ti.frq_ptr);
    Aiequal(84, te->curr_ti.prx_ptr);
    Aiequal(0, te->curr_ti.skip_offset);
    Asequal("dumb", te->skip_to(te, "dumb"));
    Aiequal(20, te->curr_ti.doc_freq);
    Aiequal(119, te->curr_ti.frq_ptr);
    Aiequal(119, te->curr_ti.prx_ptr);
    Aiequal(119, te->curr_ti.skip_offset);
    Apnull(te->next(te));

    /* test clone */
    Asequal("dugong", te_clone->skip_to(te_clone, "dugo"));
    Aiequal(5, te_clone->curr_ti.doc_freq);
    Aiequal(84, te_clone->curr_ti.frq_ptr);
    Aiequal(84, te_clone->curr_ti.prx_ptr);
    Aiequal(0, te_clone->curr_ti.skip_offset);
    /* test clone after original is closed */
    te->close(te);
    Asequal("dumb", te_clone->skip_to(te_clone, "dumb"));
    Aiequal(20, te_clone->curr_ti.doc_freq);
    Aiequal(119, te_clone->curr_ti.frq_ptr);
    Aiequal(119, te_clone->curr_ti.prx_ptr);
    Aiequal(119, te_clone->curr_ti.skip_offset);
    Apnull(te_clone->next(te_clone));

    /* test last term */
    te_clone->set_field(te_clone, (DICT_LEN / 40) * 2);
    Asequal(DICT[DICT_LEN-1], te_clone->skip_to(te_clone, DICT[DICT_LEN-1]));
    Aiequal((DICT_LEN - 1) % 20 + 1, te_clone->curr_ti.doc_freq);
    Aiequal(DICT_LEN - 1, te_clone->curr_ti.frq_ptr);
    Aiequal(DICT_LEN - 1, te_clone->curr_ti.prx_ptr);
    Apnull(te_clone->next(te_clone));

    /* test first term */
    te_clone->set_field(te_clone, 0);
    Asequal(DICT[0], te_clone->skip_to(te_clone, DICT[0]));
    Aiequal(1, te_clone->curr_ti.doc_freq);
    Aiequal(0, te_clone->curr_ti.frq_ptr);
    Aiequal(0, te_clone->curr_ti.prx_ptr);

    te_clone->close(te_clone);
    sfi_close(sfi);
}

static void test_term_infos_reader(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    SegmentFieldIndex *sfi;
    TermInfosReader *tir;
    TermInfo *ti;
    add_multi_field_terms(store);

    sfi = sfi_open(store, "_0");
    tir = tir_open(store, sfi, "_0");
    tir_set_field(tir, 0); 
    ti = tir_get_ti(tir, DICT[0]);
    Aiequal(1, ti->doc_freq);
    Aiequal(0, ti->frq_ptr);
    Aiequal(0, ti->prx_ptr);

    /* try term not in this field */
    ti = tir_get_ti(tir, DICT[DICT_LEN - 1]);
    Apnull(ti);

    /* try existing suffix */
    ti = tir_get_ti(tir, "dubie");
    Apnull(ti);
    ti = tir_get_ti(tir, "dubiety");
    Apnotnull(ti);

    /* test tir_get_term */
    Asequal("dual", tir_get_term(tir, 1));
    Asequal("duad", tir_get_term(tir, 0));
    Asequal("duckwalk", tir_get_term(tir, 39));
    Asequal("ducktail", tir_get_term(tir, 38));
    Asequal(NULL, tir_get_term(tir, 40));
    Asequal(NULL, tir_get_term(tir, 400));
    Asequal(NULL, tir_get_term(tir, -100));

    tir_set_field(tir, 2);
    Asequal("ducky", tir_get_term(tir, 1));
    Asequal("duckweed", tir_get_term(tir, 0));
    Asequal("duffel", tir_get_term(tir, 39));
    Asequal("duff", tir_get_term(tir, 38));
    Asequal(NULL, tir_get_term(tir, 40));
    Asequal(NULL, tir_get_term(tir, 400));
    Asequal(NULL, tir_get_term(tir, -100));

    tir_set_field(tir, (DICT_LEN / 40) * 2);
    ti = tir_get_ti(tir, DICT[DICT_LEN - 1]);
    Aiequal((DICT_LEN - 1) % 20 + 1, ti->doc_freq);
    Aiequal(DICT_LEN - 1, ti->frq_ptr);
    Aiequal(DICT_LEN - 1, ti->prx_ptr);

    tir_close(tir);
    sfi_close(sfi);
}

TestSuite *ts_term(TestSuite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_segment_field_index_single_field, store);
    tst_run_test(suite, test_segment_field_index_multi_field, store);
    tst_run_test(suite, test_segment_term_enum, store);
    tst_run_test(suite, test_term_infos_reader, store);

    store_deref(store);

    return suite;
}

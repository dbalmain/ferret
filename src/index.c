#include "index.h"
#include "similarity.h"
#include "helper.h"
#include "array.h"
#include <string.h>
#include <limits.h>

const char *INDEX_EXTENSIONS[] = {
    "cfs", "fnm", "fdx", "fdt", "tii", "tis", "frq", "prx", "del",
    "tvx", "tvd", "tvf", "tvp"
};

const char *COMPOUND_EXTENSIONS[] = {
    "fnm", "frq", "prx", "fdx", "fdt", "tii", "tis"
};

const char *VECTOR_EXTENSIONS[] = {
    "tvx", "tvd", "tvf"
};
      
const Config const default_config = {
    0x100000,       /* chunk size is 1Mb */
    0x1000000,      /* Max memory used for buffer is 16 Mb */
    INDEX_INTERVAL,
    SKIP_INTERVAL,
    10,             /* default merge factor */
    10000,          /* maximum field length (number of terms) */
    true            /* use compound file by default */
};

static void ste_reset(SegmentTermEnum *ste);
static char *ste_next(SegmentTermEnum *ste);

/***************************************************************************
 *
 * CacheObject
 *
 ***************************************************************************/

static f_u32 co_hash(const void *key)
{
  return (f_u32)(long)key;
}

static int co_eq(const void *key1, const void *key2)
{
  return (key1 == key2);
}

void co_destroy(CacheObject *self)
{
  h_rem(self->ref_tab1, self->ref2, false);
  h_rem(self->ref_tab2, self->ref1, false);
  self->destroy(self->obj);
  free(self);
}

CacheObject *co_create(HashTable *ref_tab1, HashTable *ref_tab2,
    void *ref1, void *ref2, free_ft destroy, void *obj)
{
  CacheObject *self = ALLOC(CacheObject);
  h_set(ref_tab1, ref2, self);
  h_set(ref_tab2, ref1, self);
  self->ref_tab1 = ref_tab1;
  self->ref_tab2 = ref_tab2;
  self->ref1 = ref1;
  self->ref2 = ref2;
  self->destroy = destroy;
  self->obj = obj;
  return self;
}

HashTable *co_hash_create()
{
  return h_new(&co_hash, &co_eq, (free_ft)NULL, (free_ft)&co_destroy);
}

/****************************************************************************
 *
 * FieldInfo
 *
 ****************************************************************************/

inline void fi_set_store(FieldInfo *fi, int store)
{
    switch (store) {
        case STORE_NO:
            break;
        case STORE_YES:
            fi->bits |= FI_IS_STORED_BM;
            break;
        case STORE_COMPRESS:
            fi->bits |= FI_IS_COMPRESSED_BM | FI_IS_STORED_BM;
            break;
    }
}

inline void fi_set_index(FieldInfo *fi, int index)
{
    switch (index) {
        case INDEX_NO:
            break;
        case INDEX_YES:
            fi->bits |= FI_IS_INDEXED_BM | FI_IS_TOKENIZED_BM;
            break;
        case INDEX_UNTOKENIZED:
            fi->bits |= FI_IS_INDEXED_BM;
            break;
        case INDEX_YES_OMIT_NORMS:
            fi->bits |= FI_OMIT_NORMS_BM | FI_IS_INDEXED_BM |
                FI_IS_TOKENIZED_BM;
            break;
        case INDEX_UNTOKENIZED_OMIT_NORMS:
            fi->bits |= FI_OMIT_NORMS_BM | FI_IS_INDEXED_BM;
            break;
    }
}

inline void fi_set_term_vector(FieldInfo *fi, int term_vector)
{
    switch (term_vector) {
        case TERM_VECTOR_NO:
            break;
        case TERM_VECTOR_YES:
            fi->bits |= FI_STORE_TERM_VECTOR_BM;
            break;
        case TERM_VECTOR_WITH_POSITIONS:
            fi->bits |= FI_STORE_TERM_VECTOR_BM | FI_STORE_POSITIONS_BM;
            break;
        case TERM_VECTOR_WITH_OFFSETS:
            fi->bits |= FI_STORE_TERM_VECTOR_BM | FI_STORE_OFFSETS_BM;
            break;
        case TERM_VECTOR_WITH_POSITIONS_OFFSETS:
            fi->bits |= FI_STORE_TERM_VECTOR_BM | FI_STORE_POSITIONS_BM |
                FI_STORE_OFFSETS_BM;
            break;
    }
}

static void fi_check_params(int store, int index, int term_vector)
{
    (void)store;
    if ((index == INDEX_NO) && (term_vector != TERM_VECTOR_NO)) {
        RAISE(ARG_ERROR,
              "You can't store the term vectors of an unindexed field");
    }
}

FieldInfo *fi_new(const char *name,
                  int store,
                  int index,
                  int term_vector)
{
    FieldInfo *fi = ALLOC(FieldInfo);
    fi_check_params(store, index, term_vector);
    fi->name = estrdup(name);
    fi->boost = 1.0;
    fi->bits = 0;
    fi_set_store(fi, store);
    fi_set_index(fi, index);
    fi_set_term_vector(fi, term_vector);
    return fi;
}

void fi_destroy(FieldInfo *fi)
{
    free(fi->name);
    free(fi);
}

char *fi_to_s(FieldInfo *fi)
{
    char *str = ALLOC_N(char, strlen(fi->name) + 200);
    char *s = str;
    sprintf(str, "[\"%s\":(%s%s%s%s%s%s%s%s", fi->name,
            fi_is_stored(fi) ? "is_stored, " : "",
            fi_is_compressed(fi) ? "is_compressed, " : "",
            fi_is_indexed(fi) ? "is_indexed, " : "",
            fi_is_tokenized(fi) ? "is_tokenized, " : "",
            fi_omit_norms(fi) ? "omit_norms, " : "",
            fi_store_term_vector(fi) ? "store_term_vector, " : "",
            fi_store_positions(fi) ? "store_positions, " : "",
            fi_store_offsets(fi) ? "store_offsets, " : "");
    s += (int)strlen(str) - 2;
    if (*s != ',') {
        s += 2;
    }
    sprintf(s, ")]");
    return str;
}

/****************************************************************************
 *
 * FieldInfos
 *
 ****************************************************************************/

#define FIELDS_FILENAME "fields"
#define TEMPORARY_FIELDS_FILENAME "fields.new"

FieldInfos *fis_new(int store, int index, int term_vector)
{
    FieldInfos *fis = ALLOC(FieldInfos);
    fi_check_params(store, index, term_vector);
    fis->field_dict = h_new_str((free_ft)NULL, (free_ft)&fi_destroy);
    fis->size = 0;
    fis->capa = FIELD_INFOS_INIT_CAPA;
    fis->fields = ALLOC_N(FieldInfo *, fis->capa);
    fis->store = store;
    fis->index = index;
    fis->term_vector = term_vector;
    return fis;
}

FieldInfo *fis_add_field(FieldInfos *fis, FieldInfo *fi)
{
    if (fis->size == fis->capa) {
        fis->capa <<= 1;
        REALLOC_N(fis->fields, FieldInfo *, fis->capa);
    }
    if (!h_set_safe(fis->field_dict, fi->name, fi)) {
        return NULL;
    }
    fi->number = fis->size;
    fis->fields[fis->size] = fi;
    fis->size++;
    return fi;
}

FieldInfo *fis_get_field(FieldInfos *fis, const char *name)
{
    return h_get(fis->field_dict, name);
}

FieldInfo *fis_get_or_add_field(FieldInfos *fis, const char *name)
{
    FieldInfo *fi = h_get(fis->field_dict, name);
    if (!fi) {
        fi = fi_new(name, fis->store, fis->index, fis->term_vector);
        fis_add_field(fis, fi);
    }
    return fi;
}

FieldInfo *fis_by_number(FieldInfos *fis, int num)
{
    if (num >= 0 && num < fis->size) {
        return fis->fields[num];
    }
    else {
        return NULL;
    }
}

FieldInfos *fis_read(Store *store)
{
    int store_val, index_val, term_vector_val;
    int i;
    union { f_u32 i; float f; } tmp;
    FieldInfo *fi;
    FieldInfos *fis;
    InStream *is = store->open_input(store, FIELDS_FILENAME);

    store_val = is_read_vint(is);
    index_val = is_read_vint(is);
    term_vector_val = is_read_vint(is);
    fis = fis_new(store_val, index_val, term_vector_val);
    for (i = is_read_vint(is); i > 0; i--) {
        fi = ALLOC(FieldInfo);
        fi->name = is_read_string(is);
        tmp.i = is_read_u32(is);
        fi->boost = tmp.f;
        fi->bits = is_read_vint(is);
        fis_add_field(fis, fi);
    }
    is_close(is);

    return fis; 
}

void fis_write(FieldInfos *fis, Store *store)
{
    int i;
    union { f_u32 i; float f; } tmp;
    FieldInfo *fi;
    OutStream *os = store->new_output(store, TEMPORARY_FIELDS_FILENAME);

    os_write_vint(os, fis->store);
    os_write_vint(os, fis->index);
    os_write_vint(os, fis->term_vector);
    os_write_vint(os, fis->size);
    for (i = 0; i < fis->size; i++) {
        fi = fis->fields[i];
        os_write_string(os, fi->name);
        tmp.f = fi->boost;
        os_write_u32(os, tmp.i);
        os_write_vint(os, fi->bits);
    }
    os_close(os);

    store->rename(store, TEMPORARY_FIELDS_FILENAME, FIELDS_FILENAME);
}

static const char *store_str[] = {
    ":no",
    ":yes",
    "",
    ":compressed"
};

static const char *fi_store_str(FieldInfo *fi)
{
    return store_str[fi->bits & 0x3];
}

static const char *index_str[] = {
    ":no",
    ":untokenized",
    "",
    ":yes",
    "",
    ":untokenized_omit_norms",
    "",
    ":yes_omit_norms"
};

static const char *fi_index_str(FieldInfo *fi)
{
    return index_str[(fi->bits >> 2) & 0x7];
}

static const char *term_vector_str[] = {
    ":no",
    ":yes",
    "",
    ":with_positions",
    "",
    ":with_offsets",
    "",
    ":with_positions_offsets"
};

static const char *fi_term_vector_str(FieldInfo *fi)
{
    return term_vector_str[(fi->bits >> 5) & 0x7];
}

char *fis_to_s(FieldInfos *fis)
{
    int i, pos, capa = 200 + fis->size * 120;
    char *buf = ALLOC_N(char, capa);
    FieldInfo *fi;

    sprintf(buf, 
            "default:\n"
            "  store: %s\n"
            "  index: %s\n"
            "  term_vector: %s\n"
            "fields:\n",
            store_str[fis->store], index_str[fis->index],
            term_vector_str[fis->term_vector]);
    pos = (int)strlen(buf);
    for (i = 0; i < fis->size; i++) {
        fi = fis->fields[i];
        sprintf(buf + pos, 
                "  %s:\n"
                "    boost: %f\n"
                "    store: %s\n"
                "    index: %s\n"
                "    term_vector: %s\n", 
                fi->name, fi->boost, fi_store_str(fi),
                fi_index_str(fi), fi_term_vector_str(fi));

        pos += strlen(buf + pos);
    }

    return buf;
}

void fis_destroy(FieldInfos *fis)
{
    h_destroy(fis->field_dict);
    free(fis->fields);
    free(fis);
}

static bool fis_has_vectors(FieldInfos *fis)
{
    int i;
    for (i = 0; i < fis->size; i++) {
        if (fi_store_term_vector(fis->fields[i])) {
            return true;
        }
    }
    return false;
}

/****************************************************************************
 *
 * SegmentInfo
 *
 ****************************************************************************/

#define SEGMENT_NAME_MAX_LENGTH 100

SegmentInfo *si_new(char *name, int doc_cnt, Store *store)
{
    SegmentInfo *si = ALLOC(SegmentInfo);
    si->name = name;
    si->doc_cnt = doc_cnt;
    si->store = store;
    return si;
}

void si_destroy(SegmentInfo *si)
{
    free(si->name);
    free(si);
}

bool si_has_deletions(SegmentInfo *si)
{
    char del_file_name[SEGMENT_NAME_MAX_LENGTH];
    sprintf(del_file_name, "%s.del", si->name);
    return si->store->exists(si->store, del_file_name);
}

bool si_uses_compound_file(SegmentInfo *si)
{
    char compound_file_name[SEGMENT_NAME_MAX_LENGTH];
    sprintf(compound_file_name, "%s.cfs", si->name);
    return si->store->exists(si->store, compound_file_name);
}

struct NormTester {
    bool has_norm_file;
    int norm_file_pattern_len;
    char norm_file_pattern[SEGMENT_NAME_MAX_LENGTH];
};

static void is_norm_file(char *file_name, struct NormTester *nt)
{
    if (strncmp(file_name, nt->norm_file_pattern,
                nt->norm_file_pattern_len) == 0) {
        nt->has_norm_file = true;
    }
}

bool si_has_separate_norms(SegmentInfo *si)
{
    struct NormTester nt;
    sprintf(nt.norm_file_pattern, "%s.s", si->name);
    nt.norm_file_pattern_len = strlen(nt.norm_file_pattern);
    nt.has_norm_file = false;
    si->store->each(si->store, (void (*)(char *file_name, void *arg))&is_norm_file, &nt);

    return nt.has_norm_file;
}


/****************************************************************************
 *
 * SegmentInfos
 *
 ****************************************************************************/

#include <time.h>
#define FORMAT 0
#define SEGMENTS_FILENAME "segments"
#define TEMPORARY_SEGMENTS_FILENAME "segments.new"

static const char base36_digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static char *new_segment(f_u64 counter) 
{
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    int i;

    file_name[SEGMENT_NAME_MAX_LENGTH - 1] = '\0';
    for (i = SEGMENT_NAME_MAX_LENGTH - 2; ; i--) {
        file_name[i] = base36_digitmap[counter%36];
        counter /= 36;
        if (counter == 0) {
            break;
        }
    }
    i--;
    file_name[i] = '_';
    return estrdup(&file_name[i]);
}

SegmentInfos *sis_new()
{
    SegmentInfos *sis = ALLOC(SegmentInfos);
    sis->format = FORMAT;
    sis->version = (f_u64)time(NULL);
    sis->size = 0;
    sis->counter = 0;
    sis->capa = 4;
    sis->segs = ALLOC_N(SegmentInfo *, sis->capa);
    return sis;
}

SegmentInfo *sis_new_segment(SegmentInfos *sis, int doc_cnt, Store *store)
{
    return sis_add_si(sis, si_new(new_segment(sis->counter++), doc_cnt,
                                     store));
}

void sis_destroy(SegmentInfos *sis)
{
    int i;
    for (i = 0; i < sis->size; i++) {
        si_destroy(sis->segs[i]);
    }
    free(sis->segs);
    free(sis);
}

SegmentInfo *sis_add_si(SegmentInfos *sis, SegmentInfo *si)
{
    if (sis->size >= sis->capa) {
        sis->capa = sis->size * 2;
        REALLOC_N(sis->segs, SegmentInfo *, sis->capa);
    }
    sis->segs[sis->size] = si;
    sis->size++;
    return si;
}

void sis_del_at(SegmentInfos *sis, int at)
{
    int i;
    si_destroy(sis->segs[at]);
    sis->size--;
    for (i = at; i < sis->size; i++) {
        sis->segs[i] = sis->segs[i+1];
    }
}

void sis_del_from_to(SegmentInfos *sis, int from, int to)
{
    int i, num_to_del = to - from;
    sis->size -= num_to_del;
    for (i = from; i < to; i++) {
        si_destroy(sis->segs[i]);
    }
    for (i = from; i < sis->size; i++) {
        sis->segs[i] = sis->segs[i+num_to_del];
    }
}

void sis_clear(SegmentInfos *sis)
{
    int i;
    for (i = 0; i < sis->size; i++) {
        si_destroy(sis->segs[i]);
    }
    sis->size = 0;
}

SegmentInfos *sis_read(Store *store)
{
    int doc_cnt;
    int seg_count;
    int i;
    char *name;
    InStream *is = store->open_input(store, SEGMENTS_FILENAME);
    SegmentInfos *sis = ALLOC(SegmentInfos);
    sis->store = store;

    sis->format = is_read_u32(is); /* do nothing. it's the first version */
    sis->version = is_read_u64(is);
    sis->counter = is_read_u64(is);
    seg_count = is_read_vint(is);

    /* allocate space for segments */
    for (sis->capa = 4; sis->capa < seg_count; sis->capa <<= 1) {
    }
    sis->size = 0;
    sis->segs = ALLOC_N(SegmentInfo *, sis->capa);

    for (i = 0; i < seg_count; i++) {
        name = is_read_string(is);
        doc_cnt = is_read_vint(is);
        sis_add_si(sis, si_new(name, doc_cnt, store));
    }
    is_close(is);

    return sis;
}

void sis_write(SegmentInfos *sis, Store *store)
{
    int i;
    SegmentInfo *si;
    OutStream *os = store->new_output(store, TEMPORARY_SEGMENTS_FILENAME);

    os_write_u32(os, FORMAT);
    os_write_u64(os, ++(sis->version)); /* every write changes the index */
    os_write_u64(os, sis->counter);
    os_write_vint(os, sis->size); 
    for (i = 0; i < sis->size; i++) {
        si = sis->segs[i];
        os_write_string(os, si->name);
        os_write_vint(os, si->doc_cnt);
    }
    os_close(os);

    /* install new segment info */
    store->rename(store, TEMPORARY_SEGMENTS_FILENAME, SEGMENTS_FILENAME);
}

f_u64 sis_read_current_version(Store *store)
{
    InStream *is;
    f_u32 format = 0;
    f_u64 version = 0;

    if (!store->exists(store, SEGMENTS_FILENAME)) {
        return 0;
    }
    is = store->open_input(store, SEGMENTS_FILENAME);

    TRY
        format = is_read_u32(is);
        version = is_read_u64(is);
    XFINALLY
        is_close(is);
    XENDTRY

    return version;
}

/****************************************************************************
 *
 * FieldsReader
 *
 ****************************************************************************/

FieldsReader *fr_open(Store *store, const char *segment, FieldInfos *fis)
{
    FieldsReader *fr = ALLOC(FieldsReader);
    InStream *fdx_in;
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    size_t segment_len = strlen(segment);

    memcpy(file_name, segment, segment_len);

    fr->fis = fis;
    strcpy(file_name + segment_len, ".fdt");
    fr->fdt_in = store->open_input(store, file_name);
    strcpy(file_name + segment_len, ".fdx");
    fdx_in = fr->fdx_in = store->open_input(store, file_name);
    fr->size = fdx_in->m->length_i(fdx_in) / 8;

    return fr;
}

void fr_close(FieldsReader *fr)
{
    is_close(fr->fdt_in);
    is_close(fr->fdx_in);
    free(fr);
}

static DocField *fr_df_new(char *name, int size)
{
    DocField *df = ALLOC(DocField);
    df->name = estrdup(name);
    df->capa = df->size = size;
    df->data = ALLOC_N(char *, df->capa);
    df->lengths = ALLOC_N(int, df->capa);
    df->destroy_data = true;
    df->boost = 1.0;
    return df;
}

Document *fr_get_doc(FieldsReader *fr, int doc_num)
{
    int i, j;
    FieldInfo *fi;
    f_u64 pos;
    int stored_cnt, field_num;
    DocField *df;
    Document *doc = doc_new();
    InStream *fdx_in = fr->fdx_in;
    InStream *fdt_in = fr->fdt_in;

    is_seek(fdx_in, doc_num * 8);
    pos = is_read_u64(fdx_in);
    is_seek(fdt_in, (long)pos);
    stored_cnt = is_read_vint(fdt_in);

    for (i = 0; i < stored_cnt; i++) {
        field_num = is_read_vint(fdt_in);
        fi = fr->fis->fields[field_num];
        df = fr_df_new(fi->name, is_read_vint(fdt_in));

        for (j = 0; j < df->size; j++) {
            df->lengths[j] = is_read_vint(fdt_in);
        }

        for (j = 0; j < df->size; j++) {
            df->data[j] = ALLOC_N(char, df->lengths[j] + 1);
            is_read_bytes(fdt_in, (uchar *)df->data[j], df->lengths[j]);
            df->data[j][df->lengths[j]] = '\0';
        }
        doc_add_field(doc, df);
    }

    return doc;
}

/****************************************************************************
 *
 * FieldsWriter
 *
 ****************************************************************************/

FieldsWriter *fw_open(Store *store, const char *segment, FieldInfos *fis)
{
    FieldsWriter *fw = ALLOC(FieldsWriter);
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    size_t segment_len = strlen(segment);

    memcpy(file_name, segment, segment_len);

    strcpy(file_name + segment_len, ".fdt");
    fw->fdt_out = store->new_output(store, file_name);

    strcpy(file_name + segment_len, ".fdx");
    fw->fdx_out = store->new_output(store, file_name);

    fw->fis = fis;

    return fw;
}

void fw_close(FieldsWriter *fw)
{
    os_close(fw->fdt_out);
    os_close(fw->fdx_out);
    free(fw);
}

static inline void save_data(OutStream *fdt_out, char *data, int dlen)
{
    os_write_vint(fdt_out, dlen);
    os_write_bytes(fdt_out, (uchar *)data, dlen);
}

void fw_add_doc(FieldsWriter *fw, Document *doc)
{
    int i, j, stored_cnt = 0;
    DocField *df;
    FieldInfo *fi;
    OutStream *fdt_out = fw->fdt_out, *fdx_out = fw->fdx_out;

    for (i = 0; i < doc->size; i++) {
        df = doc->fields[i];
        if (fi_is_stored(fis_get_or_add_field(fw->fis, df->name))) {
            stored_cnt++;
        }
    }

    os_write_u64(fdx_out, os_pos(fdt_out));
    os_write_vint(fdt_out, stored_cnt);

    for (i = 0; i < doc->size; i++) {
        df = doc->fields[i];
        fi = fis_get_field(fw->fis, df->name);
        if (fi_is_stored(fi)) {
            os_write_vint(fdt_out, fi->number);
            os_write_vint(fdt_out, df->size);
            /**
             * TODO: add compression
             */
            for (j = 0; j < df->size; j++) {
                os_write_vint(fdt_out, df->lengths[j]);
            }
            for (j = 0; j < df->size; j++) {
                os_write_bytes(fdt_out, (uchar *)df->data[j],
                               df->lengths[j]);
            }
        }
    }
}

/****************************************************************************
 *
 * TermEnum
 *
 ****************************************************************************/

#define TE(ste) ((TermEnum *)ste)

char *te_get_term(TermEnum *te)
{
    return memcpy(ALLOC_N(char, te->curr_term_len + 1),
                  te->curr_term, te->curr_term_len + 1);
}

TermInfo *te_get_ti(TermEnum *te)
{
    return memcpy(ALLOC(TermInfo), &(te->curr_ti), sizeof(TermInfo));
}

char *te_skip_to(TermEnum *te, const char *term)
{
    char *curr_term = te->curr_term;
    if (strcmp(curr_term, term) < 0) {
        while (((curr_term = te->next(te)) != NULL) &&
               (strcmp(curr_term, term) < 0)) {
        }
    }
    return curr_term;
}

/****************************************************************************
 *
 * SegmentTermEnum
 *
 ****************************************************************************/

/****************************************************************************
 * SegmentTermIndex
 ****************************************************************************/

static void sti_destroy(SegmentTermIndex *sti)
{
    if (sti->index_terms) {
        int i;
        for (i = 0; i < sti->index_size; i++) {
            free(sti->index_terms[i]);
        }
        free(sti->index_terms);
        free(sti->index_term_lens);
        free(sti->index_term_infos);
        free(sti->index_pointers);
    }
    free(sti);
}

static void sti_ensure_index_is_read(SegmentTermIndex *sti,
                                     SegmentTermEnum *index_ste)
{
    if (sti->index_terms == NULL) {
        int i;
        int index_size = sti->index_size;
        long index_pointer = 0;
        ste_reset(index_ste);
        is_seek(index_ste->is, sti->index_pointer);
        index_ste->size = sti->index_size;
        
        sti->index_terms = ALLOC_N(char *, index_size);
        sti->index_term_lens = ALLOC_N(int, index_size);
        sti->index_term_infos = ALLOC_N(TermInfo, index_size);
        sti->index_pointers = ALLOC_N(long, index_size);
        
        for (i = 0; NULL != ste_next(index_ste); i++) {
#ifdef DEBUG
            if (i >= index_size) {
                RAISE(ERROR, "index term enum read too many terms");
            }
#endif
            sti->index_terms[i] = te_get_term((TermEnum *)index_ste);
            sti->index_term_lens[i] = TE(index_ste)->curr_term_len;
            sti->index_term_infos[i] = TE(index_ste)->curr_ti;
            index_pointer += is_read_vlong(index_ste->is);
            sti->index_pointers[i] = index_pointer;
        }
    }
}

static int sti_get_index_offset(SegmentTermIndex *sti, const char *term)
{
    int lo = 0;
    int hi = sti->index_size - 1;
    int mid, delta;
    char **index_terms = sti->index_terms;

    while (hi >= lo) {
        mid = (lo + hi) >> 1;
        delta = strcmp(term, index_terms[mid]);
        if (delta < 0) {
            hi = mid - 1;
        }
        else if (delta > 0) {
            lo = mid + 1;
        }
        else {
            return mid;
        }
    }
    return hi;
}

/****************************************************************************
 * SegmentFieldIndex
 ****************************************************************************/

#define SFI_ENSURE_INDEX_IS_READ(sfi, sti) do {\
    if (sti->index_terms == NULL) {\
        mutex_lock(&sfi->mutex);\
        sti_ensure_index_is_read(sti, sfi->index_ste);\
        mutex_unlock(&sfi->mutex);\
    }\
} while (0)

SegmentFieldIndex *sfi_open(Store *store, const char *segment)
{
    int field_count;
    SegmentFieldIndex *sfi = ALLOC(SegmentFieldIndex);
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    InStream *is;

    mutex_init(&sfi->mutex, NULL);

    sprintf(file_name, "%s.tfx", segment);
    is = store->open_input(store, file_name);
    field_count = (int)is_read_u32(is);
    sfi->index_interval = is_read_vint(is);
    sfi->skip_interval = is_read_vint(is);

    sfi->field_dict = h_new_int((free_ft)&sti_destroy);

    for (; field_count > 0; field_count--) {
        int field_num = is_read_vint(is);
        SegmentTermIndex *sti = ALLOC_AND_ZERO(SegmentTermIndex);
        sti->index_pointer = is_read_vlong(is);
        sti->pointer = is_read_vlong(is);
        sti->index_size = is_read_vint(is);
        sti->size = is_read_vint(is);
        h_set_int(sfi->field_dict, field_num, sti);
    }
    is_close(is);

    sprintf(file_name, "%s.tix", segment);
    is = store->open_input(store, file_name);
    sfi->index_ste = ste_new(is, NULL);
    return sfi;
}

void sfi_close(SegmentFieldIndex *sfi)
{
    mutex_destroy(&sfi->mutex);
    ste_close(sfi->index_ste);
    h_destroy(sfi->field_dict);
    free(sfi);
}

/****************************************************************************
 * SegmentTermEnum
 ****************************************************************************/

static inline int term_read(char *buf, InStream *is)
{
    int start = (int)is_read_vint(is);
    int length = (int)is_read_vint(is);
    int total_length = start + length;
    is_read_bytes(is, (uchar *)(buf + start), length);
    buf[total_length] = '\0';
    return total_length;
}

static char *ste_next(SegmentTermEnum *ste)
{
    TermEnum *te = (TermEnum *)ste;
    TermInfo *ti;
    InStream *is = ste->is;

    ste->pos++;
    if (ste->pos >= ste->size) {
        te->curr_term[0] = '\0';
        te->curr_term_len = 0;
        return NULL;
    }

    memcpy(&(te->prev_term), &(te->curr_term), te->curr_term_len + 1);
    te->curr_term_len = term_read(te->curr_term, is);

    ti = &(te->curr_ti);
    ti->doc_freq = is_read_vint(is);     /* read doc freq */
    ti->frq_pointer += is_read_vlong(is);/* read freq pointer */
    ti->prx_pointer += is_read_vlong(is);/* read prox pointer */
    if (ti->doc_freq >= ste->skip_interval) {
        ti->skip_offset = is_read_vlong(is);
    }

    return te->curr_term;
}

static void ste_reset(SegmentTermEnum *ste)
{
    ste->pos = -1;
    TE(ste)->curr_term[0] = '\0';
    TE(ste)->curr_term_len = 0;
    ZEROSET(&(TE(ste)->curr_ti), TermInfo);
}

static SegmentTermEnum *ste_set_field(SegmentTermEnum *ste, int field_num)
{
    SegmentTermIndex *sti = h_get_int(ste->sfi->field_dict, field_num);
    ste_reset(ste);
    ste->field_num = field_num;
    if (sti) {
        ste->size = sti->size;
        is_seek(ste->is, sti->pointer);
    }
    return ste;
}

static void ste_index_seek(SegmentTermEnum *ste,
                           SegmentTermIndex *sti,
                           int index_offset)
{
    int term_len = sti->index_term_lens[index_offset];
    is_seek(ste->is, sti->index_pointers[index_offset]);
    ste->pos = ste->sfi->index_interval * index_offset - 1;
    memcpy(TE(ste)->curr_term,
           sti->index_terms[index_offset],
           term_len + 1);
    TE(ste)->curr_term_len = term_len;
    TE(ste)->curr_ti = sti->index_term_infos[index_offset];
}

static char *ste_scan_to(SegmentTermEnum *ste, const char *term)
{
    SegmentFieldIndex *sfi = ste->sfi;
    SegmentTermIndex *sti = h_get_int(sfi->field_dict, ste->field_num);
    if (sti) {
        SFI_ENSURE_INDEX_IS_READ(sfi, sti);
        /* if current term is less than seek term */
        if (ste->pos < ste->size && strcmp(TE(ste)->curr_term, term) <= 0) {
            int enum_offset = (int)(ste->pos / sfi->index_interval) + 1;
            /* if we are at the end of the index or before the next index
             * pointer then a simple scan suffices */
            if (sti->index_size == enum_offset ||
                strcmp(term, sti->index_terms[enum_offset]) < 0) { 
                return te_skip_to((TermEnum *)ste, term);
            }
        }
        ste_index_seek(ste, sti, sti_get_index_offset(sti, term));
        return te_skip_to((TermEnum *)ste, term);
    }
    else {
        return NULL;
    }
}

static SegmentTermEnum *ste_clone(SegmentTermEnum *other_te);

static SegmentTermEnum *ste_allocate()
{
    SegmentTermEnum *ste = ALLOC_AND_ZERO(SegmentTermEnum);

    TE(ste)->next = (char *(*)(TermEnum *te))&ste_next;
    TE(ste)->set_field = (TermEnum *(*)(TermEnum *te, int fn))&ste_set_field;
    TE(ste)->skip_to = (char *(*)(TermEnum *te, const char *term))&ste_scan_to;
    TE(ste)->close = (void (*)(TermEnum *te))&ste_close;
    TE(ste)->clone = (TermEnum *(*)(TermEnum *te))&ste_clone;
    return ste;
}

static SegmentTermEnum *ste_clone(SegmentTermEnum *other_ste)
{
    SegmentTermEnum *ste = ste_allocate();

    memcpy(ste, other_ste, sizeof(SegmentTermEnum));
    ste->is = is_clone(other_ste->is);
    return ste;
}

void ste_close(SegmentTermEnum *ste)
{
    is_close(ste->is);
    free(ste);
}

/*
static TermInfo *ste_scan_for_term_info(SegmentTermEnum *ste, const char *term)
{
    ste_scan_to(ste, term);

    if (strcmp(TE(ste)->curr_term, term) == 0) {
        return te_get_ti((TermEnum *)ste);
    }
    else {
        return NULL;
    }
}
*/

static char *ste_get_term(SegmentTermEnum *ste, int pos)
{
    if (pos >= ste->size) {
        return NULL;
    }
    else if (pos != ste->pos) {
        int idx_int = ste->sfi->index_interval;
        if ((pos < ste->pos) || pos > (1 + ste->pos / idx_int) * idx_int) {
            SegmentTermIndex *sti = h_get_int(ste->sfi->field_dict,
                                              ste->field_num);
            SFI_ENSURE_INDEX_IS_READ(ste->sfi, sti);
            ste_index_seek(ste, sti, pos / idx_int);
        }
        while (ste->pos < pos) {
            if (ste_next(ste) == NULL) {
                return NULL;
            }
        }

    }
    return TE(ste)->curr_term;
}

SegmentTermEnum *ste_new(InStream *is, SegmentFieldIndex *sfi)
{
    SegmentTermEnum *ste = ste_allocate();

    ste->is = is;
    ste->size = 0;
    ste->pos = -1;
    ste->field_num = -1;
    ste->sfi = sfi;
    ste->skip_interval = sfi ? sfi->skip_interval : INT_MAX;

    return ste;
}

/****************************************************************************
 *
 * TermInfosReader
 * (Segment Specific)
 *
 ****************************************************************************/

TermInfosReader *tir_open(Store *store,
                          SegmentFieldIndex *sfi, const char *segment)
{
    TermInfosReader *tir = ALLOC(TermInfosReader);
    char file_name[SEGMENT_NAME_MAX_LENGTH];

    sprintf(file_name, "%s.tis", segment);
    tir->orig_te = ste_new(store->open_input(store, file_name), sfi);
    thread_key_create(&tir->thread_ste, NULL);
    tir->ste_bucket = ary_new();
    tir->field_num = -1;

    return tir;
}

static inline SegmentTermEnum *tir_enum(TermInfosReader *tir)
{
    SegmentTermEnum *ste;
    if ((ste = thread_getspecific(tir->thread_ste)) == NULL) {
        ste = ste_clone(tir->orig_te);
        ary_push(tir->ste_bucket, ste);
        thread_setspecific(tir->thread_ste, ste);
    }
    return ste;
}

TermInfosReader *tir_set_field(TermInfosReader *tir, int field_num)
{
    if (field_num != tir->field_num) {
        SegmentTermEnum *ste = tir_enum(tir);
        ste_set_field(ste, field_num);
        tir->field_num = field_num;
    }
    return tir;
}

TermInfo *tir_get_ti(TermInfosReader *tir, const char *term)
{
    SegmentTermEnum *ste = tir_enum(tir);
    char *match;

    if ((match = ste_scan_to(ste, term)) != NULL && 
        strcmp(match, term) == 0) {
        return &(TE(ste)->curr_ti);
    }
    return NULL;
}

TermInfo *tir_get_ti_field(TermInfosReader *tir, int field_num,
                           const char *term)
{
    SegmentTermEnum *ste = tir_enum(tir);
    char *match;

    if (field_num != tir->field_num) {
        ste_set_field(ste, field_num);
        tir->field_num = field_num;
    }

    if ((match = ste_scan_to(ste, term)) != NULL && 
        strcmp(match, term) == 0) {
        return &(TE(ste)->curr_ti);
    }
    return NULL;
}

char *tir_get_term(TermInfosReader *tir, int pos)
{ 
    if (pos < 0) {
        return NULL;
    } else {
        SegmentTermEnum *ste = tir_enum(tir);
        return ste_get_term(ste, pos);
    }
}

void tir_close(TermInfosReader *tir)
{
    ary_destroy(tir->ste_bucket, (free_ft)&ste_close);
    ste_close(tir->orig_te);
    thread_key_delete(tir->thread_ste);
    free(tir);
}

/****************************************************************************
 *
 * TermInfosWriter
 *
 ****************************************************************************/

static TermWriter *tw_new(Store *store, char *file_name)
{
    TermWriter *tw = ALLOC_AND_ZERO(TermWriter);
    tw->os = store->new_output(store, file_name);
    tw->last_term = EMPTY_STRING;
    return tw;
}

static void tw_close(TermWriter *tw)
{
    os_close(tw->os);
    free(tw);
}

TermInfosWriter *tiw_open(Store *store,
                          const char *segment,
                          int index_interval,
                          int skip_interval)
{
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    TermInfosWriter *tiw = ALLOC(TermInfosWriter);
    size_t segment_len = strlen(segment);

    memcpy(file_name, segment, segment_len);

    tiw->field_count = 0;
    tiw->index_interval = index_interval;
    tiw->skip_interval = skip_interval;
    tiw->last_index_pointer = 0;

    strcpy(file_name + segment_len, ".tix");
    tiw->tix_writer = tw_new(store, file_name);
    strcpy(file_name + segment_len, ".tis");
    tiw->tis_writer = tw_new(store, file_name);
    strcpy(file_name + segment_len, ".tfx");
    tiw->tfx_out = store->new_output(store, file_name);
    os_write_u32(tiw->tfx_out, 0); /* make space for field_count */

    /* The following two numbers are the first numbers written to the field
     * index when tiw_start_field is called. But they'll be zero to start with
     * so we'll write index interval and skip interval instead. */
    tiw->tix_writer->counter = tiw->index_interval;
    tiw->tis_writer->counter = tiw->skip_interval;

    return tiw;
}

static inline void tw_write_term(TermWriter *tw,
                                 OutStream *os,
                                 const char *term,
                                 int term_len)
{
    int start = hlp_string_diff(tw->last_term, term);
    int length = term_len - start;

    os_write_vint(os, start);                   /* write shared prefix length */
    os_write_vint(os, length);                  /* write delta length */
    os_write_bytes(os, (uchar *)(term + start), length);   /* write delta chars */

    tw->last_term = term;
}

static void tw_add(TermWriter *tw,
                   const char *term,
                   int term_len,
                   TermInfo *ti)
{
    OutStream *os = tw->os;

#ifdef DEBUG
    if (strcmp(tw->last_term, term) > 0) {
        RAISE(STATE_ERROR, "\"%s\" > \"%s\"", tw->last_term, term);
    }
    if (ti->frq_pointer < tw->last_term_info.frq_pointer) {
        RAISE(STATE_ERROR, "%ld > %ld", ti->frq_pointer,
              tw->last_term_info.frq_pointer);
    }
    if (ti->prx_pointer < tw->last_term_info.prx_pointer) {
        RAISE(STATE_ERROR, "%ld > %ld", ti->prx_pointer,
              tw->last_term_info.prx_pointer);
    }
#endif

    tw_write_term(tw, os, term, term_len);  /* write term */
    os_write_vint(os, ti->doc_freq);        /* write doc freq */
    os_write_vlong(os, ti->frq_pointer - tw->last_term_info.frq_pointer);
    os_write_vlong(os, ti->prx_pointer - tw->last_term_info.prx_pointer);

    tw->last_term_info = *ti;
    tw->counter++;
}

void tiw_add(TermInfosWriter *tiw,
             const char *term,
             int term_len,
             TermInfo *ti)
{
    long tis_pos;

    /*
    printf("%s:%d:%d:%d:%d\n", term, term_len, ti->doc_freq,
           ti->frq_pointer, ti->prx_pointer);
    */
    if ((tiw->tis_writer->counter % tiw->index_interval) == 0) {
        /* add an index term */
        tw_add(tiw->tix_writer,
               tiw->tis_writer->last_term,
               strlen(tiw->tis_writer->last_term),
               &(tiw->tis_writer->last_term_info));
        tis_pos = os_pos(tiw->tis_writer->os);
        os_write_vlong(tiw->tix_writer->os, tis_pos - tiw->last_index_pointer);
        tiw->last_index_pointer = tis_pos;  /* write pointer */
    }

    tw_add(tiw->tis_writer, term, term_len, ti);

    if (ti->doc_freq >= tiw->skip_interval) {
        os_write_vlong(tiw->tis_writer->os, ti->skip_offset);
    }
}

static inline void tw_reset(TermWriter *tw)
{
    tw->counter = 0;
    tw->last_term = EMPTY_STRING;
    ZEROSET(&(tw->last_term_info), TermInfo);
}

void tiw_start_field(TermInfosWriter *tiw, int field_num)
{
    OutStream *tfx_out = tiw->tfx_out;
    os_write_vint(tfx_out, tiw->tix_writer->counter);    /* write tix size */
    os_write_vint(tfx_out, tiw->tis_writer->counter);    /* write tis size */
    os_write_vint(tfx_out, field_num);
    os_write_vlong(tfx_out, os_pos(tiw->tix_writer->os)); /* write tix pointer */
    os_write_vlong(tfx_out, os_pos(tiw->tis_writer->os)); /* write tis pointer */
    tw_reset(tiw->tix_writer);
    tw_reset(tiw->tis_writer);
    tiw->last_index_pointer = 0;
    tiw->field_count++;
}

void tiw_close(TermInfosWriter *tiw)
{
    OutStream *tfx_out = tiw->tfx_out;
    os_write_vint(tfx_out, tiw->tix_writer->counter);
    os_write_vint(tfx_out, tiw->tis_writer->counter);
    os_seek(tfx_out, 0);
    os_write_u32(tfx_out, tiw->field_count);
    os_close(tfx_out);

    tw_close(tiw->tix_writer);
    tw_close(tiw->tis_writer);

    free(tiw);
}

/****************************************************************************
 *
 * TermVectorsWriter
 *
 ****************************************************************************/

TermVectorsWriter *tvw_open(Store *store, const char *segment, FieldInfos *fis)
{
    TermVectorsWriter *tvw = ALLOC(TermVectorsWriter);
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    tvw->fis = fis;
    tvw->fields = ary_new_type_capa(TVField, TV_FIELD_INIT_CAPA);

    sprintf(file_name, "%s"TVX_EXTENSION, segment);
    tvw->tvx_out = store->new_output(store, file_name);

    sprintf(file_name, "%s"TVD_EXTENSION, segment);
    tvw->tvd_out = store->new_output(store, file_name);

    return tvw;
}

void tvw_close(TermVectorsWriter *tvw)
{
    os_close(tvw->tvx_out);
    os_close(tvw->tvd_out);
    ary_free(tvw->fields);
    free(tvw);
}

void tvw_open_doc(TermVectorsWriter *tvw)
{
    ary_size(tvw->fields) = 0;
    tvw->tvd_pointer = os_pos(tvw->tvd_out);
    os_write_u64(tvw->tvx_out, tvw->tvd_pointer);
}

void tvw_close_doc(TermVectorsWriter *tvw)
{
    int i;
    OutStream *tvd_out = tvw->tvd_out;
    os_write_u32(tvw->tvx_out, (f_u32)(os_pos(tvw->tvd_out) - tvw->tvd_pointer));
    os_write_vint(tvd_out, ary_size(tvw->fields));
    for (i = 0; i < ary_size(tvw->fields); i++) {
        os_write_vint(tvd_out, tvw->fields[i].field_num);
        os_write_vint(tvd_out, tvw->fields[i].size);
    }
}

void tvw_add_postings(TermVectorsWriter *tvw,
                      int field_num,
                      PostingList **plists,
                      int size)
{
    int i, delta_start, delta_length;
    const char *last_term = EMPTY_STRING;
    long tvd_start_pos = os_pos(tvw->tvd_out);
    OutStream *tvd_out = tvw->tvd_out;
    PostingList *plist;
    Posting *posting;
    Occurence *occ;
    FieldInfo *fi = tvw->fis->fields[field_num];
    int store_positions = fi_store_positions(fi);
    int store_offsets = fi_store_offsets(fi);

    ary_grow(tvw->fields);
    ary_last(tvw->fields).field_num = field_num;

    os_write_vint(tvd_out, size);
    for (i = 0; i < size; i++) {
        plist = plists[i];
        posting = plist->last;
        delta_start = hlp_string_diff(last_term, plist->term);
        delta_length = plist->term_len - delta_start;

        os_write_vint(tvd_out, delta_start);  /* write shared prefix length */
        os_write_vint(tvd_out, delta_length); /* write delta length */
        /* write delta chars */
        os_write_bytes(tvd_out,
                       (uchar *)(plist->term + delta_start),
                       delta_length);
        os_write_vint(tvd_out, posting->freq);
        last_term = plist->term;

        if (store_positions) {
            /* use delta encoding for positions */
            int last_pos = 0;
            for (occ = posting->first_occ; occ; occ = occ->next) {
                os_write_vint(tvd_out, occ->pos - last_pos);
                last_pos = occ->pos;
            }
        }

        if (store_offsets) {
            /* use delta encoding for offsets */
            int last_end_offset = 0;
            for (occ = posting->first_occ; occ; occ = occ->next) {
                os_write_vint(tvd_out, occ->offset.start - last_end_offset);
                os_write_vint(tvd_out, occ->offset.end - occ->offset.start);
                last_end_offset = occ->offset.end;
            }
        }
    }
    ary_last(tvw->fields).size = os_pos(tvd_out) - tvd_start_pos;
}

/****************************************************************************
 *
 * TermDocEnum
 *
 ****************************************************************************/

/****************************************************************************
 * SegmentTermDocEnum
 ****************************************************************************/

static void stde_seek_ti(SegmentTermDocEnum *stde, TermInfo *ti)
{
    if (ti == NULL) {
        stde->doc_freq = 0;
    } else {
        stde->count = 0;
        stde->doc_freq = ti->doc_freq;
        stde->doc_num = 0;
        stde->skip_doc = 0;
        stde->skip_count = 0;
        stde->num_skips = stde->doc_freq / stde->skip_interval;
        stde->frq_pointer = ti->frq_pointer;
        stde->prx_pointer = ti->prx_pointer;
        stde->skip_pointer = ti->frq_pointer + ti->skip_offset;
        is_seek(stde->frq_in, ti->frq_pointer);
        stde->have_skipped = false;
    }
}

static void stde_seek(TermDocEnum *tde, int field_num, const char *term)
{
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;
    TermInfo *ti = tir_get_ti_field(stde->tir, field_num, term);
    stde_seek_ti(stde, ti);
}

static int stde_doc_num(TermDocEnum *tde)
{
    return ((SegmentTermDocEnum *)tde)->doc_num;
}

static int stde_freq(TermDocEnum *tde)
{
    return ((SegmentTermDocEnum *)tde)->freq;
}

static bool stde_next(TermDocEnum *tde)
{
    int doc_code;
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;

    while (true) { 
        if (stde->count >= stde->doc_freq) {
            return false;
        }

        doc_code = is_read_vint(stde->frq_in);
        stde->doc_num += doc_code >> 1;    /* shift off low bit */
        if ((doc_code & 1) != 0) {         /* if low bit is set */
            stde->freq = 1;                /* freq is one */
        }
        else {
            stde->freq = (int)is_read_vint(stde->frq_in); /* read freq */
        }

        stde->count++;

        if (stde->deleted_docs == NULL ||
            bv_get(stde->deleted_docs, stde->doc_num) == 0) {
            break; /* We found an undeleted doc so return */
        }

        stde->skip_prox(stde);
    }
    return true;
}

static int stde_read(TermDocEnum *tde, int *docs, int *freqs, int req_num)
{
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;
    int i = 0;
    int doc_code;

    while (i < req_num && stde->count < stde->doc_freq) {
        /* manually inlined call to next() for speed */
        doc_code = is_read_vint(stde->frq_in);
        stde->doc_num += (doc_code >> 1);            /* shift off low bit */
        if ((doc_code & 1) != 0) {                   /* if low bit is set */
            stde->freq = 1;                            /* freq is one */
        } else {
            stde->freq = is_read_vint(stde->frq_in);  /* else read freq */
        }

        stde->count++;

        if (stde->deleted_docs == NULL ||
            bv_get(stde->deleted_docs, stde->doc_num) == 0) {
            docs[i] = stde->doc_num;
            freqs[i] = stde->freq;
            i++;
        }
    }
    return i;
}

static bool stde_skip_to(TermDocEnum *tde, int target_doc_num)
{
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;

    if (stde->doc_freq >= stde->skip_interval) { /* optimized case */
        int last_skip_doc;
        int last_frq_pointer;
        int last_prx_pointer;
        int num_skipped;

        if (stde->skip_in == NULL) {
            stde->skip_in = is_clone(stde->frq_in); /* lazily clone */
        }

        if (!stde->have_skipped) {                 /* lazily seek skip stream */
            is_seek(stde->skip_in, stde->skip_pointer);
            stde->have_skipped = true;
        }

        /* scan skip data */
        last_skip_doc = stde->skip_doc;
        last_frq_pointer = is_pos(stde->frq_in);
        last_prx_pointer = -1;
        num_skipped = -1 - (stde->count % stde->skip_interval);

        while (target_doc_num > stde->skip_doc) {
            last_skip_doc = stde->skip_doc;
            last_frq_pointer = stde->frq_pointer;
            last_prx_pointer = stde->prx_pointer;

            if (stde->skip_doc != 0 && stde->skip_doc >= stde->doc_num) {
                num_skipped += stde->skip_interval;
            }

            if (stde->skip_count >= stde->num_skips) {
                break;
            }

            stde->skip_doc += is_read_vint(stde->skip_in);
            stde->frq_pointer += is_read_vint(stde->skip_in);
            stde->prx_pointer += is_read_vint(stde->skip_in);

            stde->skip_count++;
        }

        /* if we found something to skip, so skip it */
        if (last_frq_pointer > is_pos(stde->frq_in)) {
            is_seek(stde->frq_in, last_frq_pointer);
            stde->seek_prox(stde, last_prx_pointer);

            stde->doc_num = last_skip_doc;
            stde->count += num_skipped;
        }
    }

    /* done skipping, now just scan */
    do { 
        if (!tde->next(tde)) {
            return false;
        }
    } while (target_doc_num > stde->doc_num);
    return true;
}

static void stde_close(TermDocEnum *tde)
{
    is_close(((SegmentTermDocEnum *)tde)->frq_in);

    if (((SegmentTermDocEnum *)tde)->skip_in != NULL) {
        is_close(((SegmentTermDocEnum *)tde)->skip_in);
    }

    free(tde);
}

static void stde_skip_prox(SegmentTermDocEnum *stde)
{ 
    (void)stde;
}

static void stde_seek_prox(SegmentTermDocEnum *stde, int prx_pointer)
{ 
    (void)stde;
    (void)prx_pointer;
}


TermDocEnum *stde_new(TermInfosReader *tir,
                      InStream *frq_in,
                      BitVector *deleted_docs)
{
    SegmentTermDocEnum *stde = ALLOC_AND_ZERO(SegmentTermDocEnum);
    TermDocEnum *tde         = (TermDocEnum *)stde;

    /* TermDocEnum methods */
    tde->seek                = &stde_seek;
    tde->doc_num             = &stde_doc_num;
    tde->freq                = &stde_freq;
    tde->next                = &stde_next;
    tde->read                = &stde_read;
    tde->skip_to             = &stde_skip_to;
    tde->next_position       = NULL;
    tde->close               = &stde_close;

    /* SegmentTermDocEnum methods */
    stde->skip_prox          = &stde_skip_prox;
    stde->seek_prox          = &stde_seek_prox;

    /* Attributes */
    stde->tir                = tir;
    stde->frq_in             = is_clone(frq_in);
    stde->deleted_docs       = deleted_docs;
    stde->skip_interval      = tir->orig_te->sfi->skip_interval;

    return tde;
}

/****************************************************************************
 * SegmentTermPosEnum
 ****************************************************************************/

static void stpe_seek(TermDocEnum *tde, int field_num, const char *term)
{
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;
    TermInfo *ti = tir_get_ti_field(stde->tir, field_num, term);
    stde_seek_ti(stde, ti);
    if (ti != NULL) {
        is_seek(stde->prx_in, ti->prx_pointer);
    }
    stde->prx_cnt = 0;
}

bool stpe_next(TermDocEnum *tde)
{
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;
    is_skip_vints(stde->prx_in, stde->prx_cnt);

    /* if super */
    if (stde_next(tde)) {
        stde->prx_cnt = stde->freq;
        stde->position = 0;
        return true;
    }
    return false;
}

int stpe_read(TermDocEnum *tde, int *docs, int *freqs, int req_num)
{
    (void)tde; (void)docs; (void)freqs; (void)req_num;
    RAISE(ARG_ERROR, "TermPosEnum does not handle processing multiple documents"
                     " in one call. Use TermDocEnum instead.");
    return -1;
}

static int stpe_next_position(TermDocEnum *tde)
{
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;
    stde->prx_cnt--;
    return stde->position += is_read_vint(stde->prx_in);
}

static void stpe_close(TermDocEnum *tde)
{
    is_close(((SegmentTermDocEnum *)tde)->prx_in);
    ((SegmentTermDocEnum *)tde)->prx_in = NULL;
    stde_close(tde);
}

static void stpe_skip_prox(SegmentTermDocEnum *stde)
{
    is_skip_vints(stde->prx_in, stde->freq);
}

static void stpe_seek_prox(SegmentTermDocEnum *stde, int prx_pointer)
{
    is_seek(stde->prx_in, prx_pointer);
    stde->prx_cnt = 0;
}

TermDocEnum *stpe_new(TermInfosReader *tir,
                      InStream *frq_in,
                      InStream *prx_in,
                      BitVector *deleted_docs)
{
    TermDocEnum *tde         = stde_new(tir, frq_in, deleted_docs);
    SegmentTermDocEnum *stde = (SegmentTermDocEnum *)tde;

    /* TermDocEnum methods */
    tde->seek                = &stpe_seek;
    tde->next                = &stpe_next;
    tde->read                = &stpe_read;
    tde->next_position       = &stpe_next_position;
    tde->close               = &stpe_close;

    /* SegmentTermDocEnum methods */
    stde->skip_prox          = &stpe_skip_prox;
    stde->seek_prox          = &stpe_seek_prox;

    /* Attributes */
    stde->prx_in             = is_clone(prx_in);
    stde->prx_cnt            = 0;
    stde->position           = 0;

    return tde;
}

/****************************************************************************
 * MultiTermDocEnum
 ****************************************************************************/

//static void mtde_seek(TermDocEnum *tde, int field_num, char *term)
//{
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde;
//    if (mtde->term != NULL) {
//        free(mtde->term);
//    }
//    mtde->term = estrdup(term);
//    mtde->field_num = field_num;
//    mtde->base = 0;
//    mtde->pointer = 0;
//    mtde->curr_tde = NULL;
//}
//
//static TermDocEnum *mtde_term_docs_from_reader(IndexReader *ir)
//{
//    return ir->term_docs(ir);
//}
//
//static TermDocEnum *mtde_term_docs(MultiTermDocEnum *mtde, int i)
//{
//    if (mtde->term == NULL) {
//        return NULL;
//    } else {
//
//        TermDocEnum *tde = mtde->irs_tde[i];
//        if (tde == NULL) {
//            tde = mtde->irs_tde[i] = mtde->term_docs_from_reader(mtde->irs[i]);
//        }
//
//        tde->seek(tde, mtde->term);
//        return tde;
//    }
//}
//
//static bool mtde_next(TermDocEnum *tde)
//{
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde->data;
//    if (mtde->curr_tde != NULL && mtde->curr_tde->next(mtde->curr_tde)) {
//        return true;
//    } else if (mtde->pointer < mtde->ir_cnt) {
//        mtde->base = mtde->starts[mtde->pointer];
//        mtde->curr_tde = mtde_term_docs(mtde, mtde->pointer);
//        mtde->pointer++;
//        return mtde_next(tde);
//    } else {
//        return false;
//    }
//}
//
//static int mtde_doc_num(TermDocEnum *tde)
//{
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde->data;
//    return mtde->base + mtde->curr_tde->doc_num(mtde->curr_tde);
//}
//
//static int mtde_freq(TermDocEnum *tde)
//{
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde->data;
//    return mtde->curr_tde->freq(mtde->curr_tde);
//}
//
//static bool mtde_skip_to(TermDocEnum *tde, int target_doc_num)
//{
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde->data;
//    while (mtde->pointer < mtde->ir_cnt) {
//        if ((target_doc_num < mtde->starts[mtde->pointer]) &&
//            (mtde->curr_tde->skip_to(mtde->curr_tde, target_doc_num - mtde->base))) {
//            return true;
//        }
//
//        mtde->base = mtde->starts[mtde->pointer];
//        mtde->curr_tde = mtde_term_docs(mtde, mtde->pointer);
//        mtde->pointer++;
//    }
//    if (mtde->curr_tde) {
//        return mtde->curr_tde->skip_to(mtde->curr_tde, target_doc_num - mtde->base);
//    } else {
//        return false;
//    }
//}
//
//static int mtde_read(TermDocEnum *tde, int *docs, int *freqs, int req_num)
//{
//    int i, end = 0, last_end = 0, b;
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde->data;
//    while (true) {
//        while (mtde->curr_tde == NULL) {
//            if (mtde->pointer < mtde->ir_cnt) {      // try next segment
//                mtde->base = mtde->starts[mtde->pointer];
//                mtde->curr_tde = mtde_term_docs(mtde, mtde->pointer++);
//            } else {
//                return end;
//            }
//        }
//        end += mtde->curr_tde->read(mtde->curr_tde,
//                                    &docs[last_end], &freqs[last_end], req_num - last_end);
//        if (end == last_end) {          // none left in segment
//            mtde->curr_tde = NULL;
//        } else {            // got some
//            b = mtde->base;        // adjust doc numbers
//            for (i = last_end; i < end; i++)
//                docs[i] += b;
//            if (end == req_num)
//                return end;
//            else
//                last_end = end;
//        }
//    }
//}
//
//static void mtde_close(TermDocEnum *tde)
//{
//    MultiTermDocEnum *mtde = (MultiTermDocEnum *)tde->data;
//    TermDocEnum *tmp_tde;
//    int i;
//    for (i = 0; i < mtde->ir_cnt; i++) {
//        if ((tmp_tde = mtde->irs_tde[i]) != NULL)
//            tmp_tde->close(tmp_tde);
//    }
//    if (mtde->term != NULL) term_destroy(mtde->term);
//    free(mtde->irs_tde);
//    free(mtde);
//    free(tde);
//}
//
//
//TermDocEnum *mtde_new(TermDocEnum **tdes, int *starts, int cnt)
//{
//    MultiTermDocEnum *mtde = ALLOC_AND_ZERO(MultiTermDocEnum);
//    TermDocEnum *tde       = (TermDocEnum *)mtde;
//    tde->close             = &mtde_close;
//    tde->seek              = &mtde_seek;
//    tde->next              = &mtde_next;
//    tde->doc_num           = &mtde_doc_num;
//    tde->freq              = &mtde_freq;
//    tde->skip_to           = &mtde_skip_to;
//    tde->read              = &mtde_read;
//    tde->next_position     = NULL;
//
//    mtde->starts           = starts;
//    mtde->cnt              = cnt;
//    mtde->tdes             = tds;
//
//    return tde;
//}

/****************************************************************************
 *
 * IndexReader
 *
 ****************************************************************************/


void ir_acquire_not_necessary(IndexReader *ir)
{
    (void)ir;
}

#define I64_PFX POSH_I64_PRINTF_PREFIX
void ir_acquire_write_lock(IndexReader *ir)
{
    if (ir->is_stale) {
        RAISE(STATE_ERROR, "IndexReader out of date and no longer valid for "
                           "delete, undelete, or set_norm operations. To "
                           "perform any of these operations on the index you "
                           "need to close and reopen the index");
    }

    if (ir->write_lock == NULL) {
        ir->write_lock = ir->store->open_lock(ir->store, WRITE_LOCK_NAME);
        if (!ir->write_lock->obtain(ir->write_lock)) {/* obtain write lock */
            RAISE(STATE_ERROR, "Could not obtain write lock when trying to "
                               "write changes to the index. Check that there "
                               "are no stale locks in the index. Look for "
                               "files with the \".lck\" prefix. If you know "
                               "there are no processes writing to the index "
                               "you can safely delete these files.");
        }

        /* we have to check whether index has changed since this reader was opened.
         * if so, this reader is no longer valid for deletion */
        if (sis_read_current_version(ir->store) > ir->sis->version) {
            ir->is_stale = true;
            ir->write_lock->release(ir->write_lock);
            ir->store->close_lock(ir->write_lock);
            ir->write_lock = NULL;
            RAISE(STATE_ERROR, "IndexReader out of date and no longer valid "
                               "for delete, undelete, or set_norm operations. "
                               "The current version is <%"I64_PFX"d>, but this "
                               "readers version is <%"I64_PFX"d>. To perform "
                               "any of these operations on the index you need "
                               "to close and reopen the index",
                               sis_read_current_version(ir->store),
                               ir->sis->version);
        }
    }
}

IndexReader *ir_setup(IndexReader *ir, Store *store, SegmentInfos *sis,
                      FieldInfos *fis, int is_owner)
{
    mutex_init(&ir->mutex, NULL);

    ir->store = store;
    ir->sis = sis;
    ir->fis = fis;

    ir->is_owner = is_owner;
    if (is_owner) {
        ir->acquire_write_lock = &ir_acquire_write_lock;
    }
    else {
        ir->acquire_write_lock = &ir_acquire_not_necessary;
    }

    return ir;
}

bool ir_index_exists(Store *store)
{
    return store->exists(store, "segments");
}

void ir_set_norm(IndexReader *ir, int doc_num, char *field, uchar val)
{
    mutex_lock(&ir->mutex);
    ir->acquire_write_lock(ir);
    ir->set_norm_i(ir, doc_num, fis_get_field(ir->fis, field)->number, val);
    ir->has_changes = true;
    mutex_unlock(&ir->mutex);
}

void ir_undelete_all(IndexReader *ir)
{
    mutex_lock(&ir->mutex);
    ir->acquire_write_lock(ir);
    ir->undelete_all_i(ir);
    ir->has_changes = true;
    mutex_unlock(&ir->mutex);
}

void ir_delete_doc(IndexReader *ir, int doc_num)
{
    mutex_lock(&ir->mutex);
    ir->acquire_write_lock(ir);
    ir->delete_doc_i(ir, doc_num);
    ir->has_changes = true;
    mutex_unlock(&ir->mutex);
}

Document *ir_get_doc_with_term(IndexReader *ir, char *field, char *term)
{
    TermDocEnum *tde = ir_term_docs_for(ir, field, term);
    Document *doc = NULL;

    if (tde) {
        if (tde->next(tde)) {
            doc = ir->get_doc(ir, tde->doc_num(tde));
        }
        tde->close(tde);
    }
    return doc;
}

TermDocEnum *ir_term_docs_for(IndexReader *ir, char *field, char *term)
{
    TermDocEnum *tde = ir->term_docs(ir);
    tde->seek(tde, fis_get_field(ir->fis, field)->number, term);
    return tde;
}

TermDocEnum *ir_term_positions_for(IndexReader *ir, char *field, char *term)
{
    TermDocEnum *tde = ir->term_positions(ir);
    tde->seek(tde, fis_get_field(ir->fis, field)->number, term);
    return tde;
}

void ir_commit_i(IndexReader *ir)
{
    if (ir->has_changes) {
        if (ir->is_owner) {
            Lock *commit_lock;

            mutex_lock(&ir->store->mutex);
            commit_lock = ir->store->open_lock(ir->store, COMMIT_LOCK_NAME);
            if (!commit_lock->obtain(commit_lock)) { /* obtain write lock */
                RAISE(STATE_ERROR, "Error trying to commit the index. Commit "
                                   "lock already obtained");
            }

            ir->commit_i(ir);
            sis_write(ir->sis, ir->store);

            commit_lock->release(commit_lock);
            ir->store->close_lock(commit_lock);
            mutex_unlock(&ir->store->mutex);

            if (ir->write_lock != NULL) {
                /* release write lock */
                ir->write_lock->release(ir->write_lock);
                ir->store->close_lock(ir->write_lock);
                ir->write_lock = NULL;
            }
        } else {
            ir->commit_i(ir);
        }
        ir->has_changes = false;
    }
}

void ir_commit(IndexReader *ir)
{
    mutex_lock(&ir->mutex);
    ir_commit_i(ir);
    mutex_unlock(&ir->mutex);
}

void ir_close(IndexReader *ir)
{
    mutex_lock(&ir->mutex);
    ir_commit_i(ir);
    ir->close_i(ir);
    store_deref(ir->store);
    if (ir->is_owner) {
        sis_destroy(ir->sis);
        fis_destroy(ir->fis);
    }
    if (ir->cache) {
        h_destroy(ir->cache);
    }
    if (ir->sort_cache) {
        h_destroy(ir->sort_cache);
    }

    mutex_destroy(&ir->mutex);
    free(ir);
}

/**
 * Don't call this method if the cache already exists
 **/
void ir_add_cache(IndexReader *ir)
{
    ir->cache = co_hash_create();
}

bool ir_is_latest(IndexReader *ir)
{
    volatile bool is_latest = false;

    Lock *commit_lock = ir->store->open_lock(ir->store, COMMIT_LOCK_NAME);
    if (!commit_lock->obtain(commit_lock)) {
        ir->store->close_lock(commit_lock);
        RAISE(STATE_ERROR, "Error trying to commit the index. Commit "
                           "lock already obtained");
    }
    TRY
        is_latest = (sis_read_current_version(ir->store) == ir->sis->version);
    XFINALLY
        commit_lock->release(commit_lock);
        ir->store->close_lock(commit_lock);
    XENDTRY

    return is_latest;
}

/****************************************************************************
 * Norm
 ****************************************************************************/

typedef struct Norm {
    int field_num;
    InStream *is;
    uchar *bytes;
    bool is_dirty : 1;
} Norm;

static Norm *norm_create(InStream *is, int field_num)
{
    Norm *norm = ALLOC(Norm);

    norm->is = is;
    norm->field_num = field_num;
    norm->bytes = NULL;
    norm->is_dirty = false;

    return norm;
}

static void norm_destroy(Norm *norm)
{
    is_close(norm->is);
    if (norm->bytes != NULL) {
        free(norm->bytes);
    }
    free(norm);
}

static void norm_rewrite(Norm *norm, Store *store, char *segment,
                  int doc_count, Store *cfs_store)
{
    OutStream *os;
    char tmp_file_name[SEGMENT_NAME_MAX_LENGTH];
    char norm_file_name[SEGMENT_NAME_MAX_LENGTH];

    if (norm->bytes == NULL) {
        return; /* These norms do not need to be rewritten */
    }

    sprintf(tmp_file_name, "%s.tmp", segment);
    os = store->new_output(store, tmp_file_name);
    os_write_bytes(os, norm->bytes, doc_count);
    os_close(os);

    if (cfs_store) {
        sprintf(norm_file_name, "%s.s%d", segment, norm->field_num);
    }
    else {
        sprintf(norm_file_name, "%s.f%d", segment, norm->field_num);
    }
    store->rename(store, tmp_file_name, norm_file_name);
    norm->is_dirty = false;
}

/****************************************************************************
 * SegmentReader
 ****************************************************************************/

#define IR(ir) ((IndexReader *)ir)

#define SR(ir) ((SegmentReader *)ir)
#define SR_SIZE(ir) (SR(ir)->fr->size)

static inline TermVectorsReader *sr_tvr(SegmentReader *sr)
{
    TermVectorsReader *tvr;

    if ((tvr = thread_getspecific(sr->thread_tvr)) == NULL) {
        tvr = tvr_clone(sr->orig_tvr);
        ary_push(sr->tvr_bucket, tvr);
        thread_setspecific(sr->thread_tvr, tvr);
    }
    return tvr;
}

static inline bool sr_is_deleted_i(SegmentReader *sr, int doc_num)
{
    return (sr->deleted_docs != NULL && bv_get(sr->deleted_docs, doc_num));
}

static inline void sr_get_norms_into_i(SegmentReader *sr, int field_num,
                                       uchar *buf)
{
    Norm *norm = h_get_int(sr->norms, field_num);
    if (norm == NULL) {
        memset(buf, 0, SR_SIZE(sr));
    }
    else if (norm->bytes != NULL) { /* can copy from cache */
        memcpy(buf, norm->bytes, SR_SIZE(sr));
    }
    else {
        InStream *norm_in = is_clone(norm->is);
        /* read from disk */
        is_seek(norm_in, 0);
        is_read_bytes(norm_in, buf, SR_SIZE(sr));
        is_close(norm_in);
    }
}

static inline uchar *sr_get_norms_i(SegmentReader *sr, int field_num)
{
    Norm *norm = h_get_int(sr->norms, field_num);
    if (norm == NULL) {                           /* not an indexed field */
        return NULL;
    }

    if (norm->bytes == NULL) {                    /* value not yet read */
        uchar *bytes = ALLOC_N(uchar, SR_SIZE(sr));
        sr_get_norms_into_i(sr, field_num, bytes);
        norm->bytes = bytes;                        /* cache it */
    }
    return norm->bytes;
}

static void sr_set_norm_i(IndexReader *ir, int doc_num, int field_num, uchar b)
{
    Norm *norm = h_get_int(SR(ir)->norms, field_num);
    if (norm != NULL) { /* has_norms */
        norm->is_dirty = true; /* mark it dirty */
        SR(ir)->norms_dirty = true;
        sr_get_norms_i(SR(ir), field_num)[doc_num] = b;
    }
}

static void sr_delete_doc_i(IndexReader *ir, int doc_num) 
{
    if (SR(ir)->deleted_docs == NULL) {
        SR(ir)->deleted_docs = bv_new();
    }

    SR(ir)->deleted_docs_dirty = true;
    SR(ir)->undelete_all = false;
    bv_set(SR(ir)->deleted_docs, doc_num);
}

static void sr_undelete_all_i(IndexReader *ir)
{
    SR(ir)->undelete_all = true;
    SR(ir)->deleted_docs_dirty = false;
    if (SR(ir)->deleted_docs != NULL) {
        bv_destroy(SR(ir)->deleted_docs);
    }
    SR(ir)->deleted_docs = NULL;
}

static void bv_write(BitVector *bv, Store *store, char *name)
{
    OutStream *os = store->new_output(store, name);
    os_write_vint(os, bv->size);
    os_write_bytes(os, (uchar *)bv->bits, bv->size);
    os_close(os);
}

static BitVector *bv_read(Store *store, char *name)
{
    BitVector *bv = ALLOC(BitVector);
    InStream *is = store->open_input(store, name);
    bv->size = (int)is_read_vint(is);
    bv->capa = (bv->size >> 5) + 1;
    bv->bits = ALLOC_AND_ZERO_N(f_u32, bv->capa);
    is_read_bytes(is, (uchar *)bv->bits, bv->size);
    is_close(is);
    bv_recount(bv);
    return bv;
}

static void sr_commit_i(IndexReader *ir)
{
    char tmp_file_name[SEGMENT_NAME_MAX_LENGTH];
    char del_file_name[SEGMENT_NAME_MAX_LENGTH];

    sprintf(del_file_name, "%s.del", SR(ir)->segment);

    if (SR(ir)->deleted_docs_dirty) { /* re-write deleted */
        sprintf(tmp_file_name, "%s.tmp", SR(ir)->segment);
        bv_write(SR(ir)->deleted_docs, ir->store, tmp_file_name);
        ir->store->rename(ir->store, tmp_file_name, del_file_name);
    }
    if (SR(ir)->undelete_all && ir->store->exists(ir->store, del_file_name)) {
        ir->store->remove(ir->store, del_file_name);
    }
    if (SR(ir)->norms_dirty) { /* re-write norms */
        int i;
        FieldInfo *fi;
        for (i = 0; i < ir->fis->size; i++) {
            fi = ir->fis->fields[i];
            if (fi_is_indexed(fi)) {
                norm_rewrite(h_get(SR(ir)->norms, fi->name), ir->store,
                             SR(ir)->segment, SR_SIZE(ir), SR(ir)->cfs_store);
            }
        }
    }
    SR(ir)->deleted_docs_dirty = false;
    SR(ir)->norms_dirty = false;
    SR(ir)->undelete_all = false;
}

static void sr_close_i(IndexReader *ir)
{
    SegmentReader *sr = SR(ir);

    fr_close(sr->fr);
    tir_close(sr->tir);
    sfi_close(sr->sfi);

    if (sr->frq_in) {
        is_close(sr->frq_in);
    }
    if (sr->prx_in) {
        is_close(sr->prx_in);
    }

    h_destroy(sr->norms);

    if (sr->orig_tvr) {
        tvr_close(sr->orig_tvr);
        thread_key_delete(sr->thread_tvr);
        ary_destroy(sr->tvr_bucket, (free_ft)&tvr_close);
    }
    if (sr->deleted_docs) {
        bv_destroy(sr->deleted_docs);
    }
    if (sr->cfs_store) {
        store_deref(sr->cfs_store);
    }
    if (sr->fake_norms) {
        free(sr->fake_norms);
    }
}

static int sr_num_docs(IndexReader *ir)
{
    int num_docs;

    mutex_lock(&ir->mutex);
    num_docs = SR(ir)->fr->size;
    if (SR(ir)->deleted_docs != NULL) {
        num_docs -= SR(ir)->deleted_docs->count;
    }
    mutex_unlock(&ir->mutex);
    return num_docs;
}

static int sr_max_doc(IndexReader *ir)
{
    return SR(ir)->fr->size;
}

static Document *sr_get_doc(IndexReader *ir, int doc_num)
{
    Document *doc;
    mutex_lock(&ir->mutex);
    if (sr_is_deleted_i(SR(ir), doc_num)) {
        mutex_unlock(&ir->mutex);
        RAISE(STATE_ERROR, "Document %d has already been deleted", doc_num);
    }
    doc = fr_get_doc(SR(ir)->fr, doc_num);
    mutex_unlock(&ir->mutex);
    return doc;
}

static uchar *sr_get_norms(IndexReader *ir, int field_num)
{
    uchar *norms;
    mutex_lock(&ir->mutex);
    norms = sr_get_norms_i(SR(ir), field_num);
    mutex_unlock(&ir->mutex);
    return norms;
}

static void sr_get_norms_into(IndexReader *ir, int field_num,
                              uchar *buf)
{
    mutex_lock(&ir->mutex);
    sr_get_norms_into_i(SR(ir), field_num, buf);
    mutex_unlock(&ir->mutex);
}

static TermEnum *sr_terms(IndexReader *ir, int field_num)
{
    SegmentTermEnum *ste = SR(ir)->tir->orig_te;
    ste = ste_clone(ste);
    return TE(ste_set_field(ste, field_num));
}

static TermEnum *sr_terms_from(IndexReader *ir, int field_num, char *term)
{
    SegmentTermEnum *ste = SR(ir)->tir->orig_te;
    ste = ste_clone(ste);
    ste_set_field(ste, field_num);
    ste_scan_to(ste, term);
    return TE(ste);
}

static int sr_doc_freq(IndexReader *ir, int field_num, char *term)
{
    TermInfo *ti = tir_get_ti(tir_set_field(SR(ir)->tir, field_num), term);
    return ti ? ti->doc_freq : 0;
}

static TermDocEnum *sr_term_docs(IndexReader *ir)
{
    return stde_new(SR(ir)->tir, SR(ir)->frq_in, SR(ir)->deleted_docs);
}

static TermDocEnum *sr_term_positions(IndexReader *ir)
{
    SegmentReader *sr = SR(ir);
    return stpe_new(sr->tir, sr->frq_in, sr->prx_in, sr->deleted_docs);
}

static TermVector *sr_term_vector(IndexReader *ir, int doc_num, char *field)
{
    FieldInfo *fi = h_get(ir->fis->field_dict, field);
    TermVectorsReader *tvr;

    if (!fi || !fi_store_term_vector(fi) || !SR(ir)->orig_tvr ||
        !(tvr = sr_tvr(SR(ir)))) {
        return NULL;
    }

    return tvr_get_field_tv(tvr, doc_num, fi->number);
}

static HashTable *sr_term_vectors(IndexReader *ir, int doc_num)
{
    TermVectorsReader *tvr;
    if (!SR(ir)->orig_tvr || (tvr = sr_tvr(SR(ir))) == NULL) {
        return NULL;
    }

    return tvr_get_tv(tvr, doc_num);
}

static bool sr_is_deleted(IndexReader *ir, int doc_num)
{
    bool is_del;

    mutex_lock(&ir->mutex);
    is_del = sr_is_deleted_i(SR(ir), doc_num);
    mutex_unlock(&ir->mutex);

    return is_del;
}

static bool sr_has_deletions(IndexReader *ir)
{
    return (SR(ir)->deleted_docs != NULL);
}

/*
 * TODO: FIXME
 */
//static char **sr_file_names(IndexReader *ir, int *cnt)
//{
//    char **file_names;
//    FieldInfo *fi;
//    int i;
//    char file_name[SEGMENT_NAME_MAX_LENGTH];
//
//    for (i = 0; i < NELEMS(INDEX_EXTENSIONS); i++) {
//        sprintf(file_name, "%s.%s", SR(ir)->segment, INDEX_EXTENSIONS[i]);
//        if (ir->store->exists(ir->store, file_name)) {
//            file_names[i] = estrdup(file_name);
//        }
//    }
//
//    for (i = 0; i < ir->fis->size; i++) {
//        fi = ir->fis->fields[i];
//        if (fi_has_norms(fi)) {
//            if (SR(ir)->cfs_store) {
//                sprintf(file_name, "%s.s%d", SR(ir)->segment, i);
//            } else {
//                sprintf(file_name, "%s.f%d", SR(ir)->segment, i);
//            }
//            if (ir->store->exists(ir->store, file_name)) {
//                file_names[*cnt] = estrdup(file_name);
//            }
//        }
//    }
//    return file_names;
//}

static void sr_open_norms(IndexReader *ir, Store *cfs_store)
{
    int i;
    FieldInfo *fi;
    Store *tmp_store;
    char file_name[SEGMENT_NAME_MAX_LENGTH];

    for (i = 0; i < ir->fis->size; i++) {
        tmp_store = ir->store;
        fi = ir->fis->fields[i];
        if (fi_has_norms(fi)) {
            sprintf(file_name, "%s.s%d", SR(ir)->segment, fi->number);
            if (!tmp_store->exists(tmp_store, file_name)) {
                sprintf(file_name, "%s.f%d", SR(ir)->segment, fi->number);
                tmp_store = cfs_store;
            }
            h_set(SR(ir)->norms, fi->name,
                  norm_create(tmp_store->open_input(tmp_store, file_name),
                              fi->number));
        }
    }
    SR(ir)->norms_dirty = false;
}

static IndexReader *sr_setup_i(SegmentReader *sr, SegmentInfo *si)
{
    Store *store = si->store;
    IndexReader *ir = IR(sr);
    char file_name[SEGMENT_NAME_MAX_LENGTH];

    ir->num_docs = &sr_num_docs;
    ir->max_doc = &sr_max_doc;
    ir->get_doc = &sr_get_doc;
    ir->get_norms = &sr_get_norms;
    ir->get_norms_into = &sr_get_norms_into;
    ir->terms = &sr_terms;
    ir->terms_from = &sr_terms_from;
    ir->doc_freq = &sr_doc_freq;
    ir->term_docs = &sr_term_docs;
    ir->term_positions = &sr_term_positions;
    ir->term_vector = &sr_term_vector;
    ir->term_vectors = &sr_term_vectors;
    ir->is_deleted = &sr_is_deleted;
    ir->has_deletions = &sr_has_deletions;

    ir->set_norm_i = &sr_set_norm_i;
    ir->delete_doc_i = &sr_delete_doc_i;
    ir->undelete_all_i = &sr_undelete_all_i;
    ir->commit_i = &sr_commit_i;
    ir->close_i = &sr_close_i;

    sr->segment = si->name;
    sr->cfs_store = NULL;
    sr->fake_norms = NULL;

    sprintf(file_name, "%s.cfs", sr->segment);
    if (store->exists(store, file_name)) {
        sr->cfs_store = open_cmpd_store(store, file_name);
        store = sr->cfs_store;
    }

    sr->fr = fr_open(store, sr->segment, ir->fis);
    sr->sfi = sfi_open(store, sr->segment);
    sr->tir = tir_open(store, sr->sfi, sr->segment);

    sr->deleted_docs = NULL;
    sr->deleted_docs_dirty = false;
    sr->undelete_all = false;
    if (si_has_deletions(si)) {
        sprintf(file_name, "%s.del", sr->segment);
        sr->deleted_docs = bv_read(si->store, file_name);
    }

    sprintf(file_name, "%s.frq", sr->segment);
    sr->frq_in = store->open_input(store, file_name);
    sprintf(file_name, "%s.prx", sr->segment);
    sr->prx_in = store->open_input(store, file_name);
    sr->norms = h_new_str((free_ft)NULL, (free_ft)&norm_destroy);
    sr_open_norms(ir, store);

    if (fis_has_vectors(ir->fis)) {
        sr->orig_tvr = tvr_open(store, sr->segment, ir->fis);
        thread_key_create(&sr->thread_tvr, NULL);
        sr->tvr_bucket = ary_new();
    } else {
        sr->orig_tvr = NULL;
    }
    return ir;
}

//static IndexReader *sr_open_si(SegmentInfo *si, FieldInfos *fis)
//{
//    SegmentReader *sr = ALLOC_AND_ZERO(SegmentReader);
//    REF(si->store);
//    return sr_setup_i(SR(ir_setup(IR(sr), si->store, NULL, fis, false)), si);
//}

static IndexReader *sr_open(SegmentInfos *sis, FieldInfos *fis, int si_num,
                            bool is_owner)
{
    SegmentReader *sr = ALLOC_AND_ZERO(SegmentReader);
    SegmentInfo *si = sis->segs[si_num];
    IndexReader *ir = ir_setup(IR(sr), si->store, sis, fis, is_owner);
    REF(si->store);
    return sr_setup_i(SR(ir), si);
}

/****************************************************************************
 * IndexReader
 ****************************************************************************/

/**
 * Will keep a reference to the store. To let this method delete the store
 * make sure you deref the store that you pass to it
 */
IndexReader *ir_open(Store *store)
{
    int i;
    IndexReader *ir;
    SegmentInfos *sis;
    FieldInfos *fis;

    mutex_lock(&store->mutex);
    sis = sis_read(store);
    fis = fis_read(store);
    if (sis->size == 1) {
        ir = sr_open(sis, fis, 0, true);
    }
    else {
        IndexReader **readers = ALLOC_N(IndexReader *, sis->size);
        for (i = 0; i < sis->size; i++) {
            readers[i] = sr_open(sis, fis, i, false);
        }
        REF(store);
        // TODO: undo
        //ir = mr_open(store, sis, fis, readers, sis->size);
        ir = NULL;
    }
    mutex_unlock(&store->mutex);
    return ir;
}


/****************************************************************************
 *
 * Occurence
 *
 ****************************************************************************/

static Occurence *occ_new_wo(MemoryPool *mp, int pos, int start, int end)
{
    Occurence *occ = MP_ALLOC(mp, Occurence);
    occ->pos = pos;
    occ->offset.start = start;
    occ->offset.end = end;
    occ->next = NULL;
    return occ;
}

static Occurence *occ_new(MemoryPool *mp, int pos)
{
    OccurenceWithoutOffsets *occ = MP_ALLOC(mp, OccurenceWithoutOffsets);
    occ->pos = pos;
    occ->next = NULL;
    return (Occurence *)occ;
}

/****************************************************************************
 *
 * Posting
 *
 ****************************************************************************/

Posting *p_new(MemoryPool *mp, int doc_num, int pos)
{
    Posting *p = MP_ALLOC(mp, Posting);
    p->doc_num = doc_num;
    p->first_occ = occ_new(mp, pos);
    p->freq = 1;
    p->next = NULL;
    return p;
}

Posting *p_new_wo(MemoryPool *mp, int doc_num, int pos,
                  int start, int end)
{
    Posting *p = MP_ALLOC(mp, Posting);
    p->doc_num = doc_num;
    p->first_occ = occ_new_wo(mp, pos, start, end);
    p->freq = 1;
    p->next = NULL;
    return p;
}

/****************************************************************************
 *
 * PostingList
 *
 ****************************************************************************/

PostingList *pl_new(MemoryPool *mp, const char *term, int term_len, Posting *p)
{
    PostingList *pl = MP_ALLOC(mp, PostingList);
    pl->term = mp_memdup(mp, term, term_len + 1);
    pl->term_len = term_len;
    pl->first = pl->last = p;
    pl->last_occ = p->first_occ;
    return pl;
}

void pl_add_occ(MemoryPool *mp, PostingList *pl, int pos)
{
    pl->last_occ = pl->last_occ->next = occ_new(mp, pos);
    pl->last->freq++;
}

void pl_add_occ_wo(MemoryPool *mp, PostingList *pl, int pos, int start, int end)
{
    pl->last_occ = pl->last_occ->next = occ_new_wo(mp, pos, start, end);
    pl->last->freq++;
}

void pl_add_posting(PostingList *pl, Posting *p)
{
    pl->last = pl->last->next = p;
    pl->last_occ = p->first_occ;
}

int pl_cmp(const PostingList **pl1, const PostingList **pl2)
{
    return strcmp((*pl1)->term, (*pl2)->term);
}

/****************************************************************************
 *
 * TermVector
 *
 ****************************************************************************/

void tv_destroy(TermVector *tv)
{
    int i;
    for (i = 0; i < tv->size; i++) {
        free(tv->terms[i].text);
        free(tv->terms[i].positions);
        free(tv->terms[i].offsets);
    }
    free(tv->field);
    free(tv->terms);
    free(tv);
}

/****************************************************************************
 *
 * TermVectorsReader
 *
 ****************************************************************************/

TermVectorsReader *tvr_open(Store *store,
                            const char *segment,
                            FieldInfos *fis)
{
    TermVectorsReader *tvr = ALLOC(TermVectorsReader);
    char file_name[SEGMENT_NAME_MAX_LENGTH];

    tvr->fis = fis;
    sprintf(file_name, "%s"TVX_EXTENSION, segment);
    tvr->tvx_in = store->open_input(store, file_name);
    tvr->size = is_length(tvr->tvx_in) / 12;

    sprintf(file_name, "%s"TVD_EXTENSION, segment);
    tvr->tvd_in = store->open_input(store, file_name);
    return tvr;
}

TermVectorsReader *tvr_clone(TermVectorsReader *orig)
{
    TermVectorsReader *tvr = ALLOC(TermVectorsReader);

    memcpy(tvr, orig, sizeof(TermVectorsReader));
    tvr->tvx_in = is_clone(orig->tvx_in);
    tvr->tvd_in = is_clone(orig->tvd_in);
    
    return tvr;
}

void tvr_close(TermVectorsReader *tvr)
{
    is_close(tvr->tvx_in);
    is_close(tvr->tvd_in);
    free(tvr);
}

TermVector *tvr_read_term_vector(TermVectorsReader *tvr, int field_num)
{
    TermVector *tv = ALLOC_AND_ZERO(TermVector);
    InStream *tvd_in = tvr->tvd_in;
    int num_terms;
    FieldInfo *fi = tvr->fis->fields[field_num];
    
    tv->field_num = field_num;
    tv->field = estrdup(fi->name);

    num_terms = is_read_vint(tvd_in);
    if (num_terms > 0) {
        int i, j, delta_start, delta_len, total_len, freq;
        int store_positions = fi_store_positions(fi);
        int store_offsets = fi_store_offsets(fi);
        uchar buffer[MAX_WORD_SIZE];
        Term *term;

        tv->size = num_terms;
        tv->terms = ALLOC_AND_ZERO_N(Term, num_terms);

        for (i = 0; i < num_terms; i++) {
            term = &(tv->terms[i]);
            /* read delta encoded term */
            delta_start = is_read_vint(tvd_in);
            delta_len = is_read_vint(tvd_in);
            total_len = delta_start + delta_len;
            is_read_bytes(tvd_in, buffer + delta_start, delta_len);
            buffer[total_len++] = '\0';
            term->text = memcpy(ALLOC_N(char, total_len), buffer, total_len);

            /* read freq */
            freq = term->freq = is_read_vint(tvd_in);

            /* read positions if necessary */
            if (store_positions) {
                int *positions = term->positions = ALLOC_N(int, freq);
                int pos = 0;
                for (j = 0; j < freq; j++) {
                    positions[j] = pos += is_read_vint(tvd_in);
                }
            }

            /* read offsets if necessary */
            if (store_offsets) {
                Offset *offsets = term->offsets = ALLOC_N(Offset, freq);
                int offset = 0;
                for (j = 0; j < freq; j++) {
                    offsets[j].start = offset += is_read_vint(tvd_in);
                    offsets[j].end = offset += is_read_vint(tvd_in);
                }
            }
        }
    }
    return tv;
}

HashTable *tvr_get_tv(TermVectorsReader *tvr, int doc_num)
{
    HashTable *term_vectors = h_new_str((free_ft)NULL, (free_ft)&tv_destroy);
    int i;
    InStream *tvx_in = tvr->tvx_in;
    InStream *tvd_in = tvr->tvd_in;
    long data_pointer, field_index_pointer;
    int field_cnt;
    int *field_nums;

    is_seek(tvx_in, 12 * doc_num);

    data_pointer = (long)is_read_u64(tvx_in);
    field_index_pointer = data_pointer + (long)is_read_u32(tvx_in);

    /* scan fields to get position of field_num's term vector */
    is_seek(tvd_in, field_index_pointer);

    field_cnt = is_read_vint(tvd_in);
    field_nums = ALLOC_N(int, field_cnt);

    for (i = 0; i < field_cnt; i++) {
        field_nums[i] = is_read_vint(tvd_in);
        is_read_vint(tvd_in); /* skip space, we don't need it */
    }
    is_seek(tvd_in, data_pointer);

    for (i = 0; i < field_cnt; i++) {
        TermVector *tv = tvr_read_term_vector(tvr, field_nums[i]);
        h_set(term_vectors, tv->field, tv);
    }
    free(field_nums);

    return term_vectors;
}

TermVector *tvr_get_field_tv(TermVectorsReader *tvr,
                             int doc_num,
                             int field_num)
{
    int i;
    InStream *tvx_in = tvr->tvx_in;
    InStream *tvd_in = tvr->tvd_in;
    long data_pointer, field_index_pointer;
    int field_cnt;
    int offset = 0;
    TermVector *tv = NULL;

    is_seek(tvx_in, 12 * doc_num);

    data_pointer = (long)is_read_u64(tvx_in);
    field_index_pointer = data_pointer + (long)is_read_u32(tvx_in);

    /* scan fields to get position of field_num's term vector */
    is_seek(tvd_in, field_index_pointer);

    field_cnt = is_read_vint(tvd_in);
    for (i = 0; i < field_cnt; i++) {
        if ((int)is_read_vint(tvd_in) == field_num) {
            break;
        }
        offset += is_read_vint(tvd_in); /* space taken by field */
    }
    if (i < field_cnt) {
        /* field was found */
        is_seek(tvd_in, data_pointer + offset);
        tv = tvr_read_term_vector(tvr, field_num);
    }
    return tv;
}

/****************************************************************************
 *
 * Boost
 *
 ****************************************************************************/

static Boost *boost_new(MemoryPool *mp,
                        float val,
                        int doc_num,
                        Boost *next)
{
    Boost *boost = MP_ALLOC(mp, Boost);
    boost->val = val;
    boost->doc_num = doc_num;
    boost->next = next;
    return boost;
}

/****************************************************************************
 *
 * FieldInverter
 *
 ****************************************************************************/

static FieldInverter *fld_inv_new(DocWriter *dw, const char *fld_name)
{
    FieldInverter *fld_inv = MP_ALLOC(dw->mp, FieldInverter);
    FieldInfo *fi = fis_get_or_add_field(dw->fis, fld_name);

    fld_inv->is_tokenized = fi_is_tokenized(fi);
    fld_inv->store_term_vector = fi_store_term_vector(fi);
    fld_inv->store_offsets = fi_store_offsets(fi);

    fld_inv->fi = fi;

    fld_inv->boosts = boost_new(dw->mp, fi->boost, 0, NULL);

    /* this will alloc it's own memory so must be destroyed */
    fld_inv->plists = h_new_str(NULL, NULL);

    return fld_inv;
}

static void fld_inv_destroy(FieldInverter *fld_inv)
{
    h_destroy(fld_inv->plists);
}

/****************************************************************************
 *
 * SkipBuffer
 *
 ****************************************************************************/

typedef struct SkipBuffer
{
    OutStream *buf;
    OutStream *frq_out;
    OutStream *prx_out;
    int last_doc;
    int last_frq_pointer;
    int last_prx_pointer;
} SkipBuffer;

static void skip_buf_reset(SkipBuffer *skip_buf)
{
    ramo_reset(skip_buf->buf);
    skip_buf->last_doc = 0;
    skip_buf->last_frq_pointer = os_pos(skip_buf->frq_out);
    skip_buf->last_prx_pointer = os_pos(skip_buf->prx_out);
}

static SkipBuffer *skip_buf_new(MemoryPool *mp,
                                OutStream *frq_out,
                                OutStream *prx_out)
{
    SkipBuffer *skip_buf = MP_ALLOC(mp, SkipBuffer);
    skip_buf->buf = ram_new_buffer();
    skip_buf->frq_out = frq_out;
    skip_buf->prx_out = prx_out;
    return skip_buf;
}

static void skip_buf_add(SkipBuffer *skip_buf, int doc)
{
    int frq_pointer = os_pos(skip_buf->frq_out);
    int prx_pointer = os_pos(skip_buf->prx_out);

    os_write_vint(skip_buf->buf, doc - skip_buf->last_doc);
    os_write_vint(skip_buf->buf, frq_pointer - skip_buf->last_frq_pointer);
    os_write_vint(skip_buf->buf, prx_pointer - skip_buf->last_prx_pointer);

    skip_buf->last_doc = doc;
    skip_buf->last_frq_pointer = frq_pointer;
    skip_buf->last_prx_pointer = prx_pointer;
}

static int skip_buf_write(SkipBuffer *skip_buf)
{
  int skip_pointer = os_pos(skip_buf->frq_out);
  ramo_write_to(skip_buf->buf, skip_buf->frq_out);
  return skip_pointer;
}

static void skip_buf_destroy(SkipBuffer *skip_buf)
{
    ram_destroy_buffer(skip_buf->buf);
}

/****************************************************************************
 *
 * DocWriter
 *
 ****************************************************************************/

static void dw_write_norms(DocWriter *dw, FieldInverter *fld_inv)
{
    int i;
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    Boost *boost = fld_inv->boosts;
    uchar *norms = ALLOC_N(uchar, dw->doc_num);
    OutStream *norms_out;

    for (i = dw->doc_num - 1; i >= 0; i--) {
        norms[i] = sim_encode_norm(dw->similarity, boost->val);
        if (i <= boost->doc_num) {
            boost = boost->next;
        }
    }

    sprintf(file_name, "%s.f%d", dw->segment, fld_inv->fi->number);
    norms_out = dw->store->new_output(dw->store, file_name);
    os_write_bytes(norms_out, norms, dw->doc_num);
    os_close(norms_out);
    free(norms);
}

//static void dw_write_dummy_norms(DocWriter *dw, int field_num)
//{
//    uchar *norms = ALLOC_AND_ZERO_N(uchar, dw->doc_num);
//    char file_name[SEGMENT_NAME_MAX_LENGTH];
//    OutStream *norms_out;
//    sprintf(file_name, "%s.f%d", dw->segment, field_num);
//    norms_out = dw->store->new_output(dw->store, file_name);
//    os_write_bytes(norms_out, norms, dw->doc_num);
//    os_close(norms_out);
//    free(norms);
//}

/* we'll use the postings HashTable's table area to sort the postings as it is
 * going to be zeroset soon anyway */
static PostingList **dw_sort_postings(HashTable *plists_ht)
{
    int i, j;
    HashEntry *he;
    PostingList **plists = (PostingList **)plists_ht->table;
    for (i = 0, j = 0; i <= plists_ht->mask; i++) {
        he = &plists_ht->table[i];
        if (he->value) {
            plists[j++] = (PostingList *)he->value;
        }
    }

    qsort(plists, plists_ht->size, sizeof(PostingList *),
          (int (*)(const void *, const void *))&pl_cmp);

    return plists;
}

static void dw_flush(DocWriter *dw)
{
    int i, j, last_doc, doc_code, doc_freq, last_pos;
    int skip_interval = dw->skip_interval;
    FieldInfos *fis = dw->fis;
    FieldInverter *fld_inv;
    FieldInfo *fi;
    PostingList **pls, *pl;
    Posting *p;
    Occurence *occ;
    Store *store = dw->store;
    TermInfosWriter *tiw = tiw_open(store, dw->segment,
                                    dw->index_interval, skip_interval);
    TermInfo ti;
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    OutStream *frq_out, *prx_out;
    SkipBuffer *skip_buf;

    sprintf(file_name, "%s.frq", dw->segment);
    frq_out = store->new_output(store, file_name);
    sprintf(file_name, "%s.prx", dw->segment);
    prx_out = store->new_output(store, file_name);
    skip_buf = skip_buf_new(dw->mp, frq_out, prx_out);

    for (i = 0; i < fis->size; i++) {
        fi = fis->fields[i];
        if (!fi_is_indexed(fi)) {
            continue;
        }
        if ((fld_inv = h_get(dw->fields, fi->name)) == NULL) {
            continue;
        }
        if (!fi_omit_norms(fi)) {
            dw_write_norms(dw, fld_inv);
        }

        pls = dw_sort_postings(fld_inv->plists);
        tiw_start_field(tiw, fi->number);
        for (j = 0; j < fld_inv->plists->size; j++) {
            pl = pls[j];
            ti.frq_pointer = os_pos(frq_out);
            ti.prx_pointer = os_pos(prx_out);
            last_doc = 0;
            doc_freq = 0;
            skip_buf_reset(skip_buf);
            for (p = pl->first; p != NULL; p = p->next) {
                doc_freq++;
                if ((doc_freq % dw->skip_interval) == 0) {
                    skip_buf_add(skip_buf, last_doc);
                }

                doc_code = (p->doc_num - last_doc) << 1;
                last_doc = p->doc_num;

                if (p->freq == 1) {
                    os_write_vint(frq_out, 1|doc_code);
                }
                else {
                    os_write_vint(frq_out, doc_code);
                    os_write_vint(frq_out, p->freq);
                }

                last_pos = 0;
                for (occ = p->first_occ; occ != NULL; occ = occ->next) {
                    os_write_vint(prx_out, occ->pos - last_pos);
                    last_pos = occ->pos;
                }
            }
            ti.skip_offset = skip_buf_write(skip_buf) - ti.frq_pointer;
            ti.doc_freq = doc_freq;
            tiw_add(tiw, pl->term, pl->term_len, &ti);
        }
    }
    os_close(prx_out);
    os_close(frq_out);
    tiw_close(tiw);
    skip_buf_destroy(skip_buf);
} 

DocWriter *dw_open(IndexWriter *iw, const char *segment)
{
    Store *store = iw->store;
    Config *config = iw->config;
    MemoryPool *mp = mp_new_capa(config->chunk_size,
             config->max_buffer_memory/config->chunk_size);

    DocWriter *dw = ALLOC(DocWriter);

    dw->mp = mp;
    dw->analyzer = iw->analyzer;
    dw->fis = iw->fis;
    dw->store = store;
    dw->tvw = tvw_open(store, segment, iw->fis);
    dw->fw = fw_open(store, segment, iw->fis);
    dw->segment = segment;

    dw->curr_plists = h_new_str(NULL, NULL);
    dw->fields = h_new_str(NULL, (free_ft)fld_inv_destroy);
    dw->doc_num = 0;

    dw->index_interval = config->index_interval;
    dw->skip_interval = config->skip_interval;
    dw->max_field_length = config->max_field_length;

    dw->similarity = iw->similarity;
    return dw;
}

void dw_new_segment(DocWriter *dw, char *segment)
{
    dw_flush(dw);
    mp_reset(dw->mp);
    tvw_close(dw->tvw);
    fw_close(dw->fw);

    h_clear(dw->fields);

    dw->tvw = tvw_open(dw->store, segment, dw->fis);
    dw->fw = fw_open(dw->store, segment, dw->fis);
    dw->segment = segment;
    dw->doc_num = 0;
}

void dw_close(DocWriter *dw)
{
    if (dw->doc_num) { 
        dw_flush(dw);
    }
    tvw_close(dw->tvw);
    fw_close(dw->fw);
    h_destroy(dw->curr_plists);
    h_destroy(dw->fields);
    mp_destroy(dw->mp);
    free(dw);
}

FieldInverter *dw_get_fld_inv(DocWriter *dw, const char *fld_name)
{
    FieldInverter *fld_inv = h_get(dw->fields, fld_name);

    if (!fld_inv) {
        fld_inv = fld_inv_new(dw, fld_name);
        h_set(dw->fields, fld_inv->fi->name, fld_inv);
    }
    return fld_inv;
}

static void dw_add_posting(MemoryPool *mp,
                           HashTable *curr_plists,
                           HashTable *fld_plists,
                           int doc_num,
                           const char *text,
                           int len,
                           int pos,
                           int start,
                           int end,
                           bool store_offsets)
{
    HashEntry *pl_he = h_set_ext(curr_plists, text);
    if (pl_he->value) {
        if (store_offsets) {
            pl_add_occ_wo(mp, pl_he->value, pos, start, end);
        }
        else {
            pl_add_occ(mp, pl_he->value, pos);
        }
    } else {
        HashEntry *fld_pl_he = h_set_ext(fld_plists, text);
        PostingList *pl = fld_pl_he->value;
        Posting *p;
        if (store_offsets) {
            p =  p_new_wo(mp, doc_num, pos, start, end);
        }
        else {
            p =  p_new(mp, doc_num, pos);
        }
        if (!pl) {
            pl = fld_pl_he->value = pl_new(mp, text, len, p);
            pl_he->key = fld_pl_he->key = (char *)pl->term;
        }
        else {
            pl_add_posting(pl, p);
            pl_he->key = (char *)pl->term;
        }
        pl_he->value = pl;
    }
}

HashTable *dw_invert_field(DocWriter *dw,
                           FieldInverter *fld_inv,
                           DocField *df)
{
    MemoryPool *mp = dw->mp;
    Analyzer *a = dw->analyzer;
    HashTable *curr_plists = dw->curr_plists;
    HashTable *fld_plists = fld_inv->plists;
    const bool store_offsets = fld_inv->store_offsets;
    int doc_num = dw->doc_num;
    int i;
    if (fld_inv->is_tokenized) {
        Token *tk;
        int pos = -1, num_terms = 0;
        TokenStream *ts = a_get_ts(a, df->name, "");

        for (i = 0; i < df->size; i++) {
            ts->reset(ts, df->data[i]);
            while (NULL != (tk = ts->next(ts))) {
                pos += tk->pos_inc;
                dw_add_posting(mp, curr_plists, fld_plists, doc_num, tk->text,
                               tk->len, pos, tk->start, tk->end, store_offsets);
                if (num_terms++ >= dw->max_field_length) {
                    break;
                }
            }
        }
        ts_deref(ts);
        fld_inv->length = num_terms;
    }
    else {
        for (i = 0; i < df->size; i++) {
            int len = df->lengths[i];
            char buf[MAX_WORD_SIZE];
            char *data_ptr = df->data[i];
            if (len > MAX_WORD_SIZE) {
                len = MAX_WORD_SIZE - 1;
                data_ptr = memcpy(buf, df->data[i], MAX_WORD_SIZE);
            }
            dw_add_posting(mp, curr_plists, fld_plists, doc_num, data_ptr,
                           len, i, 0, df->lengths[i], store_offsets);
        }
        fld_inv->length = i;
    }
    return curr_plists;
}

void dw_reset_postings(HashTable *postings)
{
    ZEROSET_N(postings->table, HashEntry, postings->mask + 1);
    postings->fill = postings->size = 0;
}

void dw_add_doc(DocWriter *dw, Document *doc)
{
    int i;
    float boost;
    DocField *df;
    FieldInverter *fld_inv;
    HashTable *postings;

    fw_add_doc(dw->fw, doc);

    tvw_open_doc(dw->tvw);
    for (i = 0; i < doc->size; i++) {
        df = doc->fields[i];
        fld_inv = dw_get_fld_inv(dw, df->name);

        postings = dw_invert_field(dw, fld_inv, df);
        if (fld_inv->store_term_vector) {
            tvw_add_postings(dw->tvw, fld_inv->fi->number,
                             dw_sort_postings(postings),
                             postings->size);
        }

        boost = fld_inv->fi->boost * doc->boost * df->boost *
          sim_length_norm(dw->similarity, fld_inv->fi->number, fld_inv->length);
        if (boost != fld_inv->boosts->val) {
            fld_inv->boosts
                = boost_new(dw->mp, boost, dw->doc_num, fld_inv->boosts);
        }

        dw_reset_postings(postings);
    }
    tvw_close_doc(dw->tvw);
    dw->doc_num++;
}

/****************************************************************************
 *
 * IndexWriter
 *
 ****************************************************************************/

/* prepare an index ready for writing */
void index_create(Store *store, FieldInfos *fis)
{
    SegmentInfos *sis = sis_new();
    store->clear_all(store);
    sis_write(sis, store);
    sis_destroy(sis);
    fis_write(fis, store);
}

IndexWriter *iw_open(Store *store, Analyzer *analyzer, const Config *config)
{
    IndexWriter *iw = ALLOC(IndexWriter);
    mutex_init(&iw->mutex, NULL);
    iw->store = store;
    iw->analyzer = analyzer;
    if (!config) {
        config = &default_config;
    }
    iw->config = memcpy(ALLOC(Config), config, sizeof(Config));

    iw->sis = sis_read(store);
    iw->fis = fis_read(store);
    iw->dw = NULL;
    iw->similarity = sim_create_default();
    return iw;
}

void iw_close(IndexWriter *iw)
{
    mutex_destroy(&iw->mutex);
    if (iw->dw) {
        dw_close(iw->dw);
    }
    a_deref(iw->analyzer);
    sis_write(iw->sis, iw->store);
    fis_write(iw->fis, iw->store);
    sis_destroy(iw->sis);
    fis_destroy(iw->fis);
    sim_destroy(iw->similarity);
    free(iw->config);
    free(iw);
}

static void iw_maybe_flush_segment(IndexWriter *iw)
{
    DocWriter *dw = iw->dw;
    SegmentInfos *sis = iw->sis;
    if (mp_used(dw->mp) > iw->config->max_buffer_memory) {
        sis->segs[sis->size - 1]->doc_cnt = dw->doc_num;
        dw_new_segment(dw, sis_new_segment(sis, 1, iw->store)->name);
    }
}

void iw_add_doc(IndexWriter *iw, Document *doc)
{
    mutex_lock(&iw->mutex);
    if (!iw->dw) {
        iw->dw =
            dw_open(iw, sis_new_segment(iw->sis, 0, iw->store)->name);
    }
    dw_add_doc(iw->dw, doc);
    iw_maybe_flush_segment(iw);
    mutex_unlock(&iw->mutex);
}

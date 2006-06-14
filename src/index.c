#include "index.h"
#include "helper.h"
#include <string.h>
#include <limits.h>

/****************************************************************************
 *
 * FieldInfo
 *
 ****************************************************************************/

inline void fi_set_store(FieldInfo *self, int store)
{
    switch (store) {
        case STORE_NO:
            break;
        case STORE_YES:
            self->bits |= FI_IS_STORED_BM;
            break;
        case STORE_COMPRESS:
            self->bits |= FI_IS_COMPRESSED_BM | FI_IS_STORED_BM;
            break;
    }
}

inline void fi_set_index(FieldInfo *self, int index)
{
    switch (index) {
        case INDEX_NO:
            break;
        case INDEX_YES:
            self->bits |= FI_IS_INDEXED_BM | FI_IS_TOKENIZED_BM;
            break;
        case INDEX_UNTOKENIZED:
            self->bits |= FI_IS_INDEXED_BM;
            break;
        case INDEX_YES_OMIT_NORMS:
            self->bits |= FI_OMIT_NORMS_BM | FI_IS_INDEXED_BM |
                FI_IS_TOKENIZED_BM;
            break;
        case INDEX_UNTOKENIZED_OMIT_NORMS:
            self->bits |= FI_OMIT_NORMS_BM | FI_IS_INDEXED_BM;
            break;
    }
}

inline void fi_set_term_vector(FieldInfo *self, int term_vector)
{
    switch (term_vector) {
        case TERM_VECTOR_NO:
            break;
        case TERM_VECTOR_YES:
            self->bits |= FI_STORE_TERM_VECTOR_BM;
            break;
        case TERM_VECTOR_WITH_POSITIONS:
            self->bits |= FI_STORE_TERM_VECTOR_BM | FI_STORE_POSITIONS_BM;
            break;
        case TERM_VECTOR_WITH_OFFSETS:
            self->bits |= FI_STORE_TERM_VECTOR_BM | FI_STORE_OFFSETS_BM;
            break;
        case TERM_VECTOR_WITH_POSITIONS_OFFSETS:
            self->bits |= FI_STORE_TERM_VECTOR_BM | FI_STORE_POSITIONS_BM |
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

FieldInfo *fi_new(char *name,
                  int store,
                  int index,
                  int term_vector)
{
    FieldInfo *self = ALLOC(FieldInfo);
    fi_check_params(store, index, term_vector);
    self->name = estrdup(name);
    self->boost = 1.0;
    self->bits = 0;
    fi_set_store(self, store);
    fi_set_index(self, index);
    fi_set_term_vector(self, term_vector);
    return self;
}

void fi_destroy(FieldInfo *self)
{
    free(self->name);
    free(self);
}

char *fi_to_s(FieldInfo *self)
{
    char *str = ALLOC_N(char, strlen(self->name) + 200);
    char *s = str;
    sprintf(str, "[\"%s\":(%s%s%s%s%s%s%s%s", self->name,
            fi_is_stored(self) ? "is_stored, " : "",
            fi_is_compressed(self) ? "is_compressed, " : "",
            fi_is_indexed(self) ? "is_indexed, " : "",
            fi_is_tokenized(self) ? "is_tokenized, " : "",
            fi_omit_norms(self) ? "omit_norms, " : "",
            fi_store_term_vector(self) ? "store_term_vector, " : "",
            fi_store_offsets(self) ? "store_offsets, " : "",
            fi_store_positions(self) ? "store_positions, " : "");
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
    FieldInfos *self = ALLOC(FieldInfos);
    fi_check_params(store, index, term_vector);
    self->field_dict = h_new_str((free_ft)NULL, (free_ft)&fi_destroy);
    self->size = 0;
    self->capa = FIELD_INFOS_INIT_CAPA;
    self->fields = ALLOC_N(FieldInfo *, self->capa);
    self->store = store;
    self->index = index;
    self->term_vector = term_vector;
    return self;
}

FieldInfo *fis_add_field(FieldInfos *self, FieldInfo *fi)
{
    if (self->size == self->capa) {
        self->capa <<= 1;
        REALLOC_N(self->fields, FieldInfo *, self->capa);
    }
    if (!h_set_safe(self->field_dict, fi->name, fi)) {
        return NULL;
    }
    fi->number = self->size;
    self->fields[self->size] = fi;
    self->size++;
    return fi;
}

FieldInfo *fis_get_field(FieldInfos *self, char *name)
{
    return h_get(self->field_dict, name);
}

FieldInfo *fis_get_or_add_field(FieldInfos *self, char *name)
{
    FieldInfo *fi = h_get(self->field_dict, name);
    if (!fi) {
        fi = fi_new(name, self->store, self->index, self->term_vector);
        fis_add_field(self, fi);
    }
    return fi;
}

FieldInfo *fis_by_number(FieldInfos *self, int num)
{
    if (num >= 0 && num < self->size) {
        return self->fields[num];
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
    FieldInfos *self;
    InStream *is = store->open_input(store, FIELDS_FILENAME);

    store_val = is_read_vint(is);
    index_val = is_read_vint(is);
    term_vector_val = is_read_vint(is);
    self = fis_new(store_val, index_val, term_vector_val);
    for (i = is_read_vint(is); i > 0; i--) {
        fi = ALLOC(FieldInfo);
        fi->name = is_read_string(is);
        tmp.i = is_read_u32(is);
        fi->boost = tmp.f;
        fi->bits = is_read_vint(is);
        fis_add_field(self, fi);
    }
    is_close(is);

    return self; 
}

void fis_write(FieldInfos *self, Store *store)
{
    int i;
    union { f_u32 i; float f; } tmp;
    FieldInfo *fi;
    OutStream *os = store->new_output(store, TEMPORARY_FIELDS_FILENAME);

    os_write_vint(os, self->store);
    os_write_vint(os, self->index);
    os_write_vint(os, self->term_vector);
    os_write_vint(os, self->size);
    for (i = 0; i < self->size; i++) {
        fi = self->fields[i];
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
    return term_vector_str[(fi->bits >> 2) & 0x7];
}

char *fis_to_s(FieldInfos *self)
{
    int i, pos, capa = 200 + self->size * 120;
    char *buf = ALLOC_N(char, capa);
    FieldInfo *fi;

    sprintf(buf, 
            "default:\n"
            "  store: %s\n"
            "  index: %s\n"
            "  term_vector: %s\n"
            "fields:\n",
            store_str[self->store], index_str[self->index],
            term_vector_str[self->term_vector]);
    pos = (int)strlen(buf);
    for (i = 0; i < self->size; i++) {
        fi = self->fields[i];
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

void fis_destroy(FieldInfos *self)
{
    h_destroy(self->field_dict);
    free(self->fields);
    free(self);
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
    if (strncmp(file_name,
                nt->norm_file_pattern,
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

static char *new_seg_name(f_u64 counter) 
{
  char file_name[SEGMENT_NAME_MAX_LENGTH];
  int i;

  file_name[SEGMENT_NAME_MAX_LENGTH - 1] = '\0';
  for (i = SEGMENT_NAME_MAX_LENGTH - 2; ; i--) {
    file_name[i] = base36_digitmap[counter%36];
    counter /= 36;
    if (counter == 0) break;
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
    return sis_add_si(sis, si_new(new_seg_name(sis->counter++), doc_cnt,
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

FieldsReader *fr_open(Store *store, char *seg_name, FieldInfos *fis)
{
    FieldsReader *fr = ALLOC(FieldsReader);
    InStream *fdx_in;
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    size_t seg_name_len = strlen(seg_name);

    memcpy(file_name, seg_name, seg_name_len);

    fr->fis = fis;
    strcpy(file_name + seg_name_len, ".fdt");
    fr->fdt_in = store->open_input(store, file_name);
    strcpy(file_name + seg_name_len, ".fdx");
    fdx_in = fr->fdx_in = store->open_input(store, file_name);
    fr->len = fdx_in->m->length_i(fdx_in) / 8;

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
    return df;
}

Document *fr_get_doc(FieldsReader *fr, int doc_num)
{
    int i, j;
    FieldInfo *fi;
    f_u64 position;
    int stored_cnt, field_num;
    DocField *df;
    Document *doc = doc_new();
    InStream *fdx_in = fr->fdx_in;
    InStream *fdt_in = fr->fdt_in;

    is_seek(fdx_in, doc_num * 8);
    position = is_read_u64(fdx_in);
    is_seek(fdt_in, (long)position);
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

FieldsWriter *fw_open(Store *store, char *seg_name, FieldInfos *fis)
{
    FieldsWriter *fw = ALLOC(FieldsWriter);
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    size_t seg_name_len = strlen(seg_name);

    memcpy(file_name, seg_name, seg_name_len);

    strcpy(file_name + seg_name_len, ".fdt");
    fw->fdt_out = store->new_output(store, file_name);

    strcpy(file_name + seg_name_len, ".fdx");
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
                os_write_bytes(fdt_out, (unsigned char *)df->data[j],
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

static void ste_reset(SegmentTermEnum *ste);
static char *ste_next(SegmentTermEnum *ste);

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
            sti->index_term_lens[i] = index_ste->te.curr_term_len;
            sti->index_term_infos[i] = index_ste->te.curr_ti;
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

SegmentFieldIndex *sfi_open(Store *store, char *seg_name)
{
    int field_count;
    SegmentFieldIndex *sfi = ALLOC(SegmentFieldIndex);
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    InStream *is;

    mutex_init(&sfi->mutex, NULL);

    sprintf(file_name, "%s.tfx", seg_name);
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

    sprintf(file_name, "%s.tix", seg_name);
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
    ti->doc_freq = (long)is_read_vint(is);     /* read doc freq */
    ti->freq_pointer += (long)is_read_vint(is);/* read freq pointer */
    ti->prox_pointer += (long)is_read_vint(is);/* read prox pointer */
    if (ti->doc_freq >= ste->skip_interval) {
        ti->skip_offset = (int)is_read_vint(is);
    }

    return te->curr_term;
}

static void ste_reset(SegmentTermEnum *ste)
{
    ste->pos = -1;
    ste->te.curr_term[0] = '\0';
    ste->te.curr_term_len = 0;
    ZEROSET(&(ste->te.curr_ti), TermInfo);
}

static void ste_set_field(SegmentTermEnum *ste, int field_num)
{
    SegmentTermIndex *sti = h_get_int(ste->sfi->field_dict, field_num);
    ste_reset(ste);
    ste->field_num = field_num;
    if (sti) {
        ste->size = sti->size;
        is_seek(ste->is, sti->pointer);
    }
}

static void ste_index_seek(SegmentTermEnum *ste,
                           SegmentTermIndex *sti,
                           int index_offset)
{
    int term_len = sti->index_term_lens[index_offset];
    is_seek(ste->is, sti->index_pointers[index_offset]);
    ste->pos = ste->sfi->index_interval * index_offset - 1;
    memcpy(ste->te.curr_term,
           sti->index_terms[index_offset],
           term_len + 1);
    ste->te.curr_term_len = term_len;
    ste->te.curr_ti = sti->index_term_infos[index_offset];
}

static char *ste_scan_to(SegmentTermEnum *ste, const char *term)
{
    SegmentFieldIndex *sfi = ste->sfi;
    SegmentTermIndex *sti = h_get_int(sfi->field_dict, ste->field_num);
    if (sti) {
        SFI_ENSURE_INDEX_IS_READ(sfi, sti);
        /* if current term is less than seek term */
        if (ste->pos < ste->size && strcmp(ste->te.curr_term, term) <= 0) {
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

    ste->te.next = (char *(*)(TermEnum *te))&ste_next;
    ste->te.set_field = (TermEnum *(*)(TermEnum *te, int fn))&ste_set_field;
    ste->te.skip_to = (char *(*)(TermEnum *te, const char *term))&ste_scan_to;
    ste->te.close = (void (*)(TermEnum *te))&ste_close;
    ste->te.clone = (TermEnum *(*)(TermEnum *te))&ste_clone;
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

    if (strcmp(ste->te.curr_term, term) == 0) {
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
    return ste->te.curr_term;
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
 * TermIndexReader
 * (Segment Specific)
 *
 ****************************************************************************/

TermIndexReader *tir_open(Store *store, SegmentFieldIndex *sfi, char *seg_name)
{
    TermIndexReader *tir = ALLOC(TermIndexReader);
    char file_name[SEGMENT_NAME_MAX_LENGTH];

    sprintf(file_name, "%s.tis", seg_name);
    tir->orig_te = ste_new(store->open_input(store, file_name), sfi);
    thread_key_create(&tir->thread_ste, NULL);
    tir->ste_bucket = ALLOC(SegmentTermEnum *);
    tir->ste_bucket_size = 0;
    tir->ste_bucket_capa = 1;

    return tir;
}

static inline SegmentTermEnum *tir_enum(TermIndexReader *tir)
{
    SegmentTermEnum *ste;
    if ((ste = thread_getspecific(tir->thread_ste)) == NULL) {
        ste = ste_clone(tir->orig_te);
        if (tir->ste_bucket_size >= tir->ste_bucket_capa) {
            tir->ste_bucket_capa <<= 1;
            REALLOC_N(tir->ste_bucket, SegmentTermEnum *, tir->ste_bucket_capa);
        }
        tir->ste_bucket[tir->ste_bucket_size++] = ste;
        thread_setspecific(tir->thread_ste, ste);
    }
    return ste;
}

void tir_set_field(TermIndexReader *tir, int field_num)
{
    SegmentTermEnum *ste = tir_enum(tir);
    ste_set_field(ste, field_num);
}

TermInfo *tir_get_ti(TermIndexReader *tir, const char *term)
{
    SegmentTermEnum *ste = tir_enum(tir);
    char *match;

    if ((match = ste_scan_to(ste, term)) != NULL && 
        strcmp(match, term) == 0) {
        return &(ste->te.curr_ti);
    }
    return NULL;
}

char *tir_get_term(TermIndexReader *tir, int pos)
{ 
    if (pos < 0) {
        return NULL;
    } else {
        SegmentTermEnum *ste = tir_enum(tir);
        return ste_get_term(ste, pos);
    }
}

void tir_close(TermIndexReader *tir)
{
    int i;
    for (i = 0; i < tir->ste_bucket_size; i++) {
        ste_close(tir->ste_bucket[i]);
    }
    free(tir->ste_bucket);
    ste_close(tir->orig_te);
    thread_key_delete(tir->thread_ste);
    free(tir);
}

/****************************************************************************
 *
 * TermIndexWriter
 *
 ****************************************************************************/

static TermWriter *tw_new(Store *store, char *file_name)
{
    TermWriter *tw = ALLOC_AND_ZERO(TermWriter);
    tw->os = store->new_output(store, file_name);
    return tw;
}

static void tw_close(TermWriter *tw)
{
    os_close(tw->os);
    free(tw);
}

TermIndexWriter *tiw_open(Store *store,
                          const char *seg_name,
                          int index_interval,
                          int skip_interval)
{
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    TermIndexWriter *tiw = ALLOC(TermIndexWriter);
    size_t seg_name_len = strlen(seg_name);
    memcpy(file_name, seg_name, seg_name_len);

    tiw->field_count = 0;
    tiw->index_interval = index_interval;
    tiw->skip_interval = skip_interval;
    tiw->last_index_pointer = 0;

    strcpy(file_name + seg_name_len, ".tix");
    tiw->tix_writer = tw_new(store, file_name);
    strcpy(file_name + seg_name_len, ".tis");
    tiw->tis_writer = tw_new(store, file_name);
    strcpy(file_name + seg_name_len, ".tfx");
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
    if (ti->freq_pointer < tw->last_term_info.freq_pointer) {
        RAISE(STATE_ERROR, "%ld > %ld", ti->freq_pointer,
              tw->last_term_info.freq_pointer);
    }
    if (ti->prox_pointer < tw->last_term_info.prox_pointer) {
        RAISE(STATE_ERROR, "%ld > %ld", ti->prox_pointer,
              tw->last_term_info.prox_pointer);
    }
#endif

    tw_write_term(tw, os, term, term_len);  /* write term */
    os_write_vint(os, ti->doc_freq);        /* write doc freq */
    os_write_vint(os, ti->freq_pointer - tw->last_term_info.freq_pointer);
    os_write_vint(os, ti->prox_pointer - tw->last_term_info.prox_pointer);

    tw->last_term_info = *ti;
    tw->counter++;
}

void tiw_add(TermIndexWriter *tiw,
             const char *term,
             int term_len,
             TermInfo *ti)
{
    long tis_pos;

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
        os_write_vint(tiw->tis_writer->os, ti->skip_offset);
    }
}

static inline void tw_reset(TermWriter *tw)
{
    tw->counter = 0;
    tw->last_term = EMPTY_STRING;
    ZEROSET(&(tw->last_term_info), TermInfo);
}

void tiw_start_field(TermIndexWriter *tiw, int field_num)
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

void tiw_close(TermIndexWriter *tiw)
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
 * TVField
 *
 ****************************************************************************/


/****************************************************************************
 *
 * TermVectorsWriter
 *
 ****************************************************************************/

TermVectorsWriter *tvw_open(Store *store,
                            const char *seg_name,
                            FieldInfos *fis)
{
    TermVectorsWriter *tvw = ALLOC(TermVectorsWriter);
    char file_name[SEGMENT_NAME_MAX_LENGTH];
    tvw->fis = fis;
    tvw->f_size = 0;
    tvw->f_capa = TV_FIELD_INIT_CAPA;
    tvw->fields = ALLOC_N(TVField, tvw->f_capa);

    sprintf(file_name, "%s"TVX_EXTENSION, seg_name);
    tvw->tvx_out = store->new_output(store, file_name);

    sprintf(file_name, "%s"TVD_EXTENSION, seg_name);
    tvw->tvd_out = store->new_output(store, file_name);

    return tvw;
}

void tvw_close(TermVectorsWriter *tvw)
{
    os_close(tvw->tvx_out);
    os_close(tvw->tvd_out);
    free(tvw->fields);
    free(tvw);
}

void tvw_open_doc(TermVectorsWriter *tvw)
{
    tvw->f_size = 0;
    tvw->tvd_pointer = os_pos(tvw->tvd_out);
    os_write_u64(tvw->tvx_out, tvw->tvd_pointer);
}

void tvw_close_doc(TermVectorsWriter *tvw)
{
    int i;
    OutStream *tvd_out = tvw->tvd_out;
    os_write_u32(tvw->tvx_out, (f_u32)(os_pos(tvw->tvd_out) - tvw->tvd_pointer));
    os_write_vint(tvd_out, tvw->f_size);
    for (i = 0; i < tvw->f_size; i++) {
        os_write_vint(tvd_out, tvw->fields[i].field_num);
        os_write_vint(tvd_out, tvw->fields[i].size);
    }
}

static inline void check_fields_capa(TermVectorsWriter *tvw)
{
    if (tvw->f_size >= tvw->f_capa) {
        tvw->f_capa <<= 1;
        REALLOC_N(tvw->fields, TVField, tvw->f_capa);
    }
}

void tvw_add_postings(TermVectorsWriter *tvw,
                      int field_num,
                      Posting **postings,
                      int size)
{
    int i, delta_start, delta_length;
    const char *last_term = EMPTY_STRING;
    long tvd_start_pos = os_pos(tvw->tvd_out);
    OutStream *tvd_out = tvw->tvd_out;
    Posting *posting;
    Occurence *occ;
    FieldInfo *fi = tvw->fis->fields[field_num];
    int store_positions = fi_store_positions(fi);
    int store_offsets = fi_store_offsets(fi);

    check_fields_capa(tvw);
    tvw->fields[tvw->f_size].field_num = field_num;

    os_write_vint(tvd_out, size);
    for (i = 0; i < size; i++) {
        posting = postings[i];
        delta_start = hlp_string_diff(last_term, posting->term);
        delta_length = posting->term_len - delta_start;

        os_write_vint(tvd_out, delta_start);  /* write shared prefix length */
        os_write_vint(tvd_out, delta_length); /* write delta length */
        /* write delta chars */
        os_write_bytes(tvd_out,
                       (uchar *)(posting->term + delta_start),
                       delta_length);
        os_write_vint(tvd_out, posting->freq);
        last_term = posting->term;

        if (store_positions) {
            /* use delta encoding for positions */
            int last_pos = 0;
            for (occ = posting->first_occ; occ; occ = occ->next) {
                os_write_vint(tvd_out, occ->position - last_pos);
                last_pos = occ->position;
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
    tvw->fields[tvw->f_size++].size = os_pos(tvd_out) - tvd_start_pos;
}

/****************************************************************************
 *
 * Occurence
 *
 ****************************************************************************/

static inline Occurence *occ_new(MemoryPool *mp, int position, int start, int end)
{
    Occurence *occ = MP_ALLOC(mp, Occurence);
    occ->position = position;
    occ->offset.start = start;
    occ->offset.end = end;
    occ->next = NULL;
    return occ;
}

static inline Occurence *occ_wo_offsets_new(MemoryPool *mp, int position)
{
    OccurenceWithoutOffsets *occ = MP_ALLOC(mp, OccurenceWithoutOffsets);
    occ->position = position;
    occ->next = NULL;
    return (Occurence *)occ;
}

/****************************************************************************
 *
 * Posting
 *
 ****************************************************************************/

Posting *p_new(MemoryPool *mp, char *term, int term_len, int position)
{
    Posting *p = MP_ALLOC(mp, Posting);
    p->term = term;
    p->term_len = term_len;
    p->first_occ = p->last_occ = occ_wo_offsets_new(mp, position);
    p->freq = 1;
    return p;
}

Posting *p_new_with_offsets(MemoryPool *mp,
                            char *term,
                            int term_len,
                            int position,
                            int start,
                            int end)
{
    Posting *p = MP_ALLOC(mp, Posting);
    p->term = term;
    p->term_len = term_len;
    p->first_occ = p->last_occ = occ_new(mp, position, start, end);
    p->freq = 1;
    return p;
}

void p_add_occurence(MemoryPool *mp, Posting *p, int position)
{
    p->last_occ = p->last_occ->next = occ_wo_offsets_new(mp, position);
    p->freq++;
}

void p_add_occurence_with_offsets(MemoryPool *mp,
                                  Posting *p,
                                  int position,
                                  int start,
                                  int end)
{
    p->last_occ = p->last_occ->next = occ_new(mp, position, start, end);
    p->freq++;
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
    free(tv->terms);
    free(tv);
}

/****************************************************************************
 *
 * TermVectorsReader
 *
 ****************************************************************************/

TermVectorsReader *tvr_open(Store *store,
                            const char *seg_name,
                            FieldInfos *fis)
{
    TermVectorsReader *tvr = ALLOC(TermVectorsReader);
    char file_name[SEGMENT_NAME_MAX_LENGTH];

    tvr->fis = fis;
    sprintf(file_name, "%s"TVX_EXTENSION, seg_name);
    tvr->tvx_in = store->open_input(store, file_name);
    tvr->size = is_length(tvr->tvx_in) / 12;

    sprintf(file_name, "%s"TVD_EXTENSION, seg_name);
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
    
    tv->field_num = field_num;

    num_terms = is_read_vint(tvd_in);
    if (num_terms > 0) {
        int i, j, delta_start, delta_len, total_len, freq;
        FieldInfo *fi = tvr->fis->fields[field_num];
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
    HashTable *term_vectors = h_new_int((free_ft)&tv_destroy);
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
        h_set_int(term_vectors, field_nums[i],
                  tvr_read_term_vector(tvr, field_nums[i]));
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

#define DI_ADD_POSTING(text, len, pos, start, end) do {\
    he = postings->lookup_i(postings, text);\
    if (he->value) {\
        if (store_offsets) {\
            p_add_occurence_with_offsets(mp, he->value, pos, start, end);\
        }\
        else {\
            p_add_occurence(mp, he->value, pos);\
        }\
    } else {\
        char *txt = he->key = mp_memdup(mp, text, len);\
        if (store_offsets) {\
            he->value = p_new_with_offsets(mp, txt, len, pos, start, end);\
        }\
        else {\
            he->value = p_new(mp, txt, len, pos);\
        }\
    }\
} while (0)

HashTable *di_invert_field(DocumentInverter *di,
                           DocField *df,
                           FieldInfo *fi)
{
    MemoryPool *mp = di->mp;
    Analyzer *a = di->analyzer;
    HashTable *postings = di->postings;
    HashEntry *he;
    bool is_tokenized = fi_is_tokenized(fi);
    bool store_offsets = fi_store_offsets(fi);
    int i;
    if (is_tokenized) {
        Token *tk;
        int position = -1;
        TokenStream *ts = a_get_ts(a, df->name, "");
        for (i = 0; i < df->size; i++) {
            ts->reset(ts, df->data[i]);
            while (NULL != (tk = ts->next(ts))) {
                position += tk->pos_inc;
                DI_ADD_POSTING(tk->text, tk->len, position, tk->start, tk->end);
            }
        }
        ts_deref(ts);
    }
    else {
        for (i = 0; i < df->size; i++) {
            DI_ADD_POSTING(df->data[i], df->lengths[i], i, 0, df->lengths[i]);
        }
    }
    return postings;
}

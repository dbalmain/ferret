#include "index.h"
#include <string.h>

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

FieldInfo *fi_create(char *name,
                     int store,
                     int index,
                     int term_vector)
{
    FieldInfo *self = ALLOC(FieldInfo);
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

FieldInfos *fis_create(int store, int index, int term_vector)
{
    FieldInfos *self = ALLOC(FieldInfos);
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
        fi = fi_create(name, self->store, self->index, self->term_vector);
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

void fis_write(FieldInfos *self, OutStream *os)
{
    int i;
    union { f_u32 i; float f; } tmp;
    FieldInfo *fi;

    os_write_uint(os, self->store);
    os_write_uint(os, self->index);
    os_write_uint(os, self->term_vector);
    os_write_uint(os, self->size);
    for (i = 0; i < self->size; i++) {
        fi = self->fields[i];
        os_write_string(os, fi->name);
        tmp.f = fi->boost;
        os_write_uint(os, tmp.i);
        os_write_uint(os, fi->bits);
    }
}

FieldInfos *fis_read(InStream *is)
{
    int store, index, term_vector;
    int i;
    union { f_u32 i; float f; } tmp;
    FieldInfo *fi;
    FieldInfos *self;

    store = is_read_uint(is);
    index = is_read_uint(is);
    term_vector = is_read_uint(is);
    self = fis_create(store, index, term_vector);
    for (i = is_read_uint(is); i > 0; i--) {
        fi = ALLOC(FieldInfo);
        fi->name = is_read_string(is);
        tmp.i = is_read_uint(is);
        fi->boost = tmp.f;
        fi->bits = is_read_uint(is);
        fis_add_field(self, fi);
    }
    return self; 
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

SegmentInfo *si_create(char *name, int doc_cnt, Store *store)
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

static void is_norm_file(char *fname, struct NormTester *nt)
{
    if (strncmp(fname, nt->norm_file_pattern, nt->norm_file_pattern_len) == 0) {
        nt->has_norm_file = true;
    }
}

bool si_has_separate_norms(SegmentInfo *si)
{
    struct NormTester nt;
    sprintf(nt.norm_file_pattern, "%s.s", si->name);
    nt.norm_file_pattern_len = strlen(nt.norm_file_pattern);
    nt.has_norm_file = false;
    si->store->each(si->store, (void (*)(char *fname, void *arg))&is_norm_file, &nt);

    return nt.has_norm_file;
}


/****************************************************************************
 *
 * SegmentInfos
 *
 ****************************************************************************/

#include <time.h>
#define FORMAT 0
#define INDEX_FILENAME "index"
#define TEMPORARY_INDEX_FILENAME "index.new"

static const char base36_digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static char *new_seg_name(f_u64 counter) 
{
  char buf[SEGMENT_NAME_MAX_LENGTH];
  int i;

  buf[SEGMENT_NAME_MAX_LENGTH - 1] = '\0';
  for (i = SEGMENT_NAME_MAX_LENGTH - 2; ; i--) {
    buf[i] = base36_digitmap[counter%36];
    counter /= 36;
    if (counter == 0) break;
  }
  i--;
  buf[i] = '_';
  return estrdup(&buf[i]);
}

SegmentInfos *sis_create(FieldInfos *fis)
{
    SegmentInfos *sis = ALLOC(SegmentInfos);
    sis->format = FORMAT;
    sis->version = (f_u64)time(NULL);
    sis->size = 0;
    sis->counter = 0;
    sis->capa = 4;
    sis->segs = ALLOC_N(SegmentInfo *, sis->capa);
    sis->fis = fis;
    return sis;
}

SegmentInfo *sis_new_segment(SegmentInfos *sis, int doc_cnt, Store *store)
{
    return sis_add_si(sis, si_create(new_seg_name(sis->counter++), doc_cnt,
                                     store));
}

void sis_destroy(SegmentInfos *sis)
{
    int i;
    for (i = 0; i < sis->size; i++) {
        si_destroy(sis->segs[i]);
    }
    fis_destroy(sis->fis);
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
    SegmentInfos *sis = ALLOC(SegmentInfos);
    int doc_cnt;
    int seg_count;
    int i;
    char *name;
    InStream *is = store->open_input(store, INDEX_FILENAME);
    sis->store = store;

    TRY
        sis->format = is_read_uint(is); /* do nothing. it's the first version */
        sis->version = is_read_ulong(is);
        sis->counter = is_read_ulong(is);
        seg_count = is_read_int(is);

        /* allocate space for segments */
        for (sis->capa = 4; sis->capa < seg_count; sis->capa <<= 1) {
        }
        sis->size = 0;
        sis->segs = ALLOC_N(SegmentInfo *, sis->capa);

        for (i = 0; i < seg_count; i++) {
            name = is_read_string(is);
            doc_cnt = is_read_int(is);
            sis_add_si(sis, si_create(name, doc_cnt, store));
        }
        sis->fis = fis_read(is);
    XFINALLY
        is_close(is);
    XENDTRY

    return sis;
}

void sis_write(SegmentInfos *sis, Store *store)
{
    int i;
    SegmentInfo *si;
    OutStream *os = store->create_output(store, TEMPORARY_INDEX_FILENAME);
    TRY
        os_write_uint(os, FORMAT);
        os_write_ulong(os, ++(sis->version)); /* every write changes the index */
        os_write_ulong(os, sis->counter);
        os_write_int(os, sis->size); 
        for (i = 0; i < sis->size; i++) {
            si = sis->segs[i];
            os_write_string(os, si->name);
            os_write_int(os, si->doc_cnt);
        }
        fis_write(sis->fis, os);

    XFINALLY
        os_close(os);
    XENDTRY

    /* install new segment info */
    store->rename(store, TEMPORARY_INDEX_FILENAME, INDEX_FILENAME);
}

f_u64 sis_read_current_version(Store *store)
{
    InStream *is;
    int format = 0;
    f_u64 version = 0;

    if (!store->exists(store, INDEX_FILENAME)) {
        return 0;
    }
    is = store->open_input(store, INDEX_FILENAME);

    TRY
        format = is_read_uint(is);
        version = is_read_ulong(is);
    XFINALLY
        is_close(is);
    XENDTRY

    return version;
}

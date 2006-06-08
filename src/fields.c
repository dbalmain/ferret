#include "fields.h"
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
            self->props |= FI_IS_STORED_BM;
            break;
        case STORE_COMPRESS:
            self->props |= FI_IS_COMPRESSED_BM;
            self->props |= FI_IS_STORED_BM;
            break;
    }
}

inline void fi_set_index(FieldInfo *self, int index)
{
    switch (index) {
        case INDEX_NO:
            break;
        case INDEX_YES:
            self->props |= FI_IS_INDEXED_BM;
            self->props |= FI_IS_TOKENIZED_BM;
            break;
        case INDEX_UNTOKENIZED:
            self->props |= FI_IS_INDEXED_BM;
            break;
        case INDEX_YES_OMIT_NORMS:
            self->props |= FI_OMIT_NORMS_BM;
            self->props |= FI_IS_INDEXED_BM;
            self->props |= FI_IS_TOKENIZED_BM;
            break;
        case INDEX_UNTOKENIZED_OMIT_NORMS:
            self->props |= FI_OMIT_NORMS_BM;
            self->props |= FI_IS_INDEXED_BM;
            break;
    }
}

inline void fi_set_term_vector(FieldInfo *self, int term_vector)
{
    switch (term_vector) {
        case TERM_VECTOR_NO:
            break;
        case TERM_VECTOR_YES:
            self->props |= FI_STORE_TERM_VECTOR_BM;
            break;
        case TERM_VECTOR_WITH_POSITIONS:
            self->props |= FI_STORE_TERM_VECTOR_BM;
            self->props |= FI_STORE_POSITIONS_BM;
            break;
        case TERM_VECTOR_WITH_OFFSETS:
            self->props |= FI_STORE_TERM_VECTOR_BM;
            self->props |= FI_STORE_OFFSETS_BM;
            break;
        case TERM_VECTOR_WITH_POSITIONS_OFFSETS:
            self->props |= FI_STORE_TERM_VECTOR_BM;
            self->props |= FI_STORE_POSITIONS_BM;
            self->props |= FI_STORE_OFFSETS_BM;
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
    self->props = 0;
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

/*****
 *
 * FieldInfos
 *
 *****/

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

int fis_add_field(FieldInfos *self, FieldInfo *fi)
{
    if (self->size == self->capa) {
        self->capa <<= 1;
        REALLOC_N(self->fields, FieldInfo *, self->capa);
    }
    if (!h_set_safe(self->field_dict, fi->name, fi)) {
        return false;
    }
    fi->number = self->size;
    self->fields[self->size] = fi;
    self->size++;
    return true;
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
    union {
        f_u32 i;
        float f;
    } tmp;
    FieldInfo *fi;
    os_write_long(os, FIELD_INFOS_VERSION);
    os_write_uint(os, self->store);
    os_write_uint(os, self->index);
    os_write_uint(os, self->term_vector);
    os_write_uint(os, self->size);
    for (i = 0; i < self->size; i++) {
        fi = self->fields[i];
        os_write_string(os, fi->name);
        tmp.f = fi->boost;
        os_write_uint(os, POSH_LittleU32(tmp.i));
        os_write_uint(os, fi->props);
    }
}

FieldInfos *fis_read(InStream *is)
{
    int store, index, term_vector;
    int i;
    union {
        f_u32 i;
        float f;
    } tmp;
    FieldInfo *fi;
    FieldInfos *self;
    is_read_long(is); /* ignore as we are only at the first version */
    store = is_read_uint(is);
    index = is_read_uint(is);
    term_vector = is_read_uint(is);
    self = fis_create(store, index, term_vector);
    for (i = is_read_uint(is); i > 0; i--) {
        fi = ALLOC(FieldInfo);
        fi->name = is_read_string(is);
        tmp.i = POSH_LittleU32(is_read_uint(is));
        fi->boost = tmp.f;
        fi->props = is_read_uint(is);
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
    return store_str[fi->props & 0x3];
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
    return index_str[(fi->props >> 2) & 0x7];
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
    return term_vector_str[(fi->props >> 2) & 0x7];
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

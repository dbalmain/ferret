#include <string.h>
#include "field_index.h"
#include "internal.h"

/***************************************************************************
 *
 * FieldIndex
 *
 ***************************************************************************/

static unsigned long field_index_hash(const void *p)
{
    FieldIndex *self = (FieldIndex *)p;
    return sym_hash(self->field) ^ (unsigned long)(self->klass);
}

static int field_index_eq(const void *p1, const void *p2)
{
    FieldIndex *fi1 = (FieldIndex *)p1;
    FieldIndex *fi2 = (FieldIndex *)p2;
    return (fi1->field == fi2->field) &&
        (fi1->klass->type == fi2->klass->type);
}

static void field_index_destroy(void *p)
{
    FieldIndex *self = (FieldIndex *)p;
    if (self->index) {
        self->klass->destroy_index(self->index);
    }
    free(self);
}

FieldIndex *field_index_get(IndexReader *ir, Symbol field,
                            const FieldIndexClass *klass)
{
    int length = 0;
    TermEnum *volatile te = NULL;
    TermDocEnum *volatile tde = NULL;
    FieldInfo *fi = fis_get_field(ir->fis, field);
    const volatile int field_num = fi ? fi->number : -1;
    FieldIndex *volatile self = NULL;
    FieldIndex key;

    if (field_num < 0) {
        RAISE(ARG_ERROR,
              "Cannot sort by field \"%s\". It doesn't exist in the index.",
              S(field));
    }

    if (!ir->field_index_cache) {
        ir->field_index_cache = h_new(&field_index_hash, &field_index_eq,
                                      NULL, &field_index_destroy);
    }

    key.field = field;
    key.klass = klass;
    self = (FieldIndex *)h_get(ir->field_index_cache, &key);

    if (self == NULL) {
        self = ALLOC(FieldIndex);
        self->klass = klass;
        /* FieldIndex only lives as long as the IndexReader lives so we can
         * just use the field_infos field string */
        self->field = fi->name;

        length = ir->max_doc(ir);
        if (length > 0) {
            TRY
            {
                void *index;
                tde = ir->term_docs(ir);
                te = ir->terms(ir, field_num);
                index = self->index = klass->create_index(length);
                while (te->next(te)) {
                    tde->seek_te(tde, te);
                    klass->handle_term(index, tde, te->curr_term);
                }
            }
            XFINALLY
                tde->close(tde);
                te->close(te);
            XENDTRY
        }
        h_set(ir->field_index_cache, self, self);
    }

    return self;
}

/******************************************************************************
 * ByteFieldIndex < FieldIndex
 *
 * The ByteFieldIndex holds an array of integers for each document in the
 * index where the integer represents the sort value for the document.  This
 * index should only be used for sorting and not as a field cache of the
 * column's value.
 ******************************************************************************/
static void byte_handle_term(void *index_ptr,
                             TermDocEnum *tde,
                             const char *text)
{
    long *index = (long *)index_ptr;
    long val = index[-1]++;
    (void)text;
    while (tde->next(tde)) {
        index[tde->doc_num(tde)] = val;
    }
}

static void *byte_create_index(int size)
{
    long *index = ALLOC_AND_ZERO_N(long, size + 1);
    index[0] = 1;
    return &index[1];
}

static void byte_destroy_index(void *p)
{
    long *index = (long *)p;
    free(&index[-1]);
}

const FieldIndexClass BYTE_FIELD_INDEX_CLASS = {
    "byte",
    &byte_create_index,
    &byte_destroy_index,
    &byte_handle_term
};

/******************************************************************************
 * IntegerFieldIndex < FieldIndex
 ******************************************************************************/
static void *integer_create_index(int size)
{
    return ALLOC_AND_ZERO_N(long, size);
}

static void integer_handle_term(void *index_ptr,
                                TermDocEnum *tde,
                                const char *text)
{
    long *index = (long *)index_ptr;
    long val;
    sscanf(text, "%ld", &val);
    while (tde->next(tde)) {
        index[tde->doc_num(tde)] = val;
    }
}

const FieldIndexClass INTEGER_FIELD_INDEX_CLASS = {
    "integer",
    &integer_create_index,
    &free,
    &integer_handle_term
};

long get_integer_value(FieldIndex *field_index, long doc_num)
{
    if (field_index->klass == &INTEGER_FIELD_INDEX_CLASS && doc_num >= 0) {
        return ((long *)field_index->index)[doc_num];
    }
    return 0l;
}


/******************************************************************************
 * FloatFieldIndex < FieldIndex
 ******************************************************************************/
#define VALUES_ARRAY_START_SIZE 8
static void *float_create_index(int size)
{
    return ALLOC_AND_ZERO_N(float, size);
}

static void float_handle_term(void *index_ptr,
                              TermDocEnum *tde,
                              const char *text)
{
    float *index = (float *)index_ptr;
    float val;
    sscanf(text, "%g", &val);
    while (tde->next(tde)) {
        index[tde->doc_num(tde)] = val;
    }
}

const FieldIndexClass FLOAT_FIELD_INDEX_CLASS = {
    "float",
    &float_create_index,
    &free,
    &float_handle_term
};

float get_float_value(FieldIndex *field_index, long doc_num)
{
    if (field_index->klass == &FLOAT_FIELD_INDEX_CLASS && doc_num >= 0) {
        return ((float *)field_index->index)[doc_num];
    }
    return 0.0f;
}

/******************************************************************************
 * StringFieldIndex < FieldIndex
 ******************************************************************************/
 
static void *string_create_index(int size)
{
    StringIndex *self = ALLOC_AND_ZERO(StringIndex);
    self->size = size;
    self->index = ALLOC_AND_ZERO_N(long, size);
    self->v_capa = VALUES_ARRAY_START_SIZE;
    self->v_size = 1; /* leave the first value as NULL */
    self->values = ALLOC_AND_ZERO_N(char *, VALUES_ARRAY_START_SIZE);
    return self;
}

static void string_destroy_index(void *p)
{
    StringIndex *self = (StringIndex *)p;
    int i;
    free(self->index);
    for (i = 0; i < self->v_size; i++) {
        free(self->values[i]);
    }
    free(self->values);
    free(self);
}

static void string_handle_term(void *index_ptr,
                               TermDocEnum *tde,
                               const char *text)
{
    StringIndex *index = (StringIndex *)index_ptr;
    if (index->v_size >= index->v_capa) {
        index->v_capa *= 2;
        index->values = REALLOC_N(index->values, char *, index->v_capa);
    }
    index->values[index->v_size] = estrdup(text);
    while (tde->next(tde)) {
        index->index[tde->doc_num(tde)] = index->v_size;
    }
    index->v_size++;
}

const FieldIndexClass STRING_FIELD_INDEX_CLASS = {
    "string",
    &string_create_index,
    &string_destroy_index,
    &string_handle_term
};

const char *get_string_value(FieldIndex *field_index, long doc_num)
{
    if (field_index->klass == &STRING_FIELD_INDEX_CLASS) {
        StringIndex *string_index = (StringIndex *)field_index->index;
        if (doc_num >= 0 && doc_num < string_index->size) {
            return string_index->values[string_index->index[doc_num]];
        }
    }
    return NULL;
}

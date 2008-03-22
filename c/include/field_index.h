#ifndef FRT_FIELD_INDEX_H
#define FRT_FIELD_INDEX_H

#include "index.h"

/***************************************************************************
 *
 * FieldIndex
 *
 ***************************************************************************/

typedef struct StringIndex {
    int size;
    long *index;
    char **values;
    int v_size;
    int v_capa;
} StringIndex;

typedef struct FieldIndexClass {
    const char *type;
    void *(*create_index)(int size);
    void  (*destroy_index)(void *p);
    void  (*handle_term)(void *index, TermDocEnum *tde, const char *text);
} FieldIndexClass;

typedef struct FieldIndex {
    const char *field;
    const FieldIndexClass *klass;
    void *index;
} FieldIndex;

extern const FieldIndexClass INTEGER_FIELD_INDEX_CLASS;
extern const FieldIndexClass   FLOAT_FIELD_INDEX_CLASS;
extern const FieldIndexClass  STRING_FIELD_INDEX_CLASS;
extern const FieldIndexClass    BYTE_FIELD_INDEX_CLASS;

extern FieldIndex *field_index_new(IndexReader *ir, const char *field,
                                   const FieldIndexClass *klass);
extern const char *get_string_value(FieldIndex *field_index, long doc_num);
extern float get_float_value(FieldIndex *field_index, long doc_num);
extern long get_integer_value(FieldIndex *field_index, long doc_num);
#endif

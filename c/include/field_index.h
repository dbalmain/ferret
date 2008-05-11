#ifndef FRT_FIELD_INDEX_H
#define FRT_FIELD_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "index.h"

/***************************************************************************
 *
 * FrtFieldIndex
 *
 ***************************************************************************/

typedef struct FrtStringIndex {
    int size;
    long *index;
    char **values;
    int v_size;
    int v_capa;
} FrtStringIndex;

typedef struct FrtFieldIndexClass {
    const char *type;
    void *(*create_index)(int size);
    void  (*destroy_index)(void *p);
    void  (*handle_term)(void *index, FrtTermDocEnum *tde, const char *text);
} FrtFieldIndexClass;

typedef struct FrtFieldIndex {
    FrtSymbol field;
    const FrtFieldIndexClass *klass;
    void *index;
} FrtFieldIndex;

extern const FrtFieldIndexClass FRT_INTEGER_FIELD_INDEX_CLASS;
extern const FrtFieldIndexClass   FRT_FLOAT_FIELD_INDEX_CLASS;
extern const FrtFieldIndexClass  FRT_STRING_FIELD_INDEX_CLASS;
extern const FrtFieldIndexClass    FRT_BYTE_FIELD_INDEX_CLASS;

extern FrtFieldIndex *frt_field_index_get(FrtIndexReader *ir, FrtSymbol field,
                                   const FrtFieldIndexClass *klass);
extern const char *frt_get_string_value(FrtFieldIndex *field_index, long doc_num);
extern float frt_get_float_value(FrtFieldIndex *field_index, long doc_num);
extern long frt_get_integer_value(FrtFieldIndex *field_index, long doc_num);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

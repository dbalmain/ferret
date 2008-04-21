#ifndef FRT_DOCUMENT_H
#define FRT_DOCUMENT_H

#include "global.h"
#include "hash.h"

/****************************************************************************
 *
 * FrtDocField
 *
 ****************************************************************************/

#define FRT_DF_INIT_CAPA 1
typedef struct FrtDocField
{
    const char *name;
    int size;
    int capa;
    int *lengths;
    char **data;
    float boost;
    bool destroy_data : 1;
    bool is_compressed : 1;
} FrtDocField;

extern FrtDocField *frt_df_new(const char *name);
extern FrtDocField *frt_df_add_data(FrtDocField *df, char *data);
extern FrtDocField *frt_df_add_data_len(FrtDocField *df, char *data, int len);
extern void frt_df_destroy(FrtDocField *df);
extern char *frt_df_to_s(FrtDocField *df);

/****************************************************************************
 *
 * FrtDocument
 *
 ****************************************************************************/

#define FRT_DOC_INIT_CAPA 8
typedef struct FrtDocument
{
    FrtHash *field_dict;
    int size;
    int capa;
    FrtDocField **fields;
    float boost;
} FrtDocument;

extern FrtDocument *frt_doc_new();
extern FrtDocField *frt_doc_add_field(FrtDocument *doc, FrtDocField *df);
extern FrtDocField *frt_doc_get_field(FrtDocument *doc, const char *fname);
extern char *frt_doc_to_s(FrtDocument *doc);
extern void frt_doc_destroy(FrtDocument *doc);

#endif

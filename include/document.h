#ifndef FRT_DOCUMENT_H
#define FRT_DOCUMENT_H

#include "global.h"
#include "hash.h"

/****************************************************************************
 *
 * DocField
 *
 ****************************************************************************/

#define DF_INIT_CAPA 1
typedef struct DocField
{
    char *name;
    int size;
    int capa;
    int *lengths;
    char **data;
    float boost;
    bool destroy_data : 1;
} DocField;

DocField *df_create(const char *name, char *data);
DocField *df_create_len(const char *name, char *data, int len);
void df_add_data(DocField *df, char *data);
void df_add_data_len(DocField *df, char *data, int len);
void df_destroy(DocField *df);
char *df_to_s(DocField *df);

/****************************************************************************
 *
 * Document
 *
 ****************************************************************************/

#define DOC_INIT_CAPA 8
typedef struct Document
{
    HashTable *field_dict;
    int size;
    int capa;
    DocField **fields;
    float boost;
} Document;

Document *doc_create();
void doc_add_field(Document *doc, DocField *df);
DocField *doc_get_field(Document *doc, const char *fname);
char *doc_to_s(Document *doc);
void doc_destroy(Document *doc);

#endif

#include "document.h"
#include "symbol.h"
#include <string.h>
#include "internal.h"

/****************************************************************************
 *
 * DocField
 *
 ****************************************************************************/

DocField *df_new(Symbol name)
{
    DocField *df = ALLOC(DocField);
    df->name = name;
    df->size = 0;
    df->capa = DF_INIT_CAPA;
    df->data = ALLOC_N(char *, df->capa);
    df->lengths = ALLOC_N(int, df->capa);
    df->destroy_data = false;
    df->boost = 1.0;
    return df;
}

DocField *df_add_data_len(DocField *df, char *data, int len)
{
    if (df->size >= df->capa) {
        df->capa <<= 2;
        REALLOC_N(df->data, char *, df->capa);
        REALLOC_N(df->lengths, int, df->capa);
    }
    df->data[df->size] = data;
    df->lengths[df->size] = len;
    df->size++;
    return df;
}

DocField *df_add_data(DocField *df, char *data)
{
    return df_add_data_len(df, data, strlen(data));
}

void df_destroy(DocField *df)
{
    if (df->destroy_data) {
        int i;
        for (i = 0; i < df->size; i++) {
            free(df->data[i]);
        }
    }
    free(df->data);
    free(df->lengths);
    free(df);
}

/*
 * Format for one item is: name: "data"
 *        for more items : name: ["data", "data", "data"]
 */
char *df_to_s(DocField *df)
{
#define APPEND(dst, src) ((dst)[0] = (src)[0], 1)
#define APPEND2(dst, src) (APPEND(dst, src), APPEND(dst+1, src+1), 2)

    int i, len = 0, namelen = sym_len(df->name);
    char *str, *s;
    for (i = 0; i < df->size; i++) {
        len += df->lengths[i] + 4;
    }
    s = str = ALLOC_N(char, namelen + len + 5);
    memcpy(s, df->name, namelen);
    s += namelen;
    s += APPEND2(s, ": ");

    if (df->size > 1) {
        s += APPEND(s, "[");
    }
    for (i = 0; i < df->size; i++) {
        if (i != 0) {
            s += APPEND2(s, ", ");
        }
        s += APPEND(s, "\"");
        memcpy(s, df->data[i], df->lengths[i]);
        s += df->lengths[i];
        s += APPEND(s, "\"");
    }

    if (df->size > 1) {
        s += APPEND(s, "]");
    }
    *s = 0;
    return str;
}

/****************************************************************************
 *
 * Document
 *
 ****************************************************************************/

Document *doc_new()
{
    Document *doc = ALLOC(Document);
    doc->field_dict = h_new_ptr((free_ft)&df_destroy);
    doc->size = 0;
    doc->capa = DOC_INIT_CAPA;
    doc->fields = ALLOC_N(DocField *, doc->capa);
    doc->boost = 1.0;
    return doc;
}

DocField *doc_add_field(Document *doc, DocField *df)
{
    if (!h_set_safe(doc->field_dict, df->name, df)) {
        RAISE(EXCEPTION, "tried to add %s field which alread existed\n",
              S(df->name));
    }
    if (doc->size >= doc->capa) {
        doc->capa <<= 1;
        REALLOC_N(doc->fields, DocField *, doc->capa);
    }
    doc->fields[doc->size] = df;
    doc->size++;
    return df;
}

DocField *doc_get_field(Document *doc, Symbol name)
{
    return (DocField *)h_get(doc->field_dict, name);
}

char *doc_to_s(Document *doc)
{
    int i;
    int len = 0;
    char **fields = ALLOC_N(char *, doc->size);
    char *buf, *s;

    for (i = 0; i < doc->size; i++) {
        fields[i] = df_to_s(doc->fields[i]);
        len += strlen(fields[i]) + 5;
    }
    s = buf = ALLOC_N(char, len + 12);
    s += sprintf(buf, "Document [\n");
    for (i = 0; i < doc->size; i++) {
        s += sprintf(s, "  =>%s\n", fields[i]);
        free(fields[i]);
    }
    free(fields);
    return buf;
}

void doc_destroy(Document *doc)
{
    h_destroy(doc->field_dict);
    free(doc->fields);
    free(doc);
}


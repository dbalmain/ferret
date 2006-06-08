#ifndef FRT_FIELDS_H
#define FRT_FIELDS_H

#include "global.h"
#include "hash.h"
#include "store.h"

enum StoreValues {
    STORE_NO = 0,    
    STORE_YES = 1,    
    STORE_COMPRESS = 2
};

enum IndexValues {
    INDEX_NO = 0,    
    INDEX_YES = 1,    
    INDEX_UNTOKENIZED = 3,
    INDEX_YES_OMIT_NORMS = 5,
    INDEX_UNTOKENIZED_OMIT_NORMS = 7
};

enum TermVectorValues {
    TERM_VECTOR_NO = 0,    
    TERM_VECTOR_YES = 1,    
    TERM_VECTOR_WITH_POSITIONS = 3,
    TERM_VECTOR_WITH_OFFSETS = 5,
    TERM_VECTOR_WITH_POSITIONS_OFFSETS = 7
};

#define FI_IS_STORED_BM         0x001
#define FI_IS_COMPRESSED_BM     0x002
#define FI_IS_INDEXED_BM        0x004
#define FI_IS_TOKENIZED_BM      0x008
#define FI_OMIT_NORMS_BM        0x010
#define FI_STORE_TERM_VECTOR_BM 0x020
#define FI_STORE_POSITIONS_BM   0x040
#define FI_STORE_OFFSETS_BM     0x080

/*****
 *
 * FieldInfo
 *
 *****/

typedef struct FieldInfo {
    char *name;
    float boost;
    int number;
    unsigned int props;
} FieldInfo;

extern FieldInfo *fi_create(char *name,
                            int store,
                            int index,
                            int term_vector);
extern char *fi_to_s(FieldInfo *self);
extern void fi_destroy(FieldInfo *self);
#define fi_is_stored(fi)         (((fi)->props & FI_IS_STORED_BM) != 0)
#define fi_is_compressed(fi)     (((fi)->props & FI_IS_COMPRESSED_BM) != 0)
#define fi_is_indexed(fi)        (((fi)->props & FI_IS_INDEXED_BM) != 0)
#define fi_is_tokenized(fi)      (((fi)->props & FI_IS_TOKENIZED_BM) != 0)
#define fi_omit_norms(fi)        (((fi)->props & FI_OMIT_NORMS_BM) != 0)
#define fi_store_term_vector(fi) (((fi)->props & FI_STORE_TERM_VECTOR_BM) != 0)
#define fi_store_positions(fi)   (((fi)->props & FI_STORE_POSITIONS_BM) != 0)
#define fi_store_offsets(fi)     (((fi)->props & FI_STORE_OFFSETS_BM) != 0)

/*****
 *
 * FieldInfos
 *
 *****/

#define FIELD_INFOS_VERSION 0
#define FIELD_INFOS_INIT_CAPA 4
typedef struct FieldInfos {
    int store;
    int index;
    int term_vector;
    HashTable *field_dict;
    int size;
    int capa;
    FieldInfo **fields;
} FieldInfos;

extern FieldInfos *fis_create(int store, int index, int term_vector);
extern int fis_add_field(FieldInfos *self, FieldInfo *fi);
extern FieldInfo *fis_get_field(FieldInfos *self, char *name);
extern FieldInfo *fis_get_or_add_field(FieldInfos *self, char *name);
extern void fis_write(FieldInfos *fis, OutStream *os);
extern FieldInfos *fis_read(InStream *is);
extern char *fis_to_s(FieldInfos *self);
extern void fis_destroy(FieldInfos *self);

#endif

#ifndef FRT_INDEX_H
#define FRT_INDEX_H

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

/****************************************************************************
 *
 * FieldInfo
 *
 ****************************************************************************/

typedef struct FieldInfo {
    char *name;
    float boost;
    int number;
    unsigned int bits;
} FieldInfo;

extern FieldInfo *fi_create(char *name,
                            int store,
                            int index,
                            int term_vector);
extern char *fi_to_s(FieldInfo *self);
extern void fi_destroy(FieldInfo *self);
#define fi_is_stored(fi)         (((fi)->bits & FI_IS_STORED_BM) != 0)
#define fi_is_compressed(fi)     (((fi)->bits & FI_IS_COMPRESSED_BM) != 0)
#define fi_is_indexed(fi)        (((fi)->bits & FI_IS_INDEXED_BM) != 0)
#define fi_is_tokenized(fi)      (((fi)->bits & FI_IS_TOKENIZED_BM) != 0)
#define fi_omit_norms(fi)        (((fi)->bits & FI_OMIT_NORMS_BM) != 0)
#define fi_store_term_vector(fi) (((fi)->bits & FI_STORE_TERM_VECTOR_BM) != 0)
#define fi_store_positions(fi)   (((fi)->bits & FI_STORE_POSITIONS_BM) != 0)
#define fi_store_offsets(fi)     (((fi)->bits & FI_STORE_OFFSETS_BM) != 0)

/****************************************************************************
 *
 * FieldInfos
 *
 ****************************************************************************/

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
extern FieldInfo *fis_add_field(FieldInfos *self, FieldInfo *fi);
extern FieldInfo *fis_get_field(FieldInfos *self, char *name);
extern FieldInfo *fis_get_or_add_field(FieldInfos *self, char *name);
extern void fis_write(FieldInfos *fis, OutStream *os);
extern FieldInfos *fis_read(InStream *is);
extern char *fis_to_s(FieldInfos *self);
extern void fis_destroy(FieldInfos *self);

/****************************************************************************
 *
 * SegmentInfo
 *
 ****************************************************************************/

typedef struct SegmentInfo
{
    char *name;
    int doc_cnt;
    Store *store;
} SegmentInfo;

extern SegmentInfo *si_create(char *name, int doc_cnt, Store *store);
extern void si_destroy(SegmentInfo *si);
extern bool si_has_deletions(SegmentInfo *si);
extern bool si_uses_compound_file(SegmentInfo *si);
extern bool si_has_separate_norms(SegmentInfo *si);

/****************************************************************************
 *
 * SegmentInfos
 *
 ****************************************************************************/

typedef struct SegmentInfos
{
    f_u32 format;
    Store *store;
    SegmentInfo **segs;
    int size;
    int capa;
    f_u64 counter;
    f_u64 version;
    FieldInfos *fis;
} SegmentInfos;

extern SegmentInfos *sis_create(FieldInfos *fis);
extern SegmentInfo *sis_new_segment(SegmentInfos *sis, int dcnt, Store *store);
extern SegmentInfo *sis_add_si(SegmentInfos *sis, SegmentInfo *si);
extern void sis_del_at(SegmentInfos *sis, int at);
extern void sis_del_from_to(SegmentInfos *sis, int from, int to);
extern void sis_clear(SegmentInfos *sis);
extern SegmentInfos *sis_read(Store *store);
extern void sis_write(SegmentInfos *sis, Store *store);
extern int sis_read_current_version(Store *store);
extern void sis_destroy(SegmentInfos *sis);

#endif

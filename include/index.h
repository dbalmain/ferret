#ifndef FRT_INDEX_H
#define FRT_INDEX_H

#include "global.h"
#include "document.h"
#include "analysis.h"
#include "hash.h"
#include "store.h"
#include "mem_pool.h"

enum StoreValues
{
    STORE_NO = 0,    
    STORE_YES = 1,    
    STORE_COMPRESS = 2
};

enum IndexValues
{
    INDEX_NO = 0,    
    INDEX_YES = 1,    
    INDEX_UNTOKENIZED = 3,
    INDEX_YES_OMIT_NORMS = 5,
    INDEX_UNTOKENIZED_OMIT_NORMS = 7
};

enum TermVectorValues
{
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

typedef struct FieldInfo
{
    char *name;
    float boost;
    int number;
    unsigned int bits;
} FieldInfo;

extern FieldInfo *fi_new(char *name,
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
typedef struct FieldInfos
{
    int store;
    int index;
    int term_vector;
    int size;
    int capa;
    FieldInfo **fields;
    HashTable *field_dict;
} FieldInfos;

extern FieldInfos *fis_new(int store, int index, int term_vector);
extern FieldInfo *fis_add_field(FieldInfos *self, FieldInfo *fi);
extern FieldInfo *fis_get_field(FieldInfos *self, char *name);
extern FieldInfo *fis_get_or_add_field(FieldInfos *self, char *name);
extern void fis_write(FieldInfos *fis, Store *store);
extern FieldInfos *fis_read(Store *store);
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

extern SegmentInfo *si_new(char *name, int doc_cnt, Store *store);
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
    f_u64 counter;
    f_u64 version;
    f_u32 format;
    Store *store;
    SegmentInfo **segs;
    int size;
    int capa;
} SegmentInfos;

extern SegmentInfos *sis_new();
extern SegmentInfo *sis_new_segment(SegmentInfos *sis, int dcnt, Store *store);
extern SegmentInfo *sis_add_si(SegmentInfos *sis, SegmentInfo *si);
extern void sis_del_at(SegmentInfos *sis, int at);
extern void sis_del_from_to(SegmentInfos *sis, int from, int to);
extern void sis_clear(SegmentInfos *sis);
extern SegmentInfos *sis_read(Store *store);
extern void sis_write(SegmentInfos *sis, Store *store);
extern f_u64 sis_read_current_version(Store *store);
extern void sis_destroy(SegmentInfos *sis);

/****************************************************************************
 *
 * FieldsReader
 *
 ****************************************************************************/

typedef struct FieldsReader
{
  int len;
  FieldInfos *fis;
  InStream *fdt_in;
  InStream *fdx_in;
} FieldsReader;

extern FieldsReader *fr_open(Store *store, char *seg_name, FieldInfos *fis);
extern void fr_close(FieldsReader *fr);
extern Document *fr_get_doc(FieldsReader *fr, int doc_num);

/****************************************************************************
 *
 * FieldsWriter
 *
 ****************************************************************************/

typedef struct FieldsWriter
{
  FieldInfos *fis;
  OutStream *fdt_out;
  OutStream *fdx_out;
} FieldsWriter;

extern FieldsWriter *fw_open(Store *store, char *seg_name, FieldInfos *fis);
extern void fw_close(FieldsWriter *fw);
extern void fw_add_doc(FieldsWriter *fw, Document *doc);

/****************************************************************************
 *
 * TermInfo
 *
 ****************************************************************************/

typedef struct TermInfo
{
    int doc_freq;
    long freq_pointer;
    long prox_pointer;
    long skip_offset;
} TermInfo;

#define ti_set(ti, mdf, mfp, mpp, mso) do {\
    ti->doc_freq = mdf;\
    ti->freq_pointer = mfp;\
    ti->prox_pointer = mpp;\
    ti->skip_offset = mso;\
} while (0)

/****************************************************************************
 *
 * TermEnum
 *
 ****************************************************************************/

typedef struct TermEnum TermEnum;

struct TermEnum
{
    char        curr_term[MAX_WORD_SIZE];
    char        prev_term[MAX_WORD_SIZE];
    TermInfo    curr_ti;
    int         curr_term_len;
    char     *(*next)(TermEnum *te);
    TermEnum *(*set_field)(TermEnum *te, int field_num);
    char     *(*skip_to)(TermEnum *te, const char *term);
    void      (*close)(TermEnum *te);
    TermEnum *(*clone)(TermEnum *te);
};

char *te_get_term(struct TermEnum *te);
TermInfo *te_get_ti(struct TermEnum *te);

/****************************************************************************
 *
 * SegmentTermEnum
 *
 ****************************************************************************/

/* * SegmentFieldIndex * */

typedef struct SegmentTermIndex
{
    long        index_pointer;
    long        pointer;
    int         index_size;
    int         size;
    char      **index_terms;
    int        *index_term_lens;
    TermInfo   *index_term_infos;
    long       *index_pointers;
} SegmentTermIndex;

/* * SegmentFieldIndex * */

typedef struct SegmentTermEnum SegmentTermEnum;

typedef struct SegmentFieldIndex
{
    mutex_t          mutex;
    int              skip_interval;
    int              index_interval;
    long             index_pointer;
    SegmentTermEnum *index_ste;
    HashTable       *field_dict;
} SegmentFieldIndex;

extern SegmentFieldIndex *sfi_open(Store *store, char *seg_name);
extern void sfi_close(SegmentFieldIndex *sfi);


/* * SegmentTermEnum * */
struct SegmentTermEnum
{
    TermEnum te;
    InStream *is;
    int size;
    int pos;
    int skip_interval;
    int field_num;
    SegmentFieldIndex *sfi;
};

extern void ste_close(SegmentTermEnum *ste);
extern SegmentTermEnum *ste_new(InStream *is, SegmentFieldIndex *sfi);

/* * MultiTermEnum * */
/*
typedef struct MultiTermEnum
{
    TermEnum te;
    int doc_freq;
    PriorityQueue *smi_queue;
} MultiTermEnum;

TermEnum *mte_new(IndexReader **readers, int *starts, int rcnt, Term *term);
*/

/****************************************************************************
 *
 * TermIndexReader
 *
 ****************************************************************************/

#define TE_BUCKET_INIT_CAPA 1

typedef struct TermIndexReader
{
    thread_key_t        thread_ste;
    SegmentTermEnum   **ste_bucket;
    int                 ste_bucket_size;
    int                 ste_bucket_capa;
    SegmentTermEnum    *orig_te;
} TermIndexReader;

extern TermIndexReader *tir_open(Store *store,
                                 SegmentFieldIndex *sfi,
                                 char *seg_name);
extern void tir_set_field(TermIndexReader *tir, int field_num);
extern TermInfo *tir_get_ti(TermIndexReader *tir, const char *term);
extern char *tir_get_term(TermIndexReader *tir, int pos);
extern void tir_close(TermIndexReader *tir);

/****************************************************************************
 *
 * TermIndexWriter
 *
 ****************************************************************************/

#define SKIP_INTERVAL 16

typedef struct TermWriter
{
    int counter;
    const char *last_term;
    TermInfo last_term_info;
    OutStream *os;
} TermWriter;

typedef struct TermIndexWriter
{
    int field_count;
    int index_interval;
    int skip_interval;
    long last_index_pointer;
    OutStream *tfx_out;
    TermWriter *tix_writer;
    TermWriter *tis_writer;
} TermIndexWriter;

extern TermIndexWriter *tiw_open(Store *store,
                                 const char *seg_name,
                                 int index_interval, 
                                 int skip_interval);
extern void tiw_start_field(TermIndexWriter *tiw, int field_num);
extern void tiw_add(TermIndexWriter *tiw,
                    const char *term,
                    int t_len,
                    TermInfo *ti);
extern void tiw_close(TermIndexWriter *tiw);

/****************************************************************************
 *
 * Offset
 *
 ****************************************************************************/

typedef struct Offset
{
    int start;
    int end;
} Offset;

/****************************************************************************
 *
 * Occurence
 *
 ****************************************************************************/

typedef struct Occurence
{
    struct Occurence *next;
    int position;
    Offset offset;
} Occurence;

/****************************************************************************
 *
 * OccurenceWithoutOffsets
 *
 ****************************************************************************/

typedef struct OccurenceWithoutOffsets
{
    struct Occurence *next;
    int position;
} OccurenceWithoutOffsets;

/****************************************************************************
 *
 * Posting
 *
 ****************************************************************************/

typedef struct Posting
{
    const char *term;
    int term_len;
    int freq;
    Occurence *first_occ;
    Occurence *last_occ;
    struct Posting *next;
} Posting;

extern Posting *p_new(MemoryPool *mp,
                      const char *term,
                      int term_len,
                      int position);
extern Posting *p_new_with_offsets(MemoryPool *mp,
                                   const char *term,
                                   int term_len,
                                   int position,
                                   int start,
                                   int end);
extern void p_add_occurence(MemoryPool *mp, Posting *p, int position);
extern void p_add_occurence_with_offsets(MemoryPool *mp,
                                         Posting *p,
                                         int position,
                                         int start_offset,
                                         int end_offset);

/****************************************************************************
 *
 * TVField
 *
 ****************************************************************************/

typedef struct TVField
{
    int field_num;
    int size;
} TVField;

/****************************************************************************
 *
 * Term
 *
 ****************************************************************************/

typedef struct Term
{
    char *text;
    int freq;
    int *positions;
    Offset *offsets;
} Term;

/****************************************************************************
 *
 * TermVector
 *
 ****************************************************************************/

typedef struct TermVector
{
    int field_num;
    int size;
    Term *terms;
} TermVector;

extern void tv_destroy(TermVector *tv);

/****************************************************************************
 *
 * TermVectorsWriter
 *
 ****************************************************************************/

#define TVX_EXTENSION ".tvx"
#define TVD_EXTENSION ".tvd"

#define TV_FIELD_INIT_CAPA 8

typedef struct TermVectorsWriter
{
    OutStream *tvx_out;
    OutStream *tvd_out;
    FieldInfos *fis;
    TVField *fields;
    int f_size;
    int f_capa;
    long tvd_pointer;
} TermVectorsWriter;

extern TermVectorsWriter *tvw_open(Store *store,
                                   const char *seg_name,
                                   FieldInfos *fis);
extern void tvw_open_doc(TermVectorsWriter *tvw);
extern void tvw_close_doc(TermVectorsWriter *tvw);
extern void tvw_add_postings(TermVectorsWriter *tvw,
                             int field_num,
                             Posting **postings,
                             int size);
extern void tvw_close(TermVectorsWriter *tvw);

/****************************************************************************
 *
 * TermVectorsReader
 *
 ****************************************************************************/

typedef struct TermVectorsReader
{
  int size;
  InStream *tvx_in;
  InStream *tvd_in;
  FieldInfos *fis;
} TermVectorsReader;

extern TermVectorsReader *tvr_open(Store *store,
                                   const char *seg_name,
                                   FieldInfos *fis);
extern TermVectorsReader *tvr_clone(TermVectorsReader *orig);
extern void tvr_close(TermVectorsReader *tvr);
extern HashTable *tvr_get_tv(TermVectorsReader *tvr, int doc_num);
extern TermVector *tvr_get_field_tv(TermVectorsReader *tvr,
                                    int doc_num,
                                    int field_num);

/****************************************************************************
 *
 * PostingList
 *
 ****************************************************************************/

typedef struct PostingList
{
    const char *term;
    Posting *first;
    Posting *last;
} PostingList;

/****************************************************************************
 *
 * DocInverter
 *
 ****************************************************************************/

typedef struct DocInverter
{
    MemoryPool *mp;
    Analyzer *analyzer;
    HashTable *postings;
    HashTable *field_postings;
} DocInverter;

extern HashTable *di_invert_field(DocInverter *di,
                                  DocField *df,
                                  FieldInfo *fi);

#endif

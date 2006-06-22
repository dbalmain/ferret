#ifndef FRT_INDEX_H
#define FRT_INDEX_H

#include "global.h"
#include "document.h"
#include "analysis.h"
#include "hash.h"
#include "hashset.h"
#include "store.h"
#include "mem_pool.h"
#include "similarity.h"
#include "bitvector.h"
#include "priorityqueue.h"

typedef struct IndexReader IndexReader;

/****************************************************************************
 *
 * Config
 *
 ****************************************************************************/

typedef struct Config
{
    int chunk_size;
    int max_buffer_memory;
    int index_interval;
    int skip_interval;
    int merge_factor;
    int max_buffered_docs;
    int max_merge_docs;
    int max_field_length;
    bool use_compound_file;
} Config;

extern const Config const default_config;

/***************************************************************************
 *
 * CacheObject
 *
 ***************************************************************************/

typedef struct CacheObject {
    HashTable *ref_tab1;
    HashTable *ref_tab2;
    void *ref1;
    void *ref2;
    void *obj;
    void (*destroy)(void *p);
} CacheObject;

extern void cache_destroy(CacheObject *co);
extern CacheObject *co_create(HashTable *ref_tab1, HashTable *ref_tab2,
            void *ref1, void *ref2, void (*destroy)(void *p), void *obj);
extern HashTable *co_hash_create();

/****************************************************************************
 *
 * FieldInfo
 *
 ****************************************************************************/

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

typedef struct FieldInfo
{
    char *name;
    float boost;
    int number;
    unsigned int bits;
} FieldInfo;

extern FieldInfo *fi_new(const char *name,
                         int store,
                         int index,
                         int term_vector);
extern char *fi_to_s(FieldInfo *fi);
extern void fi_destroy(FieldInfo *fi);
#define fi_is_stored(fi)         (((fi)->bits & FI_IS_STORED_BM) != 0)
#define fi_is_compressed(fi)     (((fi)->bits & FI_IS_COMPRESSED_BM) != 0)
#define fi_is_indexed(fi)        (((fi)->bits & FI_IS_INDEXED_BM) != 0)
#define fi_is_tokenized(fi)      (((fi)->bits & FI_IS_TOKENIZED_BM) != 0)
#define fi_omit_norms(fi)        (((fi)->bits & FI_OMIT_NORMS_BM) != 0)
#define fi_store_term_vector(fi) (((fi)->bits & FI_STORE_TERM_VECTOR_BM) != 0)
#define fi_store_positions(fi)   (((fi)->bits & FI_STORE_POSITIONS_BM) != 0)
#define fi_store_offsets(fi)     (((fi)->bits & FI_STORE_OFFSETS_BM) != 0)
#define fi_has_norms(fi)\
    (((fi)->bits & (FI_OMIT_NORMS_BM|FI_IS_INDEXED_BM)) == FI_IS_INDEXED_BM)

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
extern FieldInfo *fis_add_field(FieldInfos *fis, FieldInfo *fi);
extern FieldInfo *fis_get_field(FieldInfos *fis, const char *name);
extern FieldInfo *fis_get_or_add_field(FieldInfos *fis, const char *name);
extern void fis_write(FieldInfos *fis, Store *store);
extern FieldInfos *fis_read(Store *store);
extern char *fis_to_s(FieldInfos *fis);
extern void fis_destroy(FieldInfos *fis);

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
  int size;
  FieldInfos *fis;
  InStream *fdt_in;
  InStream *fdx_in;
} FieldsReader;

extern FieldsReader *fr_open(Store *store,
                             const char *segment, FieldInfos *fis);
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

extern FieldsWriter *fw_open(Store *store,
                             const char *segment, FieldInfos *fis);
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
    off_t frq_ptr;
    off_t prx_ptr;
    off_t skip_offset;
} TermInfo;

#define ti_set(ti, mdf, mfp, mpp, mso) do {\
    (ti).doc_freq = mdf;\
    (ti).frq_ptr = mfp;\
    (ti).prx_ptr = mpp;\
    (ti).skip_offset = mso;\
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
    TermEnum *(*set_field)(TermEnum *te, int field_num);
    char     *(*next)(TermEnum *te);
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

/* * SegmentTermIndex * */

typedef struct SegmentTermIndex
{
    off_t       index_ptr;
    off_t       ptr;
    int         index_size;
    int         size;
    char      **index_terms;
    int        *index_term_lens;
    TermInfo   *index_term_infos;
    off_t      *index_ptrs;
} SegmentTermIndex;

/* * SegmentFieldIndex * */

typedef struct SegmentTermEnum SegmentTermEnum;

typedef struct SegmentFieldIndex
{
    mutex_t     mutex;
    int         skip_interval;
    int         index_interval;
    off_t       index_ptr;
    TermEnum   *index_te;
    HashTable  *field_dict;
} SegmentFieldIndex;

extern SegmentFieldIndex *sfi_open(Store *store, const char *segment);
extern void sfi_close(SegmentFieldIndex *sfi);


/* * SegmentTermEnum * */
struct SegmentTermEnum
{
    TermEnum    te;
    InStream   *is;
    int         size;
    int         pos;
    int         skip_interval;
    int         field_num;
    SegmentFieldIndex *sfi;
};

extern void ste_close(TermEnum *te);
extern TermEnum *ste_clone(TermEnum *te);
extern TermEnum *ste_new(InStream *is, SegmentFieldIndex *sfi);

/* * MultiTermEnum * */
extern TermEnum *mte_new(IndexReader **readers, int *starts, int r_cnt,
                         int field_num, char *t);

/****************************************************************************
 *
 * TermInfosReader
 *
 ****************************************************************************/

#define TE_BUCKET_INIT_CAPA 1

typedef struct TermInfosReader
{
    thread_key_t thread_te;
    void       **te_bucket;
    TermEnum     *orig_te;
    int          field_num;
} TermInfosReader;

extern TermInfosReader *tir_open(Store *store,
                                 SegmentFieldIndex *sfi,
                                 const char *segment);
extern TermInfosReader *tir_set_field(TermInfosReader *tir, int field_num);
extern TermInfo *tir_get_ti(TermInfosReader *tir, const char *term);
extern char *tir_get_term(TermInfosReader *tir, int pos);
extern void tir_close(TermInfosReader *tir);

/****************************************************************************
 *
 * TermInfosWriter
 *
 ****************************************************************************/

#define INDEX_INTERVAL 128
#define SKIP_INTERVAL 16

typedef struct TermWriter
{
    int counter;
    const char *last_term;
    TermInfo last_term_info;
    OutStream *os;
} TermWriter;

typedef struct TermInfosWriter
{
    int field_count;
    int index_interval;
    int skip_interval;
    off_t last_index_ptr;
    OutStream *tfx_out;
    TermWriter *tix_writer;
    TermWriter *tis_writer;
} TermInfosWriter;

extern TermInfosWriter *tiw_open(Store *store,
                                 const char *segment,
                                 int index_interval, 
                                 int skip_interval);
extern void tiw_start_field(TermInfosWriter *tiw, int field_num);
extern void tiw_add(TermInfosWriter *tiw,
                    const char *term,
                    int t_len,
                    TermInfo *ti);
extern void tiw_close(TermInfosWriter *tiw);

/****************************************************************************
 *
 * TermDocEnum
 *
 ****************************************************************************/

typedef struct TermDocEnum TermDocEnum;
struct TermDocEnum
{
    void (*seek)(TermDocEnum *tde, int field_num, const char *term);
    int  (*doc_num)(TermDocEnum *tde);
    int  (*freq)(TermDocEnum *tde);
    bool (*next)(TermDocEnum *tde);
    int  (*read)(TermDocEnum *tde, int *docs, int *freqs, int req_num);
    bool (*skip_to)(TermDocEnum *tde, int target);
    int  (*next_position)(TermDocEnum *tde);
    void (*close)(TermDocEnum *tde);
};

/* * SegmentTermDocEnum * */

typedef struct SegmentTermDocEnum SegmentTermDocEnum;
struct SegmentTermDocEnum
{
    TermDocEnum tde;
    void (*seek_prox)(SegmentTermDocEnum *stde, int prx_ptr);
    void (*skip_prox)(SegmentTermDocEnum *stde);
    TermInfosReader *tir;
    InStream        *frq_in;
    InStream        *prx_in;
    InStream        *skip_in;
    BitVector       *deleted_docs;
    int count;               /* number of docs for this term  skipped */
    int doc_freq;            /* number of doc this term appears in */
    int doc_num;
    int freq;
    int num_skips;
    int skip_interval;
    int skip_count;
    int skip_doc;
    int frq_ptr;
    int prx_ptr;
    int skip_ptr;
    int prx_cnt;
    int position;
    bool have_skipped : 1;
};

extern TermDocEnum *stde_new(TermInfosReader *tir, InStream *frq_in,
                             BitVector *deleted_docs, int skip_interval);

/* * SegmentTermDocEnum * */
extern TermDocEnum *stpe_new(TermInfosReader *tir, InStream *frq_in,
                             InStream *prx_in, BitVector *deleted_docs,
                             int skip_interval);

/****************************************************************************
 * MultipleTermDocPosEnum
 ****************************************************************************/

extern TermDocEnum *mtdpe_new(IndexReader *ir, int field_num, char **terms,
                              int t_cnt);

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
    int pos;
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
    int pos;
} OccurenceWithoutOffsets;

/****************************************************************************
 *
 * Posting
 *
 ****************************************************************************/

typedef struct Posting
{
    int freq;
    int doc_num;
    Occurence *first_occ;
    struct Posting *next;
} Posting;

extern inline Posting *p_new(MemoryPool *mp, int doc_num, int pos);
extern inline Posting *p_new_wo(MemoryPool *mp, int doc_num, int pos,
                                int start, int end);

/****************************************************************************
 *
 * PostingList
 *
 ****************************************************************************/

typedef struct PostingList
{
    const char *term;
    int term_len;
    Posting *first;
    Posting *last;
    Occurence *last_occ;
} PostingList;

extern PostingList *pl_new(MemoryPool *mp, const char *term,
                           int term_len, Posting *p);
extern void pl_add_occ(MemoryPool *mp, PostingList *pl, int pos);
extern void pl_add_occ_wo(MemoryPool *mp, PostingList *pl, int pos,
                          int start_offset, int end_offset);

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
    char *field;
    int size;
    Term *terms;
} TermVector;

extern void tv_destroy(TermVector *tv);

/****************************************************************************
 *
 * TermVectorsWriter
 *
 ****************************************************************************/

#define TV_FIELD_INIT_CAPA 8

typedef struct TermVectorsWriter
{
    OutStream *tvx_out;
    OutStream *tvd_out;
    FieldInfos *fis;
    TVField *fields;
    off_t tvd_ptr;
} TermVectorsWriter;

extern TermVectorsWriter *tvw_open(Store *store,
                                   const char *segment,
                                   FieldInfos *fis);
extern void tvw_open_doc(TermVectorsWriter *tvw);
extern void tvw_close_doc(TermVectorsWriter *tvw);
extern void tvw_add_postings(TermVectorsWriter *tvw,
                             int field_num,
                             PostingList **plists,
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
                                   const char *segment,
                                   FieldInfos *fis);
extern TermVectorsReader *tvr_clone(TermVectorsReader *orig);
extern void tvr_close(TermVectorsReader *tvr);
extern HashTable *tvr_get_tv(TermVectorsReader *tvr, int doc_num);
extern TermVector *tvr_get_field_tv(TermVectorsReader *tvr,
                                    int doc_num,
                                    int field_num);

/****************************************************************************
 *
 * IndexReader
 *
 ****************************************************************************/

#define WRITE_LOCK_NAME "write"
#define COMMIT_LOCK_NAME "commit"

struct IndexReader
{
    int           (*num_docs)(IndexReader *ir);
    int           (*max_doc)(IndexReader *ir);
    Document     *(*get_doc)(IndexReader *ir, int doc_num);
    uchar        *(*get_norms)(IndexReader *ir, int field_num);
    uchar        *(*get_norms_into)(IndexReader *ir, int field_num, uchar *buf);
    TermEnum     *(*terms)(IndexReader *ir, int field_num);
    TermEnum     *(*terms_from)(IndexReader *ir, int field_num, char *term);
    int           (*doc_freq)(IndexReader *ir, int field_num, char *term);
    TermDocEnum  *(*term_docs)(IndexReader *ir);
    TermDocEnum  *(*term_positions)(IndexReader *ir);
    TermVector   *(*term_vector)(IndexReader *ir, int doc_num, char *field);
    HashTable    *(*term_vectors)(IndexReader *ir, int doc_num);
    bool          (*is_deleted)(IndexReader *ir, int doc_num);
    bool          (*has_deletions)(IndexReader *ir);
    void          (*acquire_write_lock)(IndexReader *ir);
    void          (*set_norm_i)(IndexReader *ir, int doc_num, int field_num,
                        uchar val);
    void          (*delete_doc_i)(IndexReader *ir, int doc_num);
    void          (*undelete_all_i)(IndexReader *ir);
    void          (*commit_i)(IndexReader *ir);
    void          (*close_i)(IndexReader *ir);
    Store *store;
    Lock *write_lock;
    SegmentInfos *sis;
    FieldInfos *fis;
    HashTable *cache;
    HashTable *sort_cache;
    mutex_t mutex;
    bool has_changes : 1;
    bool is_stale    : 1;
    bool is_owner    : 1;
};

extern IndexReader *ir_create(Store *store, SegmentInfos *sis, int is_owner);
extern IndexReader *ir_open(Store *store);
extern bool ir_index_exists(Store *store);
extern void ir_close(IndexReader *ir);
extern void ir_commit(IndexReader *ir);
extern void ir_delete_doc(IndexReader *ir, int doc_num);
extern void ir_undelete_all(IndexReader *ir);
extern void ir_set_norm(IndexReader *ir, int doc_num, char *field, uchar val);
extern uchar *ir_get_norms(IndexReader *ir, char *field);
extern uchar *ir_get_norms_into(IndexReader *ir, char *field, uchar *buf);
extern void ir_destroy(IndexReader *self);
extern Document *ir_get_doc_with_term(IndexReader *ir, char *field, char *term);
extern TermDocEnum *ir_term_docs_for(IndexReader *ir, char *field, char *term);
extern TermDocEnum *ir_term_positions_for(IndexReader *ir, char *fld, char *t);
extern void ir_add_cache(IndexReader *ir);
extern bool ir_is_latest(IndexReader *ir);

/****************************************************************************
 *
 * Boost
 *
 ****************************************************************************/

typedef struct Boost
{
    float val;
    int doc_num;
    struct Boost *next;
} Boost;

/****************************************************************************
 *
 * FieldInverter
 *
 ****************************************************************************/

typedef struct FieldInverter
{
    HashTable *plists;
    uchar *norms;
    FieldInfo *fi;
    int length;
    bool is_tokenized : 1;
    bool store_term_vector : 1;
    bool store_offsets : 1;
    bool has_norms : 1;
} FieldInverter;

/****************************************************************************
 *
 * DocWriter
 *
 ****************************************************************************/

typedef struct IndexWriter IndexWriter;

typedef struct DocWriter
{
    Store *store;
    const char *segment;
    FieldInfos *fis;
    TermVectorsWriter *tvw;
    FieldsWriter *fw;
    MemoryPool *mp;
    Analyzer *analyzer;
    HashTable *curr_plists;
    HashTable *fields;
    Similarity *similarity;
    int doc_num;
    int index_interval;
    int skip_interval;
    int max_field_length;
    int max_buffered_docs;
} DocWriter;

extern DocWriter *dw_open(IndexWriter *is, const char *segment);
extern void dw_close(DocWriter *dw);
extern void dw_add_doc(DocWriter *dw, Document *doc);
extern void dw_new_segment(DocWriter *dw, char *segment);

/****************************************************************************
 *
 * IndexWriter
 *
 ****************************************************************************/

struct IndexWriter
{
    Config config;
    mutex_t mutex;
    Store *store;
    Analyzer *analyzer;
    SegmentInfos *sis;
    FieldInfos *fis;
    DocWriter *dw;
    Similarity *similarity;
    Lock *write_lock;
};

extern void index_create(Store *store, FieldInfos *fis);
extern IndexWriter *iw_open(Store *store, Analyzer *analyzer, const Config *config);
extern void iw_close(IndexWriter *iw);
extern void iw_add_doc(IndexWriter *iw, Document *doc);
extern int iw_doc_count(IndexWriter *iw);
extern void iw_commit(IndexWriter *iw);
extern void iw_optimize(IndexWriter *iw);

/****************************************************************************
 *
 * CompoundWriter
 *
 ****************************************************************************/

#define CW_INIT_CAPA 16
typedef struct CWFileEntry
{
    char *name;
    off_t dir_offset;
    off_t data_offset;
} CWFileEntry;

typedef struct CompoundWriter {
    Store *store;
    const char *name;
    HashSet *ids;
    CWFileEntry *file_entries;
} CompoundWriter;

extern CompoundWriter *open_cw(Store *store, char *name);
extern void cw_add_file(CompoundWriter *cw, char *id);
extern void cw_close(CompoundWriter *cw);


#endif

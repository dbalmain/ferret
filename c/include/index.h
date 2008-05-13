#ifndef FRT_INDEX_H
#define FRT_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"
#include "document.h"
#include "analysis.h"
#include "hash.h"
#include "hashset.h"
#include "store.h"
#include "mempool.h"
#include "similarity.h"
#include "bitvector.h"
#include "priorityqueue.h"

typedef struct FrtIndexReader FrtIndexReader;
typedef struct FrtMultiReader FrtMultiReader;
typedef struct FrtDeleter FrtDeleter;

extern bool frt_file_name_filter_is_index_file(const char *file_name, bool include_locks);

/****************************************************************************
 *
 * FrtConfig
 *
 ****************************************************************************/

typedef struct FrtConfig
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
} FrtConfig;

extern const FrtConfig frt_default_config;

/***************************************************************************
 *
 * FrtCacheObject
 *
 ***************************************************************************/

typedef struct FrtCacheObject {
    FrtHash *ref_tab1;
    FrtHash *ref_tab2;
    void *ref1;
    void *ref2;
    void *obj;
    void (*destroy)(void *p);
} FrtCacheObject;

extern void frt_cache_destroy(FrtCacheObject *co);
extern FrtCacheObject *frt_co_create(FrtHash *ref_tab1,
                              FrtHash *ref_tab2,
            void *ref1, void *ref2, frt_free_ft destroy, void *obj);
extern FrtHash *frt_co_hash_create();

/****************************************************************************
 *
 * FrtFieldInfo
 *
 ****************************************************************************/

typedef enum
{
    FRT_STORE_NO = 0,
    FRT_STORE_YES = 1,
    FRT_STORE_COMPRESS = 2
} FrtStoreValue;

typedef enum
{
    FRT_INDEX_NO = 0,
    FRT_INDEX_UNTOKENIZED = 1,
    FRT_INDEX_YES = 3,
    FRT_INDEX_UNTOKENIZED_OMIT_NORMS = 5,
    FRT_INDEX_YES_OMIT_NORMS = 7
} FrtIndexValue;

typedef enum
{
    FRT_TERM_VECTOR_NO = 0,
    FRT_TERM_VECTOR_YES = 1,
    FRT_TERM_VECTOR_WITH_POSITIONS = 3,
    FRT_TERM_VECTOR_WITH_OFFSETS = 5,
    FRT_TERM_VECTOR_WITH_POSITIONS_OFFSETS = 7
} FrtTermVectorValue;

#define FRT_FI_IS_STORED_BM         0x001
#define FRT_FI_IS_COMPRESSED_BM     0x002
#define FRT_FI_IS_INDEXED_BM        0x004
#define FRT_FI_IS_TOKENIZED_BM      0x008
#define FRT_FI_OMIT_NORMS_BM        0x010
#define FRT_FI_STORE_TERM_VECTOR_BM 0x020
#define FRT_FI_STORE_POSITIONS_BM   0x040
#define FRT_FI_STORE_OFFSETS_BM     0x080

typedef struct FrtFieldInfo
{
    FrtSymbol name;
    float boost;
    unsigned int bits;
    int number;
    int ref_cnt;
} FrtFieldInfo;

extern FrtFieldInfo *frt_fi_new(FrtSymbol name,
                                FrtStoreValue store,
                                FrtIndexValue index,
                                FrtTermVectorValue term_vector);
extern char *frt_fi_to_s(FrtFieldInfo *fi);
extern void frt_fi_deref(FrtFieldInfo *fi);

#define fi_is_stored(fi)         (((fi)->bits & FRT_FI_IS_STORED_BM) != 0)
#define fi_is_compressed(fi)     (((fi)->bits & FRT_FI_IS_COMPRESSED_BM) != 0)
#define fi_is_indexed(fi)        (((fi)->bits & FRT_FI_IS_INDEXED_BM) != 0)
#define fi_is_tokenized(fi)      (((fi)->bits & FRT_FI_IS_TOKENIZED_BM) != 0)
#define fi_omit_norms(fi)        (((fi)->bits & FRT_FI_OMIT_NORMS_BM) != 0)
#define fi_store_term_vector(fi) (((fi)->bits & FRT_FI_STORE_TERM_VECTOR_BM) != 0)
#define fi_store_positions(fi)   (((fi)->bits & FRT_FI_STORE_POSITIONS_BM) != 0)
#define fi_store_offsets(fi)     (((fi)->bits & FRT_FI_STORE_OFFSETS_BM) != 0)
#define fi_has_norms(fi)\
    (((fi)->bits & (FRT_FI_OMIT_NORMS_BM|FRT_FI_IS_INDEXED_BM)) == FRT_FI_IS_INDEXED_BM)

/****************************************************************************
 *
 * FrtFieldInfos
 *
 ****************************************************************************/

#define FIELD_INFOS_INIT_CAPA 4
/* carry changes over to dummy_fis in test/test_segments.c */
typedef struct FrtFieldInfos
{
    FrtStoreValue store;
    FrtIndexValue index;
    FrtTermVectorValue term_vector;
    int size;
    int capa;
    FrtFieldInfo **fields;
    FrtHash *field_dict;
    int ref_cnt;
} FrtFieldInfos;

FrtFieldInfos *frt_fis_new(FrtStoreValue store, FrtIndexValue index,
                                  FrtTermVectorValue term_vector);
extern FrtFieldInfo *frt_fis_add_field(FrtFieldInfos *fis, FrtFieldInfo *fi);
extern FrtFieldInfo *frt_fis_get_field(FrtFieldInfos *fis, FrtSymbol name);
extern int frt_fis_get_field_num(FrtFieldInfos *fis, FrtSymbol name);
extern FrtFieldInfo *frt_fis_by_number(FrtFieldInfos *fis, int num);
extern FrtFieldInfo *frt_fis_get_or_add_field(FrtFieldInfos *fis,
                                              FrtSymbol name);
extern void frt_fis_write(FrtFieldInfos *fis, FrtOutStream *os);
extern FrtFieldInfos *frt_fis_read(FrtInStream *is);
extern char *frt_fis_to_s(FrtFieldInfos *fis);
extern void frt_fis_deref(FrtFieldInfos *fis);

/****************************************************************************
 *
 * FrtSegmentInfo
 *
 ****************************************************************************/

#define FRT_SEGMENT_NAME_MAX_LENGTH 100
#define FRT_SEGMENTS_FILE_NAME "segments"

typedef struct FrtSegmentInfo
{
    int ref_cnt;
    char *name;
    FrtStore *store;
    int doc_cnt;
    int del_gen;
    int *norm_gens;
    int norm_gens_size;
    bool use_compound_file;
} FrtSegmentInfo;

extern FrtSegmentInfo *frt_si_new(char *name, int doc_cnt, FrtStore *store);
extern void frt_si_deref(FrtSegmentInfo *si);
extern bool frt_si_has_deletions(FrtSegmentInfo *si);
extern bool frt_si_uses_compound_file(FrtSegmentInfo *si);
extern bool frt_si_has_separate_norms(FrtSegmentInfo *si);
extern void frt_si_advance_norm_gen(FrtSegmentInfo *si, int field_num);

/****************************************************************************
 *
 * FrtSegmentInfos
 *
 ****************************************************************************/

typedef struct FrtSegmentInfos
{
    FrtFieldInfos *fis;
    frt_u64 counter;
    frt_u64 version;
    frt_i64 generation;
    frt_i32 format;
    FrtStore *store;
    FrtSegmentInfo **segs;
    int size;
    int capa;
} FrtSegmentInfos;

extern char *frt_fn_for_generation(char *buf, char *base, char *ext, frt_i64 gen);

extern FrtSegmentInfos *frt_sis_new(FrtFieldInfos *fis);
extern FrtSegmentInfo *frt_sis_new_segment(FrtSegmentInfos *sis, int dcnt, FrtStore *store);
extern FrtSegmentInfo *frt_sis_add_si(FrtSegmentInfos *sis, FrtSegmentInfo *si);
extern void frt_sis_del_at(FrtSegmentInfos *sis, int at);
extern void frt_sis_del_from_to(FrtSegmentInfos *sis, int from, int to);
extern void frt_sis_clear(FrtSegmentInfos *sis);
extern FrtSegmentInfos *frt_sis_read(FrtStore *store);
extern void frt_sis_write(FrtSegmentInfos *sis, FrtStore *store, FrtDeleter *deleter);
extern frt_u64 frt_sis_read_current_version(FrtStore *store);
extern void frt_sis_destroy(FrtSegmentInfos *sis);
extern frt_i64 frt_sis_current_segment_generation(FrtStore *store);
extern char *frt_sis_curr_seg_file_name(char *buf, FrtStore *store);
extern void frt_sis_put(FrtSegmentInfos *sis, FILE *stream);

/****************************************************************************
 *
 * FrtTermInfo
 *
 ****************************************************************************/

typedef struct FrtTermInfo
{
    int doc_freq;
    off_t frq_ptr;
    off_t prx_ptr;
    off_t skip_offset;
} FrtTermInfo;

#define frt_ti_set(ti, mdf, mfp, mpp, mso) do {\
    (ti).doc_freq = mdf;\
    (ti).frq_ptr = mfp;\
    (ti).prx_ptr = mpp;\
    (ti).skip_offset = mso;\
} while (0)

/****************************************************************************
 *
 * FrtTermEnum
 *
 ****************************************************************************/

typedef struct FrtTermEnum FrtTermEnum;

struct FrtTermEnum
{
    char        curr_term[FRT_MAX_WORD_SIZE];
    char        prev_term[FRT_MAX_WORD_SIZE];
    FrtTermInfo    curr_ti;
    int         curr_term_len;
    int         field_num;
    FrtTermEnum *(*set_field)(FrtTermEnum *te, int field_num);
    char     *(*next)(FrtTermEnum *te);
    char     *(*skip_to)(FrtTermEnum *te, const char *term);
    void      (*close)(FrtTermEnum *te);
    FrtTermEnum *(*clone)(FrtTermEnum *te);
};

char *frt_te_get_term(struct FrtTermEnum *te);
FrtTermInfo *frt_te_get_ti(struct FrtTermEnum *te);

/****************************************************************************
 *
 * FrtSegmentTermEnum
 *
 ****************************************************************************/

/* * FrtSegmentTermIndex * */

typedef struct FrtSegmentTermIndex
{
    off_t       index_ptr;
    off_t       ptr;
    int         index_cnt;
    int         size;
    char      **index_terms;
    int        *index_term_lens;
    FrtTermInfo   *index_term_infos;
    off_t      *index_ptrs;
} FrtSegmentTermIndex;

/* * FrtSegmentFieldIndex * */

typedef struct FrtSegmentTermEnum FrtSegmentTermEnum;

typedef struct FrtSegmentFieldIndex
{
    frt_mutex_t     mutex;
    int         skip_interval;
    int         index_interval;
    off_t       index_ptr;
    FrtTermEnum   *index_te;
    FrtHash  *field_dict;
} FrtSegmentFieldIndex;

extern FrtSegmentFieldIndex *frt_sfi_open(FrtStore *store, const char *segment);
extern void frt_sfi_close(FrtSegmentFieldIndex *sfi);


/* * FrtSegmentTermEnum * */
struct FrtSegmentTermEnum
{
    FrtTermEnum    te;
    FrtInStream   *is;
    int         size;
    int         pos;
    int         skip_interval;
    FrtSegmentFieldIndex *sfi;
};

extern void frt_ste_close(FrtTermEnum *te);
extern FrtTermEnum *frt_ste_clone(FrtTermEnum *te);
extern FrtTermEnum *frt_ste_new(FrtInStream *is, FrtSegmentFieldIndex *sfi);

/* * MultiTermEnum * */

extern FrtTermEnum *frt_mte_new(FrtMultiReader *mr, int field_num, const char *term);

/****************************************************************************
 *
 * FrtTermInfosReader
 *
 ****************************************************************************/

#define FRT_TE_BUCKET_INIT_CAPA 1

typedef struct FrtTermInfosReader
{
    frt_thread_key_t thread_te;
    void       **te_bucket;
    FrtTermEnum     *orig_te;
    int          field_num;
} FrtTermInfosReader;

extern FrtTermInfosReader *frt_tir_open(FrtStore *store,
                                 FrtSegmentFieldIndex *sfi,
                                 const char *segment);
extern FrtTermInfosReader *frt_tir_set_field(FrtTermInfosReader *tir, int field_num);
extern FrtTermInfo *frt_tir_get_ti(FrtTermInfosReader *tir, const char *term);
extern char *frt_tir_get_term(FrtTermInfosReader *tir, int pos);
extern void frt_tir_close(FrtTermInfosReader *tir);

/****************************************************************************
 *
 * FrtTermInfosWriter
 *
 ****************************************************************************/

#define FRT_INDEX_INTERVAL 128
#define FRT_SKIP_INTERVAL 16

typedef struct FrtTermWriter
{
    int counter;
    const char *last_term;
    FrtTermInfo last_term_info;
    FrtOutStream *os;
} FrtTermWriter;

typedef struct FrtTermInfosWriter
{
    int field_count;
    int index_interval;
    int skip_interval;
    off_t last_index_ptr;
    FrtOutStream *tfx_out;
    FrtTermWriter *tix_writer;
    FrtTermWriter *tis_writer;
} FrtTermInfosWriter;

extern FrtTermInfosWriter *frt_tiw_open(FrtStore *store,
                                 const char *segment,
                                 int index_interval,
                                 int skip_interval);
extern void frt_tiw_start_field(FrtTermInfosWriter *tiw, int field_num);
extern void frt_tiw_add(FrtTermInfosWriter *tiw,
                    const char *term,
                    int t_len,
                    FrtTermInfo *ti);
extern void frt_tiw_close(FrtTermInfosWriter *tiw);

/****************************************************************************
 *
 * FrtTermDocEnum
 *
 ****************************************************************************/

typedef struct FrtTermDocEnum FrtTermDocEnum;
struct FrtTermDocEnum
{
    void (*seek)(FrtTermDocEnum *tde, int field_num, const char *term);
    void (*seek_te)(FrtTermDocEnum *tde, FrtTermEnum *te);
    void (*seek_ti)(FrtTermDocEnum *tde, FrtTermInfo *ti);
    int  (*doc_num)(FrtTermDocEnum *tde);
    int  (*freq)(FrtTermDocEnum *tde);
    bool (*next)(FrtTermDocEnum *tde);
    int  (*read)(FrtTermDocEnum *tde, int *docs, int *freqs, int req_num);
    bool (*skip_to)(FrtTermDocEnum *tde, int target);
    int  (*next_position)(FrtTermDocEnum *tde);
    void (*close)(FrtTermDocEnum *tde);
};

/* * FrtSegmentTermDocEnum * */

typedef struct FrtSegmentTermDocEnum FrtSegmentTermDocEnum;
struct FrtSegmentTermDocEnum
{
    FrtTermDocEnum tde;
    void (*seek_prox)(FrtSegmentTermDocEnum *stde, off_t prx_ptr);
    void (*skip_prox)(FrtSegmentTermDocEnum *stde);
    FrtTermInfosReader *tir;
    FrtInStream        *frq_in;
    FrtInStream        *prx_in;
    FrtInStream        *skip_in;
    FrtBitVector       *deleted_docs;
    int count;               /* number of docs for this term  skipped */
    int doc_freq;            /* number of doc this term appears in */
    int doc_num;
    int freq;
    int num_skips;
    int skip_interval;
    int skip_count;
    int skip_doc;
    int prx_cnt;
    int position;
    off_t frq_ptr;
    off_t prx_ptr;
    off_t skip_ptr;
    bool have_skipped : 1;
};

extern FrtTermDocEnum *frt_stde_new(FrtTermInfosReader *tir, FrtInStream *frq_in,
                             FrtBitVector *deleted_docs, int skip_interval);

/* * FrtSegmentTermDocEnum * */
extern FrtTermDocEnum *frt_stpe_new(FrtTermInfosReader *tir, FrtInStream *frq_in,
                             FrtInStream *prx_in, FrtBitVector *deleted_docs,
                             int skip_interval);

/****************************************************************************
 * MultipleTermDocPosEnum
 ****************************************************************************/

extern FrtTermDocEnum *frt_mtdpe_new(FrtIndexReader *ir, int field_num, char **terms,
                              int t_cnt);

/****************************************************************************
 *
 * FrtOffset
 *
 ****************************************************************************/

typedef struct FrtOffset
{
    off_t start;
    off_t end;
} FrtOffset;

extern FrtOffset *frt_offset_new(off_t start, off_t end);

/****************************************************************************
 *
 * FrtOccurence
 *
 ****************************************************************************/

typedef struct FrtOccurence
{
    struct FrtOccurence *next;
    int pos;
} FrtOccurence;

/****************************************************************************
 *
 * FrtPosting
 *
 ****************************************************************************/

typedef struct FrtPosting
{
    int freq;
    int doc_num;
    FrtOccurence *first_occ;
    struct FrtPosting *next;
} FrtPosting;

extern FrtPosting *frt_p_new(FrtMemoryPool *mp, int doc_num, int pos);

/****************************************************************************
 *
 * FrtPostingList
 *
 ****************************************************************************/

typedef struct FrtPostingList
{
    const char *term;
    int term_len;
    FrtPosting *first;
    FrtPosting *last;
    FrtOccurence *last_occ;
} FrtPostingList;

extern FrtPostingList *frt_pl_new(FrtMemoryPool *mp, const char *term,
                           int term_len, FrtPosting *p);
extern void frt_pl_add_occ(FrtMemoryPool *mp, FrtPostingList *pl, int pos);
extern int frt_pl_cmp(const FrtPostingList **pl1, const FrtPostingList **pl2);

/****************************************************************************
 *
 * FrtTVField
 *
 ****************************************************************************/

typedef struct FrtTVField
{
    int field_num;
    int size;
} FrtTVField;

/****************************************************************************
 *
 * FrtTVTerm
 *
 ****************************************************************************/

typedef struct FrtTVTerm
{
    char   *text;
    int     freq;
    int    *positions;
} FrtTVTerm;

/****************************************************************************
 *
 * FrtTermVector
 *
 ****************************************************************************/

#define FRT_TV_FIELD_INIT_CAPA 8
typedef struct FrtTermVector
{
    int       field_num;
    FrtSymbol field;
    int       term_cnt;
    FrtTVTerm *terms;
    int       offset_cnt;
    FrtOffset *offsets;
} FrtTermVector;

extern void frt_tv_destroy(FrtTermVector *tv);
extern int frt_tv_get_term_index(FrtTermVector *tv, const char *term);
extern int frt_tv_scan_to_term_index(FrtTermVector *tv, const char *term);
extern FrtTVTerm *frt_tv_get_tv_term(FrtTermVector *tv, const char *term);

/****************************************************************************
 *
 * FrtLazyDoc
 *
 ****************************************************************************/

/* * * FrtLazyDocField * * */
typedef struct FrtLazyDocFieldData
{
    off_t start;
    int   length;
    char *text;
} FrtLazyDocFieldData;

typedef struct FrtLazyDoc FrtLazyDoc;
typedef struct FrtLazyDocField
{
    FrtSymbol           name;
    FrtLazyDocFieldData *data;
    FrtLazyDoc          *doc;
    int                 size; /* number of data elements */
    int                 len;  /* length of data elements concatenated */
    bool                is_compressed : 2; /* set to 2 after all data is loaded */
} FrtLazyDocField;

extern char *frt_lazy_df_get_data(FrtLazyDocField *self, int i);
extern void frt_lazy_df_get_bytes(FrtLazyDocField *self, char *buf,
                              int start, int len);

/* * * FrtLazyDoc * * */
struct FrtLazyDoc
{
    FrtHash *field_dictionary;
    int size;
    FrtLazyDocField **fields;
    FrtInStream *fields_in;
};

extern void frt_lazy_doc_close(FrtLazyDoc *self);
extern FrtLazyDocField *frt_lazy_doc_get(FrtLazyDoc *self, FrtSymbol field);

/****************************************************************************
 *
 * FrtFieldsReader
 *
 ****************************************************************************/

typedef struct FrtFieldsReader
{
    int           size;
    FrtFieldInfos *fis;
    FrtStore      *store;
    FrtInStream   *fdx_in;
    FrtInStream   *fdt_in;
} FrtFieldsReader;

extern FrtFieldsReader *frt_fr_open(FrtStore *store,
                             const char *segment, FrtFieldInfos *fis);
extern FrtFieldsReader *frt_fr_clone(FrtFieldsReader *orig);
extern void frt_fr_close(FrtFieldsReader *fr);
extern FrtDocument *frt_fr_get_doc(FrtFieldsReader *fr, int doc_num);
extern FrtLazyDoc *frt_fr_get_lazy_doc(FrtFieldsReader *fr, int doc_num);
extern FrtHash *frt_fr_get_tv(FrtFieldsReader *fr, int doc_num);
extern FrtTermVector *frt_fr_get_field_tv(FrtFieldsReader *fr, int doc_num,
                                   int field_num);

/****************************************************************************
 *
 * FrtFieldsWriter
 *
 ****************************************************************************/

typedef struct FrtFieldsWriter
{
    FrtFieldInfos *fis;
    FrtOutStream  *fdt_out;
    FrtOutStream  *fdx_out;
    FrtOutStream  *buffer;
    FrtTVField    *tv_fields;
    off_t       start_ptr;
} FrtFieldsWriter;

extern FrtFieldsWriter *frt_fw_open(FrtStore *store,
                             const char *segment, FrtFieldInfos *fis);
extern void frt_fw_close(FrtFieldsWriter *fw);
extern void frt_fw_add_doc(FrtFieldsWriter *fw, FrtDocument *doc);
extern void frt_fw_add_postings(FrtFieldsWriter *fw,
                            int field_num,
                            FrtPostingList **plists,
                            int posting_count,
                            FrtOffset *offsets,
                            int offset_count);
extern void frt_fw_write_tv_index(FrtFieldsWriter *fw);

/****************************************************************************
 *
 * FrtDeleter
 *
 * A utility class (used by both FrtIndexReader and FrtIndexWriter) to keep track of
 * files that need to be deleted because they are no longer referenced by the
 * index.
 *
 ****************************************************************************/

struct FrtDeleter
{
    FrtStore         *store;
    FrtSegmentInfos  *sis;
    FrtHashSet       *pending;
};

extern FrtDeleter *frt_deleter_new(FrtSegmentInfos *sis, FrtStore *store);
extern void frt_deleter_destroy(FrtDeleter *dlr);
extern void frt_deleter_clear_pending_files(FrtDeleter *dlr);
extern void frt_deleter_delete_file(FrtDeleter *dlr, char *file_name);
extern void frt_deleter_find_deletable_files(FrtDeleter *dlr);
extern void frt_deleter_commit_pending_files(FrtDeleter *dlr);
extern void frt_deleter_delete_files(FrtDeleter *dlr, char **files, int file_cnt);

/****************************************************************************
 *
 * FrtIndexReader
 *
 ****************************************************************************/

#define FRT_WRITE_LOCK_NAME "write"
#define FRT_COMMIT_LOCK_NAME "commit"

struct FrtIndexReader
{
    int                 (*num_docs)(FrtIndexReader *ir);
    int                 (*max_doc)(FrtIndexReader *ir);
    FrtDocument           *(*get_doc)(FrtIndexReader *ir, int doc_num);
    FrtLazyDoc            *(*get_lazy_doc)(FrtIndexReader *ir, int doc_num);
    frt_uchar              *(*get_norms)(FrtIndexReader *ir, int field_num);
    frt_uchar              *(*get_norms_into)(FrtIndexReader *ir, int field_num,
                                          frt_uchar *buf);
    FrtTermEnum           *(*terms)(FrtIndexReader *ir, int field_num);
    FrtTermEnum           *(*terms_from)(FrtIndexReader *ir, int field_num,
                                      const char *term);
    int                 (*doc_freq)(FrtIndexReader *ir, int field_num,
                                    const char *term);
    FrtTermDocEnum        *(*term_docs)(FrtIndexReader *ir);
    FrtTermDocEnum        *(*term_positions)(FrtIndexReader *ir);
    FrtTermVector         *(*term_vector)(FrtIndexReader *ir, int doc_num,
                                          FrtSymbol field);
    FrtHash    *(*term_vectors)(FrtIndexReader *ir, int doc_num);
    bool                (*is_deleted)(FrtIndexReader *ir, int doc_num);
    bool                (*has_deletions)(FrtIndexReader *ir);
    void                (*acquire_write_lock)(FrtIndexReader *ir);
    void                (*set_norm_i)(FrtIndexReader *ir, int doc_num,
                                      int field_num, frt_uchar val);
    void                (*delete_doc_i)(FrtIndexReader *ir, int doc_num);
    void                (*undelete_all_i)(FrtIndexReader *ir);
    void                (*set_deleter_i)(FrtIndexReader *ir, FrtDeleter *dlr);
    bool                (*is_latest_i)(FrtIndexReader *ir);
    void                (*commit_i)(FrtIndexReader *ir);
    void                (*close_i)(FrtIndexReader *ir);
    int                 ref_cnt;
    FrtDeleter            *deleter;
    FrtStore              *store;
    FrtLock               *write_lock;
    FrtSegmentInfos       *sis;
    FrtFieldInfos         *fis;
    FrtHash    *cache;
    FrtHash    *field_index_cache;
    frt_mutex_t             field_index_mutex;
    frt_uchar              *fake_norms;
    frt_mutex_t             mutex;
    bool                has_changes : 1;
    bool                is_stale    : 1;
    bool                is_owner    : 1;
};

extern FrtIndexReader *frt_ir_create(FrtStore *store, FrtSegmentInfos *sis, int is_owner);
extern FrtIndexReader *frt_ir_open(FrtStore *store);
extern int frt_ir_get_field_num(FrtIndexReader *ir, FrtSymbol field);
extern bool frt_ir_index_exists(FrtStore *store);
extern void frt_ir_close(FrtIndexReader *ir);
extern void frt_ir_commit(FrtIndexReader *ir);
extern void frt_ir_delete_doc(FrtIndexReader *ir, int doc_num);
extern void frt_ir_undelete_all(FrtIndexReader *ir);
extern int frt_ir_doc_freq(FrtIndexReader *ir, FrtSymbol field, const char *term);
extern void frt_ir_set_norm(FrtIndexReader *ir, int doc_num, FrtSymbol field,
                        frt_uchar val);
extern frt_uchar *frt_ir_get_norms_i(FrtIndexReader *ir, int field_num);
extern frt_uchar *frt_ir_get_norms(FrtIndexReader *ir, FrtSymbol field);
extern frt_uchar *frt_ir_get_norms_into(FrtIndexReader *ir, FrtSymbol field, frt_uchar *buf);
extern void frt_ir_destroy(FrtIndexReader *self);
extern FrtDocument *frt_ir_get_doc_with_term(FrtIndexReader *ir, FrtSymbol field,
                                      const char *term);
extern FrtTermEnum *frt_ir_terms(FrtIndexReader *ir, FrtSymbol field);
extern FrtTermEnum *frt_ir_terms_from(FrtIndexReader *ir, FrtSymbol field,
                               const char *t);
extern FrtTermDocEnum *frt_ir_term_docs_for(FrtIndexReader *ir, FrtSymbol field,
                                     const char *term);
extern FrtTermDocEnum *frt_ir_term_positions_for(FrtIndexReader *ir,
                                                 FrtSymbol field,
                                                 const char *t);
extern void frt_ir_add_cache(FrtIndexReader *ir);
extern bool frt_ir_is_latest(FrtIndexReader *ir);

/****************************************************************************
 * FrtMultiReader
 ****************************************************************************/

struct FrtMultiReader {
    FrtIndexReader ir;
    int max_doc;
    int num_docs_cache;
    int r_cnt;
    int *starts;
    FrtIndexReader **sub_readers;
    FrtHash *norms_cache;
    bool has_deletions : 1;
    int **field_num_map;
};

extern int frt_mr_get_field_num(FrtMultiReader *mr, int ir_num, int f_num);
extern FrtIndexReader *frt_mr_open(FrtIndexReader **sub_readers, const int r_cnt);


/****************************************************************************
 *
 * FrtBoost
 *
 ****************************************************************************/

typedef struct FrtBoost
{
    float val;
    int doc_num;
    struct FrtBoost *next;
} FrtBoost;

/****************************************************************************
 *
 * FrtFieldInverter
 *
 ****************************************************************************/

typedef struct FrtFieldInverter
{
    FrtHash *plists;
    frt_uchar *norms;
    FrtFieldInfo *fi;
    int length;
    bool is_tokenized : 1;
    bool store_term_vector : 1;
    bool store_offsets : 1;
    bool has_norms : 1;
} FrtFieldInverter;

/****************************************************************************
 *
 * FrtDocWriter
 *
 ****************************************************************************/

#define DW_OFFSET_INIT_CAPA 512
typedef struct FrtIndexWriter FrtIndexWriter;

typedef struct FrtDocWriter
{
    FrtStore *store;
    FrtSegmentInfo *si;
    FrtFieldInfos *fis;
    FrtFieldsWriter *fw;
    FrtMemoryPool *mp;
    FrtAnalyzer *analyzer;
    FrtHash *curr_plists;
    FrtHash *fields;
    FrtSimilarity *similarity;
    FrtOffset *offsets;
    int offsets_size;
    int offsets_capa;
    int doc_num;
    int index_interval;
    int skip_interval;
    int max_field_length;
    int max_buffered_docs;
} FrtDocWriter;

extern FrtDocWriter *frt_dw_open(FrtIndexWriter *is, FrtSegmentInfo *si);
extern void frt_dw_close(FrtDocWriter *dw);
extern void frt_dw_add_doc(FrtDocWriter *dw, FrtDocument *doc);
extern void frt_dw_new_segment(FrtDocWriter *dw, FrtSegmentInfo *si);
/* For testing. need to remove somehow. FIXME */
extern FrtHash *frt_dw_invert_field(FrtDocWriter *dw,
                                  FrtFieldInverter *fld_inv,
                                  FrtDocField *df);
extern FrtFieldInverter *frt_dw_get_fld_inv(FrtDocWriter *dw, FrtFieldInfo *fi);
extern void frt_dw_reset_postings(FrtHash *postings);

/****************************************************************************
 *
 * FrtIndexWriter
 *
 ****************************************************************************/

typedef struct FrtDelTerm
{
    int field_num;
    char *term;
} FrtDelTerm;

struct FrtIndexWriter
{
    FrtConfig config;
    frt_mutex_t mutex;
    FrtStore *store;
    FrtAnalyzer *analyzer;
    FrtSegmentInfos *sis;
    FrtFieldInfos *fis;
    FrtDocWriter *dw;
    FrtSimilarity *similarity;
    FrtLock *write_lock;
    FrtDeleter *deleter;
};

extern void frt_index_create(FrtStore *store, FrtFieldInfos *fis);
extern bool frt_index_is_locked(FrtStore *store);
extern FrtIndexWriter *frt_iw_open(FrtStore *store, FrtAnalyzer *analyzer,
                            const FrtConfig *config);
extern void frt_iw_delete_term(FrtIndexWriter *iw, FrtSymbol field,
                           const char *term);
extern void frt_iw_delete_terms(FrtIndexWriter *iw, FrtSymbol field,
                            char **terms, const int term_cnt);
extern void frt_iw_close(FrtIndexWriter *iw);
extern void frt_iw_add_doc(FrtIndexWriter *iw, FrtDocument *doc);
extern int frt_iw_doc_count(FrtIndexWriter *iw);
extern void frt_iw_commit(FrtIndexWriter *iw);
extern void frt_iw_optimize(FrtIndexWriter *iw);
extern void frt_iw_add_readers(FrtIndexWriter *iw, FrtIndexReader **readers,
                           const int r_cnt);

/****************************************************************************
 *
 * FrtCompoundWriter
 *
 ****************************************************************************/

#define FRT_CW_INIT_CAPA 16
typedef struct FrtCWFileEntry
{
    char *name;
    off_t dir_offset;
    off_t data_offset;
} FrtCWFileEntry;

typedef struct FrtCompoundWriter {
    FrtStore *store;
    const char *name;
    FrtHashSet *ids;
    FrtCWFileEntry *file_entries;
} FrtCompoundWriter;

extern FrtCompoundWriter *frt_open_cw(FrtStore *store, char *name);
extern void frt_cw_add_file(FrtCompoundWriter *cw, char *id);
extern void frt_cw_close(FrtCompoundWriter *cw);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

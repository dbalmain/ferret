#ifndef FRT_IND_H
#define FRT_IND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "search.h"
#include "index.h"
#include "symbol.h"

/***************************************************************************
 *
 * FrtIndex
 *
 ***************************************************************************/

typedef struct FrtIndex
{
    FrtConfig config;
    frt_mutex_t mutex;
    FrtStore *store;
    FrtAnalyzer *analyzer;
    FrtIndexReader *ir;
    FrtIndexWriter *iw;
    FrtSearcher *sea;
    FrtQParser *qp;
    FrtHashSet *key;
    FrtSymbol id_field;
    FrtSymbol def_field;
    /* for FrtIndexWriter */
    bool auto_flush : 1;
    bool has_writes : 1;
    bool check_latest : 1;
} FrtIndex;

extern FrtIndex *frt_index_new(FrtStore *store, FrtAnalyzer *analyzer,
                        FrtHashSet *def_fields, bool create);
extern void frt_index_destroy(FrtIndex *self);
extern void frt_index_flush(FrtIndex *self);
extern int frt_index_size(FrtIndex *self);
extern void frt_index_optimize(FrtIndex *self);
extern bool frt_index_has_del(FrtIndex *self);
extern bool frt_index_is_deleted(FrtIndex *self, int doc_num);
extern void frt_index_add_doc(FrtIndex *self, FrtDocument *doc);
extern void frt_index_add_string(FrtIndex *self, char *str);
extern void frt_index_add_array(FrtIndex *self, char **ary);
extern FrtTopDocs *frt_index_search_str(FrtIndex *self, char *query, int first_doc,
                                 int num_docs, FrtFilter *filter,
                                 FrtSort *sort, FrtPostFilter *post_filter);
extern FrtQuery *frt_index_get_query(FrtIndex *self, char *qstr);
extern FrtDocument *frt_index_get_doc(FrtIndex *self, int doc_num);
extern FrtDocument *frt_index_get_doc_ts(FrtIndex *self, int doc_num);
extern FrtDocument *frt_index_get_doc_id(FrtIndex *self, const char *id);
extern FrtDocument *frt_index_get_doc_term(FrtIndex *self, FrtSymbol field,
                                    const char *term);
extern void frt_index_delete(FrtIndex *self, int doc_num);
extern void frt_index_delete_term(FrtIndex *self, FrtSymbol field, const char *term);
extern void frt_index_delete_id(FrtIndex *self, const char *id);
extern void frt_index_delete_query(FrtIndex *self, FrtQuery *q, FrtFilter *f, FrtPostFilter *pf);
extern void frt_index_delete_query_str(FrtIndex *self, char *qstr,
                                   FrtFilter *f, FrtPostFilter *pf);
extern int frt_index_term_id(FrtIndex *self, FrtSymbol field, const char *term);
extern FrtExplanation *frt_index_explain(FrtIndex *self, FrtQuery *q, int doc_num);
extern void frt_index_auto_flush_ir(FrtIndex *self);
extern void frt_index_auto_flush_iw(FrtIndex *self);

extern FRT_INLINE void frt_ensure_searcher_open(FrtIndex *self);
extern FRT_INLINE void frt_ensure_reader_open(FrtIndex *self);
extern FRT_INLINE void frt_ensure_writer_open(FrtIndex *self);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

#ifndef FRT_IND_H
#define FRT_IND_H

#include "search.h"
#include "index.h"

/***************************************************************************
 *
 * FrtIndex
 *
 ***************************************************************************/

typedef struct FrtIndex
{
    Config config;
    mutex_t mutex;
    Store *store;
    FrtAnalyzer *analyzer;
    IndexReader *ir;
    IndexWriter *iw;
    Searcher *sea;
    QParser *qp;
    FrtHashSet *key;
    char *id_field;
    char *def_field;
    /* for IndexWriter */
    bool auto_flush : 1;
    bool has_writes : 1;
    bool check_latest : 1;
} FrtIndex;

extern FrtIndex *frt_index_new(Store *store, FrtAnalyzer *analyzer,
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
extern TopDocs *frt_index_search_str(FrtIndex *self, char *query, int first_doc,
                                 int num_docs, FrtFilter *filter,
                                 Sort *sort, FrtPostFilter *post_filter);
extern Query *frt_index_get_query(FrtIndex *self, char *qstr);
extern FrtDocument *frt_index_get_doc(FrtIndex *self, int doc_num);
extern FrtDocument *frt_index_get_doc_ts(FrtIndex *self, int doc_num);
extern FrtDocument *frt_index_get_doc_id(FrtIndex *self, const char *id);
extern FrtDocument *frt_index_get_doc_term(FrtIndex *self, const char *field,
                                    const char *term);
extern void frt_index_delete(FrtIndex *self, int doc_num);
extern void frt_index_delete_term(FrtIndex *self, const char *field, const char *term);
extern void frt_index_delete_id(FrtIndex *self, const char *id);
extern void frt_index_delete_query(FrtIndex *self, Query *q, FrtFilter *f, FrtPostFilter *pf);
extern void frt_index_delete_query_str(FrtIndex *self, char *qstr,
                                   FrtFilter *f, FrtPostFilter *pf);
extern int frt_index_term_id(FrtIndex *self, const char *field, const char *term);
extern Explanation *frt_index_explain(FrtIndex *self, Query *q, int doc_num);
extern void frt_index_auto_flush_ir(FrtIndex *self);
extern void frt_index_auto_flush_iw(FrtIndex *self);

extern FRT_INLINE void frt_ensure_searcher_open(FrtIndex *self);
extern FRT_INLINE void frt_ensure_reader_open(FrtIndex *self);
extern FRT_INLINE void frt_ensure_writer_open(FrtIndex *self);

#endif

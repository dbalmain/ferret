#ifndef FRT_IND_H
#define FRT_IND_H

#include "search.h"
#include "index.h"

/***************************************************************************
 *
 * Index
 *
 ***************************************************************************/

typedef struct Index
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
} Index;

extern Index *index_new(Store *store, FrtAnalyzer *analyzer,
                        FrtHashSet *def_fields, bool create);
extern void index_destroy(Index *self);
extern void index_flush(Index *self);
extern int index_size(Index *self);
extern void index_optimize(Index *self);
extern bool index_has_del(Index *self);
extern bool index_is_deleted(Index *self, int doc_num);
extern void index_add_doc(Index *self, FrtDocument *doc);
extern void index_add_string(Index *self, char *str);
extern void index_add_array(Index *self, char **ary);
extern TopDocs *index_search_str(Index *self, char *query, int first_doc,
                                 int num_docs, Filter *filter,
                                 Sort *sort, PostFilter *post_filter);
extern Query *index_get_query(Index *self, char *qstr);
extern FrtDocument *index_get_doc(Index *self, int doc_num);
extern FrtDocument *index_get_doc_ts(Index *self, int doc_num);
extern FrtDocument *index_get_doc_id(Index *self, const char *id);
extern FrtDocument *index_get_doc_term(Index *self, const char *field,
                                    const char *term);
extern void index_delete(Index *self, int doc_num);
extern void index_delete_term(Index *self, const char *field, const char *term);
extern void index_delete_id(Index *self, const char *id);
extern void index_delete_query(Index *self, Query *q, Filter *f, PostFilter *pf);
extern void index_delete_query_str(Index *self, char *qstr,
                                   Filter *f, PostFilter *pf);
extern int index_term_id(Index *self, const char *field, const char *term);
extern Explanation *index_explain(Index *self, Query *q, int doc_num);
extern void index_auto_flush_ir(Index *self);
extern void index_auto_flush_iw(Index *self);

extern FRT_INLINE void ensure_searcher_open(Index *self);
extern FRT_INLINE void ensure_reader_open(Index *self);
extern FRT_INLINE void ensure_writer_open(Index *self);

#endif

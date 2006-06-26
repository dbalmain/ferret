#include "search.h"
#include <string.h>

/***************************************************************************
 *
 * Filter
 *
 ***************************************************************************/

void filt_destroy(Filter *filt)
{
    h_destroy(filt->cache);
    free(filt->name);
    free(filt);
}

BitVector *filt_get_bv(Filter *filt, IndexReader *ir)
{
    CacheObject *co = h_get(filt->cache, ir);

    if (!co) {
        BitVector *bv;
        if (!ir->cache) {
            ir_add_cache(ir);
        }
        bv = filt->get_bv(filt, ir);
        co = co_create(filt->cache, ir->cache, filt, ir,
                       (free_ft)&bv_destroy, (void *)bv);
    }
    return (BitVector *)co->obj;
}

static char *filt_to_s_i(Filter *filt)
{
    return estrdup(filt->name);
}

f_u32 filt_hash_default(Filter *filt)
{
    (void)filt;
    return 0;
}

int filt_eq_default(Filter *filt, Filter *o)
{
    (void)filt; (void)o;
    return false;
}

Filter *filt_create(size_t size, const char *name)
{
    Filter *filt    = (Filter *)emalloc(size);
    filt->cache     = co_hash_create();
    filt->name      = estrdup(name);
    filt->to_s      = &filt_to_s_i;
    filt->hash      = &filt_hash_default;
    filt->eq        = &filt_eq_default;
    filt->destroy   = &filt_destroy;
    return filt;
}

f_u32 filt_hash(Filter *filt)
{
    return str_hash(filt->name) ^ filt->hash(filt);
}

int filt_eq(Filter *filt, Filter *o)
{
    return ((filt == o)
            || ((strcmp(filt->name, o->name) == 0)
                && (filt->eq == o->eq)
                && (filt->eq(filt, o))));
}

/***************************************************************************
 *
 * QueryFilter
 *
 ***************************************************************************/

#define QF(filt) ((QueryFilter *)(filt))
typedef struct QueryFilter
{
    Filter f;
    Query *query;
} QueryFilter;

static char *qfilt_to_s(Filter *filt)
{
    Query *query = QF(filt)->query;
    char *query_str = query->to_s(query, "");
    char *filter_str = strfmt("QueryFilter< %s >", query_str);
    free(query_str);
    return filter_str;
}

static BitVector *qfilt_get_bv(Filter *filt, IndexReader *ir)
{
    BitVector *bv = bv_new_capa(ir->max_doc(ir));
    Searcher *sea = stdsea_new(ir);
    Weight *weight = q_weight(QF(filt)->query, sea);
    Scorer *scorer = weight->scorer(weight, ir);
    if (scorer) {
        while (scorer->next(scorer)) {
            bv_set(bv, scorer->doc);
        }
        scorer->destroy(scorer);
    }
    weight->destroy(weight);
    free(sea);
    return bv;
}

static f_u32 qfilt_hash(Filter *filt)
{
    return q_hash(QF(filt)->query);
}

static int qfilt_eq(Filter *filt, Filter *o)
{
    return q_eq(QF(filt)->query, QF(o)->query);
}

static void qfilt_destroy(Filter *filt)
{
    Query *query = QF(filt)->query;
    q_deref(query);
    filt_destroy(filt);
}

Filter *qfilt_create(Query *query)
{
    Filter *filt = filt_new(QueryFilter);
    QF(filt)->query = query;

    filt->get_bv    = &qfilt_get_bv;
    filt->hash      = &qfilt_hash;
    filt->eq        = &qfilt_eq;
    filt->to_s      = &qfilt_to_s;
    filt->destroy   = &qfilt_destroy;
    return filt;
}

#include <string.h>
#include "search.h"
#include "array.h"

/***************************************************************************
 *
 * Explanation
 *
 ***************************************************************************/

Explanation *expl_new(float value, const char *description, ...)
{
    Explanation *expl = ALLOC(Explanation);

    va_list args;
    va_start(args, description);
    expl->description = vstrfmt(description, args);
    va_end(args);

    expl->value = value;
    expl->details = ary_new_type_capa(Explanation *,
                                      EXPLANATION_DETAILS_START_SIZE);
    return expl;
}

void expl_destroy(Explanation *expl)
{
    ary_destroy((void **)expl->details, (free_ft)expl_destroy);
    free(expl->description);
    free(expl);
}

Explanation *expl_add_detail(Explanation *expl, Explanation *detail)
{
    ary_push(expl->details, detail);
    return expl;
}

char *expl_to_s_depth(Explanation *expl, int depth)
{
    int i;
    char *buffer = ALLOC_N(char, depth * 2 + 1);
    const int num_details = ary_size(expl->details);

    memset(buffer, ' ', sizeof(char) * depth * 2);
    buffer[depth*2] = 0;

    buffer = estrcat(buffer, strfmt("%f = %s\n", expl->value, expl->description));
    for (i = 0; i < num_details; i++) {
        buffer = estrcat(buffer, expl_to_s_depth(expl->details[i], depth + 1));
    }

    return buffer;
}

char *expl_to_html(Explanation *expl)
{
    int i;
    char *buffer;
    const int num_details = ary_size(expl->details);

    buffer = strfmt("<ul>\n<li>%f = %s</li>\n", expl->value, expl->description);

    for (i = 0; i < num_details; i++) {
        estrcat(buffer, expl_to_html(expl->details[i]));
    }

    REALLOC_N(buffer, char, strlen(buffer) + 10);
    return strcat(buffer, "</ul>\n");
}

/****************************************************************************
 *
 * Term
 *
 ****************************************************************************/

#define term_set_new() \
    hs_new((hash_ft)&term_hash, (eq_ft)&term_eq, (free_ft)&term_destroy)

Term *term_new(const char *field, const char *text)
{
    Term *t = ALLOC(Term);
    t->field = estrdup(field);
    t->text = estrdup(text);
    return t;
}

void term_destroy(Term *self)
{
    free(self->text);
    free(self->field);
    free(self);
}

int term_eq(const void *t1, const void *t2)
{
    return (strcmp(((Term *)t1)->text, ((Term *)t2)->text)) == 0 &&
        (strcmp(((Term *)t1)->field, ((Term *)t2)->field) == 0);
}

f_u32 term_hash(const void *t)
{
    return str_hash(((Term *)t)->text) * str_hash(((Term *)t)->field);
}

/***************************************************************************
 *
 * Hit
 *
 ***************************************************************************/

static bool hit_less_than(const Hit *hit1, const Hit *hit2)
{
    if (hit1->score == hit2->score) {
        return hit1->doc > hit2->doc;
    }
    else {
        return hit1->score < hit1->score;
    }
}

static bool hit_lt(Hit *hit1, Hit *hit2)
{
    if (hit1->score == hit2->score) {
        return hit1->doc > hit2->doc;
    }
    else {
        return hit1->score < hit2->score;
    }
}

static void hit_pq_down(PriorityQueue *pq)
{
    register int i = 1;
    register int j = 2;     /* i << 1; */
    register int k = 3;     /* j + 1;  */
    Hit **heap = (Hit **)pq->heap;
    Hit *node = heap[i];    /* save top node */

    if ((k <= pq->size) && hit_lt(heap[k], heap[j])) {
        j = k;
    }

    while ((j <= pq->size) && hit_lt(heap[j], node)) {
        heap[i] = heap[j];  /* shift up child */
        i = j;
        j = i << 1;
        k = j + 1;
        if ((k <= pq->size) && hit_lt(heap[k], heap[j])) {
            j = k;
        }
    }
    heap[i] = node;
}

static Hit *hit_pq_pop(PriorityQueue *pq)
{
    if (pq->size > 0) {
        Hit *result = (Hit *)pq->heap[1]; /* save first value */
        pq->heap[1] = pq->heap[pq->size]; /* move last to first */
        pq->heap[pq->size] = NULL;
        pq->size--;
        hit_pq_down(pq);                  /* adjust heap */
        return result;
    }
    else {
        return NULL;
    }
}

static void hit_pq_up(PriorityQueue *pq)
{
    Hit **heap = (Hit **)pq->heap;
    Hit *node;
    int i = pq->size;
    int j = i >> 1;
    node = heap[i];

    while ((j > 0) && hit_lt(node, heap[j])) {
        heap[i] = heap[j];
        i = j;
        j = j >> 1;
    }
    heap[i] = node;
}

static void hit_pq_insert(PriorityQueue *pq, Hit *hit) 
{
    if (pq->size < pq->capa) {
        Hit *new_hit = ALLOC(Hit);
        memcpy(new_hit, hit, sizeof(Hit));
        pq->size++;
        pq->heap[pq->size] = new_hit;
        hit_pq_up(pq);
    }
    else if (pq->size > 0 && hit_lt((Hit *)pq->heap[1], hit)) {
        memcpy(pq->heap[1], hit, sizeof(Hit));
        hit_pq_down(pq);
    }
}

/***************************************************************************
 *
 * TopDocs
 *
 ***************************************************************************/

TopDocs *td_new(int total_hits, int size, Hit **hits)
{
    TopDocs *td = ALLOC(TopDocs);
    td->total_hits = total_hits;
    td->size = size;
    td->hits = hits;
    return td;
}

void td_destroy(TopDocs *td)
{
    int i;
    for (i = 0; i < td->size; i++) {
        free(td->hits[i]);
    }
    free(td->hits);
    free(td);
}

char *td_to_s(TopDocs *td)
{
    int i;
    Hit *hit;
    char *buffer = strfmt("%d hits sorted by <score, doc_num>\n",
                          td->total_hits);
    for (i = 0; i < td->size; i++) {
        hit = td->hits[i];
        estrcat(buffer, strfmt("\t%d:%f\n", hit->doc, hit->score));
    }
    return buffer;
}

/***************************************************************************
 *
 * Weight
 *
 ***************************************************************************/

Query *w_get_query(Weight *self)
{
    return self->query;
}

float w_get_value(Weight *self)
{
    return self->value;
}

float w_sum_of_squared_weights(Weight *self)
{
    self->qweight = self->idf * self->query->boost;
    return self->qweight * self->qweight;   /* square it */
}

void w_normalize(Weight *self, float normalization_factor)
{
    self->qnorm = normalization_factor;
    self->qweight *= normalization_factor;  /* normalize query weight */
    self->value = self->qweight * self->idf;/* idf for document */
}

void w_destroy(Weight *self)
{
    q_deref(self->query);
    free(self);
}

Weight *w_create(size_t size, Query *query)
{
    Weight *self                    = (Weight *)ecalloc(size);
#ifdef DEBUG
    if (size < sizeof(Weight)) {
        RAISE(ERROR, "size of weight <%d> should be at least <%d>",
              size, sizeof(Weight));
    }
#endif
    REF(query);
    self->query                     = query;
    self->get_query                 = &w_get_query;
    self->get_value                 = &w_get_value;
    self->normalize                 = &w_normalize;
    self->destroy                   = &w_destroy;
    self->sum_of_squared_weights    = &w_sum_of_squared_weights;
    return self;
}

/***************************************************************************
 *
 * Query
 *
 ***************************************************************************/

static Query *q_rewrite(Query *self, IndexReader *ir)
{
    (void)ir;
    self->ref_cnt++;
    return self;
}

static void q_extract_terms(Query *self, HashSet *terms)
{
    /* do nothing by default */
    (void)self;
    (void)terms;
}

Similarity *q_get_similarity_i(Query *self, Searcher *searcher)
{
    (void)self;
    return searcher->get_similarity(searcher);
}

void q_destroy_i(Query *self)
{
    free(self);
}

void q_deref(Query *self)
{  
    if (--self->ref_cnt == 0) {
        self->destroy_i(self);
    }
}

Weight *q_create_weight_unsup(Query *self, Searcher *searcher)
{
    (void)self;
    (void)searcher;
    RAISE(UNSUPPORTED_ERROR,
          "Create weight is unsupported for this type of query");
    return NULL;
}

Weight *q_weight(Query *self, Searcher *searcher)
{
    Query      *query   = searcher->rewrite(searcher, self);
    Weight     *weight  = query->create_weight_i(query, searcher);
    float       sum     = weight->sum_of_squared_weights(weight);
    Similarity *sim     = query->get_similarity(query, searcher);
    float       norm    = sim_query_norm(sim, sum);
    q_deref(query);

    weight->normalize(weight, norm);
    return self->weight = weight;
}

#define BQ(query) ((BooleanQuery *)(query))
Query *q_combine(Query **queries, int q_cnt)
{
    int i;
    Query *q, *ret_q;
    HashSet *uniques = hs_new((hash_ft)&q_hash, (eq_ft)&q_eq, NULL);

    for (i = 0; i < q_cnt; i++) {
        q = queries[i];
        if (q->type == BOOLEAN_QUERY) {
            int j;
            bool splittable = true;
            if (BQ(q)->coord_disabled == false) {
                splittable = false;
            }
            else {
                for (j = 0; j < BQ(q)->clause_cnt; j++) {
                    if (BQ(q)->clauses[j]->occur != BC_SHOULD) {
                        splittable = false;
                        break;
                    }
                }
            }
            if (splittable) {
                for (j = 0; j < BQ(q)->clause_cnt; j++) {
                    q = BQ(q)->clauses[j]->query;
                    hs_add(uniques, q);
                }
            }
            else {
                hs_add(uniques, q);
            }
        }
        else {
            hs_add(uniques, q);
        }
    }
    if (uniques->size == 1) {
        ret_q = (Query *)uniques->elems[0]; 
        REF(ret_q);
    }
    else {
        ret_q = bq_new(true);
        for (i = 0; i < uniques->size; i++) {
            q = (Query *)uniques->elems[i];
            REF(q);
            bq_add_query(ret_q, q, BC_SHOULD);
        }
    }
    hs_destroy(uniques);

    return ret_q;
}

f_u32 q_hash(Query *self)
{
    return (self->hash(self) << 4) | self->type;
}

int q_eq(Query *self, Query *o)
{
    return (self == o)
        || ((self->type == o->type)
            && (self->boost == o->boost)
            && self->eq(self, o));
}

Query *q_create(size_t size)
{
    Query *self = (Query *)ecalloc(size);
#ifdef DEBUG
    if (size < sizeof(Query)) {
        RAISE(ERROR, "Size of a query <%d> should never be smaller than the "
              "size of a Query struct <%d>", size, sizeof(Query));
    }
#endif
    self->destroy_all       = true;
    self->boost             = 1.0;
    self->rewrite           = &q_rewrite;
    self->get_similarity    = &q_get_similarity_i;
    self->extract_terms     = &q_extract_terms;
    self->weight            = NULL;
    self->ref_cnt           = 1;
    return self;
}

/***************************************************************************
 *
 * Scorer
 *
 ***************************************************************************/

void scorer_destroy_i(Scorer *scorer)
{
    free(scorer);
}

Scorer *scorer_create(size_t size, Similarity *similarity)
{
    Scorer *self        = (Scorer *)ecalloc(size);
#ifdef DEBUG
    if (size < sizeof(Scorer)) {
        RAISE(ERROR, "size of scorer <%d> should be at least <%d>",
              size, sizeof(Scorer));
    }
#endif
    self->destroy       = &scorer_destroy_i;
    self->similarity    = similarity;
    return self;
}

bool scorer_less_than(void *p1, void *p2)
{
    Scorer *s1 = (Scorer *)p1;
    Scorer *s2 = (Scorer *)p2;
    return s1->score(s1) < s2->score(s2);
}

bool scorer_doc_less_than(const Scorer *s1, const Scorer *s2)
{
    return s1->doc < s2->doc;
}

int scorer_doc_cmp(const void *p1, const void *p2)
{
    return (*(Scorer **)p1)->doc - (*(Scorer **)p2)->doc;
}

/***************************************************************************
 *
 * StdSearcher
 *
 ***************************************************************************/

#define STDSEA(searcher) ((StdSearcher *)(searcher))

typedef struct StdSearcher {
    Searcher        super;
    IndexReader    *ir;
    bool            close_ir : 1;
} StdSearcher;

static int stdsea_doc_freq(Searcher *self, const char *field, const char *term)
{
    return ir_doc_freq(STDSEA(self)->ir, field, term);
}

static Document *stdsea_get_doc(Searcher *self, int doc_num)
{
    IndexReader *ir = STDSEA(self)->ir;
    return ir->get_doc(ir, doc_num);
}

static int stdsea_max_doc(Searcher *self)
{
    IndexReader *ir = STDSEA(self)->ir;
    return ir->max_doc(ir);
}

static Weight *sea_create_weight(Searcher *self, Query *query)
{
    return q_weight(query, self);
}

static void sea_check_args(int num_docs, int first_doc)
{
    if (num_docs <= 0) {
        RAISE(ARG_ERROR, ":num_docs was set to %d but should be greater "
              "than 0 : %d <= 0", num_docs, num_docs);
    }

    if (first_doc < 0) {
        RAISE(ARG_ERROR, ":first_doc was set to %d but should be greater "
              "than or equal to 0 : %d < 0", first_doc, first_doc);
    }
}

static TopDocs *stdsea_search(Searcher *self, Query *query, int first_doc,
                             int num_docs, Filter *filter, Sort *sort)
{
    int max_size = first_doc + num_docs;
    int i;
    Weight *weight;
    Scorer *scorer;
    Hit **score_docs = NULL;
    Hit hit;
    int total_hits = 0;
    float score;
    BitVector *bits = (filter
                       ? filter->get_bv(filter, STDSEA(self)->ir)
                       : NULL);
    Hit *(*hq_pop)(PriorityQueue *pq);
    void (*hq_insert)(PriorityQueue *pq, Hit *hit);
    void (*hq_destroy)(PriorityQueue *self);
    PriorityQueue *hq;

    sea_check_args(num_docs, first_doc);

    weight = q_weight(query, self);
    scorer = weight->scorer(weight, STDSEA(self)->ir);
    if (!scorer) {
        if (bits) {
            bv_destroy(bits);
        }
        weight->destroy(weight);
        return td_new(0, 0, NULL);
    }

    if (sort) {
        hq = fshq_pq_new(max_size, sort, STDSEA(self)->ir);
        hq_pop = &fshq_pq_pop;
        hq_insert = &fshq_pq_insert;
        hq_destroy = &fshq_pq_destroy;
    } else {
        hq = pq_new(max_size, (lt_ft)&hit_less_than, &free);
        hq_pop = &hit_pq_pop;
        hq_insert = &hit_pq_insert;
        hq_destroy = &pq_destroy;
    }

    while (scorer->next(scorer)) {
        if (bits && !bv_get(bits, scorer->doc)) {
            /* document has been filtered out */
            continue;
        }
        total_hits++;
        score = scorer->score(scorer);
        hit.doc = scorer->doc; hit.score = score;
        hq_insert(hq, &hit);
    }
    scorer->destroy(scorer);
    weight->destroy(weight);

    if (hq->size > first_doc) {
        if ((hq->size - first_doc) < num_docs) {
            num_docs = hq->size - first_doc;
        }
        score_docs = ALLOC_N(Hit *, num_docs);
        for (i = num_docs - 1; i >= 0; i--) {
            score_docs[i] = hq_pop(hq);
            /*
            hit = score_docs[i] = pq_pop(hq);
            printf("hit = %d-->%f\n", hit->doc, hit->score);
            */
        }
    }
    else {
        num_docs = 0;
    }
    pq_clear(hq);
    hq_destroy(hq);

    if (bits) {
        bv_destroy(bits);
    }
    return td_new(total_hits, num_docs, score_docs);
}

static void stdsea_search_each_w(Searcher *self, Weight *weight, Filter *filter,
                                 void (*fn)(Searcher *, int, float, void *),
                                 void *arg)
{
    Scorer *scorer;
    BitVector *bits = (filter
                       ? filter->get_bv(filter, STDSEA(self)->ir)
                       : NULL);

    scorer = weight->scorer(weight, STDSEA(self)->ir);
    if (!scorer) {
        if (bits) {
            bv_destroy(bits);
        }
        return;
    }

    while (scorer->next(scorer)) {
        if (bits && !bv_get(bits, scorer->doc)) {
            /* doc has been filtered */
            continue;
        }
        fn(self, scorer->doc, scorer->score(scorer), arg);
    }
    scorer->destroy(scorer);
}

static void stdsea_search_each(Searcher *self, Query *query, Filter *filter,
                               void (*fn)(Searcher *, int, float, void *),
                               void *arg)
{
    Weight *weight = q_weight(query, self);
    stdsea_search_each_w(self, weight, filter, fn, arg);
    weight->destroy(weight);
}

static Query *stdsea_rewrite(Searcher *self, Query *original)
{
    int q_is_destroyed = false;
    Query *query = original;
    Query *rewritten_query = query->rewrite(query, STDSEA(self)->ir);
    while (q_is_destroyed || (query != rewritten_query)) {
        query = rewritten_query;
        rewritten_query = query->rewrite(query, STDSEA(self)->ir);
        q_is_destroyed = (query->ref_cnt <= 1);
        q_deref(query); /* destroy intermediate queries */
    }
    return query;
}

static Explanation *stdsea_explain(Searcher *self, Query *query, int doc_num)
{
    Weight *weight = q_weight(query, self);
    Explanation *e =  weight->explain(weight, STDSEA(self)->ir, doc_num);
    weight->destroy(weight);
    return e;
}

static Explanation *stdsea_explain_w(Searcher *self, Weight *w, int doc_num)
{
    return w->explain(w, STDSEA(self)->ir, doc_num);
}

static Similarity *sea_get_similarity(Searcher *self)
{
    return self->similarity;
}

static void stdsea_close(Searcher *self)
{
    if (STDSEA(self)->ir && STDSEA(self)->close_ir) {
        ir_close(STDSEA(self)->ir);
    }
    free(self);
}

Searcher *stdsea_new(IndexReader *ir)
{
    Searcher *self          = (Searcher *)ecalloc(sizeof(StdSearcher));

    STDSEA(self)->ir        = ir;
    STDSEA(self)->close_ir  = true;

    self->similarity        = sim_create_default();
    self->doc_freq          = &stdsea_doc_freq;
    self->get_doc           = &stdsea_get_doc;
    self->max_doc           = &stdsea_max_doc;
    self->create_weight     = &sea_create_weight;
    self->search            = &stdsea_search;
    self->search_each       = &stdsea_search_each;
    self->search_each_w     = &stdsea_search_each_w;
    self->rewrite           = &stdsea_rewrite;
    self->explain           = &stdsea_explain;
    self->explain_w         = &stdsea_explain_w;
    self->get_similarity    = &sea_get_similarity;
    self->close             = &stdsea_close;

    return self;
}

/***************************************************************************
 *
 * CachedDFSearcher
 *
 ***************************************************************************/

#define CDFSEA(searcher) ((CachedDFSearcher *)(searcher))
typedef struct CachedDFSearcher
{
    Searcher    super;
    HashTable  *df_map;
    int         max_doc;
} CachedDFSearcher;

static int cdfsea_doc_freq(Searcher *self, const char *field, const char *text)
{
    Term term;
    int *df;
    term.field = (char *)field;
    term.text = (char *)text;
    df = (int *)h_get(CDFSEA(self)->df_map, &term);
    return df ? *df : 0;
}

static Document *cdfsea_get_doc(Searcher *self, int doc_num)
{
    (void)self; (void)doc_num;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static int cdfsea_max_doc(Searcher *self)
{
    (void)self;
    return CDFSEA(self)->max_doc;
}

static Weight *cdfsea_create_weight(Searcher *self, Query *query)
{
    (void)self; (void)query;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static TopDocs *cdfsea_search(Searcher *self, Query *query, int first_doc,
                              int num_docs, Filter *filter, Sort *sort)
{
    (void)self; (void)query; (void)first_doc; (void)num_docs;
    (void)filter; (void)sort;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static void cdfsea_search_each(Searcher *self, Query *query, Filter *filter,
                               void (*fn)(Searcher *, int, float, void *),
                               void *arg)
{
    (void)self; (void)query; (void)filter; (void)fn; (void)arg;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
}

static void cdfsea_search_each_w(Searcher *self, Weight *w, Filter *filter,
                                 void (*fn)(Searcher *, int, float, void *),
                                 void *arg)
{
    (void)self; (void)w; (void)filter; (void)fn; (void)arg;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
}

static Query *cdfsea_rewrite(Searcher *self, Query *original)
{
    (void)self;
    original->ref_cnt++;
    return original;
}

static Explanation *cdfsea_explain(Searcher *self, Query *query, int doc_num)
{
    (void)self; (void)query; (void)doc_num;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static Explanation *cdfsea_explain_w(Searcher *self, Weight *w, int doc_num)
{
    (void)self; (void)w; (void)doc_num;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static Similarity *cdfsea_get_similarity(Searcher *self)
{
    (void)self;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static void cdfsea_close(Searcher *self)
{
    h_destroy(CDFSEA(self)->df_map);
    free(self);
}

static Searcher *cdfsea_new(HashTable *df_map, int max_doc)
{
    Searcher *self          = (Searcher *)ecalloc(sizeof(CachedDFSearcher));

    CDFSEA(self)->df_map    = df_map;
    CDFSEA(self)->max_doc   = max_doc;

    self->doc_freq          = &cdfsea_doc_freq;
    self->get_doc           = &cdfsea_get_doc;
    self->max_doc           = &cdfsea_max_doc;
    self->create_weight     = &cdfsea_create_weight;
    self->search            = &cdfsea_search;
    self->search_each       = &cdfsea_search_each;
    self->search_each_w     = &cdfsea_search_each_w;
    self->rewrite           = &cdfsea_rewrite;
    self->explain           = &cdfsea_explain;
    self->explain_w         = &cdfsea_explain_w;
    self->get_similarity    = &cdfsea_get_similarity;
    self->close             = &cdfsea_close;
    return self;
}

/***************************************************************************
 *
 * MultiSearcher
 *
 ***************************************************************************/

#define MSEA(searcher) ((MultiSearcher *)(searcher))
typedef struct MultiSearcher
{
    Searcher    super;
    int         s_cnt;
    Searcher  **searchers;
    int        *starts;
    int         max_doc;
    bool        close_subs : 1;
} MultiSearcher;

static inline int msea_get_searcher_index(Searcher *self, int n)
{
    MultiSearcher *msea = MSEA(self);
    int lo = 0;                 /* search starts array */
    int hi = msea->s_cnt - 1;   /* for 1st element < n, return its index */
    int mid, mid_val;

    while (hi >= lo) {
        mid = (lo + hi) >> 1;
        mid_val = msea->starts[mid];
        if (n < mid_val) {
            hi = mid - 1;
        }
        else if (n > mid_val) {
            lo = mid + 1;
        }
        else {                  /* found a match */
            while (((mid+1) < msea->s_cnt)
                   && (msea->starts[mid+1] == mid_val)) {
                mid++;          /* scan to last match */
            }
            return mid;
        }
    }
    return hi;
}

static int msea_doc_freq(Searcher *self, const char *field, const char *term)
{
    int i;
    int doc_freq = 0;
    MultiSearcher *msea = MSEA(self);
    for (i = 0; i < msea->s_cnt; i++) {
        Searcher *s = msea->searchers[i];
        doc_freq += s->doc_freq(s, field, term);
    }

    return doc_freq;
}

static Document *msea_get_doc(Searcher *self, int doc_num)
{
    MultiSearcher *msea = MSEA(self);
    int i = msea_get_searcher_index(self, doc_num);
    Searcher *s = msea->searchers[i];
    return s->get_doc(s, doc_num - msea->starts[i]);
}

static int msea_max_doc(Searcher *self)
{
    return MSEA(self)->max_doc;
}

static int *msea_get_doc_freqs(Searcher *self, HashSet *terms)
{
    int i;
    const int num_terms = terms->size;
    int *doc_freqs = ALLOC_N(int, num_terms);
    for (i = 0; i < num_terms; i++) {
        Term *t = (Term *)terms->elems[i];
        doc_freqs[i] = msea_doc_freq(self, t->field, t->text);
    }
    return doc_freqs;
}

static Weight *msea_create_weight(Searcher *self, Query *query)
{
    int i, *doc_freqs;
    Searcher *cdfsea;
    Weight *w;
    HashTable *df_map = h_new((hash_ft)&term_hash, (eq_ft)&term_eq,
                             (free_ft)NULL, (free_ft)NULL);
    Query *rewritten_query = self->rewrite(self, query);
    HashSet *terms = term_set_new();

    rewritten_query->extract_terms(rewritten_query, terms);
    doc_freqs = msea_get_doc_freqs(self, terms);

    for (i = 0; i < terms->size; i++) {
        h_set(df_map, terms->elems[i], (void *)doc_freqs[i]); 
    }
    hs_destroy(terms);
    free(doc_freqs);

    cdfsea = cdfsea_new(df_map, MSEA(self)->max_doc);

    w = q_weight(rewritten_query, cdfsea);
    q_deref(rewritten_query);
    cdfsea->close(cdfsea);

    return w;
}

struct MultiSearchEachArg {
    int start;
    void *arg;
    void (*fn)(Searcher *, int, float, void *);
};

void msea_search_each_i(Searcher *self, int doc_num, float score, void *arg)
{
    struct MultiSearchEachArg *mse_arg = (struct MultiSearchEachArg *)arg;

    mse_arg->fn(self, doc_num + mse_arg->start, score, mse_arg->arg);
}

static void msea_search_each_w(Searcher *self, Weight *w, Filter *filter,
                               void (*fn)(Searcher *, int, float, void *),
                               void *arg)
{
    int i;
    struct MultiSearchEachArg mse_arg;
    MultiSearcher *msea = MSEA(self);
    Searcher *s;

    mse_arg.fn = fn;
    mse_arg.arg = arg;
    for (i = 0; i < msea->s_cnt; i++) {
        s = msea->searchers[i];
        mse_arg.start = msea->starts[i];
        s->search_each_w(s, w, filter, &msea_search_each_i, &mse_arg);
    }
}

static void msea_search_each(Searcher *self, Query *query, Filter *filter,
                             void (*fn)(Searcher *, int, float, void *), void *arg)
{
    Weight *w = q_weight(query, self);
    msea_search_each_w(self, w, filter, fn, arg);
    w->destroy(w);
}

struct MultiSearchArg {
    int total_hits, max_size;
    float min_score;
    PriorityQueue *hq;
    void (*hq_insert)(PriorityQueue *pq, Hit *hit);
};

void msea_search_i(Searcher *self, int doc_num, float score, void *arg)
{
    struct MultiSearchArg *ms_arg = (struct MultiSearchArg *)arg;
    Hit hit;
    (void)self;

    ms_arg->total_hits++;
    hit.doc = doc_num;
    hit.score = score;
    ms_arg->hq_insert(ms_arg->hq, &hit);
}

static TopDocs *msea_search(Searcher *self, Query *query, int first_doc,
                            int num_docs, Filter *filter, Sort *sort)
{
    int max_size = first_doc + num_docs;
    int i;
    Weight *weight;
    Hit **score_docs = NULL;
    Hit *(*hq_pop)(PriorityQueue *pq);
    void (*hq_insert)(PriorityQueue *pq, Hit *hit);
    void (*hq_destroy)(PriorityQueue *self);
    PriorityQueue *hq;
    struct MultiSearchArg ms_arg;
    //FIXME
    (void)sort;

    sea_check_args(num_docs, first_doc);

    weight = q_weight(query, self);
    hq = pq_new(max_size, (lt_ft)&hit_less_than, NULL);
    hq_pop = &hit_pq_pop;
    hq_insert = &hit_pq_insert;
    hq_destroy = &pq_destroy;


    ms_arg.hq = hq;
    ms_arg.total_hits = 0;
    ms_arg.max_size = max_size;
    ms_arg.min_score = 0.0;
    ms_arg.hq_insert = hq_insert;

    msea_search_each_w(self, weight, filter, &msea_search_i, &ms_arg);

    weight->destroy(weight);

    if (hq->size > first_doc) {
        if ((hq->size - first_doc) < num_docs) {
            num_docs = hq->size - first_doc;
        }
        score_docs = ALLOC_N(Hit *, num_docs);
        for (i = num_docs - 1; i >= 0; i--) {
            score_docs[i] = hq_pop(hq);
            /*
            hit = score_docs[i] = pq_pop(hq);
            printf("hit = %d-->%f\n", hit->doc, hit->score);
            */
        }
    } else {
        num_docs = 0;
    }
    pq_clear(hq);
    hq_destroy(hq);

    return td_new(ms_arg.total_hits, num_docs, score_docs);
}

static Query *msea_rewrite(Searcher *self, Query *original)
{
    int i;
    Searcher *s;
    MultiSearcher *msea = MSEA(self);
    Query **queries = ALLOC_N(Query *, msea->s_cnt), *rewritten;

    for (i = 0; i < msea->s_cnt; i++) {
        s = msea->searchers[i];
        queries[i] = s->rewrite(s, original);
    }
    rewritten = q_combine(queries, msea->s_cnt);

    for (i = 0; i < msea->s_cnt; i++) {
        q_deref(queries[i]);
    }
    free(queries);
    return rewritten;
}

static Explanation *msea_explain(Searcher *self, Query *query, int doc_num)
{
    MultiSearcher *msea = MSEA(self);
    int i = msea_get_searcher_index(self, doc_num);
    Weight *w = q_weight(query, self);
    Searcher *s = msea->searchers[i];
    Explanation *e = s->explain_w(s, w, doc_num - msea->starts[i]);
    w->destroy(w);
    return e;
}

static Explanation *msea_explain_w(Searcher *self, Weight *w, int doc_num)
{
    MultiSearcher *msea = MSEA(self);
    int i = msea_get_searcher_index(self, doc_num);
    Searcher *s = msea->searchers[i];
    Explanation *e = s->explain_w(s, w, doc_num - msea->starts[i]);
    return e;
}

static Similarity *msea_get_similarity(Searcher *self)
{
    return self->similarity;
}

static void msea_close(Searcher *self)
{
    int i;
    Searcher *s;
    MultiSearcher *msea = MSEA(self);
    if (msea->close_subs) {
        for (i = 0; i < msea->s_cnt; i++) {
            s = msea->searchers[i];
            s->close(s);
        }
        free(msea->searchers);
    }
    free(msea->starts);
    free(msea);
    free(self);
}

Searcher *msea_new(Searcher **searchers, int s_cnt, bool close_subs)
{
    int i, max_doc = 0;
    Searcher *self = (Searcher *)ecalloc(sizeof(MultiSearcher));
    int *starts = ALLOC_N(int, s_cnt + 1);
    for (i = 0; i < s_cnt; i++) {
        starts[i] = max_doc;
        max_doc += searchers[i]->max_doc(searchers[i]);
    }
    starts[i] = max_doc;

    MSEA(self)->s_cnt           = s_cnt;
    MSEA(self)->searchers       = searchers;
    MSEA(self)->starts          = starts;
    MSEA(self)->max_doc         = max_doc;
    MSEA(self)->close_subs      = close_subs;

    self->similarity            = sim_create_default();
    self->doc_freq              = &msea_doc_freq;
    self->get_doc               = &msea_get_doc;
    self->max_doc               = &msea_max_doc;
    self->create_weight         = &msea_create_weight;
    self->search                = &msea_search;
    self->search_each           = &msea_search_each;
    self->search_each_w         = &msea_search_each_w;
    self->rewrite               = &msea_rewrite;
    self->explain               = &msea_explain;
    self->explain_w             = &msea_explain_w;
    self->get_similarity        = &msea_get_similarity;
    self->close                 = &msea_close;
    return self;
}

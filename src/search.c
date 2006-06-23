#include <string.h>
#include "search.h"
#include "array.h"

/***************************************************************************
 *
 * Explanation
 *
 ***************************************************************************/

Explanation *expl_new(float value, char *description)
{
    Explanation *expl = ALLOC(Explanation);
    expl->value = value;
    expl->description = description;
    expl->details = ary_new_type_capa(Explanation *,
                                      EXPLANATION_DETAILS_START_SIZE);
    return expl;
}

void expl_destoy(Explanation *expl)
{
    ary_destroy((void **)expl, (free_ft)expl_destoy);
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

/***************************************************************************
 *
 * Hit
 *
 ***************************************************************************/

static bool hit_less_than(void *hit1, void *hit2)
{
    if (((Hit *)hit1)->score == ((Hit *)hit2)->score) {
        return ((Hit *)hit1)->doc > ((Hit *)hit2)->doc;
    }
    else {
        return ((Hit *)hit1)->score < ((Hit *)hit2)->score;
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
    if (pq->size < pq->size) {
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

TopDocs *td_create(int total_hits, int size, Hit **hits)
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
#ifdef
    ref(query);
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

Query *q_combine(Query **queries, int q_cnt)
{
    int i;
    Query *q, *ret_q;
    HashSet *uniques = hs_create((hash_ft)&q_hash, (eq_ft)&q_eq, NULL);

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
        ref(ret_q);
    }
    else {
        ret_q = bq_create(true);
        for (i = 0; i < uniques->size; i++) {
            q = (Query *)uniques->elems[i];
            ref(q);
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

void *scorer_destroy_i(Scorer *scorer)
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
#ifdef
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

bool scorer_doc_less_than(void *p1, void *p2)
{
    return ((Scorer *)p1)->doc < ((Scorer *)p2)->doc;
}

int scorer_doc_cmp(const void *p1, const void *p2)
{
    return (*(Scorer **)p1)->doc - (*(Scorer **)p2)->doc;
}

/***************************************************************************
 *
 * Searcher
 *
 ***************************************************************************/

struct Searcher {
    IndexReader *ir;
    Similarity  *similarity;
    int          (*doc_freq)(Searcher *self, Term *term);
    int         *(*doc_freqs)(Searcher *self, Term **terms, int tcnt);
    Document    *(*get_doc)(Searcher *self, int doc_num);
    int          (*max_doc)(Searcher *self);
    Weight      *(*create_weight)(Searcher *self, Query *query);
    TopDocs     *(*search)(Searcher *self, Query *query, int first_doc,
                           int num_docs, Filter *filter, Sort *sort);
    void         (*search_each)(Searcher *self, Query *query, Filter *filter,
                                void (*fn)(Searcher *, int, float, void *), void *arg);
    void         (*search_each_w)(Searcher *self, Weight *weight,
                                  Filter *filter,
                                  void (*fn)(Searcher *, int, float, void *),
                                  void *arg);
    Query       *(*rewrite)(Searcher *self, Query *original);
    Explanation *(*explain)(Searcher *self, Query *query, int doc_num);
    Explanation *(*explain_w)(Searcher *self, Weight *weight, int doc_num);
    Similarity  *(*get_similarity)(Searcher *self);
    void         (*close)(Searcher *self);
    bool         close_ir : 1;
};

#define sea_doc_freq(s, t)  s->doc_freq(s, t)
#define sea_doc_freqs(s, t, c)  s->doc_freqs(s, t, c)
#define sea_get_doc(s, dn)  s->get_doc(s, dn)
#define sea_max_doc(s)  s->max_doc(s)
#define sea_search(s, q, fd, nd, filt, sort)\
    s->search(s, q, fd, nd, filt, sort)
#define sea_search_each(s, q, filt, fn, arg)\
    s->search_each(s, q, filt, fn, arg)
#define sea_search_each_w(s, q, filt, fn, arg)\
    s->search_each_w(s, q, filt, fn, arg)
#define sea_rewrite(s, q)  s->rewrite(s, q)
#define sea_explain(s, q, dn)  s->explain(s, q, dn)
#define sea_explain_w(s, q, dn)  s->explain_w(s, q, dn)
#define sea_get_similarity(s)  s->get_similarity(s)
#define sea_close(s)  s->close(s)

extern Searcher *sea_create(IndexReader *ir);


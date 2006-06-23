#include <string.h>
#include "search.h"

#define PHQ(query) ((PhraseQuery *)(query))

/***************************************************************************
 *
 * PhraseScorer
 *
 ***************************************************************************/

/***************************************************************************
 * PhrasePosition
 ***************************************************************************/

#define PP(p) ((PhrasePosition *)(p))
typedef struct PhrasePosition
{
    TermDocEnum *tpe;
    int offset;
    int count;
    int doc;
    int position;
} PhrasePosition;

static bool pp_next(PhrasePosition *self)
{
    TermDocEnum *tpe = self->tpe;
    if (!tpe->next(tpe)) {
        tpe->close(tpe);            /* close stream */
        self->tpe = NULL;
        self->doc = INT_MAX;        /* sentinel value */
        return false;
    }
    self->doc = tpe->doc_num(tpe);
    self->position = 0;
    return true;
}

static bool pp_skip_to(PhrasePosition *self, int doc_num)
{
    TermDocEnum *tpe = self->tpe;
    if (!tpe->skip_to(tpe, doc_num)) {
        tpe->close(tpe);            /* close stream */
        self->tpe = NULL;
        self->doc = INT_MAX;        /* sentinel value */
        return false;
    }
    self->doc = tpe->doc_num(tpe);
    self->position = 0;
    return true;
}

static bool pp_next_position(PhrasePosition *self)
{
    TermDocEnum *tpe = self->tpe;
    self->count -= 1;
    if (self->count >= 0) {         /* read subsequent pos's */
        self->position = tpe->next_position(tpe) - self->offset;
        return true;
    }
    else {
        return false;
    }
}

static bool pp_first_position(PhrasePosition *self)
{
    TermDocEnum *tpe = self->tpe;
    self->count = tpe->freq(tpe);   /* read first pos */
    return pp_next_position(self);
}

static char *pp_to_s(PhrasePosition *self)
{
    return strfmt("pp->(doc => %d, position => %d)", self->doc, self->position);
}

#define PP_pp(p) (*(PhrasePosition **)p)
static int pp_cmp(const void *const p1, const void *const p2)
{
    int cmp = PP_pp(p1)->doc - PP_pp(p2)->doc;
    if (cmp == 0) {
        return PP_pp(p1)->position - PP_pp(p2)->position;
    }
    else {
        return cmp;
    }
}

static int pp_pos_cmp(const void *const p1, const void *const p2)
{
    return PP_pp(p1)->position - PP_pp(p2)->position;
}

static bool pp_less_than(void *p1, void *p2)
{
    /* docs will all be equal when this method is used */
    return PP(p)->position < PP(p)->position;
    /*
    if (PP(p)->doc == PP(p)->doc) {
        return PP(p)->position < PP(p)->position;
    }
    else {
        return PP(p)->doc < PP(p)->doc;
    }
    */
}

void pp_destroy(PhrasePosition *pp)
{
    if (pp->tpe) {
        pp->tpe->close(pp->tpe);
    }
    free(pp);
}

PhrasePosition *pp_create(TermDocEnum *tpe, int offset)
{
    PhrasePosition *self = ALLOC(PhrasePosition);

    self->tpe = tpe;
    self->count = self->doc = self->position = -1;
    self->offset = offset;

    return self;
}

/***************************************************************************
 * PhraseScorer
 ***************************************************************************/

#define PHSC(scorer) ((PhraseScorer *)(scorer))

typedef struct PhraseScorer
{
    Scorer              super;
    float               (*phrase_freq)(Scorer *self);
    float               freq;
    uchar              *norms;
    float               value;
    Weight             *weight;
    PhrasePosition    **phrase_pos;
    int                 pp_first_idx;
    int                 pp_cnt;
    int                 slop;
    bool                first_time : 1;
    bool                more : 1;
} PhraseScorer;

static void phsc_init(PhraseScorer *phsc)
{
    int i;
    for (i = phsc->pp_cnt - 1; i >= 0; i--) {
        if (!(phsc->more = pp_next(phsc->phrase_pos[i]))) break;
    }

    if (phsc->more) {
        qsort(phsc->phrase_pos, phsc->pp_cnt,
              sizeof(PhrasePosition *), &pp_cmp);
        phsc->pp_first_idx = 0;
    }
}

static bool phsc_do_next(Scorer *self)
{
    PhraseScorer *phsc = PHSC(self);
    const int pp_cnt = phsc->pp_cnt;
    int pp_first_idx = phsc->pp_first_idx;
    PhrasePosition **phrase_positions;

    PhrasePosition *first = phrase_positions[pp_first_idx];
    PhrasePosition *last  = phrase_positions[(pp_first_idx - 1) % pp_cnt];

    while (phsc->more) {
        /* find doc with all the terms */
        while (phsc->more && first->doc < last->doc) {
            /* skip first upto last */
            phsc->more = pp_skip_to(first, last->doc);
            last = first;
            pp_first_idx = (pp_first_idx + 1) % pp_cnt;
            first = phrase_positions[pp_first_idx];
        }

        if (phsc->more) {
            /* pp_first_idx will be used by phrase_freq */
            phsc->pp_first_idx = pp_first_idx;

            /* found a doc with all of the terms */
            phsc->freq = phsc->phrase_freq(self);

            if (phsc->freq == 0.0) {            /* no match */
                /* continuing search so re-set first and last */
                pp_first_idx = phsc->pp_first_idx;
                first = phrase_positions[pp_first_idx];
                last =  phrase_positions[(pp_first_idx - 1) % pp_cnt];
                phsc->more = pp_next(last);     /* trigger further scanning */
            }
            else {
                self->doc = first->doc;
                return true;                    /* found a match */
            }

        }
    }
    return false;
}

static float phsc_score(Scorer *self)
{
    PhraseScorer *phsc = PHSC(self);
    float raw_score = sim_tf(self->similarity, phsc->freq) * phsc->value;
    /* normalize */
    return raw_score * sim_decode_norm(
        self->similarity,
        phsc->norms[phsc->phrase_pos[phsc->pp_first_idx]->doc]);
}

static bool phsc_next(Scorer *self)
{
    PhraseScorer *phsc = PHSC(self);
    if (phsc->first_time) {
        phsc_init(phsc);
        phsc->first_time = false;
    }
    else if (phsc->more) {
        /* trigger further scanning */
        phsc->more = pp_next(
            phsc->phrase_pos[(phsc->pp_first_idx - 1) % phsc->pp_cnt]);
    }

    return phsc_do_next(self);
}

static bool phsc_skip_to(Scorer *self, int doc_num)
{
    PhraseScorer *phsc = PHSC(self);
    int i;
    for (i = phsc->pp_cnt - 1; i >= 0; i--) {
        if (!(phsc->more = pp_skip_to(phsc->phrase_pos[i], doc_num))) {
            break;
        }
    }

    if (phsc->more) {
        qsort(phsc->phrase_pos, phsc->pp_cnt,
              sizeof(PhrasePosition *), &pp_cmp);
        phsc->pp_first_idx = 0;
    }
    return phsc_do_next(self);
}

static Explanation *phsc_explain(Scorer *self, int doc_num)
{
    PhraseScorer *phsc = PHSC(self);
    float phrase_freq;

    while (phsc_next(self) && self->doc < doc_num) {
    }

    phrase_freq = (self->doc == doc_num) ? phsc->freq : (float)0.0;
    return expl_create(sim_tf(self->similarity, phrase_freq), 
                       strfmt("tf(phrase_freq=%f)", phrase_freq));
}

static void phsc_destroy(Scorer *self)
{
    PhraseScorer *phsc = PHSC(self);
    int i;
    for (i = phsc->pp_cnt - 1; i >= 0; i--) {
        pp_destroy(phsc->phrase_pos[i]);
    }
    free(phsc->phrase_pos);
    scorer_destroy_i(self);
}

static Scorer *phsc_create(Weight *weight, TermDocEnum **term_pos_enum,
                           int *positions, int t_cnt, Similarity *similarity,
                           uchar *norms)
{
    int i;
    Scorer *self                = scorer_new(PhraseScorer, similarity);

    PHSC(self)->weight          = weight;
    PHSC(self)->norms           = norms;
    PHSC(self)->value           = weight->value;
    PHSC(self)->phrase_pos      = ALLOC_N(PhrasePosition *, t_cnt);
    PHSC(self)->pp_first_idx    = 0;
    PHSC(self)->pp_cnt          = t_cnt;
    PHSC(self)->slop            = 0;
    PHSC(self)->first_time      = true;
    PHSC(self)->more            = true;

    for (i = 0; i < t_cnt; i++) {
        PHSC(self)->phrase_pos[i] = pp_create(term_pos_enum[i], positions[i]);
    }

    self->data      = phsc;
    self->score     = &phsc_score;
    self->next      = &phsc_next;
    self->skip_to   = &phsc_skip_to;
    self->explain   = &phsc_explain;
    self->destroy   = &phsc_destroy;

    return self;
}

/***************************************************************************
 * ExactPhraseScorer
 ***************************************************************************/

static float ephsc_phrase_freq(Scorer *self)
{
    PhraseScorer *phsc = PHSC(self);
    int i;
    int pp_first_idx = 0;
    const int pp_cnt = phsc->pp_cnt;
    float freq = 0.0;
    PhrasePosition **phrase_positions = phsc->phrase_pos;
    PhrasePosition *first;
    PhrasePosition *last;

    for (i = 0; i < pp_cnt; i++) {
        pp_first_position(phrase_positions[i]);
    }
    qsort(phrase_positions, pp_cnt, sizeof(PhrasePosition *), &pp_pos_cmp);

    first = phrase_positions[0];
    last =  phrase_positions[pp_cnt - 1];

    /* scan to position with all terms */
    do {
        /* scan forward in first */
        while (first->position < last->position) {
            do {
                if (! pp_next_position(first)) {
                    /* maintain first position */
                    phsc->pp_first_idx = pp_first_idx;
                    return freq;
                }
            } while (first->position < last->position);
            last = first;
            pp_first_idx = (pp_first_idx + 1) % pp_cnt;
            first = phrase_positions[pp_first_idx];
        }
        freq += 1.0; /* all equal: a match */
    } while (pp_next_position(last));

    /* maintain first position */ 
    phsc->pp_first_idx = pp_first_idx;
    return freq;
}

static Scorer *exact_phrase_scorer_create(Weight *weight,
                                          TermDocEnum **term_pos_enum,
                                          int *positions, int t_cnt,
                                          Similarity *similarity, uchar *norms)
{
    Scorer *self =
        phsc_create(weight, term_pos_enum, positions, t_cnt, similarity, norms);

    PHSC(self)->phrase_freq = &ephsc_phrase_freq;
    return self;
}

/***************************************************************************
 * SloppyPhraseScorer
 ***************************************************************************/

static float sphsc_phrase_freq(Scorer *self)
{
    PhraseScorer *phsc = PHSC(self);
    PhrasePosition *pp;
    PriorityQueue *pq = pq_create(phsc->pp_cnt, &pp_less_than, NULL);
    const int pp_cnt = phsc->pp_cnt;

    int last_pos = 0, pos, next_pos, start, match_length, i;
    bool done = false;
    float freq = 0.0;

    for (i = 0; i < pp_cnt; i++) {
        pp = phsc->phrase_pos[i];
        pp_first_position(pp);
        if (pp->position > last_pos) {
            last_pos = pp->position;
        }
        pq_push(pq, pp);
    }

    do {
        pp = pq_pop(pq);
        pos = start = pp->position;
        next_pos = ((PhrasePosition *)pq_top(pq))->position;
        while (pos <= next_pos) {
            start = pos;        /* advance pp to min window */
            if (!pp_next_position(pp)) {
                done = true;    /* ran out of a positions for a term - done */
                break;
            }
            pos = pp->position;
        }

        match_length = last_pos - start;
        if (match_length <= phsc->slop) {
            /* score match */
            freq += sim_sloppy_freq(self->similarity, match_length);
        }

        if (pp->position > last_pos) {
            last_pos = pp->position;
        }
        pq_push(pq, pp);        /* restore pq */
    } while (!done);

    pq_destroy(pq);
    return freq;
}

static Scorer *sloppy_phrase_scorer_create(Weight *weight,
                                           TermDocEnum **term_pos_enum,
                                           int *positions, int t_cnt,
                                           Similarity *similarity, int slop,
                                           uchar *norms)
{
    Scorer *self =
        phsc_create(weight, term_pos_enum, positions, t_cnt, similarity, norms);

    PHSC(self)->slop        = slop;
    PHSC(self)->phrase_freq = &sphsc_phrase_freq;
    return self;
}

/***************************************************************************
 *
 * PhraseWeight
 *
 ***************************************************************************/

static Scorer *phw_scorer(Weight *self, IndexReader *ir)
{
    Scorer *phsc;
    PhraseQuery *phq = PHQ(self->query);
    int i;
    TermDocEnum **tps;

    if (phq->t_cnt == 0) {
        /* optimize zero-term case */
        return NULL;
    }

    tps = ALLOC_N(TermDocEnum *, phq->t_cnt);

    for (i = 0; i < phq->t_cnt; i++) {
        tps[i] = ir_term_positions_for(ir, phq->field, phq->terms[i]);
        if (tps[i] == NULL) {
            /* free everything we just created and return NULL */
            int j;
            for (j = 0; j < i; j++) {
                tps[i]->close(tps[i]);
            }
            free(tps);
            return NULL;
        }
    }

    if (phq->slop == 0) {/* optimize exact (common) case */
        phsc = exact_phrase_scorer_create(self, tps, phq->positions,
                                          phq->t_cnt, self->similarity,
                                          ir->get_norms(ir, phq->field));
    } else {
        phsc = sloppy_phrase_scorer_create(self, tps, phq->positions,
                                           phq->t_cnt, self->similarity,
                                           phq->slop,
                                           ir->get_norms(ir, phq->field));
    }
    free(tps);
    return phsc;
}

static Explanation *phw_explain(Weight *self, IndexReader *ir, int doc_num)
{
    Explanation *idf_expl1;
    Explanation *idf_expl2;
    Explanation *query_expl;
    Explanation *qnorm_expl;
    Explanation *field_expl;
    Explanation *tf_expl;
    Scorer *scorer;
    uchar *field_norms;
    float field_norm;
    Explanation *field_norm_expl;

    char *query_str = self->query->to_s(self->query, "");
    PhraseQuery *phq = (PhraseQuery *)self->query->data;
    int i;
    char *doc_freqs = NULL;
    size_t len = 0, pos = 0;

    Explanation *expl = expl_create(
        0.0, strfmt("weight(%s in %d), product of:", query_str, doc_num));

    for (i = 0; i < phq->t_cnt; i++) {
        len += strlen(phq->terms[i]->text) + 30;
    }
    doc_freqs = ALLOC_N(char, len);
    for (i = 0; i < phq->t_cnt; i++) {
        Term *term = phq->terms[i];
        sprintf(doc_freqs + pos, "%s=%d, ", term->text, ir->doc_freq(ir, term));
        pos += strlen(doc_freqs + pos);
    }
    pos -= 2; /* remove ", " from the end */
    doc_freqs[pos] = 0;

    idf_expl1 = expl_create(self->idf,
                            strfmt("idf(%s:<%s>)", phq->field, doc_freqs));
    idf_expl2 = expl_create(self->idf,
                            strfmt("idf(%s:<%s>)", phq->field, doc_freqs));
    free(doc_freqs);

    /* explain query weight */
    query_expl = expl_create(
        0.0, strfmt("query_weight(%s), product of:", query_str));

    if (self->query->boost != 1.0) {
        expl_add_detail(query_expl,
                        expl_create(self->query->boost, estrdup("boost")));
    }
    expl_add_detail(query_expl, idf_expl1);

    qnorm_expl = expl_create(self->qnorm, estrdup("query_norm"));
    expl_add_detail(query_expl, qnorm_expl);

    query_expl->value = self->query->boost * self->idf * self->qnorm;

    expl_add_detail(expl, query_expl);

    /* explain field weight */
    field_expl = expl_create(
        0.0, strfmt("field_weight(%s in %d), product of:", query_str, doc_num));
    free(query_str);

    scorer = self->scorer(self, ir);
    tf_expl = scorer->explain(scorer, doc_num);
    scorer->destroy(scorer);
    expl_add_detail(field_expl, tf_expl);
    expl_add_detail(field_expl, idf_expl2);

    field_norms = ir->get_norms(ir, phq->field);
    field_norm = (field_norms != NULL)
        ? sim_decode_norm(self->similarity, field_norms[doc_num])
        : (float)0.0;
    field_norm_expl = expl_create(
        field_norm, strfmt("field_norm(field=%s, doc=%d)",
                           phq->field, doc_num));

    expl_add_detail(field_expl, field_norm_expl);

    field_expl->value = tf_expl->value * self->idf * field_norm;

    /* combine them */
    if (query_expl->value == 1.0) {
        expl_destoy(expl);
        return field_expl;
    }
    else {
        expl->value = (query_expl->value * field_expl->value);
        expl_add_detail(expl, field_expl);
        return expl;
    }
}

static char *phw_to_s(Weight *self)
{
    return strfmt("PhraseWeight(%f)", self->value);
}

static Weight *phw_create(Query *query, Searcher *searcher)
{
    Weight *self = w_create(query);
    PhraseQuery *phq = (PhraseQuery *)query->data;

    self->scorer    = &phw_scorer;
    self->explain   = &phw_explain;
    self->to_s      = &phw_to_s;
    self->sum_of_squared_weights = &w_sum_of_squared_weights;

    self->similarity = query->get_similarity(query, searcher);
    self->value = query->boost;
    self->idf = sim_idf_phrase(self->similarity, phq->terms, phq->t_cnt, searcher);

    return self;
}

/***************************************************************************
 *
 * PhraseQuery
 *
 ***************************************************************************/

#define PHQ_INIT_CAPA 4
typedef struct PhraseQuery
{
    Query   super;
    int     slop;
    char   *field
    char  **terms;
    int    *positions;
    int     t_cnt;
    int     t_capa;
    char   *field;
} PhraseQuery;

static void phq_extract_terms(Query *self, HashSet *terms)
{
    PhraseQuery *phq = PHQ(self);
    int i;
    for (i = 0; i < phq->t_cnt; i++) {
        hs_add(terms, term_clone(phq->terms[i]));
    }
}

static char *phq_to_s(Query *self, char *field)
{
    PhraseQuery *phq = PHQ(self);
    int i, j, buf_index = 0, pos, last_pos = -1;
    size_t len = 0;
    char *buffer;
    if (!phq->t_cnt) {
        return NULL;
    }

    len = strlen(phq->field) + 1;

    for (i = 0; i < phq->t_cnt; i++) {
        len += strlen(phq->terms[i]->text) + 1;
    }
    /* add space for extra characters and boost and slop */
    len += 100 + 3 * phq->positions[phq->t_cnt - 1];

    buffer = ALLOC_N(char, len);

    if (strcmp(field, phq->field) != 0) {
        len = strlen(phq->field);
        memcpy(buffer, phq->field, len);
        buffer[len] = ':';
        buf_index += len + 1;
    }
    buffer[buf_index++] = '"';

    for (i = 0; i < phq->t_cnt; i++) {
        Term *term = phq->terms[i];
        pos = phq->positions[i];
        for (j = last_pos; j < pos - 1; j++) {
            memcpy(buffer + buf_index, "<> ", 3);
            buf_index += 3;
        }
        last_pos = pos;

        len = strlen(term->text);
        memcpy(buffer + buf_index, term->text, len);
        buf_index += len;
        buffer[buf_index++] = ' ';
    }
    if (buffer[buf_index-1] == ' ') {
        buf_index--;
    }
    buffer[buf_index++] = '"';
    buffer[buf_index] = 0;
    if (phq->slop != 0) {
        sprintf(buffer + buf_index, "~%d", phq->slop);
        buf_index += strlen(buffer + buf_index);
    }
    if (self->boost != 1.0) {
        buffer[buf_index++] = '^';
        dbl_to_s(buffer + buf_index, self->boost);
    }
    return buffer;
}

void phq_destroy(Query *self)
{
    PhraseQuery *phq = PHQ(self);
    int i;
    if (self->destroy_all) {
        for (i = 0; i < phq->t_cnt; i++) {
            term_destroy(phq->terms[i]);
        }
    }
    free(phq->terms);
    free(phq->positions);
    free(phq);

    q_destroy_i(self);
}

Query *phq_rewrite(Query *self, IndexReader *ir)
{
    PhraseQuery *phq = PHQ(self);
    if (phq->t_cnt == 1) {
        /* optimize one-term case */
        Query *tq = tq_create(estrdup(phq->field), estrdup(phq->term));
        tq->boost = self->boost;
        return tq;
    } else {
        self->ref_cnt++;
        return self;
    }
}

void phq_add_term(Query *self, Term *term, int pos_inc)
{
    PhraseQuery *phq = PHQ(self);
    int position, index = phq->t_cnt;
    if (index >= phq->t_capa) {
        phq->t_capa << 2;
        REALLOC_N(phq->terms, char *, phq->t_capa);
        REALLOC_N(phq->positions, int, phq->t_capa);
    }
    if (index == 0) {
        position = 0;
        phq->field = term->field;
    } 
    else {
        position = phq->positions[index - 1] + pos_inc;
        if (strcmp(term->field, phq->field) != 0) {
            RAISE(ARG_ERROR, FIELD_CHANGE_ERROR_MSG);
        }
    }
    phq->terms[index] = term;
    phq->positions[index] = position;
    phq->t_cnt++;
}

static f_u32 phq_hash(Query *self)
{
    int i;
    f_u32 hash = 0;
    PhraseQuery *phq = (PhraseQuery *)self->data;
    for (i = 0; i < phq->t_cnt; i++) {
        hash = (hash << 1) ^ (term_hash(phq->terms[i]) ^ phq->positions[i]);
    }
    return (hash ^ phq->slop);
}

static int phq_eq(Query *self, Query *o)
{
    int i;
    PhraseQuery *phq1 = PHQ(self);
    PhraseQuery *phq2 = PHQ(o);
    if (phq1->slop != phq2->slop || phq1->term != phq2->term) {
        return false;
    }
    for (i = 0; i < phq1->t_cnt; i++) {
        if (strcmp(phq1->terms[i], phq2->terms[i]) != 0
            || (phq1->positions[i] != phq2->positions[i])) {
            return false;
        }
    }
    return true;
}

Query *phq_create()
{
    Query *self = q_new(PhraseQuery);

    PHQ(self)->t_capa       = PHQ_INIT_CAPA;
    PHQ(self)->terms        = ALLOC_N(char *, PHQ_INIT_CAPA);
    PHQ(self)->positions    = ALLOC_N(int, PHQ_INIT_CAPA);

    self->type              = PHRASE_QUERY;
    self->rewrite           = &phq_rewrite;
    self->extract_terms     = &phq_extract_terms;
    self->to_s              = &phq_to_s;
    self->hash              = &phq_hash;
    self->eq                = &phq_eq;
    self->destroy_i         = &phq_destroy;
    self->create_weight_i   = &phw_create;
    return self;
}


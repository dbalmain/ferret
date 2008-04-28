#include <string.h>
#include "search.h"
#include "helper.h"
#include "internal.h"

/****************************************************************************
 *
 * FuzzyStuff
 *
 * The main method here is the fuzq_score_mn method which scores a term
 * against another term. The other methods all act in support.
 *
 * To learn more about the fuzzy scoring algorithm see;
 *
 *     http://en.wikipedia.org/wiki/Levenshtein_distance
 *
 ****************************************************************************/

/**
 * Calculate the maximum nomber of allowed edits (or maximum edit distance)
 * for a word to be a match.
 *
 * Note that fuzq->text_len and m are both the lengths text *after* the prefix
 * so `MIN(fuzq->text_len, m) + fuzq->pre_len)` actually gets the byte length
 * of the shorter string out of the query string and the index term being
 * compared.
 */
static INLINE int fuzq_calculate_max_distance(FuzzyQuery *fuzq, int m)
{
    return (int)((1.0 - fuzq->min_sim) * (MIN(fuzq->text_len, m) + fuzq->pre_len));
}

/**
 * The max-distance formula gets used a lot - it needs to be calculated for
 * every possible match in the index - so we cache the results for all
 * lengths up to the TYPICAL_LONGEST_WORD limit. For words longer than this we
 * calculate the value live.
 */
static void fuzq_initialize_max_distances(FuzzyQuery *fuzq)
{
    int i;
    for (i = 0; i < TYPICAL_LONGEST_WORD; i++) {
        fuzq->max_distances[i] = fuzq_calculate_max_distance(fuzq, i);
    }
}

/**
 * Return the cached max-distance value if the word is within the
 * TYPICAL_LONGEST_WORD limit.
 */
static INLINE int fuzq_get_max_distance(FuzzyQuery *fuzq, int m)
{
    if (m < TYPICAL_LONGEST_WORD)
        return fuzq->max_distances[m];
    return fuzq_calculate_max_distance(fuzq, m);
}

/**
 * Calculate the similarity score for the +target+ against the query.
 *
 * @params fuzq The Fuzzy Query
 * @params target *the term to compare against minus the prefix
 * @params m the string length of +target+
 * @params n the string length of the query string minus length of the prefix
 */
static INLINE float fuzq_score_mn(FuzzyQuery *fuzq,
                                  const char *target,
                                  const int m, const int n)
{
    int i, j, prune;
    int *d_curr, *d_prev;
    const char *text = fuzq->text;
    const int max_distance = fuzq_get_max_distance(fuzq, m);

    /* Just adding the characters of m to n or vice-versa results in
     * too many edits for example "pre" length is 3 and "prefixes"
     * length is 8. We can see that given this optimal circumstance,
     * the edit distance cannot be less than 5 which is 8-3 or more
     * precisesly Math.abs(3-8). If our maximum edit distance is 4,
     * then we can discard this word without looking at it. */
    if (max_distance < ABS(m-n)) {
        return 0.0f;
    }

    d_curr = fuzq->da;
    d_prev = d_curr + n + 1;

    /* init array */
    for (j = 0; j <= n; j++) {
        d_curr[j] = j;
    }

    /* start computing edit distance */
    for (i = 0; i < m;) {
        char s_i = target[i];
        /* swap d_current into d_prev */
        int *d_tmp = d_prev;
        d_prev = d_curr;
        d_curr = d_tmp;
        prune = (d_curr[0] = ++i) > max_distance;

        for (j = 0; j < n; j++) {
            d_curr[j + 1] = (s_i == text[j])
                ? min3(d_prev[j + 1] + 1, d_curr[j] + 1, d_prev[j])
                : min3(d_prev[j + 1], d_curr[j], d_prev[j]) + 1;
            if (prune && d_curr[j + 1] <= max_distance) {
                prune = false;
            }
        }
        if (prune) {
            return 0.0f;
        }
    }

    /* this will return less than 0.0 when the edit distance is greater
     * than the number of characters in the shorter word.  but this was
     * the formula that was previously used in FuzzyTermEnum, so it has
     * not been changed (even though min_sim must be greater than 0.0) */
    return 1.0f - ((float)d_curr[n] / (float) (fuzq->pre_len + min2(n, m)));
}

/**
 * The following algorithm is taken from Bob Carpenter's FuzzyTermEnum
 * implentation here;
 *
 * http://mail-archives.apache.org/mod_mbox/lucene-java-dev/200606.mbox/%3c448F0E8C.3050901@alias-i.com%3e
 */
float fuzq_score(FuzzyQuery *fuzq, const char *target)
{
    const int m = (int)strlen(target);
    const int n = fuzq->text_len;

    /* we don't have anything to compare.  That means if we just add
     * the letters for m we get the new word */
    if (m == 0 || n == 0) {
        if (fuzq->pre_len == 0)
            return 0.0f;
        return 1.0f - ((float) (m+n) / fuzq->pre_len);
    }

    return fuzq_score_mn(fuzq, target, m, n);
}

/****************************************************************************
 *
 * FuzzyQuery
 *
 ****************************************************************************/

#define FzQ(query) ((FuzzyQuery *)(query))

static char *fuzq_to_s(Query *self, Symbol curr_field)
{
    char *buffer, *bptr;
    char *term = FzQ(self)->term;
    Symbol field = FzQ(self)->field;
    bptr = buffer = ALLOC_N(char, strlen(term) + sym_len(field) + 70);

    if (curr_field != field) {
        bptr += sprintf(bptr, "%s:", S(field));
    }

    bptr += sprintf(bptr, "%s~", term);
    if (FzQ(self)->min_sim != 0.5) {
        dbl_to_s(bptr, FzQ(self)->min_sim);
        bptr += strlen(bptr);
    }

    if (self->boost != 1.0) {
        *bptr = '^';
        dbl_to_s(++bptr, self->boost);
    }

    return buffer;
}

static Query *fuzq_rewrite(Query *self, IndexReader *ir)
{
    Query *q;
    FuzzyQuery *fuzq = FzQ(self);

    int pre_len = fuzq->pre_len;
    char *prefix = NULL;
    const char *term = fuzq->term;
    const int field_num = fis_get_field_num(ir->fis, fuzq->field);
    TermEnum *te;

    if (field_num < 0) {
        return bq_new(true);
    }
    if (fuzq->pre_len >= (int)strlen(term)) {
        return tq_new(fuzq->field, term);
    }

    q = multi_tq_new_conf(fuzq->field, MTQMaxTerms(self), fuzq->min_sim);
    if (pre_len > 0) {
        prefix = ALLOC_N(char, pre_len + 1);
        strncpy(prefix, term, pre_len);
        prefix[pre_len] = '\0';
        te = ir->terms_from(ir, field_num, prefix);
    }
    else {
        te = ir->terms(ir, field_num);
    }

    assert(NULL != te);

    fuzq->scale_factor = (float)(1.0 / (1.0 - fuzq->min_sim));
    fuzq->text = term + pre_len;
    fuzq->text_len = (int)strlen(fuzq->text);
    fuzq->da = REALLOC_N(fuzq->da, int, fuzq->text_len * 2 + 2);
    fuzq_initialize_max_distances(fuzq);

    do {
        const char *curr_term = te->curr_term;
        const char *curr_suffix = curr_term + pre_len;
        float score = 0.0;

        if (prefix && strncmp(curr_term, prefix, pre_len) != 0)
            break;

        score = fuzq_score(fuzq, curr_suffix);
        multi_tq_add_term_boost(q, curr_term, score);
    } while (te->next(te) != NULL);

    te->close(te);
    if (prefix) free(prefix);
    return q;
}

static void fuzq_destroy(Query *self)
{
    free(FzQ(self)->term);
    free(FzQ(self)->da);
    q_destroy_i(self);
}

static unsigned long fuzq_hash(Query *self)
{
    return str_hash(FzQ(self)->term) ^ sym_hash(FzQ(self)->field)
        ^ float2int(FzQ(self)->min_sim) ^ FzQ(self)->pre_len;
}

static int fuzq_eq(Query *self, Query *o)
{
    FuzzyQuery *fq1 = FzQ(self);
    FuzzyQuery *fq2 = FzQ(o);

    return (strcmp(fq1->term, fq2->term) == 0)
        && (fq1->field == fq2->field)
        && (fq1->pre_len == fq2->pre_len)
        && (fq1->min_sim == fq2->min_sim);
}

Query *fuzq_new_conf(Symbol field, const char *term,
                     float min_sim, int pre_len, int max_terms)
{
    Query *self = q_new(FuzzyQuery);

    FzQ(self)->field      = field;
    FzQ(self)->term       = estrdup(term);
    FzQ(self)->pre_len    = pre_len ? pre_len : DEF_PRE_LEN;
    FzQ(self)->min_sim    = min_sim ? min_sim : DEF_MIN_SIM;
    MTQMaxTerms(self)     = max_terms ? max_terms : DEF_MAX_TERMS;

    self->type            = FUZZY_QUERY;
    self->to_s            = &fuzq_to_s;
    self->hash            = &fuzq_hash;
    self->eq              = &fuzq_eq;
    self->rewrite         = &fuzq_rewrite;
    self->destroy_i       = &fuzq_destroy;
    self->create_weight_i = &q_create_weight_unsup;

    return self;
}

Query *fuzq_new(Symbol field, const char *term)
{
    return fuzq_new_conf(field, term, 0.0f, 0, 0);
}

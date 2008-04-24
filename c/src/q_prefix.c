#include <string.h>
#include "search.h"
#include "symbol.h"
#include "internal.h"

/****************************************************************************
 *
 * PrefixQuery
 *
 ****************************************************************************/

#define PfxQ(query) ((PrefixQuery *)(query))

static char *prq_to_s(Query *self, Symbol default_field)
{
    char *buffer, *bptr;
    const char *prefix = PfxQ(self)->prefix;
    size_t plen = strlen(prefix);
    size_t flen = sym_len(PfxQ(self)->field);

    bptr = buffer = ALLOC_N(char, plen + flen + 35);

    if (PfxQ(self)->field != default_field) {
        bptr += sprintf(bptr, "%s:", S(PfxQ(self)->field));
    }

    bptr += sprintf(bptr, "%s*", prefix);
    if (self->boost != 1.0) {
        *bptr = '^';
        dbl_to_s(++bptr, self->boost);
    }

    return buffer;
}

static Query *prq_rewrite(Query *self, IndexReader *ir)
{
    const int field_num = fis_get_field_num(ir->fis, PfxQ(self)->field);
    Query *volatile q = multi_tq_new_conf(PfxQ(self)->field,
                                          MTQMaxTerms(self), 0.0);
    q->boost = self->boost;        /* set the boost */

    if (field_num >= 0) {
        const char *prefix = PfxQ(self)->prefix;
        TermEnum *te = ir->terms_from(ir, field_num, prefix);
        const char *term = te->curr_term;
        size_t prefix_len = strlen(prefix);

        TRY
            do {
                if (strncmp(term, prefix, prefix_len) != 0) {
                    break;
                }
                multi_tq_add_term(q, term);       /* found a match */
            } while (te->next(te));
        XFINALLY
            te->close(te);
        XENDTRY
    }

    return q;
}

static void prq_destroy(Query *self)
{
    free(PfxQ(self)->prefix);
    q_destroy_i(self);
}

static unsigned long prq_hash(Query *self)
{
    return sym_hash(PfxQ(self)->field) ^ str_hash(PfxQ(self)->prefix);
}

static int prq_eq(Query *self, Query *o)
{
    return (strcmp(PfxQ(self)->prefix, PfxQ(o)->prefix) == 0)
        && (PfxQ(self)->field == PfxQ(o)->field);
}

Query *prefixq_new(Symbol field, const char *prefix)
{
    Query *self = q_new(PrefixQuery);

    PfxQ(self)->field       = field;
    PfxQ(self)->prefix      = estrdup(prefix);
    MTQMaxTerms(self)       = PREFIX_QUERY_MAX_TERMS;

    self->type              = PREFIX_QUERY;
    self->rewrite           = &prq_rewrite;
    self->to_s              = &prq_to_s;
    self->hash              = &prq_hash;
    self->eq                = &prq_eq;
    self->destroy_i         = &prq_destroy;
    self->create_weight_i   = &q_create_weight_unsup;

    return self;
}

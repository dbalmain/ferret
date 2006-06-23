#ifndef FRT_SEARCH_H
#define FRT_SEARCH_H

typedef struct Query Query;
typedef struct Weight Weight;
typedef struct Scorer Scorer;

#include "index.h"
#include "bitvector.h"
#include "similarity.h"

/***************************************************************************
 *
 * Explanation
 *
 ***************************************************************************/

#define EXPLANATION_DETAILS_START_SIZE 4
typedef struct Explanation
{
    float value;
    char *description;
    struct Explanation **details;
} Explanation;
 
extern Explanation *expl_new(float value, char *description);
extern void expl_destoy(Explanation *expl);
extern Explanation *expl_add_detail(Explanation *expl, Explanation *detail);
extern char *expl_to_s_depth(Explanation *expl, int depth);
extern char *expl_to_html(Explanation *expl);

#define expl_to_s(expl) expl_to_s_depth(expl, 0)

/***************************************************************************
 *
 * Hit
 *
 ***************************************************************************/

typedef struct Hit
{
    int doc;
    float score;
} Hit;

/***************************************************************************
 *
 * TopDocs
 *
 ***************************************************************************/

typedef struct TopDocs
{
    int total_hits;
    int size;
    Hit **hits;
} TopDocs;

extern TopDocs *td_create(int total_hits, int size, Hit **hits);
extern void td_destroy(TopDocs *td);
extern char *td_to_s(TopDocs *td);

/***************************************************************************
 *
 * Filter
 *
 ***************************************************************************/

typedef struct Filter
{
    char      *name;
    HashTable *cache;
    BitVector *(*get_bv)(struct Filter *self, IndexReader *ir);
    char      *(*to_s)(struct Filter *self);
    uint       (*hash)(struct Filter *self);
    int        (*eq)(struct Filter *self, struct Filter *o);
    void       (*destroy)(struct Filter *self);
} Filter;

#define filt_new(type) filt_create(sizeof(type), #type)
extern Filter *filt_create(size_t size, char *name);
extern BitVector *filt_get_bv(Filter *filt, IndexReader *ir);
extern void filt_destroy(Filter *filt);
extern uint filt_hash(Filter *filt);
extern int filt_eq(Filter *filt, Filter *o);

/***************************************************************************
 *
 * RangeFilter
 *
 ***************************************************************************/

extern Filter *rfilt_create(const char *field,
                            char *lower_term,   char *upper_term,
                            bool include_lower, bool include_upper);

/***************************************************************************
 *
 * QueryFilter
 *
 ***************************************************************************/

extern Filter *qfilt_create(Query *query);

/***************************************************************************
 *
 * Weight
 *
 ***************************************************************************/

struct Weight
{
    float        value;
    float        qweight;
    float        qnorm;
    float        idf;
    Query       *query;
    Similarity  *similarity;
    Query       *(*get_query)(Weight *self);
    float        (*get_value)(Weight *self);
    void         (*normalize)(Weight *self, float normalization_factor);
    Scorer      *(*scorer)(Weight *self, IndexReader *ir);
    Explanation *(*explain)(Weight *self, IndexReader *ir, int doc_num);
    float        (*sum_of_squared_weights)(Weight *self);
    char        *(*to_s)(Weight *self);
    void         (*destroy)(Weight *self);
};

#define w_new(type, query) w_create(sizeof(type), query)
extern Weight *w_create(size_t size, Query *query);
extern void w_destroy(Weight *self);
extern Query *w_get_query(Weight *self);
extern float w_get_value(Weight *self);
extern float w_sum_of_squared_weights(Weight *self);
extern void w_normalize(Weight *self, float normalization_factor);

/***************************************************************************
 *
 * Query
 *
 ***************************************************************************/

enum QUERY_TYPE {
    TERM_QUERY,
    BOOLEAN_QUERY,
    PHRASE_QUERY,
    MULTI_PHRASE_QUERY,
    CONSTANT_QUERY,
    FILTERED_QUERY,
    MATCH_ALL_QUERY,
    RANGE_QUERY,
    WILD_CARD_QUERY,
    FUZZY_QUERY,
    PREFIX_QUERY,
    SPAN_TERM_QUERY,
    SPAN_FIRST_QUERY,
    SPAN_OR_QUERY,
    SPAN_NOT_QUERY,
    SPAN_NEAR_QUERY
};

struct Query
{
    int         ref_cnt;
    float       boost;
    Weight      *weight;
    Query      *(*rewrite)(Query *self, IndexReader *ir);
    void        (*extract_terms)(Query *self, HashSet *terms);
    Similarity *(*get_similarity)(Query *self, Searcher *searcher);
    char       *(*to_s)(Query *self, char *field);
    uint        (*hash)(Query *self);
    int         (*eq)(Query *self, Query *o);
    void        (*destroy_i)(Query *self);
    Weight     *(*create_weight_i)(Query *self, Searcher *searcher);
    uchar       type;
    bool        destroy_all : 1;
};

/* Internal Query Functions */
extern Similarity *q_get_similarity_i(Query *self, Searcher *searcher);
extern void q_destroy_i(Query *self);
extern Weight *q_create_weight_unsup(Query *self, Searcher *searcher);

extern void q_deref(Query *self);
extern Weight *q_weight(Query *self, Searcher *searcher);
extern Query *q_combine(Query **queries, int q_cnt);
extern uint q_hash(Query *self);
extern int q_eq(Query *self, Query *o);
extern Query *q_create(size_t size);
#define q_new(type) q_create(sizeof(type))

/***************************************************************************
 * TermQuery
 ***************************************************************************/

typedef struct TermQuery
{
    Query super;
    char *field;
    char *term;
} TermQuery;

Query *tq_new(char *field, char *term);

/***************************************************************************
 * BooleanQuery
 ***************************************************************************/

/* *** BooleanClause *** */

enum BC_TYPE
{
    BC_SHOULD,
    BC_MUST,
    BC_MUST_NOT
};

typedef struct BooleanClause {
    int ref_cnt;
    Query *query;
    Query *rewritten;
    unsigned int occur : 4;
    bool is_prohibited : 1;
    bool is_required : 1;
} BooleanClause;

extern BooleanClause *bc_create(Query *query, unsigned int occur);
extern void bc_deref(BooleanClause *self);
extern void bc_set_occur(BooleanClause *self, unsigned int occur);

/* *** BooleanQuery *** */

#define DEFAULT_MAX_CLAUSE_COUNT 1024
#define BOOLEAN_CLAUSES_START_CAPA 4
#define QUERY_STRING_START_SIZE 64

typedef struct BooleanQuery
{
    Query super;
    bool coord_disabled;
    int max_clause_cnt;
    int clause_cnt;
    int clause_capa;
    float original_boost;
    BooleanClause **clauses;
    Similarity *similarity;
} BooleanQuery;

extern Query *bq_create(bool coord_disabled);
extern BooleanClause *bq_add_query(Query *self, Query *sub_query,
                                   unsigned int occur);
extern BooleanClause *bq_add_clause(Query *self, BooleanClause *bc);

/***************************************************************************
 * PhraseQuery
 ***************************************************************************/

#define PHQ_INIT_CAPA 4
typedef struct PhraseQuery
{
    Query   super;
    int     slop;
    char   *field
    char  **terms;
    int     t_cnt;
    int     t_capa;
    int    *positions;
    char   *field;
} PhraseQuery;

extern Query *phq_create(char *field);
extern void phq_add_term(Query *self, char *term, int pos_inc);

/***************************************************************************
 * MultiPhraseQuery
 ***************************************************************************/

typedef struct MultiPhraseQuery
{
    Query   super;
    int     slop;
    char   *field
    char ***terms;
    int    *positions;
    int    *pt_cnt;
    int     t_cnt;
    int     t_capa;
    char   *field;
} MultiPhraseQuery;

extern Query *mphq_create(char *field);
extern void mphq_add_terms(Query *self, char **ts, int t_cnt, int pos_inc);

/***************************************************************************
 *
 * Scorer
 *
 ***************************************************************************/

#define SCORER_NULLIFY(mscorer) do {\
    (mscorer)->destroy(mscorer);\
    (mscorer) = NULL;\
} while (0)

struct Scorer
{
    Similarity  *similarity;
    int          doc;
    float        (*score)(Scorer *self);
    bool         (*next)(Scorer *self);
    bool         (*skip_to)(Scorer *self, int doc_num);
    Explanation *(*explain)(Scorer *self, int doc_num);
    void         (*destroy)(Scorer *self);
};

#define scorer_new(type, similarity) scorer_create(sizeof(type), similarity)
/* Internal Scorer Function */
extern void scorer_destroy_i(Scorer *self);
extern Scorer *scorer_create(size_t size, Similarity *similarity);
extern bool scorer_less_than(void *p1, void *p2);
extern bool scorer_doc_less_than(void *p1, void *p2);
extern int scorer_doc_cmp(const void *p1, const void *p2);

#endif

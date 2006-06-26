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
 
extern Explanation *expl_new(float value, const char *description, ...);
extern void expl_destroy(Explanation *expl);
extern Explanation *expl_add_detail(Explanation *expl, Explanation *detail);
extern char *expl_to_s_depth(Explanation *expl, int depth);
extern char *expl_to_html(Explanation *expl);

#define expl_to_s(expl) expl_to_s_depth(expl, 0)

/****************************************************************************
 *
 * Term
 *
 ****************************************************************************/

#define term_set_new() \
    hs_new((hash_ft)&term_hash, (eq_ft)&term_eq, (free_ft)&term_destroy)

typedef struct Term {
    char *field;
    char *text;
} Term;

extern Term *term_new(const char *field, const char *text);
extern void term_destroy(Term *self);
extern int term_eq(const void *t1, const void *t2);
extern f_u32 term_hash(const void *t);

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

extern TopDocs *td_new(int total_hits, int size, Hit **hits);
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
    f_u32      (*hash)(struct Filter *self);
    int        (*eq)(struct Filter *self, struct Filter *o);
    void       (*destroy)(struct Filter *self);
} Filter;

#define filt_new(type) filt_create(sizeof(type), #type)
extern Filter *filt_create(size_t size, const char *name);
extern BitVector *filt_get_bv(Filter *filt, IndexReader *ir);
extern void filt_destroy(Filter *filt);
extern f_u32 filt_hash(Filter *filt);
extern int filt_eq(Filter *filt, Filter *o);

/***************************************************************************
 *
 * RangeFilter
 *
 ***************************************************************************/

extern Filter *rfilt_new(const char *field,
                         const char *lower_term, const char *upper_term,
                         bool include_lower, bool include_upper);

/***************************************************************************
 *
 * QueryFilter
 *
 ***************************************************************************/

extern Filter *qfilt_new(Query *query);

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
    MULTI_TERM_QUERY,
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
    char       *(*to_s)(Query *self, const char *field);
    f_u32       (*hash)(Query *self);
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
extern const char *q_get_query_name(int type);
extern Weight *q_weight(Query *self, Searcher *searcher);
extern Query *q_combine(Query **queries, int q_cnt);
extern f_u32 q_hash(Query *self);
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

Query *tq_new(const char *field, const char *term);

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

extern BooleanClause *bc_new(Query *query, unsigned int occur);
extern void bc_deref(BooleanClause *self);
extern void bc_set_occur(BooleanClause *self, unsigned int occur);

/* *** BooleanQuery *** */

#define DEFAULT_MAX_CLAUSE_COUNT 1024
#define BOOLEAN_CLAUSES_START_CAPA 4
#define QUERY_STRING_START_SIZE 64

typedef struct BooleanQuery
{
    Query           super;
    bool            coord_disabled;
    int             max_clause_cnt;
    int             clause_cnt;
    int             clause_capa;
    float           original_boost;
    BooleanClause **clauses;
    Similarity     *similarity;
} BooleanQuery;

extern Query *bq_new(bool coord_disabled);
extern BooleanClause *bq_add_query(Query *self, Query *sub_query,
                                   unsigned int occur);
extern BooleanClause *bq_add_clause(Query *self, BooleanClause *bc);

/***************************************************************************
 * PhraseQuery
 ***************************************************************************/

#define PHQ_INIT_CAPA 4
typedef struct PhraseQuery
{
    Query           super;
    int             slop;
    char           *field;
    PhrasePosition *positions;
    int             pos_cnt;
    int             pos_capa;
} PhraseQuery;

extern Query *phq_new(const char *field);
extern void phq_add_term(Query *self, const char *term, int pos_inc);
extern void phq_append_multi_term(Query *self, const char *term);

/***************************************************************************
 * MultiTermQuery
 ***************************************************************************/

#define MULTI_TERM_QUERY_MAX_TERMS 256
typedef struct MultiTermQuery
{
    Query           super;
    char           *field;
    PriorityQueue  *boosted_terms;
    float           min_boost;
} MultiTermQuery;

extern void multi_tq_add_term(Query *self, const char *term);
extern void multi_tq_add_term_boost(Query *self, const char *term, float boost);
extern Query *multi_tq_new(const char *field);
extern Query *multi_tq_new_conf(const char *field, int max_terms,
                                          float min_boost);

/***************************************************************************
 * PrefixQuery
 ***************************************************************************/

#define PREFIX_QUERY_MAX_TERMS 256

typedef struct PrefixQuery
{
    Query super;
    char *field;
    char *prefix;
    int   max_terms;
} PrefixQuery;

extern Query *prefixq_new(const char *field, const char *prefix);

/***************************************************************************
 * WildCardQuery
 ***************************************************************************/

#define WILD_CHAR '?'
#define WILD_STRING '*'
#define WILD_CARD_QUERY_MAX_TERMS 256

typedef struct WildCardQuery
{
    Query super;
    char *field;
    char *pattern;
    int   max_terms;
} WildCardQuery;


extern Query *wcq_new(const char *field, const char *pattern);
extern bool wc_match(const char *pattern, const char *text);

/***************************************************************************
 * FuzzyQuery
 ***************************************************************************/

#define DEF_MIN_SIM 0.5
#define DEF_PRE_LEN 0
#define DEF_MAX_TERMS 256
#define TYPICAL_LONGEST_WORD 20

typedef struct FuzzyQuery
{
    Query       super;
    char       *field;
    char       *term;
    const char *text; /* term text after prefix */
    int         text_len;
    int         pre_len;
    float       min_sim;
    float       scale_factor;
    int         max_distances[TYPICAL_LONGEST_WORD];
    int        *da;
    int         max_terms;
} FuzzyQuery;

extern Query *fuzq_new(const char *term, const char *field);
extern Query *fuzq_new_conf(const char *field, const char *term,
                            float min_sim, int pre_len, int max_terms);

/***************************************************************************
 * ConstantScoreQuery
 ***************************************************************************/

typedef struct ConstantScoreQuery
{
    Query   super;
    Filter *filter;
} ConstantScoreQuery;

extern Query *csq_new(Filter *filter);

/***************************************************************************
 * FilteredQuery
 ***************************************************************************/

typedef struct FilteredQuery
{
    Query   super;
    Query  *query;
    Filter *filter;
} FilteredQuery;

extern Query *fq_new(Query *query, Filter *filter);

/***************************************************************************
 * MatchAllQuery
 ***************************************************************************/

extern Query *maq_new();

/***************************************************************************
 * RangeQuery
 ***************************************************************************/

extern Query *rq_new(const char *field, const char *lower_term,
                     const char *upper_term, bool include_lower,
                     bool include_upper);
extern Query *rq_new_less(const char *field, const char *upper_term,
                          bool include_upper);
extern Query *rq_new_more(const char *field, const char *lower_term,
                          bool include_lower);

/***************************************************************************
 * SpanQuery
 ***************************************************************************/

/* ** SpanEnum ** */
typedef struct SpanEnum SpanEnum;
struct SpanEnum
{
    Query *query;
    bool (*next)(SpanEnum *self);
    bool (*skip_to)(SpanEnum *self, int target_doc);
    int  (*doc)(SpanEnum *self);
    int  (*start)(SpanEnum *self);
    int  (*end)(SpanEnum *self);
    char *(*to_s)(SpanEnum *self);
    void (*destroy)(SpanEnum *self);
};

/* ** SpanQuery ** */
typedef struct SpanQuery
{
    Query     super;
    char     *field;
    SpanEnum *(*get_spans)(Query *self, IndexReader *ir);
    HashSet  *(*get_terms)(Query *self);
} SpanQuery;

/***************************************************************************
 * SpanTermQuery
 ***************************************************************************/

typedef struct SpanTermQuery
{
    SpanQuery super;
    char     *term;
} SpanTermQuery;
extern Query *spantq_new(const char *field, const char *term);


/***************************************************************************
 * SpanFirstQuery
 ***************************************************************************/

typedef struct SpanFirstQuery
{
    SpanQuery   super;
    int         end;
    Query      *match;
} SpanFirstQuery;

extern Query *spanfq_new(Query *match, int end);
extern Query *spanfq_new_nr(Query *match, int end);

/***************************************************************************
 * SpanOrQuery
 ***************************************************************************/

typedef struct SpanOrQuery
{
    SpanQuery   super;
    Query     **clauses;
    int         c_cnt;
    int         c_capa;
} SpanOrQuery;

extern Query *spanoq_new();
extern Query *spanoq_add_clause(Query *self, Query *clause);
extern Query *spanoq_add_clause_nr(Query *self, Query *clause);

/***************************************************************************
 * SpanNearQuery
 ***************************************************************************/

typedef struct SpanNearQuery
{
    SpanQuery   super;
    Query     **clauses;
    int         c_cnt;
    int         c_capa;
    int         slop;
    bool        in_order : 1;
} SpanNearQuery;

extern Query *spannq_new(int slop, bool in_order);
extern Query *spannq_add_clause(Query *self, Query *clause);
extern Query *spannq_add_clause_nr(Query *self, Query *clause);

/***************************************************************************
 * SpanNotQuery
 ***************************************************************************/

typedef struct SpanNotQuery
{
    SpanQuery   super;
    Query      *inc;
    Query      *exc;
} SpanNotQuery;

extern Query *spanxq_new(Query *inc, Query *exc);
extern Query *spanxq_new_nr(Query *inc, Query *exc);



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
extern bool scorer_doc_less_than(const Scorer *s1, const Scorer *s2);
extern int scorer_doc_cmp(const void *p1, const void *p2);

/***************************************************************************
 *
 * Sort
 *
 ***************************************************************************/

enum SORT_TYPE {
    SORT_TYPE_SCORE,
    SORT_TYPE_DOC,
    SORT_TYPE_INTEGER,
    SORT_TYPE_FLOAT,
    SORT_TYPE_STRING,
    SORT_TYPE_AUTO
};

/***************************************************************************
 * SortField
 ***************************************************************************/

typedef struct SortField
{
    mutex_t mutex;
    char *field;
    int   type;
    bool  reverse : 1;
    void *index;
    int   (*compare)(void *index_ptr, Hit *hit1, Hit *hit2);
    void *(*create_index)(int size);
    void  (*destroy_index)(void *p);
    void  (*handle_term)(void *index, TermDocEnum *tde, char *text);
} SortField;

extern SortField *sort_field_new(char *field, int type, bool reverse);
extern SortField *sort_field_score_new(bool reverse);
extern SortField *sort_field_doc_new(bool reverse);
extern SortField *sort_field_int_new(char *field, bool reverse);
extern SortField *sort_field_float_new(char *field, bool reverse);
extern SortField *sort_field_string_new(char *field, bool reverse);
extern SortField *sort_field_auto_new(char *field, bool reverse);
extern void sort_field_destroy(void *p);
extern char *sort_field_to_s(SortField *self);

extern const SortField SORT_FIELD_SCORE; 
extern const SortField SORT_FIELD_SCORE_REV; 
extern const SortField SORT_FIELD_DOC; 
extern const SortField SORT_FIELD_DOC_REV; 

/***************************************************************************
 * Sort
 ***************************************************************************/

typedef struct Sort
{
    SortField **sort_fields;
    int sf_cnt;
    int sf_capa;
    bool destroy_all : 1;
} Sort;

extern Sort *sort_new();
extern void sort_destroy(void *p);
extern void sort_add_sort_field(Sort *self, SortField *sf);
extern void sort_clear(Sort *self);
extern char *sort_to_s(Sort *self);

/***************************************************************************
 * FieldSortedHitQueue
 ***************************************************************************/

extern Hit *fshq_pq_pop(PriorityQueue *pq);
extern void fshq_pq_down(PriorityQueue *pq);
extern void fshq_pq_insert(PriorityQueue *pq, Hit *hit);
extern void fshq_pq_destroy(PriorityQueue *pq);
extern PriorityQueue *fshq_pq_new(int size, Sort *sort, IndexReader *ir);

/***************************************************************************
 *
 * Searcher
 *
 ***************************************************************************/

struct Searcher {
    Similarity     *similarity;
    int          (*doc_freq)(Searcher *self, const char *field,
                             const char *term);
    Document    *(*get_doc)(Searcher *self, int doc_num);
    int          (*max_doc)(Searcher *self);
    Weight      *(*create_weight)(Searcher *self, Query *query);
    TopDocs     *(*search)(Searcher *self, Query *query, int first_doc,
                           int num_docs, Filter *filter, Sort *sort);
    void         (*search_each)(Searcher *self, Query *query, Filter *filter,
                                void (*fn)(Searcher *, int, float, void *),
                                void *arg);
    void         (*search_each_w)(Searcher *self, Weight *weight,
                                  Filter *filter,
                                  void (*fn)(Searcher *, int, float, void *),
                                  void *arg);
    Query       *(*rewrite)(Searcher *self, Query *original);
    Explanation *(*explain)(Searcher *self, Query *query, int doc_num);
    Explanation *(*explain_w)(Searcher *self, Weight *weight, int doc_num);
    Similarity  *(*get_similarity)(Searcher *self);
    void         (*close)(Searcher *self);
};

#define searcher_doc_freq(s, t)  s->doc_freq(s, t)
#define searcher_get_doc(s, dn)  s->get_doc(s, dn)
#define searcher_max_doc(s)  s->max_doc(s)
#define searcher_search(s, q, fd, nd, filt, sort)\
    s->search(s, q, fd, nd, filt, sort)
#define searcher_search_each(s, q, filt, fn, arg)\
    s->search_each(s, q, filt, fn, arg)
#define searcher_search_each_w(s, q, filt, fn, arg)\
    s->search_each_w(s, q, filt, fn, arg)
#define searcher_rewrite(s, q)  s->rewrite(s, q)
#define searcher_explain(s, q, dn)  s->explain(s, q, dn)
#define searcher_explain_w(s, q, dn)  s->explain_w(s, q, dn)
#define searcher_get_similarity(s)  s->get_similarity(s)
#define searcher_close(s)  s->close(s)

/***************************************************************************
 *
 * StdSearcher
 *
 ***************************************************************************/

extern Searcher *stdsea_new(IndexReader *ir);

/***************************************************************************
 *
 * MultiSearcher
 *
 ***************************************************************************/

extern Searcher *msea_new(Searcher **searchers, int s_cnt, bool close_subs);

#endif

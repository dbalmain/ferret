#ifndef FRT_SIMILARITY_H
#define FRT_SIMILARITY_H

typedef struct FrtSearcher FrtSearcher;

/****************************************************************************
 *
 * Term
 *
 ****************************************************************************/

#define term_set_new() \
    frt_hs_new((hash_ft)&term_hash, (eq_ft)&term_eq, (free_ft)&term_destroy)

typedef struct Term
{
    char *field;
    char *text;
} Term;

extern Term *term_new(const char *field, const char *text);
extern void term_destroy(Term *self);
extern int term_eq(const void *t1, const void *t2);
extern unsigned long term_hash(const void *t);

/***************************************************************************
 *
 * PhrasePosition
 *
 ***************************************************************************/

typedef struct PhrasePosition
{
    int pos;
    char **terms;
} PhrasePosition;

/***************************************************************************
 *
 * FrtSimilarity
 *
 ***************************************************************************/

typedef struct FrtSimilarity FrtSimilarity;

struct FrtSimilarity
{
    void *data;
    float norm_table[256];
    float (*length_norm)(FrtSimilarity *self, const char *field, int num_terms);
    float (*query_norm)(FrtSimilarity *self, float sum_of_squared_weights);
    float (*tf)(FrtSimilarity *self, float freq);
    float (*sloppy_freq)(FrtSimilarity *self, int distance);
    float (*idf_term)(FrtSimilarity *self, const char *field, char *term,
                      FrtSearcher *searcher);
    float (*idf_phrase)(FrtSimilarity *self, const char *field,
                        PhrasePosition *positions,
                        int pp_cnt, FrtSearcher *searcher);
    float (*idf)(FrtSimilarity *self, int doc_freq, int num_docs);
    float (*coord)(FrtSimilarity *self, int overlap, int max_overlap);
    float (*decode_norm)(FrtSimilarity *self, unsigned char b);
    unsigned char (*encode_norm)(FrtSimilarity *self, float f);
    void  (*destroy)(FrtSimilarity *self);
};

#define sim_length_norm(msim, field, num_terms) msim->length_norm(msim, field, num_terms)
#define sim_query_norm(msim, sosw) msim->query_norm(msim, sosw)
#define sim_tf(msim, freq) msim->tf(msim, freq)
#define sim_sloppy_freq(msim, distance) msim->sloppy_freq(msim, distance)
#define sim_idf_term(msim, field, term, searcher)\
    msim->idf_term(msim, field, term, searcher)
#define sim_idf_phrase(msim, field, positions, pos_cnt, searcher)\
    msim->idf_phrase(msim, field, positions, pos_cnt, searcher)
#define sim_idf(msim, doc_freq, num_docs) msim->idf(msim, doc_freq, num_docs)
#define sim_coord(msim, overlap, max_overlap) msim->coord(msim, overlap, max_overlap)
#define sim_decode_norm(msim, b) msim->decode_norm(msim, b)
#define sim_encode_norm(msim, f) msim->encode_norm(msim, f)
#define sim_destroy(msim) msim->destroy(msim)

FrtSimilarity *sim_create_default();

#endif

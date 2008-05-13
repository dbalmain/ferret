#ifndef FRT_SIMILARITY_H
#define FRT_SIMILARITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "symbol.h"

typedef struct FrtSearcher FrtSearcher;

/****************************************************************************
 *
 * FrtTerm
 *
 ****************************************************************************/

typedef struct FrtTerm
{
    FrtSymbol field;
    char *text;
} FrtTerm;

extern FrtTerm *frt_term_new(FrtSymbol field, const char *text);
extern void frt_term_destroy(FrtTerm *self);
extern int frt_term_eq(const void *t1, const void *t2);
extern unsigned long frt_term_hash(const void *t);

/***************************************************************************
 *
 * FrtPhrasePosition
 *
 ***************************************************************************/

typedef struct FrtPhrasePosition
{
    int pos;
    char **terms;
} FrtPhrasePosition;

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
    float (*length_norm)(FrtSimilarity *self, FrtSymbol field, int num_terms);
    float (*query_norm)(FrtSimilarity *self, float sum_of_squared_weights);
    float (*tf)(FrtSimilarity *self, float freq);
    float (*sloppy_freq)(FrtSimilarity *self, int distance);
    float (*idf_term)(FrtSimilarity *self, FrtSymbol field, char *term,
                      FrtSearcher *searcher);
    float (*idf_phrase)(FrtSimilarity *self, FrtSymbol field,
                        FrtPhrasePosition *positions,
                        int pp_cnt, FrtSearcher *searcher);
    float (*idf)(FrtSimilarity *self, int doc_freq, int num_docs);
    float (*coord)(FrtSimilarity *self, int overlap, int max_overlap);
    float (*decode_norm)(FrtSimilarity *self, unsigned char b);
    unsigned char (*encode_norm)(FrtSimilarity *self, float f);
    void  (*destroy)(FrtSimilarity *self);
};

#define frt_sim_length_norm(msim, field, num_terms) msim->length_norm(msim, field, num_terms)
#define frt_sim_query_norm(msim, sosw) msim->query_norm(msim, sosw)
#define frt_sim_tf(msim, freq) msim->tf(msim, freq)
#define frt_sim_sloppy_freq(msim, distance) msim->sloppy_freq(msim, distance)
#define frt_sim_idf_term(msim, field, term, searcher)\
    msim->idf_term(msim, field, term, searcher)
#define frt_sim_idf_phrase(msim, field, positions, pos_cnt, searcher)\
    msim->idf_phrase(msim, field, positions, pos_cnt, searcher)
#define frt_sim_idf(msim, doc_freq, num_docs) msim->idf(msim, doc_freq, num_docs)
#define frt_sim_coord(msim, overlap, max_overlap) msim->coord(msim, overlap, max_overlap)
#define frt_sim_decode_norm(msim, b) msim->decode_norm(msim, b)
#define frt_sim_encode_norm(msim, f) msim->encode_norm(msim, f)
#define frt_sim_destroy(msim) msim->destroy(msim)

FrtSimilarity *frt_sim_create_default();

#ifdef __cplusplus
} // extern "C"
#endif

#endif

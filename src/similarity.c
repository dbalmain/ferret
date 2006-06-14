#include "similarity.h"
#include "helper.h"
#include <math.h>
#include <stdlib.h>


float simdef_length_norm(Similarity *s, int field_num, int num_terms)
{
    (void)s;
    (void)field_num;
    return (float)(1.0 / sqrt(num_terms));
}

float simdef_query_norm(struct Similarity *s, float sum_of_squared_weights)
{
    (void)s;
    return (float)(1.0 / sqrt(sum_of_squared_weights));
}

float simdef_tf(struct Similarity *s, float freq)
{
    (void)s;
    return (float)sqrt(freq);
}

float simdef_sloppy_freq(struct Similarity *s, int distance)
{
    (void)s;
    return (float)(1.0 / (double)(distance + 1));
}

float simdef_idf(struct Similarity *s, int doc_freq, int num_docs)
{
    (void)s;
    return (float)(log((float)num_docs/(float)(doc_freq+1)) + 1.0);
}

float simdef_coord(struct Similarity *s, int overlap, int max_overlap)
{
    (void)s;
    return (float)((double)overlap / (double)max_overlap);
}

float simdef_decode_norm(struct Similarity *s, uchar b)
{
    return s->norm_table[b];
}

uchar simdef_encode_norm(struct Similarity *s, float f)
{
    (void)s;
    return float2byte(f);
}

void simdef_destroy(Similarity *s)
{
    (void)s;
    /* nothing to do here */
}

static Similarity default_similarity = {
    NULL,
    {0},
    &simdef_length_norm,
    &simdef_query_norm,
    &simdef_tf,
    &simdef_sloppy_freq,
    &simdef_idf,
    &simdef_coord,
    &simdef_decode_norm,
    &simdef_encode_norm,
    &simdef_destroy
};

Similarity *sim_create_default()
{
    int i;
    if (!default_similarity.data) {
        for (i = 0; i < 256; i++) {
            default_similarity.norm_table[i] = byte2float(i);
        }

        default_similarity.data = &default_similarity;
    }
    return &default_similarity;
}

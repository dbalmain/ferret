#ifndef ALL_BENCHMARKS_H
#define ALL_BENCHMARKS_H

#include "benchmark.h"


void bm_strcmp_when_length_is_known(BenchMark *bm);
void bm_hash_implementations(BenchMark *bm);
void bm_specialized_string_hash(BenchMark *bm);

const struct BenchMarkList
{
    void (*initialize)(BenchMark *benchmark);
    char *name;
} all_benchmarks[] = {
    {bm_strcmp_when_length_is_known, "strcmp_when_length_is_known"},
    {bm_hash_implementations, "hash_implementations"},
    {bm_specialized_string_hash, "specialized_string_hash"}
};

#endif

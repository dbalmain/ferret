#ifndef ALL_BENCHMARKS_H
#define ALL_BENCHMARKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "benchmark.h"

void bm_vint_io(BenchMark *bm);
void bm_strcmp_when_length_is_known(BenchMark *bm);
void bm_snprintf_vs_strncat(BenchMark *bm);
void bm_hash_implementations(BenchMark *bm);
void bm_specialized_string_hash(BenchMark *bm);
void bm_bitvector_implementations(BenchMark *bm);

const struct BenchMarkList
{
    void (*initialize)(BenchMark *benchmark);
    char *name;
} all_benchmarks[] = {
    {bm_vint_io, "vint_io"},
    {bm_strcmp_when_length_is_known, "strcmp_when_length_is_known"},
    {bm_snprintf_vs_strncat, "snprintf_vs_strncat"},
    {bm_hash_implementations, "hash_implementations"},
    {bm_specialized_string_hash, "specialized_string_hash"},
    {bm_bitvector_implementations, "bitvector_implementations"}
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif

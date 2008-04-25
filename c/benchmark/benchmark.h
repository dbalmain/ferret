#ifndef BENCHMARK_H
#define BENCHMARK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"
#include "internal.h"
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>

#define BM_RUN_COUNT 6
#define BENCH(name) FRT_EXTERNC void bm_##name(BenchMark *bm)

extern const char *WORD_LIST[];

typedef void (*bm_run_ft)();

typedef struct BenchMarkTimes {
    double utime;
    double stime;
    double rtime;
} BenchMarkTimes;

typedef struct BenchMarkUnit {
    char *name;
    bm_run_ft  run;
    struct BenchMarkUnit *next;
    BenchMarkTimes final_times;
    BenchMarkTimes *times[1];
} BenchMarkUnit;

typedef struct BenchMark {
    int            count;   /* the number of bench runs to complete */
    int            discard; /* the number of outliers to discard */
    void           (*setup)();
    void           (*teardown)();
    BenchMarkUnit *head;
    BenchMarkUnit *tail;
} BenchMark;

void bm_add(BenchMark *benchmark, bm_run_ft call, const char *name);

#define BM_SETUP(func) bm->setup = &func;
#define BM_TEARDOWN(func) bm->teardown = &func;
#define BM_ADD(call) bm_add(bm, &call, #call)
#define BM_COUNT(num) bm->count = num;
#define BM_DISCARD(num) bm->discard = num;

#ifdef __cplusplus
} // extern "C"
#endif

#endif

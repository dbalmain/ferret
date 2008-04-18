#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "global.h"
#include "benchmark.h"
#include "all_benchmarks.h"
#include "word_list.h"

static int hertz;

static int bmtcmp(const void *p1, const void *p2)
{
    BenchMarkTimes *bmt1 = *(BenchMarkTimes **)p1;
    BenchMarkTimes *bmt2 = *(BenchMarkTimes **)p2;
    
    if (bmt1->rtime > bmt2->rtime) return 1;
    else if (bmt1->rtime < bmt2->rtime) return -1;
    else return 0;
}

void bm_add(BenchMark *benchmark, bm_run_ft run, const char *name)
{
    BenchMarkUnit *unit =
        (BenchMarkUnit *)emalloc(sizeof(BenchMarkUnit) +
                                 benchmark->count * sizeof(BenchMarkTimes *));
    int i;
    unit->name = estrdup(name);
    unit->run = run;
    unit->next = NULL;
    for (i = 0; i < benchmark->count; i++) {
        unit->times[i] = ALLOC(BenchMarkTimes);
    }
    if (benchmark->tail) {
        benchmark->tail = benchmark->tail->next = unit;
    }
    else {
        benchmark->tail = benchmark->head = unit;
    }
}

static void bm_clear(BenchMark *benchmark)
{
    BenchMarkUnit *unit, *next = benchmark->head;
    while (NULL != (unit = next)) {
        next = unit->next;
        if (benchmark->count > 1) {
            int i;
            for (i = 0; i < benchmark->count; i++) {
                unit->times[i] = ALLOC(BenchMarkTimes);
            }
        }
        free(unit->name);
        free(unit);
    }
    benchmark->head = benchmark->tail = NULL;
}

static void bm_single_run(BenchMarkUnit *unit, BenchMarkTimes *bm_times)
{
    struct timeval tv_before, tv_after;
    struct tms tms_before, tms_after;
    double before, after;

    if (gettimeofday(&tv_before, NULL) == -1)
        RAISE(FRT_UNSUPPORTED_ERROR, "gettimeofday failed\n");
    (void)times(&tms_before);

    unit->run();

    if (gettimeofday(&tv_after, NULL) == -1)
        RAISE(FRT_UNSUPPORTED_ERROR, "gettimeofday failed\n");
    (void)times(&tms_after);
    before = (double)tv_before.tv_sec + ((double)tv_before.tv_usec/1000000);
    after = (double)tv_after.tv_sec + ((double)tv_after.tv_usec/1000000);
    bm_times->rtime = after - before;
    bm_times->utime =
        ((double)(tms_after.tms_utime - tms_before.tms_utime))/hertz;
    bm_times->stime =
        ((double)(tms_after.tms_stime - tms_before.tms_stime))/hertz;
}

#define DO_SETUP(bm) if (bm->setup) bm->setup();
#define DO_TEARDOWN(bm) if (bm->teardown) bm->teardown();

static void bm_run(BenchMark *benchmark)
{
    int i;
    BenchMarkUnit *unit;
    int max_name_len = 0;
    char fmt[30];
    int start = 0, end = benchmark->count;
    if (benchmark->discard) {
        start += benchmark->discard;
        end   -= benchmark->discard;
    }
    if (benchmark->count > 1) {
        for (i = 0; i < benchmark->count; i++) {
            DO_SETUP(benchmark);
            for (unit = benchmark->head; unit; unit = unit->next) {
                bm_single_run(unit, unit->times[i]);
            }
            DO_TEARDOWN(benchmark);
        }
        for (unit = benchmark->head; unit; unit = unit->next) {
            double rtime = 0.0, utime = 0.0, stime = 0.0;
            int result_count = end - start;
            /* we only need to sort if we are discarding outliers */
            if (benchmark->discard > 0) {
                qsort(unit->times, benchmark->count,
                      sizeof(BenchMarkTimes*), &bmtcmp);
            }

            for (i = start; i < end; i++) {
                rtime += unit->times[i]->rtime;
                utime += unit->times[i]->utime;
                stime += unit->times[i]->stime;
            }
            unit->final_times.utime = utime/result_count;
            unit->final_times.stime = stime/result_count;
            unit->final_times.rtime = rtime/result_count;
        }
    }
    else {
        DO_SETUP(benchmark);
        for (unit = benchmark->head; unit; unit = unit->next) {
            bm_single_run(unit, &(unit->final_times));
        }
        DO_TEARDOWN(benchmark);
    }
    
    /* get maximum unit name length for print out */
    for (unit = benchmark->head; unit; unit = unit->next) {
        int name_len = (int)strlen(unit->name);
        if (name_len > max_name_len) {
            max_name_len = name_len;
        }
    }

    for (i = 0; i < max_name_len; i++) putchar(' ');
    puts("\t         user     system       real");
    sprintf(fmt, "\t%%%ds %%10.6lf %%10.6lf %%10.6lf\n", max_name_len);
    for (unit = benchmark->head; unit; unit = unit->next) {
        printf(fmt,
               unit->name,
               unit->final_times.utime,
               unit->final_times.stime,
               unit->final_times.rtime);
    }
}

int main(int argc, const char *const argv[])
{
    int i;
    BenchMark benchmark;
    (void)argc; (void)argv;
    benchmark.head = benchmark.tail = NULL;
    /* set the clock speed used for calculating run times */
    hertz = sysconf(_SC_CLK_TCK);

    for (i = 0; i < NELEMS(all_benchmarks); i++) {
        if (argc == 2) {
            if (!strstr(all_benchmarks[i].name, argv[1])) {
                continue;
            }
        }
        printf("\nBenching [%s]...\n", all_benchmarks[i].name);
        benchmark.count = 1;
        benchmark.discard = 0;
        benchmark.setup = benchmark.teardown = NULL;
        all_benchmarks[i].initialize(&benchmark);
        bm_run(&benchmark);
        bm_clear(&benchmark);
    }
    return 0;
}

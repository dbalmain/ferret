#ifndef ALL_TEST_H
#define ALL_TEST_H

#include "test.h"

extern tst_suite *ts_test(tst_suite *suite);
extern tst_suite *ts_global(tst_suite *suite);
extern tst_suite *ts_except(tst_suite *suite);
extern tst_suite *ts_hash(tst_suite *suite);
extern tst_suite *ts_hashset(tst_suite *suite);
extern tst_suite *ts_bitvector(tst_suite *suite);
extern tst_suite *ts_priorityqueue(tst_suite *suite);
extern tst_suite *ts_helper(tst_suite *suite);
extern tst_suite *ts_mem_pool(tst_suite *suite);
extern tst_suite *ts_fs_store(tst_suite *suite);
extern tst_suite *ts_ram_store(tst_suite *suite);
extern tst_suite *ts_fields(tst_suite *suite);
extern tst_suite *ts_document(tst_suite *suite);
extern tst_suite *ts_analysis(tst_suite *suite);
extern tst_suite *ts_segments(tst_suite *suite);
extern tst_suite *ts_term(tst_suite *suite);
extern tst_suite *ts_term_vectors(tst_suite *suite);
extern tst_suite *ts_similarity(tst_suite *suite);
extern tst_suite *ts_index(tst_suite *suite);

const struct test_list
{
    tst_suite *(*func)(tst_suite *suite);
} all_tests[] = {
    {ts_test},
    {ts_global},
    {ts_except},
    {ts_hash},
    {ts_hashset},
    {ts_bitvector},
    {ts_priorityqueue},
    {ts_helper},
    {ts_mem_pool},
    {ts_fs_store},
    {ts_ram_store},
    {ts_fields},
    {ts_segments},
    {ts_document},
    {ts_analysis},
    {ts_term},
    {ts_term_vectors},
    {ts_similarity},
    {ts_index}
};

#endif

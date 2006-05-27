#ifndef ALL_TEST_H
#define ALL_TEST_H

#include "test.h"

extern tst_suite *ts_test(tst_suite * suite);
extern tst_suite *ts_global(tst_suite * suite);
extern tst_suite *ts_hash(tst_suite * suite);
extern tst_suite *ts_hashset(tst_suite * suite);
extern tst_suite *ts_bitvector(tst_suite * suite);
extern tst_suite *ts_priorityqueue(tst_suite * suite);
extern tst_suite *ts_helper(tst_suite * suite);

const struct test_list
{
    tst_suite *(*func)(tst_suite *suite);
} all_tests[] = {
    {ts_test},
    {ts_global},
    {ts_hash},
    {ts_hashset},
    {ts_bitvector},
    {ts_priorityqueue},
    {ts_helper}
};

#endif

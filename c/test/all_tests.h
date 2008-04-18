#ifndef ALL_TEST_H
#define ALL_TEST_H

#include "test.h"

extern TestSuite *ts_test(TestSuite *suite);
extern TestSuite *ts_global(TestSuite *suite);
extern TestSuite *ts_except(TestSuite *suite);
extern TestSuite *ts_array(TestSuite *suite);
extern TestSuite *ts_hash(TestSuite *suite);
extern TestSuite *ts_hashset(TestSuite *suite);
extern TestSuite *ts_bitvector(TestSuite *suite);
extern TestSuite *ts_priorityqueue(TestSuite *suite);
extern TestSuite *ts_helper(TestSuite *suite);
extern TestSuite *ts_mem_pool(TestSuite *suite);
extern TestSuite *ts_fs_store(TestSuite *suite);
extern TestSuite *ts_ram_store(TestSuite *suite);
extern TestSuite *ts_compound_io(TestSuite *suite);
extern TestSuite *ts_fields(TestSuite *suite);
extern TestSuite *ts_segments(TestSuite *suite);
extern TestSuite *ts_document(TestSuite *suite);
extern TestSuite *ts_multimapper(TestSuite *suite);
extern TestSuite *ts_analysis(TestSuite *suite);
extern TestSuite *ts_term(TestSuite *suite);
extern TestSuite *ts_term_vectors(TestSuite *suite);
extern TestSuite *ts_similarity(TestSuite *suite);
extern TestSuite *ts_index(TestSuite *suite);
extern TestSuite *ts_file_deleter(TestSuite *suite);
extern TestSuite *ts_search(TestSuite *suite);
extern TestSuite *ts_multi_search(TestSuite *suite);
extern TestSuite *ts_q_fuzzy(TestSuite *suite);
extern TestSuite *ts_q_filtered(TestSuite *suite);
extern TestSuite *ts_q_span(TestSuite *suite);
extern TestSuite *ts_q_const_score(TestSuite *suite);
extern TestSuite *ts_filter(TestSuite *suite);
extern TestSuite *ts_sort(TestSuite *suite);
extern TestSuite *ts_q_parser(TestSuite *suite);
extern TestSuite *ts_highlighter(TestSuite *suite);
extern TestSuite *ts_threading(TestSuite *suite);
extern TestSuite *ts_lang(TestSuite *suite);

const struct test_list
{
    TestSuite *(*func)(TestSuite *suite);
} all_tests[] = {
    {ts_test},
    {ts_global},
    {ts_except},
    {ts_array},
    {ts_hash},
    {ts_hashset},
    {ts_bitvector},
    {ts_priorityqueue},
    {ts_helper},
    {ts_mem_pool},
    {ts_fs_store},
    {ts_ram_store},
    {ts_compound_io},
    {ts_fields},
    {ts_segments},
    {ts_document},
    {ts_multimapper},
    {ts_analysis},
    {ts_term},
    {ts_term_vectors},
    {ts_similarity},
    {ts_index},
    {ts_file_deleter},
    {ts_search},
    {ts_multi_search},
    {ts_q_fuzzy},
    {ts_q_filtered},
    {ts_q_span},
    {ts_q_const_score},
    {ts_filter},
    {ts_sort},
    {ts_q_parser},
    {ts_highlighter},
    {ts_threading},
    {ts_lang}
};

#endif

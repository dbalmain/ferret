#ifndef ALL_TESTS_H
#define ALL_TESTS_H

#include "test.h"


TestSuite *ts_analysis(TestSuite *suite);
TestSuite *ts_search(TestSuite *suite);
TestSuite *ts_multi_search(TestSuite *suite);
TestSuite *ts_except(TestSuite *suite);
TestSuite *ts_similarity(TestSuite *suite);
TestSuite *ts_term_vectors(TestSuite *suite);
TestSuite *ts_priorityqueue(TestSuite *suite);
TestSuite *ts_helper(TestSuite *suite);
TestSuite *ts_index(TestSuite *suite);
TestSuite *ts_highlighter(TestSuite *suite);
TestSuite *ts_q_span(TestSuite *suite);
TestSuite *ts_q_const_score(TestSuite *suite);
TestSuite *ts_file_deleter(TestSuite *suite);
TestSuite *ts_multimapper(TestSuite *suite);
TestSuite *ts_test(TestSuite *suite);
TestSuite *ts_document(TestSuite *suite);
TestSuite *ts_sort(TestSuite *suite);
TestSuite *ts_lang(TestSuite *suite);
TestSuite *ts_global(TestSuite *suite);
TestSuite *ts_term(TestSuite *suite);
TestSuite *ts_fields(TestSuite *suite);
TestSuite *ts_fs_store(TestSuite *suite);
TestSuite *ts_symbol(TestSuite *suite);
TestSuite *ts_q_parser(TestSuite *suite);
TestSuite *ts_bitvector(TestSuite *suite);
TestSuite *ts_compound_io(TestSuite *suite);
TestSuite *ts_mem_pool(TestSuite *suite);
TestSuite *ts_ram_store(TestSuite *suite);
TestSuite *ts_array(TestSuite *suite);
TestSuite *ts_threading(TestSuite *suite);
TestSuite *ts_segments(TestSuite *suite);
TestSuite *ts_filter(TestSuite *suite);
TestSuite *ts_hashset(TestSuite *suite);
TestSuite *ts_q_fuzzy(TestSuite *suite);
TestSuite *ts_q_filtered(TestSuite *suite);
TestSuite *ts_hash(TestSuite *suite);

const struct test_list
{
    TestSuite *(*func)(TestSuite *suite);
} all_tests[] = {
    {ts_analysis},
    {ts_search},
    {ts_multi_search},
    {ts_except},
    {ts_similarity},
    {ts_term_vectors},
    {ts_priorityqueue},
    {ts_helper},
    {ts_index},
    {ts_highlighter},
    {ts_q_span},
    {ts_q_const_score},
    {ts_file_deleter},
    {ts_multimapper},
    {ts_test},
    {ts_document},
    {ts_sort},
    {ts_lang},
    {ts_global},
    {ts_term},
    {ts_fields},
    {ts_fs_store},
    {ts_symbol},
    {ts_q_parser},
    {ts_bitvector},
    {ts_compound_io},
    {ts_mem_pool},
    {ts_ram_store},
    {ts_array},
    {ts_threading},
    {ts_segments},
    {ts_filter},
    {ts_hashset},
    {ts_q_fuzzy},
    {ts_q_filtered},
    {ts_hash}
};

#endif

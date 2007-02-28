#include "index.h"
#include "store.h"
#include "search.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void create_index(Store *store, Config config)
{
    int i;
    IndexWriter *iw;
    Analyzer *a = whitespace_analyzer_new(true);
    FieldInfos *fis = fis_new(STORE_NO, INDEX_UNTOKENIZED, TERM_VECTOR_NO);
    char data[1000];
    Document *doc;
    index_create(store, fis);
    fis_deref(fis);

    iw = iw_open(store, a, &config);


    //config.merge_factor = 10;
    //config.min_merge_docs = 1000;

    for (i = 0; i < 99999; i++) {
        if (i%10000 == 0) printf("up to %d\n", i);
        doc = doc_new();
        sprintf(data, "<%d>", rand() % 1000000);
        doc_add_field(doc, df_add_data(df_new("num"), data));
        iw_add_doc(iw, doc);
        doc_destroy(doc);
    }
    //iw_optimize(iw);
    iw_close(iw);
}

int main(int argc, char **argv)
{
    Config config = default_config;
    Store *store = open_fs_store("./bench_index/");
    Searcher *searcher;
    Sort *sort = sort_new();
    Query *q = tq_new("num", "<1234>");
    TopDocs *td;
    config.max_field_length = 0x7FFFFFFF;
    config.max_buffer_memory = 0x40000000;
    config.chunk_size = 0x8000000;
    config.max_buffered_docs = 1000;
    config.merge_factor = 11;

    (void)argv;

    if (argc == 2) {
        create_index(store, config);
    }

    printf("sort_test\n");
    clock_t time_taken = clock();
    searcher = isea_new(ir_open(store));
    sort_add_sort_field(sort, sort_field_int_new("num", false));
    td = searcher_search(searcher, q, 0, 10, NULL, sort, NULL);
    td_destroy(td);
    q_deref(q);
    sort_destroy(sort);
    time_taken = clock() - time_taken;
    printf("Took: %lu clocks in %0.3lf seconds\n", time_taken,
           (double)time_taken/CLOCKS_PER_SEC);

    printf("rangeq_test\n");
    time_taken = clock();
    q = rq_new("num", "<100000>", "<800000>", true, true);
    td = searcher_search(searcher, q, 0, 10, NULL, NULL, NULL);
    td_destroy(td);
    q_deref(q);
    time_taken = clock() - time_taken;
    printf("Took: %lu clocks in %0.3lf seconds\n", time_taken,
           (double)time_taken/CLOCKS_PER_SEC);

    searcher_close(searcher);

    store_deref(store);
    return 0;
}

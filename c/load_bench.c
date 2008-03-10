#include "index.h"
#include "store.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int
main(int argc, char **argv)
{
    Store *store = open_fs_store("./bench_index/");
    IndexReader *ir = ir_open(store);
    LazyDoc *lz_doc;
    LazyDocField *lz_df;
    Document *doc;
    const int N = 10000000;
    clock_t time_taken;
    (void)argc; (void)argv;
    /* Normal Document Load */
    /*
    time_taken = clock();
    for (int i = 0; i < N; i++) {
        doc = ir->get_doc(ir, rand()%1000);
        doc_destroy(doc);
    }
    time_taken = clock() - time_taken;
    printf("Document - Took: %lu clocks in %0.3lf seconds\n", time_taken,
           (double)time_taken/CLOCKS_PER_SEC);
           */

    /* Lazy Document Load */
    time_taken = clock();
    for (int i = 0; i < N; i++) {
        lz_doc = ir->get_lazy_doc(ir, rand()%1000);
        lazy_doc_close(lz_doc);
    }

    time_taken = clock() - time_taken;
    printf("Lazy Document - Took: %lu clocks in %0.3lf seconds\n", time_taken,
           (double)time_taken/CLOCKS_PER_SEC);

    /* Lazy Document Load (with field) */
    /*
    time_taken = clock();
    for (int i = 0; i < N; i++) {
        lz_doc = ir->get_lazy_doc(ir, rand()%1000);
        lz_df = lz_doc->fields[1];
        lazy_df_get_data(lz_df, 0);
        lazy_doc_close(lz_doc);
    }

    time_taken = clock() - time_taken;
    printf("Lazy Document (with field load) - Took: %lu clocks in %0.3lf seconds\n", time_taken,
           (double)time_taken/CLOCKS_PER_SEC);
           */

    ir_close(ir);
    return 0;
}

#include "index.h"
#include "store.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/dir.h>
#define MAX_FILE_SIZE 300000
#define CORPUS_DIR "./corpus/"

int
main(int argc, char **argv)
{
    Config config = default_config;
    Store *store = open_fs_store("./bench_index/");
    IndexWriter *iw;
    DIR *d = opendir(CORPUS_DIR);
    struct dirent *de;
    char fname[200];
    char data[MAX_FILE_SIZE];
    FILE *file;
    Analyzer *a = whitespace_analyzer_new(true);
    Document *doc;
    int end;
    clock_t time_taken = clock();
    FieldInfos *fis = fis_new(STORE_NO, INDEX_YES, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new("name", STORE_YES, INDEX_NO, TERM_VECTOR_NO));
    index_create(store, fis);
    fis_destroy(fis);

    (void)argc;
    (void)argv;


    config.max_field_length = 0x7FFFFFFF;
    config.max_buffer_memory = 0x40000000;
    config.chunk_size = 0x8000000;
    config.max_buffered_docs = 0x8000000;

    iw = iw_open(store, a, &config);
    //a_deref(a);


    //config.merge_factor = 10;
    //config.min_merge_docs = 1000;

    while ((de = readdir(d)) != NULL) {
        DIR *d2;
        struct dirent *de2;
        if (de->d_name[0] == '.') continue;
        sprintf(fname, "%s%s", CORPUS_DIR, de->d_name);
        printf("%s\n", fname);
        d2 = opendir(fname);

        while ((de2 = readdir(d2)) != NULL) {
            if (strstr(de2->d_name, ".txt") != NULL) {
                //if (strstr(de->d_name, "Bazaar") != NULL) 
                doc = doc_new();
                doc_add_field(doc, df_add_data(df_new("name"), de2->d_name));
                sprintf(fname, "%s%s/%s", CORPUS_DIR, de->d_name, de2->d_name);
                file = fopen(fname, "r");
                end = fread(data, 1, MAX_FILE_SIZE, file);
                if (end >= MAX_FILE_SIZE) {
                    RAISE(EXCEPTION, "Oh dear!\n");
                }
                data[end] = '\0';
                doc_add_field(doc, df_add_data_len(df_new("text"), data, end));
                iw_add_doc(iw, doc);
                fclose(file);
                doc_destroy(doc);
            }
        }
        closedir(d2);
    }
    printf("optimizing\n");
    iw_optimize(iw);
    iw_close(iw);
    closedir(d);

    store_deref(store);

    time_taken = clock() - time_taken;
    printf("Took: %lu clocks in %0.3lf seconds\n", time_taken,
           (double)time_taken/CLOCKS_PER_SEC);
    return 0;
}

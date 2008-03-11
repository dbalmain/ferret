#include "index.h"
#include "store.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/dir.h>
#define MAX_FILE_SIZE 300000
#define CORPUS_DIR "corpus"
#define MAX_DOCS 2936357
//#define MAX_DOCS 200000

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
    //int i;
    FILE *file;
    Analyzer *a = whitespace_analyzer_new(true);
    Document *doc;
    int end;
    clock_t time_taken = clock();
    FieldInfos *fis = fis_new(STORE_COMPRESS, INDEX_YES, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new("name", STORE_YES, INDEX_UNTOKENIZED, TERM_VECTOR_NO));
    index_create(store, fis);
    fis_deref(fis);

    (void)argc;
    (void)argv;


    config.max_field_length = 0x7FFFFFFF;
    config.max_buffer_memory = 0x10000000;
    config.merge_factor = 1000;
    config.chunk_size = 0x4000000;
    config.max_buffered_docs = 0x100000;
    config.use_compound_file = false;

    iw = iw_open(store, a, &config);
    //for (i = 0; i < MAX_DOCS; i++) {
    //    if (i % 10000 == 0) {
    //        printf("Indexed %d documents in %0.2lf seconds\n", i, (double)(clock() - time_taken) / CLOCKS_PER_SEC);
    //    }
    //    sprintf(fname, "%s/%d/%d/%d/%d/%d/%08d.txt",
    //            CORPUS_DIR,
    //            (i/1000000)%10,
    //            (i/100000)%10,
    //            (i/10000)%10,
    //            (i/1000)%10,
    //            (i/100)%10,
    //            i);
    //    doc = doc_new();
    //    doc_add_field(doc, df_add_data(df_new("name"), fname));
    //    file = fopen(fname, "r");
    //    end = fread(data, 1, MAX_FILE_SIZE, file);
    //    if (end >= MAX_FILE_SIZE) {
    //        RAISE(EXCEPTION, "Oh dear!\n");
    //    }
    //    data[end] = '\0';
    //    doc_add_field(doc, df_add_data_len(df_new("text"), data, end));
    //    iw_add_doc(iw, doc);
    //    fclose(file);
    //    doc_destroy(doc);
    //}
    //printf("Indexed %d documents in %0.2lf seconds. Now optimizing\n", i, (double)(clock() - time_taken) / CLOCKS_PER_SEC);
    //iw_optimize(iw);
    //iw_close(iw);
    //store_deref(store);

    //time_taken = clock() - time_taken;
    //printf("Took: %lu clocks in %0.3lf seconds\n", time_taken, (double)time_taken/CLOCKS_PER_SEC);
    //return 0;
    //a_deref(a);


    //config.merge_factor = 10;
    //config.min_merge_docs = 1000;

    while ((de = readdir(d)) != NULL) {
        DIR *d2;
        struct dirent *de2;
        if (de->d_name[0] == '.') continue;
        sprintf(fname, "%s/%s", CORPUS_DIR, de->d_name);
        printf("%s\n", fname);
        d2 = opendir(fname);

        while ((de2 = readdir(d2)) != NULL) {
            if (strstr(de2->d_name, ".txt") != NULL) {
                //if (strstr(de->d_name, "Bazaar") != NULL) 
                doc = doc_new();
                sprintf(fname, "%s/%s/%s", CORPUS_DIR, de->d_name, de2->d_name);
                file = fopen(fname, "r");
                end = fread(data, 1, MAX_FILE_SIZE, file);
                if (end >= MAX_FILE_SIZE) {
                    RAISE(EXCEPTION, "Oh dear!\n");
                }
                data[end] = '\0';
                doc_add_field(doc, df_add_data_len(df_new("text"), data, end));
                doc_add_field(doc, df_add_data_len(df_new("text2"), data, end));
                doc_add_field(doc, df_add_data_len(df_new("text3"), data, end));
                doc_add_field(doc, df_add_data(df_new("name"), de2->d_name));
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

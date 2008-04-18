#include <string.h>
#include "benchmark.h"

#define N 10

static void do_strcmp()
{
    const char **word;
    char buf[100];
    int res;

    for (int i = 0; i < N; i++)
        for (word = WORD_LIST; *word; word++) {
            size_t len = strlen(*word);
            memcpy(buf, *word, len+1);
            res = strcmp(buf, *word);
        }
}

static void do_strncmp()
{
    const char **word;
    char buf[100];
    int res;

    for (int i = 0; i < N; i++)
        for (word = WORD_LIST; *word; word++) {
            size_t len = strlen(*word);
            memcpy(buf, *word, len+1);
            res = strncmp(buf, *word, len + 1);
        }
}

BENCH(strcmp_when_length_is_known)
{
    BM_COUNT(6);
    BM_DISCARD(1);
    BM_ADD(do_strcmp);
    BM_ADD(do_strncmp);
}

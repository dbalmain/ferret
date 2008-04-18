#include <string.h>
#include "benchmark.h"

#define N 10

static void do_strcmp()
{
    const char **word;
    char buf[100];
    int res, i;

    for (i = 0; i < N; i++)
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
    int res, i;

    for (i = 0; i < N; i++)
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

#undef N
#define N 20
#define BUFSIZE 32768
static void do_strncat()
{
    char buf[BUFSIZE];
    char buf_cpy[BUFSIZE];
    int i, offset;

    for (i = 0; i < N; i++) {
        buf[0] = '\0';
        for (i = 0; i < 8000; i++) {
            strcpy(buf_cpy, buf);
            offset = strlen(buf);
            strncat(buf, "my word", BUFSIZE - offset);
        }
    }
}

static void do_snprintf()
{
    char buf[BUFSIZE];
    char buf_cpy[BUFSIZE];
    int i, offset;

    for (i = 0; i < N; i++) {
        buf[0] = '\0';
        for (i = 0; i < 8000; i++) {
            strcpy(buf_cpy, buf);
            offset = strlen(buf);
            snprintf(buf, BUFSIZE, "%smy word", buf_cpy);
        }
    }
}

BENCH(snprintf_vs_strncat)
{
    BM_COUNT(6);
    BM_DISCARD(1);
    BM_ADD(do_snprintf);
    BM_ADD(do_strncat);
}

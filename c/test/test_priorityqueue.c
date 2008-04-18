#include "priorityqueue.h"
#include "global.h"
#include <string.h>
#include "test.h"

/**
 * Simple string less than function used for testing adding of strings to the
 * priority queue.
 */
static bool str_lt(void *p1, void *p2)
{
    return (strcmp((char *) p1, (char *) p2) < 0);
}

/**
 * Test basic PriorityQueue functions.
 */
static void test_pq(TestCase *tc, void *data)
{
    char *tmp;
    PriorityQueue *pq = pq_new(4, (lt_ft)&str_lt, &free);
    (void)data; /* suppress unused argument warning */

    Aiequal(0, pq->size);
    Aiequal(4, pq->capa);
    pq_push(pq, estrdup("bword"));
    Aiequal(1, pq->size);
    Asequal("bword", (char *) pq_top(pq));
    pq_push(pq, estrdup("cword"));
    Aiequal(2, pq->size);
    Asequal("bword", (char *) pq_top(pq));
    pq_push(pq, estrdup("aword"));
    Aiequal(3, pq->size);
    Asequal("aword", (char *) pq_top(pq));
    pq_push(pq, estrdup("dword"));
    Aiequal(4, pq->size);
    Asequal("aword", (char *) pq_top(pq));
    Asequal("aword", tmp = (char *) pq_pop(pq));
    Aiequal(3, pq->size);
    free(tmp);
    Asequal("bword", tmp = (char *) pq_pop(pq));
    Aiequal(2, pq->size);
    free(tmp);
    Asequal("cword", tmp = (char *) pq_pop(pq));
    Aiequal(1, pq->size);
    free(tmp);
    Asequal("dword", tmp = (char *) pq_pop(pq));
    Aiequal(0, pq->size);
    free(tmp);
    pq_destroy(pq);
}

/**
 * Free mock used to test that the PriorityQueue is being destroyed correctly
 * including it's elements
 */
static void pq_free_mock(void *p)
{
    char *str = (char *) p;
    strcpy(str, "free");
}

/**
 * Test pq_clear function
 */
static void test_pq_clear(TestCase *tc, void *data)
{
    char word1[10] = "word1";
    char word2[10] = "word2";
    char word3[10] = "word3";
    PriorityQueue *pq = pq_new(3, (lt_ft)&str_lt, &pq_free_mock);
    (void)data; /* suppress unused argument warning */

    pq_push(pq, word1);
    pq_push(pq, word2);
    pq_push(pq, word3);
    Aiequal(3, pq->size);
    pq_clear(pq);
    Aiequal(0, pq->size);
    Asequal("free", word1);
    Asequal("free", word2);
    Asequal("free", word3);
    pq_destroy(pq);
}

/**
 * Test that PriorityQueue will handle insert overflow. That is, when you
 * insert more than the PriorityQueue's capacity, the extra elements that drop
 * off the bottom are destroyed.
 */
static void test_pq_insert_overflow(TestCase *tc, void *data)
{
    char word1[10] = "word1";
    char word2[10] = "word2";
    char word3[10] = "word3";
    char word4[10] = "word4";
    char word5[10] = "word5";
    char word6[10] = "word6";
    PriorityQueue *pq = pq_new(3, (lt_ft)&str_lt, &pq_free_mock);
    (void)data; /* suppress unused argument warning */

    Aiequal(PQ_ADDED, pq_insert(pq, word2));
    Aiequal(PQ_ADDED, pq_insert(pq, word3));
    Aiequal(PQ_ADDED, pq_insert(pq, word4));
    Aiequal(PQ_INSERTED, pq_insert(pq, word5));
    Aiequal(PQ_INSERTED, pq_insert(pq, word6));
    Aiequal(PQ_DROPPED, pq_insert(pq, word1));
    Aiequal(3, pq->size);
    Asequal("free", word1);
    Asequal("free", word2);
    Asequal("free", word3);
    Asequal("word4", word4);
    Asequal("word5", word5);
    Asequal("word6", word6);
    pq_clear(pq);
    Aiequal(0, pq->size);
    Asequal("free", word4);
    Asequal("free", word5);
    Asequal("free", word6);
    pq_destroy(pq);
}

/**
 * Stress test the PriorityQueue. Make PQ_STRESS_SIZE much larger if you want
 * to really stress test PriorityQueue.
 */
#define PQ_STRESS_SIZE 1000
static void stress_pq(TestCase *tc, void *data)
{
    int i;
    char buf[100], *prev, *curr;
    PriorityQueue *pq = pq_new(PQ_STRESS_SIZE, (lt_ft)&str_lt, &free);
    (void)data; /* suppress unused argument warning */

    for (i = 0; i < PQ_STRESS_SIZE; i++) {
        sprintf(buf, "<%d>", rand());
        pq_push(pq, estrdup(buf));
    }
    Aiequal(PQ_STRESS_SIZE, pq->size);

    prev = (char *) pq_pop(pq);
    for (i = 0; i < PQ_STRESS_SIZE - 1; i++) {
        curr = (char *) pq_pop(pq);
        if (str_lt(curr, prev) == true) {
            Assert(false, "previous should be less than or equal to current");
            Tmsg("%d: %s, %s\n", i, prev, curr);
        }
        free(prev);
        prev = curr;
    }
    free(prev);
    pq_clear(pq);
    pq_destroy(pq);
}

/**
 * PriorityQueue's test suite
 */
TestSuite *ts_priorityqueue(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_pq, NULL);
    tst_run_test(suite, test_pq_clear, NULL);
    tst_run_test(suite, test_pq_insert_overflow, NULL);
    tst_run_test(suite, stress_pq, NULL);

    return suite;
}

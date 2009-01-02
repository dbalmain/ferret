#include "hash.h"
#include "symbol.h"
#include "global.h"
#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "testhelper.h"

static int *malloc_int(int val)
{
    int *i = ALLOC(int);
    *i = val;
    return i;
}

static void mark_free(void *p)
{
    strcpy((char *)p, "freed");
}
/**
 * Basic test for string Hash. Make sure string can be retrieved
 */
static void test_hash_str(TestCase *tc, void *data)
{
    Hash *h = h_new_str(NULL, &free);
    FILE *f;
    char buf[100], *t;
    memset(buf, 0, 100);
    (void)data; /* suppress unused argument warning */

    Assert(h_get(h, "one") == NULL, "No entries added yet");

    Aiequal(0, h_set(h, "one", malloc_int(1)));
    Aiequal(1, *(int *)h_get(h, "one"));
    Aiequal(true, h_del(h, "one"));

    Assert(h_get(h, "one") == NULL, "The Hash Entry has been deleted");

    /* test that h_has_key works even when value is set to null */
    h_set(h, "one", NULL);
    Apnull(h_get(h, "one"));
    Atrue(h_has_key(h, "one"));
    h_set(h, "two", malloc_int(2));
    h_set(h, "three", malloc_int(3));
    h_set(h, "four", malloc_int(4));

    f = temp_open();
    h_str_print_keys(h, f);
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, 100, f);
    fclose(f);
    Asequal("keys:\n\tfour\n\tone\n\tthree\n\ttwo\n", buf);
    h_destroy(h);

    /* test h_rem with allocated key */
    strcpy(buf, "key");
    h_new_str(&mark_free, (free_ft)NULL);
    h_set(h, buf, "val");
    Asequal("val", h_get(h, "key"));
    t = (char *)h_rem(h, "key", false);
    Asequal("val", t);
    Asequal("key", buf);
    h_set(h, buf, "new val");
    Asequal("new val", h_get(h, "key"));
    t = (char *)h_rem(h, "key", true);
    Asequal("new val", t);
    Asequal("freed", buf);
    h_destroy(h);
}

typedef struct Point
{
    int x;
    int y;
} Point;

static int point_eq(const void *q1, const void *q2)
{
    Point *p1 = (Point *)q1;
    Point *p2 = (Point *)q2;
    return p1->x == p2->x && p1->y == p2->y;
}

static unsigned long point_hash(const void *q)
{
    Point *p = (Point *)q;
    return p->x * p->y;
}

static Point *point_new(int x, int y)
{
    Point *p = ALLOC(Point);
    p->x = x;
    p->y = y;
    return p;
}

/**
 * Basic test for standard Hash. Make sure a non-string structure can be
 * used to key the Hash
 */
static void test_hash_point(TestCase *tc, void *data)
{
    Point *p1 = point_new(1, 2);
    Point *p2 = point_new(2, 1);
    Point *p3 = point_new(1, 2);
    Hash *h = h_new(&point_hash, &point_eq, NULL, &free);
    (void)data; /* suppress unused argument warning */

    Assert(point_eq(p1, p3), "should be equal");

    Assert(h_get(h, p1) == NULL, "No entries added yet");
    Assert(h_get(h, p2) == NULL, "No entries added yet");
    Assert(h_get(h, p3) == NULL, "No entries added yet");
    Aiequal(0, h->size);
    Aiequal(HASH_KEY_DOES_NOT_EXIST, h_set(h, p1, malloc_int(0)));
    Aiequal(1, h->size);
    Aiequal(0, *(int *)h_get(h, p1));
    Aiequal(HASH_KEY_SAME, h_set(h, p1, malloc_int(1)));
    Aiequal(1, h->size);
    Aiequal(1, *(int *)h_get(h, p1));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, h_set(h, p2, malloc_int(2)));
    Aiequal(2, h->size);
    Aiequal(2, *(int *)h_get(h, p2));
    Aiequal(HASH_KEY_EQUAL, h_set(h, p3, malloc_int(3)));
    Aiequal(2, h->size);
    Aiequal(3, *(int *)h_get(h, p3));
    Aiequal(3, *(int *)h_get(h, p1));
    Aiequal(true, h_del(h, p1));
    Aiequal(1, h->size);
    Assert(h_get(h, p1) == NULL, "Entry should be deleted");
    h_clear(h);
    Assert(h_get(h, p2) == NULL, "Entry should be deleted");
    Aiequal(0, h->size);
    h_destroy(h);
    free(p1);
    free(p2);
    free(p3);
}

/**
 * Test using integers as the key. This is also an example as to how to use
 * integers as the key.
 */
#define HASH_INT_TEST_SIZE 1000
static void test_hash_int(TestCase *tc, void *data)
{
    int i;
    Hash *h = h_new_int(&free);
    char buf[100];
    char *word;
    (void)data; /* suppress unused argument warning */

    Aiequal(0, h->size);
    Aiequal(HASH_KEY_DOES_NOT_EXIST, h_set_int(h, 0, estrdup("one")));
    Aiequal(1, h->size);
    Atrue(h_has_key_int(h, 0));
    Assert(h_set_safe_int(h, 10, estrdup("ten")), "Not existing");
    Assert(!h_set_safe_int(h, 10, "10"), "Won't overwrite existing");
    Asequal("ten", h_get_int(h, 10));
    Aiequal(2, h->size);
    Atrue(h_has_key_int(h, 10));
    h_set_int(h, 1000, estrdup("thousand"));
    Aiequal(3, h->size);
    Atrue(h_has_key_int(h, 1000));
    Asequal("thousand", word = (char *)h_rem_int(h, 1000));
    Aiequal(2, h->size);
    Atrue(!h_has_key_int(h, 1000));
    Atrue(h_has_key_int(h, 10));
    Atrue(!h_set_safe_int(h, 10, word));
    free(word);
    h_del_int(h, 10);

    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
        sprintf(buf, "<%d>", i);
        h_set_int(h, i, estrdup(buf));
    }
    Asequal("<0>", h_get_int(h, 0));
    Asequal("<100>", h_get_int(h, 100));
    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
        sprintf(buf, "<%d>", i);
        Asequal(buf, h_get_int(h, i));
    }

    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
        h_del_int(h, i);
    }
    Aiequal(0, h->size);

    h_destroy(h);
}

/**
 * Test using pointers as the key. This is also an example as to how to use
 * pointers as the key.
 */
#define HASH_INT_TEST_SIZE 1000
static void test_hash_ptr(TestCase *tc, void *data)
{
    Hash *h = h_new_ptr(&free);
    Symbol word1 = intern("one");
    Symbol word2 = intern("two");
    char *word_one = estrdup("one");
    int i;
    char buf[100];
    (void)data; /* suppress unused argument warning */

    Aiequal(ptr_hash(word1), (unsigned long)intern("one"));
    Atrue(ptr_eq(word1, intern("one")));
    h_set(h, word1, estrdup("1"));
    h_set(h, word2, estrdup("2"));
    h_set(h, word_one, estrdup("3"));
    Asequal("1", h_get(h, word1));
    Asequal("2", h_get(h, word2));
    Asequal("3", h_get(h, word_one));

    Aiequal(3, h->size);
    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
        char *str = strfmt("<%d>", i);
        h_set(h, I(str), str);
    }
    Asequal("<0>", h_get(h, I("<0>")));
    Asequal("<100>", h_get(h, I("<100>")));
    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
        sprintf(buf, "<%d>", i);
        Asequal(buf, h_get(h, I(buf)));
    }

    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
        sprintf(buf, "<%d>", i);
        h_del(h, I(buf));
    }
    Aiequal(3, h->size);

    h_destroy(h);
    free(word_one);
}

/**
 * Stress test the Hash. This test makes sure that the Hash still
 * works as it grows in size. The test has been run with 20,000,000 elements
 * on a 1Gb machine, but STRESS_SIZE is kept lower generally so that the tests
 * don't take too long.
 */
#define STRESS_SIZE 1000
static void stress_hash(TestCase *tc, void *data)
{
    int i, j, k;
    char buf[20];
    (void)data; /* suppress unused argument warning */

    for (k = 0; k < 1; k++) {
        Hash *h = h_new_str(&free, &free);
        for (i = 0; i < STRESS_SIZE; i++) {
            sprintf(buf, "(%d)", i);
            if (h_get(h, buf) != NULL) {
                Assert(false,
                       "h_get returned a result when it shouldn't have\n");
                return;
            }
            h_set(h, estrdup(buf), malloc_int(i));
        }


        for (j = 0; j < 1; j++) {
            for (i = 0; i < STRESS_SIZE; i++) {
                sprintf(buf, "(%d)", i);
                if (i != *(int *)h_get(h, buf)) {
                    Assert(false, "h_get returned an incorrect result\n");
                    return;
                }
            }
        }

        for (i = 0; i < STRESS_SIZE / 2; i++) {
            sprintf(buf, "(%d)", i);
            if (!h_del(h, buf)) {
                Assert(false, "h_del returned an error code\n");
                return;
            }
            if (h_get(h, buf) != NULL) {
                Assert(false, "h_get returned an incorrect result\n");
                return;
            }
        }

        Aiequal(STRESS_SIZE / 2, h->size);
        h_destroy(h);
    }
}

/**
 * Test that the hash table is ok while constantly growing and shrinking in
 * size
 */
static void test_hash_up_and_down(TestCase *tc, void *data)
{
    int i, j;
    char buf[20];

    Hash *h = h_new_str(&free, &free);
    (void)data; /* suppress unused argument warning */

    for (j = 0; j < 50; j++) {
        for (i = j * 10; i < j * 10 + 10; i++) {
            sprintf(buf, "(%d)", i);
            if (h_get(h, buf) != NULL) {
                Assert(false,
                       "h_get returned a result when it shouldn't have\n");
                return;
            }
            h_set(h, estrdup(buf), malloc_int(i));
            if (i != *(int *)h_get(h, buf)) {
                Assert(false, "h_get returned an incorrect result\n");
                return;
            }
        }

        for (i = j * 10; i < j * 10 + 10; i++) {
            sprintf(buf, "(%d)", i);
            if (!h_del(h, buf)) {
                Assert(false, "h_del returned an error code\n");
                return;
            }
            if (h_get(h, buf) != NULL) {
                Assert(false, "h_get returned an incorrect result\n");
                return;
            }
        }
    }
    Aiequal(0, h->size);
    h_destroy(h);
}

/**
 * Method used in h_each test
 */
static void test_each_ekv(void *key, void *value, Hash *h)
{
    if ((strlen((char *)key) % 2) == 0) {
        h_del(h, key);
    }
    else {
        h_del(h, value);
    }
}

/**
 * Test Hash cloning, ie. the h_clone function
 *
 * There is also a test in here of the h_each method.
 */
static void test_hash_each_and_clone(TestCase *tc, void *data)
{
    char *strs[] =
        { "one", "two", "three", "four", "five", "six", "seven", NULL };
    char **s = strs;
    Hash *h = h_new_str(&free, &free);
    Hash *ht2;
    (void)data; /* suppress unused argument warning */

    while (*s) {
        h_set(h, estrdup(*s), estrdup(*s));
        s++;
    }
    h_del(h, "two");
    h_del(h, "four");

    Aiequal(7, h->fill);
    Aiequal(5, h->size);

    ht2 = h_clone(h, (h_clone_ft)&estrdup, (h_clone_ft)&estrdup);

    Aiequal(7, h->fill);
    Aiequal(5, h->size);
    Aiequal(5, ht2->fill);
    Aiequal(5, ht2->size);

    h_del(h, "seven");

    Aiequal(7, h->fill);
    Aiequal(4, h->size);
    Aiequal(5, ht2->fill);
    Aiequal(5, ht2->size);

    h_each(h, (void (*)(void *k, void *v, void *a))&test_each_ekv, ht2);

    Aiequal(7, h->fill);
    Aiequal(4, h->size);
    Aiequal(5, ht2->fill);
    Aiequal(1, ht2->size);

    Apnotnull(h_get(ht2, "seven"));
    Apnull(h_get(ht2, "one"));
    Apnull(h_get(ht2, "two"));
    Apnull(h_get(ht2, "three"));
    Apnull(h_get(ht2, "four"));
    Apnull(h_get(ht2, "five"));
    Apnull(h_get(ht2, "six"));
    h_destroy(h);
    h_destroy(ht2);
}

/*
 * The following code is given as an example of how to use the h_each
 * function
 */

struct StringArray {
    char **strings;
    int cnt;
    int size;
};

static void add_string_ekv(void *key, void *value, struct StringArray *str_arr)
{
    (void)key; /* suppress unused argument warning */
    str_arr->strings[str_arr->cnt] = (char *)value;
    str_arr->cnt++;
}

static struct StringArray *h_extract_strings(Hash *h)
{
    struct StringArray *str_arr = ALLOC(struct StringArray);

    str_arr->strings = ALLOC_N(char *, h->size);
    str_arr->cnt = 0;
    str_arr->size = h->size;

    h_each(h, (h_each_key_val_ft)add_string_ekv, str_arr);

    return str_arr;
}

/**
 * Again, test the h_each function, this time testing the example given in the
 * documentation for the each function.
 */
static void test_hash_extract_strings(TestCase *tc, void *data)
{
    int i;
    struct StringArray *str_arr;
    const char *strs[] = {"one", "two", "three", "four", "five"};
    Hash *h = h_new_str(NULL, NULL);
    (void)data; /* suppress unused argument warning */

    for (i = 0; i < (int)NELEMS(strs); i++) {
        h_set(h, strs[i], (void *)strs[i]);
    }

    str_arr = h_extract_strings(h);

    if (Aiequal(NELEMS(strs), str_arr->size)) {
        for (i = 0; i < (int)NELEMS(strs); i++) {
            int j;
            bool str_found = false;
            for (j = 0; j < (int)NELEMS(strs); j++) {
                if (strcmp(strs[i], str_arr->strings[j]) == 0) {
                    str_found = true;
                }
            }
            Assert(str_found, "String was not found where it should've been");
        }
    }

    h_destroy(h);
    free(str_arr->strings);
    free(str_arr);
}

TestSuite *ts_hash(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_hash_str, NULL);
    tst_run_test(suite, test_hash_point, NULL);
    tst_run_test(suite, test_hash_int, NULL);
    tst_run_test(suite, test_hash_ptr, NULL);
    tst_run_test(suite, stress_hash, NULL);
    tst_run_test(suite, test_hash_up_and_down, NULL);
    tst_run_test(suite, test_hash_each_and_clone, NULL);
    tst_run_test(suite, test_hash_extract_strings, NULL);

    return suite;
}

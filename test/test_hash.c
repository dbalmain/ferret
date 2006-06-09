#include "test.h"
#include "hash.h"
#include "global.h"
#include <stdlib.h>

static int *malloc_int(int val)
{
    int *i = ALLOC(int);
    *i = val;
    return i;
}

/**
 * Basic test for string HashTable. Make sure string can be retrieved
 */
static void test_hash_str(tst_case *tc, void *data)
{
    HashTable *ht = h_new_str(NULL, &free);
    (void)data; /* suppress unused argument warning */

    Assert(h_get(ht, "one") == NULL, "No entries added yet");

    Aiequal(0, h_set(ht, "one", malloc_int(1)));
    Aiequal(1, *(int *)h_get(ht, "one"));
    Aiequal(true, h_del(ht, "one"));

    Assert(h_get(ht, "one") == NULL, "The Hash Entry has been deleted");

    /* test that h_has_key works even when value is set to null */
    h_set(ht, "one", NULL);
    Apnull(h_get(ht, "one"));
    Atrue(h_has_key(ht, "one"));
    h_destroy(ht);
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

static f_u32 point_hash(const void *q)
{
    Point *p = (Point *)q;
    return p->x * p->y;
}

static Point *point_create(int x, int y)
{
    Point *p = ALLOC(Point);
    p->x = x;
    p->y = y;
    return p;
}

/**
 * Basic test for standard HashTable. Make sure a non-string structure can be
 * used to key the HashTable
 */
static void test_hash_point(tst_case *tc, void *data)
{
    Point *p1 = point_create(1, 2);
    Point *p2 = point_create(2, 1);
    Point *p3 = point_create(1, 2);
    HashTable *ht = h_new(&point_hash, &point_eq, NULL, &free);
    (void)data; /* suppress unused argument warning */

    Assert(point_eq(p1, p3), "should be equal");

    Assert(h_get(ht, p1) == NULL, "No entries added yet");
    Assert(h_get(ht, p2) == NULL, "No entries added yet");
    Assert(h_get(ht, p3) == NULL, "No entries added yet");
    Aiequal(0, ht->size);
    Aiequal(HASH_KEY_DOES_NOT_EXIST, h_set(ht, p1, malloc_int(0)));
    Aiequal(1, ht->size);
    Aiequal(0, *(int *)h_get(ht, p1));
    Aiequal(HASH_KEY_SAME, h_set(ht, p1, malloc_int(1)));
    Aiequal(1, ht->size);
    Aiequal(1, *(int *)h_get(ht, p1));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, h_set(ht, p2, malloc_int(2)));
    Aiequal(2, ht->size);
    Aiequal(2, *(int *)h_get(ht, p2));
    Aiequal(HASH_KEY_EQUAL, h_set(ht, p3, malloc_int(3)));
    Aiequal(2, ht->size);
    Aiequal(3, *(int *)h_get(ht, p3));
    Aiequal(3, *(int *)h_get(ht, p1));
    Aiequal(true, h_del(ht, p1));
    Aiequal(1, ht->size);
    Assert(h_get(ht, p1) == NULL, "Entry should be deleted");
    h_clear(ht);
    Assert(h_get(ht, p2) == NULL, "Entry should be deleted");
    Aiequal(0, ht->size);
    h_destroy(ht);
    free(p1);
    free(p2);
    free(p3);
}

/**
 * Test using integers as the key. This is also an example as to how to use
 * integers as the key.
 */
#define HASH_INT_TEST_SIZE 1000
static void test_hash_int(tst_case *tc, void *data)
{
    f_u32 i;
    HashTable *ht = h_new_int(&free);
    char buf[100];
    char *word;
    (void)data; /* suppress unused argument warning */

    Aiequal(0, ht->size);
    Aiequal(HASH_KEY_DOES_NOT_EXIST, h_set_int(ht, 0, estrdup("one")));
    Aiequal(1, ht->size);
    Atrue(h_has_key_int(ht, 0));
    h_set_int(ht, 10, estrdup("ten"));
    Aiequal(2, ht->size);
    Atrue(h_has_key_int(ht, 10));
    h_set_int(ht, 1000, estrdup("thousand"));
    Aiequal(3, ht->size);
    Atrue(h_has_key_int(ht, 1000));
    Asequal("thousand", word = h_rem_int(ht, 1000));
    Aiequal(2, ht->size);
    Atrue(!h_has_key_int(ht, 1000));
    Atrue(h_has_key_int(ht, 10));
    Atrue(!h_set_safe_int(ht, 10, word));
    free(word);
    h_del_int(ht, 10);

    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
      sprintf(buf, "<%d>", i);
      h_set_int(ht, i, estrdup(buf));
    }
    Asequal("<0>", h_get_int(ht, 0));
    Asequal("<100>", h_get_int(ht, 100));
    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
      sprintf(buf, "<%d>", i);
      Asequal(buf, h_get_int(ht, i));
    }

    for (i = 0; i < HASH_INT_TEST_SIZE; i++) {
      h_del_int(ht, i);
    }
    Aiequal(0, ht->size);

    h_destroy(ht);
}

/**
 * Stress test the HashTable. This test makes sure that the HashTable still
 * works as it grows in size. The test has been run with 20,000,000 elements
 * on a 1Gb machine, but STRESS_SIZE is kept lower generally so that the tests
 * don't take too long.
 */
#define STRESS_SIZE 1000
static void stress_hash(tst_case *tc, void *data)
{
    int i, j, k;
    char buf[20];
    (void)data; /* suppress unused argument warning */

    for (k = 0; k < 1; k++) {
        HashTable *ht = h_new_str(&free, &free);
        for (i = 0; i < STRESS_SIZE; i++) {
            sprintf(buf, "(%d)", i);
            if (h_get(ht, buf) != NULL) {
                Assert(false,
                       "h_get returned a result when it shouldn't have\n");
                return;
            }
            h_set(ht, estrdup(buf), malloc_int(i));
        }


        for (j = 0; j < 1; j++) {
            for (i = 0; i < STRESS_SIZE; i++) {
                sprintf(buf, "(%d)", i);
                if (i != *(int *)h_get(ht, buf)) {
                    Assert(false, "h_get returned an incorrect result\n");
                    return;
                }
            }
        }

        for (i = 0; i < STRESS_SIZE / 2; i++) {
            sprintf(buf, "(%d)", i);
            if (!h_del(ht, buf)) {
                Assert(false, "h_del returned an error code\n");
                return;
            }
            if (h_get(ht, buf) != NULL) {
                Assert(false, "h_get returned an incorrect result\n");
                return;
            }
        }

        Aiequal(STRESS_SIZE / 2, ht->size);
        h_destroy(ht);
    }
}

/**
 * Test that the hash table is ok while constantly growing and shrinking in
 * size
 */
static void test_hash_up_and_down(tst_case *tc, void *data)
{
    int i, j;
    char buf[20];

    HashTable *ht = h_new_str(&free, &free);
    (void)data; /* suppress unused argument warning */

    for (j = 0; j < 50; j++) {
        for (i = j * 10; i < j * 10 + 10; i++) {
            sprintf(buf, "(%d)", i);
            if (h_get(ht, buf) != NULL) {
                Assert(false,
                       "h_get returned a result when it shouldn't have\n");
                return;
            }
            h_set(ht, estrdup(buf), malloc_int(i));
            if (i != *(int *)h_get(ht, buf)) {
                Assert(false, "h_get returned an incorrect result\n");
                return;
            }
        }

        for (i = j * 10; i < j * 10 + 10; i++) {
            sprintf(buf, "(%d)", i);
            if (!h_del(ht, buf)) {
                Assert(false, "h_del returned an error code\n");
                return;
            }
            if (h_get(ht, buf) != NULL) {
                Assert(false, "h_get returned an incorrect result\n");
                return;
            }
        }
    }
    Aiequal(0, ht->size);
    h_destroy(ht);
}

/**
 * Method used in h_each test
 */
static void test_each_ekv(void *key, void *value, HashTable *ht)
{
    if ((strlen((char *)key) % 2) == 0) {
        h_del(ht, key);
    }
    else {
        h_del(ht, value);
    }
}

/**
 * Test HashTable cloning, ie. the h_clone function
 *
 * There is also a test in here of the h_each method.
 */
static void test_hash_each_and_clone(tst_case *tc, void *data)
{
    char *strs[] =
        { "one", "two", "three", "four", "five", "six", "seven", NULL };
    char **s = strs;
    HashTable *ht = h_new_str(&free, &free);
    HashTable *ht2;
    (void)data; /* suppress unused argument warning */

    while (*s) {
        h_set(ht, estrdup(*s), estrdup(*s));
        s++;
    }
    h_del(ht, "two");
    h_del(ht, "four");

    Aiequal(7, ht->fill);
    Aiequal(5, ht->size);

    ht2 = h_clone(ht, (h_clone_func_t)&estrdup, (h_clone_func_t)&estrdup);

    Aiequal(7, ht->fill);
    Aiequal(5, ht->size);
    Aiequal(5, ht2->fill);
    Aiequal(5, ht2->size);

    h_del(ht, "seven");

    Aiequal(7, ht->fill);
    Aiequal(4, ht->size);
    Aiequal(5, ht2->fill);
    Aiequal(5, ht2->size);

    h_each(ht, (void (*)(void *k, void *v, void *a))&test_each_ekv, ht2);

    Aiequal(7, ht->fill);
    Aiequal(4, ht->size);
    Aiequal(5, ht2->fill);
    Aiequal(1, ht2->size);

    Apnotnull(h_get(ht2, "seven"));
    Apnull(h_get(ht2, "one"));
    Apnull(h_get(ht2, "two"));
    Apnull(h_get(ht2, "three"));
    Apnull(h_get(ht2, "four"));
    Apnull(h_get(ht2, "five"));
    Apnull(h_get(ht2, "six"));
    h_destroy(ht);
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

static struct StringArray *h_extract_strings(HashTable *ht)
{
    struct StringArray *str_arr = ALLOC(struct StringArray);

    str_arr->strings = ALLOC_N(char *, ht->size);
    str_arr->cnt = 0;
    str_arr->size = ht->size;

    h_each(ht, (h_each_key_val_ft)add_string_ekv, str_arr);

    return str_arr;
}

/**
 * Again, test the h_each function, this time testing the example given in the
 * documentation for the each function.
 */
static void test_hash_extract_strings(tst_case *tc, void *data)
{
    int i;
    struct StringArray *str_arr;
    const char *strs[] = {"one", "two", "three", "four", "five"};
    HashTable *ht = h_new_str(NULL, NULL);
    (void)data; /* suppress unused argument warning */

    for (i = 0; i < (int)NELEMS(strs); i++) {
        h_set(ht, strs[i], (void *)strs[i]);
    }

    str_arr = h_extract_strings(ht);

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

    h_destroy(ht);
    free(str_arr->strings);
    free(str_arr);
}

tst_suite *ts_hash(tst_suite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_hash_str, NULL);
    tst_run_test(suite, test_hash_point, NULL);
    tst_run_test(suite, test_hash_int, NULL);
    tst_run_test(suite, stress_hash, NULL);
    tst_run_test(suite, test_hash_up_and_down, NULL);
    tst_run_test(suite, test_hash_each_and_clone, NULL);
    tst_run_test(suite, test_hash_extract_strings, NULL);

    return suite;
}

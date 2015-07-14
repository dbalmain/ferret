#include "hashset.h"
#include "symbol.h"
#include "test.h"

/**
 * Test basic HashSet functions like adding elements and testing for
 * existence.
 */
static void test_hs(TestCase *tc, void *data)
{
    char *two = estrdup("two");
    HashSet *hs = hs_new_str(&free);
    (void)data; /* suppress unused argument warning */

    Atrue(HASH_KEY_EQUAL);
    Atrue(!HASH_KEY_DOES_NOT_EXIST);
    Aiequal(0, hs->size);

    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_add(hs, estrdup("one")));
    Aiequal(1, hs->size);
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, two));

    Aiequal(HASH_KEY_EQUAL, hs_add(hs, estrdup("one")));
    Aiequal(1, hs->size);
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "one"));

    hs_add(hs, two);
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "two"));
    Aiequal(HASH_KEY_SAME, hs_exists(hs, two));
    Apequal(two, hs_orig(hs, "two"));
    Apequal(two, hs_orig(hs, two));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "three"));
    Aiequal(2, hs->size);

    hs_add(hs, estrdup("three"));
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "two"));
    Aiequal(HASH_KEY_SAME, hs_exists(hs, two));
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "three"));
    Aiequal(3, hs->size);

    hs_del(hs, "two");
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "two"));
    Aiequal(HASH_KEY_EQUAL, hs_exists(hs, "three"));
    Aiequal(2, hs->size);
    Asequal("one", hs->first->elem);
    Asequal("three", hs->first->next->elem);

    hs_clear(hs);
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "two"));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "three"));
    Aiequal(0, hs->size);

    hs_destroy(hs);
}

static void test_hs_ptr(TestCase *tc, void *data)
{
    HashSet *hs = hs_new_ptr(NULL);
    Symbol word1 = intern("one"), word2 = intern("two");
    char *word_one = estrdup("one");
    (void)data; /* suppress unused argument warning */

    Aiequal(0, hs->size);

    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_add(hs, (void *)word1));
    Aiequal(1, hs->size);
    Aiequal(HASH_KEY_SAME, hs_exists(hs, word1));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_SAME, hs_add(hs, (void *)word1));
    Aiequal(1, hs->size);

    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_add(hs, (void *)word2));
    Aiequal(2, hs->size);
    Aiequal(HASH_KEY_SAME, hs_exists(hs, word2));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "two"));
    Aiequal(HASH_KEY_SAME, hs_add(hs, (void *)word2));
    Aiequal(2, hs->size);

    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_add(hs, (void *)word_one));
    Aiequal(3, hs->size);
    Aiequal(HASH_KEY_SAME, hs_exists(hs, word_one));
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs, "one"));
    Aiequal(HASH_KEY_SAME, hs_add(hs, (void *)word_one));
    Aiequal(3, hs->size);

    free(word_one);
    hs_destroy(hs);
}

/**
 * Test hs_add_safe
 */

static void test_hs_add_safe(TestCase *tc, void *data)
{
    char *str = estrdup("one");
    HashSet *hs = hs_new_str(&free);
    (void)data; /* suppress unused argument warning */

    Atrue(hs_add_safe(hs, str));
    Atrue(hs_add_safe(hs, (char *)str));
    Atrue(!hs_add_safe(hs, (char *)"one"));

    int to_add = 100;
    int idx = 0;
    for (; to_add >= 0; --to_add)
    {
        snprintf(str, sizeof(str)/sizeof(str[0]), "%d", idx);
        Atrue(hs_add_safe(hs, estrdup(str)));
        ++idx;
    }

    for (idx = 0; idx <= to_add; ++idx)
    {
        snprintf(str, sizeof(str)/sizeof(str[0]), "%d", idx);
        Aiequal(HASH_KEY_EQUAL, hs_exists(hs, str));
    }

    hs_destroy(hs);
}

/**
 * Test merging of two HashSets. When one HashSet is merged into another the
 * HashSet that was merged should be destroyed including all elements that
 * weren't added to the final HashSet.
 */
static void test_hs_merge(TestCase *tc, void *data)
{
    HashSet *hs1 = hs_new_str(&free);
    HashSet *hs2 = hs_new_str(&free);
    HashSetEntry *hse;
    (void)data; /* suppress unused argument warning */

    hs_add(hs1, estrdup("one"));
    hs_add(hs1, estrdup("two"));
    hs_add(hs1, estrdup("three"));
    hs_add(hs2, estrdup("two"));
    hs_add(hs2, estrdup("three"));
    hs_add(hs2, estrdup("four"));
    Aiequal(3, hs1->size);
    Aiequal(3, hs2->size);
    hs_merge(hs1, hs2);
    Aiequal(4, hs1->size);
    hse = hs1->first;
    Asequal("one", hse->elem);
    hse = hse->next;
    Asequal("two", hse->elem);
    hse = hse->next;
    Asequal("three", hse->elem);
    hse = hse->next;
    Asequal("four", hse->elem);
    hse = hse->next;
    Apnull(hse);
    hs_destroy(hs1);
}

/**
 * Free Mock used to test that certain elements are being freed when the
 * HashSet is destroyed.
 */
static void hs_free_mock(void *p)
{
    char *str = (char *) p;
    strcpy(str, "free");
}

/**
 * Test that HashSets are freed correctly. That is, make sure that when a
 * HashSet is destroyed, all elements have the correct free function called on
 * them
 */
static void test_hs_free(TestCase *tc, void *data)
{
    char str1[10], str2[10], str3[10], str4[10], str5[10];
    HashSet *hs1 = hs_new_str(&hs_free_mock);
    HashSet *hs2 = hs_new_str(&hs_free_mock);
    HashSetEntry *hse;
    (void)data; /* suppress unused argument warning */

    strcpy(str1, "one");
    strcpy(str2, "one");
    Atrue(hs_add_safe(hs1, (char *)str1));
    Atrue(hs_add_safe(hs1, (char *)str1));
    Atrue(!hs_add_safe(hs1, (char *)"one"));
    Asequal("one", str1);

    hs_add(hs1, str2);
    Asequal("free", str2);
    hs_rem(hs1, "one");
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs1, "one"));
    Asequal("one", str1);
    hs_add(hs1, str1);
    hs_del(hs1, "one");
    Aiequal(HASH_KEY_DOES_NOT_EXIST, hs_exists(hs1, "one"));
    Asequal("free", str1);

    strcpy(str1, "one");
    strcpy(str2, "two");
    strcpy(str3, "three");
    strcpy(str4, "three");
    strcpy(str5, "four");
    hs_add(hs1, str1);
    hs_add(hs1, str2);
    hs_add(hs1, str3);
    hs_add(hs2, str2);
    hs_add(hs2, str4);
    hs_add(hs2, str5);
    Aiequal(3, hs1->size);
    Aiequal(3, hs2->size);
    hs_merge(hs1, hs2);
    Aiequal(4, hs1->size);
    Asequal("free", str4);
    hse = hs1->first;
    Apequal(str1, hse->elem);
    hse = hse->next;
    Apequal(str2, hse->elem);
    hse = hse->next;
    Apequal(str3, hse->elem);
    hse = hse->next;
    Apequal(str5, hse->elem);
    hse = hse->next;
    Apnull(hse);
    Asequal("one", str1);
    Asequal("two", str2);
    Asequal("three", str3);
    Asequal("four", str5);
    hs_destroy(hs1);
    Asequal("free", str1);
    Asequal("free", str2);
    Asequal("free", str3);
    Asequal("free", str5);
}

/**
 * HashSet stress test. Make sure the HashSet works under load.
 */
#define HS_STRESS_NUM 10000 /* number of adds to the HashSet */
#define HS_STRESS_MAX 100   /* number of elements allowed in the HashSet */
static void stress_hs(TestCase *tc, void *data)
{
    int i;
    char buf[100];
    HashSet *hs = hs_new_str(&free);
    (void)data; /* suppress unused argument warning */

    for (i = 0; i < HS_STRESS_NUM; i++) {
        sprintf(buf, "<%d>", rand() % HS_STRESS_MAX);
        hs_add(hs, estrdup(buf));
    }
    Assert(hs->size <= HS_STRESS_MAX,
           "all numbers should be between 0 and %d", HS_STRESS_MAX);

    /* make sure none of the slots were left out for next test */
    for (i = 0; i < HS_STRESS_MAX; i++) {
        sprintf(buf, "<%d>", i);
        hs_add(hs, estrdup(buf));
    }

    for (i = 0; i < HS_STRESS_MAX; i++) {
        sprintf(buf, "<%d>", i);
        if (!Atrue(hs_exists(hs, buf))) {
            Tmsg("Couldn't find \"%s\"", buf);
        }
    }
    hs_destroy(hs);
}

/**
 * HashSet Test Suite
 */
TestSuite *ts_hashset(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_hs, NULL);
    tst_run_test(suite, test_hs_ptr, NULL);
    tst_run_test(suite, test_hs_add_safe, NULL);
    tst_run_test(suite, test_hs_merge, NULL);
    tst_run_test(suite, test_hs_free, NULL);
    tst_run_test(suite, stress_hs, NULL);

    return suite;
}

#include "except.h"
#include "test.h"

static void raise_eof()
{
    RAISE(EOF_ERROR, "Error, end of file");
}

/**
 * Test the basic test functions
 */
static void test_asserts(TestCase *tc, void *data)
{
    char *p[10];

    int ia1[3] = { 1, 2, 3 };
    int ia2[3] = { 1, 2, 3 };

    static char *sa1[10] = { "one", "two", "three" };
    static char *sa2[10] = { "one", "two", "three" };
    (void)data; /* suppress unused argument warning */

    Aaiequal(ia1, ia2, 3);
    Aasequal(sa1, sa2, 3);
    Aiequal(1, 1);
    Asequal("String One", "String One");
    Asnequal("String One", "String Two", 7);
    Apnotnull(p);
    Apequal(p, p);
    Atrue(p != NULL);
    Atrue(1);
    Atrue(!0);
    Assert(1 == 1, "%d != %d", 1, 1);
    Araise(EOF_ERROR, &raise_eof, NULL);
}

static void raise_nothing(void *nothing)
{
    (void)nothing;
}
/**
 * Test test failures. This method isn't called because we want 100% tests
 * passing but if you want to check the tests work in case of failure, run
 * this test.
 */
static void test_failures(TestCase *tc, void *data)
{
    void *q = NULL;
    void *p = emalloc(10);

    int ia1[3] = { 1, 2, 3 };
    int ia2[3] = { 1, 2, 4 };

    static char *sa1[10] = { "one", "two", "three" };
    static char *sa2[10] = { "one", "two", "there" };
    (void)data; /* suppress unused argument warning */

    Aaiequal(ia1, ia2, 3);
    Aasequal(sa1, sa2, 3);
    Aiequal(1, 2);
    Asequal("String One", "String Two");
    Asnequal("String One", "String Two", 8);
    Apnotnull(q);
    Apequal(p, q);
    Atrue(1 == 2);
    Assert(1 == 2, "%d != %d", 1, 2);
    Atrue(0);
    free(p);
    Araise(EXCEPTION, &raise_eof, NULL);
    Araise(EXCEPTION, &raise_nothing, NULL);
}

TestSuite *ts_test(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_asserts, NULL);
    if (false) {
        tst_run_test(suite, test_failures, NULL);
    }

    return suite;
}

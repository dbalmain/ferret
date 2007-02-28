#include "test.h"

/**
 * Test the strfmt functions. This method is like sprintf except that it
 * allocates the necessary space for the string and pretty prints floats.
 */
static void test_strfmt(tst_case *tc, void *data)
{
    char *s;
    (void)data; /* suppress unused argument warning */

    s = strfmt("%s%s%s%s", "one", "two", "three", "four");
    Asequal("onetwothreefour", s);
    free(s);
    s = strfmt("%s%s%s%s", "one", "two", "three", "four", "five");
    Asequal("onetwothreefour", s);
    free(s);

    s = strfmt("%d, %d, %d, %d", 987831, 3212346, 1231231, 431231);
    Asequal("987831, 3212346, 1231231, 431231", s);
    free(s);
    s = strfmt("%d, %d, %d, %d", 987831, 3212346, 1231231, 431231, 1209387);
    Asequal("987831, 3212346, 1231231, 431231", s);
    free(s);

    s = strfmt("%f, %f, %f, %f", 0.000234234, 1234.123, 7890.342,
               78907.1200000);
    Asequal("0.000234234, 1234.123, 7890.342, 78907.12", s);
    free(s);

    s = strfmt("%f, %f, %f, %f", 0.000234234, 1234.123, 7890.342, 78907.120,
               1.0);
    Asequal("0.000234234, 1234.123, 7890.342, 78907.12", s);
    free(s);

    s = strfmt("%s, %d, %f, %s, %d, %f", "one", 23462346, 0.0002342,
               "two", 23452346, 0.213451);
    Asequal("one, 23462346, 0.0002342, two, 23452346, 0.213451", s);
    free(s);
    s = strfmt("%s, %d, %f, %s, %d, %f", "one", 23462346, 0.0002342,
               "two", 23452346, 0.213451, "three", 342234, 0.908709);
    Asequal("one, 23462346, 0.0002342, two, 23452346, 0.213451", s);
    free(s);
}

tst_suite *ts_global(tst_suite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_strfmt, NULL);

    return suite;
}

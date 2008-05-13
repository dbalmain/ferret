#include <signal.h>
#include "test.h"

/**
 * Test the strfmt functions. This method is like sprintf except that it
 * allocates the necessary space for the string and pretty prints floats.
 */
static void test_strfmt(TestCase *tc, void *data)
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

/**
 * Generate a stacktrace, make sure it does something
 */
static void test_stacktrace(TestCase *tc, void *data)
{
    FILE *old_stream = x_exception_stream;
    (void)data; /* suppress warning */
    (void)tc;   /* won't be used when no variadic arg macro support */

    x_exception_stream = tmpfile();
    print_stacktrace();

    Assert(ftell(x_exception_stream), "Stream position should not be 0");
    fclose(x_exception_stream);
    x_exception_stream = old_stream;
}

/**
 * Generate a normally fatal signal, which gets caught
 */
static void test_sighandler(TestCase *tc, void *data)
{
    bool  old_abort = x_abort_on_exception;
    FILE *old_stream = x_exception_stream;
    (void)data;
    (void)tc;

    x_abort_on_exception = false;
    x_exception_stream = tmpfile();

    raise(SIGSEGV);

    Assert(ftell(x_exception_stream), "Stream position should not be 0");
    fclose(x_exception_stream);

    x_exception_stream = old_stream;
    x_abort_on_exception = old_abort;
}

static void test_count_leading_zeros(TestCase *tc, void *data)
{
    (void)data;
    Aiequal(32, frt_count_leading_zeros(0));
    Aiequal(31, frt_count_leading_zeros(1));
    Aiequal(30, frt_count_leading_zeros(2));
    Aiequal(30, frt_count_leading_zeros(3));
    Aiequal( 0, frt_count_leading_zeros(0xffffffff));
}

static void test_count_leading_ones(TestCase *tc, void *data)
{
    (void)data;
    Aiequal( 0, frt_count_leading_ones(0));
    Aiequal( 0, frt_count_leading_ones(1));
    Aiequal( 0, frt_count_leading_ones(2));
    Aiequal( 0, frt_count_leading_ones(3));
    Aiequal(32, frt_count_leading_ones(0xffffffff));
}

static void test_count_trailing_zeros(TestCase *tc, void *data)
{
    (void)data;
    Aiequal(32, frt_count_trailing_zeros(0));
    Aiequal( 0, frt_count_trailing_zeros(1));
    Aiequal( 1, frt_count_trailing_zeros(2));
    Aiequal( 0, frt_count_trailing_zeros(3));
    Aiequal( 4, frt_count_trailing_zeros(0xfffffff0));
    Aiequal( 0, frt_count_trailing_zeros(0xffffffff));
}

static void test_count_trailing_ones(TestCase *tc, void *data)
{
    (void)data;
    Aiequal( 0, frt_count_trailing_ones(0));
    Aiequal( 1, frt_count_trailing_ones(1));
    Aiequal( 0, frt_count_trailing_ones(2));
    Aiequal( 2, frt_count_trailing_ones(3));
    Aiequal( 0, frt_count_trailing_ones(0xfffffff0));
    Aiequal(32, frt_count_trailing_ones(0xffffffff));
}

static void test_count_zeros(TestCase *tc, void *data)
{
    (void)data;
    Aiequal(32, frt_count_zeros(0));
    Aiequal(31, frt_count_zeros(1));
    Aiequal(31, frt_count_zeros(2));
    Aiequal(30, frt_count_zeros(3));
    Aiequal( 4, frt_count_zeros(0xfffffff0));
    Aiequal( 0, frt_count_zeros(0xffffffff));
}

static void test_count_ones(TestCase *tc, void *data)
{
    (void)data;
    Aiequal( 0, frt_count_ones(0));
    Aiequal( 1, frt_count_ones(1));
    Aiequal( 1, frt_count_ones(2));
    Aiequal( 2, frt_count_ones(3));
    Aiequal(28, frt_count_ones(0xfffffff0));
    Aiequal(32, frt_count_ones(0xffffffff));
}

static void test_round2(TestCase *tc, void *data)
{
    (void)data;
    Aiequal(   1, frt_round2(0));
    Aiequal(   2, frt_round2(1));
    Aiequal(   4, frt_round2(2));
    Aiequal(   4, frt_round2(3));
    Aiequal(1024, frt_round2(1023));
    Aiequal(2048, frt_round2(1024));
}

TestSuite *ts_global(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_strfmt, NULL);
    tst_run_test(suite, test_stacktrace, NULL);
    tst_run_test(suite, test_sighandler, NULL);
    tst_run_test(suite, test_count_leading_zeros, NULL);
    tst_run_test(suite, test_count_leading_ones, NULL);
    tst_run_test(suite, test_count_trailing_zeros, NULL);
    tst_run_test(suite, test_count_trailing_ones, NULL);
    tst_run_test(suite, test_count_zeros, NULL);
    tst_run_test(suite, test_count_ones, NULL);
    tst_run_test(suite, test_round2, NULL);
    return suite;
}

#ifndef TEST_H
#define TEST_H

#include "global.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

/**
 * A test suite is a linked-list of sub-suites that are made up of test cases
 * which contain a list of assertions.
 */
typedef struct TestSubSuite
{
    char *name;
    int num_test;
    int failed;
    int not_run;
    int not_impl;
    struct TestSubSuite *next;
} TestSubSuite;

typedef struct TestSuite
{
    TestSubSuite *head;
    TestSubSuite *tail;
} TestSuite;

typedef struct TestCase
{
    char *name;
    int failed;
    TestSubSuite *suite;
} TestCase;

/* a test function needs to match this signature */
typedef void (*test_func)(TestCase *tc, void *data);

/**
 * Add a sub-suite to the linked-list of test suites. This function is usually
 * used like this;
 * 
 * TestSuite *ts_<sub-suite-name>(TestSuite *suite)
 * {
 *     suite = ADD_SUITE(suite);
 * 
 *     tst_run_test(suite, test_<test-name>, NULL);
 * 
 *     return suite;
 * }
 *
 * This function then needs to be included in the test_list in test/all_tests.h
 *
 * @param the previous suite that was added to which we will add this suite
 * @param the suites name. This will be displayed in the test output. Use the
 *   ADD_SUITE macro if you want the file_name to be used as the name for the
 *   suite. This will suffice in most cases.
 * @return the current suite
 */
extern TestSuite *tst_add_suite(TestSuite *suite, const char *suite_name);
#define ADD_SUITE(suite) tst_add_suite(suite, __FILE__);


/**
 * Run a test function. This function should be run from within
 * ts_<sub-suite-name> function. See tst_add_suite.
 *
 * @param suite the test suite to include this test in
 * @param func the test function to call
 * @param value a parameter to pass to the test function. If you need to pass
 *   more than one value to the function use an argument struct
 * @param test_name the name of the test which will be displayed in the error
 *   output. Use the tst_run_tests macro if you just want the name of the
 *   function to be used. This will suffice in most cases.
 */
extern void tst_run_test_with_name(TestSuite *suite, test_func func,
                                   void *value, char *test_name);
#define tst_run_test(ts, f, val) tst_run_test_with_name(ts, f, (val), #f)

/**
 * Add a message to the test output diagnostics. This function should be used
 * like this;
 *
 * static void test_something(TestCase *tc, void *data)
 * {
 *   // do some stuff
 *
 *   if (!Atrue(a->eq(b)) {
 *     // Error diagnostics will already be logged, now we just want to add a
 *     //little more information
 *     TMsg("%s != %s", a->to_s(a), b->to_s(b));
 *   }
 *   ///...
 * }
 *
 * @param fmt the format for the message. This is exactly the same as the
 *   format you would pass to printf.
 * @param ... the arguments to print in the format
 */
extern void Tmsg(const char *fmt, ...);

/**
 * Same as Tmsg but you must add your own new-lines and formatting.
 *
 * @param fmt the format for the message. This is exactly the same as the
 *   format you would pass to printf.
 * @param ... the arguments to print in the format
 */
extern void Tmsg_nf(const char *fmt, ...);

/**
 * Add a message to the test output diagnostics. See Tmsg for usage. vTmsg is
 * to Tmsg want vprintf is to printf
 */
extern void vTmsg(const char *fmt, va_list args);

/**
 * Test that a function raises a particular exception. Pass a function to call
 * that should raise the exception if everything is working as expected. You
 * can use the _args_ parameter to pass arguments to the function.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param err_code the exception you expect to be raised
 * @param func the function to call that is supposed to raise the exception
 * @param args the args to pass to the function
 * @return true if the test passed
 */
extern bool tst_raise(int line_num, TestCase *tc, const int err_code,
                      void (*func)(void *args), void *args);

/**
 * Test that two integers are equal. If they are not equal then add an error
 * test diagnostics. You should use the Aiequal(expected, actual) macro to
 * call this function so that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @return true if the test passed
 */
extern bool tst_int_equal(int line_num, TestCase *tc, const u64 expected,
                          const u64 actual);

/**
 * Test that two floats (or doubles) are equal (within 0.001%). If they are
 * not equal then add an error test diagnostics. You should use the
 * Afequal(expected, actual) macro to call this function so that the
 * line number will be added automatically.  A delta is used to account for
 * floating point error. The test for equality is;
 *
 * <pre>
 *
 *    diff = (expected - actual) / expected;
 *    if (abs(diff) < 0.00001) {
 *        return true;
 *    }
 *
 * </pre>
 *
 * To use a different delta value use tst_flt_delta_equal or the macro
 * Afdequal.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @return true if the test passed
 */
extern bool tst_flt_equal(int line_num, TestCase *tc, const double expected,
                          const double actual);

/**
 * Test that two floats (or doubles) are equal (within +delta+/100%). If they
 * are not equal then add an error test diagnostics. You should use the
 * Afdequal(expected, actual, delta) macro to call this function so that the
 * line number will be added automatically. The test for equality is;
 *
 * <pre>
 *
 *    diff = (expected - actual) / expected;
 *    if (abs(diff) < delta) {
 *        return true;
 *    }
 *
 * </pre>
 *
 * To use the default delta value use tst_flt_equal or the macro Afequal.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @param delta the allowed fraction of difference
 * @return true if the test passed
 */
extern bool tst_flt_delta_equal(int line_num, TestCase *tc, const double expected,
                                const double actual, const double delta);

/**
 * Test that two strings are equal. If they are not equal then add an error to
 * the test diagnostics. You should use the Asequal(expected, actual) macro to
 * call this function so that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @return true if the test passed
 */
extern bool tst_str_equal(int line_num, TestCase *tc, const char *expected,
                          const char *actual);

/**
 * Test one string contains another string. This test is similar to the
 * Standard C strstr function. If haystack doesn't contain needle then add an
 * error to the test diagnostics. You should use the Astrstr(haystack, needle)
 * macro to call this function so that the line number will be added
 * automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if the test passed
 */
extern bool tst_strstr(int line_num, TestCase *tc, const char *haystack,
                       const char *needle);

/**
 * Test that two arrays of integers are equal, ie the have the same elements
 * for up to +size+ elements. If they are not equal then add an error to the
 * test diagnostics. You should use the Aaiequal(expected, actual, size) macro
 * to call this function so that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @param size the number of elements in the array. Both arrays should have at
 *   least this number of elements allocated or you will get memory overflow
 * @return true if the test passed
 */
extern bool tst_arr_int_equal(int line_num, TestCase *tc, const int *expected,
                              const int *actual, int size);

/**
 * Test that two arrays of strings are equal, ie the have the same elements
 * for up to +size+ elements. If they are not equal then add an error to the
 * test diagnostics. You should use the Aasequal(expected, actual, size) macro
 * to call this function so that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @param size the number of elements in the array. Both arrays should have at
 *   least this number of elements allocated or you will get memory overflow
 * @return true if the test passed
 */
extern bool tst_arr_str_equal(int line_num, TestCase *tc, const char **expected,
                              const char **actual, int size);

/**
 * Test that two strings are equal up to +n+ bytes. If they are not equal then
 * add an error to the test diagnostics. You should use the Asnequal(expected,
 * actual, bytes) macro to call this function so that the line number will be
 * added automatically. For example Asnequal("Discovery", "Discotheque", 5)
 * will pass and return try but Asnequal("Discovery", "Discotheque", 6) will
 * fail and return false.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @param bytes the number of bytes in the strings. Both strings should have at
 *   least this number of bytes allocated or you will get memory overflow
 * @return true if the test passed
 */
extern bool tst_str_nequal(int line_num, TestCase *tc, const char *expected,
                           const char *actual, size_t n);

/**
 * Test that +ptr+ is NULL. If it is not NULL then add an error to the test
 * diagnostics. You should use the Apnull(ptr) macro to call this function so
 * that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param ptr fail if this is NULL
 * @return true if the test passed, ie +ptr+ was NULL.
 */
extern bool tst_ptr_null(int line_num, TestCase *tc, const void *ptr);

/**
 * Test that +ptr+ is not NULL. If it is NULL then add an error to the test
 * diagnostics. You should use the Apnotnull(ptr) macro to call this function so
 * that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param ptr fail if this is NULL
 * @return true if the test passed, ie +ptr+ was NULL.
 */
extern bool tst_ptr_notnull(int line_num, TestCase *tc, const void *ptr);

/**
 * Test that two ptrs point to the same memory (ie, they are equal). If they
 * are not equal then add an error to the test diagnostics. You should use the
 * Apequal(expected, actual) macro to call this function so that the line
 * number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @return true if the test passed
 */
extern bool tst_ptr_equal(int line_num, TestCase *tc, const void *expected,
                          const void *actual);

/**
 * Test that the +condition+ is true. If it is false, add an error to the test
 * diagnostics. You should use the Atrue(condition) macro to call this
 * function so that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param expected the expected value
 * @param actual the actual value
 * @return true if the test passed
 */
extern bool tst_true(int line_num, TestCase *tc, int condition);

/**
 * Fail. Add an error to the test diagnostics. You should use the Afail(msg)
 * macro to call this function so that the line number will be added
 * automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param fmt message to display. It can contain the same formatting options
 *   as printf
 * @param ... variables to interpolate into the format
 * @return false always (for consistency with other test functions)
 */
extern bool tst_fail(int line_num, TestCase *tc, const char *fmt, ...);

/**
 * Add an error to the test diagnotistics specifying that this test has not
 * yet been implemented. You should use the Anotimpl(message) macro to call
 * this function so that the line number will be added automatically.
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param message message to be added to the test diagnostics along with the
 *   "Not Implemented" message
 * @return true if the test passed
 */
extern bool tst_not_impl(int line_num, TestCase *tc, const char *message);

/**
 * Test that +condition+ is true. If it isn't, add an error to the test
 * diagnostics along with the message in +fmt+, .... You can use the
 * Assert(condition, fmt, ...) macro but note that it is not ANSI C compatible
 * because of the variadic arguments. Instead you can use Atrue without the
 * error message or an of the other Assertions along with Tmsg like this;
 *
 * <pre>
 *
 * if (!Aiequal(int1, int2)) {
 *     Tmsg("int1:%d should have been equal to int2:%d\n", int1, int2);
 * }
 * 
 * </pre>
 *
 * @param line_num the line number this function is called from
 * @param tc the test case to record the diagnostics
 * @param condition condition to test
 * @param fmt message to display. It can contain the same formatting options
 *   as printf
 * @param ... variables to interpolate into the format
 * @return true if the test passed
 */
extern bool tst_assert(int line_num, TestCase *tc, int condition,
                       const char *fmt, ...);

#define Araise(e, f, b)\
    tst_raise(__LINE__, tc, e, (void (*)(void *))(f), (void *)(b))
#define Aiequal(a, b)      tst_int_equal(__LINE__, tc, (u64)(a), (u64)(b))
#define Afequal(a, b)      tst_flt_equal(__LINE__, tc, (a), (b))
#define Afdequal(a, b, d)  tst_flt_delta_equal(__LINE__, tc, (a), (b), (d))
#define Asequal(a, b)      tst_str_equal(__LINE__, tc, (const char *)(a), (const char *)(b))
#define Astrstr(a, b)      tst_strstr(__LINE__, tc, (const char *)(a), (const char *)(b))
#define Aaiequal(a, b, n)  tst_arr_int_equal(__LINE__, tc, (const int *)(a), (const int *)(b), (n))
#define Aasequal(a, b, n)  tst_arr_str_equal(__LINE__, tc, (const char **)(a), (const char **)(b), (n))
#define Asnequal(a, b, n)  tst_str_nequal(__LINE__, tc, (const char *)(a), (const char *)(b), (n))
#define Apnull(a)          tst_ptr_null(__LINE__, tc, (a))
#define Apnotnull(a)       tst_ptr_notnull(__LINE__, tc, (a))
#define Apequal(a, b)      tst_ptr_equal(__LINE__, tc, (a), (b))
#define Atrue(a)           tst_true(__LINE__, tc, (a))
#define Afail(a)           tst_fail(__LINE__, tc, (a))
#define Anotimpl(a)        tst_not_impl(__LINE__, tc, (a))
#ifdef FRT_HAS_ISO_VARARGS
# define Assert(a, ...) tst_assert(__LINE__, tc, (a), __VA_ARGS__)
#elif defined(FRT_HAS_GNUC_VARARGS)
# define Assert(a, args...) tst_assert(__LINE__, tc, (a), ##args)
#else
extern bool Assert(int condition, const char *fmt, ...);
#endif

#endif

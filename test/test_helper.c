#include "helper.h"
#include "test.h"

static void test_hlp_string_diff(tst_case * tc, void *data)
{
    (void)data; /* suppress unused argument warning */

    Aiequal(3, hlp_string_diff("David", "Dave"));
    Aiequal(0, hlp_string_diff("David", "Erik"));
    Aiequal(4, hlp_string_diff("book", "bookworm"));
    Aiequal(4, hlp_string_diff("bookstop", "book"));
    Aiequal(4, hlp_string_diff("bookstop", "bookworm"));
}

tst_suite *ts_helper(tst_suite * suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_hlp_string_diff, NULL);

    return suite;
}

#include "symbol.h"
#include "test.h"

static void test_intern(TestCase *tc, void *data)
{
    Symbol word1 = intern("word"), word2 = intern("word");
    (void)data; /* suppress unused argument warning */

    Asequal("One",  intern("One"));
    Asequal("Two",  intern("Two"));
    Asequal("Three",intern("Three"));
    Asequal("Four", intern("Four"));
    Asequal("Five", intern("Five"));
    Asequal("One",  intern_and_free(estrdup("One")));
    Asequal("Two",  intern_and_free(estrdup("Two")));
    Asequal("Three",intern_and_free(estrdup("Three")));
    Asequal("Four", intern_and_free(estrdup("Four")));
    Asequal("Five", intern_and_free(estrdup("Five")));
    Asequal("word", word1);
    Asequal("word", word2);
    Apequal(word1, word2);
}

TestSuite *ts_symbol(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_intern, NULL);

    return suite;
}

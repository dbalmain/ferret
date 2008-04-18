#include "testhelper.h"
#include "multimapper.h"
#include "test.h"

static void test_multimapper(TestCase *tc, void *data)
{
    char text[] = "abc cabc abd cabcd";
    char dest[1000];
    MultiMapper *mapper = mulmap_new();
    (void)data;

    mulmap_add_mapping(mapper, "abc", "hello");

    mulmap_compile(mapper);
    Aiequal(24, mulmap_map_len(mapper, dest, text, 1000));
    Asequal("hello chello abd chellod", mulmap_map(mapper, dest, text, 1000));
    Asequal("hello chello abd chel", mulmap_map(mapper, dest, text, 22));
    Aiequal(21, mulmap_map_len(mapper, dest, text, 22));
    Asequal("hello chello a", mulmap_map(mapper, dest, text, 15));

    mulmap_add_mapping(mapper, "abcd", "hello");
    mulmap_compile(mapper);
    Asequal("hello chello abd chellod", mulmap_map(mapper, dest, text, 1000));

    mulmap_add_mapping(mapper, "cab", "taxi");
    mulmap_compile(mapper);
    Asequal("hello taxic abd taxicd", mulmap_map(mapper, dest, text, 1000));

    mulmap_destroy(mapper);
}

static void test_multimapper_utf8(TestCase *tc, void *data)
{
    char text[] = "zàáâãäåāăz";
    char dest[1000];
    char *dest_dynamic;
    MultiMapper *mapper = mulmap_new();
    (void)data;

    mulmap_add_mapping(mapper, "à", "a");
    mulmap_add_mapping(mapper, "á", "a");
    mulmap_add_mapping(mapper, "â", "a");
    mulmap_add_mapping(mapper, "ã", "a");
    mulmap_add_mapping(mapper, "ä", "a");
    mulmap_add_mapping(mapper, "å", "a");
    mulmap_add_mapping(mapper, "ā", "a");
    mulmap_add_mapping(mapper, "ă", "a");
    mulmap_compile(mapper);
    Asequal("zaaaaaaaaz", mulmap_map(mapper, dest, text, 1000));
    dest_dynamic = mulmap_dynamic_map(mapper, text);
    Asequal("zaaaaaaaaz", dest_dynamic);
    free(dest_dynamic);
    mulmap_destroy(mapper);
}

TestSuite *ts_multimapper(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_multimapper, NULL);
    tst_run_test(suite, test_multimapper_utf8, NULL);

    return suite;
}

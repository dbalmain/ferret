#include "test.h"
#include "testhelper.h"
#include "multimapper.h"

static void test_multimapper(tst_case * tc, void *data)
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

static void test_multimapper_utf8(tst_case * tc, void *data)
{
    char text[] = "zàáâãäåāăz";
    char dest[1000];
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
    mulmap_destroy(mapper);
}

tst_suite *ts_multimapper(tst_suite * suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_multimapper, NULL);
    tst_run_test(suite, test_multimapper_utf8, NULL);

    return suite;
}

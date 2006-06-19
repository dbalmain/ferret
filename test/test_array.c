#include "array.h"
#include "test.h"
#include <string.h>

static void ary_free_mock(void *p)
{
    char *str = (char *)p;
    strcpy(str, "free");
}

static void test_ary(tst_case *tc, void *data)
{
    int i;
    char *tmp;
    void **ary = ary_new();
    (void)data;

    Aiequal(0, ary_sz(ary));
    Aiequal(ARY_INIT_CAPA, ary_capa(ary));
    ary_free(ary);

    ary = ary_new_capa(10);
    Aiequal(0, ary_sz(ary));
    Aiequal(10, ary_capa(ary));
    ary_set(ary, 1, "one");
    Aiequal(2, ary_sz(ary));
    Asequal("one", ary[1]);
    Apnull(ary[0]);
    Apnull(ary_get(ary, 0));
    Apnull(ary[2]);
    Apnull(ary_get(ary, 2));

    /* cannot use the simple reference outside of the allocated range */
    Asequal("one", ary_get(ary, -1));
    Apnull(ary_get(ary, 22));
    Apnull(ary_get(ary, -22));

    ary_set(ary, 2, "two");
    Aiequal(3, ary_sz(ary));
    Asequal("one", ary[1]);
    Asequal("two", ary[2]);
    Apnull(ary[0]);
    Apnull(ary[3]);
    Asequal("one", ary_get(ary, -2));
    Asequal("two", ary_get(ary, -1));
    ary_set(ary, -1, "two");
    ary_set(ary, -3, "zero");

    Asequal("zero", ary[0]);
    Asequal("one", ary[1]);
    Asequal("two", ary[2]);
    Aiequal(3, ary_sz(ary));

    ary_set(ary, 19, "nineteen");
    Aiequal(20, ary_sz(ary));
    for (i = 4; i < 19; i++) {
        Apnull(ary[i]);
    }

    ary_push(ary, "twenty");
    Aiequal(21, ary_sz(ary));
    Asequal("twenty", ary_pop(ary));
    Aiequal(20, ary_sz(ary));

    Asequal("nineteen", ary_pop(ary));
    Aiequal(19, ary_sz(ary));

    Apnull(ary_pop(ary));
    Aiequal(18, ary_sz(ary));

    ary_push(ary, "eighteen");
    Aiequal(19, ary_sz(ary));
    Asequal("eighteen", ary[18]);
    Asequal("eighteen", ary_get(ary, -1));
    Asequal("zero", ary_get(ary, -19));
    Asequal("one", ary_get(ary, -18));
    Asequal("two", ary_get(ary, -17));
    Apnull(ary_get(ary, -16));
    Apnull(ary_get(ary, -20));

    Asequal("zero", ary_shift(ary));
    Aiequal(18, ary_sz(ary));
    Asequal("eighteen", ary[17]);
    Apnull(ary[18]);
    Asequal("one", ary_get(ary, -18));
    Asequal("two", ary_get(ary, -17));
    Apnull(ary_get(ary, -16));
    Apnull(ary_get(ary, -19));

    Asequal("one", ary_shift(ary));
    Aiequal(17, ary_sz(ary));
    Asequal("eighteen", ary[16]);
    Apnull(ary[18]);
    Apnull(ary[17]);
    Asequal("two", ary_get(ary, -17));
    Apnull(ary_get(ary, -16));
    Apnull(ary_get(ary, -18));

    ary[5] = "five";
    ary[6] = "six";
    ary[7] = "seven";
    
    Asequal("five", ary_get(ary, 5));
    Asequal("six", ary_get(ary, 6));
    Asequal("seven", ary_get(ary, 7));

    ary_remove(ary, 6);
    Aiequal(16, ary_sz(ary));

    Asequal("five", ary_get(ary, 5));
    Asequal("seven", ary_get(ary, 6));
    Apnull(ary_get(ary, 4));
    Apnull(ary_get(ary, 7));
    Asequal("eighteen", ary[15]);
    Asequal("two", ary_get(ary, -16));
    Apnull(ary_get(ary, -15));
    Apnull(ary_get(ary, -17));
    Asequal("five", ary_get(ary, 5));
    Asequal("seven", ary_get(ary, 6));

    tmp = estrdup("sixsix");
    ary[6] = tmp;
    ary[7] = "seven";
    Asequal("sixsix", ary_get(ary, 6));
    Asequal("seven", ary_get(ary, 7));

    ary_delete(ary, 6, &ary_free_mock);
    Aiequal(15, ary_sz(ary));
    Asequal("free", tmp);
    free(tmp);

    Asequal("five", ary_get(ary, 5));
    Asequal("seven", ary_get(ary, 6));
    Apnull(ary_get(ary, 4));
    Apnull(ary_get(ary, 7));

    ary_free(ary);
}

void test_ary_destroy(tst_case *tc, void *data)
{
    void **ary = ary_new();
    char str1[10] = "alloc1";
    char str2[10] = "alloc2";
    (void)data;

    ary_set(ary, 0, str1);
    Aiequal(1, ary_sz(ary));
    Asequal("alloc1", ary[0]);
    ary_set(ary, 0, str2);
    Asequal("alloc2", ary[0]);
    ary_push(ary, str1);
    Aiequal(2, ary_sz(ary));
    Asequal("alloc1", ary[1]);
    ary_delete(ary, 0, &ary_free_mock);
    Aiequal(1, ary_sz(ary));
    Asequal("free", str2);
    ary_destroy(ary, &ary_free_mock);
    Asequal("free", str1);
}

#define ARY_STRESS_SIZE 1000
void stress_ary(tst_case *tc, void *data)
{
    int i;
    char buf[100], *t;
    void **ary = ary_new();
    (void)data;

    for (i = 0; i < ARY_STRESS_SIZE; i++) {
        sprintf(buf, "<%d>", i);
        ary_push(ary, estrdup(buf));
    }

    for (i = 0; i < ARY_STRESS_SIZE; i++) {
        sprintf(buf, "<%d>", i);
        t = ary_shift(ary);
        Asequal(buf, t);
        free(t);
    }

    Aiequal(0, ary_sz(ary));

    for (i = 0; i < ARY_STRESS_SIZE; i++) {
        Apnull(ary[i]);
    }
    ary_destroy(ary, &free);
}

tst_suite *ts_array(tst_suite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_ary, NULL);
    tst_run_test(suite, test_ary_destroy, NULL);
    tst_run_test(suite, stress_ary, NULL);

    return suite;
}

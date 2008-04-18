#include "array.h"
#include <string.h>
#include "test.h"

static void ary_free_mock(void *p)
{
    char *str = (char *)p;
    strcpy(str, "free");
}

static void test_ary(TestCase *tc, void *data)
{
    int i;
    int raised = 0;
    char *tmp;
    void **ary = ary_new();
    (void)data;

    Aiequal(0, ary_sz(ary));
    Aiequal(ARY_INIT_CAPA, ary_capa(ary));
    ary_free(ary);

    ary = ary_new();
    ary_push(ary, (char *)"one");
    Aiequal(1, ary_sz(ary));
    ary_unshift(ary, (char *)"zero");
    Aiequal(2, ary_sz(ary));
    Asequal("zero", ary[0]);
    Asequal("one", ary[1]);
    Apnull(ary_remove(ary, 2));

    TRY
        ary_set(ary, -3, (char *)"minusone");
    XCATCHALL
        HANDLED();
        raised = 1;
    XENDTRY
    Aiequal(1, raised);
    ary_free(ary);

    ary = ary_new_capa(10);
    Aiequal(0, ary_sz(ary));
    Aiequal(10, ary_capa(ary));
    ary_set(ary, 1, (char *)"one");
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

    ary_set(ary, 2, (char *)"two");
    Aiequal(3, ary_sz(ary));
    Asequal("one", ary[1]);
    Asequal("two", ary[2]);
    Apnull(ary[0]);
    Apnull(ary[3]);
    Asequal("one", ary_get(ary, -2));
    Asequal("two", ary_get(ary, -1));
    ary_set(ary, -1, (char *)"two");
    ary_set(ary, -3, (char *)"zero");

    Asequal("zero", ary[0]);
    Asequal("one", ary[1]);
    Asequal("two", ary[2]);
    Aiequal(3, ary_sz(ary));

    ary_set(ary, 19, (char *)"nineteen");
    Aiequal(20, ary_sz(ary));
    for (i = 4; i < 19; i++) {
        Apnull(ary[i]);
    }

    ary_push(ary, (char *)"twenty");
    Aiequal(21, ary_sz(ary));
    Asequal("twenty", ary_pop(ary));
    Aiequal(20, ary_sz(ary));

    Asequal("nineteen", ary_pop(ary));
    Aiequal(19, ary_sz(ary));

    Apnull(ary_pop(ary));
    Aiequal(18, ary_sz(ary));

    ary_push(ary, (char *)"eighteen");
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

    ary[5] = (char *)"five";
    ary[6] = (char *)"six";
    ary[7] = (char *)"seven";

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
    ary[7] = (char *)"seven";
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

void test_ary_destroy(TestCase *tc, void *data)
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
void stress_ary(TestCase *tc, void *data)
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
        t = (char *)ary_shift(ary);
        Asequal(buf, t);
        free(t);
    }

    Aiequal(0, ary_sz(ary));

    for (i = 0; i < ARY_STRESS_SIZE; i++) {
        Apnull(ary[i]);
    }
    ary_destroy(ary, &free);
}

struct TestPoint {
    int x;
    int y;
};

#define tp_ary_set(ary, i, x_val, y_val) do {\
    ary_resize(ary, i);\
    ary[i].x = x_val;\
    ary[i].y = y_val;\
} while (0)

void test_typed_ary(TestCase *tc, void *data)
{
    struct TestPoint *points = ary_new_type_capa(struct TestPoint, 5);
    (void)data;

    Aiequal(5, ary_capa(points));
    Aiequal(0, ary_sz(points));
    Aiequal(sizeof(struct TestPoint), ary_type_size(points));

    tp_ary_set(points, 0, 1, 2);
    Aiequal(5, ary_capa(points));
    Aiequal(1, ary_sz(points));
    Aiequal(sizeof(struct TestPoint), ary_type_size(points));
    Aiequal(1, points[0].x);
    Aiequal(2, points[0].y);

    tp_ary_set(points, 5, 15, 20);
    Aiequal(6, ary_size(points));
    Aiequal(15, points[5].x);
    Aiequal(20, points[5].y);

    tp_ary_set(points, 1, 1, 1);
    tp_ary_set(points, 2, 2, 2);
    tp_ary_set(points, 3, 3, 3);
    tp_ary_set(points, 4, 4, 4);

    Aiequal(6, ary_size(points));
    Aiequal(1, points[0].x);
    Aiequal(2, points[0].y);
    Aiequal(1, points[1].x);
    Aiequal(1, points[1].y);
    Aiequal(2, points[2].x);
    Aiequal(2, points[2].y);
    Aiequal(3, points[3].x);
    Aiequal(3, points[3].y);
    Aiequal(4, points[4].x);
    Aiequal(4, points[4].y);
    Aiequal(15, points[5].x);
    Aiequal(20, points[5].y);
    ary_free(points);
}

TestSuite *ts_array(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_ary, NULL);
    tst_run_test(suite, test_ary_destroy, NULL);
    tst_run_test(suite, stress_ary, NULL);
    tst_run_test(suite, test_typed_ary, NULL);

    return suite;
}

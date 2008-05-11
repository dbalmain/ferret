#include "document.h"
#include "test.h"

void test_df_standard(TestCase *tc, void *data)
{
    char *s;
    DocField *df;
    (void)data;

    df = df_add_data(df_new(I("title")), estrdup("Life of Pi"));
    df->destroy_data = true;
    Aiequal(1, df->size);
    Asequal("title", df->name);
    Asequal("Life of Pi", df->data[0]);
    Aiequal(strlen("Life of Pi"), df->lengths[0]);
    Asequal("title: \"Life of Pi\"", s = df_to_s(df));
    Afequal(1.0, df->boost);
    free(s);
    df_destroy(df);

    df = df_add_data_len(df_new(I("title")), "new title", 9);
    Aiequal(1, df->size);
    Asequal("title", df->name);
    Asequal("new title", df->data[0]);
    Aiequal(9, df->lengths[0]);
    df_destroy(df);
}

void test_df_multi_fields(TestCase *tc, void *data)
{
    int i;
    char *s;
    DocField *df;
    (void)data;

    df = df_add_data(df_new(I("title")), estrdup("Vernon God Little"));
    df->destroy_data = true;
    Aiequal(1, df->size);
    Asequal("title", df->name);
    Asequal("Vernon God Little", df->data[0]);
    Aiequal(strlen("Vernon God Little"), df->lengths[0]);
    
    df_add_data(df, estrdup("some more data"));
    Aiequal(2, df->size);
    Asequal("title: [\"Vernon God Little\", \"some more data\"]",
            s = df_to_s(df));
    free(s);
    df_add_data_len(df, estrdup("and more data"), 14);
    Aiequal(3, df->size);
    Asequal("title", df->name);
    Asequal("Vernon God Little", df->data[0]);
    Asequal("some more data", df->data[1]);
    Asequal("and more data", df->data[2]);

    df_destroy(df);

    df = df_add_data(df_new(I("data")), estrdup("start"));
    df->destroy_data = true;
    Aiequal(1, df->size);
    for (i = 0; i < 1000; i++) {
        char buf[100];
        sprintf(buf, "<<%d>>", i);
        df_add_data(df, estrdup(buf));
        Aiequal(i + 2, df->size);
    }
    df_destroy(df);
}

void test_doc(TestCase *tc, void *data)
{
    int i;
    Document *doc;
    DocField *df;
    (void)data;

    doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(I("title")), "title"));
    Aiequal(1, doc->size);
    df = df_add_data(df_new(I("data")), "data1");
    df_add_data(df, "data2");
    df_add_data(df, "data3");
    df_add_data(df, "data4");
    doc_add_field(doc, df);
    Aiequal(2, doc->size);
    Asequal("title", doc_get_field(doc, I("title"))->name);
    Aiequal(1, doc_get_field(doc, I("title"))->size);
    Asequal("title", doc_get_field(doc, I("title"))->data[0]);
    Asequal("data", doc_get_field(doc, I("data"))->name);
    Aiequal(4, doc_get_field(doc, I("data"))->size);
    Asequal("data1", doc_get_field(doc, I("data"))->data[0]);
    Asequal("data2", doc_get_field(doc, I("data"))->data[1]);
    Asequal("data3", doc_get_field(doc, I("data"))->data[2]);
    Asequal("data4", doc_get_field(doc, I("data"))->data[3]);
    Afequal(1.0, doc->boost);
    doc_destroy(doc);

    doc = doc_new();
    for (i = 0; i < 1000; i++) {
        char buf[100];
        sprintf(buf, "<<%d>>", i);
        df = df_add_data(df_new(I(buf)), estrdup(buf));
        df->destroy_data = true;
        doc_add_field(doc, df);
        Aiequal(i + 1, doc->size);
    }

    for (i = 0; i < 1000; i++) {
        char buf[100];
        sprintf(buf, "<<%d>>", i);
        Aiequal(1, doc_get_field(doc, I(buf))->size);
        Aiequal(strlen(buf), doc_get_field(doc, I(buf))->lengths[0]);
        Asequal(buf, doc_get_field(doc, I(buf))->data[0]);
    }
    doc_destroy(doc);
} 

void test_double_field_exception(TestCase *tc, void *data)
{
    volatile bool exception_thrown = false;
    Document *doc;
    DocField *volatile df = NULL;
    (void)data;

    doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(I("title")), "title"));

    TRY
        df = df_add_data_len(df_new(I("title")), "title", 5);
        doc_add_field(doc, df);
    case EXCEPTION:
        exception_thrown = true;
        HANDLED();
        break;
    case FINALLY:
        df_destroy(df);
        break;
    ENDTRY

    Atrue(exception_thrown);

    doc_destroy(doc);
}

TestSuite *ts_document(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_df_standard, NULL);
    tst_run_test(suite, test_df_multi_fields, NULL);
    tst_run_test(suite, test_doc, NULL);
    tst_run_test(suite, test_double_field_exception, NULL);

    return suite;
}

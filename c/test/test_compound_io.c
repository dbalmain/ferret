#include "store.h"
#include "index.h"
#include "testhelper.h"
#include "test.h"

void test_compound_reader(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    char *p;
    OutStream *os = store->new_output(store, "cfile");
    InStream *is1;
    InStream *is2;
    Store *c_reader;
    os_write_vint(os, 2);
    os_write_u64(os, 29);
    os_write_string(os, "file1");
    os_write_u64(os, 33);
    os_write_string(os, "file2");
    os_write_u32(os, 20);
    os_write_string(os, "this is file 2");
    os_close(os);

    c_reader = open_cmpd_store(store, "cfile");
    Aiequal(4, c_reader->length(c_reader, "file1"));
    Aiequal(15, c_reader->length(c_reader, "file2"));
    is1 = c_reader->open_input(c_reader, "file1");
    is2 = c_reader->open_input(c_reader, "file2");
    Aiequal(20, is_read_u32(is1));
    Asequal("this is file 2", p=is_read_string(is2)); free(p);
    is_close(is1);
    is_close(is2);
    store_deref(c_reader);
}

void test_compound_writer(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    char *p;
    OutStream *os1 = store->new_output(store, "file1");
    OutStream *os2 = store->new_output(store, "file2");
    CompoundWriter *cw;
    InStream *is;

    os_write_u32(os1, 20);
    os_write_string(os2,"this is file2");
    os_close(os1);
    os_close(os2);
    cw = open_cw(store, "cfile");
    cw_add_file(cw, "file1");
    cw_add_file(cw, "file2");
    cw_close(cw);

    is = store->open_input(store, "cfile");
    Aiequal(2, is_read_vint(is));
    Aiequal(29, is_read_u64(is));
    Asequal("file1", p=is_read_string(is)); free(p);
    Aiequal(33, is_read_u64(is));
    Asequal("file2", p=is_read_string(is)); free(p);
    Aiequal(20, is_read_u32(is));
    Asequal("this is file2", p=is_read_string(is)); free(p);

    is_close(is);
}

void test_compound_io(TestCase *tc, void *data)
{
    Store *c_reader;
    InStream *is1, *is2, *is3;
    Store *store = (Store *)data;
    CompoundWriter *cw;
    char *p;
    OutStream *os1 = store->new_output(store, "file1");
    OutStream *os2 = store->new_output(store, "file2");
    OutStream *os3 = store->new_output(store, "file3");
    char long_string[10000];
    char *short_string = "this is a short string";
    int slen = (int)strlen(short_string);
    int i;

    for (i = 0; i < 20; i++) {
        os_write_u32(os1, rand()%10000);
    }

    for (i = 0; i < 10000 - slen; i += slen) {
        sprintf(long_string + i, "%s", short_string);
    }
    long_string[i] = 0;
    os_write_string(os2, long_string);
    os_write_string(os3, short_string);
    os_close(os1);
    os_close(os2);
    os_close(os3);
    cw = open_cw(store, "cfile");
    cw_add_file(cw, "file1");
    cw_add_file(cw, "file2");
    cw_add_file(cw, "file3");
    cw_close(cw);

    c_reader = open_cmpd_store(store, "cfile");
    is1 = c_reader->open_input(c_reader, "file1");
    for (i = 0; i < 20; i++) {
        Assert(is_read_u32(is1) < 10000, "should be a rand between 0 and 10000");
    }
    is_close(is1);
    is2 = c_reader->open_input(c_reader, "file2");
    Asequal(long_string, p=is_read_string(is2)); free(p);
    is_close(is2);
    is3 = c_reader->open_input(c_reader, "file3");
    Asequal(short_string, p=is_read_string(is3)); free(p);
    is_close(is3);

    store_deref(c_reader);
}

#define MAX_TEST_WORDS 50
#define TEST_FILE_CNT 100

void test_compound_io_many_files(TestCase *tc, void *data)
{
    static const int MAGIC = 250777;

    Store *store = (Store *)data;
    char buf[MAX_TEST_WORDS * (TEST_WORD_LIST_MAX_LEN + 1)];
    char *str;
    int i;
    OutStream *os;
    InStream *is;
    CompoundWriter *cw;
    Store *c_reader;

    cw = open_cw(store, "_.cfs");
    for (i = 0; i < TEST_FILE_CNT; i++) {
        sprintf(buf, "_%d.txt", i);
        cw_add_file(cw, buf);
        os = store->new_output(store, buf);
        os_write_string(os, make_random_string(buf, MAX_TEST_WORDS));
        os_write_vint(os, MAGIC);
        os_close(os);
    }
    cw_close(cw);

    c_reader = open_cmpd_store(store, "_.cfs");
    for (i = 0; i < TEST_FILE_CNT; i++) {
        sprintf(buf, "_%d.txt", i);
        is = c_reader->open_input(c_reader, buf);
        str = is_read_string(is);

        free(str);
        Aiequal(MAGIC, is_read_vint(is));
        Aiequal(is_length(is), is_pos(is));
        is_close(is);
    }
    store_deref(c_reader);
}

TestSuite *ts_compound_io(TestSuite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_compound_reader, store);
    tst_run_test(suite, test_compound_writer, store);
    tst_run_test(suite, test_compound_io, store);
    tst_run_test(suite, test_compound_io_many_files, store);

    store_deref(store);

    return suite;
}

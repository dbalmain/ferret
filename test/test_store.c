#include "test.h"
#include "store.h"
#include <string.h>

#define TEST_LOCK_NAME "test"

/**
 * Test that the lock is created and deleted correctly
 */
static void test_lock(tst_case *tc, void *data)
{
    Store *store = (Store *) data;
    Lock *lock1, *lock2;

    lock1 = store->open_lock(store, TEST_LOCK_NAME);
    Aiequal(false, lock1->is_locked(lock1));
    Aiequal(true, lock1->obtain(lock1));
    Aiequal(true, lock1->is_locked(lock1));
    lock1->release(lock1);
    Aiequal(false, lock1->is_locked(lock1));
    Aiequal(true, lock1->obtain(lock1));
    lock2 = store->open_lock(store, TEST_LOCK_NAME);
    Aiequal(true, lock2->is_locked(lock1));
    lock1->release(lock1);
    Aiequal(false, lock2->is_locked(lock1));
    store->close_lock(lock1);
    store->close_lock(lock2);
}

/**
 * Do basic file operations test.
 */
static void test_basic_file_ops(tst_case *tc, void *data)
{
    Store *store = (Store *) data;

    store->clear_all(store);    /* Make sure the test directory is empty. */
    Assert(!store->exists(store, "file1"),
           "File1 should not been created yet");
    store->touch(store, "file1");
    Aiequal(1, store->count(store));
    Assert(store->count(store) == 1, "The store now contains one file");
    Assert(store->exists(store, "file1"), "File1 should now been created");
    store->touch(store, "file2");
    Assert(store->count(store) == 2, "The store now contains two files");
    Assert(store->exists(store, "file2"), "File2 should now been created");
    store->remove(store, "file1");
    Assert(store->count(store) == 1, "The store now contains one file");
    Assert(!store->exists(store, "file1"), "File1 should have been removed");
    Assert(store->exists(store, "file2"), "File2 should still exist");
}

/**
 * Test argument used to test the store->each function
 */
struct EachArg
{
    char str[100];
    char *p;
};

/**
 * Test function used to test store->each function
 */
static void concat_filenames(char *fname, void *arg)
{
    struct EachArg *ea = (struct EachArg *) arg;
    strcpy(ea->p, fname);
    ea->p += strlen(fname);
}

/**
 * Test the store->each function
 */
static void test_each(tst_case *tc, void *data)
{
    Store *store = (Store *) data;

    struct EachArg ea;
    ea.p = ea.str;

    store->clear_all(store);    /* Make sure the test directory is empty. */
    store->touch(store, "file1");
    store->touch(store, "file2");
    store->touch(store, "file3");
    store->touch(store, "file4");
    store->each(store, &concat_filenames, &ea);
    *(ea.p) = 0;
    Assert(strstr(ea.str, "file1") != NULL, "should contain this file");
    Assert(strstr(ea.str, "file2") != NULL, "should contain this file");
    Assert(strstr(ea.str, "file3") != NULL, "should contain this file");
    Assert(strstr(ea.str, "file4") != NULL, "should contain this file");
    Aiequal(20, strlen(ea.str));
}

/**
 * Test the store->rename function
 */
static void test_rename(tst_case *tc, void *data)
{
    int cnt_before, cnt_after;
    Store *store = (Store *) data;
    store->touch(store, "from");
    Assert(store->exists(store, "from"), "File should exist");
    Assert(!store->exists(store, "to"), "File should not exist");
    cnt_before = store->count(store);
    store->rename(store, "from", "to");
    cnt_after = store->count(store);
    Aiequal(cnt_before, cnt_after);
    Assert(store->exists(store, "to"), "File should now exist");
    Assert(!store->exists(store, "from"), "File should no longer exist");
}

/**
 * Test the reading and writing of bytes (8 bit)
 */
static void test_rw_bytes(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    uchar bytes[6] = { 0x34, 0x87, 0xF9, 0xEA, 0x00, 0xFF };
    OutStream *ostream = store->create_output(store, "rw_byte.test");
    InStream *istream;
    Assert(store->exists(store, "rw_byte.test"), "File should now exist");
    for (i = 0; i < 6; i++) {
        os_write_byte(ostream, bytes[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_byte.test");
    for (i = 0; i < 6; i++) {
        Aiequal(bytes[i], is_read_byte(istream));
    }
    is_close(istream);
    Aiequal(6, store->length(store, "rw_byte.test"));
    Assert(store->exists(store, "rw_byte.test"), "File should now exist");
}

/**
 * Test the reading and writing of 32-bit integers
 */
static void test_rw_ints(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    OutStream *ostream = store->create_output(store, "rw_int.test");
    InStream *istream;
    f_i32 ints[4] = { POSH_I32_MAX, POSH_I32_MIN, -1, 0 };

    for (i = 0; i < 4; i++) {
        os_write_int(ostream, (int) ints[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_int.test");
    for (i = 0; i < 4; i++) {
        Aiequal(ints[i], is_read_int(istream));
    }

    is_close(istream);
    Aiequal(16, store->length(store, "rw_int.test"));
}

/**
 * Test the reading and writing of 64-bit integers
 */
static void test_rw_longs(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    f_u64 longs[4] =
        { POSH_I64_MIN, POSH_I64_MAX, POSH_I64(-1), POSH_I64(0) };
    OutStream *ostream = store->create_output(store, "rw_long.test");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_long(ostream, longs[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_long.test");
    for (i = 0; i < 4; i++) {
        Aiequal(longs[i], is_read_long(istream));
    }
    is_close(istream);
    Aiequal(32, store->length(store, "rw_long.test"));
}

/**
 * Test the reading and writing of 32-bit unsigned integers
 */
static void test_rw_uints(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    f_u32 uints[4] = { POSH_U32_MAX, POSH_U32_MIN, 100000, 1 };
    OutStream *ostream = store->create_output(store, "rw_uint.test");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_uint(ostream, uints[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_uint.test");
    for (i = 0; i < 4; i++) {
        Aiequal(uints[i], is_read_uint(istream));
    }
    is_close(istream);
    Aiequal(16, store->length(store, "rw_uint.test"));
}

/**
 * Test the reading and writing of 64-bit unsigned integers
 */
static void test_rw_ulongs(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    unsigned long long ulongs[4] =
        { POSH_U64_MAX, POSH_U64_MIN, POSH_U64(100000000000000), POSH_U64(1) };
    OutStream *ostream = store->create_output(store, "rw_ulong.test");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_ulong(ostream, ulongs[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_ulong.test");
    for (i = 0; i < 4; i++) {
        Aiequal(ulongs[i], is_read_ulong(istream));
    }
    is_close(istream);
    Aiequal(32, store->length(store, "rw_ulong.test"));
}

/**
 * Test reading and writing of variable size 32-bit integers
 */
static void test_rw_vints(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    f_u32 vints[4] = { POSH_U32_MAX, POSH_U32_MIN, 100000, 1 };
    OutStream *ostream = store->create_output(store, "rw_vint.test");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_vint(ostream, vints[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_vint.test");
    for (i = 0; i < 4; i++) {
        Aiequal(vints[i], is_read_vint(istream));
    }
    is_close(istream);
    Aiequal(10, store->length(store, "rw_vint.test"));
}

/**
 * Test reading and writing of variable size 64-bit integers
 */
static void test_rw_vlongs(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    f_u64 vlongs[4] =
        { POSH_U64_MAX, POSH_U64_MIN, POSH_U64(100000000000000), POSH_U64(1) };
    OutStream *ostream = store->create_output(store, "rw_vlong.test");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_vlong(ostream, vlongs[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "rw_vlong.test");
    for (i = 0; i < 4; i++) {
        Aiequal(vlongs[i], is_read_vlong(istream));
    }
    is_close(istream);
    Aiequal(19, store->length(store, "rw_vlong.test"));
}

/**
 * Test reading and writing of strings
 */
static void test_rw_strings(tst_case *tc, void *data)
{
    int i;
    char *tmp;
    Store *store = (Store *) data;
    char str[60] =
        "This is a c ferret test string ~!@#$%^&*()`123456790-=\\)_+|";
    char buf[60000] = "";
    OutStream *ostream;
    InStream *istream;

    for (i = 0; i < 1000; i++) {
        strcat(buf, str);
    }

    ostream = store->create_output(store, "rw_string.test");
    os_write_string(ostream, str);
    os_write_string(ostream, buf);
    os_close(ostream);

    istream = store->open_input(store, "rw_string.test");

    tmp = is_read_string(istream);
    Asequal(str, tmp);
    free(tmp);
    tmp = is_read_string(istream);
    Asequal(buf, tmp);
    free(tmp);
    is_close(istream);
    Aiequal(59063, store->length(store, "rw_string.test"));
}

/**
 * Test reading and writing of non-ascii characters
 */
static void test_rw_funny_strings(tst_case *tc, void *data)
{
    int i;
    char *tmp;
    Store *store = (Store *) data;
    char str[12] = "³³ ëêðïéÄ";
    char buf[12000] = "";
    OutStream *ostream;
    InStream *istream;

    for (i = 0; i < 1000; i++) {
        strcat(buf, str);
    }

    ostream = store->create_output(store, "rw_funny_string.test");
    os_write_string(ostream, str);
    os_write_string(ostream, buf);
    os_close(ostream);

    istream = store->open_input(store, "rw_funny_string.test");
    tmp = is_read_string(istream);
    Asequal(str, tmp);
    free(tmp);
    tmp = is_read_string(istream);
    Asequal(buf, tmp);
    free(tmp);
    is_close(istream);
    Aiequal(9012, store->length(store, "rw_funny_string.test"));
}

/**
 * Test seek in both input stream and output stream, ie test the os_seek and
 * is_seek functions
 */
static void test_buffer_seek(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;

    OutStream *ostream = store->create_output(store, "rw_seek.test");
    char text[60] = "This is another int test string !@#$%#$%&%$*%^&*()(_";
    InStream *istream;

    for (i = 0; i < 1000; i++) {
        os_write_long(ostream, i);
        os_write_string(ostream, text);
    }
    os_seek(ostream, 987);
    Aiequal(987, os_pos(ostream));
    os_write_vint(ostream, 555);
    os_seek(ostream, 56);
    Aiequal(56, os_pos(ostream));
    os_write_vint(ostream, 1234567890);
    os_seek(ostream, 4000);
    Aiequal(4000, os_pos(ostream));
    os_write_vlong(ostream, 9876543210LL);
    os_close(ostream);

    istream = store->open_input(store, "rw_seek.test");
    is_seek(istream, 56);
    Aiequal(56, is_pos(istream));
    Aiequal(1234567890, is_read_vint(istream));
    is_seek(istream, 4000);
    Aiequal(4000, is_pos(istream));
    Aiequal(9876543210LL, is_read_vlong(istream));
    is_seek(istream, 987);
    Aiequal(987, is_pos(istream));
    Aiequal(555, is_read_vint(istream));
    is_close(istream);
}

/**
 * Test cloning of InputStream
 */
static void test_is_clone(tst_case *tc, void *data)
{
    int i;
    Store *store = (Store *) data;
    OutStream *ostream = store->create_output(store, "clone.test");
    InStream *istream, *alt_istream;

    for (i = 0; i < 10; i++) {
        os_write_long(ostream, i);
    }
    os_close(ostream);
    istream = store->open_input(store, "clone.test");
    is_seek(istream, 24);
    alt_istream = is_clone(istream);
    Aiequal(is_pos(istream), is_pos(alt_istream));
    for (i = 3; i < 10; i++) {
        Aiequal(i, is_read_long(alt_istream));
    }
    Aiequal(80, is_pos(alt_istream));
    Aiequal(24, is_pos(istream));
    for (i = 3; i < 10; i++) {
        Aiequal(i, is_read_long(istream));
    }
    is_close(alt_istream);
    is_close(istream);
}

/**
 * Test the read_bytes method. This method reads a number of bytes into a
 * buffer.
 */
static void test_read_bytes(tst_case *tc, void *data)
{
    char str[11] = "0000000000";
    Store *store = (Store *) data;
    OutStream *ostream = store->create_output(store, "read_bytes.test");
    InStream *istream;

    os_write_bytes(ostream, (uchar *) "how are you doing?", 18);
    os_close(ostream);
    istream = store->open_input(store, "read_bytes.test");
    is_read_bytes(istream, (uchar *) str, 2, 4);
    Asequal("00how 0000", str);
    is_read_bytes(istream, (uchar *) str, 1, 8);
    Asequal("0are you 0", str);
    is_close(istream);
}

/**
 * Create a test suite for a store. This function can be used to create a test
 * suite for both a FileSystem store and a RAM store and any other type of
 * store that you might want to create.
 */
void create_test_store_suite(tst_suite *suite, Store *store)
{
    store->clear_all(store);

    tst_run_test(suite, test_basic_file_ops, store);
    tst_run_test(suite, test_rename, store);
    tst_run_test(suite, test_each, store);
    tst_run_test(suite, test_rw_bytes, store);
    tst_run_test(suite, test_rw_ints, store);
    tst_run_test(suite, test_rw_longs, store);
    tst_run_test(suite, test_rw_uints, store);
    tst_run_test(suite, test_rw_ulongs, store);
    tst_run_test(suite, test_rw_vints, store);
    tst_run_test(suite, test_rw_vlongs, store);
    tst_run_test(suite, test_rw_strings, store);
    tst_run_test(suite, test_rw_funny_strings, store);
    tst_run_test(suite, test_buffer_seek, store);
    tst_run_test(suite, test_is_clone, store);
    tst_run_test(suite, test_read_bytes, store);
    tst_run_test(suite, test_lock, store);

    store->clear_all(store);
}

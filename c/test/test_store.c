#include "store.h"
#include <string.h>
#include <limits.h>
#include "test.h"

#define TEST_LOCK_NAME "test"

typedef struct WithLockTestArg {
    Lock *lock;
    TestCase *tc;
} WithLockTestArg;

typedef struct WithLockNameTestArg {
    char *lock_name;
    Store *store;
    TestCase *tc;
} WithLockNameTestArg;

static void with_lock_test(void *p)
{
    WithLockTestArg *l = (WithLockTestArg *)p;
    TestCase *tc = l->tc;
    Assert(l->lock->is_locked(l->lock), "lock should be locked");
}

static void with_lock_name_test(void *p)
{
    WithLockNameTestArg *l = (WithLockNameTestArg *)p;
    TestCase *tc = l->tc;
    Lock *lock = open_lock(l->store, l->lock_name);
    Assert(lock->is_locked(lock), "lock should be locked");
    close_lock(lock);
}

/**
 * Test that the lock is created and deleted correctly
 */
static void test_lock(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Lock *lock, *lock1, *lock2;
    bool handled = false;
    WithLockTestArg wlta;
    WithLockNameTestArg wlnta;

    lock1 = open_lock(store, TEST_LOCK_NAME);
    Aiequal(false, lock1->is_locked(lock1));
    Aiequal(true, lock1->obtain(lock1));
    Aiequal(true, lock1->is_locked(lock1));
    lock1->release(lock1);
    Aiequal(false, lock1->is_locked(lock1));
    Aiequal(true, lock1->obtain(lock1));
    lock2 = open_lock(store, TEST_LOCK_NAME);
    Aiequal(true, lock2->is_locked(lock2));
    lock1->release(lock1);
    Aiequal(false, lock2->is_locked(lock2));
    close_lock(lock1);
    close_lock(lock2);

    /* test with_lock */
    lock = open_lock(store, TEST_LOCK_NAME);
    Assert(!lock->is_locked(lock), "lock shouldn't be locked yet");
    wlta.lock = lock; wlta.tc = tc;
    with_lock(lock, &with_lock_test, &wlta);
    Assert(!lock->is_locked(lock), "lock should be unlocked again");
    Assert(lock->obtain(lock), "lock should be obtainable");
    handled = false;
    TRY
        with_lock(lock, &with_lock_test, &wlta);
        Assert(false, "A locking exception should have been raised");
        break;
    case LOCK_ERROR:
        handled = true;
        HANDLED();
        break;
    default:
        Assert(false, "This exception shouldn't have been raised");
        break;
    case FINALLY:
        break;
    XENDTRY
    Assert(handled, "A LOCK_ERROR should have been raised");
    lock->release(lock);
    close_lock(lock);

    /* test with_lock_name */
    lock = open_lock(store, TEST_LOCK_NAME);
    Assert(!lock->is_locked(lock), "lock shouldn't be locked yet");
    wlnta.lock_name = TEST_LOCK_NAME; wlnta.tc = tc; wlnta.store = store;
    with_lock_name(store, TEST_LOCK_NAME, &with_lock_name_test, &wlnta);
    Assert(!lock->is_locked(lock), "lock should be unlocked again");
    Assert(lock->obtain(lock), "lock should be obtainable");
    handled = false;
    TRY
        with_lock_name(store, TEST_LOCK_NAME, &with_lock_name_test, &wlnta);
        Assert(false, "A locking exception should have been raised");
        break;
    case LOCK_ERROR:
        handled = true;
        HANDLED();
        break;
    default:
        Assert(false, "This exception shouldn't have been raised");
        break;
    case FINALLY:
        break;
    XENDTRY
    Assert(handled, "A LOCK_ERROR should have been raised");
    lock->release(lock);
    close_lock(lock);
}

/**
 * Do basic file operations test.
 */
static void test_basic_file_ops(TestCase *tc, void *data)
{
    Store *store = (Store *)data;

    store->clear_all(store);    /* Make sure the test directory is empty. */
    Assert(!store->exists(store, "_1.f1"),
           "File1 should not been created yet");
    store->touch(store, "_1.f1");
    Aiequal(1, store->count(store));
    Assert(store->count(store) == 1, "The store now contains one file");
    Assert(store->exists(store, "_1.f1"), "File1 should now been created");
    store->touch(store, "_1.f2");
    Assert(store->count(store) == 2, "The store now contains two files");
    Assert(store->exists(store, "_1.f2"), "File2 should now been created");
    store->remove(store, "_1.f1");
    Assert(store->count(store) == 1, "The store now contains one file");
    Assert(!store->exists(store, "_1.f1"), "File1 should have been removed");
    Assert(store->exists(store, "_1.f2"), "File2 should still exist");

    /* test that lock files get deleted by clear_all */
    store->touch(store, "ferret-write.lck");
    Assert(store->exists(store, "ferret-write.lck"),"lock should still exist");
    store->clear_all(store);
    Assert(!store->exists(store, "ferret-write.lck"), "lock should be deleted");
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
static void concat_filenames(const char *fname, struct EachArg *ea)
{
    strcpy(ea->p, fname);
    ea->p += strlen(fname);
}

/**
 * Test the store->each function
 */
static void test_each(TestCase *tc, void *data)
{
    Store *store = (Store *)data;

    struct EachArg ea;
    ea.p = ea.str;

    store->clear_all(store);    /* Make sure the test directory is empty. */
    store->touch(store, "_1.f1");
    store->touch(store, "_1.f2");
    store->touch(store, "_1.f3");
    store->touch(store, "_1.f4");
    store->each(store, (void(*)(const char *fname, void *arg))&concat_filenames, &ea);
    *(ea.p) = 0;
    Assert(strstr(ea.str, "_1.f1") != NULL, "should contain this file");
    Assert(strstr(ea.str, "_1.f2") != NULL, "should contain this file");
    Assert(strstr(ea.str, "_1.f3") != NULL, "should contain this file");
    Assert(strstr(ea.str, "_1.f4") != NULL, "should contain this file");
    Aiequal(20, strlen(ea.str));
}

/**
 * Test the store->rename function
 */
static void test_rename(TestCase *tc, void *data)
{
    int cnt_before, cnt_after;
    Store *store = (Store *)data;
    store->touch(store, "_from.f1");
    Assert(store->exists(store, "_from.f1"), "File should exist");
    Assert(!store->exists(store, "_to.f1"), "File should not exist");
    cnt_before = store->count(store);
    store->rename(store, "_from.f1", "_to.f1");
    cnt_after = store->count(store);
    Aiequal(cnt_before, cnt_after);
    Assert(store->exists(store, "_to.f1"), "File should now exist");
    Assert(!store->exists(store, "_from.f1"), "File should no longer exist");
}

/**
 * Test the reading and writing of bytes (8 bit)
 */
static void test_rw_bytes(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    uchar bytes[6] = { 0x34, 0x87, 0xF9, 0xEA, 0x00, 0xFF };
    OutStream *ostream = store->new_output(store, "_rw_byte.cfs");
    InStream *istream;
    Assert(store->exists(store, "_rw_byte.cfs"), "File should now exist");
    for (i = 0; i < 6; i++) {
        os_write_byte(ostream, bytes[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_byte.cfs");
    for (i = 0; i < 6; i++) {
        Aiequal(bytes[i], is_read_byte(istream));
    }
    is_close(istream);
    Aiequal(6, store->length(store, "_rw_byte.cfs"));
    Assert(store->exists(store, "_rw_byte.cfs"), "File should now exist");
}

/**
 * Test the reading and writing of 32-bit integers
 */
static void test_rw_i32(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    OutStream *ostream = store->new_output(store, "_rw_int.cfs");
    InStream *istream;
    i32 ints[4] = { POSH_I32_MAX, POSH_I32_MIN, -1, 0 };

    for (i = 0; i < 4; i++) {
        os_write_i32(ostream, (int)ints[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_int.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(ints[i], is_read_i32(istream));
    }

    is_close(istream);
    Aiequal(16, store->length(store, "_rw_int.cfs"));
}

/**
 * Test the reading and writing of 64-bit integers
 */
static void test_rw_i64(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    u64 longs[4] =
        { POSH_I64_MIN, POSH_I64_MAX, POSH_I64(-1), POSH_I64(0) };
    OutStream *ostream = store->new_output(store, "_rw_long.cfs");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_i64(ostream, longs[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_long.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(longs[i], is_read_i64(istream));
    }
    is_close(istream);
    Aiequal(32, store->length(store, "_rw_long.cfs"));
}

/**
 * Test the reading and writing of 32-bit unsigned integers
 */
static void test_rw_u32(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    u32 uints[4] = { POSH_U32_MAX, POSH_U32_MIN, 100000, 1 };
    OutStream *ostream = store->new_output(store, "_rw_uint.cfs");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_u32(ostream, uints[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_uint.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(uints[i], is_read_u32(istream));
    }
    is_close(istream);
    Aiequal(16, store->length(store, "_rw_uint.cfs"));
}

/**
 * Test the reading and writing of 64-bit unsigned integers
 */
static void test_rw_u64(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    u64 ulongs[4] =
        { POSH_U64_MAX, POSH_U64_MIN, POSH_U64(100000000000000), POSH_U64(1) };
    OutStream *ostream = store->new_output(store, "_rw_ulong.cfs");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_u64(ostream, ulongs[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_ulong.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(ulongs[i], is_read_u64(istream));
    }
    is_close(istream);
    Aiequal(32, store->length(store, "_rw_ulong.cfs"));
}

/**
 * Test reading and writing of variable size integers
 */
static void test_rw_vints(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    unsigned int vints[4] = { UINT_MAX, 0, 10000, 1 };
    OutStream *ostream = store->new_output(store, "_rw_vint.cfs");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_vint(ostream, vints[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_vint.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(vints[i], is_read_vint(istream));
    }
    is_close(istream);
}

/**
 * Test reading and writing of variable size integers
 */
static void test_rw_vlls(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    unsigned long vlls[4] = { ULLONG_MAX, 0, 10000, 1 };
    OutStream *ostream = store->new_output(store, "_rw_vll.cfs");
    InStream *istream;

    for (i = 0; i < 4; i++) {
        os_write_vll(ostream, vlls[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_vll.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(vlls[i], is_read_vll(istream));
    }
    is_close(istream);
}

/**
 * Test reading and writing of variable size 64-bit integers
 */
static void test_rw_voff_ts(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    OutStream *ostream;
    InStream *istream;
    i64 voff_ts[4] =
        { LONG_MAX, 0, 1000000, 1 };
    if (sizeof(off_t) == 8) {
        voff_ts[0] = POSH_I64_MAX;
    }
    else {
        voff_ts[0] = POSH_I32_MAX;
    }
    ostream = store->new_output(store, "_rw_voff_t.cfs");

    for (i = 0; i < 4; i++) {
        os_write_voff_t(ostream, (off_t)voff_ts[i]);
    }
    os_close(ostream);

    istream = store->open_input(store, "_rw_voff_t.cfs");
    for (i = 0; i < 4; i++) {
        Aiequal(voff_ts[i], is_read_voff_t(istream));
    }
    is_close(istream);
}

/**
 * Test reading and writing of strings
 */
static void test_rw_strings(TestCase *tc, void *data)
{
    int i;
    char *tmp;
    Store *store = (Store *)data;
    char str[60] =
        "This is a c ferret test string ~!@#$%^&*()`123456790-=\\)_+|";
    char buf[60000] = "";
    OutStream *ostream;
    InStream *istream;

    for (i = 0; i < 1000; i++) {
        strcat(buf, str);
    }

    ostream = store->new_output(store, "_rw_string.cfs");
    os_write_string(ostream, str);
    os_write_string(ostream, buf);
    os_close(ostream);

    istream = store->open_input(store, "_rw_string.cfs");

    tmp = is_read_string(istream);
    Asequal(str, tmp);
    free(tmp);
    tmp = is_read_string(istream);
    Asequal(buf, tmp);
    free(tmp);
    is_close(istream);
    Aiequal(59063, store->length(store, "_rw_string.cfs"));
}

/**
 * Test reading and writing of non-ascii characters
 */
static void test_rw_funny_strings(TestCase *tc, void *data)
{
    int i;
    char *tmp;
    Store *store = (Store *)data;
    char str[18] = "³³ ëêðïéÄ";
    char buf[18000] = "";
    OutStream *ostream;
    InStream *istream;

    for (i = 0; i < 1000; i++) {
        strcat(buf, str);
    }

    ostream = store->new_output(store, "_funny_string.cfs");
    os_write_string(ostream, str);
    os_write_string(ostream, buf);
    os_close(ostream);

    istream = store->open_input(store, "_funny_string.cfs");
    tmp = is_read_string(istream);
    Asequal(str, tmp);
    free(tmp);
    tmp = is_read_string(istream);
    Asequal(buf, tmp);
    free(tmp);
    is_close(istream);
    Aiequal(17021, store->length(store, "_funny_string.cfs"));
}

/**
 * Test seek in both input stream and output stream, ie test the os_seek and
 * is_seek functions
 */
static void test_buffer_seek(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;

    OutStream *ostream = store->new_output(store, "_rw_seek.cfs");
    char text[60] = "This is another int test string !@#$%#$%&%$*%^&*()(_";
    InStream *istream;

    for (i = 0; i < 1000; i++) {
        os_write_i64(ostream, i);
        os_write_string(ostream, text);
    }
    os_seek(ostream, 987);
    Aiequal(987, os_pos(ostream));
    os_write_vint(ostream, 555);
    os_seek(ostream, 56);
    Aiequal(56, os_pos(ostream));
    os_write_vint(ostream, 12345);
    os_seek(ostream, 4000);
    Aiequal(4000, os_pos(ostream));
    os_write_voff_t(ostream, 98763210);
    os_close(ostream);

    istream = store->open_input(store, "_rw_seek.cfs");
    is_seek(istream, 56);
    Aiequal(56, is_pos(istream));
    Aiequal(12345, is_read_vint(istream));
    is_seek(istream, 4000);
    Aiequal(4000, is_pos(istream));
    Aiequal(98763210, is_read_voff_t(istream));
    is_seek(istream, 987);
    Aiequal(987, is_pos(istream));
    Aiequal(555, is_read_vint(istream));
    is_close(istream);
}

/**
 * Test cloning of InputStream
 */
static void test_is_clone(TestCase *tc, void *data)
{
    int i;
    Store *store = (Store *)data;
    OutStream *ostream = store->new_output(store, "_clone.cfs");
    InStream *istream, *alt_istream;

    for (i = 0; i < 10; i++) {
        os_write_i64(ostream, i);
    }
    os_close(ostream);
    istream = store->open_input(store, "_clone.cfs");
    is_seek(istream, 24);
    alt_istream = is_clone(istream);
    Aiequal(is_pos(istream), is_pos(alt_istream));
    for (i = 3; i < 10; i++) {
        Aiequal(i, is_read_i64(alt_istream));
    }
    Aiequal(80, is_pos(alt_istream));
    Aiequal(24, is_pos(istream));
    for (i = 3; i < 10; i++) {
        Aiequal(i, is_read_i64(istream));
    }
    is_close(alt_istream);
    is_close(istream);
}

/**
 * Test the read_bytes method. This method reads a number of bytes into a
 * buffer.
 */
static void test_read_bytes(TestCase *tc, void *data)
{
    char str[11] = "0000000000";
    Store *store = (Store *)data;
    OutStream *ostream = store->new_output(store, "_read_bytes.cfs");
    InStream *istream;

    os_write_bytes(ostream, (uchar *)"how are you doing?", 18);
    os_close(ostream);
    istream = store->open_input(store, "_read_bytes.cfs");
    is_read_bytes(istream, (uchar *)(str + 2), 4);
    Asequal("00how 0000", str);
    is_read_bytes(istream, (uchar *)(str + 1), 8);
    Asequal("0are you 0", str);
    is_close(istream);
}

/**
 * Create a test suite for a store. This function can be used to create a test
 * suite for both a FileSystem store and a RAM store and any other type of
 * store that you might want to create.
 */
void create_test_store_suite(TestSuite *suite, Store *store)
{
    store->clear_all(store);

    tst_run_test(suite, test_basic_file_ops, store);
    tst_run_test(suite, test_rename, store);
    tst_run_test(suite, test_each, store);
    tst_run_test(suite, test_rw_bytes, store);
    tst_run_test(suite, test_rw_i32, store);
    tst_run_test(suite, test_rw_i64, store);
    tst_run_test(suite, test_rw_u32, store);
    tst_run_test(suite, test_rw_u64, store);
    tst_run_test(suite, test_rw_vints, store);
    tst_run_test(suite, test_rw_vlls, store);
    tst_run_test(suite, test_rw_voff_ts, store);
    tst_run_test(suite, test_rw_strings, store);
    tst_run_test(suite, test_rw_funny_strings, store);
    tst_run_test(suite, test_buffer_seek, store);
    tst_run_test(suite, test_is_clone, store);
    tst_run_test(suite, test_read_bytes, store);
    tst_run_test(suite, test_lock, store);

    store->clear_all(store);
}

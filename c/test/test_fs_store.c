#include "store.h"
#include "test_store.h"
#include "test.h"

/**
 * Test a FileSystem store
 */
TestSuite *ts_fs_store(TestSuite *suite)
{

#ifdef POSH_OS_WIN32
    Store *store = open_fs_store(".\\test\\testdir\\store");
#else
    Store *store = open_fs_store("./test/testdir/store");
#endif
    store->clear(store);

    suite = ADD_SUITE(suite);

    create_test_store_suite(suite, store);

    store_deref(store);

    return suite;
}

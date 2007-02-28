#include "store.h"
#include "test_store.h"

/**
 * Test ramo_write_to which basically writes the contents of a RAM output
 * stream to another output stream, usually a FileSystem output stream.
 */
void test_write_to(tst_case * tc, void *data)
{
    int i;
    char *tmp;
    Store *ram_store = open_ram_store();
    Store *fs_store = open_fs_store("./test/testdir/store");
    char str[12] = "³³ øãíøäÄ";
    char buf[12000] = "";
    OutStream *ostream, *fs_ostream;
    InStream *istream;
    (void)data;/* suppress unused parameter warning */

    fs_store->clear(fs_store);

    for (i = 0; i < 1000; i++) {
        strcat(buf, str);
    }

    ostream = ram_store->new_output(ram_store, "rw_funny_string.test");
    os_write_string(ostream, str);
    os_write_string(ostream, buf);

    fs_ostream = fs_store->new_output(fs_store, "rw_funny_string.test");
    ramo_write_to(ostream, fs_ostream);

    os_close(ostream);
    os_close(fs_ostream);

    istream = fs_store->open_input(fs_store, "rw_funny_string.test");
    Asequal(str, tmp = is_read_string(istream));
    free(tmp);
    Asequal(buf, tmp = is_read_string(istream));
    free(tmp);
    is_close(istream);

    Aiequal(9012, fs_store->length(fs_store, "rw_funny_string.test"));
    store_deref(ram_store);
    ram_store = open_ram_store_and_copy(fs_store, false);

    istream = ram_store->open_input(ram_store, "rw_funny_string.test");
    Asequal(str, tmp = is_read_string(istream));
    free(tmp);
    Asequal(buf, tmp = is_read_string(istream));
    free(tmp);
    is_close(istream);
    Aiequal(9012, ram_store->length(ram_store, "rw_funny_string.test"));

    fs_store->clear_all(fs_store);
    store_deref(fs_store);
    store_deref(ram_store);
}

/**
 * Create the RAMStore test suite
 */
tst_suite *ts_ram_store(tst_suite * suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    create_test_store_suite(suite, store);

    tst_run_test(suite, test_write_to, NULL);

    store_deref(store);

    return suite;
}

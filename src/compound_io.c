#include "index.h" 

extern void store_destroy(Store *store);
extern InStream *is_new();
extern Store *store_new();

/****************************************************************************
 *
 * CompoundStore
 *
 ****************************************************************************/

typedef struct FileEntry {
    long offset;
    long length;
} FileEntry;

static void cmpd_touch(Store *store, char *file_name)
{
    store->dir.cmpd->store->touch(store->dir.cmpd->store, file_name);
}

static int cmpd_exists(Store *store, char *file_name)
{
    if (h_get(store->dir.cmpd->entries, file_name) != NULL) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * @throws UNSUPPORTED_ERROR
 */
static int cmpd_remove(Store *store, char *file_name)
{
    (void)store;
    (void)file_name;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return 0;
}

/**
 * @throws UNSUPPORTED_ERROR
 */
static void cmpd_rename(Store *store, char *from, char *to)
{
    (void)store;
    (void)from;
    (void)to;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
}

static int cmpd_count(Store *store)
{
    return store->dir.cmpd->entries->size;
}

/**
 * @throws UNSUPPORTED_ERROR
 */
static void cmpd_clear(Store *store)
{
    (void)store;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
}

static void cmpd_close_i(Store *store)
{
    CompoundStore *cmpd = store->dir.cmpd;
    if (cmpd->stream == NULL) {
        RAISE(IO_ERROR, "Tried to close already closed compound store");
    }

    h_destroy(cmpd->entries);

    is_close(cmpd->stream);
    cmpd->stream = NULL;
    free(store->dir.cmpd);
    store_destroy(store);
}

static long cmpd_length(Store *store, char *file_name)
{
    FileEntry *fe = h_get(store->dir.cmpd->entries, file_name);
    if (fe != NULL) {
        return fe->length;
    }
    else {
        return 0;
    }
}

static void cmpdi_seek_i(InStream *is, long pos)
{
    (void)is;
    (void)pos;
}

static void cmpdi_close_i(InStream *is)
{
    free(is->d.cis);
}

static void cmpdi_clone_i(InStream *is, InStream *new_is)
{
    CompoundInStream *cis = ALLOC(CompoundInStream);
    cis->sub = is->d.cis->sub;
    cis->offset = is->d.cis->offset;
    cis->length = is->d.cis->length;
    new_is->d.cis = cis;
}

static long cmpdi_length_i(InStream *is)
{
    return (is->d.cis->length);
}

/*
 * raises: EOF_ERROR
 */
static void cmpdi_read_i(InStream *is, uchar *b, int len)
{
    CompoundInStream *cis = is->d.cis;
    long start = is_pos(is);

    if ((start + len) > cis->length) {
        RAISE(EOF_ERROR, "Tried to read past end of file. File length is "
              "<%ld> and tried to read to <%ld>", cis->length, start + len);
    }

    is_seek(cis->sub, cis->offset + start);
    is_read_bytes(cis->sub, b, len);
}

static const struct InStreamMethods CMPD_IN_STREAM_METHODS = {
    cmpdi_read_i,
    cmpdi_seek_i,
    cmpdi_length_i,
    cmpdi_clone_i,
    cmpdi_close_i
};

static InStream *cmpd_create_input(InStream *sub_is, long offset, long length)
{
    InStream *is = is_new();
    CompoundInStream *cis = ALLOC(CompoundInStream);

    cis->sub = sub_is;
    cis->offset = offset;
    cis->length = length;
    is->d.cis = cis;
    is->file.p = NULL;
    is->m = &CMPD_IN_STREAM_METHODS;

    return is;
}

static InStream *cmpd_open_input(Store *store, const char *file_name)
{
    FileEntry *entry;
    CompoundStore *cmpd = store->dir.cmpd;
    InStream *is;

    mutex_lock(&store->mutex);
    if (cmpd->stream == NULL) {
        mutex_unlock(&store->mutex);
        RAISE(IO_ERROR, "Can't open compound file input stream. Parent "
              "stream is closed.");
    }

    entry = h_get(cmpd->entries, file_name);
    if (entry == NULL) {
        mutex_unlock(&store->mutex);
        RAISE(IO_ERROR, "File %s does not exist", file_name);
    }

    is = cmpd_create_input(cmpd->stream, entry->offset, entry->length);
    mutex_unlock(&store->mutex);

    return is;
}

static OutStream *cmpd_new_output(Store *store, const char *file_name)
{
    (void)store;
    (void)file_name;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static Lock *cmpd_open_lock(Store *store, char *lock_name)
{
    (void)store;
    (void)lock_name;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
    return NULL;
}

static void cmpd_close_lock(Lock *lock)
{
    (void)lock;
    RAISE(UNSUPPORTED_ERROR, UNSUPPORTED_ERROR_MSG);
}

Store *open_cmpd_store(Store *store, const char *name)
{
    int count, i;
    long offset;
    char *fname;
    FileEntry *entry;
    Store *new_store = NULL;
    CompoundStore *cmpd = NULL;
    InStream *is = NULL;

    new_store = store_new();
    cmpd = ALLOC(CompoundStore);

    cmpd->store       = store;
    cmpd->name        = name;
    cmpd->entries     = h_new_str(&free, &free);
    is = cmpd->stream = store->open_input(store, cmpd->name);

    /* read the directory and init files */
    count = is_read_vint(is);
    entry = NULL;
    for (i = 0; i < count; i++) {
        offset = (long)is_read_i64(is);
        fname = is_read_string(is);

        if (entry != NULL) {
            /* set length of the previous entry */
            entry->length = offset - entry->offset;
        }

        entry = ALLOC(FileEntry);
        entry->offset = offset;
        h_set(cmpd->entries, fname, entry);
    }

    /* set the length of the final entry */
    if (entry != NULL) {
        entry->length = is_length(is) - entry->offset;
    }

    free(new_store);
    free(cmpd);
    if (is) {
        is_close(is);
    }

    new_store->dir.cmpd     = cmpd;
    new_store->touch        = &cmpd_touch;
    new_store->exists       = &cmpd_exists;
    new_store->remove       = &cmpd_remove;
    new_store->rename       = &cmpd_rename;
    new_store->count        = &cmpd_count;
    new_store->clear        = &cmpd_clear;
    new_store->length       = &cmpd_length;
    new_store->close_i      = &cmpd_close_i;
    new_store->new_output   = &cmpd_new_output;
    new_store->open_input   = &cmpd_open_input;
    new_store->open_lock    = &cmpd_open_lock;
    new_store->close_lock   = &cmpd_close_lock;

    return new_store;
}

/****************************************************************************
 *
 * CompoundWriter
 *
 ****************************************************************************/

typedef struct WFileEntry {
    char *name;
    long dir_offset;
    long data_offset;
} WFileEntry;

static WFileEntry *wfe_create(char *name)
{
    WFileEntry *wfe = ALLOC(WFileEntry);
    wfe->name = name;
    return wfe;
}

CompoundWriter *open_cw(Store *store, char *name)
{
    CompoundWriter *cw = ALLOC(CompoundWriter);
    cw->store = store;
    cw->name = name;
    cw->ids = hs_str_new(NULL);
    // TODO: FIXME
    cw->file_entries = NULL;//ary_create(1, &free);
    cw->merged = false;
    return cw;
}

void cw_add_file(CompoundWriter *cw, char *id)
{
    if (cw->merged) {
        RAISE(IO_ERROR, "Tried to merge already merged compound store");
    }
    if (hs_add(cw->ids, id) != HASH_KEY_DOES_NOT_EXIST) {
        RAISE(IO_ERROR, "Tried to add file \"%s\" which has already been "
              "added to the compound store", id);
    }

    hs_add(cw->ids, id);
    //FIXME
    //ary_append(cw->file_entries, wfe_create(id));
}

static void cw_copy_file(CompoundWriter *cw, WFileEntry *src, OutStream *os)
{
    long start_ptr = os_pos(os);
    long end_ptr;
    long remainder, length, len;
    uchar buffer[BUFFER_SIZE];

    InStream *is = cw->store->open_input(cw->store, src->name);

    remainder = length = is_length(is);

    while (remainder > 0) {
        len = MIN(remainder, BUFFER_SIZE);
        is_read_bytes(is, buffer, len);
        os_write_bytes(os, buffer, len);
        remainder -= len;
    }

    /* Verify that remainder is 0 */
    if (remainder != 0) {
        RAISE(IO_ERROR, "There seems to be an error in the compound file "
              "should have read to the end but there are <%ld> bytes left",
              remainder);
    }

    /* Verify that the output length diff is equal to original file */
    end_ptr = os_pos(os);
    len = end_ptr - start_ptr;
    if (len != length) {
        RAISE(IO_ERROR, "Difference in compound file output file offsets <%ld> "
              "does not match the original file lenght <%ld>", len, length);
    }

    is_close(is);
}

void cw_close(CompoundWriter *cw)
{
    OutStream *os = NULL;
    int i;
    WFileEntry *wfe;

    if (cw->merged) {
        RAISE(STATE_ERROR, "Tried to merge already merged compound file");
    }

    if (cw->ids->size <= 0) {
        RAISE(STATE_ERROR, "Tried to merge compound file with no entries");
    }

    cw->merged = true;

    os = cw->store->new_output(cw->store, cw->name);

    // FIXME: file_entries is an array
    //os_write_vint(os, cw->file_entries->size);

    /* Write the directory with all offsets at 0.
     * Remember the positions of directory entries so that we can adjust the
     * offsets later */

    // FIXME: file_entries is an array
    //for (i = 0; i < cw->file_entries->size; i++) {
    for (i = 0; i < 10; i++) {
        wfe = (WFileEntry *)cw->file_entries[i];
        wfe->dir_offset = os_pos(os);
        os_write_u64(os, 0);  /* for now */
        os_write_string(os, wfe->name);
    }

    /* Open the files and copy their data into the stream.  Remember the
     * locations of each file's data section. */
    // FIXME: file_entries is an array
    //for (i = 0; i < cw->file_entries->size; i++) {
    for (i = 0; i < 10; i++) {
        wfe = (WFileEntry *)cw->file_entries[i];
        wfe->data_offset = os_pos(os);
        cw_copy_file(cw, wfe, os);
    }

    /* Write the data offsets into the directory of the compound stream */
    // FIXME: file_entries is an array
    //for (i = 0; i < cw->file_entries->size; i++) {
    for (i = 0; i < 10; i++) {
        wfe = (WFileEntry *)cw->file_entries[i];
        os_seek(os, wfe->dir_offset);
        os_write_u64(os, wfe->data_offset);
    }

    if (os) {
        os_close(os);
    }

    hs_destroy(cw->ids);
    //FIXME
    //ary_destroy(cw->file_entries);
    free(cw);
}

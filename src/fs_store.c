#include "store.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#ifdef POSH_OS_WIN32
# include <io.h>
# include "win32.h"
# ifndef sleep
#	define sleep _sleep
# endif
# ifndef DIR_SEPARATOR
#   define DIR_SEPARATOR "\\"
# endif
# ifndef S_IRUSR
#	define S_IRUSR _S_IREAD
# endif
# ifndef S_IWUSR
#	define S_IWUSR _S_IWRITE
# endif
#else
# define DIR_SEPARATOR "/"
# include <unistd.h>
# include <dirent.h>
#endif

static char *const FILE_OPEN_ERROR_MSG = "Couldn't open the file to read";
static char *const SEEK_ERROR_MSG = "Seek error message";
static char *const WRITE_ERROR_MSG = "Write error message";

extern Store *store_create();
extern void store_destroy(Store *store);
extern OutStream *os_create();
extern InStream *is_create();
extern int file_is_lock(char *filename);

/**
 * Create a filepath for a file in the store using the operating systems
 * default file seperator.
 */
static char *join_path(char *buf, const char *base, const char *filename)
{
  sprintf(buf, "%s"DIR_SEPARATOR"%s", base, filename);
  return buf;
}

static void fs_touch(Store *store, char *filename)
{
    int f;
    char path[MAX_FILE_PATH];
    join_path(path, store->dir.path, filename);
    if ((f = creat(path, S_IRUSR | S_IWUSR)) == 0) {
        RAISE(IO_ERROR, strerror(errno));
    }
    close(f);
}

static int fs_exists(Store *store, char *filename)
{
	int fd;
    char buf[MAX_FILE_PATH];
    join_path(buf, store->dir.path, filename);
    fd = open(buf, 0);
    if (fd < 0) {
        if (errno != ENOENT) {
            RAISE(IO_ERROR, strerror(errno));
        }
        return false;
    }
    close(fd);
    return true;
}

static int fs_remove(Store *store, char *filename)
{
    char buf[MAX_FILE_PATH];
    return remove(join_path(buf, store->dir.path, filename));
}

static void fs_rename(Store *store, char *from, char *to)
{
    char buf1[MAX_FILE_PATH], buf2[MAX_FILE_PATH];

#ifdef POSH_OS_WIN32
    remove(join_path(buf1, store->dir.path, to));
#endif

    if (rename(join_path(buf1, store->dir.path, from),
               join_path(buf2, store->dir.path, to)) < 0) {
        RAISE(IO_ERROR, strerror(errno));
    }
}

static int fs_count(Store *store)
{
    int cnt = 0;
    struct dirent *de;
    DIR *d = opendir(store->dir.path);

    if (!d) RAISE(IO_ERROR, strerror(errno));

    while ((de = readdir(d)) != NULL) {
        if (de->d_name[0] > '/') {
            cnt++;
        }
    }
    closedir(d);

    return cnt;
}

static void fs_each(Store *store, void (*func) (char *fname, void *arg), void *arg)
{
    struct dirent *de;
    DIR *d = opendir(store->dir.path);

    if (!d) RAISE(IO_ERROR, strerror(errno));

    while ((de = readdir(d)) != NULL) {
		if (de->d_name[0] > '/' && !file_is_lock(de->d_name)) {
            func(de->d_name, arg);
        }
    }
    closedir(d);
}

static void fs_clear_locks(Store *store)
{
  struct dirent *de;
  DIR *d = opendir(store->dir.path);

  if (!d) RAISE(IO_ERROR, strerror(errno));

  while ((de = readdir(d)) != NULL) {
    if (file_is_lock(de->d_name)) {
      char buf[MAX_FILE_PATH];
      remove(join_path(buf, store->dir.path, de->d_name));
    }
  }
  closedir(d);
}

static void fs_clear(Store *store)
{
  struct dirent *de;
  DIR *d = opendir(store->dir.path);

  if (!d) RAISE(IO_ERROR, strerror(errno));

  while ((de = readdir(d)) != NULL) {
    if (de->d_name[0] > '/' && !file_is_lock(de->d_name)) {
      char buf[MAX_FILE_PATH];
      remove(join_path(buf, store->dir.path, de->d_name));
    }
  }
  closedir(d);
}

static void fs_clear_all(Store *store)
{
  struct dirent *de;
  DIR *d = opendir(store->dir.path);

  if (!d) RAISE(IO_ERROR, strerror(errno));

  while ((de = readdir(d)) != NULL) {
    if (de->d_name[0] > '/') {
      char buf[MAX_FILE_PATH];
      remove(join_path(buf, store->dir.path, de->d_name));
    }
  }
  closedir(d);
}

/**
 * Destroy the store.
 *
 * @param p the store to destroy
 * @raise IO_ERROR if there is an error deleting the locks
 */
static void fs_destroy(Store *store)
{
    fs_clear_locks(store);
    free(store->dir.path);
    store_destroy(store);
}

static long fs_length(Store *store, char *filename)
{
    char buf[MAX_FILE_PATH];
    struct stat stt;

    if (stat(join_path(buf, store->dir.path, filename), &stt)) {
        RAISE(IO_ERROR, strerror(errno));
    }

    return (long)stt.st_size;
}

static void fso_flush_i(OutStream *os, uchar *src, int len)
{
    if (len != (int)fwrite(src, sizeof(uchar), len, (FILE *)os->file)) {
        RAISE(IO_ERROR, WRITE_ERROR_MSG);
    }
}

static void fso_seek_i(OutStream *os, long pos)
{
    if (fseek((FILE *) os->file, pos, SEEK_SET)) {
        RAISE(IO_ERROR, strerror(errno));
    }
}

static void fso_close_i(OutStream *os)
{
    if (fclose((FILE *) os->file)) {
        RAISE(IO_ERROR, strerror(errno));
    }
}

static OutStream *fs_create_output(Store *store, const char *filename)
{
    char buf[MAX_FILE_PATH];
    FILE *f = fopen(join_path(buf, store->dir.path, filename), "wb");
    OutStream *os;
    if (!f) {
        RAISE(IO_ERROR, strerror(errno));
    }

    os = os_create();
    os->file = f;
    os->flush_i = &fso_flush_i;
    os->seek_i = &fso_seek_i;
    os->close_i = &fso_close_i;
    return os;
}

static void fsi_read_i(InStream *is, uchar *buf, int len)
{
    int fd = is->file.fd;
    int pos = is_pos(is);
    if (pos != lseek(fd, 0, SEEK_CUR)) {
        lseek(fd, pos, SEEK_SET);
    }
    if (read(fd, buf, len) != len) {
        /* win: the wrong value can be returned for some reason so double check */
        if (lseek(fd, 0, SEEK_CUR) != (pos + len)) {
            RAISE(EOF_ERROR, strerror(errno));
        }
    }
}

static void fsi_seek_i(InStream *is, long pos)
{
    if (lseek(is->file.fd, pos, SEEK_SET) < 0) {
        RAISE(IO_ERROR, strerror(errno));
    }
}

static void fsi_close_i(InStream *is)
{
    if (!is->is_clone) {
        if (close(is->file.fd)) {
            RAISE(IO_ERROR, strerror(errno));
        }
        free(is->d.path);
    }
}

static long fsi_length(InStream *is)
{
    struct stat stt;
    if (fstat(is->file.fd, &stt)) {
        RAISE(IO_ERROR, strerror(errno));
    }
    return (long)stt.st_size;
}

/*
 * Clone the input stream. Nothing to do for a file system input stream
 */
static void fsi_clone_i(InStream *is, InStream *new_is)
{
    (void)is;
    (void)new_is;
}

static InStream *fs_open_input(Store *store, const char *filename)
{
    InStream *is;
    char buf[MAX_FILE_PATH];
    int fd = open(join_path(buf, store->dir.path, filename), O_RDONLY);
    if (fd < 0) {
        RAISE(IO_ERROR, FILE_OPEN_ERROR_MSG);
    }

    is = is_create();
    is->file.fd = fd;
    is->d.path = estrdup(buf);
    is->is_clone = false;
    is->read_i = &fsi_read_i;
    is->seek_i = &fsi_seek_i;
    is->close_i = &fsi_close_i;
    is->clone_i = &fsi_clone_i;
    is->length_i = &fsi_length;
    return is;
}

#define LOCK_OBTAIN_TIMEOUT 5

static int fs_lock_obtain(Lock *lock)
{
    int f;
    int trys = LOCK_OBTAIN_TIMEOUT;
    while (((f =
             open(lock->name, O_CREAT | O_EXCL | O_WRONLY,
                   S_IRUSR | S_IWUSR)) < 0) && (trys > 0)) {
        trys--;
        sleep(1);
    }
    if (f >= 0) {
        close(f);
        return true;
    }
    else {
        return false;
    }
}

static int fs_lock_is_locked(Lock *lock)
{
    int f = open(lock->name, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
    if (f >= 0) {
        if (close(f) || remove(lock->name)) {
            RAISE(IO_ERROR, strerror(errno));
        }
        return false;
    }
    else {
        return true;
    }
}

void fs_lock_release(Lock *lock)
{
    remove(lock->name);
}

static Lock *fs_open_lock(Store *store, char *lockname)
{
    Lock *lock = ALLOC(Lock);
    char lname[100];
    char buf[MAX_FILE_PATH];
    sprintf(lname, "%s%s.lck", LOCK_PREFIX, lockname);
    lock->name = estrdup(join_path(buf, store->dir.path, lname));
    lock->obtain = &fs_lock_obtain;
    lock->release = &fs_lock_release;
    lock->is_locked = &fs_lock_is_locked;
    return lock;
}

static void fs_close_lock(Lock *lock)
{
    remove(lock->name);
    free(lock->name);
    free(lock);
}

static HashTable stores = {
    /* fill */       0,
    /* used */       0,
    /* mask */       HASH_MINSIZE - 1,
    /* table */      stores.smalltable,
    /* smalltable */ {{0, NULL, NULL}},
    /* lookup */     (h_lookup_ft)&h_lookup_str,
    /* hash */       NULL,
    /* eq */         NULL,
    /* free_key */   (free_ft)&dummy_free,
    /* free_value */ (free_ft)&fs_destroy
};

#ifndef UNTHREADED
static mutex_t stores_mutex = MUTEX_INITIALIZER;
#endif

static void fs_close_i(Store *store)
{
    mutex_lock(&stores_mutex);
    h_del(&stores, store->dir.path);
    mutex_unlock(&stores_mutex);
}

static Store *fs_store_create(const char *pathname)
{
    Store *new_store = store_create();

    new_store->dir.path      = estrdup(pathname);
    new_store->touch         = &fs_touch;
    new_store->exists        = &fs_exists;
    new_store->remove        = &fs_remove;
    new_store->rename        = &fs_rename;
    new_store->count         = &fs_count;
    new_store->close_i       = &fs_close_i;
    new_store->clear         = &fs_clear;
    new_store->clear_all     = &fs_clear_all;
    new_store->clear_locks   = &fs_clear_locks;
    new_store->length        = &fs_length;
    new_store->each          = &fs_each;
    new_store->create_output = &fs_create_output;
    new_store->open_input    = &fs_open_input;
    new_store->open_lock     = &fs_open_lock;
    new_store->close_lock    = &fs_close_lock;
    return new_store;
}

Store *open_fs_store(const char *pathname)
{
    Store *store = NULL;

    mutex_lock(&stores_mutex);
    store = h_get(&stores, pathname);
    if (store) {
        mutex_lock(&store->mutex);
        store->ref_cnt++;
        mutex_unlock(&store->mutex);
    }
    else {
        store = fs_store_create(pathname);
        h_set(&stores, store->dir.path, store);
    }
    mutex_unlock(&stores_mutex);

    return store;
}

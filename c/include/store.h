#ifndef FRT_STORE_H
#define FRT_STORE_H

#include <sys/types.h>
#include "global.h"
#include "hash.h"
#include "hashset.h"
#include "threading.h"

#define FRT_LOCK_PREFIX "ferret-"
#define FRT_LOCK_EXT ".lck"

typedef struct FrtBuffer
{
    frt_uchar buf[FRT_BUFFER_SIZE];
    off_t start;
    off_t pos;
    off_t len;
} FrtBuffer;

typedef struct FrtOutStream FrtOutStream;
struct FrtOutStreamMethods {
    /* internal functions for the FrtInStream */
    /**
     * Flush +len+ characters from +src+ to the output stream +os+
     *
     * @param os self
     * @param src the characters to write to the output stream
     * @param len the number of characters to write
     * @raise FRT_IO_ERROR if there is an error writing the characters
     */
    void (*flush_i)(struct FrtOutStream *os, const frt_uchar *buf, int len);

    /**
     * Seek +pos+ in the output stream
     *
     * @param os self
     * @param pos the position to seek in the stream
     * @raise FRT_IO_ERROR if there is an error seeking in the output stream
     */
    void (*seek_i)(struct FrtOutStream *os, off_t pos);

    /**
     * Close any resources used by the output stream +os+
     *
     * @param os self
     * @raise FRT_IO_ERROR if there is an closing the file
     */
    void (*close_i)(struct FrtOutStream *os);
};

typedef struct FrtRAMFile
{
    char   *name;
    frt_uchar **buffers;
    int     bufcnt;
    off_t   len;
    int     ref_cnt;
} FrtRAMFile;

struct FrtOutStream
{
    FrtBuffer buf;
    union
    {
        int fd;
        FrtRAMFile *rf;
    } file;
    off_t  pointer;             /* only used by RAMOut */
    const struct FrtOutStreamMethods *m;
};

typedef struct FrtCompoundInStream FrtCompoundInStream;

typedef struct FrtInStream FrtInStream;

struct FrtInStreamMethods
{
    /**
     * Read +len+ characters from the input stream into the +offset+ position in
     * +buf+, an array of unsigned characters.
     *
     * @param is self
     * @param buf an array of characters which must be allocated with  at least
     *          +offset+ + +len+ bytes
     * @param len the number of bytes to read
     * @raise FRT_IO_ERROR if there is an error reading from the input stream
     */
    void (*read_i)(struct FrtInStream *is, frt_uchar *buf, int len);

    /**
     * Seek position +pos+ in input stream +is+
     *
     * @param is self
     * @param pos the position to seek
     * @raise FRT_IO_ERROR if the seek fails
     */
    void (*seek_i)(struct FrtInStream *is, off_t pos);

    /**
     * Returns the length of the input stream +is+
     *
     * @param is self
     * @raise FRT_IO_ERROR if there is an error getting the file length
     */
    off_t (*length_i)(struct FrtInStream *is);

    /**
     * Close the resources allocated to the inputstream +is+
     *
     * @param is self
     * @raise FRT_IO_ERROR if the close fails
     */
    void (*close_i)(struct FrtInStream *is);
};

struct FrtInStream
{
    FrtBuffer buf;
    union
    {
        int fd;
        FrtRAMFile *rf;
    } file;
    union
    {
        off_t pointer;          /* only used by RAMIn */
        char *path;             /* only used by FSIn */
        FrtCompoundInStream *cis;
    } d;
    int *ref_cnt_ptr;
    const struct FrtInStreamMethods *m;
};

struct FrtCompoundInStream
{
    FrtInStream *sub;
    off_t offset;
    off_t length;
};

#define is_length(mis) mis->m->length_i(mis)

typedef struct FrtStore FrtStore;
typedef struct FrtLock FrtLock;
struct FrtLock
{
    char *name;
    FrtStore *store;
    int (*obtain)(FrtLock *lock);
    int (*is_locked)(FrtLock *lock);
    void (*release)(FrtLock *lock);
};

typedef struct FrtCompoundStore
{
    FrtStore *store;
    const char *name;
    FrtHash *entries;
    FrtInStream *stream;
} FrtCompoundStore;

struct FrtStore
{
    int ref_cnt;                /* for fs_store only */
    frt_mutex_t mutex_i;        /* for internal use only */
    frt_mutex_t mutex;          /* external mutex for use outside */
    union
    {
        char *path;             /* for fs_store only */
        FrtHash *ht;            /* for ram_store only */
        FrtCompoundStore *cmpd; /* for compound_store only */
    } dir;

#ifdef POSH_OS_WIN32
    int file_mode;
#else
    mode_t file_mode;
#endif
    FrtHashSet *locks;

    /**
     * Create the file +filename+ in the +store+.
     *
     * @param store self
     * @param filename the name of the file to create
     * @raise FRT_IO_ERROR if the file cannot be created
     */
    void (*touch)(FrtStore *store, const char *filename);

    /**
     * Return true if a file of name +filename+ exists in +store+.
     *
     * @param store self
     * @param filename the name of the file to check for
     * @returns true if the file exists
     * @raise FRT_IO_ERROR if there is an error checking for the files existance
     */
    int (*exists)(FrtStore *store, const char *filename);

    /**
     * Remove the file +filename+ from the +store+
     *
     * @param store self
     * @param filename the name of the file to remove
     * @returns On success, zero is returned.  On error, -1 is returned, and errno
     *          is set appropriately.
     */
    int (*remove)(FrtStore *store, const char *filename);

    /**
     * Rename the file in the +store+ from the name +from+ to the name +to+.
     *
     * @param store self
     * @param from the name of the file to rename
     * @param to the new name of the file
     * @raise FRT_IO_ERROR if there is an error renaming the file
     */
    void (*rename)(FrtStore *store, const char *from, const char *to);

    /**
     * Returns the number of files in the store.
     *
     * @param store self
     * @return the number of files in the store
     * @raise FRT_IO_ERROR if there is an error opening the directory
     */
    int (*count)(FrtStore *store);

    /**
     * Call the function +func+ with each filename in the store and the arg
     * that you passed. If you need to open the file you should pass the store
     * as the argument. If you need to pass more than one argument, you should
     * pass a struct.
     *
     * @param store self
     * @param func the function to call with each files name and the +arg+
     *   passed
     * @param arg the argument to pass to the function
     * @raise FRT_IO_ERROR if there is an error opening the directory
     */
    void (*each)(FrtStore *store, void (*func)(const char *fname, void *arg),
                  void *arg);

    /**
     * Clear all the locks in the store.
     *
     * @param store self
     * @raise FRT_IO_ERROR if there is an error opening the directory
     */
    void (*clear_locks)(FrtStore *store);

    /**
     * Clear all files from the store except the lock files.
     *
     * @param store self
     * @raise FRT_IO_ERROR if there is an error deleting the files
     */
    void (*clear)(FrtStore *store);

    /**
     * Clear all files from the store including the lock files.
     *
     * @param store self
     * @raise FRT_IO_ERROR if there is an error deleting the files
     */
    void (*clear_all)(FrtStore *store);

    /**
     * Return the length of the file +filename+ in +store+
     *
     * @param store self
     * @param the name of the file to check the length of
     * @return the length of the file in bytes
     * @raise FRT_IO_ERROR if there is an error checking the file length
     */
    off_t (*length)(FrtStore *store, const char *filename);

    /**
     * Allocate the resources needed for the output stream in the +store+ with
     * the name +filename+
     *
     * @param store self
     * @param filename the name of the output stream
     * @return a newly allocated filestream
     * @raise FRT_IO_ERROR if there is an error opening the output stream
     *   resources
     */
    FrtOutStream *(*new_output)(FrtStore *store, const char *filename);

    /**
     * Open an input stream in the +store+ with the name +filename+
     *
     * @param store self
     * @param filename the name of the input stream
     * @raise FRT_FILE_NOT_FOUND_ERROR if the input stream cannot be opened
     */
    FrtInStream *(*open_input)(FrtStore *store, const char *filename);

    /**
     * Obtain a lock on the lock +lock+
     *
     * @param store self
     * @param lock the lock to obtain
     */
    FrtLock *(*open_lock_i)(FrtStore *store, const char *lockname);

    /**
     * Returns true if +lock+ is locked. To test if the file is locked:wq
     *
     * @param lock the lock to test
     * @raise FRT_IO_ERROR if there is an error detecting the lock status
     */
    void (*close_lock_i)(FrtLock *lock);

    /**
     * Internal function to close the store freeing implementation specific
     * resources.
     *
     * @param store self
     */
    void (*close_i)(FrtStore *store);
};

/**
 * Create a newly allocated file-system FrtStore at the pathname designated. The
 * pathname must be the name of an existing directory.
 *
 * @param pathname the pathname of the directory to be used by the index
 * @return a newly allocated file-system FrtStore.
 */
extern FrtStore *frt_open_fs_store(const char *pathname);

/**
 * Create a newly allocated in-memory or RAM FrtStore.
 *
 * @return a newly allocated RAM FrtStore.
 */
extern FrtStore *frt_open_ram_store();

/**
 * Create a newly allocated in-memory or RAM FrtStore. Copy the contents of
 * another store into this store. Then close the other store if required. This
 * method would be used for example to read an index into memory for faster
 * searching.
 *
 * @param store the whose contents will be copied into the newly allocated RAM
 *   store
 * @param close_store close the store whose contents where copied
 * @return a newly allocated RAM FrtStore.
 */
extern FrtStore *frt_open_ram_store_and_copy(FrtStore *store, bool close_store);

/**
 * Open a compound store. This is basically store which is stored within a
 * single file and can in turn be stored within either a FileSystem or RAM
 * store.
 *
 * @param store the store within which this compound store will be stored
 * @param filename the name of the file in which to store the compound store
 * @return a newly allocated Compound FrtStore.
 */
extern FrtStore *frt_open_cmpd_store(FrtStore *store, const char *filename);

/*
 * == RamStore functions ==
 *
 * These functions or optimizations to be used when you know you are using a
 * Ram FrtOutStream.
 */

/**
 * Return the length of the FrtOutStream in bytes.
 *
 * @param os the FrtOutStream who's length you want
 * @return the length of +os+ in bytes
 */
extern off_t frt_ramo_length(FrtOutStream *os);

/**
 * Reset the FrtOutStream removing any data written to it. Since it is a RAM
 * file, all that needs to be done is set the length to 0.
 *
 * @param os the FrtOutStream to reset
 */
extern void frt_ramo_reset(FrtOutStream *os);

/**
 * Write the contents of a RAM FrtOutStream to another FrtOutStream.
 *
 * @param from_os the FrtOutStream to write from
 * @param to_os the FrtOutStream to write to
 */
extern void frt_ramo_write_to(FrtOutStream *from_os, FrtOutStream *to_os);

/**
 * Create a buffer RAM FrtOutStream which is unassociated with any RAM FrtStore.
 * This FrtOutStream can be used to write temporary data too. When the time
 * comes, this data can be written to another FrtOutStream (which might possibly
 * be a file-system FrtOutStream) using frt_ramo_write_to.
 *
 * @return A newly allocated RAM FrtOutStream
 */
extern FrtOutStream *frt_ram_new_buffer();

/**
 * Destroy a RAM FrtOutStream which is unassociated with any RAM FrtStore, freeing
 * all resources allocated to it.
 *
 * @param os the FrtOutStream to destroy
 */
extern void frt_ram_destroy_buffer(FrtOutStream *os);

/**
 * Call the function +func+ with the +lock+ locked. The argument +arg+ will be
 * passed to +func+. If you need to pass more than one argument you should use
 * a struct. When the function is finished, release the lock.
 *
 * @param lock     lock to be locked while func is called
 * @param func     function to call with the lock locked
 * @param arg      argument to pass to the function
 * @raise FRT_IO_ERROR if the lock is already locked
 * @see frt_with_lock_name
 */
extern void frt_with_lock(FrtLock *lock, void (*func)(void *arg), void *arg);

/**
 * Create a lock in the +store+ with the name +lock_name+. Call the function
 * +func+ with the lock locked. The argument +arg+ will be passed to +func+.
 * If you need to pass more than one argument you should use a struct. When
 * the function is finished, release and destroy the lock.
 *
 * @param store     store to open the lock in
 * @param lock_name name of the lock to open
 * @param func      function to call with the lock locked
 * @param arg       argument to pass to the function
 * @raise FRT_IO_ERROR  if the lock is already locked
 * @see frt_with_lock
 */
extern void frt_with_lock_name(FrtStore *store, const char *lock_name,
                           void (*func)(void *arg), void *arg);

/**
 * Remove a reference to the store. If the reference count gets to zero free
 * all resources used by the store.
 *
 * @param store the store to be dereferenced
 */
extern void frt_store_deref(FrtStore *store);

/**
 * Flush the buffered contents of the FrtOutStream to the store.
 *
 * @param os the FrtOutStream to flush
 */
extern void frt_os_flush(FrtOutStream *os);

/**
 * Close the FrtOutStream after flushing the buffers, also freeing all allocated
 * resources.
 *
 * @param os the FrtOutStream to close
 */
extern void frt_os_close(FrtOutStream *os);

/**
 * Return the current position of FrtOutStream +os+.
 *
 * @param os the FrtOutStream to get the position from
 * @return the current position in FrtOutStream +os+
 */
extern off_t frt_os_pos(FrtOutStream *os);

/**
 * Set the current position in FrtOutStream +os+.
 *
 * @param os the FrtOutStream to set the position in
 * @param pos the new position in the FrtOutStream
 * @raise FRT_IO_ERROR if there is a file-system IO error seeking the file
 */
extern void frt_os_seek(FrtOutStream *os, off_t new_pos);

/**
 * Write a single byte +b+ to the FrtOutStream +os+
 *
 * @param os the FrtOutStream to write to @param b  the byte to write @raise
 * FRT_IO_ERROR if there is an IO error writing to the file-system
 */
extern void frt_os_write_byte(FrtOutStream *os, frt_uchar b);
/**
 * Write +len+ bytes from buffer +buf+ to the FrtOutStream +os+.
 *
 * @param os  the FrtOutStream to write to
 * @param len the number of bytes to write
 * @param buf the buffer from which to get the bytes to write.
 * @raise FRT_IO_ERROR if there is an IO error writing to the file-system
 */
extern void frt_os_write_bytes(FrtOutStream *os, const frt_uchar *buf, int len);

/**
 * Write a 32-bit signed integer to the FrtOutStream
 *
 * @param os FrtOutStream to write to
 * @param num the 32-bit signed integer to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_i32(FrtOutStream *os, frt_i32 num);

/**
 * Write a 64-bit signed integer to the FrtOutStream
 *
 *
 * @param os FrtOutStream to write to
 * @param num the 64-bit signed integer to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_i64(FrtOutStream *os, frt_i64 num);

/**
 * Write a 32-bit unsigned integer to the FrtOutStream
 *
 * @param os FrtOutStream to write to
 * @param num the 32-bit unsigned integer to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_u32(FrtOutStream *os, frt_u32 num);

/**
 * Write a 64-bit unsigned integer to the FrtOutStream
 *
 * @param os FrtOutStream to write to
 * @param num the 64-bit unsigned integer to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_u64(FrtOutStream *os, frt_u64 num);

/**
 * Write an unsigned integer to FrtOutStream in compressed VINT format.
 * TODO: describe VINT format
 *
 * @param os FrtOutStream to write to
 * @param num the integer to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_vint(FrtOutStream *os, register unsigned int num);

/**
 * Write an unsigned off_t to FrtOutStream in compressed VINT format.
 * TODO: describe VINT format
 *
 * @param os FrtOutStream to write to
 * @param num the off_t to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_voff_t(FrtOutStream *os, register off_t num);

/**
 * Write an unsigned 64bit int to FrtOutStream in compressed VINT format.
 * TODO: describe VINT format
 *
 * @param os FrtOutStream to write to
 * @param num the 64bit int to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_vll(FrtOutStream *os, register frt_u64 num);

/**
 * Write a string with known length to the FrtOutStream. A string is an
 * integer +length+ in VINT format (see frt_os_write_vint) followed by
 * +length+ bytes. The string can then be read using frt_is_read_string.
 *
 * @param os FrtOutStream to write to
 * @param str the string to write
 * @param len the length of the string to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern FRT_INLINE void frt_os_write_string_len(FrtOutStream *os,
                                               const char *str,
                                               int len);

/**
 * Write a string to the FrtOutStream. A string is an integer +length+ in VINT
 * format (see frt_os_write_vint) followed by +length+ bytes. The string can then
 * be read using frt_is_read_string.
 *
 * @param os FrtOutStream to write to
 * @param str the string to write
 * @raise FRT_IO_ERROR if there is an error writing to the file-system
 */
extern void frt_os_write_string(FrtOutStream *os, const char *str);

/**
 * Get the current position within an FrtInStream.
 *
 * @param is the FrtInStream to get the current position from
 * @return the current position within the FrtInStream +is+
 */
extern off_t frt_is_pos(FrtInStream *is);

/**
 * Set the current position in FrtInStream +is+ to +pos+.
 *
 * @param is the FrtInStream to set the current position in
 * @param pos the position in FrtInStream to seek
 * @raise FRT_IO_ERROR if there is a error seeking from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to seek past the end of the file
 */
extern void frt_is_seek(FrtInStream *is, off_t pos);

/**
 * Close the FrtInStream freeing all allocated resources.
 *
 * @param is the FrtInStream to close
 * @raise FRT_IO_ERROR if there is an error closing the associated file
 */
extern void frt_is_close(FrtInStream *is);

/**
 * Clone the FrtInStream allocating a new FrtInStream structure
 *
 * @param is the FrtInStream to clone
 * @return a newly allocated FrtInStream which is a clone of +is+
 */
extern FrtInStream *frt_is_clone(FrtInStream *is);

/**
 * Read a singly byte (unsigned char) from the FrtInStream +is+.
 *
 * @param is the Instream to read from
 * @return a single unsigned char read from the FrtInStream +is+
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern FRT_INLINE frt_uchar frt_is_read_byte(FrtInStream *is);

/**
 * Read +len+ bytes from FrtInStream +is+ and write them to buffer +buf+
 *
 * @param is     the FrtInStream to read from
 * @param buf    the buffer to read into, that is copy the bytes read to
 * @param len    the number of bytes to read
 * @return       the resultant buffer +buf+
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern frt_uchar *frt_is_read_bytes(FrtInStream *is, frt_uchar *buf, int len);

/**
 * Read a 32-bit unsigned integer from the FrtInStream.
 *
 * @param is the FrtInStream to read from
 * @return a 32-bit unsigned integer
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern frt_i32 frt_is_read_i32(FrtInStream *is);

/**
 * Read a 64-bit unsigned integer from the FrtInStream.
 *
 * @param is the FrtInStream to read from
 * @return a 64-bit unsigned integer
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern frt_i64 frt_is_read_i64(FrtInStream *is);

/**
 * Read a 32-bit signed integer from the FrtInStream.
 *
 * @param is the FrtInStream to read from
 * @return a 32-bit signed integer
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern frt_u32 frt_is_read_u32(FrtInStream *is);

/**
 * Read a 64-bit signed integer from the FrtInStream.
 *
 * @param is the FrtInStream to read from
 * @return a 64-bit signed integer
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern frt_u64 frt_is_read_u64(FrtInStream *is);

/**
 * Read a compressed (VINT) unsigned integer from the FrtInStream.
 * TODO: describe VINT format
 *
 * @param is the FrtInStream to read from
 * @return an int
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern FRT_INLINE unsigned int frt_is_read_vint(FrtInStream *is);

/**
 * Skip _cnt_ vints. This is a convenience method used for performance reasons
 * to skip large numbers of vints. It is mostly used by TermDocEnums. When
 * skipping positions os the proximity index file.
 *
 * @param is the FrtInStream to read from
 * @param cnt the number of vints to skip
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern FRT_INLINE void frt_is_skip_vints(FrtInStream *is, register int cnt);

/**
 * Read a compressed (VINT) unsigned off_t from the FrtInStream.
 * TODO: describe VINT format
 *
 * @param is the FrtInStream to read from
 * @return a off_t
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern FRT_INLINE off_t frt_is_read_voff_t(FrtInStream *is);

/**
 * Read a compressed (VINT) unsigned 64bit int from the FrtInStream.
 * TODO: describe VINT format
 *
 * @param is the FrtInStream to read from
 * @return a 64bit int
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern FRT_INLINE frt_u64 frt_is_read_vll(FrtInStream *is);

/**
 * Read a string from the FrtInStream. A string is an integer +length+ in vint
 * format (see frt_is_read_vint) followed by +length+ bytes. This is the format
 * used by frt_os_write_string.
 *
 * @param is the FrtInStream to read from
 * @return a null byte delimited string
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern char *frt_is_read_string(FrtInStream *is);

/**
 * Read a string from the FrtInStream. A string is an integer +length+ in vint
 * format (see frt_is_read_vint) followed by +length+ bytes. This is the format
 * used by frt_os_write_string. This method is similar to +frt_is_read_string+ except
 * that it will safely free all memory if there is an error reading the
 * string.
 *
 * @param is the FrtInStream to read from
 * @return a null byte delimited string
 * @raise FRT_IO_ERROR if there is a error reading from the file-system
 * @raise FRT_EOF_ERROR if there is an attempt to read past the end of the file
 */
extern char *frt_is_read_string_safe(FrtInStream *is);

/**
 * Copy cnt bytes from Instream _is_ to FrtOutStream _os_.
 *
 * @param is the FrtInStream to read from
 * @param os the FrtOutStream to write to
 * @raise FRT_IO_ERROR
 * @raise FRT_EOF_ERROR
 */
extern void frt_is2os_copy_bytes(FrtInStream *is, FrtOutStream *os, int cnt);

/**
 * Copy cnt vints from Instream _is_ to FrtOutStream _os_.
 *
 * @param is the FrtInStream to read from
 * @param os the FrtOutStream to write to
 * @raise FRT_IO_ERROR
 * @raise FRT_EOF_ERROR
 */
extern void frt_is2os_copy_vints(FrtInStream *is, FrtOutStream *os, int cnt);

/**
 * Print the filenames in a store to a buffer.
 *
 * @param store the store to get the filenames from
 */
extern char *frt_store_to_s(FrtStore *store);

extern FrtLock *frt_open_lock(FrtStore *store, const char *lockname);
extern void frt_close_lock(FrtLock *lock);

/* required by submodules
 * FIXME document. Perhaps include in different header?? */
extern FrtStore *frt_store_new();
extern void frt_store_destroy(FrtStore *store);
extern FrtOutStream *frt_os_new();
extern FrtInStream *frt_is_new();
extern int frt_file_is_lock(const char *filename);
extern bool frt_file_name_filter_is_index_file(const char *file_name,
                                               bool include_locks);

#endif

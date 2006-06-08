#ifndef FRT_STORE_H
#define FRT_STORE_H

#include "global.h"
#include "hash.h"
#include "threading.h"

#define BUFFER_SIZE 1024
#define LOCK_PREFIX "ferret-"
#define LOCK_EXT ".lck"

#define VINT_MAX_LEN 10
#define VINT_END BUFFER_SIZE - VINT_MAX_LEN

typedef struct Buffer
{
    uchar buf[BUFFER_SIZE];
    long  start;
    long  pos;
    long  len;
} Buffer;

typedef struct OutStream OutStream;
struct OutStreamMethods {
    /* internal functions for the InStream */
    /**
     * Flush +len+ characters from +src+ to the output stream +os+
     *
     * @param os self
     * @param src the characters to write to the output stream
     * @param len the number of characters to write
     * @raise IO_ERROR if there is an error writing the characters
     */
    void (*flush_i)(struct OutStream *os, uchar *buf, int len);

    /**
     * Seek +pos+ in the output stream
     *
     * @param os self
     * @param pos the position to seek in the stream
     * @raise IO_ERROR if there is an error seeking in the output stream
     */
    void (*seek_i)(struct OutStream *os, long pos);

    /**
     * Close any resources used by the output stream +os+
     *
     * @param os self
     * @raise IO_ERROR if there is an closing the file
     */
    void (*close_i)(struct OutStream *os);
};

struct OutStream
{
    Buffer buf;
    void  *file;
    long   pointer;             /* only used by RAMOut */
    const struct OutStreamMethods *m;
};

typedef struct CompoundInStream CompoundInStream;

typedef struct InStream InStream;

struct InStreamMethods
{
    /**
     * Read +len+ characters from the input stream into the +offset+ position in
     * +buf+, an array of unsigned characters.
     *
     * @param is self
     * @param buf an array of characters which must be allocated with  at least
     *          +offset+ + +len+ bytes
     * @param len the number of bytes to read
     * @raise IO_ERROR if there is an error reading from the input stream
     */
    void (*read_i)(struct InStream *is, uchar *buf, int len);

    /**
     * Seek position +pos+ in input stream +is+
     *
     * @param is self
     * @param pos the position to seek
     * @raise IO_ERROR if the seek fails
     */
    void (*seek_i)(struct InStream *is, long pos);

    /**
     * Returns the length of the input stream +is+
     *
     * @param is self
     * @raise IO_ERROR if there is an error getting the file length
     */
    long (*length_i)(struct InStream *is);

    /**
     * Do internal cloning processing for this particular type of InStream.
     *
     * @param is self
     * @param new_is the newly allocated clone.
     */
    void (*clone_i)(struct InStream *is, struct InStream *new_is);
    
    /**
     * Close the resources allocated to the inputstream +is+
     *
     * @param is self
     * @raise IO_ERROR if the close fails
     */
    void (*close_i)(struct InStream *is);
};

struct InStream
{
    int is_clone;
    Buffer buf;
    union
    {
        int fd;
        void *p;
    } file;
    union
    {
        long pointer;           /* only used by RAMIn */
        char *path;             /* only used by FSIn */
        CompoundInStream *cis;
    } d;
    const struct InStreamMethods *m;
};

struct CompoundInStream
{
    InStream *sub;
    long offset;
    long length;
};

#define is_length(mis) mis->m->length_i(mis)

typedef struct Store Store;
typedef struct Lock Lock;
struct Lock
{
    char *name;
    Store *store;
    int (*obtain)(Lock *lock);
    int (*is_locked)(Lock *lock);
    void (*release)(Lock *lock);
};

typedef struct CompoundStore
{
    Store *store;
    const char *name;
    HashTable *entries;
    InStream *stream;
} CompoundStore;

struct Store
{
    int ref_cnt;                /* for fs_store only */
    mutex_t mutex_i;            /* for internal use only */
    mutex_t mutex;              /* external mutex for use outside */
    union
    {
        char *path;             /* for fs_store only */
        HashTable *ht;          /* for ram_store only */
        CompoundStore *cmpd;    /* for compound_store only */
    } dir;

    /**
     * Create the file +filename+ in the +store+.
     *
     * @param store self
     * @param filename the name of the file to create
     * @raise IO_ERROR if the file cannot be created
     */
    void (*touch)(Store *store, char *filename);

    /**
     * Return true if a file of name +filename+ exists in +store+.
     *
     * @param store self
     * @param filename the name of the file to check for
     * @returns true if the file exists
     * @raise IO_ERROR if there is an error checking for the files existance
     */
    int (*exists)(Store *store, char *filename);

    /**
     * Remove the file +filename+ from the +store+
     *
     * @param store self
     * @param filename the name of the file to remove
     * @returns On success, zero is returned.  On error, -1 is returned, and errno
     *          is set appropriately.
     */
    int (*remove)(Store *store, char *filename);

    /**
     * Rename the file in the +store+ from the name +from+ to the name +to+.
     *
     * @param store self
     * @param from the name of the file to rename
     * @param to the new name of the file
     * @raise IO_ERROR if there is an error renaming the file
     */
    void (*rename)(Store *store, char *from, char *to);

    /**
     * Returns the number of files in the store.
     *
     * @param store self
     * @return the number of files in the store
     * @raise IO_ERROR if there is an error opening the directory
     */
    int (*count)(Store *store);

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
     * @raise IO_ERROR if there is an error opening the directory
     */
    void (*each)(Store *store, void (*func)(char *fname, void *arg),
                  void *arg);

    /**
     * Clear all the locks in the store.
     *
     * @param store self
     * @raise IO_ERROR if there is an error opening the directory
     */
    void (*clear_locks)(Store *store);

    /**
     * Clear all files from the store except the lock files.
     *
     * @param store self
     * @raise IO_ERROR if there is an error deleting the files
     */
    void (*clear)(Store *store);

    /**
     * Clear all files from the store including the lock files.
     *
     * @param store self
     * @raise IO_ERROR if there is an error deleting the files
     */
    void (*clear_all)(Store *store);

    /**
     * Return the length of the file +filename+ in +store+
     *
     * @param store self
     * @param the name of the file to check the length of
     * @return the length of the file in bytes
     * @raise IO_ERROR if there is an error checking the file length
     */
    long (*length)(Store *store, char *filename);

    /**
     * Allocate the resources needed for the output stream in the +store+ with
     * the name +filename+
     *
     * @param store self
     * @param filename the name of the output stream
     * @return a newly allocated filestream
     * @raise IO_ERROR if there is an error opening the output stream
     *   resources
     */
    OutStream *(*create_output)(Store *store, const char *filename);

    /**
     * Open an input stream in the +store+ with the name +filename+
     *
     * @param store self
     * @param filename the name of the input stream
     * @raise IO_ERROR if the input stream cannot be opened
     */
    InStream *(*open_input)(Store *store, const char *filename);

    /**
     * Obtain a lock on the lock +lock+
     *
     * @param store self
     * @param lock the lock to obtain
     */
    Lock *(*open_lock)(Store *store, char *lockname);

    /**
     * Returns true if +lock+ is locked. To test if the file is locked:wq
     *
     * @param lock the lock to test
     * @raise IO_ERROR if there is an error detecting the lock status
     */
    void (*close_lock)(Lock *lock);

    /**
     * Internal function to close the store freeing implementation specific
     * resources.
     *
     * @param store self
     */
    void (*close_i)(Store *store);
};

/**
 * Print the store in a human readable format. Basically this will print all
 * files listed in the store. It is mainly used for debugging purposes
 */
extern char *store_to_s(Store *store);

/**
 * Create a newly allocated file-system Store at the pathname designated. The
 * pathname must be the name of an existing directory.
 *
 * @param pathname the pathname of the directory to be used by the index
 * @return a newly allocated file-system Store.
 */
extern Store *open_fs_store(const char *pathname);

/**
 * Create a newly allocated in-memory or RAM Store.
 *
 * @return a newly allocated RAM Store.
 */
extern Store *open_ram_store();

/**
 * Create a newly allocated in-memory or RAM Store. Copy the contents of
 * another store into this store. Then close the other store if required. This
 * method would be used for example to read an index into memory for faster
 * searching.
 *
 * @param store the whose contents will be copied into the newly allocated RAM
 *   store
 * @param close_store close the store whose contents where copied
 * @return a newly allocated RAM Store.
 */
extern Store *open_ram_store_and_copy(Store *store, bool close_store);

/**
 * Open a compound store. This is basically store which is stored within a
 * single file and can in turn be stored within either a FileSystem or RAM
 * store.
 *
 * @param store the store within which this compound store will be stored
 * @param filename the name of the file in which to store the compound store
 * @return a newly allocated Compound Store.
 */
extern Store *open_cmpd_store(Store *store, const char *filename);

/* 
 * == RamStore functions ==
 *
 * These functions or optimizations to be used when you know you are using a
 * Ram OutStream.
 */

/**
 * Return the length of the OutStream in bytes.
 *
 * @param os the OutStream who's length you want
 * @return the length of +os+ in bytes
 */
extern long ramo_length(OutStream *os);

/**
 * Reset the OutStream removing any data written to it. Since it is a RAM
 * file, all that needs to be done is set the length to 0.
 *
 * @param os the OutStream to reset
 */
extern void ramo_reset(OutStream *os);

/**
 * Write the contents of a RAM OutStream to another OutStream.
 *
 * @param from_os the OutStream to write from
 * @param to_os the OutStream to write to
 */
extern void ramo_write_to(OutStream *from_os, OutStream *to_os);

/**
 * Create a buffer RAM OutStream which is unassociated with any RAM Store.
 * This OutStream can be used to write temporary data too. When the time
 * comes, this data can be written to another OutStream (which might possibly
 * be a file-system OutStream) using ramo_write_to.
 *
 * @return A newly allocated RAM OutStream
 */
extern OutStream *ram_create_buffer();

/**
 * Destroy a RAM OutStream which is unassociated with any RAM Store, freeing
 * all resources allocated to it.
 *
 * @param os the OutStream to destroy
 */
extern void ram_destroy_buffer(OutStream *os);

/**
 * Call the function +func+ with the +lock+ locked. The argument +arg+ will be
 * passed to +func+. If you need to pass more than one argument you should use
 * a struct. When the function is finished, release the lock.
 * 
 * @param lock     lock to be locked while func is called
 * @param func     function to call with the lock locked
 * @param arg      argument to pass to the function
 * @raise IO_ERROR if the lock is already locked
 * @see with_lock_name
 */
extern void with_lock(Lock *lock, void (*func)(void *arg), void *arg);

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
 * @raise IO_ERROR  if the lock is already locked
 * @see with_lock
 */
extern void with_lock_name(Store *store, char *lock_name,
                           void (*func)(void *arg), void *arg);

/**
 * Remove a reference to the store. If the reference count gets to zero free
 * all resources used by the store.
 *
 * @param store the store to be dereferenced
 */
extern void store_deref(Store *store);

/**
 * Flush the buffered contents of the OutStream to the store.
 *
 * @param os the OutStream to flush
 */
extern void os_flush(OutStream *os);

/**
 * Close the OutStream after flushing the buffers, also freeing all allocated
 * resources.
 *
 * @param os the OutStream to close
 */
extern void os_close(OutStream *os);

/**
 * Return the current position of OutStream +os+. 
 *
 * @param os the OutStream to get the position from
 * @return the current position in OutStream +os+
 */
extern long os_pos(OutStream *os);

/**
 * Set the current position in OutStream +os+.
 *
 * @param os the OutStream to set the position in
 * @param pos the new position in the OutStream
 * @raise IO_ERROR if there is a file-system IO error seeking the file
 */
extern void os_seek(OutStream *os, long new_pos);

/**
 * Write a single byte +b+ to the OutStream +os+
 *
 * @param os the OutStream to write to @param b  the byte to write @raise
 * IO_ERROR if there is an IO error writing to the file-system
 */
extern void os_write_byte(OutStream *os, uchar b);
/**
 * Write +len+ bytes from buffer +buf+ to the OutStream +os+.
 *
 * @param os  the OutStream to write to
 * @param len the number of bytes to write
 * @param buf the buffer from which to get the bytes to write.
 * @raise IO_ERROR if there is an IO error writing to the file-system
 */
extern void os_write_bytes(OutStream *os, uchar *buf, int len);

/**
 * Write a 32-bit signed integer to the OutStream
 *
 * @param os OutStream to write to
 * @param num the 32-bit signed integer to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_int(OutStream *os, f_i32 num);

/**
 * Write a 64-bit signed integer to the OutStream
 *
 *
 * @param os OutStream to write to
 * @param num the 64-bit signed integer to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_long(OutStream *os, f_i64 num);

/**
 * Write a 32-bit unsigned integer to the OutStream
 *
 * @param os OutStream to write to
 * @param num the 32-bit unsigned integer to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_uint(OutStream *os, f_u32 num);

/**
 * Write a 64-bit unsigned integer to the OutStream
 *
 * @param os OutStream to write to
 * @param num the 64-bit unsigned integer to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_ulong(OutStream *os, f_u64 num);

/**
 * Write a 32-bit unsigned integer to OutStream in compressed VINT format.
 * TODO: describe VINT format
 *
 * @param os OutStream to write to
 * @param num the 32-bit unsigned integer to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_vint(OutStream *os, register f_u32 num);

/**
 * Write a 64-bit unsigned integer to OutStream in compressed VINT format.
 * TODO: describe VINT format
 *
 * @param os OutStream to write to
 * @param num the 64-bit unsigned integer to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_vlong(OutStream *os, register f_u64 num);

/**
 * Write a string to the OutStream. A string is an integer +length+ in VINT
 * format (see os_write_vint) followed by +length+ bytes. The string can then
 * be read using is_read_string.
 *
 * @param os OutStream to write to
 * @param str the string to write
 * @raise IO_ERROR if there is an error writing to the file-system
 */
extern void os_write_string(OutStream *os, char *str);
/**
 * Get the current position within an InStream.
 *
 * @param is the InStream to get the current position from
 * @return the current position within the InStream +is+
 */
extern long is_pos(InStream *is);

/**
 * Set the current position in InStream +is+ to +pos+.
 *
 * @param is the InStream to set the current position in
 * @param pos the position in InStream to seek
 * @raise IO_ERROR if there is a error seeking from the file-system
 * @raise EOF_ERROR if there is an attempt to seek past the end of the file
 */
extern void is_seek(InStream *is, long pos);

/**
 * Close the InStream freeing all allocated resources.
 *
 * @param is the InStream to close
 * @raise IO_ERROR if there is an error closing the associated file
 */
extern void is_close(InStream *is);

/**
 * Clone the InStream allocating a new InStream structure
 *
 * @param is the InStream to clone
 * @return a newly allocated InStream which is a clone of +is+
 */
extern InStream *is_clone(InStream *is);

/**
 * Read a singly byte (unsigned char) from the InStream +is+.
 *
 * @param is the Instream to read from
 * @return a single unsigned char read from the InStream +is+
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern inline uchar is_read_byte(InStream *is);

/**
 * Read +len+ bytes from InStream +is+ and write them to buffer +buf+ starting
 * from the position +offset+.
 *
 * @param is     the InStream to read from
 * @param buf    the buffer to read into, that is copy the bytes read to
 * @param offset the offset in the buffer to start writing the read bytes
 * @param len    the number of bytes to read
 * @return       the resultant buffer +buf+
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern uchar *is_read_bytes(InStream *is, uchar *buf, int offset, int len);

/**
 * Read a 32-bit unsigned integer from the InStream.
 *
 * @param is the InStream to read from
 * @return a 32-bit unsigned integer
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern f_i32 is_read_int(InStream *is);

/**
 * Read a 64-bit unsigned integer from the InStream.
 *
 * @param is the InStream to read from
 * @return a 64-bit unsigned integer
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern f_i64 is_read_long(InStream *is);

/**
 * Read a 32-bit signed integer from the InStream.
 *
 * @param is the InStream to read from
 * @return a 32-bit signed integer
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern f_u32 is_read_uint(InStream *is);

/**
 * Read a 64-bit signed integer from the InStream.
 *
 * @param is the InStream to read from
 * @return a 64-bit signed integer
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern f_u64 is_read_ulong(InStream *is);

/**
 * Read a compression (VINT) 32-bit unsigned integer from the InStream.
 * TODO: describe VINT format
 *
 * @param is the InStream to read from
 * @return a 32-bit unsigned integer
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern inline f_u32 is_read_vint(InStream *is);

/**
 * Read a compressed (VINT) 64-bit unsigned integer from the InStream.
 * TODO: describe VINT format
 *
 * @param is the InStream to read from
 * @return a 64-bit unsigned integer
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern inline f_u64 is_read_vlong(InStream *is);

/**
 * Read a string from the InStream. A string is an integer +length+ in vint
 * format (see is_read_vint) followed by +length+ bytes. This is the format
 * used by os_write_string.
 *
 * @param is the InStream to read from
 * @return a null byte delimited string
 * @raise IO_ERROR if there is a error reading from the file-system
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
extern char *is_read_string(InStream *is);

#endif

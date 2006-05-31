#include "store.h"
#include <string.h>

#define VINT_MAX_LEN 10
#define VINT_END BUFFER_SIZE - VINT_MAX_LEN

/*
 * TODO: add try finally
 */
void with_lock(Lock *lock, void (*func)(void *arg), void *arg)
{
    if (!lock->obtain(lock)) {
        RAISE(IO_ERROR, "couldn't obtain lock \"%s\"", lock->name);
    }
    func(arg);
    lock->release(lock);
}

/*
 * TODO: add try finally
 */
void with_lock_name(Store *store, char *lock_name,
                    void (*func)(void *arg), void *arg)
{
    Lock *lock = store->open_lock(store, lock_name);
    if (!lock->obtain(lock)) {
        RAISE(IO_ERROR, "couldn't obtain lock \"%s\"", lock->name);
    }
    func(arg);
    lock->release(lock);
    store->close_lock(lock);
}

void store_deref(Store *store)
{
    mutex_lock(&store->mutex_i);
    if (--store->ref_cnt == 0) {
        store->close_i(store);
    }
    else {
        mutex_unlock(&store->mutex_i);
    }
}

/**
 * Create a store struct initializing the mutex.
 */
Store *store_create()
{
    Store *store = ALLOC(Store);
    store->ref_cnt = 1;
    mutex_init(&store->mutex_i, NULL);
    mutex_init(&store->mutex, NULL);
    return store;
}

/**
 * Destroy the store freeing allocated resources
 *
 * @param store the store struct to free
 */
void store_destroy(Store *store)
{
    mutex_destroy(&store->mutex_i);
    mutex_destroy(&store->mutex);
    free(store);
}

/**
 * Create a newly allocated and initialized OutStream object
 *
 * @return a newly allocated and initialized OutStream object
 */
OutStream *os_create()
{
    OutStream *os = ALLOC(OutStream);
    os->buf.start = 0;
    os->buf.pos = 0;
    os->buf.len = 0;
    return os;
}

/**
 * Flush the countents of the OutStream's buffers
 *
 * @param the OutStream to flush
 */
inline void os_flush(OutStream *os)
{
    os->flush_i(os, os->buf.buf, os->buf.pos);
    os->buf.start += os->buf.pos;
    os->buf.pos = 0;
}

void os_close(OutStream *os)
{
    os_flush(os);
    os->close_i(os);
    free(os);
}

long os_pos(OutStream *os)
{
    return os->buf.start + os->buf.pos;
}

void os_seek(OutStream *os, long new_pos)
{
    os_flush(os);
    os->buf.start = new_pos;
    os->seek_i(os, new_pos);
}

/**
 * Unsafe alternative to os_write_byte. Only use this method if you know there
 * is no chance of buffer overflow.
 */
#define write_byte(os, b) os->buf.buf[os->buf.pos++] = (uchar)b

/**
 * Write a single byte +b+ to the OutStream +os+
 *
 * @param os the OutStream to write to
 * @param b  the byte to write
 * @raise IO_ERROR if there is an IO error writing to the filesystem
 */
inline void os_write_byte(OutStream *os, uchar b)
{
    if (os->buf.pos >= BUFFER_SIZE) {
        os_flush(os);
    }
    write_byte(os, b);
}

void os_write_bytes(OutStream *os, uchar *buf, int len)
{
    if (os->buf.pos > 0) {      /* flush buffer */
        os_flush(os);
    }

    if (len < BUFFER_SIZE) {
        os->flush_i(os, buf, len);
        os->buf.start += len;
    }
    else {
        int pos = 0;
        int size;
        while (pos < len) {
            if (len - pos < BUFFER_SIZE) {
                size = len - pos;
            }
            else {
                size = BUFFER_SIZE;
            }
            os->flush_i(os, buf + pos, size);
            pos += size;
            os->buf.start += size;
        }
    }
}

/**
 * Create a newly allocated and initialized InStream
 *
 * @return a newly allocated and initialized InStream
 */
InStream *is_create()
{
    InStream *is = ALLOC(InStream);
    is->buf.start = 0;
    is->buf.pos = 0;
    is->buf.len = 0;
    return is;
}

/**
 * Refill the InStream's buffer from the store source (filesystem or memory).
 *
 * @param is the InStream to refill
 * @raise IO_ERROR if there is a error reading from the filesystem
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
void is_refill(InStream *is)
{
    long start = is->buf.start + is->buf.pos;
    long last = start + BUFFER_SIZE;
    long flen = is->length_i(is);

    if (last > flen) {          /* don't read past EOF */
        last = flen;
    }

    is->buf.len = last - start;
    if (is->buf.len <= 0) {
        RAISE(EOF_ERROR, "current pos = %ld, file length = %ld", start, flen);
    }

    is->read_i(is, is->buf.buf, is->buf.len);

    is->buf.start = start;
    is->buf.pos = 0;
}

/**
 * Unsafe alternative to is_read_byte. Only use this method when you know
 * there is no chance that you will read past the end of the InStream's
 * buffer.
 */
#define read_byte(is) is->buf.buf[is->buf.pos++]

/**
 * Read a singly byte (unsigned char) from the InStream +is+.
 *
 * @param is the Instream to read from
 * @return a single unsigned char read from the InStream +is+
 * @raise IO_ERROR if there is a error reading from the filesystem
 * @raise EOF_ERROR if there is an attempt to read past the end of the file
 */
inline uchar is_read_byte(InStream *is)
{
    if (is->buf.pos >= is->buf.len) {
        is_refill(is);
    }

    return read_byte(is);
}

long is_pos(InStream *is)
{
    return is->buf.start + is->buf.pos;
}

uchar *is_read_bytes(InStream *is, uchar *buf, int offset, int len)
{
    int i;
    long start;

    if ((offset + len) < BUFFER_SIZE) {
        for (i = offset; i < offset + len; i++) {
            buf[i] = is_read_byte(is);
        }
    }
    else {                              /* read all-at-once */
        start = is_pos(is);
        is->seek_i(is, start);
        is->read_i(is, buf + offset, len);

        is->buf.start = start + len;    /* adjust stream variables */
        is->buf.pos = 0;
        is->buf.len = 0;                /* trigger refill on read */
    }
    return buf;
}

void is_seek(InStream *is, long pos)
{
    if (pos >= is->buf.start && pos < (is->buf.start + is->buf.len)) {
        is->buf.pos = pos - is->buf.start;  /* seek within buffer */
    }
    else {
        is->buf.start = pos;
        is->buf.pos = 0;
        is->buf.len = 0;                    /* trigger refill() on read() */
        is->seek_i(is, pos);
    }
}

void is_close(InStream *is)
{
    is->close_i(is);
    free(is);
}

InStream *is_clone(InStream *is)
{
    InStream *new_index_i = ALLOC(InStream);
    memcpy(new_index_i, is, sizeof(InStream));
    new_index_i->is_clone = true;
    is->clone_i(is, new_index_i);
    return new_index_i;
}

f_i32 is_read_int(InStream *is)
{
    return ((f_i32)is_read_byte(is) << 24) |
        ((f_i32)is_read_byte(is) << 16) |
        ((f_i32)is_read_byte(is) << 8) |
        ((f_i32)is_read_byte(is));
}

f_i64 is_read_long(InStream *is)
{
    return ((f_i64)is_read_byte(is) << 56) |
        ((f_i64)is_read_byte(is) << 48) |
        ((f_i64)is_read_byte(is) << 40) |
        ((f_i64)is_read_byte(is) << 32) |
        ((f_i64)is_read_byte(is) << 24) |
        ((f_i64)is_read_byte(is) << 16) |
        ((f_i64)is_read_byte(is) << 8) |
        ((f_i64)is_read_byte(is));
}

f_u32 is_read_uint(InStream *is)
{
    return ((f_u32)is_read_byte(is) << 24) |
        ((f_u32)is_read_byte(is) << 16) |
        ((f_u32)is_read_byte(is) << 8) |
        ((f_u32)is_read_byte(is));
}

f_u64 is_read_ulong(InStream *is)
{
    return ((f_u64)is_read_byte(is) << 56) |
        ((f_u64)is_read_byte(is) << 48) |
        ((f_u64)is_read_byte(is) << 40) |
        ((f_u64)is_read_byte(is) << 32) |
        ((f_u64)is_read_byte(is) << 24) |
        ((f_u64)is_read_byte(is) << 16) |
        ((f_u64)is_read_byte(is) << 8) |
        ((f_u64)is_read_byte(is));
}

/* optimized to use unchecked read_byte if there is definitely space */
inline f_u32 is_read_vint(InStream *is)
{
    register f_u32 res, b;
    register int shift = 7;

    if (is->buf.pos > (is->buf.len - VINT_MAX_LEN)) {
        b = is_read_byte(is);
        res = b & 0x7F;                 /* 0x7F = 0b01111111 */

        while ((b & 0x80) != 0) {       /* 0x80 = 0b10000000 */
            b = is_read_byte(is);
            res |= (b & 0x7F) << shift;
            shift += 7;
        }
    }
    else {                              /* unchecked optimization */
        b = read_byte(is);
        res = b & 0x7F;                 /* 0x7F = 0b01111111 */

        while ((b & 0x80) != 0) {       /* 0x80 = 0b10000000 */
            b = read_byte(is);
            res |= (b & 0x7F) << shift;
            shift += 7;
        }
    }

    return res;
}

/* optimized to use unchecked read_byte if there is definitely space */
inline f_u64 is_read_vlong(InStream *is)
{
    register f_u64 res, b;
    register int shift = 7;

    if (is->buf.pos > (is->buf.len - VINT_MAX_LEN)) {
        b = is_read_byte(is);
        res = b & 0x7F;                 /* 0x7F = 0b01111111 */

        while ((b & 0x80) != 0) {       /* 0x80 = 0b10000000 */
            b = is_read_byte(is);
            res |= (b & 0x7F) << shift;
            shift += 7;
        }
    }
    else {                              /* unchecked optimization */
        b = read_byte(is);
        res = b & 0x7F;                 /* 0x7F = 0b01111111 */

        while ((b & 0x80) != 0) {       /* 0x80 = 0b10000000 */
            b = read_byte(is);
            res |= (b & 0x7F) << shift;
            shift += 7;
        }
    }

    return res;
}

inline void is_read_chars(InStream *is, char *buffer,
                                  int off, int len)
{
    int end, i;

    end = off + len;

    for (i = off; i < end; i++) {
        buffer[i] = is_read_byte(is);
    }
}

char *is_read_string(InStream *is)
{
    register int length = (int) is_read_vint(is);
    char *str = ALLOC_N(char, length + 1);
    str[length] = '\0';

    if (is->buf.pos > (is->buf.len - length)) {
        register int i;
        for (i = 0; i < length; i++) {
            str[i] = is_read_byte(is);
        }
    }
    else {                      /* unchecked optimization */
        memcpy(str, is->buf.buf + is->buf.pos, length);
        is->buf.pos += length;
    }

    return str;
}

void os_write_int(OutStream *os, f_i32 num)
{
    os_write_byte(os, (uchar)((num >> 24) & 0xFF));
    os_write_byte(os, (uchar)((num >> 16) & 0xFF));
    os_write_byte(os, (uchar)((num >> 8) & 0xFF));
    os_write_byte(os, (uchar)(num & 0xFF));
}

void os_write_long(OutStream *os, f_i64 num)
{
    os_write_byte(os, (uchar)((num >> 56) & 0xFF));
    os_write_byte(os, (uchar)((num >> 48) & 0xFF));
    os_write_byte(os, (uchar)((num >> 40) & 0xFF));
    os_write_byte(os, (uchar)((num >> 32) & 0xFF));
    os_write_byte(os, (uchar)((num >> 24) & 0xFF));
    os_write_byte(os, (uchar)((num >> 16) & 0xFF));
    os_write_byte(os, (uchar)((num >> 8) & 0xFF));
    os_write_byte(os, (uchar)(num & 0xFF));
}

void os_write_uint(OutStream *os, f_u32 num)
{
    os_write_byte(os, (uchar)((num >> 24) & 0xFF));
    os_write_byte(os, (uchar)((num >> 16) & 0xFF));
    os_write_byte(os, (uchar)((num >> 8) & 0xFF));
    os_write_byte(os, (uchar)(num & 0xFF));
}

void os_write_ulong(OutStream *os, f_u64 num)
{
    os_write_byte(os, (uchar)((num >> 56) & 0xFF));
    os_write_byte(os, (uchar)((num >> 48) & 0xFF));
    os_write_byte(os, (uchar)((num >> 40) & 0xFF));
    os_write_byte(os, (uchar)((num >> 32) & 0xFF));
    os_write_byte(os, (uchar)((num >> 24) & 0xFF));
    os_write_byte(os, (uchar)((num >> 16) & 0xFF));
    os_write_byte(os, (uchar)((num >> 8) & 0xFF));
    os_write_byte(os, (uchar)(num & 0xFF));
}

/* optimized to use an unchecked write if there is space */
inline void os_write_vint(OutStream *os, register f_u32 num)
{
    if (os->buf.pos > VINT_END) {
        while (num > 127) {
            os_write_byte(os, (uchar)((num & 0x7f) | 0x80));
            num >>= 7;
        }
        os_write_byte(os, (uchar)(num));
    }
    else {
        while (num > 127) {
            write_byte(os, (uchar)((num & 0x7f) | 0x80));
            num >>= 7;
        }
        write_byte(os, (uchar)(num));
    }
}

/* optimized to use an unchecked write if there is space */
inline void os_write_vlong(OutStream *os, register f_u64 num)
{
    if (os->buf.pos > VINT_END) {
        while (num > 127) {
            os_write_byte(os, (uchar)((num & 0x7f) | 0x80));
            num >>= 7;
        }
        os_write_byte(os, (uchar)num);
    }
    else {
        while (num > 127) {
            write_byte(os, (uchar)((num & 0x7f) | 0x80));
            num >>= 7;
        }
        write_byte(os, (uchar)num);
    }
}

void os_write_string(OutStream *os, char *str)
{
    int len = (int) strlen(str);
    os_write_vint(os, len);

    os_write_bytes(os, (uchar *)str, len);
}

/**
 * Determine if the filename is the name of a lock file. Return 1 if it is, 0
 * otherwise.
 *
 * @param filename the name of the file to check
 * @return 1 (true) if the file is a lock file, 0 (false) otherwise
 */
int file_is_lock(char *filename)
{
    int start = (int) strlen(filename) - 4;
    return ((start > 0) && (strcmp(LOCK_EXT, &filename[start]) == 0));
}

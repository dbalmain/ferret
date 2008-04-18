#include <string.h>
#include "benchmark.h"
#include "store.h"

#define N 1
#define write_byte(os, b) os->buf.buf[os->buf.pos++] = (uchar)b

INLINE void my_os_write_voff_t(OutStream *os, register off_t num)
{
    if (!(num&0x7f)) {
        if (os->buf.pos >= BUFFER_SIZE) {
            os_write_byte(os, (uchar)num);
        }
        else {
            write_byte(os, (uchar)num);
        }
    }
    else if (!(num&0x3fff)) {
        if (os->buf.pos >= BUFFER_SIZE - 1) {
            os_write_byte(os, (uchar)(0x80 | (0x3f & num))); num >>= 6;
            os_write_byte(os, (uchar)num);
        }
        else {
            write_byte(os, (uchar)(0x80 | (0x3f & num))); num >>= 6;
            write_byte(os, (uchar)num);
        }
    }
    else if (!(num&0x1fffff)) {
        if (os->buf.pos >= BUFFER_SIZE - 2) {
            os_write_byte(os, (uchar)(0xc0 | (0x1f & num))); num >>= 5;
            os_write_byte(os, (uchar)(0xff| num)); num >>= 8;
            os_write_byte(os, (uchar)num);
        }
        else {
            write_byte(os, (uchar)(0xc0 | (0x1f & num))); num >>= 5;
            write_byte(os, (uchar)(0xff| num)); num >>= 8;
            write_byte(os, (uchar)num);
        }
    }
    else if (!(num&0xfffff)) {
        if (os->buf.pos >= BUFFER_SIZE - 3) {
            os_write_byte(os, (uchar)(0xe0 | (0x0f & num))); num >>= 4;
            os_write_byte(os, (uchar)(0xff | num)); num >>= 8;
            os_write_byte(os, (uchar)(0xff | num)); num >>= 8;
            os_write_byte(os, (uchar)num);
        }
        else {
            write_byte(os, (uchar)(0xe0 | (0x0f & num))); num >>= 4;
            write_byte(os, (uchar)(0xff | num)); num >>= 8;
            write_byte(os, (uchar)(0xff | num)); num >>= 8;
            write_byte(os, (uchar)num);
        }
    }
}

static void vint_out()
{
    int n;
    off_t i;
    OutStream *os;

    for (n = 0; n < N; n++) {
        os = ram_new_buffer();
        for (i = 0; i < 10000000; i++) {
            os_write_voff_t(os, i);
        }
        ram_destroy_buffer(os);
    }

}

static void unrolled_vint_out()
{
    int n;
    off_t i;
    OutStream *os;

    for (n = 0; n < N; n++) {
        os = ram_new_buffer();
        for (i = 0; i < 10000000; i++) {
            os_write_voff_t(os, i);
        }
        ram_destroy_buffer(os);
    }

}

BENCH(vint_io)
{
    BM_ADD(vint_out);
    BM_ADD(unrolled_vint_out);
    //BM_ADD(high_bit_vint_in);
}

#include "ferret.h"

ID frt_length, frt_flush_buffer, frt_read_internal, frt_seek_internal; 

/****************************************************************************
 *
 * BufferIndexInput Methods
 *
 ****************************************************************************/

void
frt_indexbuffer_free(void *p)
{
  IndexBuffer *my_buf = (IndexBuffer *)p;
  free((void *)my_buf->buffer);
  free(p);
}

static VALUE
frt_indexbuffer_alloc(VALUE klass)
{
  byte_t *buffer;
  IndexBuffer *my_buf;

  my_buf = (IndexBuffer *)ALLOC(IndexBuffer);
  buffer = (byte_t *)ALLOC_N(byte_t, BUFFER_SIZE);

  my_buf->start = 0;
  my_buf->pos = 0;
  my_buf->len = 0;
  my_buf->buffer = buffer;

  return Data_Wrap_Struct(klass, NULL, frt_indexbuffer_free, my_buf);
}

static VALUE
frt_indexin_init_copy(VALUE self, VALUE orig)
{
  IndexBuffer *orig_buf;
  IndexBuffer *my_buf;
  int len;
  if (self == orig)
    return self;

  Data_Get_Struct(self, IndexBuffer, my_buf);
  Data_Get_Struct(orig, IndexBuffer, orig_buf);

  len = orig_buf->len;
  my_buf->len = len;
  my_buf->pos = orig_buf->pos;
  my_buf->len = orig_buf->len;
  my_buf->start = orig_buf->start;

  MEMCPY(my_buf->buffer, orig_buf->buffer, byte_t, len);

  return self;
}

static VALUE
frt_indexin_refill(VALUE self)
{
  IndexBuffer *my_buf;
  long start;
  int stop, len_to_read;
  int input_len = FIX2INT(rb_funcall(self, frt_length, 0, NULL));

  Data_Get_Struct(self, IndexBuffer, my_buf);

  start = my_buf->start + my_buf->pos;
  stop = start + BUFFER_SIZE;
  if (stop > input_len) {
    stop = input_len;
  }

  len_to_read = stop - start;
  if (len_to_read <= 0) {
    rb_raise(rb_eEOFError, "IndexInput: Read past End of File");
  }

  VALUE rStr = rb_str_new(my_buf->buffer, BUFFER_SIZE);
  rb_funcall(self, frt_read_internal, 3,
      rStr, INT2FIX(0), INT2FIX(len_to_read));

  memcpy(my_buf->buffer, RSTRING(rStr)->ptr, BUFFER_SIZE);
  //my_buf->buffer = StringValuePtr(rStr);

  my_buf->len = len_to_read;
  my_buf->start = start;
  my_buf->pos = 0;

  return Qnil;
}

byte_t
read_byte(VALUE self, IndexBuffer *my_buf)
{
  if (my_buf->pos >= my_buf->len)
    frt_indexin_refill(self);

  byte_t res = my_buf->buffer[my_buf->pos++];
  return res;
}

byte_t
frt_read_byte(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  return read_byte(self, my_buf);
}

static VALUE
frt_indexin_read_byte(VALUE self)
{
  return INT2FIX(frt_read_byte(self));
}
  
static VALUE
frt_indexin_pos(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);
  return INT2FIX(my_buf->start + my_buf->pos);
}

static VALUE
frt_read_bytes(VALUE self, VALUE rbuffer, int offset, int len)
{
  int i;
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  VALUE rbuf = StringValue(rbuffer);

  if (RSTRING(rbuf)->len < (offset + len)) {
    rb_str_resize(rbuf, offset + len);
  }
  if ((len + offset) < BUFFER_SIZE) {
    rb_str_modify(rbuf);
    for (i = offset; i < offset + len; i++) {
      RSTRING(rbuf)->ptr[i] = read_byte(self, my_buf);
    }
  } else {
    VALUE start = frt_indexin_pos(self);
    rb_funcall(self, frt_seek_internal, 1, start);
    rb_funcall(self, frt_read_internal, 3,
        rbuf, INT2FIX(offset), INT2FIX(len));

    my_buf->start = my_buf->start + len;
    my_buf->pos = 0;
    my_buf->len = 0;              // trigger refill() on read()
  }

  return rbuf;
}

static VALUE
frt_indexin_read_bytes(VALUE self, VALUE rbuf, VALUE roffset, VALUE rlen)
{
  int len, offset;

  len = FIX2INT(rlen);
  offset = FIX2INT(roffset);

  return frt_read_bytes(self, rbuf, offset, len);
}

static VALUE
frt_indexin_seek(VALUE self, VALUE rpos)
{
  int pos = FIX2INT(rpos);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  if ((pos >= my_buf->start) && (pos < (my_buf->start + my_buf->len))) {
    my_buf->pos = pos - my_buf->start;  // seek within buffer
  } else {
    my_buf->start = pos;
    my_buf->pos = 0;
    my_buf->len = 0;              // trigger refill() on read()
    rb_funcall(self, frt_seek_internal, 1, rpos);
  }
  return Qnil;
}

static VALUE
frt_indexin_read_int(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  return LONG2NUM(((long)read_byte(self, my_buf) << 24) |
                  ((long)read_byte(self, my_buf) << 16) |
                  ((long)read_byte(self, my_buf) << 8) |
                   (long)read_byte(self, my_buf));
}

static VALUE
frt_indexin_read_long(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  return LL2NUM(((long long)read_byte(self, my_buf) << 56) |
                ((long long)read_byte(self, my_buf) << 48) |
                ((long long)read_byte(self, my_buf) << 40) |
                ((long long)read_byte(self, my_buf) << 32) |
                ((long long)read_byte(self, my_buf) << 24) |
                ((long long)read_byte(self, my_buf) << 16) |
                ((long long)read_byte(self, my_buf) << 8) |
                 (long long)read_byte(self, my_buf));
}

static VALUE
frt_indexin_read_uint(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  return ULONG2NUM(((unsigned long)read_byte(self, my_buf) << 24) |
                   ((unsigned long)read_byte(self, my_buf) << 16) |
                   ((unsigned long)read_byte(self, my_buf) << 8) |
                    (unsigned long)read_byte(self, my_buf));
}

static VALUE
frt_indexin_read_ulong(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  return ULL2NUM(((unsigned long long)read_byte(self, my_buf) << 56) |
                 ((unsigned long long)read_byte(self, my_buf) << 48) |
                 ((unsigned long long)read_byte(self, my_buf) << 40) |
                 ((unsigned long long)read_byte(self, my_buf) << 32) |
                 ((unsigned long long)read_byte(self, my_buf) << 24) |
                 ((unsigned long long)read_byte(self, my_buf) << 16) |
                 ((unsigned long long)read_byte(self, my_buf) << 8) |
                  (unsigned long long)read_byte(self, my_buf));
}

unsigned long long
frt_read_vint(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  register unsigned long long i, b;
  register int shift = 7;

  b = read_byte(self, my_buf);
  i = b & 0x7F; // 0x7F = 0b01111111
  
  while ((b & 0x80) != 0) {// 0x80 = 0b10000000
    b = read_byte(self, my_buf);
    i |= (b & 0x7F) << shift;
    shift += 7;
  }
  
  return i;
}

static VALUE
frt_indexin_read_vint(VALUE self)
{
  return ULL2NUM(frt_read_vint(self));
}

static VALUE
frt_indexin_read_string(VALUE self)
{
  int length = (int)frt_read_vint(self);
  VALUE str = rb_str_new(NULL, length);

  frt_read_bytes(self, str, 0, length);

  return str;
}

/****************************************************************************
 *
 * BufferIndexInput Methods
 *
 ****************************************************************************/

static VALUE
frt_indexout_flush(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  rb_funcall(self, frt_flush_buffer, 2,
      rb_str_new(my_buf->buffer, BUFFER_SIZE), INT2FIX(my_buf->pos));

  my_buf->start += my_buf->pos;
  my_buf->pos = 0;

  return Qnil;
}

void
write_byte(VALUE self, IndexBuffer *my_buf, byte_t b)
{
  my_buf->buffer[my_buf->pos++] = b;
  if (my_buf->pos >= BUFFER_SIZE)
    frt_indexout_flush(self);
}

static VALUE
frt_write_byte(VALUE self, byte_t b)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  my_buf->buffer[my_buf->pos++] = b;

  if (my_buf->pos >= BUFFER_SIZE)
    frt_indexout_flush(self);
  return Qnil;
}

static VALUE
frt_indexout_write_byte(VALUE self, VALUE rbyte)
{
  byte_t b = (byte_t)FIX2INT(rbyte);
  frt_write_byte(self, b);
  return Qnil;
}

static VALUE
frt_indexout_write_bytes(VALUE self, VALUE rbuffer, VALUE rlen)
{
  int len = FIX2INT(rlen);
  int i;
  VALUE rbuf = StringValue(rbuffer);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  for (i = 0; i < len; i++)
    write_byte(self, my_buf, RSTRING(rbuf)->ptr[i]); 

  return Qnil;
}

static VALUE
frt_indexout_pos(VALUE self)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);
  return INT2FIX(my_buf->start + my_buf->pos);
}

static VALUE
frt_indexout_seek(VALUE self, VALUE pos)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  frt_indexout_flush(self);
  my_buf->start = FIX2INT(pos);

  return Qnil;
}

static VALUE
frt_indexout_write_int(VALUE self, VALUE rint)
{
  long l = NUM2LONG(rint);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  write_byte(self, my_buf, (l >> 24) & 0xFF);
  write_byte(self, my_buf, (l >> 16) & 0xFF);
  write_byte(self, my_buf, (l >>  8) & 0xFF);
  write_byte(self, my_buf, l & 0xFF);

  return Qnil;
}

static VALUE
frt_indexout_write_long(VALUE self, VALUE rlong)
{
  long long l = NUM2LL(rlong);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  write_byte(self, my_buf, (l >> 56) & 0xFF);
  write_byte(self, my_buf, (l >> 48) & 0xFF);
  write_byte(self, my_buf, (l >> 40) & 0xFF);
  write_byte(self, my_buf, (l >> 32) & 0xFF);
  write_byte(self, my_buf, (l >> 24) & 0xFF);
  write_byte(self, my_buf, (l >> 16) & 0xFF);
  write_byte(self, my_buf, (l >>  8) & 0xFF);
  write_byte(self, my_buf, l & 0xFF);

  return Qnil;
}

static VALUE
frt_indexout_write_uint(VALUE self, VALUE ruint)
{
  unsigned long l = NUM2ULONG(ruint);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  write_byte(self, my_buf, (l >> 24) & 0xFF);
  write_byte(self, my_buf, (l >> 16) & 0xFF);
  write_byte(self, my_buf, (l >>  8) & 0xFF);
  write_byte(self, my_buf, l & 0xFF);

  return Qnil;
}

static VALUE
frt_indexout_write_ulong(VALUE self, VALUE rulong)
{
  unsigned long long l;
  l = rb_num2ull(rulong); // ruby 1.8 doesn't have NUM2ULL. Added in 1.9
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  write_byte(self, my_buf, (l >> 56) & 0xFF);
  write_byte(self, my_buf, (l >> 48) & 0xFF);
  write_byte(self, my_buf, (l >> 40) & 0xFF);
  write_byte(self, my_buf, (l >> 32) & 0xFF);
  write_byte(self, my_buf, (l >> 24) & 0xFF);
  write_byte(self, my_buf, (l >> 16) & 0xFF);
  write_byte(self, my_buf, (l >>  8) & 0xFF);
  write_byte(self, my_buf, l & 0xFF);

  return Qnil;
}

static VALUE
frt_write_vint(VALUE self, register unsigned long long i)
{
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  while (i > 127) {
    write_byte(self, my_buf, (i & 0x7f) | 0x80);
    i >>= 7;
  }
  write_byte(self, my_buf, i);

  return Qnil;
}

static VALUE
frt_indexout_write_vint(VALUE self, VALUE rulong)
{
  register unsigned long long i = rb_num2ull(rulong);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  while (i > 127) {
    write_byte(self, my_buf, (i & 0x7f) | 0x80);
    i >>= 7;
  }
  write_byte(self, my_buf, i);

  return Qnil;
}

static VALUE
frt_write_chars(VALUE self, VALUE rbuf, int start, int length)
{
  int i;
  VALUE rstr = StringValue(rbuf);
  IndexBuffer *my_buf;
  Data_Get_Struct(self, IndexBuffer, my_buf);

  for (i = start; i < start + length; i++) {
    write_byte(self, my_buf, RSTRING(rstr)->ptr[i]);
  }

  return Qnil;
}

static VALUE
frt_indexout_write_chars(VALUE self, VALUE rstr, VALUE rstart, VALUE rlength)
{
  int start = FIX2INT(rstart);
  int length = FIX2INT(rlength);

  return frt_write_chars(self, rstr, start, length);
}

static VALUE
frt_indexout_write_string(VALUE self, VALUE rstr)
{
  int len = RSTRING(StringValue(rstr))->len;
  frt_write_vint(self, len);

  frt_write_chars(self, rstr, 0, len);
  return Qnil;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_indexio(void)
{
  // IDs
  frt_length = rb_intern("length");
  frt_flush_buffer = rb_intern("flush_buffer");
  frt_read_internal = rb_intern("read_internal");
  frt_seek_internal = rb_intern("seek_internal");

  // IndexInput
  cIndexIn = rb_define_class_under(mStore, "IndexInput", rb_cObject);
  cBufferedIndexIn = rb_define_class_under(mStore, "BufferedIndexInput", cIndexIn);
  rb_define_alloc_func(cBufferedIndexIn, frt_indexbuffer_alloc);

  rb_define_method(cBufferedIndexIn, "initialize_copy", frt_indexin_init_copy, 1);
  rb_define_method(cBufferedIndexIn, "refill", frt_indexin_refill, 0);
  rb_define_method(cBufferedIndexIn, "read_byte", frt_indexin_read_byte, 0);
  rb_define_method(cBufferedIndexIn, "read_bytes", frt_indexin_read_bytes, 3);
  rb_define_method(cBufferedIndexIn, "pos", frt_indexin_pos, 0);
  rb_define_method(cBufferedIndexIn, "seek", frt_indexin_seek, 1);
  rb_define_method(cBufferedIndexIn, "read_int", frt_indexin_read_int, 0);
  rb_define_method(cBufferedIndexIn, "read_long", frt_indexin_read_long, 0);
  rb_define_method(cBufferedIndexIn, "read_uint", frt_indexin_read_uint, 0);
  rb_define_method(cBufferedIndexIn, "read_ulong", frt_indexin_read_ulong, 0);
  rb_define_method(cBufferedIndexIn, "read_vint", frt_indexin_read_vint, 0);
  rb_define_method(cBufferedIndexIn, "read_vlong", frt_indexin_read_vint, 0);
  rb_define_method(cBufferedIndexIn, "read_string", frt_indexin_read_string, 0);
  rb_define_method(cBufferedIndexIn, "read_chars", frt_indexin_read_bytes, 3);

  // IndexOutput
  cIndexOut = rb_define_class_under(mStore, "IndexOutput", rb_cObject);
  cBufferedIndexOut = rb_define_class_under(mStore, "BufferedIndexOutput", cIndexOut);
  rb_define_alloc_func(cBufferedIndexOut, frt_indexbuffer_alloc);

  rb_define_method(cBufferedIndexOut, "write_byte", frt_indexout_write_byte, 1);
  rb_define_method(cBufferedIndexOut, "write_bytes", frt_indexout_write_bytes, 2);
  rb_define_method(cBufferedIndexOut, "flush", frt_indexout_flush, 0);
  rb_define_method(cBufferedIndexOut, "close", frt_indexout_flush, 0);
  rb_define_method(cBufferedIndexOut, "pos", frt_indexout_pos, 0);
  rb_define_method(cBufferedIndexOut, "seek", frt_indexout_seek, 1);
  rb_define_method(cBufferedIndexOut, "write_int", frt_indexout_write_int, 1);
  rb_define_method(cBufferedIndexOut, "write_long", frt_indexout_write_long, 1);
  rb_define_method(cBufferedIndexOut, "write_uint", frt_indexout_write_uint, 1);
  rb_define_method(cBufferedIndexOut, "write_ulong", frt_indexout_write_ulong, 1);
  rb_define_method(cBufferedIndexOut, "write_vint", frt_indexout_write_vint, 1);
  rb_define_method(cBufferedIndexOut, "write_vlong", frt_indexout_write_vint, 1);
  rb_define_method(cBufferedIndexOut, "write_chars", frt_indexout_write_chars, 3);
  rb_define_method(cBufferedIndexOut, "write_string", frt_indexout_write_string, 1);

  // FSIndexInput
  //cFSIndexIn = rb_define_class_under(mStore, "FSIndexInput", cBufferedIndexIn);
}

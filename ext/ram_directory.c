#include "ferret.h"

ID flush, seek;

/****************************************************************************
 *
 * RAMFile Methods
 *
 ****************************************************************************/

void
frt_rf_free(void *p)
{
  int i;
  RAMFile *rf = (RAMFile *)p;

  for (i = 0; i < rf->bufcnt; i++) {
    free(rf->buffers[i]);
  }
  free(rf->buffers);
  free(rf);
}

void
frt_rf_mark(void *p)
{
  RAMFile *rf = (RAMFile *)p;
  rb_gc_mark(rf->mtime);
}

static VALUE
frt_rf_alloc(VALUE klass)
{
  RAMFile *rf;
  char *buf;

  rf = (RAMFile *)ALLOC(RAMFile);
  buf = (byte_t *)ALLOC_N(byte_t, BUFFER_SIZE);
  rf->buffers = (void **)ALLOC_N(void *, 1);
  rf->buffers[0] = buf;
  rf->bufcnt = 1;
  rf->length = 0;
  rf->mtime = rb_funcall(rb_cTime, frt_newobj, 0);

  VALUE rrf = Data_Wrap_Struct(klass, frt_rf_mark, frt_rf_free, rf);
  return rrf;
}

void
frt_rf_extend(RAMFile *rf)
{
  char *buf = (byte_t *)ALLOC_N(byte_t, BUFFER_SIZE);
  rf->bufcnt++;
  REALLOC_N(rf->buffers, void *, rf->bufcnt);
  rf->buffers[rf->bufcnt - 1] = buf;
}

static VALUE
frt_rf_length(VALUE self)
{
  RAMFile *rf;
  Data_Get_Struct(self, RAMFile, rf);

  return INT2FIX(rf->length);
}

/****************************************************************************
 *
 * RAMIndexOutput Methods
 *
 ****************************************************************************/

static VALUE
frt_rio_init(VALUE self, VALUE ramfile)
{
  rb_iv_set(self, "file", ramfile); 
  rb_iv_set(self, "pointer", INT2FIX(0)); 
  return self;
}

static VALUE
frt_rio_length(VALUE self)
{
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);
  return INT2FIX(rf->length);
}

void
frt_extend_buffer_if_necessary(RAMFile *rf, int bufnum)
{
  while (bufnum >= rf->bufcnt) {
    frt_rf_extend(rf);
  }
}

static VALUE
frt_rio_flush_buffer(VALUE self, VALUE rsrc, VALUE rlen)
{
  int buffer_number, buffer_offset, bytes_in_buffer, bytes_to_copy;
  int src_offset;
  int len = FIX2INT(rlen);
  //char *src = StringValuePtr(rsrc);
  int pointer = FIX2INT(rb_iv_get(self, "pointer"));
 
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);

  buffer_number = (int)(pointer / BUFFER_SIZE);
  buffer_offset = pointer % BUFFER_SIZE;
  bytes_in_buffer = BUFFER_SIZE - buffer_offset;
  bytes_to_copy = bytes_in_buffer < len ? bytes_in_buffer : len;

  frt_extend_buffer_if_necessary(rf, buffer_number);

  byte_t *buffer = rf->buffers[buffer_number];
  MEMCPY(buffer + buffer_offset, RSTRING(rsrc)->ptr, byte_t, bytes_to_copy);
  
  if (bytes_to_copy < len) {
    src_offset = bytes_to_copy;
    bytes_to_copy = len - bytes_to_copy;
    buffer_number += 1;
    frt_extend_buffer_if_necessary(rf, buffer_number);
    buffer = rf->buffers[buffer_number];

    MEMCPY(buffer, RSTRING(rsrc)->ptr + src_offset, byte_t, bytes_to_copy);
  }
  pointer += len;
  rb_iv_set(self, "pointer", INT2FIX(pointer));

  if (pointer > rf->length)
    rf->length = pointer;

  rf->mtime = rb_funcall(rb_cTime, frt_newobj, 0);
  return Qnil;
}

static VALUE
frt_rio_seek(VALUE self, VALUE rpos)
{
  rb_call_super(1, &rpos);

  rb_iv_set(self, "pointer", rpos);

  return rpos;
}

static VALUE
frt_rio_reset(VALUE self)
{
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);

  rf->length = 0;

  rb_funcall(self, seek, 1, INT2FIX(0));

  return Qnil;
}

static VALUE
frt_rio_close(VALUE self)
{
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);
  rf->mtime = rb_funcall(rb_cTime, frt_newobj, 0);
  rb_call_super(0, NULL);
  return Qnil;
}

static VALUE
frt_rio_write_to(VALUE self, VALUE routput)
{
  int i, len;
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);

  rb_funcall(self, flush, 0);

  int last_buffer_number = (int)(rf->length / BUFFER_SIZE);
  int last_buffer_offset = rf->length % BUFFER_SIZE;

  for (i = 0; i < rf->bufcnt; i++) {
    len = ((i == last_buffer_number) ? last_buffer_offset : BUFFER_SIZE);
    frt_write_bytes(routput, rf->buffers[i], len);
  }

  return Qnil;
}

/****************************************************************************
 *
 * RAMIndexInput Methods
 *
 ****************************************************************************/

static VALUE
frt_rii_init(VALUE self, VALUE ramfile)
{
  rb_iv_set(self, "file", ramfile); 
  rb_iv_set(self, "pointer", INT2FIX(0)); 
  return self;
}

static VALUE
frt_rii_length(VALUE self)
{
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);
  return INT2FIX(rf->length);
}

static VALUE
frt_rii_read_internal(VALUE self, VALUE rb, VALUE roffset, VALUE rlen)
{
	VALUE file = rb_iv_get(self, "file");
  RAMFile *rf;
	Data_Get_Struct(file, RAMFile, rf);

  int buffer_number, buffer_offset, bytes_in_buffer, bytes_to_copy;
  int offset = FIX2INT(roffset);
  int len = FIX2INT(rlen);
  int remainder = len;
  int pointer = FIX2INT(rb_iv_get(self, "pointer"));
  int start = pointer;
  byte_t *buffer;
  byte_t *b = StringValuePtr(rb);
  
  while (remainder > 0) {
    buffer_number = (int)(start / BUFFER_SIZE);
    buffer_offset = start % BUFFER_SIZE;
    bytes_in_buffer = BUFFER_SIZE - buffer_offset;
    
    if (bytes_in_buffer >= remainder) {
      bytes_to_copy = remainder;
    } else {
      bytes_to_copy = bytes_in_buffer;
    }
    buffer = rf->buffers[buffer_number];
    MEMCPY(b + offset, buffer + buffer_offset, byte_t, bytes_to_copy);
    offset += bytes_to_copy;
    start += bytes_to_copy;
    remainder -= bytes_to_copy;
  }
  
  rb_iv_set(self, "pointer", INT2FIX(pointer + len));
  return Qnil;
}

static VALUE
frt_rii_seek_internal(VALUE self, VALUE rpos)
{
  rb_iv_set(self, "pointer", rpos);
  return Qnil;
}

static VALUE
frt_rii_close(VALUE self)
{
  return Qnil;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_ram_directory(void)
{
  // IDs
  flush = rb_intern("flush");
  seek = rb_intern("seek");

  // RAMDirectory
  VALUE cDirectory = rb_define_class_under(mStore, "Directory", rb_cObject);
  cRAMDirectory = rb_define_class_under(mStore, "RAMDirectory", cDirectory);

  // RAMFile
  VALUE cRAMFile = rb_define_class_under(cRAMDirectory, "RAMFile", rb_cObject);
  rb_define_alloc_func(cRAMFile, frt_rf_alloc);

  // Methods
  rb_define_method(cRAMFile, "length", frt_rf_length, 0);

  // RAMIndexOutput
  cRAMIndexOut = rb_define_class_under(cRAMDirectory, "RAMIndexOutput", cBufferedIndexOut);
  //rb_define_alloc_func(cRAMIndexOut, frt_ramio_alloc);

  // Methods
  rb_define_method(cRAMIndexOut, "initialize", frt_rio_init, 1);
  rb_define_method(cRAMIndexOut, "length", frt_rio_length, 0);
  rb_define_method(cRAMIndexOut, "flush_buffer", frt_rio_flush_buffer, 2);
  rb_define_method(cRAMIndexOut, "reset", frt_rio_reset, 0);
  rb_define_method(cRAMIndexOut, "seek", frt_rio_seek, 1);
  rb_define_method(cRAMIndexOut, "close", frt_rio_close, 0);
  rb_define_method(cRAMIndexOut, "write_to", frt_rio_write_to, 1);

  // RAMIndexInput
  cRAMIndexIn = rb_define_class_under(cRAMDirectory, "RAMIndexInput", cBufferedIndexIn);
  //rb_define_alloc_func(cRAMIndexIn, frt_ramio_alloc);

  // Methods
  rb_define_method(cRAMIndexIn, "initialize", frt_rii_init, 1);
  rb_define_method(cRAMIndexIn, "length", frt_rii_length, 0);
  rb_define_method(cRAMIndexIn, "read_internal", frt_rii_read_internal, 3);
  rb_define_method(cRAMIndexIn, "seek_internal", frt_rii_seek_internal, 1);
  rb_define_method(cRAMIndexIn, "close", frt_rii_close, 0);
}

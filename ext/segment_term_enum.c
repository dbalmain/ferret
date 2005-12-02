#include "ferret.h"

/****************************************************************************
 *
 * SegmentTermEnum Methods
 *
 ****************************************************************************/
void
frt_ste_free(void *p)
{
  SegmentTermEnum *ste = (SegmentTermEnum *)p;
  free(ste->ti);
  free(p);
}
void
frt_ste_mark(void *p)
{
  SegmentTermEnum *ste = (SegmentTermEnum *)p;
  rb_gc_mark(ste->input);
  rb_gc_mark(ste->field_infos);
  rb_gc_mark(ste->rtb_curr);
  rb_gc_mark(ste->rtb_prev);
}

static VALUE
frt_ste_alloc(VALUE klass)
{
  SegmentTermEnum *ste = ALLOC(SegmentTermEnum);
  MEMZERO(ste, SegmentTermEnum, 1);
  return Data_Wrap_Struct(klass, frt_ste_mark, frt_ste_free, ste);
}

#define GET_STE SegmentTermEnum *ste; Data_Get_Struct(self, SegmentTermEnum, ste)
static VALUE
frt_ste_init(VALUE self, VALUE input, VALUE field_infos, VALUE is_index)
{
  int first_int;
  GET_STE;
  ste->is_index = RTEST(is_index);
  ste->input = input;
  Data_Get_Struct(input, IndexBuffer, ste->buf);
  ste->field_infos = field_infos;
  ste->position = -1;
  ste->rtb_curr = rb_class_new_instance(0, NULL, cTermBuffer);
  Data_Get_Struct(ste->rtb_curr, Term, ste->tb_curr);
  ste->rtb_prev = rb_class_new_instance(0, NULL, cTermBuffer);
  Data_Get_Struct(ste->rtb_prev, Term, ste->tb_prev);
  ste->ti = ALLOC(TermInfo);
  MEMZERO(ste->ti, TermInfo, 1);
  ste->index_pointer = 0;

  first_int = FIX2INT(frt_indexin_read_int(input));

  if (first_int >= 0) {
    // original-format file, without explicit format version number
    ste->format = 0;
    ste->size = first_int;

    // back-compatible settings
    ste->index_interval = 128;
    ste->skip_interval = 0x7FFFFFFF; // switch off skip_to optimization

  } else {
    // we have a format version number
    ste->format = first_int;
    ste->size = FIX2INT(frt_indexin_read_long(input));  // read the size
    
    if (ste->format == -1) {
      if (!ste->is_index) {
        ste->index_interval = FIX2INT(frt_indexin_read_int(input));
        ste->format_m1skip_interval = FIX2INT(frt_indexin_read_int(input));
      }
      // switch off skip_to optimization for file format prior to
      // 1.4rc2 in order to avoid a bug in skip_to implementation
      // of these versions
      ste->skip_interval = 0x7FFFFFFF;
    } else {
      ste->index_interval = FIX2INT(frt_indexin_read_int(input));
      ste->skip_interval = FIX2INT(frt_indexin_read_int(input));
    }
  }
  return self;
}

static VALUE
frt_ste_init_copy(VALUE self, VALUE rother)
{
  SegmentTermEnum *other; 
  GET_STE;
  Data_Get_Struct(rother, SegmentTermEnum, other);
  MEMCPY(ste, other, SegmentTermEnum, 1);
  ste->rtb_curr = rb_obj_clone(other->rtb_curr);
  Data_Get_Struct(ste->rtb_curr, Term, ste->tb_curr);
  ste->rtb_prev = rb_obj_clone(other->rtb_prev);
  Data_Get_Struct(ste->rtb_prev, Term, ste->tb_prev);
  ste->input = rb_obj_clone(other->input);
  Data_Get_Struct(ste->input, IndexBuffer, ste->buf);
  ste->ti = ALLOC(TermInfo);
  MEMCPY(ste->ti, other->ti, TermInfo, 1);
  return self;
}

static VALUE
frt_ste_seek(VALUE self, VALUE pointer, VALUE position, VALUE term, VALUE term_info)
{
  TermInfo *ti; 
  GET_STE;

  frt_indexin_seek(ste->input, pointer);

  ste->position = FIX2INT(position);
  frt_termbuffer_init_copy(ste->rtb_curr, term);
  free(ste->tb_prev->text);
  MEMZERO(ste->tb_prev, Term, 1);
  ste->tb_prev->field = Qnil;

  Data_Get_Struct(term_info, TermInfo, ti);
  MEMCPY(ste->ti, ti, TermInfo, 1);
  return Qnil;
}

static VALUE
frt_ste_next(VALUE self)
{
  IndexBuffer *zzbuf; 
  TermInfo *ti;
  GET_STE;
  ste->position++;

  Data_Get_Struct(ste->input, IndexBuffer, zzbuf);

  if (ste->position >= ste->size) {
    free(ste->tb_curr->text);
    MEMZERO(ste->tb_curr, Term, 1);
    ste->tb_curr->field = Qnil;
    return Qnil;
  }

  frt_termbuffer_init_copy(ste->rtb_prev, ste->rtb_curr);
  frt_termbuffer_read(ste->rtb_curr, ste->input, ste->field_infos);
  
  ti = ste->ti;
  ti->doc_freq = frt_read_vint(ste->input, zzbuf);         // read doc freq
  ti->freq_pointer += frt_read_vint(ste->input, zzbuf);    // read freq pointer
  ti->prox_pointer += frt_read_vint(ste->input, zzbuf);    // read prox pointer
  

  if (ste->format == -1) {
    // just read skip_offset in order to increment  file pointer
    // value is never used since skip_to is switched off
    if (!ste->is_index) {
      if (ti->doc_freq > ste->format_m1skip_interval)
        ti->skip_offset = frt_read_vint(ste->input, zzbuf);
    }
  } else {
    if (ti->doc_freq >= ste->skip_interval)
      ti->skip_offset = frt_read_vint(ste->input, zzbuf);
  }
  
  if (ste->is_index) {
    ste->index_pointer += frt_read_vint(ste->input, zzbuf); // read index pointer
  }

  return Qtrue;
}

static VALUE
frt_ste_scan_to(VALUE self, VALUE rterm)
{
  Term *term;
  GET_STE;
  Data_Get_Struct(rterm, Term, term); 
  while (frt_term_cmp(term, ste->tb_curr) > 0 && frt_ste_next(self) == Qtrue)
    ;
  return Qnil;
}

static VALUE
frt_ste_get_term(VALUE self)
{
  GET_STE;
  return frt_termbuffer_to_term(ste->rtb_curr);
}

static VALUE
frt_ste_get_term_buffer(VALUE self)
{
  GET_STE;
  return ste->rtb_curr;
}

static VALUE
frt_ste_get_prev(VALUE self)
{
  GET_STE;
  return frt_termbuffer_to_term(ste->rtb_prev);
}

static VALUE
frt_ste_get_term_info(VALUE self)
{
  VALUE rti;
  TermInfo *ti;
  GET_STE;
  rti = rb_obj_alloc(cTermInfo);
  Data_Get_Struct(rti, TermInfo, ti);
  MEMCPY(ti, ste->ti, TermInfo, 1);
  return rti;
}

static VALUE
frt_ste_set_term_info(VALUE self, VALUE rti)
{
  TermInfo *ti; 
  GET_STE;
  Data_Get_Struct(rti, TermInfo, ti);
  MEMCPY(ste->ti, ti, TermInfo, 1);
  return Qnil;
}

static VALUE
frt_ste_get_doc_freq(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->ti->doc_freq);
}

static VALUE
frt_ste_get_freq_pointer(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->ti->freq_pointer);
}

static VALUE
frt_ste_get_prox_pointer(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->ti->prox_pointer);
}

static VALUE
frt_ste_close(VALUE self)
{
  GET_STE;
  return rb_funcall(ste->input, id_close, 0);
}

static VALUE
frt_ste_get_field_infos(VALUE self)
{
  GET_STE;
  return ste->field_infos;
}

static VALUE
frt_ste_get_size(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->size);
}

static VALUE
frt_ste_get_position(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->position);
}

static VALUE
frt_ste_get_index_pointer(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->index_pointer);
}

static VALUE
frt_ste_get_index_interval(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->index_interval);
}

static VALUE
frt_ste_get_skip_interval(VALUE self)
{
  GET_STE;
  return INT2FIX(ste->skip_interval);
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_segment_term_enum(void)
{
  /* SegmentTermEnum */
  cSegmentTermEnum = rb_define_class_under(mIndex, "SegmentTermEnum", cTermEnum);
  rb_define_alloc_func(cSegmentTermEnum, frt_ste_alloc);

  rb_define_method(cSegmentTermEnum, "initialize", frt_ste_init, 3);
  rb_define_method(cSegmentTermEnum, "initialize_copy", frt_ste_init_copy, 1);
  rb_define_method(cSegmentTermEnum, "next?", frt_ste_next, 0);
  rb_define_method(cSegmentTermEnum, "seek", frt_ste_seek, 4);
  rb_define_method(cSegmentTermEnum, "scan_to", frt_ste_scan_to, 1);
  rb_define_method(cSegmentTermEnum, "term", frt_ste_get_term, 0);
  rb_define_method(cSegmentTermEnum, "term_buffer", frt_ste_get_term_buffer, 0);
  rb_define_method(cSegmentTermEnum, "prev", frt_ste_get_prev, 0);
  rb_define_method(cSegmentTermEnum, "term_info", frt_ste_get_term_info, 0);
  rb_define_method(cSegmentTermEnum, "term_info=", frt_ste_set_term_info, 1);
  rb_define_method(cSegmentTermEnum, "doc_freq", frt_ste_get_doc_freq, 0);
  rb_define_method(cSegmentTermEnum, "freq_pointer", frt_ste_get_freq_pointer, 0);
  rb_define_method(cSegmentTermEnum, "prox_pointer", frt_ste_get_prox_pointer, 0);
  rb_define_method(cSegmentTermEnum, "close", frt_ste_close, 0);
  rb_define_method(cSegmentTermEnum, "field_infos", frt_ste_get_field_infos, 0);
  rb_define_method(cSegmentTermEnum, "size", frt_ste_get_size, 0);
  rb_define_method(cSegmentTermEnum, "position", frt_ste_get_position, 0);
  rb_define_method(cSegmentTermEnum, "index_pointer", frt_ste_get_index_pointer, 0);
  rb_define_method(cSegmentTermEnum, "index_interval", frt_ste_get_index_interval, 0);
  rb_define_method(cSegmentTermEnum, "skip_interval", frt_ste_get_skip_interval, 0);
}

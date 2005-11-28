#include "ferret.h"

ID id_ste_position, id_ste_term_buffer, id_ste_prev_buffer, id_ste_term_info,
   id_ste_input, id_ste_field_infos, id_ste_format, id_ste_is_index,
   id_ste_skip_interval, id_ste_format_m1skip_interval, id_ste_index_pointer;
/****************************************************************************
 *
 * SegmentTermEnum Methods
 *
 ****************************************************************************/

static VALUE
frt_ste_next(VALUE self)
{
  int pos = FIX2INT(rb_ivar_get(self, id_ste_position));
  pos++;
  rb_ivar_set(self, id_ste_position, INT2FIX(pos));
  int size = FIX2INT(rb_ivar_get(self, id_iv_size));

  VALUE rtb_curr = rb_ivar_get(self, id_ste_term_buffer);
  Term *tb_curr; Data_Get_Struct(rtb_curr, Term, tb_curr);

  VALUE rinput = rb_ivar_get(self, id_ste_input);
  IndexBuffer *input; Data_Get_Struct(rinput, IndexBuffer, input);

  VALUE rfield_infos = rb_ivar_get(self, id_ste_field_infos);
  
  if (pos >= size) {
    free(tb_curr->text);
    MEMZERO(tb_curr, Term, 1);
    tb_curr->field = Qnil;
    return Qnil;
  }

  
  frt_termbuffer_init_copy(rb_ivar_get(self, id_ste_prev_buffer), rtb_curr);
  frt_termbuffer_read(rtb_curr, rinput, rfield_infos);
  
  TermInfo *ti;
  Data_Get_Struct(rb_ivar_get(self, id_ste_term_info), TermInfo, ti);
  ti->doc_freq = frt_read_vint(rinput, input);         // read doc freq
  ti->freq_pointer += frt_read_vint(rinput, input);    // read freq pointer
  ti->prox_pointer += frt_read_vint(rinput, input);    // read prox pointer
  
  int format = FIX2INT(rb_ivar_get(self, id_ste_format));
  int is_index = RTEST(rb_ivar_get(self, id_ste_is_index));

  if (format == -1) {
    // just read skip_offset in order to increment  file pointer
    // value is never used since skip_to is switched off
    if (!is_index) {
      int format_m1skip_interval =
        FIX2INT(rb_ivar_get(self, id_ste_format_m1skip_interval));
      if (ti->doc_freq > format_m1skip_interval)
        ti->skip_offset = frt_read_vint(rinput, input);
    }
  } else {
    int skip_interval =
        FIX2INT(rb_ivar_get(self, id_ste_skip_interval));
    if (ti->doc_freq >= skip_interval)
      ti->skip_offset = frt_read_vint(rinput, input);
  }
  
  if (is_index) {
    int index_pointer = FIX2INT(rb_ivar_get(self, id_ste_index_pointer));
    index_pointer += frt_read_vint(rinput, input); // read index pointer
    rb_ivar_set(self, id_ste_index_pointer, INT2FIX(index_pointer));
  }

  return Qtrue;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_segment_term_enum(void)
{
  /* IDs */
  id_ste_position = rb_intern("@position");
  id_ste_term_buffer = rb_intern("@term_buffer");
  id_ste_prev_buffer = rb_intern("@prev_buffer");
  id_ste_term_info = rb_intern("@term_info");
  id_ste_input = rb_intern("@input");;
  id_ste_field_infos = rb_intern("@field_infos");
  id_ste_format = rb_intern("@format");
  id_ste_is_index = rb_intern("@is_index");
  id_ste_skip_interval = rb_intern("@skip_interval");
  id_ste_format_m1skip_interval = rb_intern("@format_m1skip_interval");
  id_ste_index_pointer = rb_intern("@index_pointer");

  /* SegmentTermEnum */
	cSegmentTermEnum = rb_define_class_under(mIndex, "SegmentTermEnum", cTermEnum);

	rb_define_method(cSegmentTermEnum, "next?", frt_ste_next, 0);
}

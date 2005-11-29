#include "ferret.h"

/****************************************************************************
 *
 * Token Methods
 *
 ****************************************************************************/

ID id_tk_text, id_tk_pos_inc, id_tk_start_offset, id_tk_end_offset, id_tk_type;
ID id_tk_pos_inc_set;

static VALUE
frt_token_pos_inc (VALUE self, VALUE pI)
{
	if(FIX2INT(pI) < 0)
		rb_raise(rb_eArgError, "position_increment < 0");
	rb_ivar_set(self, id_tk_pos_inc, pI);
  return self;
}

static VALUE
frt_token_init(int argc, VALUE *argv, VALUE self)
{
  VALUE text, start_offset, end_offset, type, pos_inc;
  rb_scan_args(argc, argv, "32", &text,
      &start_offset, &end_offset, &type, &pos_inc);
  rb_ivar_set(self, id_tk_text, text);
  rb_ivar_set(self, id_tk_start_offset, start_offset);
  rb_ivar_set(self, id_tk_end_offset, end_offset);
  if (argc < 4) {
    rb_ivar_set(self, id_tk_type, rb_str_new("word", 4));
  } else {
    rb_ivar_set(self, id_tk_type, type);
  }
  if (argc < 5) {
    rb_ivar_set(self, id_tk_pos_inc, INT2FIX(1));
  } else {
    rb_ivar_set(self, id_tk_pos_inc, pos_inc);
  }
  return self;
}

static VALUE
frt_token_eql(VALUE self, VALUE other)
{
  if (!rb_respond_to(other, id_tk_pos_inc_set))
    return Qfalse;
  VALUE rself_text = rb_ivar_get(self, id_tk_text);
  VALUE rother_text = rb_ivar_get(other, id_tk_text);
  char *self_text = StringValuePtr(rself_text);
  char *other_text = StringValuePtr(rother_text);
  if (rb_ivar_get(self, id_tk_start_offset) == rb_ivar_get(other, id_tk_start_offset) &&
      rb_ivar_get(self, id_tk_end_offset) == rb_ivar_get(other, id_tk_end_offset) &&
      (strcmp(self_text, other_text) == 0))
    return Qtrue;
  else
    return Qfalse;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_token(void)
{
  /* IDs */
  id_tk_text = rb_intern("@term_text");
  id_tk_start_offset = rb_intern("@start_offset");
  id_tk_end_offset = rb_intern("@end_offset");
  id_tk_type = rb_intern("@type");
  id_tk_pos_inc = rb_intern("@position_increment");
  id_tk_pos_inc_set = rb_intern("position_increment=");


  /* IndexWriter */
  cToken = rb_define_class_under(mAnalysis, "Token", rb_cObject);

  rb_define_method(cToken, "initialize",   frt_token_init, -1);
	rb_define_method(cToken, "position_increment=", frt_token_pos_inc, 1);
	rb_define_method(cToken, "==", frt_token_eql, 1);
	rb_define_method(cToken, "eql", frt_token_eql, 1);
	
	rb_define_attr(cToken, "term_text", 1, 1);
	rb_define_attr(cToken, "position_increment", 1, 0);
	rb_define_attr(cToken, "start_offset", 1, 0);
	rb_define_attr(cToken, "end_offset", 1, 0);
	rb_define_attr(cToken, "type", 1, 1);
}

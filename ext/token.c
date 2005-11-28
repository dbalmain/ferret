#include "ferret.h"

/****************************************************************************
 *
 * Token Methods
 *
 ****************************************************************************/

ID id_tk_text, id_tk_pos_inc, id_tk_start_offset, id_tk_end_offset, id_tk_type;

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

  /* IndexWriter */
  cToken = rb_define_class_under(mAnalysis, "Token", rb_cObject);

  rb_define_method(cToken, "initialize",   frt_token_init, -1);
	rb_define_method(cToken, "position_increment=", frt_token_pos_inc, 1);
	
	rb_define_attr(cToken, "term_text", 1, 1);
	rb_define_attr(cToken, "position_increment", 1, 0);
	rb_define_attr(cToken, "start_offset", 1, 0);
	rb_define_attr(cToken, "end_offset", 1, 0);
	rb_define_attr(cToken, "type", 1, 1);
}

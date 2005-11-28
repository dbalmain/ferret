#include "ferret.h"


/****************************************************************************
 *
 * TermInfo Methods
 *
 ****************************************************************************/

void
frt_ti_free(void *p)
{
	free(p);
}

static VALUE
frt_ti_alloc(VALUE klass)
{
	TermInfo *ti = (TermInfo *)ALLOC(TermInfo);
	VALUE rbuffer = Data_Wrap_Struct(klass, NULL, frt_ti_free, ti);
	return rbuffer;
}

#define GET_TI TermInfo *ti; Data_Get_Struct(self, TermInfo, ti)
inline VALUE 
frt_ti_set(int argc, VALUE *argv, VALUE self)
{
  GET_TI;
  MEMZERO(ti, TermInfo, 1);
  VALUE df, fp, pp, so;
  rb_scan_args(argc, argv, "04", &df, &fp, &pp, &so);
  switch (argc) {
    case 4:
      ti->skip_offset = FIX2INT(so);
    case 3:
      ti->prox_pointer = FIX2INT(pp);
    case 2:
      ti->freq_pointer = FIX2INT(fp);
    case 1:
      ti->doc_freq = FIX2INT(df);
    case 0:
      break;
  }
	return Qnil;
}

static VALUE 
frt_ti_init(int argc, VALUE *argv, VALUE self)
{
  frt_ti_set(argc, argv, self);
	return self;
}

static VALUE 
frt_ti_init_copy(VALUE self, VALUE rother)
{
  GET_TI;
  TermInfo *other_ti; Data_Get_Struct(rother, TermInfo, other_ti);
  MEMCPY(ti, other_ti, TermInfo, 1);
	return self;
}

static VALUE
frt_ti_eql(VALUE self, VALUE rother)
{
  if (NIL_P(rother)) return Qfalse;
  GET_TI;
  TermInfo *other_ti; Data_Get_Struct(rother, TermInfo, other_ti);
	return (MEMCMP(ti, other_ti, TermInfo, 1) == 0) ? Qtrue : Qfalse;
}

static VALUE
frt_ti_get_df(VALUE self)
{
  GET_TI;
  return INT2FIX(ti->doc_freq);
}

static VALUE
frt_ti_get_fp(VALUE self)
{
  GET_TI;
  return INT2FIX(ti->freq_pointer);
}

static VALUE
frt_ti_get_pp(VALUE self)
{
  GET_TI;
  return INT2FIX(ti->prox_pointer);
}

static VALUE
frt_ti_get_so(VALUE self)
{
  GET_TI;
  return INT2FIX(ti->skip_offset);
}

static VALUE
frt_ti_set_df(VALUE self, VALUE val)
{
  GET_TI;
  ti->doc_freq = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_ti_set_fp(VALUE self, VALUE val)
{
  GET_TI;
  ti->freq_pointer = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_ti_set_pp(VALUE self, VALUE val)
{
  GET_TI;
  ti->prox_pointer = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_ti_set_so(VALUE self, VALUE val)
{
  GET_TI;
  ti->skip_offset = FIX2INT(val);
  return Qnil;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_term_info(void)
{
  /* TermInfo */
	cTermInfo = rb_define_class_under(mIndex, "TermInfo", rb_cObject);
	rb_define_alloc_func(cTermInfo, frt_ti_alloc);

	rb_define_method(cTermInfo, "initialize", frt_ti_init, -1);
	rb_define_method(cTermInfo, "set_values!", frt_ti_set, -1);
	rb_define_method(cTermInfo, "initialize_copy", frt_ti_init_copy, 1);
	rb_define_method(cTermInfo, "set!", frt_ti_init_copy, 1);
	rb_define_method(cTermInfo, "==", frt_ti_eql, 1);
	rb_define_method(cTermInfo, "doc_freq", frt_ti_get_df, 0);
	rb_define_method(cTermInfo, "doc_freq=", frt_ti_set_df, 1);
	rb_define_method(cTermInfo, "freq_pointer", frt_ti_get_fp, 0);
	rb_define_method(cTermInfo, "freq_pointer=", frt_ti_set_fp, 1);
	rb_define_method(cTermInfo, "prox_pointer", frt_ti_get_pp, 0);
	rb_define_method(cTermInfo, "prox_pointer=", frt_ti_set_pp, 1);
	rb_define_method(cTermInfo, "skip_offset", frt_ti_get_so, 0);
	rb_define_method(cTermInfo, "skip_offset=", frt_ti_set_so, 1);
}

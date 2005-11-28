#include "ferret.h"

ID id_field_name;
ID id_field_array;

/****************************************************************************
 *
 * TermBuffer Methods
 *
 ****************************************************************************/

void
frt_termbuffer_free(void *p)
{
	Term *tb = (Term *)p;
	free(tb->text);
	free(p);
}

void
frt_termbuffer_mark(void *p)
{
	Term *tb = (Term *)p;
  rb_gc_mark(tb->field);
}

static VALUE
frt_termbuffer_alloc(VALUE klass)
{
	Term *tb = ALLOC(Term);
  MEMZERO(tb, Term, 1);
  tb->field = Qnil;
	return Data_Wrap_Struct(klass, frt_termbuffer_mark, frt_termbuffer_free, tb);
}

static VALUE
frt_termbuffer_init(VALUE self)
{
	rb_iv_set(self, "@term", Qnil);
  return Qnil;
}

#define GET_TB Term *tb; Data_Get_Struct(self, Term, tb)
static VALUE
frt_termbuffer_get_text_length(VALUE self)
{
  GET_TB;
  return INT2FIX(tb->tlen);
}

static VALUE
frt_termbuffer_get_text(VALUE self)
{
  GET_TB;
  return rb_str_new(tb->text, tb->tlen);
}

static VALUE
frt_termbuffer_get_field_name(VALUE self)
{
  GET_TB;
  return tb->field;
}

static VALUE
frt_termbuffer_reset(VALUE self)
{
  GET_TB;

  free(tb->text);
  MEMZERO(tb, Term, 1);
  tb->field = Qnil;

	return Qnil;
}

VALUE
frt_termbuffer_to_term(VALUE self)
{
  GET_TB;

	if(NIL_P(tb->field)) {
		return Qnil;
  } else {
    VALUE args[2];
    args[0] = tb->field;
    args[1] = rb_str_new(tb->text, tb->tlen);
    return rb_class_new_instance(2, args, cTerm);
	}
}

int 
frt_termbuffer_compare_to_int(VALUE self, VALUE rother)
{
  GET_TB;
	Term *other; Data_Get_Struct(rother, Term, other);
  return frt_term_cmp(tb, other);
}

VALUE
frt_termbuffer_lt(VALUE self, VALUE rother)
{
  return frt_termbuffer_compare_to_int(self, rother) < 0 ? Qtrue : Qfalse;
}

VALUE
frt_termbuffer_gt(VALUE self, VALUE rother)
{
  return frt_termbuffer_compare_to_int(self, rother) > 0 ? Qtrue : Qfalse;
}

VALUE
frt_termbuffer_le(VALUE self, VALUE rother)
{
  return frt_termbuffer_compare_to_int(self, rother) <= 0 ? Qtrue : Qfalse;
}

VALUE
frt_termbuffer_ge(VALUE self, VALUE rother)
{
  return frt_termbuffer_compare_to_int(self, rother) >= 0 ? Qtrue : Qfalse;
}

VALUE
frt_termbuffer_eq(VALUE self, VALUE rother)
{
  if (rother == Qnil)
    return Qfalse;
  return frt_termbuffer_compare_to_int(self, rother) == 0 ? Qtrue : Qfalse;
}

static VALUE
frt_termbuffer_compare_to(VALUE self, VALUE rother)
{
	return INT2FIX(frt_termbuffer_compare_to_int(self, rother));
}

VALUE
frt_termbuffer_init_copy(VALUE self, VALUE rother)
{
  GET_TB;
  Term *tb_other;
	Data_Get_Struct(rother, Term, tb_other);

	int tlen = tb_other->tlen;
  REALLOC_N(tb->text, char, tlen+1);
	tb->tlen = tlen;
	MEMCPY(tb->text, tb_other->text, char, tlen);

	tb->field = tb_other->field;

	return Qnil;
}

VALUE
frt_termbuffer_read(VALUE self, VALUE rinput, VALUE rfield_infos)
{
	GET_TB;
  IndexBuffer *input; Data_Get_Struct(rinput, IndexBuffer, input);
	int tlen, start, length, fnum;
	
	start = frt_read_vint(rinput, input);
	length = frt_read_vint(rinput, input);
	tlen = start + length;
  REALLOC_N(tb->text, char, tlen+1);
	
	frt_read_chars(rinput, tb->text, start, length);
  fnum = frt_read_vint(rinput, input);
  if (fnum < 0) {
    tb->field = rb_str_new("", 0);
  } else {
    tb->field = rb_ivar_get(
        rb_ary_entry(rb_ivar_get(rfield_infos, id_field_array), fnum),
        id_field_name);
  }
  
  tb->tlen = tlen;
	return Qnil;
}

static VALUE
frt_termbuffer_hash(VALUE self)
{
  GET_TB;
  return INT2FIX(frt_hash(tb->text, tb->tlen) +
      frt_hash(RSTRING(tb->field)->ptr, RSTRING(tb->field)->len));
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/


void 
Init_term_buffer(void) {
  /* IDs */
	id_field_name = rb_intern("@name");
	id_field_array = rb_intern("@fi_array");

	/* TermBuffer */
	cTermBuffer = rb_define_class_under(mIndex, "TermBuffer", rb_cObject);
	rb_define_alloc_func(cTermBuffer, frt_termbuffer_alloc);
	rb_include_module(cTermBuffer, rb_mComparable);

  /* Methods */
	rb_define_method(cTermBuffer, "initialize", frt_termbuffer_init, 0);
	rb_define_method(cTermBuffer, "initialize_copy", frt_termbuffer_init_copy, 1);
	rb_define_method(cTermBuffer, "text", frt_termbuffer_get_text, 0);
	rb_define_method(cTermBuffer, "field", frt_termbuffer_get_field_name, 0);
	rb_define_method(cTermBuffer, "text_length", frt_termbuffer_get_text_length, 0);
	rb_define_method(cTermBuffer, "<=>", frt_termbuffer_compare_to, 1);
	rb_define_method(cTermBuffer, "<", frt_termbuffer_lt, 1);
	rb_define_method(cTermBuffer, ">", frt_termbuffer_gt, 1);
	rb_define_method(cTermBuffer, "<=", frt_termbuffer_le, 1);
	rb_define_method(cTermBuffer, ">=", frt_termbuffer_ge, 1);
	rb_define_method(cTermBuffer, "eql?", frt_termbuffer_eq, 1);
	rb_define_method(cTermBuffer, "==", frt_termbuffer_eq, 1);
	rb_define_method(cTermBuffer, "hash", frt_termbuffer_hash, 0);
	rb_define_method(cTermBuffer, "read", frt_termbuffer_read, 2);
	rb_define_method(cTermBuffer, "reset", frt_termbuffer_reset, 0);
	rb_define_method(cTermBuffer, "to_term", frt_termbuffer_to_term, 0);
	rb_define_method(cTermBuffer, "term", frt_termbuffer_to_term, 0);
	rb_define_method(cTermBuffer, "term=", frt_termbuffer_init_copy, 1);
	rb_define_method(cTermBuffer, "set!", frt_termbuffer_init_copy, 1);
}

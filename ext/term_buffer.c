#include "ferret.h"

ID id_field_name;

/****************************************************************************
 *
 * TermBuffer Methods
 *
 ****************************************************************************/

void
frt_termbuffer_free(void *p)
{
	Term *tb;
	tb = (Term *)p;
	free(tb->text);
	//free(tb->field);
	free(p);
}

static VALUE
frt_termbuffer_alloc(VALUE klass)
{
	Term *tb;
	tb = ALLOC(Term);
	tb->text = NULL;
	tb->field = NULL;
	tb->tlen = 0;
	tb->flen = 0;

	VALUE rbuffer = Data_Wrap_Struct(klass, NULL, frt_termbuffer_free, tb);
	return rbuffer;
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
  return rb_str_new(tb->field, tb->flen);
}

static VALUE
frt_termbuffer_reset(VALUE self)
{
  GET_TB;

  //free(tb->field);
  free(tb->text);
  MEMZERO(tb, Term, 1);

	return Qnil;
}

static VALUE
frt_termbuffer_to_term(VALUE self)
{
  GET_TB;

	if (tb->field == NULL) {
		return Qnil;
  } else {
    VALUE field = rb_str_new(tb->field, tb->flen);
    VALUE text = rb_str_new(tb->text, tb->tlen);
    return rb_funcall(cTerm, id_new, 2, field, text);
	}
}

int 
frt_termbuffer_compare_to_int(VALUE self, VALUE rother)
{
	int comp, size, my_len, o_len;
  GET_TB;
	Term *other;
	Data_Get_Struct(rother, Term, other);
	
	my_len = tb->flen;
	o_len = other->flen;
	size = my_len >= o_len ? o_len : my_len;
	comp = memcmp(tb->field, other->field, size);
	if(comp == 0){
		if(my_len == o_len) {
			my_len = tb->tlen;
			o_len = other->tlen;
			size = my_len >= o_len ? o_len : my_len;
			comp = memcmp(tb->text, other->text, size);
			if(comp == 0 && my_len != o_len)
				comp = my_len > o_len ? 1 : -1;
		} else {
			comp = my_len > o_len ? 1 : -1;
    }
	}
	return comp;
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
  Term *term;
	int tlen, flen;
	
	Data_Get_Struct(rother, Term, term);

	tlen = term->tlen;
	flen = term->flen;
	
  REALLOC_N(tb->text, char, tlen+1);
  //REALLOC_N(tb->field, char, flen+1);
	tb->flen = flen;
	tb->tlen = tlen;
	MEMCPY(tb->text, term->text, char, tlen);
	//MEMCPY(tb->field, term->field, char, flen);
  tb->field = term->field;

	return Qnil;
}

VALUE
frt_termbuffer_read(VALUE self, VALUE rinput, VALUE info)
{
	GET_TB;
  IndexBuffer *input; Data_Get_Struct(rinput, IndexBuffer, input);
	int tlen, flen, start, length;
	VALUE field, fnum;
	
	start = frt_read_vint(rinput, input);
	length = frt_read_vint(rinput, input);
	tlen = start + length;
  REALLOC_N(tb->text, char, tlen+1);
	
	frt_read_chars(rinput, tb->text, start, length);
  fnum = INT2FIX(frt_read_vint(rinput, input));
  field = rb_funcall(info, id_field_name, 1, fnum);
  flen = RSTRING(field)->len;
  
  //REALLOC_N(tb->field, char, flen+1);
  
  //MEMCPY(tb->field, RSTRING(field)->ptr, char, flen);
  tb->field = RSTRING(field)->ptr;

  tb->flen = flen;
  tb->tlen = tlen;
	return Qnil;
}

static VALUE
frt_termbuffer_hash(VALUE self)
{
  GET_TB;
  return INT2FIX(frt_hash(tb->text, tb->tlen) +
      frt_hash(tb->field, tb->flen));
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/


void 
Init_term_buffer(void) {
  /* IDs */
	id_field_name = rb_intern("name");

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

#include "ferret.h"

ID field_name;

/****************************************************************************
 *
 * TermBuffer Methods
 *
 ****************************************************************************/

void
frt_termbuffer_free(void *p)
{
	TermBuffer *tb;
	tb = (TermBuffer *)p;
	free((void *)(tb->text));
	free((void *)(tb->field));
	free(p);
}

static VALUE
frt_termbuffer_alloc(VALUE klass)
{
	TermBuffer *tb;
	tb = (TermBuffer *)ALLOC(TermBuffer);
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

static VALUE
frt_termbuffer_get_text_length(VALUE self)
{

	TermBuffer *tb;
	Data_Get_Struct(self, TermBuffer, tb);
  return INT2FIX(tb->tlen);
}

static VALUE
frt_termbuffer_get_text(VALUE self)
{

	TermBuffer *tb;
	Data_Get_Struct(self, TermBuffer, tb);
  return rb_str_new(tb->text, tb->tlen);
}

static VALUE
frt_termbuffer_get_field_name(VALUE self)
{

	TermBuffer *tb;
	Data_Get_Struct(self, TermBuffer, tb);
  return rb_str_new(tb->field, tb->flen);
}

static VALUE
frt_termbuffer_reset(VALUE self)
{
	TermBuffer *tb;
	Data_Get_Struct(self, TermBuffer, tb);

	tb->field = NULL;
	tb->text = NULL;
	tb->tlen = 0;
	tb->flen = 0;

	return Qnil;
}

static VALUE
frt_termbuffer_to_term(VALUE self)
{
	TermBuffer *tb;
	Data_Get_Struct(self, TermBuffer, tb);

	if(tb->field == NULL) {
		return Qnil;
  } else {
    VALUE field = rb_str_new(tb->field, tb->flen);
    VALUE text = rb_str_new(tb->text, tb->tlen);
    return rb_funcall(cTerm, frt_newobj, 2, field, text);
	}
}

int 
frt_termbuffer_compare_to_int(VALUE self, VALUE rother)
{
	int comp, size, my_len, o_len;
	TermBuffer *tb, *other;
	Data_Get_Struct(self, TermBuffer, tb);
	Data_Get_Struct(rother, TermBuffer, other);
	
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

static VALUE
frt_termbuffer_set_term(VALUE self, VALUE rterm)
{
	TermBuffer *tb;
  Term *term;
	int tlen, flen;
	
	Data_Get_Struct(self, TermBuffer, tb);
	Data_Get_Struct(rterm, Term, term);

	tlen = term->tlen;
	flen = term->flen;
	
	if(tb->field == NULL){
		tb->field = (char *)ALLOC_N(char, flen+1);
		tb->text = (char *)ALLOC_N(char, tlen+1);
	} else {
		REALLOC_N(tb->text, char, tlen+1);
		REALLOC_N(tb->field, char, flen+1);
	}
	
	tb->flen = flen;
	tb->tlen = tlen;
	MEMCPY(tb->text, term->text, char, tlen);
	MEMCPY(tb->field, term->field, char, flen);

	return Qnil;
}

static VALUE
frt_termbuffer_init_copy(VALUE self, VALUE rother)
{
	TermBuffer *tb, *other;
	int tlen, flen;
	
	Data_Get_Struct(self, TermBuffer, tb);
	Data_Get_Struct(rother, TermBuffer, other);

	tlen = other->tlen;
	flen = other->flen;
	
	if(tb->field == NULL){
		tb->field = (char *)ALLOC_N(char, flen+1);
		tb->text = (char *)ALLOC_N(char, tlen+1);
	} else {
		REALLOC_N(tb->text, char, tlen+1);
		REALLOC_N(tb->field, char, flen+1);
	}
	
	tb->flen = flen;
	tb->tlen = tlen;
	MEMCPY(tb->text, other->text, char, tlen);
	MEMCPY(tb->field, other->field, char, flen);

	return Qnil;
}

static VALUE
frt_termbuffer_read(VALUE self, VALUE input, VALUE info)
{
	TermBuffer *tb;
	int tlen, flen, start, length;
	VALUE field, fnum;
	Data_Get_Struct(self, TermBuffer, tb);
	
	start = frt_read_vint(input);
	length = frt_read_vint(input);
	tlen = start + length;
	
	if(tb->field == NULL){
		tb->text = (char *)ALLOC_N(char, tlen+1);
	} else {
		REALLOC_N(tb->text, char, tlen+1);
	}
	
	frt_read_chars(input, tb->text, start, length);
  fnum = INT2FIX(frt_read_vint(input));
  field = rb_funcall(info, field_name, 1, fnum);
  flen = RSTRING(field)->len;
  
  REALLOC_N(tb->field, char, flen+1);
  
  MEMCPY(tb->field, RSTRING(field)->ptr, char, flen);

  tb->flen = flen;
  tb->tlen = tlen;
	return Qnil;
}

static VALUE
frt_termbuffer_hash(VALUE self)
{
	TermBuffer *tb;
	Data_Get_Struct(self, TermBuffer, tb);
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
  // IDs
	field_name = rb_intern("name");

	// TermBuffer
	cTermBuffer = rb_define_class_under(mIndex, "TermBuffer", rb_cObject);
	rb_define_alloc_func(cTermBuffer, frt_termbuffer_alloc);
	rb_include_module(cTermBuffer, rb_mComparable);

  // Methods
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
	rb_define_method(cTermBuffer, "term=", frt_termbuffer_set_term, 1);
	rb_define_method(cTermBuffer, "set!", frt_termbuffer_init_copy, 1);
	rb_define_method(cTermBuffer, "text_str", frt_termbuffer_get_text, 0);
}

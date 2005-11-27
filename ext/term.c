#include "ferret.h"


/****************************************************************************
 *
 * Term Methods
 *
 ****************************************************************************/

void
frt_term_free(void *p)
{
	Term *term = (Term *)p;
	free(term->field);
	free(term->text);
	free(p);
}

static VALUE
frt_term_alloc(VALUE klass)
{
	Term *term;
	term = ALLOC(Term);
	term->field = ALLOC_N(char, 1);
	term->text = ALLOC_N(char, 1);
	
	VALUE rbuffer = Data_Wrap_Struct(klass, NULL, frt_term_free, term);
	return rbuffer;
}

VALUE 
frt_term_set(VALUE self, VALUE rfield, VALUE rtext)
{
	Term *term;
	int flen = RSTRING(rfield)->len;
	int tlen = RSTRING(rtext)->len;
 	char *field = RSTRING(rfield)->ptr;
 	char *text = RSTRING(rtext)->ptr;
	Data_Get_Struct(self, Term, term);

	REALLOC_N(term->field, char, flen + 1);
	REALLOC_N(term->text, char, tlen + 1);
	
	MEMCPY(term->field, field, char, flen);
	MEMCPY(term->text, text, char, tlen);
	term->flen = flen;
	term->tlen = tlen;
	
	return Qnil;
}

static VALUE 
frt_term_init(VALUE self, VALUE rfield, VALUE rtext)
{
	frt_term_set(self, rfield, rtext);
	return self;
}

static VALUE 
frt_term_get_text(VALUE self)
{
	Term *term;
	Data_Get_Struct(self, Term, term);
	return rb_str_new(term->text, term->tlen);
}

static VALUE 
frt_term_set_text(VALUE self, VALUE rtext)
{
	Term *term;
	int tlen = RSTRING(rtext)->len;
 	char *text = RSTRING(rtext)->ptr;
	Data_Get_Struct(self, Term, term);

	REALLOC_N(term->text, char, tlen + 1);
	
	MEMCPY(term->text, text, char, tlen);
	term->tlen = tlen;
	
	return Qnil;
}

static VALUE 
frt_term_get_field(VALUE self)
{
	Term *term;
	Data_Get_Struct(self, Term, term);
	return rb_str_new(term->field, term->flen);
}

static VALUE 
frt_term_set_field(VALUE self, VALUE rfield)
{
	Term *term;
	int flen = RSTRING(rfield)->len;
 	char *field = RSTRING(rfield)->ptr;
	Data_Get_Struct(self, Term, term);

	REALLOC_N(term->field, char, flen + 1);
	
	MEMCPY(term->field, field, char, flen);
	term->flen = flen;
	
	return Qnil;
}

VALUE 
frt_term_to_s(VALUE self)
{
	Term *term;
	int tlen, flen;
	Data_Get_Struct(self, Term, term);
	tlen = term->tlen;
	flen = term->flen;
	char res[flen + tlen + 1];
	char delim[] = ":";
	
	MEMCPY(res, term->field, char, flen);
	MEMCPY(res + flen, delim, char, 1);
	MEMCPY(res + flen + 1, term->text, char, tlen);
	return rb_str_new(res, tlen + flen + 1 );
}

int
frt_term_compare_to_int(VALUE self, VALUE rother)
{
	int comp, size, mylen, olen;
	Term *term, *other;
	Data_Get_Struct(self, Term, term);
	Data_Get_Struct(rother, Term, other);
	
	mylen = term->flen;
	olen = other->flen;
	size = mylen >= olen ? olen : mylen;
	comp = memcmp(term->field, other->field, size);
	if(comp == 0){
		if(mylen == olen){
			mylen = term->tlen;
			olen = other->tlen;
			size = mylen >= olen ? olen : mylen;
			comp = memcmp(term->text, other->text, size);
			if(comp == 0 && mylen != olen)
				comp = mylen > olen ? 1 : -1;
		} else
			comp = mylen > olen ? 1 : -1;
	}
  /*
	comp = strcmp(term->field, other->field);
	if(comp == 0)
		comp = strcmp(term->text, other->text);
  */
	return comp;
}

VALUE
frt_term_lt(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) < 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_gt(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) > 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_le(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) <= 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_ge(VALUE self, VALUE rother)
{
  return frt_term_compare_to_int(self, rother) >= 0 ? Qtrue : Qfalse;
}

VALUE
frt_term_eq(VALUE self, VALUE rother)
{
  if (rother == Qnil)
    return Qfalse;
  return frt_term_compare_to_int(self, rother) == 0 ? Qtrue : Qfalse;
}


static VALUE
frt_term_compare_to(VALUE self, VALUE other)
{
	return INT2FIX(frt_term_compare_to_int(self, other));
}

static VALUE
frt_term_hash(VALUE self)
{
	Term *term;
	Data_Get_Struct(self, Term, term);
  return INT2FIX(frt_hash(term->text, term->tlen) +
      frt_hash(term->field, term->flen));
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_term(void)
{
	/* Term */
	cTerm = rb_define_class_under(mIndex, "Term", rb_cObject);
	rb_define_alloc_func(cTerm, frt_term_alloc);
	rb_include_module(cTerm, rb_mComparable);

	rb_define_method(cTerm, "initialize", frt_term_init, 2);
	rb_define_method(cTerm, "set!", frt_term_set, 2);
	rb_define_method(cTerm, "to_s", frt_term_to_s, 0);
	rb_define_method(cTerm, "<=>", frt_term_compare_to, 1);
	rb_define_method(cTerm, "<", frt_term_lt, 1);
	rb_define_method(cTerm, ">", frt_term_gt, 1);
	rb_define_method(cTerm, "<=", frt_term_le, 1);
	rb_define_method(cTerm, ">=", frt_term_ge, 1);
	rb_define_method(cTerm, "eql?", frt_term_eq, 1);
	rb_define_method(cTerm, "==", frt_term_eq, 1);
	rb_define_method(cTerm, "text", frt_term_get_text, 0);
	rb_define_method(cTerm, "text=", frt_term_set_text, 1);
	rb_define_method(cTerm, "field", frt_term_get_field, 0);
	rb_define_method(cTerm, "field=", frt_term_set_field, 1);
	rb_define_method(cTerm, "hash", frt_term_hash, 0);
}

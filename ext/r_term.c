#include "ferret.h"
#include "index.h"

VALUE cTerm;
/****************************************************************************
 *
 * Term Methods
 *
 ****************************************************************************/

typedef struct RTerm {
  VALUE field;
  VALUE text;
} RTerm;

void
frt_term_mark(void *p)
{
  RTerm *term = (RTerm *)p;
  rb_gc_mark(term->field);
  rb_gc_mark(term->text);
}

static VALUE
frt_term_alloc(VALUE klass)
{
  RTerm *term = ALLOC(RTerm);
  term->field = Qnil;
  term->text = Qnil;
  return Data_Wrap_Struct(klass, &frt_term_mark, &free, term);
}

#define GET_TERM RTerm *term; Data_Get_Struct(self, RTerm, term)
static VALUE 
frt_term_init(VALUE self, VALUE rfield, VALUE rtext)
{
  GET_TERM;
  term->field = rb_obj_as_string(rfield);
  term->text = rb_obj_as_string(rtext);
  return self;
}

VALUE
frt_get_rterm(char *field, char *text)
{
  RTerm *rterm = ALLOC(RTerm);
  rterm->field = rb_str_new2(field);
  rterm->text = rb_str_new2(text);
  return Data_Wrap_Struct(cTerm, &frt_term_mark, &free, rterm);
}

Term *
frt_set_term(VALUE self, Term *t)
{
  GET_TERM;
  t->field = RSTRING(term->field)->ptr;
  t->text = RSTRING(term->text)->ptr;
  return t;
}

Term *
frt_get_term(VALUE self)
{
  Term *t = NULL;
  if (self != Qnil) {
    GET_TERM;
    t = ALLOC(Term);
    /* store text and field in text so that field will be freed with text */
    t->text = ALLOC_N(char, RSTRING(term->text)->len +
        RSTRING(term->field)->len + 2);
    sprintf(t->text, "%s %s", RSTRING(term->text)->ptr,
        RSTRING(term->field)->ptr);
    t->text[RSTRING(term->text)->len] = '\0';
    t->field = t->text + RSTRING(term->text)->len + 1;
  }
  return t;
}

static VALUE 
frt_term_get_text(VALUE self)
{
  GET_TERM;
  return term->text;
}

static VALUE 
frt_term_set_text(VALUE self, VALUE rtext)
{
  GET_TERM;
  term->text = rb_obj_as_string(rtext);
  return Qnil;
}

static VALUE 
frt_term_get_field(VALUE self)
{
  GET_TERM;
  return term->field;
}

static VALUE 
frt_term_set_field(VALUE self, VALUE rfield)
{
  GET_TERM;
  term->field = rb_obj_as_string(rfield);
  return Qnil;
}

VALUE 
frt_term_to_s(VALUE self)
{
  int tlen, flen;
  char *res;
  GET_TERM;
  tlen = RSTRING(term->text)->len;
  flen = RSTRING(term->field)->len;
  res = alloca(flen + tlen + 1);    
  
  MEMCPY(res, StringValuePtr(term->field), char, flen);
  res[flen] = ':';
  MEMCPY(res + flen + 1, StringValuePtr(term->text), char, tlen);
  return rb_str_new(res, tlen + flen + 1 );
}

inline int
frt_term_cmp(RTerm *t1, RTerm *t2)
{
  int comp = rb_str_cmp(t1->field, t2->field);
  if (comp == 0) {
    comp = rb_str_cmp(t1->text, t2->text);
  }
  return comp;
}

int
frt_term_compare_to_int(VALUE self, VALUE rother)
{
  RTerm *other;
  GET_TERM;
  Data_Get_Struct(rother, RTerm, other);
  return frt_term_cmp(term, other);
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
  GET_TERM;
  return INT2FIX(rb_str_hash(term->field) + rb_str_hash(term->text));
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
  rb_define_method(cTerm, "set!", frt_term_init, 2);
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

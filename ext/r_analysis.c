#include "ferret.h"
#include "analysis.h"

static VALUE cToken;
static VALUE cLetterTokenizer;

static VALUE cAnalyzer;
static VALUE cLetterAnalyzer;
static VALUE cWhiteSpaceAnalyzer;
static VALUE cStandardAnalyzer;

/****************************************************************************
 *
 * Token Methods
 *
 ****************************************************************************/

typedef struct RToken {
  VALUE text;
  int start;
  int end;
  int pos_inc;
} RToken;

static void
frt_token_free(void *p)
{
  free(p);
}
  
static void
frt_token_mark(void *p)
{
  RToken *token = (RToken *)p;
  rb_gc_mark(token->text);
}

static VALUE
frt_token_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, &frt_token_mark, &frt_token_free, ALLOC(RToken));
}

#define GET_TK RToken *token; Data_Get_Struct(self, RToken, token);
static VALUE
frt_token_init(int argc, VALUE *argv, VALUE self) 
{
  GET_TK;
  VALUE rtext, rstart, rend, rpos_inc, rtype;
  token->pos_inc = 1;
  switch (rb_scan_args(argc, argv, "32", &rtext, &rstart, &rend, &rpos_inc, &rtype)) {
    case 5: /* type gets ignored at this stage */
    case 4: token->pos_inc = FIX2INT(rpos_inc);
  }
  token->text = rb_obj_as_string(rtext);
  token->start = FIX2INT(rstart);
  token->end = FIX2INT(rend);
  return self;
}

static VALUE
frt_token_cmp(VALUE self, VALUE rother)
{
  RToken *other;
  int cmp;
  GET_TK;
  Data_Get_Struct(rother, RToken, other);
  if (token->start > other->start) {
    cmp = 1;
  } else if (token->start < other->start) {
    cmp = -1;
  } else {
    if (token->end > other->end) {
      cmp = 1;
    } else if (token->end < other->end) {
      cmp = -1;
    } else {
      cmp = strcmp(RSTRING(token->text)->ptr, RSTRING(other->text)->ptr);
    }
  }
  return INT2FIX(cmp);
}

static VALUE
frt_token_get_text(VALUE self)
{
  GET_TK;
  return token->text;
}

static VALUE
frt_token_set_text(VALUE self, VALUE rtext)
{
  GET_TK;
  token->text = rtext;
  return rtext;
}

static VALUE
frt_token_get_start_offset(VALUE self)
{
  GET_TK;
  return INT2FIX(token->start);
}

static VALUE
frt_token_get_end_offset(VALUE self)
{
  GET_TK;
  return INT2FIX(token->end);
}

static VALUE
frt_token_get_pos_inc(VALUE self)
{
  GET_TK;
  return INT2FIX(token->pos_inc);
}

static VALUE
frt_token_to_s(VALUE self)
{
  GET_TK;
  char *buf = alloca(RSTRING(token->text)->len + 80);
  sprintf(buf, "token[\"%s\":%d:%d:%d]", RSTRING(token->text)->ptr, token->start,
      token->end, token->pos_inc);
  return rb_str_new2(buf);
}

/****************************************************************************
 *
 * Tokenizer Methods
 *
 ****************************************************************************/

static void
frt_tokenizer_free(void *p)
{
  TokenStream *ts = (TokenStream *)p;
  object_del(p);
  ts->destroy(ts);
}

static VALUE
frt_letter_tokenizer_init(VALUE self, VALUE rstr) 
{
  TokenStream *ts = letter_tokenizer_create();
  Frt_Wrap_Struct(self, NULL, &frt_tokenizer_free, ts);
  return self;
}

/****************************************************************************
 *
 * Analyzer Methods
 *
 ****************************************************************************/

static void
frt_analyzer_free(void *p)
{
  Analyzer *a = (Analyzer *)p;
  object_del(a);
  a->destroy(a);
}

VALUE
frt_get_analyzer(Analyzer *a)
{
  VALUE self = Data_Wrap_Struct(cAnalyzer, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** WhiteSpaceAnalyzer ***/
static VALUE
frt_white_space_analyzer_init(VALUE self)
{
  Analyzer *a = whitespace_analyzer_create();
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** LetterAnalyzer ***/
static VALUE
frt_letter_analyzer_init(VALUE self)
{
  Analyzer *a = letter_analyzer_create();
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** StandardAnalyzer ***/
static VALUE
frt_standard_analyzer_init(VALUE self)
{
  Analyzer *a = standard_analyzer_create();
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_analysis(void)
{
  cToken = rb_define_class_under(mAnalysis, "Token", rb_cObject);
  rb_define_alloc_func(cToken, frt_token_alloc);
  rb_include_module(cToken, rb_mComparable);

  rb_define_method(cToken, "initialize", frt_token_init, -1);
  rb_define_method(cToken, "<=>", frt_token_cmp, 1);
  rb_define_method(cToken, "text", frt_token_get_text, 0);
  rb_define_method(cToken, "text=", frt_token_set_text, 1);
  rb_define_method(cToken, "start_offset", frt_token_get_start_offset, 0);
  rb_define_method(cToken, "end_offset", frt_token_get_end_offset, 0);
  rb_define_method(cToken, "pos_inc", frt_token_get_pos_inc, 0);
  rb_define_method(cToken, "to_s", frt_token_to_s, 0);

  cLetterTokenizer =
    rb_define_class_under(mAnalysis, "LetterTokenizer", rb_cObject);
  rb_define_alloc_func(cLetterTokenizer, frt_data_alloc);
  rb_define_method(cLetterTokenizer, "initialize",
      frt_letter_tokenizer_init, 1);

  cAnalyzer =
    rb_define_class_under(mAnalysis, "Analyzer", rb_cObject);
  rb_define_alloc_func(cAnalyzer, frt_data_alloc);
  rb_define_method(cAnalyzer, "initialize",
      frt_letter_analyzer_init, 0);

  cLetterAnalyzer =
    rb_define_class_under(mAnalysis, "LetterAnalyzer", cAnalyzer);
  rb_define_alloc_func(cLetterAnalyzer, frt_data_alloc);
  rb_define_method(cAnalyzer, "initialize",
      frt_letter_analyzer_init, 0);

  cWhiteSpaceAnalyzer =
    rb_define_class_under(mAnalysis, "WhiteSpaceAnalyzer", cAnalyzer);
  rb_define_alloc_func(cWhiteSpaceAnalyzer, frt_data_alloc);
  rb_define_method(cWhiteSpaceAnalyzer, "initialize",
      frt_white_space_analyzer_init, 0);

  cStandardAnalyzer =
    rb_define_class_under(mAnalysis, "StandardAnalyzer", cAnalyzer);
  rb_define_alloc_func(cStandardAnalyzer, frt_data_alloc);
  rb_define_method(cStandardAnalyzer, "initialize",
      frt_standard_analyzer_init, 0);
}

#include "ferret.h"
#include "analysis.h"
#include "locale.h"

static VALUE cToken;
static VALUE cLetterTokenizer;

static VALUE cAnalyzer;
static VALUE cAsciiLetterAnalyzer;
static VALUE cLetterAnalyzer;
static VALUE cAsciiWhiteSpaceAnalyzer;
static VALUE cWhiteSpaceAnalyzer;
static VALUE cAsciiStandardAnalyzer;
static VALUE cStandardAnalyzer;

//static VALUE cRegexAnalyzer;
static VALUE cTokenStream;

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

static VALUE
get_token(Token *tk)
{
  RToken *token = ALLOC(RToken);

  token->text = rb_str_new2(tk->text);
  token->start = tk->start;
  token->end = tk->end;
  token->pos_inc = tk->pos_inc;
  return Data_Wrap_Struct(cToken, &frt_token_mark, &frt_token_free, token);
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
frt_ts_mark(void *p)
{
  TokenStream *ts = (TokenStream *)p;
  frt_gc_mark(&ts->text);
}

static void
frt_ts_free(void *p)
{
  TokenStream *ts = (TokenStream *)p;
  object_del(&ts->text);
  object_del(ts);
  ts->destroy(ts);
}

static VALUE
get_token_stream(TokenStream *ts)
{
  VALUE rts = object_get(ts);
  if (rts == Qnil) {
    rts = Data_Wrap_Struct(cTokenStream, &frt_ts_mark, &frt_ts_free, ts);
    object_add(ts, rts);
  }
  return rts;
}

static VALUE
frt_letter_tokenizer_init(VALUE self, VALUE rstr) 
{
  TokenStream *ts = letter_tokenizer_create();
  Frt_Wrap_Struct(self, NULL, &frt_ts_free, ts);
  object_add(ts, self);
  return self;
}

static VALUE
frt_token_stream_next(VALUE self)
{
  TokenStream *ts; 
  Data_Get_Struct(self, TokenStream, ts);
  Token *next = ts->next(ts);
  if (next == NULL) {
    return Qnil;
  }

  return get_token(next);
}

/****************************************************************************
 *
 * Analyzer Methods
 *
 ****************************************************************************/

static void
frt_analyzer_mark(void *p)
{
  Analyzer *a = (Analyzer *)p;
  frt_gc_mark(a->current_ts);
}

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

static VALUE
frt_analyzer_token_stream(VALUE self, VALUE rfield, VALUE rstring)
{
  Analyzer *a = ((struct RData *)(self))->data;
  rfield = rb_obj_as_string(rfield);
  rstring = rb_obj_as_string(rstring);
  
  TokenStream *ts = a_get_new_ts(a, RSTRING(rfield)->ptr, RSTRING(rstring)->ptr);

  // Make sure that there is no text entry already for this TokenStream
  if (object_get(&ts->text) != Qnil) {
    object_del(&ts->text);
  }
  object_add(&ts->text, rstring); // Make sure that there is no entry already
  return get_token_stream(ts);
}

#define GET_LOWER(dflt) \
  bool lower;\
  VALUE rlower;\
  rb_scan_args(argc, argv, "01", &rlower);\
  lower = ((rlower == Qnil) ? dflt : RTEST(rlower));

/*** AsciiWhiteSpaceAnalyzer ***/
static VALUE
frt_a_white_space_analyzer_init(int argc, VALUE *argv, VALUE self)
{
  GET_LOWER(false);
  Analyzer *a = whitespace_analyzer_create(lower);
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** WhiteSpaceAnalyzer ***/
static VALUE
frt_white_space_analyzer_init(int argc, VALUE *argv, VALUE self)
{
  GET_LOWER(false);
  Analyzer *a = mb_whitespace_analyzer_create(lower);
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** AsciiLetterAnalyzer ***/
static VALUE
frt_a_letter_analyzer_init(int argc, VALUE *argv, VALUE self)
{
  GET_LOWER(true);
  Analyzer *a = letter_analyzer_create(lower);
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** LetterAnalyzer ***/
static VALUE
frt_letter_analyzer_init(int argc, VALUE *argv, VALUE self)
{
  GET_LOWER(true);
  Analyzer *a = mb_letter_analyzer_create(lower);
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

static char **
get_stop_words(VALUE rstop_words)
{
  char **stop_words;
  int i, len;
  VALUE rstr;
  Check_Type(rstop_words, T_ARRAY);
  len = RARRAY(rstop_words)->len;
  stop_words = ALLOC_N(char *, RARRAY(rstop_words)->len + 1);
  stop_words[len] = NULL;
  for (i = 0; i < len; i++) {
    rstr = rb_obj_as_string(RARRAY(rstop_words)->ptr[i]);
    stop_words[i] = RSTRING(rstr)->ptr;
  }
  return stop_words;
}

/*** AsciiStandardAnalyzer ***/
static VALUE
frt_a_standard_analyzer_init(int argc, VALUE *argv, VALUE self)
{
  bool lower;
  VALUE rlower, rstop_words;
  Analyzer *a;
  rb_scan_args(argc, argv, "02", &rlower, &rstop_words);
  lower = ((rlower == Qnil) ? true : RTEST(rlower));
  if (rstop_words != Qnil) {
    char **stop_words = get_stop_words(rstop_words);
    a = standard_analyzer_create_with_words((const char **)stop_words, lower);
    free(stop_words);
  } else {
    a = standard_analyzer_create(lower);
  }
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/*** StandardAnalyzer ***/
static VALUE
frt_standard_analyzer_init(int argc, VALUE *argv, VALUE self)
{
  bool lower;
  VALUE rlower, rstop_words;
  Analyzer *a;
  rb_scan_args(argc, argv, "02", &rstop_words, &rlower);
  lower = ((rlower == Qnil) ? true : RTEST(rlower));
  if (rstop_words != Qnil) {
    char **stop_words = get_stop_words(rstop_words);
    a = mb_standard_analyzer_create_with_words((const char **)stop_words, lower);
    free(stop_words);
  } else {
    a = mb_standard_analyzer_create(lower);
  }
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  object_add(a, self);
  return self;
}

/** RegexAnalyzer **/
/*
static VALUE
frt_regex_analyzer_init(VALUE self)
{
  Analyzer *a = regex_analyzer_create();
  // keine Ahnung warum hier das Makro und nicht Data_Wrap_Struct:
  Frt_Wrap_Struct(self, NULL, &frt_analyzer_free, a);
  // wofuer?:
  object_add(a, self);
  return self;
}

// convenience method
// XXX this sets the locale for the entire program
static VALUE frt_regex_analyzer_setlocale(VALUE self, VALUE locale)
{
  char *l = StringValuePtr( locale );
  setlocale(LC_ALL, l);
  return Qnil;
}

static VALUE
frt_regex_analyzer_token_stream(VALUE self, VALUE field, VALUE string)
{
  Analyzer *a =((struct RData *)(self))->data;
  TokenStream *ts = a->get_ts( a, StringValuePtr(field), StringValuePtr(string) );
  // already freed via analyzer's free()
  VALUE token_stream = Data_Wrap_Struct(cTokenStream, NULL, NULL, ts);
  return token_stream;
}
*/
/** /RegexAnalyzer **/

/** TokenStream **/
/** /TokenStream **/

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
  rb_define_method(cAnalyzer, "initialize", frt_letter_analyzer_init, -1);
  rb_define_method(cAnalyzer, "token_stream", frt_analyzer_token_stream, 2);

  cAsciiLetterAnalyzer =
    rb_define_class_under(mAnalysis, "AsciiLetterAnalyzer", cAnalyzer);
  rb_define_alloc_func(cAsciiLetterAnalyzer, frt_data_alloc);
  rb_define_method(cAsciiLetterAnalyzer, "initialize",
      frt_a_letter_analyzer_init, -1);

  cLetterAnalyzer =
    rb_define_class_under(mAnalysis, "LetterAnalyzer", cAnalyzer);
  rb_define_alloc_func(cLetterAnalyzer, frt_data_alloc);
  rb_define_method(cLetterAnalyzer, "initialize",
        frt_letter_analyzer_init, -1);

  cAsciiWhiteSpaceAnalyzer =
    rb_define_class_under(mAnalysis, "AsciiWhiteSpaceAnalyzer", cAnalyzer);
  rb_define_alloc_func(cAsciiWhiteSpaceAnalyzer, frt_data_alloc);
  rb_define_method(cAsciiWhiteSpaceAnalyzer, "initialize",
      frt_a_white_space_analyzer_init, -1);

  cWhiteSpaceAnalyzer =
    rb_define_class_under(mAnalysis, "WhiteSpaceAnalyzer", cAnalyzer);
  rb_define_alloc_func(cWhiteSpaceAnalyzer, frt_data_alloc);
  rb_define_method(cWhiteSpaceAnalyzer, "initialize",
      frt_white_space_analyzer_init, -1);

  cAsciiStandardAnalyzer =
    rb_define_class_under(mAnalysis, "AsciiStandardAnalyzer", cAnalyzer);
  rb_define_alloc_func(cAsciiStandardAnalyzer, frt_data_alloc);
  rb_define_method(cAsciiStandardAnalyzer, "initialize",
      frt_a_standard_analyzer_init, -1);

  cStandardAnalyzer =
    rb_define_class_under(mAnalysis, "StandardAnalyzer", cAnalyzer);
  rb_define_alloc_func(cStandardAnalyzer, frt_data_alloc);
  rb_define_method(cStandardAnalyzer, "initialize",
      frt_standard_analyzer_init, -1);

  /** RegexAnalyzer **/
  /*
  cRegexAnalyzer =
    rb_define_class_under(mAnalysis, "RegexAnalyzer", cAnalyzer);
  rb_define_alloc_func(cRegexAnalyzer, frt_data_alloc);
  rb_define_method(cRegexAnalyzer, "initialize",
      frt_regex_analyzer_init, 0);
  rb_define_method(cRegexAnalyzer, "token_stream",
      frt_regex_analyzer_token_stream, 2);
  rb_define_method(cRegexAnalyzer, "setlocale",
      frt_regex_analyzer_setlocale, 1);
  */

  /** TokenStream **/
  cTokenStream = rb_define_class_under(mAnalysis, "TokenStream", rb_cObject);
  rb_define_method(cTokenStream, "next", frt_token_stream_next, 0);
}

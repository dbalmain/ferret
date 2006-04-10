#include "ferret.h"
#include "analysis.h"
#include "locale.h"

static VALUE cToken;
static VALUE cAsciiLetterTokenizer;
static VALUE cLetterTokenizer;
static VALUE cAsciiWhiteSpaceTokenizer;
static VALUE cWhiteSpaceTokenizer;
static VALUE cAsciiStandardTokenizer;
static VALUE cStandardTokenizer;

static VALUE cAnalyzer;
static VALUE cAsciiLetterAnalyzer;
static VALUE cLetterAnalyzer;
static VALUE cAsciiWhiteSpaceAnalyzer;
static VALUE cWhiteSpaceAnalyzer;
static VALUE cAsciiStandardAnalyzer;
static VALUE cStandardAnalyzer;

//static VALUE cRegexAnalyzer;
static VALUE cTokenStream;

static ID id_next;
static ID id_reset;
static ID id_clone;

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
 * TokenStream Methods
 *
 ****************************************************************************/

static void
frt_ts_mark(void *p)
{
  TokenStream *ts = (TokenStream *)p;
  frt_gc_mark(&ts->text);
  if (ts->sub_ts) frt_gc_mark(ts->sub_ts);
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

static inline VALUE
get_wrapped_ts(VALUE self, VALUE rstr, TokenStream *ts)
{
  rstr = rb_obj_as_string(rstr);
  ts->reset(ts, RSTRING(rstr)->ptr);
  Frt_Wrap_Struct(self, &frt_ts_mark, &frt_ts_free, ts);
  object_add(&ts->text, rstr);
  object_add(ts, self);
  return self;
}

static VALUE
frt_ts_set_text(VALUE self, VALUE rtext)
{
  TokenStream *ts; 
  Data_Get_Struct(self, TokenStream, ts);
  rtext = rb_obj_as_string(rtext);
  ts->reset(ts, RSTRING(rtext)->ptr);
  object_set(&ts->text, rtext);

  return rtext;
}

static VALUE
frt_ts_get_text(VALUE self)
{
  VALUE rtext = Qnil;
  TokenStream *ts; 
  Data_Get_Struct(self, TokenStream, ts);
  if (ts->text) {
    if ((rtext = object_get(&ts->text)) == Qnil) {
      rtext = rb_str_new2(ts->text);
      object_set(&ts->text, rtext);
    } 
  }
  return rtext;
}

static VALUE
frt_ts_next(VALUE self)
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
 * Tokenizers
 ****************************************************************************/

#define TS_ARGS(dflt) \
  bool lower;\
  VALUE rlower, rstr;\
  rb_scan_args(argc, argv, "11", &rstr, &rlower);\
  lower = (argc ? RTEST(rlower) : dflt)

static VALUE
frt_a_letter_tokenizer_init(VALUE self, VALUE rstr) 
{
  return get_wrapped_ts(self, rstr, letter_tokenizer_create());
}

static VALUE
frt_letter_tokenizer_init(int argc, VALUE *argv, VALUE self) 
{
  TS_ARGS(false);
  return get_wrapped_ts(self, rstr, mb_letter_tokenizer_create(lower));
}

static VALUE
frt_a_whitespace_tokenizer_init(VALUE self, VALUE rstr) 
{
  return get_wrapped_ts(self, rstr, whitespace_tokenizer_create());
}

static VALUE
frt_whitespace_tokenizer_init(int argc, VALUE *argv, VALUE self) 
{
  TS_ARGS(false);
  return get_wrapped_ts(self, rstr, mb_whitespace_tokenizer_create(lower));
}

static VALUE
frt_a_standard_tokenizer_init(VALUE self, VALUE rstr) 
{
  return get_wrapped_ts(self, rstr, standard_tokenizer_create());
}

static VALUE
frt_standard_tokenizer_init(VALUE self, VALUE rstr) 
{
  return get_wrapped_ts(self, rstr, mb_standard_tokenizer_create());
}

/****************************************************************************
 * Filters
 ****************************************************************************/

Token *cwrts_next(TokenStream *ts)
{
  VALUE rts = (VALUE)ts->data;
  VALUE rtoken = rb_funcall(rts, id_next, 0);
  Data_Get_Struct(rtoken, Token, ts->token);
  return ts->token;
}

void cwrts_reset(TokenStream *ts, char *text)
{
  ts->t = ts->text = text;
  rb_funcall(rts, id_reset, 1, rb_str_new2(text));
}


static TokenStream *
get_cwrapped_rts(VALUE rts) 
{
  TokenStream *ts;
  switch (TYPE(rts)) {
    case T_DATA:
      Data_Get_Struct(rts, TokenStream, ts);
      break;
    default:
      ts = ALLOC(TokenStream *ts);
      ts->data = (void *)rts;
      ts->next = &cwrts_next;
      rb_raise(rb_eArgError, "Unknown SortField Type");
      break;
  }
  return ts;
}

static VALUE
frt_lowercase_filter_init(VALUE self, VALUE rsub_ts) 
{
  return get_wrapped_ts(self, rstr, mb_standard_tokenizer_create());
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

static VALUE
frt_analyzer_token_stream(VALUE self, VALUE rfield, VALUE rstring)
{
  Analyzer *a = ((struct RData *)(self))->data;
  rfield = rb_obj_as_string(rfield);
  rstring = rb_obj_as_string(rstring);
  
  TokenStream *ts = a_get_new_ts(a, RSTRING(rfield)->ptr, RSTRING(rstring)->ptr);

  object_set(&ts->text, rstring); // Make sure that there is no entry already
  return get_token_stream(ts);
}

#define GET_LOWER(dflt) \
  bool lower;\
  VALUE rlower;\
  rb_scan_args(argc, argv, "01", &rlower);\
  lower = (argc ? RTEST(rlower) : dflt)

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
get_stopwords(VALUE rstop_words)
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

static VALUE
get_rstopwords(const char **stop_words)
{
  char **w = (char **)stop_words;
  VALUE rstopwords = rb_ary_new();

  while (*w) {
    rb_ary_push(rstopwords, rb_str_new2(*w));
    w++;
  }
  return rstopwords;
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
    char **stop_words = get_stopwords(rstop_words);
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
    char **stop_words = get_stopwords(rstop_words);
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
 * Locale stuff
 *
 ****************************************************************************/

static char *frt_locale = NULL;

static VALUE frt_getlocale(VALUE self, VALUE locale)
{
  return (frt_locale ? rb_str_new2(frt_locale) : Qnil);
}

static VALUE frt_setlocale(VALUE self, VALUE locale)
{
  char *l = ((locale == Qnil) ? NULL : RSTRING(rb_obj_as_string(locale))->ptr);
  frt_locale = setlocale(LC_ALL, l);
  return frt_locale ? rb_str_new2(frt_locale) : Qnil;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_analysis(void)
{
	id_next = rb_intern("next");
	id_reset = rb_intern("text=");
	id_clone = rb_intern("clone");

  /*** * * Locale stuff * * ***/
  frt_locale = setlocale(LC_ALL, "");
  rb_define_singleton_method(mFerret, "locale=", frt_setlocale, 1);
  rb_define_singleton_method(mFerret, "locale", frt_getlocale, 0);

  /*********************/
  /*** * * Token * * ***/
  /*********************/
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

  /****************************/
  /*** * * TokenStreams * * ***/
  /****************************/

  cTokenStream = rb_define_class_under(mAnalysis, "TokenStream", rb_cObject);
  rb_define_method(cTokenStream, "next", frt_ts_next, 0);
  rb_define_method(cTokenStream, "text=", frt_ts_set_text, 1);
  rb_define_method(cTokenStream, "text", frt_ts_get_text, 0);

  /******************/
  /*** Tokenizers ***/
  /******************/

  /*** * * AsciiLetterTokenizer * * ***/
  cAsciiLetterTokenizer =
    rb_define_class_under(mAnalysis, "AsciiLetterTokenizer", cTokenStream);
  rb_define_alloc_func(cAsciiLetterTokenizer, frt_data_alloc);
  rb_define_method(cAsciiLetterTokenizer, "initialize",
      frt_a_letter_tokenizer_init, 1);

  /*** * * LetterTokenizer * * ***/
  cLetterTokenizer =
    rb_define_class_under(mAnalysis, "LetterTokenizer", cTokenStream);
  rb_define_alloc_func(cLetterTokenizer, frt_data_alloc);
  rb_define_method(cLetterTokenizer, "initialize",
      frt_letter_tokenizer_init, -1);

  /*** * * AsciiWhiteSpaceTokenizer * * ***/
  cAsciiWhiteSpaceTokenizer =
    rb_define_class_under(mAnalysis, "AsciiWhiteSpaceTokenizer", cTokenStream);
  rb_define_alloc_func(cAsciiWhiteSpaceTokenizer, frt_data_alloc);
  rb_define_method(cAsciiWhiteSpaceTokenizer, "initialize",
      frt_a_whitespace_tokenizer_init, 1);

  /*** * * WhiteSpaceTokenizer * * ***/
  cWhiteSpaceTokenizer =
    rb_define_class_under(mAnalysis, "WhiteSpaceTokenizer", cTokenStream);
  rb_define_alloc_func(cWhiteSpaceTokenizer, frt_data_alloc);
  rb_define_method(cWhiteSpaceTokenizer, "initialize",
      frt_whitespace_tokenizer_init, -1);

  /*** * * AsciiStandardTokenizer * * ***/
  cAsciiStandardTokenizer =
    rb_define_class_under(mAnalysis, "AsciiStandardTokenizer", cTokenStream);
  rb_define_alloc_func(cAsciiStandardTokenizer, frt_data_alloc);
  rb_define_method(cAsciiStandardTokenizer, "initialize",
      frt_a_standard_tokenizer_init, 1);

  /*** * * StandardTokenizer * * ***/
  cStandardTokenizer =
    rb_define_class_under(mAnalysis, "StandardTokenizer", cTokenStream);
  rb_define_alloc_func(cStandardTokenizer, frt_data_alloc);
  rb_define_method(cStandardTokenizer, "initialize",
      frt_standard_tokenizer_init, 1);

  /***************/
  /*** Filters ***/
  /***************/
  rb_define_const(mAnalysis, "ENGLISH_STOP_WORDS",
      get_rstopwords(ENGLISH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_ENGLISH_STOP_WORDS",
      get_rstopwords(FULL_ENGLISH_STOP_WORDS));
  rb_define_const(mAnalysis, "EXTENDED_ENGLISH_STOP_WORDS",
      get_rstopwords(EXTENDED_ENGLISH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_FRENCH_STOP_WORDS",
      get_rstopwords(FULL_FRENCH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_SPANISH_STOP_WORDS",
      get_rstopwords(FULL_SPANISH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_PORTUGUESE_STOP_WORDS",
      get_rstopwords(FULL_PORTUGUESE_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_ITALIAN_STOP_WORDS",
      get_rstopwords(FULL_ITALIAN_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_GERMAN_STOP_WORDS",
      get_rstopwords(FULL_GERMAN_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_DUTCH_STOP_WORDS",
      get_rstopwords(FULL_DUTCH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_SWEDISH_STOP_WORDS",
      get_rstopwords(FULL_SWEDISH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_NORWEGIAN_STOP_WORDS",
      get_rstopwords(FULL_NORWEGIAN_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_DANISH_STOP_WORDS",
      get_rstopwords(FULL_DANISH_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_RUSSIAN_STOP_WORDS",
      get_rstopwords(FULL_RUSSIAN_STOP_WORDS));
  rb_define_const(mAnalysis, "FULL_FINNISH_STOP_WORDS",
      get_rstopwords(FULL_FINNISH_STOP_WORDS));

  /*************************/
  /*** * * Analyzers * * ***/
  /*************************/

  /*** * * Analyzer * * ***/
  cAnalyzer =
    rb_define_class_under(mAnalysis, "Analyzer", rb_cObject);
  rb_define_alloc_func(cAnalyzer, frt_data_alloc);
  rb_define_method(cAnalyzer, "initialize", frt_letter_analyzer_init, -1);
  rb_define_method(cAnalyzer, "token_stream", frt_analyzer_token_stream, 2);

  /*** * * AsciiLetterAnalyzer * * ***/
  cAsciiLetterAnalyzer =
    rb_define_class_under(mAnalysis, "AsciiLetterAnalyzer", cAnalyzer);
  rb_define_alloc_func(cAsciiLetterAnalyzer, frt_data_alloc);
  rb_define_method(cAsciiLetterAnalyzer, "initialize",
      frt_a_letter_analyzer_init, -1);

  /*** * * LetterAnalyzer * * ***/
  cLetterAnalyzer =
    rb_define_class_under(mAnalysis, "LetterAnalyzer", cAnalyzer);
  rb_define_alloc_func(cLetterAnalyzer, frt_data_alloc);
  rb_define_method(cLetterAnalyzer, "initialize",
        frt_letter_analyzer_init, -1);

  /*** * * AsciiWhiteSpaceAnalyzer * * ***/
  cAsciiWhiteSpaceAnalyzer =
    rb_define_class_under(mAnalysis, "AsciiWhiteSpaceAnalyzer", cAnalyzer);
  rb_define_alloc_func(cAsciiWhiteSpaceAnalyzer, frt_data_alloc);
  rb_define_method(cAsciiWhiteSpaceAnalyzer, "initialize",
      frt_a_white_space_analyzer_init, -1);

  /*** * * WhiteSpaceAnalyzer * * ***/
  cWhiteSpaceAnalyzer =
    rb_define_class_under(mAnalysis, "WhiteSpaceAnalyzer", cAnalyzer);
  rb_define_alloc_func(cWhiteSpaceAnalyzer, frt_data_alloc);
  rb_define_method(cWhiteSpaceAnalyzer, "initialize",
      frt_white_space_analyzer_init, -1);

  /*** * * AsciiStandardAnalyzer * * ***/
  cAsciiStandardAnalyzer =
    rb_define_class_under(mAnalysis, "AsciiStandardAnalyzer", cAnalyzer);
  rb_define_alloc_func(cAsciiStandardAnalyzer, frt_data_alloc);
  rb_define_method(cAsciiStandardAnalyzer, "initialize",
      frt_a_standard_analyzer_init, -1);

  /*** * * StandardAnalyzer * * ***/
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

}

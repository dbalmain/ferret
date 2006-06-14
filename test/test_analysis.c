#include "test.h"
#include "analysis.h"
#include <string.h>
#include <locale.h>
#include <libstemmer.h>

#define test_token(mtk, mstr, mstart, mend) \
  tt_token(mtk, mstr, mstart, mend, tc, __LINE__)

void tt_token(Token *tk,
              char *str, int start, int end, tst_case *tc, int line_num)
{
    Token tk_exp;
    static char buf[3000];
    if (!tk_eq(tk_set(&tk_exp, str, (int)strlen(str), start, end, 1), tk)) {
        sprintf(buf, "Token1[%d:%d:%s] != Token2[%d:%d:%s]\n",
                tk->start, tk->end, tk->text, start, end, str);
        tst_assert(line_num, tc, false, buf);
    }
}

#define test_token_pi(mtk, mstr, mstart, mend, mpi) \
  tt_token_pi(mtk, mstr, mstart, mend, mpi, tc, __LINE__)

void tt_token_pi(Token *tk,
                 char *str,
                 int start, int end, int pi, tst_case *tc, int line_num)
{
    Token tk_exp;
    static char buf[3000];
    if (!tk_eq(tk_set(&tk_exp, str, (int)strlen(str), start, end, pi), tk)) {
        sprintf(buf, "Token1[%d:%d:%s-%d] != Token2[%d:%d:%s-%d]\n",
                tk->start, tk->end, tk->text, tk->pos_inc, start, end, str,
                pi);
        tst_assert(line_num, tc, false, buf);
    }
}

void test_tk(tst_case *tc, void *data)
{
    Token *tk1 = tk_new();
    Token *tk2 = tk_new();
    (void)data;

    tk_set_no_len(tk1, "DBalmain", 1, 8, 5);
    tk_set_no_len(tk2, "DBalmain", 1, 8, 5);
    Assert(tk_eq(tk1, tk2), "tokens are equal");
    tk_set_no_len(tk2, "DBalmain", 1, 8, 1);
    Assert(tk_eq(tk1, tk2), "tokens are equal");

    tk_set_no_len(tk2, "CBalmain", 1, 8, 5);
    Assert(!tk_eq(tk1, tk2), "tokens aren't equal");
    tk_set_no_len(tk2, "DBalmain", 0, 8, 5);
    Assert(!tk_eq(tk1, tk2), "tokens aren't equal");
    tk_set_no_len(tk2, "DBalmain", 1, 7, 5);
    Assert(!tk_eq(tk1, tk2), "tokens aren't equal");

    tk_set_no_len(tk2, "CBalmain", 2, 7, 1);
    Aiequal(-1, tk_cmp(tk1, tk2));
    tk_set_no_len(tk2, "EBalmain", 0, 9, 1);
    Aiequal(1, tk_cmp(tk1, tk2));
    tk_set_no_len(tk2, "CBalmain", 1, 9, 1);
    Aiequal(-1, tk_cmp(tk1, tk2));
    tk_set_no_len(tk2, "EBalmain", 1, 7, 1);
    Aiequal(1, tk_cmp(tk1, tk2));
    tk_set_no_len(tk2, "EBalmain", 1, 8, 1);
    Aiequal(-1, tk_cmp(tk1, tk2));
    tk_set_no_len(tk2, "CBalmain", 1, 8, 1);
    Aiequal(1, tk_cmp(tk1, tk2));

    Asequal("DBalmain", tk1->text);
    sprintf(tk1->text, "Hello");
    Asequal("Hello", tk1->text);
    Aiequal(1, tk1->start);
    Aiequal(8, tk1->end);
    Aiequal(5, tk1->pos_inc);
    tk_destroy(tk1);
    tk_destroy(tk2);
}

/****************************************************************************
 *
 * Whitespace
 *
 ****************************************************************************/

void test_whitespace_tokenizer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = whitespace_tokenizer_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#@$";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

void test_mb_whitespace_tokenizer(tst_case *tc, void *data)
{
    Token *t, *tk = tk_new();
    TokenStream *ts = mb_whitespace_tokenizer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#@$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    test_token(t = ts_next(ts), "ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts = mb_lowercase_filter_new(ts);
    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    test_token(ts_next(ts), "áägç®êëì¯úøã¬öîí", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    ts = mb_whitespace_tokenizer_new(true);
    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    test_token(ts_next(ts), "áägç®êëì¯úøã¬öîí", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
    tk_destroy(tk);
}

void test_whitespace_analyzer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = whitespace_analyzer_new(false);
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#@$";
    TokenStream *ts = a_get_ts(a, "random", text);
    (void)data;

    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    a_deref(a);
}

void test_mb_whitespace_analyzer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = mb_whitespace_analyzer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#@$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, "random", text);
    (void)data;

    test_token(ts_next(ts), "DBalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    test_token(ts_next(ts), "ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    a = mb_whitespace_analyzer_new(true);
    ts = a_get_ts(a, "random", text);
    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    test_token(ts_next(ts), "áägç®êëì¯úøã¬öîí", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    a_deref(a);
}

/****************************************************************************
 *
 * Letter
 *
 ****************************************************************************/

void test_letter_tokenizer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = letter_tokenizer_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#@$";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmain", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

void test_mb_letter_tokenizer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = mb_letter_tokenizer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#@$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmän", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "ÁÄGÇ", 55, 62);
    test_token(ts_next(ts), "ÊËÌ", 64, 70);
    test_token(ts_next(ts), "ÚØÃ", 72, 78);
    test_token(ts_next(ts), "ÖÎÍ", 80, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts = mb_lowercase_filter_new(ts);
    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmän", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "áägç", 55, 62);
    test_token(ts_next(ts), "êëì", 64, 70);
    test_token(ts_next(ts), "úøã", 72, 78);
    test_token(ts_next(ts), "öîí", 80, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    ts = mb_letter_tokenizer_new(true);
    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmän", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "áägç", 55, 62);
    test_token(ts_next(ts), "êëì", 64, 70);
    test_token(ts_next(ts), "úøã", 72, 78);
    test_token(ts_next(ts), "öîí", 80, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
    tk_destroy(tk);
}

void test_letter_analyzer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = letter_analyzer_new(true);
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#@$";
    TokenStream *ts = a_get_ts(a, "random", text);
    (void)data;

    test_token(ts_next(ts), "dbalmain", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    a_deref(a);
}

void test_mb_letter_analyzer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = mb_letter_analyzer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#@$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, "random", text);
    (void)data;

    test_token(ts_next(ts), "DBalmän", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "ÁÄGÇ", 55, 62);
    test_token(ts_next(ts), "ÊËÌ", 64, 70);
    test_token(ts_next(ts), "ÚØÃ", 72, 78);
    test_token(ts_next(ts), "ÖÎÍ", 80, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    a = mb_letter_analyzer_new(true);
    ts = a_get_ts(a, "random", text);
    test_token(ts_next(ts), "dbalmän", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "áägç", 55, 62);
    test_token(ts_next(ts), "êëì", 64, 70);
    test_token(ts_next(ts), "úøã", 72, 78);
    test_token(ts_next(ts), "öîí", 80, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    tk_destroy(tk);
}


/****************************************************************************
 *
 * Standard
 *
 ****************************************************************************/

void test_standard_tokenizer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = standard_tokenizer_new();
    char text[200] =
        "DBalmain@gmail.com is My e-mail 52   #$ Address. 23#@$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "Address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 84);
    test_token(ts_next(ts), "TNT", 86, 91);
    test_token(ts_next(ts), "123-1235-ASD-1234", 93, 110);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

void test_mb_standard_tokenizer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = mb_standard_tokenizer_new();
    char text[200] =
        "DBalmán@gmail.com is My e-mail 52   #$ Address. 23#@$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 23#@$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmán@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "Address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 84);
    test_token(ts_next(ts), "TNT", 86, 91);
    test_token(ts_next(ts), "123-1235-ASD-1234", 93, 110);
    test_token(ts_next(ts), "23", 111, 113);
    test_token(ts_next(ts), "ÁÄGÇ", 117, 124);
    test_token(ts_next(ts), "ÊËÌ", 126, 132);
    test_token(ts_next(ts), "ÚØÃ", 134, 140);
    test_token(ts_next(ts), "ÖÎÍ", 142, 148);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

void test_standard_analyzer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, true);
    char text[200] =
        "DBalmain@gmail.com is My e-mail and the Address. 23#@$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234";
    TokenStream *ts = a_get_ts(a, "random", text);
    (void)data;

    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 2);
    test_token_pi(ts_next(ts), "e-mail", 25, 31, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 3);
    test_token_pi(ts_next(ts), "23", 49, 51, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 84, 1);
    test_token_pi(ts_next(ts), "tnt", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    a_deref(a);
}

void test_mb_standard_analyzer(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        mb_standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, false);
    const char *words[] = { "is", "the", "23", "tnt", NULL };
    char text[200] =
        "DBalmán@gmail.com is My e-mail and the Address. 23#@$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 23#@$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, "random", text), *ts2;
    (void)data;

    test_token(ts_next(ts), "DBalmán@gmail.com", 0, 18);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "Address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 84);
    test_token(ts_next(ts), "TNT", 86, 91);
    test_token(ts_next(ts), "123-1235-ASD-1234", 93, 110);
    test_token(ts_next(ts), "23", 111, 113);
    test_token(ts_next(ts), "ÁÄGÇ", 117, 124);
    test_token(ts_next(ts), "ÊËÌ", 126, 132);
    test_token(ts_next(ts), "ÚØÃ", 134, 140);
    test_token(ts_next(ts), "ÖÎÍ", 142, 148);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    a = mb_standard_analyzer_new(true);
    ts = a_get_ts(a, "random", text);
    test_token(ts_next(ts), "dbalmán@gmail.com", 0, 18);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 84);
    test_token(ts_next(ts), "tnt", 86, 91);
    test_token(ts_next(ts), "123-1235-asd-1234", 93, 110);
    test_token(ts_next(ts), "23", 111, 113);
    test_token(ts_next(ts), "áägç", 117, 124);
    test_token(ts_next(ts), "êëì", 126, 132);
    test_token(ts_next(ts), "úøã", 134, 140);
    test_token(ts_next(ts), "öîí", 142, 148);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    a = mb_standard_analyzer_new_with_words(words, true);
    ts = a_get_new_ts(a, "random", text);
    ts2 = a_get_new_ts(a, "random", text);
    test_token(ts_next(ts), "dbalmán@gmail.com", 0, 18);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "and", 32, 35);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "www.google.com/results", 55, 84);
    test_token(ts_next(ts), "123-1235-asd-1234", 93, 110);
    test_token(ts_next(ts), "áägç", 117, 124);
    test_token(ts_next(ts), "êëì", 126, 132);
    test_token(ts_next(ts), "úøã", 134, 140);
    test_token(ts_next(ts), "öîí", 142, 148);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    test_token(ts_next(ts2), "dbalmán@gmail.com", 0, 18);
    test_token(ts_next(ts2), "my", 22, 24);
    test_token(ts_next(ts2), "e-mail", 25, 31);
    test_token(ts_next(ts2), "and", 32, 35);
    test_token(ts_next(ts2), "address", 40, 47);
    test_token(ts_next(ts2), "www.google.com/results", 55, 84);
    test_token(ts_next(ts2), "123-1235-asd-1234", 93, 110);
    test_token(ts_next(ts2), "áägç", 117, 124);
    test_token(ts_next(ts2), "êëì", 126, 132);
    test_token(ts_next(ts2), "úøã", 134, 140);
    test_token(ts_next(ts2), "öîí", 142, 148);
    Assert(ts_next(ts2) == NULL, "Should be no more tokens");
    ts2->ref_cnt = 3;
    ts = ts_clone(ts2);
    Aiequal(3, ts2->ref_cnt);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts2);
    Aiequal(2, ts2->ref_cnt);
    ts_deref(ts2);
    Aiequal(1, ts2->ref_cnt);
    ts_deref(ts2);
    ts_deref(ts);
    a_deref(a);
    tk_destroy(tk);
}

void test_long_word(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, true);
    char text[400] =
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" " two";
    TokenStream *ts = a_get_ts(a, "random", text);
    (void)data;

    test_token_pi(ts_next(ts), text, 0, 290, 1);        /* text gets truncated anyway */
    test_token_pi(ts_next(ts), "two", 291, 294, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    a = mb_standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, true);
    ts = a_get_ts(a, "random", text);
    test_token_pi(ts_next(ts), text, 0, 290, 1);        /* text gets truncated anyway */
    test_token_pi(ts_next(ts), "two", 291, 294, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    a_deref(a);
    tk_destroy(tk);
}

/****************************************************************************
 *
 * Filters
 *
 ****************************************************************************/

void test_lowercase_filter(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = lowercase_filter_new(standard_tokenizer_new());
    char text[200] =
        "DBalmain@gmail.com is My e-mail 52   #$ Address. 23#@$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 84);
    test_token(ts_next(ts), "tnt", 86, 91);
    test_token(ts_next(ts), "123-1235-asd-1234", 93, 110);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

const char *words[] = { "one", "four", "five", "seven", NULL };
void test_stop_filter(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts =
        stop_filter_new_with_words(letter_tokenizer_new(), words);
    char text[200] =
        "one, two, three, four, five, six, seven, eight, nine, ten.";
    (void)data;

    ts->reset(ts, text);
    test_token_pi(ts_next(ts), "two", 5, 8, 2);
    test_token_pi(ts_next(ts), "three", 10, 15, 1);
    test_token_pi(ts_next(ts), "six", 29, 32, 3);
    test_token_pi(ts_next(ts), "eight", 41, 46, 2);
    test_token_pi(ts_next(ts), "nine", 48, 52, 1);
    test_token_pi(ts_next(ts), "ten", 54, 57, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

void test_stemmer(tst_case *tc, void *data)
{
    int stemmer_cnt = 0;
    const char **stemmers = sb_stemmer_list();
    const char **st = stemmers;
    struct sb_stemmer *stemmer;
    (void)data;

    while (*st) {
        stemmer_cnt++;
        /* printf("%d. -> %s\n", stemmer_cnt, *st); */
        st++;
    }

    stemmer = sb_stemmer_new("english", NULL);

    Apnotnull(stemmer);
    sb_stemmer_delete(stemmer);
    Assert(stemmer_cnt >= 13, "There should be at least 10 stemmers");
}

void test_stem_filter(tst_case *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = stem_filter_new(mb_letter_tokenizer_new(true),
                                         "english", NULL);
    TokenStream *ts2;
    char text[200] = "debate debates debated debating debater";
    char text2[200] = "dêbate dêbates dêbated dêbating dêbater";
    (void)data;

    ts->reset(ts, text);
    ts2 = ts_clone(ts);
    test_token(ts_next(ts), "debat", 0, 6);
    test_token(ts_next(ts), "debat", 7, 14);
    test_token(ts_next(ts), "debat", 15, 22);
    test_token(ts_next(ts), "debat", 23, 31);
    test_token(ts_next(ts), "debat", 32, 39);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts->reset(ts, text2);
    test_token(ts_next(ts), "dêbate", 0, 7);
    test_token(ts_next(ts), "dêbate", 8, 16);
    test_token(ts_next(ts), "dêbate", 17, 25);
    test_token(ts_next(ts), "dêbate", 26, 35);
    test_token(ts_next(ts), "dêbater", 36, 44);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    REF(ts);
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
    test_token(ts_next(ts2), "debat", 0, 6);
    test_token(ts_next(ts2), "debat", 7, 14);
    test_token(ts_next(ts2), "debat", 15, 22);
    test_token(ts_next(ts2), "debat", 23, 31);
    test_token(ts_next(ts2), "debat", 32, 39);
    Assert(ts_next(ts2) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts2);
}

void test_per_field_analyzer(tst_case *tc, void *data)
{
    TokenStream *ts;
    Token *tk = tk_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#@$";
    Analyzer *pfa = per_field_analyzer_new(standard_analyzer_new(true));
    (void)data;

    pfa_add_field(pfa, "white", whitespace_analyzer_new(false));
    pfa_add_field(pfa, "white_l", whitespace_analyzer_new(true));
    pfa_add_field(pfa, "letter", letter_analyzer_new(false));
    pfa_add_field(pfa, "letter", letter_analyzer_new(true));
    pfa_add_field(pfa, "letter_u", letter_analyzer_new(false));
    ts = a_get_ts(pfa, "white", text);
    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts = a_get_ts(pfa, "white_l", text);
    test_token(ts_next(ts), "dbalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#@$", 49, 54);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts = a_get_ts(pfa, "letter_u", text);
    test_token(ts_next(ts), "DBalmain", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts = a_get_new_ts(pfa, "letter", text);
    test_token(ts_next(ts), "dbalmain", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    /* used a_get_new_ts so must destroy ts */
    ts_deref(ts);
    ts = a_get_new_ts(pfa, "XXX", text);        /* should use default analyzer */
    test_token(ts_next(ts), "dbalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    /* used a_get_new_ts so must destroy ts */
    ts_deref(ts);
    a_deref(pfa);
}

tst_suite *ts_analysis(tst_suite *suite)
{
#ifdef WIN32
    bool u = false;

    setlocale(LC_ALL, ".65001");
#else
    bool u = true;

    setlocale(LC_ALL, "");
#endif

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_tk, NULL);

    /* Whitespace */
    tst_run_test(suite, test_whitespace_tokenizer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_whitespace_tokenizer, NULL);
    }

    tst_run_test(suite, test_whitespace_analyzer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_whitespace_analyzer, NULL);
    }

    /* Letter */
    tst_run_test(suite, test_letter_tokenizer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_letter_tokenizer, NULL);
    }

    tst_run_test(suite, test_letter_analyzer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_letter_analyzer, NULL);
    }

    /* Standard */
    tst_run_test(suite, test_standard_tokenizer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_standard_tokenizer, NULL);
    }
    tst_run_test(suite, test_standard_analyzer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_standard_analyzer, NULL);
    }

    tst_run_test(suite, test_long_word, NULL);

    /* PerField */
    tst_run_test(suite, test_per_field_analyzer, NULL);

    /* Filters */
    tst_run_test(suite, test_lowercase_filter, NULL);
    tst_run_test(suite, test_stop_filter, NULL);
    tst_run_test(suite, test_stemmer, NULL);
    if (u) {
        tst_run_test(suite, test_stem_filter, NULL);
    }

    return suite;
}

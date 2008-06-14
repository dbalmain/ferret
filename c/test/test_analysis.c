#include "analysis.h"
#include <string.h>
#include <locale.h>
#include <libstemmer.h>
#include "test.h"

#define test_token(mtk, mstr, mstart, mend) \
  tt_token(mtk, mstr, mstart, mend, tc, __LINE__)

static void tt_token(Token *tk,
                     char *str, int start, int end, TestCase *tc, int line_num)
{
    Token tk_exp;
    static char buf[3000];
    if (tk == NULL) {
        sprintf(buf, "Token1[NULL] != Token2[%d:%d:%s]\n",
                start, end, str);
        tst_assert(line_num, tc, false, buf);
        return;
    }
    if (!tk_eq(tk_set(&tk_exp, str, (int)strlen(str), start, end, 1), tk)) {
        sprintf(buf, "Token1[%d:%d:%s] != Token2[%d:%d:%s]\n",
                (int)tk->start, (int)tk->end, tk->text, start, end, str);
        tst_assert(line_num, tc, false, buf);
    }
    tst_int_equal(line_num, tc, strlen(tk->text), tk->len);
}

#define test_token_pi(mtk, mstr, mstart, mend, mpi) \
  tt_token_pi(mtk, mstr, mstart, mend, mpi, tc, __LINE__)

static void tt_token_pi(Token *tk, char *str,
                        int start, int end, int pi, TestCase *tc, int line_num)
{
    Token tk_exp;
    static char buf[3000];
    if (tk == NULL) {
        sprintf(buf, "Token1[NULL] != Token2[%d:%d:%s]\n",
                start, end, str);
        tst_assert(line_num, tc, false, buf);
        return;
    }
    if (!tk_eq(tk_set(&tk_exp, str, (int)strlen(str), start, end, pi), tk)) {
        sprintf(buf, "Token1[%d:%d:%s-%d] != Token2[%d:%d:%s-%d]\n",
                (int)tk->start, (int)tk->end, tk->text, tk->pos_inc,
                start, end, str, pi);
        tst_assert(line_num, tc, false, buf);
    }
    tst_int_equal(line_num, tc, strlen(tk->text), tk->len);
}

static void test_tk(TestCase *tc, void *data)
{
    Token *tk1 = tk_new();
    Token *tk2 = tk_new();
    (void)data;

    tk_set_no_len(tk1, "DBalmain", 1, 8, 5);
    tk_set_no_len(tk2, "DBalmain", 1, 8, 5);
    Assert(tk_eq(tk1, tk2), "tokens are equal");
    tk_set_no_len(tk2, "DBalmain", 1, 8, 1);
    Assert(!tk_eq(tk1, tk2), "tokens are not equal");

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
 * Non
 *
 ****************************************************************************/

static void test_non_tokenizer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = non_tokenizer_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#!$";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), text, 0, strlen(text));
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

static void test_non_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = non_analyzer_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#!$";
    TokenStream *ts = a_get_ts(a, I("random"), text);
    (void)data;

    test_token(ts_next(ts), text, 0, strlen(text));
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts);
    a_deref(a);
}

/****************************************************************************
 *
 * Whitespace
 *
 ****************************************************************************/

static void test_whitespace_tokenizer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = whitespace_tokenizer_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#!$";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#!$", 49, 54);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

static void test_mb_whitespace_tokenizer(TestCase *tc, void *data)
{
    Token *t, *tk = tk_new();
    TokenStream *ts = mb_whitespace_tokenizer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#!$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#!$", 49, 54);
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
    test_token(ts_next(ts), "23#!$", 49, 54);
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
    test_token(ts_next(ts), "23#!$", 49, 54);
    test_token(ts_next(ts), "áägç®êëì¯úøã¬öîí", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
    tk_destroy(tk);
}

static void test_whitespace_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = whitespace_analyzer_new(false);
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#!$";
    TokenStream *ts = a_get_ts(a, I("random"), text);
    (void)data;

    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#!$", 49, 54);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts);
    a_deref(a);
}

static void test_mb_whitespace_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = mb_whitespace_analyzer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#!$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, I("random"), text);
    (void)data;

    test_token(ts_next(ts), "DBalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#!$", 49, 54);
    test_token(ts_next(ts), "ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    a = mb_whitespace_analyzer_new(true);
    ts = a_get_ts(a, I("random"), text);
    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmän@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "#$", 37, 39);
    test_token(ts_next(ts), "address.", 40, 48);
    test_token(ts_next(ts), "23#!$", 49, 54);
    test_token(ts_next(ts), "áägç®êëì¯úøã¬öîí", 55, 86);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts);
    a_deref(a);
}

/****************************************************************************
 *
 * Letter
 *
 ****************************************************************************/

static void test_letter_tokenizer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = letter_tokenizer_new();
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#!$";
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

static void test_mb_letter_tokenizer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = mb_letter_tokenizer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#!$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
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

static void test_letter_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = letter_analyzer_new(true);
    char text[100] = "DBalmain@gmail.com is My e-mail 52   #$ address. 23#!$";
    TokenStream *ts = a_get_ts(a, I("random"), text);
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
    ts_deref(ts);
    a_deref(a);
}

static void test_mb_letter_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a = mb_letter_analyzer_new(false);
    char text[100] =
        "DBalmän@gmail.com is My e-mail 52   #$ address. 23#!$ "
        "ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, I("random"), text);
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
    ts_deref(ts);
    a_deref(a);
    a = mb_letter_analyzer_new(true);
    ts = a_get_ts(a, I("random"), text);
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
    ts_deref(ts);
    tk_destroy(tk);
}


/****************************************************************************
 *
 * Standard
 *
 ****************************************************************************/

static void do_standard_tokenizer(TestCase *tc, TokenStream *ts)
{
    Token *tk = tk_new();
    char text[200] =
        "DBalmain@gmail.com is My e-mail -52  #$ Address. 23#!$ "
        "http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 "
        "underscored_word, won't we're";

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "-52", 32, 35);
    test_token(ts_next(ts), "Address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 85);
    test_token(ts_next(ts), "TNT", 86, 91);
    test_token(ts_next(ts), "123-1235-ASD-1234", 93, 110);
    test_token(ts_next(ts), "underscored_word", 111, 127);
    test_token(ts_next(ts), "won't", 129, 134);
    test_token(ts_next(ts), "we're", 135, 140);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts->reset(ts, "http://xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    test_token(ts_next(ts), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxx", 0, 280);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
}

static void test_standard_tokenizer(TestCase *tc, void *data)
{
    TokenStream *ts = standard_tokenizer_new();
    (void)data;
    do_standard_tokenizer(tc, ts);
    ts_deref(ts);
}

static void test_legacy_standard_tokenizer(TestCase *tc, void *data)
{
    TokenStream *ts = legacy_standard_tokenizer_new();
    (void)data;
    do_standard_tokenizer(tc, ts);
    ts_deref(ts);
}

static void do_mb_standard_tokenizer(TestCase *tc, TokenStream *ts)
{
    Token *tk = tk_new();
    char text[512] =
        "DBalmain@gmail.com is My e-mail -52  #$ Address. 23#!$ "
        "http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 "
        "underscored_word, won't we're 23#!$ ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ "
        "\200 badchar it's groups' Barnes&Noble file:///home/user/ "
        "svn://www.davebalmain.com/ www,.google.com www.google.com "
        "dave@balmain@gmail.com \"quoted string\" continue *star";

    ts->reset(ts, text);
    test_token(ts_next(ts), "DBalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "-52", 32, 35);
    test_token(ts_next(ts), "Address", 40, 47);
    test_token(ts_next(ts), "23", 49, 51);
    test_token(ts_next(ts), "www.google.com/results", 55, 85);
    test_token(ts_next(ts), "TNT", 86, 91);
    test_token(ts_next(ts), "123-1235-ASD-1234", 93, 110);
    test_token(ts_next(ts), "underscored_word", 111, 127);
    test_token(ts_next(ts), "won't", 129, 134);
    test_token(ts_next(ts), "we're", 135, 140);
    test_token(ts_next(ts), "23", 141, 143);
    test_token(ts_next(ts), "ÁÄGÇ", 147, 154);
    test_token(ts_next(ts), "ÊËÌ", 156, 162);
    test_token(ts_next(ts), "ÚØÃ", 164, 170);
    test_token(ts_next(ts), "ÖÎÍ", 172, 178);
    test_token(ts_next(ts), "badchar", 181, 188);
    test_token(ts_next(ts), "it", 189, 193);
    test_token(ts_next(ts), "groups", 194, 201);
    test_token(ts_next(ts), "Barnes&Noble", 202, 214);
    test_token(ts_next(ts), "home/user", 215, 233);
    test_token(ts_next(ts), "svn://www.davebalmain.com", 234, 260);
    test_token(ts_next(ts), "www", 261, 264);
    test_token(ts_next(ts), "google.com", 266, 276);
    test_token(ts_next(ts), "www.google.com", 277, 291);
    test_token(ts_next(ts), "dave@balmain", 292, 304);
    test_token(ts_next(ts), "gmail.com", 305, 314);
    test_token(ts_next(ts), "quoted", 316, 322);
    test_token(ts_next(ts), "string", 323, 329);
    test_token(ts_next(ts), "continue", 331, 339);
    test_token(ts_next(ts), "star", 341, 345);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);                    /* test ref_cnt */
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts->reset(ts, "http://xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    test_token(ts_next(ts), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxx", 0, 280);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts->reset(ts, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    test_token(ts_next(ts), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxx", 0, 348);
}

static void test_mb_standard_tokenizer(TestCase *tc, void *data)
{
    TokenStream *ts = mb_standard_tokenizer_new();
    (void)data;
    do_mb_standard_tokenizer(tc, ts);
    ts_deref(ts);
}

static void test_mb_legacy_standard_tokenizer(TestCase *tc, void *data)
{
    TokenStream *ts = mb_legacy_standard_tokenizer_new();
    (void)data;
    do_mb_standard_tokenizer(tc, ts);
    ts_deref(ts);
}

static void test_standard_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, true);
    char text[200] =
        "DBalmain@gmail.com is My e-mail and the Address. -23!$ "
        "http://www.google.com/results/ T.N.T. 123-1235-ASD-1234";
    TokenStream *ts = a_get_ts(a, I("random"), text);
    (void)data;

    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 2);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 3);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "tnt", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts);
    a_deref(a);
}

static void test_mb_standard_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        mb_standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, false);
    const char *words[] = { "is", "the", "-23", "tnt", NULL };
    char text[200] =
        "DBalmain@gmail.com is My e-mail and the Address. -23!$ "
        "http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 23#!$ "
        "ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, I("random"), text), *ts2;
    (void)data;

    test_token_pi(ts_next(ts), "DBalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "My", 22, 24, 2);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "Address", 40, 47, 3);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "TNT", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-ASD-1234", 93, 110, 1);
    test_token_pi(ts_next(ts), "23", 111, 113, 1);
    test_token_pi(ts_next(ts), "ÁÄGÇ", 117, 124, 1);
    test_token_pi(ts_next(ts), "ÊËÌ", 126, 132, 1);
    test_token_pi(ts_next(ts), "ÚØÃ", 134, 140, 1);
    test_token_pi(ts_next(ts), "ÖÎÍ", 142, 148, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    a = mb_standard_analyzer_new(true);
    ts = a_get_ts(a, I("random"), text);
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "email", 25, 31, 3);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 3);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "tnt", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 1);
    test_token_pi(ts_next(ts), "23", 111, 113, 1);
    test_token_pi(ts_next(ts), "áägç", 117, 124, 1);
    test_token_pi(ts_next(ts), "êëì", 126, 132, 1);
    test_token_pi(ts_next(ts), "úøã", 134, 140, 1);
    test_token_pi(ts_next(ts), "öîí", 142, 148, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    a = mb_standard_analyzer_new_with_words(words, true);
    ts = a_get_ts(a, I("random"), text);
    ts2 = a_get_ts(a, I("random"), text);
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 2);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "and", 32, 35, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 2);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 2);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 2);
    test_token_pi(ts_next(ts), "23", 111, 113, 1);
    test_token_pi(ts_next(ts), "áägç", 117, 124, 1);
    test_token_pi(ts_next(ts), "êëì", 126, 132, 1);
    test_token_pi(ts_next(ts), "úøã", 134, 140, 1);
    test_token_pi(ts_next(ts), "öîí", 142, 148, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    test_token_pi(ts_next(ts2), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts2), "my", 22, 24, 2);
    test_token_pi(ts_next(ts2), "email", 25, 31, 1);
    test_token_pi(ts_next(ts2), "e", 25, 26, 0);
    test_token_pi(ts_next(ts2), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts2), "and", 32, 35, 1);
    test_token_pi(ts_next(ts2), "address", 40, 47, 2);
    test_token_pi(ts_next(ts2), "www.google.com/results", 55, 85, 2);
    test_token_pi(ts_next(ts2), "123-1235-asd-1234", 93, 110, 2);
    test_token_pi(ts_next(ts2), "23", 111, 113, 1);
    test_token_pi(ts_next(ts2), "áägç", 117, 124, 1);
    test_token_pi(ts_next(ts2), "êëì", 126, 132, 1);
    test_token_pi(ts_next(ts2), "úøã", 134, 140, 1);
    test_token_pi(ts_next(ts2), "öîí", 142, 148, 1);
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

static void test_legacy_standard_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        legacy_standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, true);
    char text[200] =
        "DBalmain@gmail.com is My e-mail and the Address. -23!$ "
        "http://www.google.com/results/ T.N.T. 123-1235-ASD-1234";
    TokenStream *ts = a_get_ts(a, I("random"), text);
    (void)data;

    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 2);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 3);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "tnt", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts);
    a_deref(a);
}

static void test_mb_legacy_standard_analyzer(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    Analyzer *a =
        mb_legacy_standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, false);
    const char *words[] = { "is", "the", "-23", "tnt", NULL };
    char text[200] =
        "DBalmain@gmail.com is My e-mail and the Address. -23!$ "
        "http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 23#!$ "
        "ÁÄGÇ®ÊËÌ¯ÚØÃ¬ÖÎÍ";
    TokenStream *ts = a_get_ts(a, I("random"), text), *ts2;
    (void)data;

    test_token_pi(ts_next(ts), "DBalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "My", 22, 24, 2);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "Address", 40, 47, 3);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "TNT", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-ASD-1234", 93, 110, 1);
    test_token_pi(ts_next(ts), "23", 111, 113, 1);
    test_token_pi(ts_next(ts), "ÁÄGÇ", 117, 124, 1);
    test_token_pi(ts_next(ts), "ÊËÌ", 126, 132, 1);
    test_token_pi(ts_next(ts), "ÚØÃ", 134, 140, 1);
    test_token_pi(ts_next(ts), "ÖÎÍ", 142, 148, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    a = mb_legacy_standard_analyzer_new(true);
    ts = a_get_ts(a, I("random"), text);
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "email", 25, 31, 3);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 3);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "tnt", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 1);
    test_token_pi(ts_next(ts), "23", 111, 113, 1);
    test_token_pi(ts_next(ts), "áägç", 117, 124, 1);
    test_token_pi(ts_next(ts), "êëì", 126, 132, 1);
    test_token_pi(ts_next(ts), "úøã", 134, 140, 1);
    test_token_pi(ts_next(ts), "öîí", 142, 148, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    a = mb_legacy_standard_analyzer_new_with_words(words, true);
    ts = a_get_ts(a, I("random"), text);
    ts2 = a_get_ts(a, I("random"), text);
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 2);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "and", 32, 35, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 2);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 2);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 2);
    test_token_pi(ts_next(ts), "23", 111, 113, 1);
    test_token_pi(ts_next(ts), "áägç", 117, 124, 1);
    test_token_pi(ts_next(ts), "êëì", 126, 132, 1);
    test_token_pi(ts_next(ts), "úøã", 134, 140, 1);
    test_token_pi(ts_next(ts), "öîí", 142, 148, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    test_token_pi(ts_next(ts2), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts2), "my", 22, 24, 2);
    test_token_pi(ts_next(ts2), "email", 25, 31, 1);
    test_token_pi(ts_next(ts2), "e", 25, 26, 0);
    test_token_pi(ts_next(ts2), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts2), "and", 32, 35, 1);
    test_token_pi(ts_next(ts2), "address", 40, 47, 2);
    test_token_pi(ts_next(ts2), "www.google.com/results", 55, 85, 2);
    test_token_pi(ts_next(ts2), "123-1235-asd-1234", 93, 110, 2);
    test_token_pi(ts_next(ts2), "23", 111, 113, 1);
    test_token_pi(ts_next(ts2), "áägç", 117, 124, 1);
    test_token_pi(ts_next(ts2), "êëì", 126, 132, 1);
    test_token_pi(ts_next(ts2), "úøã", 134, 140, 1);
    test_token_pi(ts_next(ts2), "öîí", 142, 148, 1);
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

static void test_long_word(TestCase *tc, void *data)
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
    TokenStream *ts = a_get_ts(a, I("random"), text);
    (void)data;

    test_token_pi(ts_next(ts), text, 0, 290, 1);        /* text gets truncated anyway */
    test_token_pi(ts_next(ts), "two", 291, 294, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    a = mb_standard_analyzer_new_with_words(ENGLISH_STOP_WORDS, true);
    ts = a_get_ts(a, I("random"), text);
    test_token_pi(ts_next(ts), text, 0, 290, 1);        /* text gets truncated anyway */
    test_token_pi(ts_next(ts), "two", 291, 294, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    a_deref(a);
    tk_destroy(tk);
}

/****************************************************************************
 *
 * Filters
 *
 ****************************************************************************/

static void test_lowercase_filter(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = lowercase_filter_new(standard_tokenizer_new());
    char text[200] =
        "DBalmain@gmail.com is My e-mail 52   #$ Address. -23!$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234";
    (void)data;

    ts->reset(ts, text);
    test_token(ts_next(ts), "dbalmain@gmail.com", 0, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e-mail", 25, 31);
    test_token(ts_next(ts), "52", 32, 34);
    test_token(ts_next(ts), "address", 40, 47);
    test_token(ts_next(ts), "-23", 49, 52);
    test_token(ts_next(ts), "www.google.com/results", 55, 85);
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

static void test_hyphen_filter(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = hyphen_filter_new(lowercase_filter_new(standard_tokenizer_new()));
    char text[200] =
        "DBalmain@gmail.com is My e-mail 52   #$ Address. -23!$ http://www.google.com/results/ T.N.T. 123-1235-ASD-1234 long-hyph-en-at-ed-word";
    (void)data;

    ts->reset(ts, text);
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "is", 19, 21, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 1);
    test_token_pi(ts_next(ts), "email", 25, 31, 1);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "52", 32, 34, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 1);
    test_token_pi(ts_next(ts), "-23", 49, 52, 1);
    test_token_pi(ts_next(ts), "www.google.com/results", 55, 85, 1);
    test_token_pi(ts_next(ts), "tnt", 86, 91, 1);
    test_token_pi(ts_next(ts), "123-1235-asd-1234", 93, 110, 1);
    test_token_pi(ts_next(ts), "longhyphenatedword", 111, 134, 1);
    test_token_pi(ts_next(ts), "long", 111, 115, 0);
    test_token_pi(ts_next(ts), "hyph", 116, 120, 1);
    test_token_pi(ts_next(ts), "en", 121, 123, 1);
    test_token_pi(ts_next(ts), "at", 124, 126, 1);
    test_token_pi(ts_next(ts), "ed", 127, 129, 1);
    test_token_pi(ts_next(ts), "word", 130, 134, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

const char *words[] = { "one", "four", "five", "seven", NULL };
static void test_stop_filter(TestCase *tc, void *data)
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

static void test_mapping_filter(TestCase *tc, void *data)
{
    Token *tk = tk_new();
    TokenStream *ts = mapping_filter_new(letter_tokenizer_new());
    char text[200] =
        "one, two, three, four, five, six, seven, eight, nine, ten.";
    char long_word[301] =
       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    (void)data;

    mapping_filter_add(ts, "ne", "hello");
    mapping_filter_add(ts, "four", long_word);

    ts->reset(ts, text);
    test_token(ts_next(ts), "ohello", 0, 3);
    test_token(ts_next(ts), "two", 5, 8);
    test_token(ts_next(ts), "three", 10, 15);
    test_token(ts_next(ts), long_word, 17, 21);
    test_token(ts_next(ts), "five", 23, 27);
    test_token(ts_next(ts), "six", 29, 32);
    test_token(ts_next(ts), "seven", 34, 39);
    test_token(ts_next(ts), "eight", 41, 46);
    test_token(ts_next(ts), "nihello", 48, 52);
    test_token(ts_next(ts), "ten", 54, 57);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");

    mapping_filter_add(ts, "thr", "start");
    mapping_filter_add(ts, "en", "goodbye");
    ts->reset(ts, text);
    test_token(ts_next(ts), "ohello", 0, 3);
    test_token(ts_next(ts), "two", 5, 8);
    test_token(ts_next(ts), "startee", 10, 15);
    test_token(ts_next(ts), long_word, 17, 21);
    test_token(ts_next(ts), "five", 23, 27);
    test_token(ts_next(ts), "six", 29, 32);
    test_token(ts_next(ts), "sevgoodbye", 34, 39);
    test_token(ts_next(ts), "eight", 41, 46);
    test_token(ts_next(ts), "nihello", 48, 52);
    test_token(ts_next(ts), "tgoodbye", 54, 57);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    REF(ts);
    Aiequal(2, ts->ref_cnt);
    ts_deref(ts);
    Aiequal(1, ts->ref_cnt);
    ts_deref(ts);
}

static void test_stemmer(TestCase *tc, void *data)
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

static void test_stem_filter(TestCase *tc, void *data)
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

static void test_per_field_analyzer(TestCase *tc, void *data)
{
    TokenStream *ts;
    Token *tk = tk_new();
    char text[100] = "DBalmain@gmail.com is My E-mail 52   #$ address. 23#!$";
    Analyzer *pfa = per_field_analyzer_new(standard_analyzer_new(true));
    (void)data;

    pfa_add_field(pfa, I("white"), whitespace_analyzer_new(false));
    pfa_add_field(pfa, I("white_l"), whitespace_analyzer_new(true));
    pfa_add_field(pfa, I("letter"), letter_analyzer_new(false));
    pfa_add_field(pfa, I("letter"), letter_analyzer_new(true));
    pfa_add_field(pfa, I("letter_u"), letter_analyzer_new(false));
    ts = a_get_ts(pfa, I("white"), text);
    test_token_pi(ts_next(ts), "DBalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "is", 19, 21, 1);
    test_token_pi(ts_next(ts), "My", 22, 24, 1);
    test_token_pi(ts_next(ts), "E-mail", 25, 31, 1);
    test_token_pi(ts_next(ts), "52", 32, 34, 1);
    test_token_pi(ts_next(ts), "#$", 37, 39, 1);
    test_token_pi(ts_next(ts), "address.", 40, 48, 1);
    test_token_pi(ts_next(ts), "23#!$", 49, 54, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    ts = a_get_ts(pfa, I("white_l"), text);
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "is", 19, 21, 1);
    test_token_pi(ts_next(ts), "my", 22, 24, 1);
    test_token_pi(ts_next(ts), "e-mail", 25, 31, 1);
    test_token_pi(ts_next(ts), "52", 32, 34, 1);
    test_token_pi(ts_next(ts), "#$", 37, 39, 1);
    test_token_pi(ts_next(ts), "address.", 40, 48, 1);
    test_token_pi(ts_next(ts), "23#!$", 49, 54, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    ts = a_get_ts(pfa, I("letter_u"), text);
    test_token(ts_next(ts), "DBalmain", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "My", 22, 24);
    test_token(ts_next(ts), "E", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    ts = a_get_ts(pfa, I("letter"), text);
    test_token(ts_next(ts), "dbalmain", 0, 8);
    test_token(ts_next(ts), "gmail", 9, 14);
    test_token(ts_next(ts), "com", 15, 18);
    test_token(ts_next(ts), "is", 19, 21);
    test_token(ts_next(ts), "my", 22, 24);
    test_token(ts_next(ts), "e", 25, 26);
    test_token(ts_next(ts), "mail", 27, 31);
    test_token(ts_next(ts), "address", 40, 47);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    ts_deref(ts);
    ts = a_get_ts(pfa, I("XXX"), text);        /* should use default analyzer */
    test_token_pi(ts_next(ts), "dbalmain@gmail.com", 0, 18, 1);
    test_token_pi(ts_next(ts), "email", 25, 31, 3);
    test_token_pi(ts_next(ts), "e", 25, 26, 0);
    test_token_pi(ts_next(ts), "mail", 27, 31, 1);
    test_token_pi(ts_next(ts), "52", 32, 34, 1);
    test_token_pi(ts_next(ts), "address", 40, 47, 1);
    test_token_pi(ts_next(ts), "23", 49, 51, 1);
    Assert(ts_next(ts) == NULL, "Should be no more tokens");
    tk_destroy(tk);
    ts_deref(ts);
    a_deref(pfa);
}

TestSuite *ts_analysis(TestSuite *suite)
{
    bool u = false;
    char *locale = setlocale(LC_ALL, "");
    if (locale && (strstr(locale, "utf") || strstr(locale, "UTF"))) u = true;

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_tk, NULL);

    /* Non */
    tst_run_test(suite, test_non_tokenizer, NULL);
    tst_run_test(suite, test_non_analyzer, NULL);

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

    /* LegacyStandard */
    tst_run_test(suite, test_legacy_standard_tokenizer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_legacy_standard_tokenizer, NULL);
    }
    tst_run_test(suite, test_legacy_standard_analyzer, NULL);
    if (u) {
        tst_run_test(suite, test_mb_legacy_standard_analyzer, NULL);
    }

    tst_run_test(suite, test_long_word, NULL);

    /* PerField */
    tst_run_test(suite, test_per_field_analyzer, NULL);

    /* Filters */
    tst_run_test(suite, test_lowercase_filter, NULL);
    tst_run_test(suite, test_hyphen_filter, NULL);
    tst_run_test(suite, test_stop_filter, NULL);
    tst_run_test(suite, test_mapping_filter, NULL);
    tst_run_test(suite, test_stemmer, NULL);
    if (u) {
        tst_run_test(suite, test_stem_filter, NULL);
    }

    return suite;
}

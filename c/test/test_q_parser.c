#include "test.h"
#include "search.h"
#include "internal.h"

static char *f1 = "f1";
static char *f2 = "f2";
static char *field = "field";
static char *xx = "xx";

typedef struct QPTestPair {
    char *qstr;
    char *qres;
} QPTestPair;

#define PARSER_TEST(str, res) do {\
    Query *q = qp_parse(parser, str);\
    char *qres = q->to_s(q, xx);\
    Asequal(res, qres);\
    q_deref(q);\
    free(qres);\
} while (0);

static void test_q_parser(tst_case *tc, void *data)
{
    int i;
    HashSet *all_fields = hs_new_str(NULL);
    HashSet *def_fields = hs_new_str(NULL);
    HashSet *tkz_fields = hs_new_str(NULL);
    Analyzer *analyzer = letter_analyzer_new(true);
    QParser *parser;
    QPTestPair pairs[] = {
        {"", ""},
        {"word", "word"},
        {"f1:word", "f1:word"},
        {"f1|f2:word", "f1:word f2:word"},
        {"field:word", "field:word"},
        {"\"word1 word2 word3\"", "\"word word word\""},
        {"\"word1 2342 word3\"", "\"word <> word\"~1"},
        {"field:\"one TWO ThrEe\"", "field:\"one TWO ThrEe\""},
        {"field:\"one 222 three\"", "field:\"one 222 three\""},
        {"xx:\"one 222 three\"", "\"one <> three\"~1"},
        {"field:\"one <> three\"", "field:\"one <> three\""},
        {"field:\"<> two three\"", "field:\"two three\""},
        {"field:\"one <> three <>\"", "field:\"one <> three\""},
        {"field:\"oNe <> <> <> three <>\"", "field:\"oNe <> <> <> three\""},
        {"field:\"one <> <> <> three|four|five <>\"",
            "field:\"one <> <> <> three|four|five\""},
        {"field:\"on1|tw2 three|four|five six|seven\"",
            "field:\"on1|tw2 three|four|five six|seven\""},
        {"field:\"testing|trucks\"", "field:\"testing|trucks\""},
        {"[aaa bbb]", "[aaa bbb]"},
        {"{aaa bbb]", "{aaa bbb]"},
        {"field:[aaa bbb}", "field:[aaa bbb}"},
        {"{aaa bbb}", "{aaa bbb}"},
        {"{aaa>", "{aaa>"},
        {"[aaa>", "[aaa>"},
        {"field:<a\\ aa}", "field:<a aa}"},
        {"<aaa]", "<aaa]"},
        {">aaa", "{aaa>"},
        {">=aaa", "[aaa>"},
        {"<aaa", "<aaa}"},
        {"field:<=aaa", "field:<aaa]"},
        {"REQ one REQ two", "+one +two"},
        {"REQ one two", "+one two"},
        {"one REQ two", "one +two"},
        {"+one +two", "+one +two"},
        {"+one two", "+one two"},
        {"one +two", "one +two"},
        {"-one -two", "-one -two"},
        {"-one two", "-one two"},
        {"one -two", "one -two"},
        {"!one !two", "-one -two"},
        {"!one two", "-one two"},
        {"one !two", "one -two"},
        {"NOT one NOT two", "-one -two"},
        {"NOT one two", "-one two"},
        {"one NOT two", "one -two"},
        {"one two", "one two"},
        {"one OR two", "one two"},
        {"one AND two", "+one +two"},
        {"one two AND three", "one two +three"},
        {"one two OR three", "one two three"},
        {"Opus::City", "\"opus city\"~1"},
        {"()", ""},
        {"field:()", ""},
        {"xx:\"Hello Newman\" field:()", "\"hello newman\" ()"},
        {"one (two AND three)", "one (+two +three)"},
        {"one AND (two OR three)", "+one +(two three)"},
        {"field:(one AND (two OR t\\=h\\=r\\=e\\=e))",
            "+field:one +(field:two field:t=h=r=e=e)"},
        {"one AND (two OR [aaa vvv})", "+one +(two [aaa vvv})"},
        {"one AND (f1:two OR f2:three) AND four",
            "+one +(f1:two f2:three) +four"},
        {"one^1.2300", "one^1.23"},
        {"(one AND two)^100.23", "(+one +two)^100.23"},
        {"field:(one AND two)^100.23", "(+field:one +field:two)^100.23"},
        {"field:(one AND [aaa bbb]^23.300)^100.23",
            "(+field:one +field:[aaa bbb]^23.3)^100.23"},
        {"(REQ field:\"one two three\")^23.000",
            "field:\"one two three\"^23.0"},
        {"asdf~0.2", "asdf~0.2"},
        {"field:asdf~0.2", "field:asdf~0.2"},
        {"asdf~0.2^100.00", "asdf~0.2^100.0"},
        {"field:asdf~0.2^0.1", "field:asdf~0.2^0.1"},
        {"field:\"asdf <> asdf|asdf\"~4", "field:\"asdf <> asdf|asdf\"~4"},
        {"\"one two three four five\"~5", "\"one two three four five\"~5"},
        {"ab?de", "ab?de"},
        {"ab*de", "ab*de"},
        {"asdf?*?asd*dsf?asfd*asdf?", "asdf?*?asd*dsf?asfd*asdf?"},
        {"field:a* AND field:(b*)", "+field:a* +field:b*"},
        {"field:abc~ AND field:(b*)", "+field:abc~ +field:b*"},
        {"asdf?*?asd*dsf?asfd*asdf?^20.0", "asdf?*?asd*dsf?asfd*asdf?^20.0"},
        {"field:ASDF?*?22d*dsf?ASFD*asdf?^20.0",
            "field:ASDF?*?22d*dsf?ASFD*asdf?^20.0"},

        {"*:xxx", "xxx f1:xxx f2:xxx field:xxx"},
        {"f1|f2:xxx", "f1:xxx f2:xxx"},

        {"*:asd~0.2", "asd~0.2 f1:asd~0.2 f2:asd~0.2 field:asd~0.2"},
        {"f1|f2:asd~0.2", "f1:asd~0.2 f2:asd~0.2"},

        {"*:a?d*^20.0", "(a?d* f1:a?d* f2:a?d* field:a?d*)^20.0"},
        {"f1|f2:a?d*^20.0", "(f1:a?d* f2:a?d*)^20.0"},

        {"*:\"asdf <> xxx|yyy\"",
            "\"asdf <> xxx|yyy\" f1:\"asdf <> xxx|yyy\" f2:\"asdf <> xxx|yyy\" "
                "field:\"asdf <> xxx|yyy\""},
        {"f1|f2:\"asdf <> do|yyy\"",
            "f1:\"asdf <> do|yyy\" f2:\"asdf <> do|yyy\""},

        {"*:[bbb xxx]", "[bbb xxx] f1:[bbb xxx] f2:[bbb xxx] field:[bbb xxx]"},
        {"f1|f2:[bbb xxx]", "f1:[bbb xxx] f2:[bbb xxx]"},

        {"*:(xxx AND bbb)",
            "+(xxx f1:xxx f2:xxx field:xxx) +(bbb f1:bbb f2:bbb field:bbb)"},
        {"f1|f2:(xxx AND bbb)", "+(f1:xxx f2:xxx) +(f1:bbb f2:bbb)"},
        {"ASDF?*?asd*dsf?ASFD*asdf?^20.0", "asdf?*?asd*dsf?asfd*asdf?^20.0"},
        {"ASDFasdAasAasASD~", "asdfasdaasaasasd~"},
        {"\"onewordphrase\"", "onewordphrase"},
        {"one billion eight hundred and thirty three million four hundred "
         "and eighty eight thousand two hundred and sixty three",
            "one billion eight hundred and thirty three million four hundred "
            "and eighty eight thousand two hundred and sixty three"},
        {"f1:*", "*"},
        {"f1:*^100.0", "*^100.0"},
        {"f1:?*", "f1:?*"},
        {"f1:?*^100.0", "f1:?*^100.0"}
         /*
            */
    };  
    (void)data;

    hs_add(all_fields, xx);
    hs_add(all_fields, f1);
    hs_add(all_fields, f2);
    hs_add(all_fields, field);
    hs_add(tkz_fields, xx);
    hs_add(tkz_fields, f1);
    hs_add(tkz_fields, f2);
    hs_add(def_fields, xx);

    REF(analyzer);
    parser = qp_new(all_fields, def_fields, tkz_fields, analyzer);
    parser->close_def_fields = false;

    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    PARSER_TEST("not_field:word", "");
    qp_destroy(parser);

    all_fields = hs_new_str(NULL);
    tkz_fields = hs_new_str(NULL);
    hs_add(all_fields, xx);
    hs_add(all_fields, f1);
    hs_add(all_fields, f2);
    hs_add(all_fields, field);
    hs_add(tkz_fields, xx);
    hs_add(tkz_fields, f1);
    hs_add(tkz_fields, f2);

    /* This time let the query parser destroy the analyzer */
    parser = qp_new(all_fields, def_fields, tkz_fields, analyzer);
    parser->clean_str = false;
    parser->allow_any_fields = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    PARSER_TEST("not_field:word", "not_field:word");

    parser->wild_lower = false;
    PARSER_TEST("ASDF?*?asd*dsf?ASFD*asdf?^20.0", "ASDF?*?asd*dsf?ASFD*asdf?^20.0");
    PARSER_TEST("ASDFasdAasAasASD~", "asdfasdaasaasasd~");
    qp_destroy(parser);
}

static void test_q_parser_standard_analyzer(tst_case *tc, void *data)
{
    int i;
    HashSet *all_fields = hs_new_str(NULL);
    HashSet *def_fields = hs_new_str(NULL);
    Analyzer *analyzer = mb_standard_analyzer_new(true);
    QParser *parser;
    QPTestPair pairs[] = {
        {"", ""},
        {"word", "word"},
        {"f1:word", "f1:word"},
        {"f1|f2:word", "f1:word f2:word"},
        {"field:word", "field:word"},
        {"\"word1 word2 word3\"", "\"word1 word2 word3\""},
        {"\"word1 2342 word3\"", "\"word1 2342 word3\""},
        {"field:\"one two three\"", "field:\"one two three\""},
        {"field:\"one 222 three\"", "field:\"one 222 three\""},
        {"field:\"one <> three\"", "field:\"one <> three\""},
        {"field:\"<> two three\"", "field:\"two three\""},
        {"field:\"one <> three <>\"", "field:\"one <> three\""},
        {"field:\"one <> <> <> three <>\"", "field:\"one <> <> <> three\""},
        {"field:\"one <> <> <> three|four|five <>\"",
            "field:\"one <> <> <> three|four|five\""},
        {"field:\"one|two three|four|five six|seven\"",
            "field:\"one|two three|four|five six|seven\""},
        {"field:\"testing|trucks\"", "field:\"testing|trucks\""},
        {"[aaa bbb]", "[aaa bbb]"},
        {"{aaa bbb]", "{aaa bbb]"},
        {"field:[aaa bbb}", "field:[aaa bbb}"},
        {"{aaa bbb}", "{aaa bbb}"},
        {"{aaa>", "{aaa>"},
        {"[aaa>", "[aaa>"},
        {"field:<aaa}", "field:<aaa}"},
        {"<aaa]", "<aaa]"},
        {">aaa", "{aaa>"},
        {">=aaa", "[aaa>"},
        {"<aaa", "<aaa}"},
        {"field:<=aaa", "field:<aaa]"},
        {"REQ one REQ two", "+one +two"},
        {"REQ one two", "+one two"},
        {"one REQ two", "one +two"},
        {"+one +two", "+one +two"},
        {"+one two", "+one two"},
        {"one +two", "one +two"},
        {"-one -two", "-one -two"},
        {"-one two", "-one two"},
        {"one -two", "one -two"},
        {"!one !two", "-one -two"},
        {"!one two", "-one two"},
        {"one !two", "one -two"},
        {"NOT one NOT two", "-one -two"},
        {"NOT one two", "-one two"},
        {"one NOT two", "one -two"},
        {"one two", "one two"},
        {"one OR two", "one two"},
        {"one AND two", "+one +two"},
        {"one two AND three", "one two +three"},
        {"one two OR three", "one two three"},
        {"one (two AND three)", "one (+two +three)"},
        {"one AND (two OR three)", "+one +(two three)"},
        {"field:(one AND (two OR three))", "+field:one +(field:two field:three)"},
        {"one AND (two OR [aaa vvv})", "+one +(two [aaa vvv})"},
        {"one AND (f1:two OR f2:three) AND four", "+one +(f1:two f2:three) +four"},
        {"one^1.2300", "one^1.23"},
        {"(one AND two)^100.23", "(+one +two)^100.23"},
        {"field:(one AND two)^100.23", "(+field:one +field:two)^100.23"},
        {"field:(one AND [aaa bbb]^23.300)^100.23",
            "(+field:one +field:[aaa bbb]^23.3)^100.23"},
        {"(REQ field:\"one two three\")^23.000", "field:\"one two three\"^23.0"},
        {"asdf~0.2", "asdf~0.2"},
        {"field:asdf~0.2", "field:asdf~0.2"},
        {"asdf~0.2^100.00", "asdf~0.2^100.0"},
        {"field:asdf~0.2^0.1", "field:asdf~0.2^0.1"},
        {"field:\"asdf <> asdf|asdf\"~4", "field:\"asdf <> asdf|asdf\"~4"},
        {"\"one two three four five\"~5", "\"one two three four five\"~5"},
        {"ab?de", "ab?de"},
        {"ab*de", "ab*de"},
        {"asdf?*?asd*dsf?asfd*asdf?", "asdf?*?asd*dsf?asfd*asdf?"},
        {"field:a* AND field:(b*)", "+field:a* +field:b*"},
        {"field:abc~ AND field:(b*)", "+field:abc~ +field:b*"},
        {"asdf?*?asd*dsf?asfd*asdf?^20.0", "asdf?*?asd*dsf?asfd*asdf?^20.0"},

        {"*:xxx", "xxx f1:xxx f2:xxx field:xxx"},
        {"f1|f2:xxx", "f1:xxx f2:xxx"},

        {"*:asd~0.2", "asd~0.2 f1:asd~0.2 f2:asd~0.2 field:asd~0.2"},
        {"f1|f2:asd~0.2", "f1:asd~0.2 f2:asd~0.2"},

        {"*:a?d*^20.0", "(a?d* f1:a?d* f2:a?d* field:a?d*)^20.0"},
        {"f1|f2:a?d*^20.0", "(f1:a?d* f2:a?d*)^20.0"},

        {"*:\"asdf <> xxx|yyy\"",
            "\"asdf <> xxx|yyy\" f1:\"asdf <> xxx|yyy\" f2:\"asdf <> xxx|yyy\" "
                "field:\"asdf <> xxx|yyy\""},
        {"f1|f2:\"asdf <> xxx|yyy\"",
            "f1:\"asdf <> xxx|yyy\" f2:\"asdf <> xxx|yyy\""},
        {"f1|f2:\"do|yyy\"", "f1:yyy f2:yyy"},
        {"f1|f2:\"asdf <> do|yyy\"",
            "f1:\"asdf <> yyy\" f2:\"asdf <> yyy\""},

        {"*:[bbb xxx]", "[bbb xxx] f1:[bbb xxx] f2:[bbb xxx] field:[bbb xxx]"},
        {"f1|f2:[bbb xxx]", "f1:[bbb xxx] f2:[bbb xxx]"},

        {"*:(xxx AND bbb)",
            "+(xxx f1:xxx f2:xxx field:xxx) +(bbb f1:bbb f2:bbb field:bbb)"},
        {"f1|f2:(xxx AND bbb)", "+(f1:xxx f2:xxx) +(f1:bbb f2:bbb)"},
        {"ASDF?*?asd*dsf?ASFD*asdf?^20.0", "asdf?*?asd*dsf?asfd*asdf?^20.0"},
        {"ASDFasdAasAasASD~", "asdfasdaasaasasd~"},
        {"\"onewordphrase\"", "onewordphrase"},
        {"one billion eight hundred and thirty three million four hundred and "
         "eighty eight thousand two hundred and sixty three",
            "one billion eight hundred thirty three million four hundred "
            "eighty eight thousand two hundred sixty three"},
        {"f1:*", "*"},
        {"f1:*^100.0", "*^100.0"},
        {"f1:?*", "f1:?*"},
        {"*:this", ""},
        {"this-is-a-hyphenated-word", "\"thisisahyphenatedword|this is a hyphenated word\"~4"},
        {"\"the phrase and the phrase\"", "\"phrase <> <> phrase\"~3"},
        {"\"the e-mail was in the inbox\"", "\"email|e mail <> <> <> inbox\"~5"},
        {"f1:?*^100.0", "f1:?*^100.0"}
         /*
            */
    };  
    (void)data;

    hs_add(all_fields, xx);
    hs_add(all_fields, f1);
    hs_add(all_fields, f2);
    hs_add(all_fields, field);
    hs_add(def_fields, xx);

    REF(analyzer);
    parser = qp_new(all_fields, def_fields, NULL, analyzer);
    parser->close_def_fields = false;

    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    PARSER_TEST("not_field:word", "");
    qp_destroy(parser);

    all_fields = hs_new_str(NULL);
    hs_add(all_fields, xx);
    hs_add(all_fields, f1);
    hs_add(all_fields, f2);
    hs_add(all_fields, field);

    /* This time let the query parser destroy the analyzer */
    parser = qp_new(all_fields, def_fields, NULL, analyzer);
    parser->clean_str = false;
    parser->allow_any_fields = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    PARSER_TEST("not_field:word", "not_field:word");

    parser->wild_lower = false;
    PARSER_TEST("ASDF?*?asd*dsf?ASFD*asdf?^20.0", "ASDF?*?asd*dsf?ASFD*asdf?^20.0");
    PARSER_TEST("ASDFasdAasAasASD~", "asdfasdaasaasasd~");
    qp_destroy(parser);
}

static void test_qp_clean_str(tst_case *tc, void *data)
{
    int i;
    QPTestPair pairs[] = {
        {"", ""},
        {"\"< <>\"", "\"\\< <>\""},
        {"\"hello there", "\"hello there\""},
        {"hello there\"", "hello there\"\""},
        {"(hello there", "(hello there)"},
        {"(hello (there", "(hello (there))"},
        {"(hello\" (there", "(hello\" \\(there\")"},
        {"(hello\" &:()[]{}!+~^-<|>=*? <>there",
            "(hello\" \\&\\:\\(\\)\\[\\]\\{\\}\\!\\+\\~\\^\\-\\<|\\>\\=\\*\\? "
            "<>there\")"},
        {"hello) there)", "((hello) there)"},
        {"hello) \"there)", "(hello) \"there\\)\""},
        {"(hello \\&\\:\\(\\)\\[\\]\\{\\}\\!\\+\\~\\^\\-\\<|\\>\\=\\*\\?",
            "(hello \\&\\:\\(\\)\\[\\]\\{\\}\\!\\+\\~\\^\\-\\<|\\>\\=\\*\\?)"},
        {"hello \\\\&", "hello \\\\&"},
        {"hello \\\\\\&", "hello \\\\\\&"},
        {"\"hello \\\\\"", "\"hello \\\""},
    };
    (void)data;
    for (i = 0; i < NELEMS(pairs); i++) {
        char *qres = qp_clean_str(pairs[i].qstr);\
                     Asequal(pairs[i].qres, qres);\
                     free(qres);\
    }
}

static void test_qp_bad_queries(tst_case *tc, void *data)
{
    int i;
    HashSet *all_fields = hs_new_str(NULL);
    HashSet *def_fields = hs_new_str(NULL);
    QParser *parser;
    QPTestPair pairs[] = {
        {"[, ]", ""},
        {"::*word", "word"},
        {"::))*&)(*^&*(", ""},
        {"::|)*&one)(*two(*&\"", "\"one two\"~1"}
    };  
    (void)data;

    hs_add(all_fields, f1);
    hs_add(all_fields, f2);
    hs_add(all_fields, field);
    hs_add(all_fields, xx);
    hs_add(def_fields, xx);

    parser = qp_new(all_fields, def_fields, NULL, letter_analyzer_new(true));
    parser->handle_parse_errors = true;

    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    qp_destroy(parser);
}

static void test_qp_prefix_query(tst_case *tc, void *data)
{
    HashSet *all_fields = hs_new_str(NULL);
    HashSet *def_fields = hs_new_str(NULL);
    QParser *parser;
    Query *q;
    (void)data;
    hs_add(all_fields, xx);
    hs_add(def_fields, xx);

    parser = qp_new(all_fields, def_fields, NULL, letter_analyzer_new(true));

    q = qp_parse(parser, "asdg*");
    Aiequal(PREFIX_QUERY, q->type);
    q_deref(q);
    q = qp_parse(parser, "a?dg*");
    Aiequal(WILD_CARD_QUERY, q->type);
    q_deref(q);
    q = qp_parse(parser, "a*dg*");
    Aiequal(WILD_CARD_QUERY, q->type);
    q_deref(q);
    q = qp_parse(parser, "asdg*a");
    Aiequal(WILD_CARD_QUERY, q->type);
    q_deref(q);
    qp_destroy(parser);
}

static void test_qp_keyword_switch(tst_case *tc, void *data)
{
    HashSet *all_fields = hs_new_str(NULL);
    HashSet *def_fields = hs_new_str(NULL);
    QParser *parser;
    (void)data;
    hs_add(all_fields, xx);
    hs_add(def_fields, xx);

    parser = qp_new(all_fields, def_fields, NULL, letter_analyzer_new(true));

    PARSER_TEST("REQ www (xxx AND yyy) OR NOT zzz", "+www (+xxx +yyy) -zzz");

    parser->use_keywords = false;
    PARSER_TEST("REQ www (xxx AND yyy) OR NOT zzz", "req www (xxx and yyy) or not zzz");

    qp_destroy(parser);
}

tst_suite *ts_q_parser(tst_suite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_q_parser, NULL);
    tst_run_test(suite, test_q_parser_standard_analyzer, NULL);
    tst_run_test(suite, test_qp_clean_str, NULL);
    tst_run_test(suite, test_qp_bad_queries, NULL);
    tst_run_test(suite, test_qp_prefix_query, NULL);
    tst_run_test(suite, test_qp_keyword_switch, NULL);

    return suite;
}

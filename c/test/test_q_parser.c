#include "search.h"
#include "test.h"

typedef struct QPTestPair {
    char *qstr;
    char *qres;
} QPTestPair;

#define PARSER_TEST(str, res) do {\
    Query *q = qp_parse(parser, str);\
    char *qres = q->to_s(q, I("xx"));\
    Asequal(res, qres);\
    q_deref(q);\
    free(qres);\
} while (0);

static void test_q_parser(TestCase *tc, void *data)
{
    int i;
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
        {"one AND (f1:two OR f2:three) AND four",
            "+one +(f1:two f2:three) +four"},
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
        {"f1:?*^100.0", "f1:?*^100.0"},
        {"f1:(aaa f2:bbb ccc)", "f1:aaa f2:bbb f1:ccc"}
    };  
    (void)data;

    REF(analyzer);
    parser = qp_new(analyzer);
    qp_add_field(parser, I("xx"),    true,  true);
    qp_add_field(parser, I("f1"),    false, true);
    qp_add_field(parser, I("f2"),    false, true);
    qp_add_field(parser, I("field"), false, false);

    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    PARSER_TEST("not_field:word", "");
    qp_destroy(parser);

    /* This time let the query parser destroy the analyzer */
    parser = qp_new(analyzer);
    qp_add_field(parser, I("xx"),    true, true);
    qp_add_field(parser, I("f1"),    false, true);
    qp_add_field(parser, I("f2"),    false, true);
    qp_add_field(parser, I("field"), false, false);

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

static void test_q_parser_standard_analyzer(TestCase *tc, void *data)
{
    int i;
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
        {"f1:?*^100.0", "f1:?*^100.0"},
        {"f1:(a1 f2:b2 c3)", "f1:a1 f2:b2 f1:c3"}
         /*
            */
    };  
    (void)data;

    REF(analyzer);
    parser = qp_new(analyzer);
    qp_add_field(parser, I("xx"),    true,  true);
    qp_add_field(parser, I("f1"),    false, true);
    qp_add_field(parser, I("f2"),    false, true);
    qp_add_field(parser, I("field"), false, true);

    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    parser->clean_str = true;
    for (i = 0; i < NELEMS(pairs); i++) {
        PARSER_TEST(pairs[i].qstr, pairs[i].qres);
    }
    PARSER_TEST("not_field:word", "");
    qp_destroy(parser);

    /* This time let the query parser destroy the analyzer */
    parser = qp_new(analyzer);
    qp_add_field(parser, I("xx"),    true,  true);
    qp_add_field(parser, I("f1"),    false, true);
    qp_add_field(parser, I("f2"),    false, true);
    qp_add_field(parser, I("field"), false, true);

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

static void test_qp_clean_str(TestCase *tc, void *data)
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

static void test_qp_bad_queries(TestCase *tc, void *data)
{
    int i;
    QParser *parser;
    QPTestPair pairs[] = {
        {"[, ]", ""},
        {"::*word", "word"},
        {"::))*&)(*^&*(", ""},
        {"::|)*&one)(*two(*&\"", "\"one two\"~1"}
    };  
    (void)data;

    parser = qp_new(letter_analyzer_new(true));
    qp_add_field(parser, I("xx"),    true,  true);
    qp_add_field(parser, I("f1"),    false, true);
    qp_add_field(parser, I("f2"),    false, true);
    qp_add_field(parser, I("field"), false, true);

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

static void test_qp_prefix_query(TestCase *tc, void *data)
{
    QParser *parser;
    Query *q;
    (void)data;

    parser = qp_new(letter_analyzer_new(true));
    qp_add_field(parser, I("xx"), true,  true);

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

static void test_qp_keyword_switch(TestCase *tc, void *data)
{
    QParser *parser;
    (void)data;

    parser = qp_new(letter_analyzer_new(true));
    qp_add_field(parser, I("xx"), true,  true);

    PARSER_TEST("REQ www (xxx AND yyy) OR NOT zzz", "+www (+xxx +yyy) -zzz");

    parser->use_keywords = false;
    PARSER_TEST("REQ www (xxx AND yyy) OR NOT zzz", "req www (xxx and yyy) or not zzz");

    qp_destroy(parser);
}

TestSuite *ts_q_parser(TestSuite *suite)
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

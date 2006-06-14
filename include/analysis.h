#ifndef FRT_ANALYSIS_H
#define FRT_ANALYSIS_H

#include "global.h"
#include "hash.h"

/****************************************************************************
 *
 * Token
 *
 ****************************************************************************/

typedef struct Token
{
    char text[MAX_WORD_SIZE];
    int len;
    int start;
    int end;
    int pos_inc;
} Token;

Token *tk_new();
void tk_destroy(void *p);
Token *tk_set(Token *tk, char *text, int tlen, int start, int end, int pos_inc);
Token *tk_set_no_len(Token *tk, char *text, int start, int end, int pos_inc);
int tk_eq(Token *tk1, Token *tk2);
int tk_cmp(Token *tk1, Token *tk2);

/****************************************************************************
 *
 * TokenStream
 *
 ****************************************************************************/


typedef struct TokenStream TokenStream;
struct TokenStream
{
    void *data;
    char *text;
    char *t;                    /* ptr used to scan text */
    Token *token;
    Token *(*next)(TokenStream *ts);
    TokenStream *(*reset)(TokenStream *ts, char *text);
    void (*clone_i)(TokenStream *orig_ts, TokenStream *new_ts);
    void (*destroy_i)(TokenStream *ts);
    TokenStream *sub_ts;        /* used by filters */
    int ref_cnt;
};

#define ts_next(mts) mts->next(mts)

void ts_deref(void *p);

TokenStream *whitespace_tokenizer_new();
TokenStream *mb_whitespace_tokenizer_new(bool lowercase);

TokenStream *letter_tokenizer_new();
TokenStream *mb_letter_tokenizer_new(bool lowercase);

TokenStream *standard_tokenizer_new();
TokenStream *mb_standard_tokenizer_new();

TokenStream *lowercase_filter_new(TokenStream *ts);
TokenStream *mb_lowercase_filter_new(TokenStream *ts);

extern const char *ENGLISH_STOP_WORDS[];
extern const char *FULL_ENGLISH_STOP_WORDS[];
extern const char *EXTENDED_ENGLISH_STOP_WORDS[];
extern const char *FULL_FRENCH_STOP_WORDS[];
extern const char *FULL_SPANISH_STOP_WORDS[];
extern const char *FULL_PORTUGUESE_STOP_WORDS[];
extern const char *FULL_ITALIAN_STOP_WORDS[];
extern const char *FULL_GERMAN_STOP_WORDS[];
extern const char *FULL_DUTCH_STOP_WORDS[];
extern const char *FULL_SWEDISH_STOP_WORDS[];
extern const char *FULL_NORWEGIAN_STOP_WORDS[];
extern const char *FULL_DANISH_STOP_WORDS[];
extern const char *FULL_RUSSIAN_STOP_WORDS[];
extern const char *FULL_FINNISH_STOP_WORDS[];

TokenStream *stop_filter_new_with_words_len(TokenStream *ts,
                                            const char **words, int len);
TokenStream *stop_filter_new_with_words(TokenStream *ts,
                                        const char **words);
TokenStream *stop_filter_new(TokenStream *ts);
TokenStream *stem_filter_new(TokenStream *ts, const char *algorithm,
                             const char *charenc);
TokenStream *ts_clone(TokenStream *orig_ts);

/****************************************************************************
 *
 * Analyzer
 *
 ****************************************************************************/

typedef struct Analyzer
{
    void *data;
    TokenStream *current_ts;
    TokenStream *(*get_ts)(struct Analyzer *a, char *field, char *text);
    void (*destroy_i)(struct Analyzer *a);
    int ref_cnt;
} Analyzer;

void a_deref(void *p);

#define a_get_ts(ma, field, text) ma->get_ts(ma, field, text)

Analyzer *analyzer_new(void *data, TokenStream *ts,
                       void (*destroy)(Analyzer *a),
                       TokenStream *(*get_ts)(Analyzer *a,
                                              char *field,
                                              char *text));
void a_standard_destroy(Analyzer *a);
Analyzer *whitespace_analyzer_new(bool lowercase);
Analyzer *mb_whitespace_analyzer_new(bool lowercase);

Analyzer *letter_analyzer_new(bool lowercase);
Analyzer *mb_letter_analyzer_new(bool lowercase);

Analyzer *standard_analyzer_new(bool lowercase);
Analyzer *mb_standard_analyzer_new(bool lowercase);

Analyzer *standard_analyzer_new_with_words(const char **words,
                                           bool lowercase);
Analyzer *standard_analyzer_new_with_words_len(const char **words, int len,
                                               bool lowercase);
Analyzer *mb_standard_analyzer_new_with_words(const char **words,
                                              bool lowercase);
Analyzer *mb_standard_analyzer_new_with_words_len(const char **words,
                                                  int len, bool lowercase);

typedef struct PerFieldAnalyzer
{
    HashTable *dict;
    Analyzer *def;
} PerFieldAnalyzer;

Analyzer *per_field_analyzer_new(Analyzer *a);
void pfa_add_field(Analyzer *self, char *field, Analyzer *analyzer);

#endif

#ifndef FRT_ANALYSIS_H
#define FRT_ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"
#include "hash.h"
#include "symbol.h"
#include "multimapper.h"
#include <wchar.h>

/****************************************************************************
 *
 * FrtToken
 *
 ****************************************************************************/

typedef struct FrtToken
{
    char text[FRT_MAX_WORD_SIZE];
    int len;
    off_t start;
    off_t end;
    int pos_inc;
} FrtToken;

extern FrtToken *frt_tk_new();
extern void frt_tk_destroy(void *p);
extern FrtToken *frt_tk_set(FrtToken *tk, char *text, int tlen, off_t start, off_t end,
                     int pos_inc);
extern FrtToken *frt_tk_set_no_len(FrtToken *tk, char *text, off_t start, off_t end,
                            int pos_inc);
extern int frt_tk_eq(FrtToken *tk1, FrtToken *tk2);
extern int frt_tk_cmp(FrtToken *tk1, FrtToken *tk2);

/****************************************************************************
 *
 * FrtTokenStream
 *
 ****************************************************************************/


typedef struct FrtTokenStream FrtTokenStream;
struct FrtTokenStream
{
    char            *t;             /* ptr used to scan text */
    char            *text;
    FrtToken        *(*next)(FrtTokenStream *ts);
    FrtTokenStream  *(*reset)(FrtTokenStream *ts, char *text);
    FrtTokenStream  *(*clone_i)(FrtTokenStream *ts);
    void            (*destroy_i)(FrtTokenStream *ts);
    int             ref_cnt;
};

#define frt_ts_new(type) frt_ts_new_i(sizeof(type))
extern FrtTokenStream *frt_ts_new_i(size_t size);
extern FrtTokenStream *frt_ts_clone_size(FrtTokenStream *orig_ts, size_t size);

typedef struct FrtCachedTokenStream
{
    FrtTokenStream super;
    FrtToken       token;
} FrtCachedTokenStream;

typedef struct FrtMultiByteTokenStream
{
    FrtCachedTokenStream super;
    mbstate_t            state;
} FrtMultiByteTokenStream;

typedef enum 
{
    FRT_STT_ASCII,
    FRT_STT_MB,
    FRT_STT_UTF8
} FrtStandardTokenizerType;

typedef struct FrtStandardTokenizer
{
    FrtCachedTokenStream     super;
    FrtStandardTokenizerType type;
} FrtStandardTokenizer;

typedef struct FrtLegacyStandardTokenizer
{
    FrtCachedTokenStream super;
    bool        (*advance_to_start)(FrtTokenStream *ts);
    bool        (*is_tok_char)(char *c);
    int         (*get_alpha)(FrtTokenStream *ts, char *token);
    int         (*get_apostrophe)(char *input);
} FrtLegacyStandardTokenizer;

typedef struct FrtTokenFilter
{
    FrtTokenStream super;
    FrtTokenStream *sub_ts;
} FrtTokenFilter;

extern FrtTokenStream *frt_filter_clone_size(FrtTokenStream *ts, size_t size);
#define tf_new(type, sub) frt_tf_new_i(sizeof(type), sub)
extern FrtTokenStream *frt_tf_new_i(size_t size, FrtTokenStream *sub_ts);

typedef struct FrtStopFilter
{
    FrtTokenFilter super;
    FrtHash  *words;
} FrtStopFilter;

typedef struct FrtMappingFilter
{
    FrtTokenFilter  super;
    FrtMultiMapper *mapper;
} FrtMappingFilter;

typedef struct FrtHyphenFilter
{
    FrtTokenFilter super;
    char text[FRT_MAX_WORD_SIZE];
    int start;
    int pos;
    int len;
    FrtToken *tk;
} FrtHyphenFilter;

typedef struct FrtStemFilter
{
    FrtTokenFilter        super;
    struct sb_stemmer  *stemmer;
    char               *algorithm;
    char               *charenc;
} FrtStemFilter;

#define frt_ts_next(mts) mts->next(mts)
#define frt_ts_clone(mts) mts->clone_i(mts)

extern void frt_ts_deref(FrtTokenStream *ts);

extern FrtTokenStream *frt_non_tokenizer_new();

extern FrtTokenStream *frt_whitespace_tokenizer_new();
extern FrtTokenStream *frt_mb_whitespace_tokenizer_new(bool lowercase);

extern FrtTokenStream *frt_letter_tokenizer_new();
extern FrtTokenStream *frt_mb_letter_tokenizer_new(bool lowercase);

extern FrtTokenStream *frt_standard_tokenizer_new();
extern FrtTokenStream *frt_mb_standard_tokenizer_new();
extern FrtTokenStream *frt_utf8_standard_tokenizer_new();

extern FrtTokenStream *frt_legacy_standard_tokenizer_new();
extern FrtTokenStream *frt_mb_legacy_standard_tokenizer_new();

extern FrtTokenStream *frt_hyphen_filter_new(FrtTokenStream *ts);
extern FrtTokenStream *frt_lowercase_filter_new(FrtTokenStream *ts);
extern FrtTokenStream *frt_mb_lowercase_filter_new(FrtTokenStream *ts);

extern const char *FRT_ENGLISH_STOP_WORDS[];
extern const char *FRT_FULL_ENGLISH_STOP_WORDS[];
extern const char *FRT_EXTENDED_ENGLISH_STOP_WORDS[];
extern const char *FRT_FULL_FRENCH_STOP_WORDS[];
extern const char *FRT_FULL_SPANISH_STOP_WORDS[];
extern const char *FRT_FULL_PORTUGUESE_STOP_WORDS[];
extern const char *FRT_FULL_ITALIAN_STOP_WORDS[];
extern const char *FRT_FULL_GERMAN_STOP_WORDS[];
extern const char *FRT_FULL_DUTCH_STOP_WORDS[];
extern const char *FRT_FULL_SWEDISH_STOP_WORDS[];
extern const char *FRT_FULL_NORWEGIAN_STOP_WORDS[];
extern const char *FRT_FULL_DANISH_STOP_WORDS[];
extern const char *FRT_FULL_RUSSIAN_STOP_WORDS[];
extern const char *FRT_FULL_FINNISH_STOP_WORDS[];
extern const char *FRT_FULL_HUNGARIAN_STOP_WORDS[];
extern const char *FRT_FULL_RUSSIAN_STOP_WORDS_KOI8_R[];

extern FrtTokenStream *frt_stop_filter_new_with_words_len(FrtTokenStream *ts,
                                                   const char **words, int len);
extern FrtTokenStream *frt_stop_filter_new_with_words(FrtTokenStream *ts,
                                               const char **words);
extern FrtTokenStream *frt_stop_filter_new(FrtTokenStream *ts);
extern FrtTokenStream *frt_stem_filter_new(FrtTokenStream *ts, const char *algorithm,
                                    const char *charenc);

extern FrtTokenStream *frt_mapping_filter_new(FrtTokenStream *ts);
extern FrtTokenStream *frt_mapping_filter_add(FrtTokenStream *ts, const char *pattern,
                                       const char *replacement);

/****************************************************************************
 *
 * FrtAnalyzer
 *
 ****************************************************************************/

typedef struct FrtAnalyzer
{
    FrtTokenStream *current_ts;
    FrtTokenStream *(*get_ts)(struct FrtAnalyzer *a, FrtSymbol field, char *text);
    void (*destroy_i)(struct FrtAnalyzer *a);
    int ref_cnt;
} FrtAnalyzer;

extern void frt_a_deref(FrtAnalyzer *a);

#define frt_a_get_ts(ma, field, text) ma->get_ts(ma, field, text)

extern FrtAnalyzer *frt_analyzer_new(FrtTokenStream *ts,
                              void (*destroy)(FrtAnalyzer *a),
                              FrtTokenStream *(*get_ts)(FrtAnalyzer *a,
                                                     FrtSymbol field,
                                                     char *text));
extern void frt_a_standard_destroy(FrtAnalyzer *a);
extern FrtAnalyzer *frt_non_analyzer_new();

extern FrtAnalyzer *frt_whitespace_analyzer_new(bool lowercase);
extern FrtAnalyzer *frt_mb_whitespace_analyzer_new(bool lowercase);

extern FrtAnalyzer *frt_letter_analyzer_new(bool lowercase);
extern FrtAnalyzer *frt_mb_letter_analyzer_new(bool lowercase);

extern FrtAnalyzer *frt_standard_analyzer_new(bool lowercase);
extern FrtAnalyzer *frt_mb_standard_analyzer_new(bool lowercase);
extern FrtAnalyzer *frt_utf8_standard_analyzer_new(bool lowercase);

extern FrtAnalyzer *frt_standard_analyzer_new_with_words(
    const char **words, bool lowercase);
extern FrtAnalyzer *frt_standard_analyzer_new_with_words_len(
    const char **words, int len, bool lowercase);
extern FrtAnalyzer *frt_mb_standard_analyzer_new_with_words(
    const char **words, bool lowercase);
extern FrtAnalyzer *frt_mb_standard_analyzer_new_with_words_len(
    const char **words, int len, bool lowercase);
extern FrtAnalyzer *frt_utf8_standard_analyzer_new_with_words(
    const char **words, bool lowercase);
extern FrtAnalyzer *frt_utf8_standard_analyzer_new_with_words_len(
    const char **words, int len, bool lowercase);

extern FrtAnalyzer *frt_legacy_standard_analyzer_new(bool lowercase);
extern FrtAnalyzer *frt_mb_legacy_standard_analyzer_new(bool lowercase);

extern FrtAnalyzer *frt_legacy_standard_analyzer_new_with_words(
    const char **words, bool lowercase);
extern FrtAnalyzer *frt_legacy_standard_analyzer_new_with_words_len(
    const char **words, int len, bool lowercase);
extern FrtAnalyzer *frt_mb_legacy_standard_analyzer_new_with_words(
    const char **words, bool lowercase);
extern FrtAnalyzer *frt_mb_legacy_standard_analyzer_new_with_words_len(
    const char **words, int len, bool lowercase);

#define PFA(analyzer) ((FrtPerFieldAnalyzer *)(analyzer))
typedef struct FrtPerFieldAnalyzer
{
    FrtAnalyzer    super;
    FrtHash  *dict;
    FrtAnalyzer   *default_a;
} FrtPerFieldAnalyzer;

extern FrtAnalyzer *frt_per_field_analyzer_new(FrtAnalyzer *a);
extern void frt_pfa_add_field(FrtAnalyzer *self,
                              FrtSymbol field,
                              FrtAnalyzer *analyzer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

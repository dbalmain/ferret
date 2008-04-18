#ifndef FRT_ANALYSIS_H
#define FRT_ANALYSIS_H

#include "global.h"
#include "hash.h"
#include "multimapper.h"
#include <wchar.h>

/****************************************************************************
 *
 * FerretToken
 *
 ****************************************************************************/

typedef struct FerretToken
{
    char text[FRT_MAX_WORD_SIZE];
    int len;
    off_t start;
    off_t end;
    int pos_inc;
} FerretToken;

extern FerretToken *frt_tk_new();
extern void frt_tk_destroy(void *p);
extern FerretToken *frt_tk_set(FerretToken *tk, char *text, int tlen, off_t start, off_t end,
                     int pos_inc);
extern FerretToken *frt_tk_set_no_len(FerretToken *tk, char *text, off_t start, off_t end,
                            int pos_inc);
extern int frt_tk_eq(FerretToken *tk1, FerretToken *tk2);
extern int frt_tk_cmp(FerretToken *tk1, FerretToken *tk2);

/****************************************************************************
 *
 * FerretTokenStream
 *
 ****************************************************************************/


typedef struct FerretTokenStream FerretTokenStream;
struct FerretTokenStream
{
    char        *t;             /* ptr used to scan text */
    char        *text;
    FerretToken       *(*next)(FerretTokenStream *ts);
    FerretTokenStream *(*reset)(FerretTokenStream *ts, char *text);
    FerretTokenStream *(*clone_i)(FerretTokenStream *ts);
    void         (*destroy_i)(FerretTokenStream *ts);
    int          ref_cnt;
};

#define frt_ts_new(type) frt_ts_new_i(sizeof(type))
extern FerretTokenStream *frt_ts_new_i(size_t size);
extern FerretTokenStream *frt_ts_clone_size(FerretTokenStream *orig_ts, size_t size);

typedef struct FerretCachedTokenStream
{
    FerretTokenStream super;
    FerretToken       token;
} FerretCachedTokenStream;

typedef struct FerretMultiByteTokenStream
{
    FerretCachedTokenStream super;
    mbstate_t         state;
} FerretMultiByteTokenStream;

typedef struct FerretStandardTokenizer
{
    FerretCachedTokenStream super;
    bool        (*advance_to_start)(FerretTokenStream *ts);
    bool        (*is_tok_char)(char *c);
    int         (*get_alpha)(FerretTokenStream *ts, char *token);
    int         (*get_apostrophe)(char *input);
} FerretStandardTokenizer;

typedef struct FerretTokenFilter
{
    FerretTokenStream super;
    FerretTokenStream *sub_ts;
} FerretTokenFilter;

extern FerretTokenStream *frt_filter_clone_size(FerretTokenStream *ts, size_t size);
#define tf_new(type, sub) frt_tf_new_i(sizeof(type), sub)
extern FerretTokenStream *frt_tf_new_i(size_t size, FerretTokenStream *sub_ts);

typedef struct FerretStopFilter
{
    FerretTokenFilter super;
    FerretHashTable  *words;
} FerretStopFilter;

typedef struct FerretMappingFilter 
{
    FerretTokenFilter  super;
    MultiMapper *mapper;
} FerretMappingFilter;

typedef struct FerretHyphenFilter 
{
    FerretTokenFilter super;
    char text[FRT_MAX_WORD_SIZE];
    int start;
    int pos;
    int len;
    FerretToken *tk;
} FerretHyphenFilter;

typedef struct FerretStemFilter
{
    FerretTokenFilter        super;
    struct sb_stemmer  *stemmer;
    char               *algorithm;
    char               *charenc;
} FerretStemFilter;

#define frt_ts_next(mts) mts->next(mts)
#define frt_ts_clone(mts) mts->clone_i(mts)

extern void frt_ts_deref(FerretTokenStream *ts);

extern FerretTokenStream *frt_non_tokenizer_new();

extern FerretTokenStream *frt_whitespace_tokenizer_new();
extern FerretTokenStream *frt_mb_whitespace_tokenizer_new(bool lowercase);

extern FerretTokenStream *frt_letter_tokenizer_new();
extern FerretTokenStream *frt_mb_letter_tokenizer_new(bool lowercase);

extern FerretTokenStream *frt_standard_tokenizer_new();
extern FerretTokenStream *frt_mb_standard_tokenizer_new();

extern FerretTokenStream *frt_hyphen_filter_new(FerretTokenStream *ts);
extern FerretTokenStream *frt_lowercase_filter_new(FerretTokenStream *ts);
extern FerretTokenStream *frt_mb_lowercase_filter_new(FerretTokenStream *ts);

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

extern FerretTokenStream *frt_stop_filter_new_with_words_len(FerretTokenStream *ts,
                                                   const char **words, int len);
extern FerretTokenStream *frt_stop_filter_new_with_words(FerretTokenStream *ts,
                                               const char **words);
extern FerretTokenStream *frt_stop_filter_new(FerretTokenStream *ts);
extern FerretTokenStream *frt_stem_filter_new(FerretTokenStream *ts, const char *algorithm,
                                    const char *charenc);

extern FerretTokenStream *frt_mapping_filter_new(FerretTokenStream *ts);
extern FerretTokenStream *frt_mapping_filter_add(FerretTokenStream *ts, const char *pattern,
                                       const char *replacement);

/****************************************************************************
 *
 * FerretAnalyzer
 *
 ****************************************************************************/

typedef struct FerretAnalyzer
{
    FerretTokenStream *current_ts;
    FerretTokenStream *(*get_ts)(struct FerretAnalyzer *a, char *field, char *text);
    void (*destroy_i)(struct FerretAnalyzer *a);
    int ref_cnt;
} FerretAnalyzer;

extern void frt_a_deref(FerretAnalyzer *a);

#define frt_a_get_ts(ma, field, text) ma->get_ts(ma, field, text)

extern FerretAnalyzer *frt_analyzer_new(FerretTokenStream *ts,
                              void (*destroy)(FerretAnalyzer *a),
                              FerretTokenStream *(*get_ts)(FerretAnalyzer *a,
                                                     char *field,
                                                     char *text));
extern void frt_a_standard_destroy(FerretAnalyzer *a);
extern FerretAnalyzer *frt_non_analyzer_new();

extern FerretAnalyzer *frt_whitespace_analyzer_new(bool lowercase);
extern FerretAnalyzer *frt_mb_whitespace_analyzer_new(bool lowercase);

extern FerretAnalyzer *frt_letter_analyzer_new(bool lowercase);
extern FerretAnalyzer *frt_mb_letter_analyzer_new(bool lowercase);

extern FerretAnalyzer *frt_standard_analyzer_new(bool lowercase);
extern FerretAnalyzer *frt_mb_standard_analyzer_new(bool lowercase);

extern FerretAnalyzer *frt_standard_analyzer_new_with_words(const char **words,
                                                  bool lowercase);
extern FerretAnalyzer *frt_standard_analyzer_new_with_words_len(const char **words, int len,
                                                      bool lowercase);
extern FerretAnalyzer *frt_mb_standard_analyzer_new_with_words(const char **words,
                                                     bool lowercase);
extern FerretAnalyzer *frt_mb_standard_analyzer_new_with_words_len(const char **words,
                                                  int len, bool lowercase);

#define PFA(analyzer) ((FerretPerFieldAnalyzer *)(analyzer))
typedef struct FerretPerFieldAnalyzer
{
    FerretAnalyzer    super;
    FerretHashTable  *dict;
    FerretAnalyzer   *default_a;
} FerretPerFieldAnalyzer;

extern FerretAnalyzer *frt_per_field_analyzer_new(FerretAnalyzer *a);
extern void frt_pfa_add_field(FerretAnalyzer *self, char *field, FerretAnalyzer *analyzer);

#endif

#include "analysis.h"
#include "hash.h"
#include <libstemmer.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>

/****************************************************************************
 *
 * Token
 *
 ****************************************************************************/

Token *tk_new()
{
    return ALLOC(Token);
}

void tk_destroy(void *p)
{
    free(p);
}

inline Token *tk_set(Token *tk,
                     char *text, int tlen, int start, int end, int pos_inc)
{
    if (tlen >= MAX_WORD_SIZE) {
        tlen = MAX_WORD_SIZE - 1;
    }
    memcpy(tk->text, text, sizeof(char) * tlen);
    tk->text[tlen] = '\0';
    tk->start = start;
    tk->end = end;
    tk->pos_inc = pos_inc;
    return tk;
}

inline Token *tk_set_ts(Token *tk,
                        char *start, char *end, char *text, int pos_inc)
{
    return tk_set(tk, start, (int)(end - start),
                  (int)(start - text), (int)(end - text), pos_inc);
}

inline Token *tk_set_no_len(Token *tk,
                            char *text, int start, int end, int pos_inc)
{
    return tk_set(tk, text, (int)strlen(text), start, end, pos_inc);
}

int tk_eq(Token *tk1, Token *tk2)
{
    return (strcmp((char *)tk1->text, (char *)tk2->text) == 0 &&
            tk1->start == tk2->start && tk1->end == tk2->end);
}

int tk_cmp(Token *tk1, Token *tk2)
{
    int cmp;
    if (tk1->start > tk2->start) {
        cmp = 1;
    }
    else if (tk1->start < tk2->start) {
        cmp = -1;
    }
    else {
        if (tk1->end > tk2->end) {
            cmp = 1;
        }
        else if (tk1->end < tk2->end) {
            cmp = -1;
        }
        else {
            cmp = strcmp((char *)tk1->text, (char *)tk2->text);
        }
    }
    return cmp;
}


/****************************************************************************
 *
 * TokenStream
 *
 ****************************************************************************/

void ts_deref(void *p)
{
    TokenStream *ts = (TokenStream *) p;
    if (--ts->ref_cnt <= 0) {
        ts->destroy_i(ts);
    }
}

void ts_standard_destroy_i(TokenStream *ts)
{
    tk_destroy(ts->token);
    free(ts);
}

TokenStream *ts_reset(TokenStream *ts, char *text)
{
    ts->t = ts->text = text;
    return ts;
}

TokenStream *ts_new()
{
    TokenStream *ts = ALLOC_AND_ZERO_N(TokenStream, 1);
    ts->token = tk_new();
    ts->destroy_i = &ts_standard_destroy_i;
    ts->reset = &ts_reset;
    ts->ref_cnt = 1;
    return ts;
}

TokenStream *ts_clone(TokenStream *orig_ts)
{
    TokenStream *ts = ALLOC(TokenStream);
    memcpy(ts, orig_ts, sizeof(TokenStream));
    if (orig_ts->token) {
        ts->token = ALLOC(Token);
        memcpy(ts->token, orig_ts->token, sizeof(Token));
    }
    if (orig_ts->sub_ts) {
        ts->sub_ts = ts_clone(orig_ts->sub_ts);
    }
    if (orig_ts->clone_i) {
        orig_ts->clone_i(orig_ts, ts);
    }
    ts->ref_cnt = 1;
    return ts;
}

/* * Multi-byte TokenStream * */
static char *const ENC_ERR_MSG = "Error decoding input string. "
"Check that you have the locale set correctly";
#define MB_NEXT_CHAR \
    if ((i = (int)mbrtowc(&wchr, t, MB_CUR_MAX, (mbstate_t *)ts->data)) < 0)\
RAISE(IO_ERROR, ENC_ERR_MSG)

inline Token *w_tk_set(Token *tk, wchar_t *text, int start, int end,
                       int pos_inc)
{
    tk->text[wcstombs(tk->text, text, MAX_WORD_SIZE - 1)] = '\0';
    tk->start = start;
    tk->end = end;
    tk->pos_inc = pos_inc;
    return tk;
}

void mb_ts_standard_destroy_i(TokenStream *ts)
{
    tk_destroy(ts->token);
    free(ts->data);
    free(ts);
}

TokenStream *mb_ts_reset(TokenStream *ts, char *text)
{
    ZEROSET(ts->data, mbstate_t);
    ts_reset(ts, text);
    return ts;
}

void mb_ts_clone_i(TokenStream *orig_ts, TokenStream *new_ts)
{
    new_ts->data = ALLOC(mbstate_t);
    memcpy(new_ts->data, orig_ts->data, sizeof(mbstate_t));
}

TokenStream *mb_ts_new()
{
    TokenStream *ts = ALLOC_AND_ZERO_N(TokenStream, 1);
    ts->data = ALLOC(mbstate_t);
    ts->token = tk_new();
    ts->destroy_i = &mb_ts_standard_destroy_i;
    ts->reset = &mb_ts_reset;
    ts->clone_i = &mb_ts_clone_i;
    ts->ref_cnt = 1;
    return ts;
}

/****************************************************************************
 *
 * Analyzer
 *
 ****************************************************************************/

void a_deref(void *p)
{
    Analyzer *a = (Analyzer *) p;
    if (--a->ref_cnt <= 0) {
        a->destroy_i(a);
    }
}

void a_standard_destroy_i(Analyzer *a)
{
    if (a->current_ts) {
        ts_deref(a->current_ts);
    }
    free(a);
}

TokenStream *a_standard_get_ts(Analyzer *a, char *field, char *text)
{
    TokenStream *ts;
    (void)field;
    ts = ts_clone(a->current_ts);
    return ts->reset(ts, text);
}

Analyzer *analyzer_new(void *data, TokenStream *ts,
                       void (*destroy_i)(Analyzer *a),
                       TokenStream *(*get_ts)(Analyzer *a, char *field,
                                              char *text))
{
    Analyzer *a = ALLOC(Analyzer);
    a->data = data;
    a->current_ts = ts;
    a->destroy_i = (destroy_i ? destroy_i : &a_standard_destroy_i);
    a->get_ts = (get_ts ? get_ts : &a_standard_get_ts);
    a->ref_cnt = 1;
    return a;
}

/****************************************************************************
 *
 * Whitespace
 *
 ****************************************************************************/

/*
 * WhitespaceTokenizer
 */
Token *wst_next(TokenStream *ts)
{
    char *t = ts->t;
    char *start;

    while (*t != '\0' && isspace(*t)) {
        t++;
    }

    if (*t == '\0') {
        return NULL;
    }

    start = t;
    while (*t != '\0' && !isspace(*t)) {
        t++;
    }

    ts->t = t;
    tk_set_ts(ts->token, start, t, ts->text, 1);
    return ts->token;
}

TokenStream *whitespace_tokenizer_new()
{
    TokenStream *ts = ts_new();
    ts->next = &wst_next;
    return ts;
}

/*
 * Multi-byte WhitespaceTokenizer
 */
Token *mb_wst_next(TokenStream *ts)
{
    int i;
    char *start;
    char *t = ts->t;
    wchar_t wchr;

    MB_NEXT_CHAR;
    while (wchr != 0 && iswspace(wchr)) {
        t += i;
        MB_NEXT_CHAR;
    }
    if (wchr == 0) {
        return NULL;
    }

    start = t;
    t += i;
    MB_NEXT_CHAR;
    while (wchr != 0 && !iswspace(wchr)) {
        t += i;
        MB_NEXT_CHAR;
    }
    tk_set_ts(ts->token, start, t, ts->text, 1);
    ts->t = t;
    return ts->token;
}

/*
 * Lowercasing Multi-byte WhitespaceTokenizer
 */
Token *mb_wst_next_lc(TokenStream *ts)
{
    int i;
    char *start;
    char *t = ts->t;
    wchar_t wchr;
    wchar_t wbuf[MAX_WORD_SIZE + 1], *w, *w_end;

    w = wbuf;
    w_end = &wbuf[MAX_WORD_SIZE];

    MB_NEXT_CHAR;
    while (wchr != 0 && iswspace(wchr)) {
        t += i;
        MB_NEXT_CHAR;
    }
    if (wchr == 0) {
        return NULL;
    }

    start = t;
    t += i;
    *w++ = towlower(wchr);
    MB_NEXT_CHAR;
    while (wchr != 0 && !iswspace(wchr)) {
        if (w < w_end) {
            *w++ = towlower(wchr);
        }
        t += i;
        MB_NEXT_CHAR;
    }
    *w = 0;
    w_tk_set(ts->token, wbuf, (int)(start - ts->text), (int)(t - ts->text), 1);
    ts->t = t;
    return ts->token;
}

TokenStream *mb_whitespace_tokenizer_new(bool lowercase)
{
    TokenStream *ts = mb_ts_new();
    ts->next = lowercase ? &mb_wst_next_lc : &mb_wst_next;
    return ts;
}

/*
 * WhitespaceAnalyzers
 */
Analyzer *whitespace_analyzer_new(bool lowercase)
{
    TokenStream *ts;
    if (lowercase) {
        ts = lowercase_filter_new(whitespace_tokenizer_new());
    }
    else {
        ts = whitespace_tokenizer_new();
    }
    return analyzer_new(NULL, ts, NULL, NULL);
}

Analyzer *mb_whitespace_analyzer_new(bool lowercase)
{
    return analyzer_new(NULL, mb_whitespace_tokenizer_new(lowercase),
                        NULL, NULL);
}

/****************************************************************************
 *
 * Letter
 *
 ****************************************************************************/

/*
 * LetterTokenizer
 */
Token *lt_next(TokenStream *ts)
{
    char *start;
    char *t = ts->t;

    while (*t != '\0' && !isalpha(*t)) {
        t++;
    }

    if (*t == '\0') {
        return NULL;
    }

    start = t;
    while (*t != '\0' && isalpha(*t)) {
        t++;
    }

    tk_set_ts(ts->token, start, t, ts->text, 1);
    ts->t = t;
    return ts->token;
}

TokenStream *letter_tokenizer_new()
{
    TokenStream *ts = ts_new();
    ts->next = &lt_next;
    return ts;
}

/*
 * Multi-byte LetterTokenizer
 */
Token *mb_lt_next(TokenStream *ts)
{
    int i;
    char *start;
    char *t = ts->t;
    wchar_t wchr;

    MB_NEXT_CHAR;
    while (wchr != 0 && !iswalpha(wchr)) {
        t += i;
        MB_NEXT_CHAR;
    }

    if (wchr == 0) {
        return NULL;
    }

    start = t;
    t += i;
    MB_NEXT_CHAR;
    while (wchr != 0 && iswalpha(wchr)) {
        t += i;
        MB_NEXT_CHAR;
    }
    tk_set_ts(ts->token, start, t, ts->text, 1);
    ts->t = t;
    return ts->token;
}

/*
 * Lowercasing Multi-byte LetterTokenizer
 */
Token *mb_lt_next_lc(TokenStream *ts)
{
    int i;
    char *start;
    char *t = ts->t;
    wchar_t wchr;
    wchar_t wbuf[MAX_WORD_SIZE + 1], *w, *w_end;

    w = wbuf;
    w_end = &wbuf[MAX_WORD_SIZE];

    MB_NEXT_CHAR;
    while (wchr != 0 && !iswalpha(wchr)) {
        t += i;
        MB_NEXT_CHAR;
    }
    if (wchr == 0) {
        return NULL;
    }

    start = t;
    t += i;
    *w++ = towlower(wchr);
    MB_NEXT_CHAR;
    while (wchr != 0 && iswalpha(wchr)) {
        if (w < w_end) {
            *w++ = towlower(wchr);
        }
        t += i;
        MB_NEXT_CHAR;
    }
    *w = 0;
    w_tk_set(ts->token, wbuf, (int)(start - ts->text), (int)(t - ts->text), 1);
    ts->t = t;
    return ts->token;
}

TokenStream *mb_letter_tokenizer_new(bool lowercase)
{
    TokenStream *ts = mb_ts_new();
    ts->next = lowercase ? &mb_lt_next_lc : &mb_lt_next;
    return ts;
}

/*
 * LetterAnalyzers
 */
Analyzer *letter_analyzer_new(bool lowercase)
{
    TokenStream *ts;
    if (lowercase) {
        ts = lowercase_filter_new(letter_tokenizer_new());
    }
    else {
        ts = letter_tokenizer_new();
    }
    return analyzer_new(NULL, ts, NULL, NULL);
}

Analyzer *mb_letter_analyzer_new(bool lowercase)
{
    return analyzer_new(NULL,
                        mb_letter_tokenizer_new(lowercase), NULL, NULL);
}

/****************************************************************************
 *
 * Standard
 *
 ****************************************************************************/

/*
 * StandardTokenizer
 */
int std_get_alpha(TokenStream *ts, char *token)
{
    int i = 0;
    char *t = ts->t;
    while (t[i] != '\0' && isalpha(t[i])) {
        if (i < MAX_WORD_SIZE) {
            token[i] = t[i];
        }
        i++;
    }
    return i;
}

int mb_std_get_alpha(TokenStream *ts, char *token)
{
    char *t = ts->t;
    wchar_t w;
    int i;
    if ((i = mbtowc(&w, t, MB_CUR_MAX)) < 0) {
        RAISE(IO_ERROR, ENC_ERR_MSG);
    }

    while (w != 0 && iswalpha(w)) {
        t += i;
        if ((i = mbtowc(&w, t, MB_CUR_MAX)) < 0) {
            RAISE(IO_ERROR, ENC_ERR_MSG);
        }
    }

    i = (int)(t - ts->t);
    if (i > MAX_WORD_SIZE) {
        i = MAX_WORD_SIZE - 1;
    }
    memcpy(token, ts->t, i);
    return i;
}

int std_get_alnum(TokenStream *ts, char *token)
{
    int i = 0;
    char *t = ts->t;
    while (t[i] != '\0' && isalnum(t[i])) {
        if (i < MAX_WORD_SIZE) {
            token[i] = t[i];
        }
        i++;
    }
    return i;
}

int mb_std_get_alnum(TokenStream *ts, char *token)
{
    char *t = ts->t;
    wchar_t w;
    int i;
    if ((i = mbtowc(&w, t, MB_CUR_MAX)) < 0) {
        RAISE(IO_ERROR, ENC_ERR_MSG);
    }

    while (w != 0 && iswalnum(w)) {
        t += i;
        if ((i = mbtowc(&w, t, MB_CUR_MAX)) < 0) {
            RAISE(IO_ERROR, ENC_ERR_MSG);
        }
    }

    i = (int)(t - ts->t);
    if (i > MAX_WORD_SIZE) {
        i = MAX_WORD_SIZE - 1;
    }
    memcpy(token, ts->t, i);
    return i;
}

int isnumpunc(char c)
{
    return (c == '.' || c == ',' || c == '\\' || c == '/' || c == '_'
            || c == '-');
}

int w_isnumpunc(wchar_t c)
{
    return (c == L'.' || c == L',' || c == L'\\' || c == L'/' || c == L'_'
            || c == L'-');
}

int isurlpunc(char c)
{
    return (c == '.' || c == '/' || c == '-' || c == '_');
}

int isurlc(char c)
{
    return (c == '.' || c == '/' || c == '-' || c == '_' || isalnum(c));
}

int isurlxatpunc(char c)
{
    return (c == '.' || c == '/' || c == '-' || c == '_' || c == '@');
}

int isurlxatc(char c)
{
    return (c == '.' || c == '/' || c == '-' || c == '_' || c == '@'
            || isalnum(c));
}

bool std_is_tok_char(char *c)
{
    if (isspace(*c)) {
        return false;           /* most common so check first. */
    }
    if (isalnum(*c) || isnumpunc(*c) || *c == '&' ||
        *c == '@' || *c == '\'' || *c == ':') {
        return true;
    }
    return false;
}

bool w_std_is_tok_char(char *t)
{
    wchar_t c;
    if ((mbtowc(&c, t, MB_CUR_MAX)) < 0) {
        RAISE(IO_ERROR, ENC_ERR_MSG);
    }
    if (iswspace(c)) {
        return false;           /* most common so check first. */
    }
    if (iswalnum(c) || w_isnumpunc(c) || c == L'&' ||
        c == L'@' || c == L'\'' || c == L':') {
        return true;
    }
    return false;
}

/* (alnum)((punc)(alnum))+ where every second sequence of alnum must contain at
 * least one digit.
 * (alnum) = [a-zA-Z0-9]
 * (punc) = [_\/.,-]
 */
int std_get_number(char *input)
{
    int i = 0;
    int count = 0;
    int last_seen_digit = 2;
    int seen_digit = false;

    while (last_seen_digit >= 0) {
        while ((input[i] != '\0') && isalnum(input[i])) {
            if ((last_seen_digit < 2) && isdigit(input[i])) {
                last_seen_digit = 2;
            }
            if ((seen_digit == false) && isdigit(input[i])) {
                seen_digit = true;
            }
            i++;
        }
        last_seen_digit--;
        if (!isnumpunc(input[i]) || !isalnum(input[i + 1])) {

            if (last_seen_digit >= 0) {
                count = i;
            }
            break;
        }
        count = i;
        i++;
    }
    if (seen_digit) {
        return count;
    }
    else {
        return 0;
    }
}

int std_get_apostrophe(char *input)
{
    char *t = input;

    while (isalpha(*t) || *t == '\'') {
        t++;
    }

    return (int)(t - input);
}

int mb_std_get_apostrophe(char *input)
{
    char *t = input;
    wchar_t w;
    int i;

    if ((i = mbtowc(&w, t, MB_CUR_MAX)) < 0) {
        RAISE(IO_ERROR, ENC_ERR_MSG);
    }
    while (iswalpha(w) || w == L'\'') {
        t += i;
        if ((i = mbtowc(&w, t, MB_CUR_MAX)) < 0) {
            RAISE(IO_ERROR, ENC_ERR_MSG);
        }
    }
    return (int)(t - input);
}

int std_get_url(char *input, char *token, int i)
{
    while (isurlc(input[i])) {
        if (isurlpunc(input[i]) && isurlpunc(input[i - 1])) {
            break;              /* can't have to puncs in a row */
        }
        if (i < MAX_WORD_SIZE) {
            token[i] = input[i];
        }
        i++;
    }

    /* strip trailing puncs */
    while (isurlpunc(input[i - 1])) {
        i--;
    }

    return i;
}

/* Company names can contain '@' and '&' like AT&T and Excite@Home. Let's
*/
int std_get_company_name(char *input)
{
    int i = 0;
    while (isalpha(input[i]) || input[i] == '@' || input[i] == '&') {
        i++;
    }

    return i;
}

int mb_std_get_company_name(char *input, TokenStream *ts)
{
    char *t = input;
    wchar_t wchr;
    int i;

    MB_NEXT_CHAR;
    while (iswalpha(wchr) || wchr == L'@' || wchr == L'&') {
        t += i;
        MB_NEXT_CHAR;
    }

    return (int)(t - input);
}

bool std_advance_to_start(TokenStream *ts)
{
    char *t = ts->t;
    while (*t != '\0' && !isalnum(*t)) {
        t++;
    }

    ts->t = t;

    return (*t != '\0');
}

bool mb_std_advance_to_start(TokenStream *ts)
{
    int i;
    wchar_t w;

    if ((i = mbtowc(&w, ts->t, MB_CUR_MAX)) < 0) {
        RAISE(IO_ERROR, ENC_ERR_MSG);
    }
    while (w != 0 && !iswalnum(w)) {
        ts->t += i;
        if ((i = mbtowc(&w, ts->t, MB_CUR_MAX)) < 0)
            RAISE(IO_ERROR, ENC_ERR_MSG);
    }

    return (w != 0);
}

typedef struct StandardTokenizer
{
    bool(*advance_to_start) (TokenStream *ts);
    bool(*is_tok_char) (char *c);
    int (*get_alpha) (TokenStream *ts, char *token);
    int (*get_apostrophe) (char *input);
} StandardTokenizer;

Token *std_next(TokenStream *ts)
{
    StandardTokenizer *std_tz = (StandardTokenizer *) ts->data;
    char *s;
    char *t;
    char *start = NULL;
    char *num_end = NULL;
    char token[MAX_WORD_SIZE];
    int token_i = 0;
    int len;
    bool is_acronym;
    bool seen_at_symbol;


    if (!std_tz->advance_to_start(ts)) {
        return NULL;
    }

    start = t = ts->t;
    if (isdigit(*t)) {
        t += std_get_number(t);
        ts->t = t;
        tk_set_ts(ts->token, start, t, ts->text, 1);
    }
    else {
        token_i = std_tz->get_alpha(ts, token);
        t += token_i;

        if (!std_tz->is_tok_char(t)) {
            /* very common case, ie a plain word, so check and return */
            tk_set_ts(ts->token, start, t, ts->text, 1);
            ts->t = t;
            return ts->token;
        }

        if (*t == '\'') {       /* apostrophe case. */
            t += std_tz->get_apostrophe(t);
            ts->t = t;
            len = (int)(t - start);
            /* strip possesive */
            if ((t[-1] == 's' || t[-1] == 'S') && t[-2] == '\'') {
                t -= 2;
            }

            tk_set_ts(ts->token, start, t, ts->text, 1);
            return ts->token;
        }

        if (*t == '&') {        /* apostrophe case. */
            t += std_get_company_name(t);
            ts->t = t;
            tk_set_ts(ts->token, start, t, ts->text, 1);
            return ts->token;
        }

        if (isdigit(*t) || isnumpunc(*t)) {      /* possibly a number */
            num_end = start + std_get_number(start);
            if (!std_tz->is_tok_char(num_end)) { /* we won't find a longer token */
                ts->t = num_end;
                tk_set_ts(ts->token, start, num_end, ts->text, 1);
                return ts->token;
            }
            /* else there may be a longer token so check */
        }

        if (t[0] == ':' && t[1] == '/' && t[2] == '/') {
            /* check for a known url start */
            token[token_i] = '\0';
            t += 3;
            while (*t == '/') {
                t++;
            }
            if (isalpha(*t) &&
                (memcmp(token, "ftp", 3) == 0 ||
                 memcmp(token, "http", 4) == 0 ||
                 memcmp(token, "https", 5) == 0 ||
                 memcmp(token, "file", 4) == 0)) {
                len = std_get_url(t, token, 0); /* dispose of first part of the URL */
            }
            else {              /* still treat as url but keep the first part */
                token_i = (int)(t - start);
                memcpy(token, start, token_i * sizeof(char));
                len = token_i + std_get_url(t, token, token_i); /* keep start */
            }
            ts->t = t + len;
            token[len] = 0;
            tk_set(ts->token, token, len, (int)(start - ts->text),
                   (int)(ts->t - ts->text), 1);
            return ts->token;
        }

        /* now see how long a url we can find. */
        is_acronym = true;
        seen_at_symbol = false;
        while (isurlxatc(*t)) {
            if (is_acronym && !isalpha(*t) && (*t != '.')) {
                is_acronym = false;
            }
            if (isurlxatpunc(*t) && isurlxatpunc(t[-1])) {
                break; /* can't have two punctuation characters in a row */
            }
            if (*t == '@') {
                if (seen_at_symbol) {
                    break; /* we can only have one @ symbol */
                }
                else {
                    seen_at_symbol = true;
                }
            }
            t++;
        }
        while (isurlxatpunc(t[-1])) {
            t--;                /* strip trailing punctuation */
        }

        if (t > num_end) {
            ts->t = t;

            if (is_acronym) {   /* check that it is one letter followed by one '.' */
                for (s = start; s < t - 1; s++) {
                    if (isalpha(*s) && (s[1] != '.'))
                        is_acronym = false;
                }
            }
            if (is_acronym) {   /* strip '.'s */
                for (s = start + token_i; s < t; s++) {
                    if (*s != '.') {
                        token[token_i] = *s;
                        token_i++;
                    }
                }
                tk_set(ts->token, token, token_i, (int)(start - ts->text),
                       (int)(t - ts->text), 1);
            }
            else { /* just return the url as is */
                tk_set_ts(ts->token, start, t, ts->text, 1);
            }
        }
        else {                  /* return the number */
            ts->t = num_end;
            tk_set_ts(ts->token, start, num_end, ts->text, 1);
        }
    }

    return ts->token;
}

void std_ts_destroy_i(TokenStream *ts)
{
    free(ts->data);
    ts_standard_destroy_i(ts);
}

void std_ts_clone_i(TokenStream *orig_ts, TokenStream *new_ts)
{
    new_ts->data = ALLOC(StandardTokenizer);
    memcpy(new_ts->data, orig_ts->data, sizeof(StandardTokenizer));
}

TokenStream *standard_tokenizer_new()
{
    TokenStream *ts = ts_new();

    StandardTokenizer *std_tz = ALLOC(StandardTokenizer);
    std_tz->advance_to_start = &std_advance_to_start;
    std_tz->get_alpha = &std_get_alpha;
    std_tz->is_tok_char = &std_is_tok_char;
    std_tz->get_apostrophe = &std_get_apostrophe;

    ts->data = std_tz;
    ts->destroy_i = &std_ts_destroy_i;
    ts->clone_i = &std_ts_clone_i;
    ts->next = &std_next;
    return ts;
}

TokenStream *mb_standard_tokenizer_new()
{
    TokenStream *ts = ts_new();

    StandardTokenizer *std_tz = ALLOC(StandardTokenizer);
    std_tz->advance_to_start = &mb_std_advance_to_start;
    std_tz->get_alpha = &mb_std_get_alpha;
    std_tz->is_tok_char = &w_std_is_tok_char;
    std_tz->get_apostrophe = &mb_std_get_apostrophe;

    ts->data = std_tz;
    ts->destroy_i = &std_ts_destroy_i;
    ts->clone_i = &std_ts_clone_i;
    ts->next = &std_next;
    return ts;
}

TokenStream *filter_reset(TokenStream *ts, char *text)
{
    ts->sub_ts->reset(ts->sub_ts, text);
    return ts;
}

void filter_destroy_i(TokenStream *tf)
{
    ts_deref(tf->sub_ts);
    if (tf->token != NULL) {
        tk_destroy(tf->token);
    }
    free(tf);
}

void sf_destroy_i(TokenStream *tf)
{
    HashTable *words = (HashTable *)tf->data;
    h_destroy(words);
    filter_destroy_i(tf);
}

void sf_clone_i_i(void *key, void *value, void *arg)
{
    HashTable *wordtable = (HashTable *)arg;
    char *w = estrdup(key);
    (void)value;
    h_set(wordtable, w, w);
}

void sf_clone_i(TokenStream *orig_ts, TokenStream *new_ts)
{
    new_ts->data = h_new_str(&free, NULL);
    h_each(orig_ts->data, &sf_clone_i_i, new_ts->data);
}

Token *sf_next(TokenStream *tf)
{
    int pos_inc = 1;
    HashTable *words = (HashTable *) tf->data;
    Token *tk = tf->sub_ts->next(tf->sub_ts);
    while ((tk != NULL) && (h_get(words, tk->text) != NULL)) {
        tk = tf->sub_ts->next(tf->sub_ts);
        pos_inc++;
    }
    if (tk != NULL) {
        tk->pos_inc = pos_inc;
    }
    return tk;
}

TokenStream *stop_filter_new_with_words_len(TokenStream *ts,
                                            const char **words, int len)
{
    int i;
    char *w;
    HashTable *wordtable = h_new_str(&free, (free_ft) NULL);
    TokenStream *tf = ALLOC(TokenStream);
    tf->sub_ts = ts;

    for (i = 0; i < len; i++) {
        w = estrdup(words[i]);
        h_set(wordtable, w, w);
    }
    tf->data = wordtable;
    tf->token = NULL;
    tf->next = &sf_next;
    tf->reset = &filter_reset;
    tf->destroy_i = &sf_destroy_i;
    tf->clone_i = &sf_clone_i;
    tf->ref_cnt = 1;
    return tf;
}

TokenStream *stop_filter_new_with_words(TokenStream *ts,
                                        const char **words)
{
    char *w;
    HashTable *wordtable = h_new_str(&free, (free_ft) NULL);
    TokenStream *tf = ALLOC(TokenStream);
    tf->sub_ts = ts;
    while (*words) {
        w = estrdup(*words);
        h_set(wordtable, w, w);
        words++;
    }
    tf->data = wordtable;
    tf->token = NULL;
    tf->next = &sf_next;
    tf->reset = &filter_reset;
    tf->destroy_i = &sf_destroy_i;
    tf->clone_i = &sf_clone_i;
    tf->ref_cnt = 1;
    return tf;
}

TokenStream *stop_filter_new(TokenStream *ts)
{
    return stop_filter_new_with_words(ts, FULL_ENGLISH_STOP_WORDS);
}

Token *mb_lcf_next(TokenStream *ts)
{
    wchar_t wbuf[MAX_WORD_SIZE], *w;
    /*mbstate_t state = {0}; */
    int i;
    Token *tk = ts->sub_ts->next(ts->sub_ts);
    if (tk == NULL) {
        return tk;
    }

    i = (int)mbstowcs(wbuf, tk->text, MAX_WORD_SIZE);
    w = wbuf;
    while (*w != 0) {
        *w = towlower(*w);
        w++;
    }
    wcstombs(tk->text, wbuf, MAX_WORD_SIZE);
    return tk;
}

TokenStream *mb_lowercase_filter_new(TokenStream *ts)
{
    TokenStream *tf = ALLOC(TokenStream);
    tf->token = NULL;
    tf->next = &mb_lcf_next;
    tf->reset = &filter_reset;
    tf->destroy_i = &filter_destroy_i;
    tf->sub_ts = ts;
    tf->clone_i = NULL;
    tf->ref_cnt = 1;
    return tf;
}

Token *lcf_next(TokenStream *ts)
{
    int i = 0;
    Token *tk = ts->sub_ts->next(ts->sub_ts);
    if (tk == NULL) {
        return tk;
    }
    while (tk->text[i] != '\0') {
        tk->text[i] = tolower(tk->text[i]);
        i++;
    }
    return tk;
}

TokenStream *lowercase_filter_new(TokenStream *ts)
{
    TokenStream *tf = ALLOC(TokenStream);
    tf->token = NULL;
    tf->next = &lcf_next;
    tf->reset = &filter_reset;
    tf->destroy_i = &filter_destroy_i;
    tf->sub_ts = ts;
    tf->clone_i = NULL;
    tf->ref_cnt = 1;
    return tf;
}

typedef struct StemFilter
{
    struct sb_stemmer *stemmer;
    char *algorithm;
    char *charenc;
} StemFilter;

void stemf_destroy_i(TokenStream *tf)
{
    StemFilter *stemf = (StemFilter *)tf->data;
    sb_stemmer_delete(stemf->stemmer);
    free(stemf->algorithm);
    free(stemf->charenc);
    free(stemf);
    filter_destroy_i(tf);
}

Token *stemf_next(TokenStream *ts)
{
    int len;
    const sb_symbol *stemmed;
    struct sb_stemmer *stemmer = ((StemFilter *) ts->data)->stemmer;
    Token *tk = ts->sub_ts->next(ts->sub_ts);
    if (tk == NULL) {
        return tk;
    }
    stemmed = sb_stemmer_stem(stemmer, (sb_symbol *) tk->text,
                              (int)strlen(tk->text));
    len = sb_stemmer_length(stemmer);
    if (len >= MAX_WORD_SIZE) {
        len = MAX_WORD_SIZE - 1;
    }

    memcpy(tk->text, stemmed, len);
    tk->text[len] = '\0';
    return tk;
}

void stemf_clone_i(TokenStream *orig_ts, TokenStream *new_ts)
{
    StemFilter *orig_stemf = (StemFilter *) orig_ts->data;
    StemFilter *stemf = ALLOC(StemFilter);
    stemf->stemmer =
        sb_stemmer_new(orig_stemf->algorithm, orig_stemf->charenc);
    stemf->algorithm =
        orig_stemf->algorithm ? estrdup(orig_stemf->algorithm) : NULL;
    stemf->charenc =
        orig_stemf->charenc ? estrdup(orig_stemf->charenc) : NULL;
    new_ts->data = stemf;
}

TokenStream *stem_filter_new(TokenStream *ts, const char *algorithm,
                             const char *charenc)
{
    TokenStream *tf = ALLOC(TokenStream);
    StemFilter *stemf = ALLOC(StemFilter);
    stemf->stemmer = sb_stemmer_new(algorithm, charenc);
    stemf->algorithm = algorithm ? estrdup(algorithm) : NULL;
    stemf->charenc = charenc ? estrdup(charenc) : NULL;
    tf->data = stemf;

    tf->token = NULL;
    tf->next = &stemf_next;
    tf->reset = &filter_reset;
    tf->destroy_i = &stemf_destroy_i;
    tf->clone_i = &stemf_clone_i;
    tf->sub_ts = ts;
    tf->ref_cnt = 1;
    return tf;
}

Analyzer *standard_analyzer_new_with_words_len(const char **words, int len,
                                               bool lowercase)
{
    TokenStream *ts;
    if (lowercase) {
        ts = stop_filter_new_with_words_len(lowercase_filter_new
                                            (standard_tokenizer_new()),
                                            words, len);
    }
    else {
        ts = stop_filter_new_with_words_len(standard_tokenizer_new(),
                                            words, len);
    }
    return analyzer_new(NULL, ts, NULL, NULL);
}

Analyzer *standard_analyzer_new_with_words(const char **words,
                                           bool lowercase)
{
    TokenStream *ts;
    if (lowercase) {
        ts = stop_filter_new_with_words(lowercase_filter_new
                                        (standard_tokenizer_new()),
                                        words);
    }
    else {
        ts = stop_filter_new_with_words(standard_tokenizer_new(),
                                        words);
    }
    return analyzer_new(NULL, ts, NULL, NULL);
}

Analyzer *mb_standard_analyzer_new_with_words_len(const char **words,
                                                  int len, bool lowercase)
{
    TokenStream *ts;
    if (lowercase) {
        ts = stop_filter_new_with_words_len(mb_lowercase_filter_new
                                            (mb_standard_tokenizer_new
                                             ()), words, len);
    }
    else {
        ts = stop_filter_new_with_words_len(mb_standard_tokenizer_new(),
                                            words, len);
    }
    return analyzer_new(NULL, ts, NULL, NULL);
}

Analyzer *mb_standard_analyzer_new_with_words(const char **words,
                                              bool lowercase)
{
    TokenStream *ts;
    if (lowercase) {
        ts = stop_filter_new_with_words(mb_lowercase_filter_new
                                        (mb_standard_tokenizer_new()),
                                        words);
    }
    else {
        ts = stop_filter_new_with_words(mb_standard_tokenizer_new(),
                                        words);
    }
    return analyzer_new(NULL, ts, NULL, NULL);
}

Analyzer *standard_analyzer_new(bool lowercase)
{
    return standard_analyzer_new_with_words(FULL_ENGLISH_STOP_WORDS,
                                            lowercase);
}

Analyzer *mb_standard_analyzer_new(bool lowercase)
{
    return mb_standard_analyzer_new_with_words(FULL_ENGLISH_STOP_WORDS,
                                               lowercase);
}

/****************************************************************************
 *
 * PerFieldAnalyzer
 *
 ****************************************************************************/

void pfa_destroy_i(Analyzer *self)
{
    PerFieldAnalyzer *pfa = (PerFieldAnalyzer *) self->data;
    h_destroy(pfa->dict);

    a_deref(pfa->def);
    free(pfa);
    free(self);
}

TokenStream *pfa_get_ts(Analyzer *self, char *field, char *text)
{
    PerFieldAnalyzer *pfa = (PerFieldAnalyzer *) self->data;
    Analyzer *a = h_get(pfa->dict, field);
    if (a == NULL) {
        a = pfa->def;
    }
    return a_get_ts(a, field, text);
}

void pfa_sub_a_destroy_i(void *p)
{
    Analyzer *a = (Analyzer *) p;
    a_deref(a);
}

void pfa_add_field(Analyzer *self, char *field, Analyzer *analyzer)
{
    PerFieldAnalyzer *pfa = (PerFieldAnalyzer *) self->data;
    h_set(pfa->dict, estrdup(field), analyzer);
}

Analyzer *per_field_analyzer_new(Analyzer *def)
{
    PerFieldAnalyzer *pfa = ALLOC(PerFieldAnalyzer);
    pfa->def = def;
    pfa->dict = h_new_str(&free, &pfa_sub_a_destroy_i);
    return analyzer_new(pfa, NULL, &pfa_destroy_i, &pfa_get_ts);
}

#ifdef ALONE
int main(int argc, char **argv)
{
    char buf[10000];
    Analyzer *a = standard_analyzer_new(true);
    TokenStream *ts;
    Token *tk;
    while (fgets(buf, 9999, stdin) != NULL) {
        ts = a_get_ts(a, "hello", buf);
        while ((tk = ts->next(ts)) != NULL) {
            printf("<%s:%ld:%ld> ", tk->text, tk->start, tk->end);
        }
        printf("\n");
        ts_deref(ts);
    }
    return 0;
}
#endif

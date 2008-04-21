/* scanner.rl -*-C-*- */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include "global.h"
#include "internal.h"

#define RET goto ret;

#define STRIP(c) do { \
    strip_char = c;   \
    goto ret;         \
} while(0)

%%{
    machine StdTok;
    alphtype unsigned int;
    include WChar "uchar-ucs4.rl";

    frt_alpha = ualpha;
    frt_alnum = ualnum;
    frt_digit = udigit;

    include StdTok "scanner.in";

    main := any @{ fhold; fcall frt_tokenizer; };
}%%

%% write data nofinal;

static const char *position_in_mb( const unsigned int *orig_wc,
                                   const char *orig_mb,
                                   const unsigned int *curr_wc )
{
    const char         *mb = orig_mb;
    const unsigned int *wc = orig_wc;

    while (wc < curr_wc)
    {
        char buf[MB_LEN_MAX];
        mb += wctomb(buf, *wc);
        ++wc;
    }

    return mb;
}

static int mb_next_char(unsigned int *wchr, const char *s, mbstate_t *state)
{
    int num_bytes;
    if ((num_bytes = (int)mbrtowc((wchar_t*)wchr, s, MB_CUR_MAX, state)) < 0) {
        const char *t = s;
        do {
            t++;
            ZEROSET(state, mbstate_t);
            num_bytes = (int)mbrtowc((wchar_t*)wchr, t, MB_CUR_MAX, state);
        } while ((num_bytes < 0) && (*t != 0));
        num_bytes = t - s;
        if (*t == 0) *wchr = 0;
    }
    return num_bytes;
}

static int wc_next_char(char *s, const unsigned int *wchr, mbstate_t *state)
{
    return (int)wcrtomb(s, *wchr, state);
}

/*
 * All input to Ragel must be in a buffer of 32bit unicode codepoints.
 * To that end, we require that the input to the scanner must be in a
 * codepage that mbtowc will convert to unicode codepoints.  The easy
 * way to do this is to supply the scanner with UTF8.  It will call
 * mbtowc on the buffer, pass it to the tokenizer which will extract
 * one token out.  This token will then be converted back with wctomb.
 *
 * frt_scan_mb takes in a pointer to the mb buffer +inmb+ that has max
 * size +in_size+.  The resulting token will be stored in +outmb+,
 * with at most +out_size+ bytes written.
 *
 * While tokenizing, part of the token may be stripped out. Eg,
 * 'foo!!' will become 'foo', and 'http://www.bar.com' will become
 * 'www.bar.com'.  So that the caller can track what exactly has been
 * tokenized in +inmb+, +startmb+ and +endmb+ are set to point to the
 * extremeties of the original unmodified token in +inmb+.
 *
 * The size of the token written out to +outmb+ is stored in
 * +token_size+.
 */

static void mb_to_wc(const char *in,
                     unsigned int *out, size_t out_size)
{
    mbstate_t state;
    const char *in_p     = in;
    unsigned int *out_p = out;
    ZEROSET(&state, mbstate_t);

    while (*in_p && out_p < (out + out_size/sizeof(*out)))
    {
        /* We can break out early here on, say, a space XXX */

        int n = mb_next_char(out_p, in_p, &state);
        if (n < 0)
        {
            ++in_p;
            continue;
        }

        in_p += n;
        ++out_p;
    }
}

static void wc_to_mb(char *out, size_t out_size, int *token_size,
                     const unsigned int *in_wc, size_t in_wc_size)
{
    mbstate_t state;
    char *out_p = out;
    const unsigned int *in_wc_p = in_wc;
    ZEROSET(&state, mbstate_t);
    *token_size = 0;

    while (out_p < (out + out_size) && in_wc_p < (in_wc + in_wc_size))
    {
        if (!*in_wc_p)
            break;

        int n = wc_next_char(out_p, in_wc_p, &state);
        if (n < 0)
        {
            ++in_wc_p;
            continue;
        }

        out_p += n;
        ++in_wc_p;
    }

    *token_size = out_p - out;
}

void frt_std_scan_mb(const char *in_mb,
                     char *out_mb, size_t out_mb_size,
                     const char **start_mb,
                     const char **end_mb,
                     int *token_size)
{
    int cs, act, top;
    int stack[32];
    unsigned int *ts = 0, *te = 0;

    %% write init;

    unsigned int in_wc[4096] = {0};
    mb_to_wc(in_mb, in_wc, sizeof(in_wc));

    unsigned int *p = in_wc, *pe = 0, *eof = pe;
    int skip = 0;
    int trunc = 0;
    unsigned int strip_char = 0;

    *end_mb = 0;
    *start_mb = 0;
    *token_size = 0;

    %% write exec;

    if ( cs == StdTok_error )
                   fwprintf(stderr, L"PARSE ERROR\n");
    else if ( ts ) fwprintf(stderr, L"STUFF LEFT: '%ls'\n", ts);
    return;

 ret:
    {
        unsigned int out_wc[4096] = {0};
        size_t __len = te - ts - skip - trunc;

        *start_mb = position_in_mb(in_wc, in_mb, ts);
        *end_mb   = position_in_mb(in_wc, in_mb, te);

        if (strip_char) {
            unsigned int *__p = ts + skip;
            unsigned int *__o = out_wc;
            for (; __p < (ts + skip + __len); ++__p) {
                if (*__p != strip_char)
                    *__o++ = *__p;
            }
        }
        else {
            memcpy(out_wc, ts + skip, __len*sizeof(unsigned int));
        }

        wc_to_mb(out_mb, out_mb_size, token_size, out_wc, sizeof(out_wc));
        out_mb[*token_size] = 0;
    }
}

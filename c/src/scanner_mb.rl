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

static char *position_in_mb( wchar_t *orig_wc, char *orig_mb,
                             wchar_t *curr_wc )
{
    char    *mb = orig_mb;
    wchar_t *wc = orig_wc;

    while (wc < curr_wc)
    {
        char buf[MB_LEN_MAX];
        mb += wctomb(buf, *wc);
        ++wc;
    }

    return mb;
}

#define RET do {                        \
    size_t __len = te - ts - skip - trunc; \
    if (__len > out_size)               \
        __len = out_size;               \
    *len     = __len;                   \
    *start   = position_in_mb(buf, in, ts); \
    *end     = position_in_mb(buf, in, te); \
     memcpy(out, *start + skip, __len);     \
     out[__len] = 0;                    \
     return;                            \
} while(0)

#define STRIP(c) do {                         \
    unsigned long *__p = ts;                  \
    unsigned long *__o = out;                 \
    unsigned long *__max = __p + out_size;    \
    for (; __p <= te && __p < __max; ++__p) { \
        if (*__p != c)                        \
            *__o++ = *__p;                    \
    }                                         \
    *__o = 0;                                 \
                                              \
    *start = ts;                              \
    *end   = te;                              \
    *len   = __o - (unsigned long*)out;                       \
    return;                                   \
} while(0)

%%{
    machine StdTokMb;
    alphtype unsigned long;

    delim = space;
    token = alpha alnum*;
    punc  = [.,\/_\-];
    proto = 'http'[s]? | 'ftp' | 'file';
    urlc  = alnum | punc | [\@\:];

    main := |*

        #// Token, or token with possessive
        token           { RET; };
        token [\']      { trunc = 1; RET; };
        token [\'][sS]? { trunc = 2; RET; };

        #// Token with hyphens
        alnum+ ('-' alnum+)* { RET; };

        #// Company name
        token [\&\@] token* { RET; };

        #// URL
        proto [:][/]+ %{ skip = p - ts; } urlc+ [/] { trunc = 1; RET; };
        proto [:][/]+ %{ skip = p - ts; } urlc+     { RET; };
        alnum+[:][/]+ urlc+ [/] { trunc = 1; RET; };
        alnum+[:][/]+ urlc+     { RET; };

        #// Email
        alnum+ '@' alnum+ '.' alpha+ { RET; };

        #// Acronym
        (alpha '.')+ alpha { STRIP('.'); };

        #// Int+float
        [\-\+]?digit+            { RET; };
        [\-\+]?digit+ '.' digit+ { RET; };

        #// Ignore whitespace and other crap
        0 { return; };
        (any - alnum);

        *|;
}%%

%% write data nofinal;

static int mb_next_char(wchar_t *wchr, const char *s, mbstate_t *state)
{
    int num_bytes;
    if ((num_bytes = (int)mbrtowc(wchr, s, MB_CUR_MAX, state)) < 0) {
        const char *t = s;
        do {
            t++;
            ZEROSET(state, mbstate_t);
            num_bytes = (int)mbrtowc(wchr, t, MB_CUR_MAX, state);
        } while ((num_bytes < 0) && (*t != 0));
        num_bytes = t - s;
        if (*t == 0) *wchr = 0;
    }
    return num_bytes;
}

/* Function takes in a multibyte string, converts it to wide-chars and
   then tokenizes that. */
void frt_std_scan_mb(const char *in, size_t in_size,
                 char *out, size_t out_size,
                 char **start, char **end,
                 int *len)
{
    int cs, act;
    unsigned long *ts, *te = 0;

    %% write init;

    unsigned long buf[4096] = {0};
    unsigned long *bufp = buf;
    char *inp = (char *)in;
    mbstate_t state;
    ZEROSET(&state, mbstate_t);
    printf ("TRYING TO PARSE: '%s'\n", in);
    while (inp < (in + in_size) && bufp < (buf + sizeof(buf)/sizeof(*buf)))
    {
        if (!*inp)
            break;

        int n = mb_next_char((wchar_t *)bufp, inp, &state);
        if (n < 0)
        {
            printf ("Error parsing input\n");
            ++inp;
            continue;
        }

        /* We can break out early here on, say, a space XXX */
        inp += n;
        ++bufp;
    }

    printf ("parsed: %d\n", inp - in);

    unsigned long *p = (unsigned long *)buf;
    unsigned long *pe = bufp;
    unsigned long *eof = pe;
    *len = 0;
    int skip = 0;
    int trunc = 0;

    %% write exec;

    if ( cs == StdTokMb_error )
    {
        fprintf(stderr, "PARSE ERROR\n" );
        return;
    }

    if ( ts )
    {
        fprintf(stderr, "STUFF LEFT: '%s'\n", ts);
    }
}

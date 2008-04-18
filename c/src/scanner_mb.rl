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

static char *position_in_mb( const unsigned long *orig_wc,
                             const char *orig_mb,
                             const unsigned long *curr_wc )
{
    char          *mb = (char *)orig_mb;
    unsigned long *wc = (char *)orig_wc;

    while (wc < curr_wc)
    {
        char buf[MB_LEN_MAX];
        mb += wctomb(buf, *wc);
        ++wc;
    }

    return mb;
}

#define RET do {                        \
    *start   = position_in_mb(buf, in, ts); \
    *end     = position_in_mb(buf, in, te); \
    size_t __len = *end - *start - skip - trunc; \
    *len     = __len;                   \
    if (__len > out_size)               \
        __len = out_size;               \
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
    *start = position_in_mb(buf, in, ts);                              \
    *end   = position_in_mb(buf, in, te);                              \
    *len   = (char *)__o - (char *)out;                       \
    return;                                   \
} while(0)

%%{
    machine StdTokMb;
    alphtype unsigned long;
    include WChar "src/wchar.rl";

    delim = space;
    token = walpha walnum*;
    punc  = [.,\/_\-];
    proto = 'http'[s]? | 'ftp' | 'file';
    urlc  = walnum | punc | [\@\:];

    main := |*

        #// Token, or token with possessive
        token           { RET; };
        token [\']      { trunc = 1; RET; };
        token [\'][sS]? { trunc = 2; RET; };

        #// Token with hyphens
        walnum+ ('-' walnum+)* { RET; };

        #// Company name
        token [\&\@] token* { RET; };

        #// URL
        proto [:][/]+ %{ skip = p - ts; } urlc+ [/] { trunc = 1; RET; };
        proto [:][/]+ %{ skip = p - ts; } urlc+     { RET; };
        walnum+[:][/]+ urlc+ [/] { trunc = 1; RET; };
        walnum+[:][/]+ urlc+     { RET; };

        #// Email
        walnum+ '@' walnum+ '.' walpha+ { RET; };

        #// Acronym
        (walpha '.')+ walpha { STRIP('.'); };

        #// Int+float
        [\-\+]?wdigit+            { RET; };
        [\-\+]?wdigit+ '.' wdigit+ { RET; };

        #// Ignore whitespace and other crap
        0 { return; };
        (any - walnum);

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
    wprintf (L"%ls\n", buf);
    printf ("%04x\n", buf[5]);

    unsigned long *p = (unsigned long *)buf;
    unsigned long *pe = 0;
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
        fwprintf(stderr, L"STUFF LEFT: '%ls'\n", ts);
    }
}

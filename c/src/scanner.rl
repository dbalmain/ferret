/* scanner.rl -*-C-*- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define RET do {                        \
    size_t __len = te - ts - skip - trunc; \
    if (__len > out_size)               \
        __len = out_size;               \
    *len     = __len;                   \
    *start   = ts;                      \
    *end     = te;                      \
     memcpy(out, ts + skip, __len);     \
     out[__len] = 0;                    \
     return;                            \
} while(0)

#define STRIP(c) do {                         \
    char *__p = ts;                           \
    char *__o = out;                          \
    char *__max = __p + out_size;             \
    for (; __p <= te && __p < __max; ++__p) { \
        if (*__p != c)                        \
            *__o++ = *__p;                    \
    }                                         \
    *__o = 0;                                 \
                                              \
    *start = ts;                              \
    *end   = te;                              \
    *len   = __o - out;                       \
    return;                                   \
} while(0)

%%{
    machine StdTok;

    word  = alnum;
    delim = space;
    token = alpha word*;
    punc  = [.,\/_\-];
    proto = 'http'[s]? | 'ftp' | 'file';
    urlc  = alnum | punc | [\@\:];

    main := |*

        #// Token, or token with possessive
        token           { RET; };
        token [\']      { trunc = 1; RET; };
        token [\'][sS]? { trunc = 2; RET; };

        #// Token with hyphens
        alnum+ ([\-_] alnum+)* { RET; };

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

void frt_std_scan(const char *in,
                  char *out, size_t out_size,
                  char **start, char **end,
                  int *len)
{
    int cs, act;
    char *ts, *te = 0;

    %% write init;

    char *p = (char *)in;
    char *pe = 0;
    char *eof = pe;
    *len = 0;
    int skip = 0;
    int trunc = 0;

    %% write exec;

    if ( cs == StdTok_error )
    {
        fprintf(stderr, "PARSE ERROR\n" );
        return;
    }

    if ( ts )
    {
        fprintf(stderr, "STUFF LEFT: '%s'\n", ts);
    }
}

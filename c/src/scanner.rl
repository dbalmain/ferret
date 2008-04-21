/* scanner.rl -*-C-*- */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "global.h"
#include "internal.h"

#define RET goto ret;

#define STRIP(c) do { \
    strip_char = c;   \
    goto ret;         \
} while(0)

%%{
    machine StdTok;
    alphtype unsigned char;

    frt_alpha = alpha;
    frt_alnum = alnum;
    frt_digit = digit;

    include StdTok "scanner.in";

    main := any @{ fhold; fcall frt_tokenizer; };
}%%

%% write data nofinal;

void frt_std_scan(const char *in,
                  char *out, size_t out_size,
                  const char **start,
                  const char **end,
                  int *token_size)
{
    int cs, act, top;
    int stack[32];
    char *ts = 0, *te = 0;

    %% write init;

    char *p = (char *)in, *pe = 0, *eof = pe;
    int skip = 0;
    int trunc = 0;
    char strip_char = 0;

    *end = 0;
    *start = 0;
    *token_size = 0;

    %% write exec;

    if ( cs == StdTok_error )
                   fprintf(stderr, "PARSE ERROR\n" );
    else if ( ts ) fprintf(stderr, "STUFF LEFT: '%s'\n", ts);
    return;

 ret:
    {
        size_t __len = te - ts - skip - trunc;
        if (__len > out_size)
            __len = out_size;

        *start = ts;
        *end   = te;

        if (strip_char) {
            char *__p = ts + skip;
            char *__o = out;
            for (; __p < (ts + skip + __len); ++__p) {
                if (*__p != strip_char)
                    *__o++ = *__p;
            }
            *token_size = __o - out;
        }
        else {
            memcpy(out, ts + skip, __len);
            *token_size = __len;
        }

        out[*token_size] = 0;
    }
}

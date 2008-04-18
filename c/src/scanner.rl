/* scanner.rl -*-C-*- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PUTS(desc) do {           \
    printf(desc);                 \
    fwrite(ts, 1, te-ts, stdout); \
    printf("\n");                 \
} while(0)

#define RET do {    \
    *start = ts;    \
    *len   = te-ts; \
    return;         \
} while (0)

%%{
    machine StdTok;

    delim = space;
    token = alpha alnum*;
    punc  = [.,\/_\-];
    proto = 'http'[s]? | 'ftp' | 'file';
    urlc  = alnum | punc | [\@\:];

    main := |*

        #// Token, or token with possessive
        token           { RET; };
        token [\'][sS]? { RET; };

        #// Company name
        token [\&\@] token* { RET; };

        #// URL
        proto [:][/]+ urlc+ { RET; };
        alnum+[:][/]+ urlc+ { RET; };

        #// Email
        alnum+ '@' alnum+ '.' alpha+ { RET; };

        #// Acronym
        (alpha '.')+ alpha { RET; };

        #// Int+float
        digit+            { RET; };
        digit+ '.' digit+ { RET; };

        #// Ignore whitespace and other crap
        0 { return; };
        (any - alnum);

        *|;
}%%

%% write data nofinal;

void frt_scan(const char *buf, char **start, int *len)
{
    int cs, act;
    char *ts, *te = 0;

    %% write init;

    char *p = buf;
    char *pe = 0;
    char *eof = pe;
    *start = 0;
    *len   = 0;

    %% write exec;

    if ( cs == StdTok_error )
    {
        fprintf(stderr, "PARSE ERROR\n" );
        return;
    }

    if ( ts )
    {
        // There is stuff left
        fprintf(stderr, "STUFF LEFT: '%s'\n", ts);
    }
}

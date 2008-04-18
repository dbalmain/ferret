#line 1 "src/scanner.rl"
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

#line 67 "src/scanner.rl"



#line 25 "src/scanner.c"
static const char _StdTok_actions[] = {
	0, 1, 2, 1, 3, 1, 16, 1, 
	17, 1, 18, 1, 19, 1, 20, 1, 
	21, 1, 22, 1, 23, 1, 24, 1, 
	25, 1, 26, 1, 27, 1, 28, 1, 
	29, 1, 30, 2, 4, 5, 2, 4, 
	6, 2, 4, 7, 2, 4, 8, 2, 
	4, 9, 2, 4, 10, 2, 4, 11, 
	2, 4, 12, 2, 4, 13, 2, 4, 
	14, 2, 4, 15, 4, 4, 0, 1, 
	11, 4, 4, 0, 10, 1
};

static const short _StdTok_key_offsets[] = {
	0, 2, 8, 9, 16, 22, 29, 33, 
	37, 38, 46, 57, 59, 62, 64, 75, 
	83, 91, 95, 105, 118, 122, 128, 134, 
	138, 139, 151, 153, 159, 166, 178, 193, 
	206, 219, 231, 239, 247, 260, 274, 287, 
	300
};

static const char _StdTok_trans_keys[] = {
	48, 57, 48, 57, 65, 90, 97, 122, 
	47, 95, 44, 58, 64, 90, 97, 122, 
	48, 57, 65, 90, 97, 122, 46, 48, 
	57, 65, 90, 97, 122, 65, 90, 97, 
	122, 65, 90, 97, 122, 47, 47, 95, 
	44, 58, 64, 90, 97, 122, 0, 43, 
	45, 102, 104, 48, 57, 65, 90, 97, 
	122, 48, 57, 46, 48, 57, 48, 57, 
	45, 46, 58, 64, 95, 48, 57, 65, 
	90, 97, 122, 45, 95, 48, 57, 65, 
	90, 97, 122, 47, 95, 44, 58, 64, 
	90, 97, 122, 65, 90, 97, 122, 45, 
	58, 64, 95, 48, 57, 65, 90, 97, 
	122, 38, 39, 45, 46, 58, 64, 95, 
	48, 57, 65, 90, 97, 122, 65, 90, 
	97, 122, 48, 57, 65, 90, 97, 122, 
	83, 115, 65, 90, 97, 122, 65, 90, 
	97, 122, 46, 38, 39, 45, 58, 64, 
	95, 48, 57, 65, 90, 97, 122, 83, 
	115, 48, 57, 65, 90, 97, 122, 46, 
	48, 57, 65, 90, 97, 122, 38, 39, 
	45, 58, 64, 95, 48, 57, 65, 90, 
	97, 122, 38, 39, 45, 46, 58, 64, 
	95, 105, 116, 48, 57, 65, 90, 97, 
	122, 38, 39, 45, 58, 64, 95, 108, 
	48, 57, 65, 90, 97, 122, 38, 39, 
	45, 58, 64, 95, 101, 48, 57, 65, 
	90, 97, 122, 38, 39, 45, 58, 64, 
	95, 48, 57, 65, 90, 97, 122, 47, 
	95, 44, 58, 64, 90, 97, 122, 47, 
	95, 44, 58, 64, 90, 97, 122, 38, 
	39, 45, 58, 64, 95, 112, 48, 57, 
	65, 90, 97, 122, 38, 39, 45, 46, 
	58, 64, 95, 116, 48, 57, 65, 90, 
	97, 122, 38, 39, 45, 58, 64, 95, 
	116, 48, 57, 65, 90, 97, 122, 38, 
	39, 45, 58, 64, 95, 112, 48, 57, 
	65, 90, 97, 122, 38, 39, 45, 58, 
	64, 95, 115, 48, 57, 65, 90, 97, 
	122, 0
};

static const char _StdTok_single_lengths[] = {
	0, 0, 1, 1, 0, 1, 0, 0, 
	1, 2, 5, 0, 1, 0, 5, 2, 
	2, 0, 4, 7, 0, 0, 2, 0, 
	1, 6, 2, 0, 1, 6, 9, 7, 
	7, 6, 2, 2, 7, 8, 7, 7, 
	7
};

static const char _StdTok_range_lengths[] = {
	1, 3, 0, 3, 3, 3, 2, 2, 
	0, 3, 3, 1, 1, 1, 3, 3, 
	3, 2, 3, 3, 2, 3, 2, 2, 
	0, 3, 0, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3
};

static const unsigned char _StdTok_index_offsets[] = {
	0, 2, 6, 8, 13, 17, 22, 25, 
	28, 30, 36, 45, 47, 50, 52, 61, 
	67, 73, 76, 84, 95, 98, 102, 107, 
	110, 112, 122, 125, 129, 134, 144, 157, 
	168, 179, 189, 195, 201, 212, 224, 235, 
	246
};

static const char _StdTok_indicies[] = {
	1, 0, 2, 2, 2, 0, 3, 0, 
	4, 4, 4, 4, 0, 6, 6, 6, 
	5, 7, 6, 6, 6, 0, 8, 8, 
	0, 9, 9, 0, 11, 10, 13, 12, 
	12, 12, 12, 10, 15, 16, 16, 19, 
	20, 17, 18, 18, 14, 22, 21, 24, 
	22, 23, 1, 25, 27, 24, 28, 29, 
	27, 17, 30, 30, 26, 27, 27, 2, 
	2, 2, 26, 31, 4, 4, 4, 4, 
	0, 8, 8, 32, 27, 28, 29, 27, 
	30, 30, 30, 26, 34, 35, 27, 36, 
	28, 38, 27, 37, 39, 39, 33, 41, 
	41, 40, 41, 41, 41, 40, 44, 44, 
	43, 43, 42, 43, 43, 0, 36, 45, 
	34, 46, 27, 28, 38, 27, 37, 37, 
	37, 33, 47, 47, 42, 6, 48, 48, 
	40, 7, 48, 48, 48, 40, 34, 35, 
	27, 28, 38, 27, 37, 39, 39, 33, 
	34, 35, 27, 36, 28, 38, 27, 49, 
	50, 37, 39, 39, 33, 34, 35, 27, 
	28, 38, 27, 51, 37, 39, 39, 33, 
	34, 35, 27, 28, 38, 27, 52, 37, 
	39, 39, 33, 34, 35, 27, 53, 38, 
	27, 37, 39, 39, 33, 55, 54, 54, 
	54, 54, 0, 56, 12, 12, 12, 12, 
	0, 34, 35, 27, 28, 38, 27, 52, 
	37, 39, 39, 33, 34, 35, 27, 36, 
	28, 38, 27, 57, 37, 39, 39, 33, 
	34, 35, 27, 28, 38, 27, 58, 37, 
	39, 39, 33, 34, 35, 27, 28, 38, 
	27, 59, 37, 39, 39, 33, 34, 35, 
	27, 53, 38, 27, 52, 37, 39, 39, 
	33, 0
};

static const char _StdTok_trans_targs_wi[] = {
	10, 13, 15, 3, 16, 10, 5, 6, 
	17, 24, 10, 9, 34, 35, 10, 10, 
	11, 14, 19, 30, 37, 10, 12, 10, 
	0, 10, 10, 1, 2, 4, 18, 16, 
	10, 10, 20, 22, 7, 25, 27, 29, 
	10, 21, 10, 23, 23, 10, 26, 10, 
	28, 31, 36, 32, 33, 8, 34, 34, 
	35, 38, 39, 40
};

static const char _StdTok_trans_actions_wi[] = {
	33, 0, 44, 0, 59, 31, 0, 0, 
	0, 62, 29, 0, 68, 68, 9, 7, 
	0, 44, 35, 35, 35, 27, 65, 23, 
	0, 25, 15, 0, 0, 0, 44, 56, 
	19, 11, 0, 0, 0, 35, 47, 35, 
	17, 0, 13, 41, 38, 21, 0, 5, 
	47, 35, 35, 35, 35, 0, 53, 50, 
	73, 35, 35, 35
};

static const char _StdTok_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0
};

static const char _StdTok_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 3, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0
};

static const char _StdTok_eof_trans[] = {
	1, 1, 1, 1, 6, 1, 1, 1, 
	11, 11, 0, 22, 24, 26, 27, 27, 
	1, 33, 27, 34, 41, 41, 43, 1, 
	46, 34, 43, 41, 41, 34, 34, 34, 
	34, 34, 1, 1, 34, 34, 34, 34, 
	34
};

static const int StdTok_start = 10;
static const int StdTok_error = -1;

static const int StdTok_en_main = 10;

#line 70 "src/scanner.rl"

void frt_std_scan(const char *in,
                  char *out, size_t out_size,
                  const char **start,
                  const char **end,
                  int *token_size)
{
    int cs, act;
    char *ts = 0, *te = 0;

    
#line 220 "src/scanner.c"
	{
	cs = StdTok_start;
	ts = 0;
	te = 0;
	act = 0;
	}
#line 81 "src/scanner.rl"

    char *p = (char *)in, *pe = 0, *eof = pe;
    int skip = 0;
    int trunc = 0;
    char strip_char = 0;

    *end = 0;
    *start = 0;
    *token_size = 0;

    
#line 239 "src/scanner.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
_resume:
	_acts = _StdTok_actions + _StdTok_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 3:
#line 1 "src/scanner.rl"
	{ts = p;}
	break;
#line 258 "src/scanner.c"
		}
	}

	_keys = _StdTok_trans_keys + _StdTok_key_offsets[cs];
	_trans = _StdTok_index_offsets[cs];

	_klen = _StdTok_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _StdTok_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _StdTok_indicies[_trans];
_eof_trans:
	cs = _StdTok_trans_targs_wi[_trans];

	if ( _StdTok_trans_actions_wi[_trans] == 0 )
		goto _again;

	_acts = _StdTok_actions + _StdTok_trans_actions_wi[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 47 "src/scanner.rl"
	{ skip = p - ts; }
	break;
	case 1:
#line 48 "src/scanner.rl"
	{ skip = p - ts; }
	break;
	case 4:
#line 1 "src/scanner.rl"
	{te = p+1;}
	break;
	case 5:
#line 33 "src/scanner.rl"
	{act = 1;}
	break;
	case 6:
#line 35 "src/scanner.rl"
	{act = 3;}
	break;
	case 7:
#line 38 "src/scanner.rl"
	{act = 4;}
	break;
	case 8:
#line 41 "src/scanner.rl"
	{act = 5;}
	break;
	case 9:
#line 44 "src/scanner.rl"
	{act = 6;}
	break;
	case 10:
#line 47 "src/scanner.rl"
	{act = 7;}
	break;
	case 11:
#line 48 "src/scanner.rl"
	{act = 8;}
	break;
	case 12:
#line 49 "src/scanner.rl"
	{act = 9;}
	break;
	case 13:
#line 50 "src/scanner.rl"
	{act = 10;}
	break;
	case 14:
#line 56 "src/scanner.rl"
	{act = 12;}
	break;
	case 15:
#line 59 "src/scanner.rl"
	{act = 13;}
	break;
	case 16:
#line 35 "src/scanner.rl"
	{te = p+1;{ trunc = 2; RET; }}
	break;
	case 17:
#line 63 "src/scanner.rl"
	{te = p+1;{ return; }}
	break;
	case 18:
#line 64 "src/scanner.rl"
	{te = p+1;}
	break;
	case 19:
#line 33 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 20:
#line 34 "src/scanner.rl"
	{te = p;p--;{ trunc = 1; RET; }}
	break;
	case 21:
#line 41 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 22:
#line 44 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 23:
#line 53 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 24:
#line 56 "src/scanner.rl"
	{te = p;p--;{ STRIP('.'); }}
	break;
	case 25:
#line 59 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 26:
#line 60 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 27:
#line 64 "src/scanner.rl"
	{te = p;p--;}
	break;
	case 28:
#line 33 "src/scanner.rl"
	{{p = ((te))-1;}{ RET; }}
	break;
	case 29:
#line 41 "src/scanner.rl"
	{{p = ((te))-1;}{ RET; }}
	break;
	case 30:
#line 1 "src/scanner.rl"
	{	switch( act ) {
	case 1:
	{{p = ((te))-1;} RET; }
	break;
	case 3:
	{{p = ((te))-1;} trunc = 2; RET; }
	break;
	case 4:
	{{p = ((te))-1;} RET; }
	break;
	case 5:
	{{p = ((te))-1;} RET; }
	break;
	case 6:
	{{p = ((te))-1;} RET; }
	break;
	case 7:
	{{p = ((te))-1;} trunc = 1; RET; }
	break;
	case 8:
	{{p = ((te))-1;} RET; }
	break;
	case 9:
	{{p = ((te))-1;} trunc = 1; RET; }
	break;
	case 10:
	{{p = ((te))-1;} RET; }
	break;
	case 12:
	{{p = ((te))-1;} STRIP('.'); }
	break;
	case 13:
	{{p = ((te))-1;} RET; }
	break;
	default: break;
	}
	}
	break;
#line 476 "src/scanner.c"
		}
	}

_again:
	_acts = _StdTok_actions + _StdTok_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 2:
#line 1 "src/scanner.rl"
	{ts = 0;}
	break;
#line 489 "src/scanner.c"
		}
	}

	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _StdTok_eof_trans[cs] > 0 ) {
		_trans = _StdTok_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	}
#line 92 "src/scanner.rl"

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

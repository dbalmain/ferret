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

#line 64 "src/scanner.rl"



#line 25 "src/scanner.c"
static const char _StdTok_actions[] = {
	0, 1, 2, 1, 3, 1, 14, 1, 
	15, 1, 16, 1, 17, 1, 18, 1, 
	19, 1, 20, 1, 21, 1, 22, 1, 
	23, 1, 24, 1, 25, 1, 26, 1, 
	27, 1, 28, 2, 4, 5, 2, 4, 
	6, 2, 4, 7, 2, 4, 8, 2, 
	4, 9, 2, 4, 10, 2, 4, 11, 
	2, 4, 12, 2, 4, 13, 4, 4, 
	0, 1, 9, 4, 4, 0, 8, 1
	
};

static const short _StdTok_key_offsets[] = {
	0, 2, 8, 9, 16, 22, 29, 33, 
	37, 38, 46, 57, 59, 62, 64, 75, 
	83, 91, 95, 105, 118, 122, 128, 130, 
	131, 143, 149, 156, 171, 184, 197, 209, 
	217, 225, 238, 252, 265, 278
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
	83, 115, 46, 38, 39, 45, 58, 64, 
	95, 48, 57, 65, 90, 97, 122, 48, 
	57, 65, 90, 97, 122, 46, 48, 57, 
	65, 90, 97, 122, 38, 39, 45, 46, 
	58, 64, 95, 105, 116, 48, 57, 65, 
	90, 97, 122, 38, 39, 45, 58, 64, 
	95, 108, 48, 57, 65, 90, 97, 122, 
	38, 39, 45, 58, 64, 95, 101, 48, 
	57, 65, 90, 97, 122, 38, 39, 45, 
	58, 64, 95, 48, 57, 65, 90, 97, 
	122, 47, 95, 44, 58, 64, 90, 97, 
	122, 47, 95, 44, 58, 64, 90, 97, 
	122, 38, 39, 45, 58, 64, 95, 112, 
	48, 57, 65, 90, 97, 122, 38, 39, 
	45, 46, 58, 64, 95, 116, 48, 57, 
	65, 90, 97, 122, 38, 39, 45, 58, 
	64, 95, 116, 48, 57, 65, 90, 97, 
	122, 38, 39, 45, 58, 64, 95, 112, 
	48, 57, 65, 90, 97, 122, 38, 39, 
	45, 58, 64, 95, 115, 48, 57, 65, 
	90, 97, 122, 0
};

static const char _StdTok_single_lengths[] = {
	0, 0, 1, 1, 0, 1, 0, 0, 
	1, 2, 5, 0, 1, 0, 5, 2, 
	2, 0, 4, 7, 0, 0, 2, 1, 
	6, 0, 1, 9, 7, 7, 6, 2, 
	2, 7, 8, 7, 7, 7
};

static const char _StdTok_range_lengths[] = {
	1, 3, 0, 3, 3, 3, 2, 2, 
	0, 3, 3, 1, 1, 1, 3, 3, 
	3, 2, 3, 3, 2, 3, 0, 0, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3
};

static const unsigned char _StdTok_index_offsets[] = {
	0, 2, 6, 8, 13, 17, 22, 25, 
	28, 30, 36, 45, 47, 50, 52, 61, 
	67, 73, 76, 84, 95, 98, 102, 105, 
	107, 117, 121, 126, 139, 150, 161, 171, 
	177, 183, 194, 206, 217, 228
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
	28, 38, 27, 37, 37, 37, 33, 40, 
	40, 39, 40, 40, 40, 39, 42, 42, 
	41, 36, 43, 34, 35, 27, 28, 38, 
	27, 37, 37, 37, 33, 6, 44, 44, 
	39, 7, 44, 44, 44, 39, 34, 35, 
	27, 36, 28, 38, 27, 45, 46, 37, 
	37, 37, 33, 34, 35, 27, 28, 38, 
	27, 47, 37, 37, 37, 33, 34, 35, 
	27, 28, 38, 27, 48, 37, 37, 37, 
	33, 34, 35, 27, 49, 38, 27, 37, 
	37, 37, 33, 51, 50, 50, 50, 50, 
	0, 52, 12, 12, 12, 12, 0, 34, 
	35, 27, 28, 38, 27, 48, 37, 37, 
	37, 33, 34, 35, 27, 36, 28, 38, 
	27, 53, 37, 37, 37, 33, 34, 35, 
	27, 28, 38, 27, 54, 37, 37, 37, 
	33, 34, 35, 27, 28, 38, 27, 55, 
	37, 37, 37, 33, 34, 35, 27, 49, 
	38, 27, 48, 37, 37, 37, 33, 0
};

static const char _StdTok_trans_targs_wi[] = {
	10, 13, 15, 3, 16, 10, 5, 6, 
	17, 23, 10, 9, 31, 32, 10, 10, 
	11, 14, 19, 27, 34, 10, 12, 10, 
	0, 10, 10, 1, 2, 4, 18, 16, 
	10, 10, 20, 22, 7, 24, 25, 10, 
	21, 10, 10, 10, 26, 28, 33, 29, 
	30, 8, 31, 31, 32, 35, 36, 37
};

static const char _StdTok_trans_actions_wi[] = {
	33, 0, 38, 0, 53, 31, 0, 0, 
	0, 56, 29, 0, 62, 62, 9, 7, 
	0, 38, 35, 35, 35, 27, 59, 23, 
	0, 25, 15, 0, 0, 0, 38, 50, 
	19, 11, 0, 0, 0, 35, 41, 17, 
	0, 13, 5, 21, 41, 35, 35, 35, 
	35, 0, 47, 44, 67, 35, 35, 35
};

static const char _StdTok_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const char _StdTok_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 3, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const char _StdTok_eof_trans[] = {
	1, 1, 1, 1, 6, 1, 1, 1, 
	11, 11, 0, 22, 24, 26, 27, 27, 
	1, 33, 27, 34, 40, 40, 42, 44, 
	34, 40, 40, 34, 34, 34, 34, 1, 
	1, 34, 34, 34, 34, 34
};

static const int StdTok_start = 10;
static const int StdTok_error = -1;

static const int StdTok_en_main = 10;

#line 67 "src/scanner.rl"

void frt_std_scan(const char *in,
                  char *out, size_t out_size,
                  const char **start,
                  const char **end,
                  int *token_size)
{
    int cs, act;
    char *ts = 0, *te = 0;

    
#line 205 "src/scanner.c"
	{
	cs = StdTok_start;
	ts = 0;
	te = 0;
	act = 0;
	}
#line 78 "src/scanner.rl"

    char *p = (char *)in, *pe = 0, *eof = pe;
    int skip = 0;
    int trunc = 0;
    char strip_char = 0;

    *end = 0;
    *start = 0;
    *token_size = 0;

    
#line 224 "src/scanner.c"
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
#line 243 "src/scanner.c"
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
#line 44 "src/scanner.rl"
	{ skip = p - ts; }
	break;
	case 1:
#line 45 "src/scanner.rl"
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
#line 38 "src/scanner.rl"
	{act = 4;}
	break;
	case 7:
#line 41 "src/scanner.rl"
	{act = 5;}
	break;
	case 8:
#line 44 "src/scanner.rl"
	{act = 6;}
	break;
	case 9:
#line 45 "src/scanner.rl"
	{act = 7;}
	break;
	case 10:
#line 46 "src/scanner.rl"
	{act = 8;}
	break;
	case 11:
#line 47 "src/scanner.rl"
	{act = 9;}
	break;
	case 12:
#line 53 "src/scanner.rl"
	{act = 11;}
	break;
	case 13:
#line 56 "src/scanner.rl"
	{act = 12;}
	break;
	case 14:
#line 35 "src/scanner.rl"
	{te = p+1;{ trunc = 2; RET; }}
	break;
	case 15:
#line 60 "src/scanner.rl"
	{te = p+1;{ return; }}
	break;
	case 16:
#line 61 "src/scanner.rl"
	{te = p+1;}
	break;
	case 17:
#line 33 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 18:
#line 34 "src/scanner.rl"
	{te = p;p--;{ trunc = 1; RET; }}
	break;
	case 19:
#line 38 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 20:
#line 41 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 21:
#line 50 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 22:
#line 53 "src/scanner.rl"
	{te = p;p--;{ STRIP('.'); }}
	break;
	case 23:
#line 56 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 24:
#line 57 "src/scanner.rl"
	{te = p;p--;{ RET; }}
	break;
	case 25:
#line 61 "src/scanner.rl"
	{te = p;p--;}
	break;
	case 26:
#line 33 "src/scanner.rl"
	{{p = ((te))-1;}{ RET; }}
	break;
	case 27:
#line 38 "src/scanner.rl"
	{{p = ((te))-1;}{ RET; }}
	break;
	case 28:
#line 1 "src/scanner.rl"
	{	switch( act ) {
	case 1:
	{{p = ((te))-1;} RET; }
	break;
	case 4:
	{{p = ((te))-1;} RET; }
	break;
	case 5:
	{{p = ((te))-1;} RET; }
	break;
	case 6:
	{{p = ((te))-1;} trunc = 1; RET; }
	break;
	case 7:
	{{p = ((te))-1;} RET; }
	break;
	case 8:
	{{p = ((te))-1;} trunc = 1; RET; }
	break;
	case 9:
	{{p = ((te))-1;} RET; }
	break;
	case 11:
	{{p = ((te))-1;} STRIP('.'); }
	break;
	case 12:
	{{p = ((te))-1;} RET; }
	break;
	default: break;
	}
	}
	break;
#line 447 "src/scanner.c"
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
#line 460 "src/scanner.c"
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
#line 89 "src/scanner.rl"

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

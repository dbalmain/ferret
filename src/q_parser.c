/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     WORD = 258,
     WILD_STR = 259,
     LOW = 260,
     OR = 261,
     AND = 262,
     NOT = 263,
     REQ = 264,
     HIGH = 265
   };
#endif
/* Tokens.  */
#define WORD 258
#define WILD_STR 259
#define LOW 260
#define OR 261
#define AND 262
#define NOT 263
#define REQ 264
#define HIGH 265




/* Copy the first part of user declarations.  */
#line 1 "src/q_parser.y"

#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include "search.h"
#include "array.h"

typedef struct Phrase {
    int             size;
    int             capa;
    int             pos_inc;
    PhrasePosition *positions;
} Phrase;

#define BCA_INIT_CAPA 4
typedef struct BCArray {
    int size;
    int capa;
    BooleanClause **clauses;
} BCArray;



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 23 "src/q_parser.y"
typedef union YYSTYPE {
    Query *query;
    BooleanClause *bcls;
    BCArray *bclss;
    HashSet *hashset;
    Phrase *phrase;
    char *str;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 137 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 31 "src/q_parser.y"

static int yylex(YYSTYPE *lvalp, QParser *qp);
static int yyerror(QParser *qp, char const *msg);

#define PHRASE_INIT_CAPA 4
static Query *get_bool_q(BCArray *bca);

static BCArray *first_cls(BooleanClause *boolean_clause);
static BCArray *add_and_cls(BCArray *bca, BooleanClause *clause);
static BCArray *add_or_cls(BCArray *bca, BooleanClause *clause);
static BCArray *add_default_cls(QParser *qp, BCArray *bca, BooleanClause *clause);

static BooleanClause *get_bool_cls(Query *q, unsigned int occur);

static Query *get_term_q(QParser *qp, char *field, char *word);
static Query *get_fuzzy_q(QParser *qp, char *field, char *word, char *slop);
static Query *get_wild_q(QParser *qp, char *field, char *pattern);

static HashSet *first_field(QParser *qp, char *field);
static HashSet *add_field(QParser *qp, char *field);

static Query *get_phrase_q(QParser *qp, Phrase *phrase, char *slop);

static Phrase *ph_first_word(char *word);
static Phrase *ph_add_word(Phrase *self, char *word);
static Phrase *ph_add_multi_word(Phrase *self, char *word);

static Query *get_range_q(const char *field, const char *from, const char *to,
                          bool inc_lower, bool inc_upper);

#define FLDS(q, func) do {\
    char *field;\
    if (qp->fields->size == 0) {\
        q = NULL;\
    } else if (qp->fields->size == 1) {\
        field = (char *)qp->fields->elems[0];\
        q = func;\
    } else {\
        int i;Query *sq;\
        q = bq_new(false);\
        for (i = 0; i < qp->fields->size; i++) {\
            field = (char *)qp->fields->elems[i];\
            sq = func;\
            if (sq) bq_add_query_nr(q, sq, BC_SHOULD);\
        }\
    }\
} while (0)


/* Line 219 of yacc.c.  */
#line 197 "y.tab.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  38
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   98

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  26
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  16
/* YYNRULES -- Number of rules. */
#define YYNRULES  50
/* YYNRULES -- Number of states. */
#define YYNSTATES  79

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   265

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    18,     2,     2,     2,     2,     2,
      13,    14,    16,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    10,     2,
      19,    25,    20,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    21,     2,    22,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    24,    17,    23,    15,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    11
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    12,    16,    19,    22,
      25,    27,    29,    33,    35,    39,    41,    43,    45,    47,
      49,    53,    56,    58,    59,    64,    65,    66,    72,    74,
      78,    82,    88,    91,    96,    98,   101,   104,   108,   112,
     117,   122,   127,   132,   136,   140,   144,   148,   151,   155,
     159
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      27,     0,    -1,    -1,    28,    -1,    29,    -1,    28,     7,
      29,    -1,    28,     6,    29,    -1,    28,    29,    -1,     9,
      30,    -1,     8,    30,    -1,    30,    -1,    31,    -1,    31,
      12,     3,    -1,    32,    -1,    13,    28,    14,    -1,    34,
      -1,    39,    -1,    41,    -1,    33,    -1,     3,    -1,     3,
      15,     3,    -1,     3,    15,    -1,     4,    -1,    -1,    38,
      10,    31,    35,    -1,    -1,    -1,    16,    36,    10,    31,
      37,    -1,     3,    -1,    38,    17,     3,    -1,    18,    40,
      18,    -1,    18,    40,    18,    15,     3,    -1,    18,    18,
      -1,    18,    18,    15,     3,    -1,     3,    -1,    19,    20,
      -1,    40,     3,    -1,    40,    19,    20,    -1,    40,    17,
       3,    -1,    21,     3,     3,    22,    -1,    21,     3,     3,
      23,    -1,    24,     3,     3,    22,    -1,    24,     3,     3,
      23,    -1,    19,     3,    23,    -1,    19,     3,    22,    -1,
      21,     3,    20,    -1,    24,     3,    20,    -1,    19,     3,
      -1,    19,    25,     3,    -1,    20,    25,     3,    -1,    20,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    95,    95,    96,    98,    99,   100,   101,   103,   104,
     105,   107,   108,   110,   111,   112,   113,   114,   115,   117,
     118,   119,   121,   123,   123,   125,   125,   125,   128,   129,
     131,   132,   133,   134,   136,   137,   138,   139,   140,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "WORD", "WILD_STR", "LOW", "OR", "AND",
  "NOT", "REQ", "':'", "HIGH", "'^'", "'('", "')'", "'~'", "'*'", "'|'",
  "'\"'", "'<'", "'>'", "'['", "']'", "'}'", "'{'", "'='", "$accept",
  "bool_q", "bool_clss", "bool_cls", "boosted_q", "q", "term_q", "wild_q",
  "field_q", "@1", "@2", "@3", "field", "phrase_q", "ph_words", "range_q", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
      58,   265,    94,    40,    41,   126,    42,   124,    34,    60,
      62,    91,    93,   125,   123,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    26,    27,    27,    28,    28,    28,    28,    29,    29,
      29,    30,    30,    31,    31,    31,    31,    31,    31,    32,
      32,    32,    33,    35,    34,    36,    37,    34,    38,    38,
      39,    39,    39,    39,    40,    40,    40,    40,    40,    41,
      41,    41,    41,    41,    41,    41,    41,    41,    41,    41,
      41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     1,     1,     3,     3,     2,     2,     2,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       3,     2,     1,     0,     4,     0,     0,     5,     1,     3,
       3,     5,     2,     4,     1,     2,     2,     3,     3,     4,
       4,     4,     4,     3,     3,     3,     3,     2,     3,     3,
       2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,    19,    22,     0,     0,     0,    25,     0,     0,     0,
       0,     0,     0,     3,     4,    10,    11,    13,    18,    15,
       0,    16,    17,    21,     9,     8,     0,     0,    34,    32,
       0,     0,    47,     0,    50,     0,     0,     0,     1,     0,
       0,     7,     0,     0,     0,    20,    14,     0,     0,    35,
      36,     0,    30,     0,    44,    43,    48,    49,     0,    45,
       0,    46,     6,     5,    12,    23,    29,    26,    33,    38,
       0,    37,    39,    40,    41,    42,    24,    27,    31
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    76,
      27,    77,    20,    21,    31,    22
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -32
static const yysigned_char yypact[] =
{
      44,    75,   -32,    63,    63,    44,   -32,    55,    -2,    -1,
       0,     3,    11,    25,   -32,   -32,    18,   -32,   -32,   -32,
      76,   -32,   -32,    33,   -32,   -32,     1,    32,   -32,    41,
      39,    52,    17,    58,   -32,    69,    15,    34,   -32,    44,
      44,   -32,    72,    63,    77,   -32,   -32,    63,    88,   -32,
     -32,    91,    80,    78,   -32,   -32,   -32,   -32,    28,   -32,
      66,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
      93,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -32,   -32,    92,   -13,    74,   -31,   -32,   -32,   -32,   -32,
     -32,   -32,   -32,   -32,   -32,   -32
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -29
static const yysigned_char yytable[] =
{
      41,    32,    34,    36,     1,     2,    37,    39,    40,     3,
       4,    38,    65,    41,     5,    46,    67,     6,    58,     7,
       8,     9,    10,    33,    35,    11,    62,    63,     1,     2,
      42,    39,    40,     3,     4,    59,    45,    60,     5,    54,
      55,     6,    47,     7,     8,     9,    10,     1,     2,    11,
      72,    73,     3,     4,    61,    50,    48,     5,    28,    49,
       6,    56,     7,     8,     9,    10,     1,     2,    11,    51,
      52,    53,    57,    29,    30,    64,     5,    24,    25,     6,
      66,     7,     8,     9,    10,   -28,    43,    11,    74,    75,
      23,    68,   -28,    44,    69,    70,    78,    26,    71
};

static const unsigned char yycheck[] =
{
      13,     3,     3,     3,     3,     4,     3,     6,     7,     8,
       9,     0,    43,    26,    13,    14,    47,    16,     3,    18,
      19,    20,    21,    25,    25,    24,    39,    40,     3,     4,
      12,     6,     7,     8,     9,    20,     3,     3,    13,    22,
      23,    16,    10,    18,    19,    20,    21,     3,     4,    24,
      22,    23,     8,     9,    20,     3,    15,    13,     3,    20,
      16,     3,    18,    19,    20,    21,     3,     4,    24,    17,
      18,    19,     3,    18,    19,     3,    13,     3,     4,    16,
       3,    18,    19,    20,    21,    10,    10,    24,    22,    23,
      15,     3,    17,    17,     3,    15,     3,     5,    20
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     8,     9,    13,    16,    18,    19,    20,
      21,    24,    27,    28,    29,    30,    31,    32,    33,    34,
      38,    39,    41,    15,    30,    30,    28,    36,     3,    18,
      19,    40,     3,    25,     3,    25,     3,     3,     0,     6,
       7,    29,    12,    10,    17,     3,    14,    10,    15,    20,
       3,    17,    18,    19,    22,    23,     3,     3,     3,    20,
       3,    20,    29,    29,     3,    31,     3,    31,     3,     3,
      15,    20,    22,    23,    22,    23,    35,    37,     3
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (qp, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, qp)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (QParser *qp);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (QParser *qp)
#else
int
yyparse (qp)
    QParser *qp;
#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 95 "src/q_parser.y"
    { qp->result = (yyval.query) = NULL; }
    break;

  case 3:
#line 96 "src/q_parser.y"
    { qp->result = (yyval.query) = get_bool_q((yyvsp[0].bclss)); }
    break;

  case 4:
#line 98 "src/q_parser.y"
    { (yyval.bclss) = first_cls((yyvsp[0].bcls)); }
    break;

  case 5:
#line 99 "src/q_parser.y"
    { (yyval.bclss) = add_and_cls((yyvsp[-2].bclss), (yyvsp[0].bcls)); }
    break;

  case 6:
#line 100 "src/q_parser.y"
    { (yyval.bclss) = add_or_cls((yyvsp[-2].bclss), (yyvsp[0].bcls)); }
    break;

  case 7:
#line 101 "src/q_parser.y"
    { (yyval.bclss) = add_default_cls(qp, (yyvsp[-1].bclss), (yyvsp[0].bcls)); }
    break;

  case 8:
#line 103 "src/q_parser.y"
    { (yyval.bcls) = get_bool_cls((yyvsp[0].query), BC_MUST); }
    break;

  case 9:
#line 104 "src/q_parser.y"
    { (yyval.bcls) = get_bool_cls((yyvsp[0].query), BC_MUST_NOT); }
    break;

  case 10:
#line 105 "src/q_parser.y"
    { (yyval.bcls) = get_bool_cls((yyvsp[0].query), BC_SHOULD); }
    break;

  case 12:
#line 108 "src/q_parser.y"
    { if ((yyvsp[-2].query)) sscanf((yyvsp[0].str),"%f",&((yyvsp[-2].query)->boost)); (yyval.query)=(yyvsp[-2].query); }
    break;

  case 14:
#line 111 "src/q_parser.y"
    { (yyval.query) = get_bool_q((yyvsp[-1].bclss)); }
    break;

  case 19:
#line 117 "src/q_parser.y"
    { FLDS((yyval.query), get_term_q(qp, field, (yyvsp[0].str))); }
    break;

  case 20:
#line 118 "src/q_parser.y"
    { FLDS((yyval.query), get_fuzzy_q(qp, field, (yyvsp[-2].str), (yyvsp[0].str))); }
    break;

  case 21:
#line 119 "src/q_parser.y"
    { FLDS((yyval.query), get_fuzzy_q(qp, field, (yyvsp[-1].str), NULL)); }
    break;

  case 22:
#line 121 "src/q_parser.y"
    { FLDS((yyval.query), get_wild_q(qp, field, (yyvsp[0].str))); }
    break;

  case 23:
#line 123 "src/q_parser.y"
    { qp->fields = qp->def_fields; }
    break;

  case 24:
#line 124 "src/q_parser.y"
    { (yyval.query) = (yyvsp[-1].query); }
    break;

  case 25:
#line 125 "src/q_parser.y"
    { qp->fields = qp->all_fields; }
    break;

  case 26:
#line 125 "src/q_parser.y"
    {qp->fields = qp->def_fields;}
    break;

  case 27:
#line 126 "src/q_parser.y"
    { (yyval.query) = (yyvsp[-1].query); }
    break;

  case 28:
#line 128 "src/q_parser.y"
    { (yyval.hashset) = first_field(qp, (yyvsp[0].str)); }
    break;

  case 29:
#line 129 "src/q_parser.y"
    { (yyval.hashset) = add_field(qp, (yyvsp[0].str));}
    break;

  case 30:
#line 131 "src/q_parser.y"
    { (yyval.query) = get_phrase_q(qp, (yyvsp[-1].phrase), NULL); }
    break;

  case 31:
#line 132 "src/q_parser.y"
    { (yyval.query) = get_phrase_q(qp, (yyvsp[-3].phrase), (yyvsp[0].str)); }
    break;

  case 32:
#line 133 "src/q_parser.y"
    { (yyval.query) = NULL; }
    break;

  case 33:
#line 134 "src/q_parser.y"
    { (yyval.query) = NULL; }
    break;

  case 34:
#line 136 "src/q_parser.y"
    { (yyval.phrase) = ph_first_word((yyvsp[0].str)); }
    break;

  case 35:
#line 137 "src/q_parser.y"
    { (yyval.phrase) = ph_first_word(NULL); }
    break;

  case 36:
#line 138 "src/q_parser.y"
    { (yyval.phrase) = ph_add_word((yyvsp[-1].phrase), (yyvsp[0].str)); }
    break;

  case 37:
#line 139 "src/q_parser.y"
    { (yyval.phrase) = ph_add_word((yyvsp[-2].phrase), NULL); }
    break;

  case 38:
#line 140 "src/q_parser.y"
    { (yyval.phrase) = ph_add_multi_word((yyvsp[-2].phrase), (yyvsp[0].str));  }
    break;

  case 39:
#line 142 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[-2].str),  (yyvsp[-1].str),  true,  true)); }
    break;

  case 40:
#line 143 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[-2].str),  (yyvsp[-1].str),  true,  false)); }
    break;

  case 41:
#line 144 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[-2].str),  (yyvsp[-1].str),  false, true)); }
    break;

  case 42:
#line 145 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[-2].str),  (yyvsp[-1].str),  false, false)); }
    break;

  case 43:
#line 146 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, NULL,(yyvsp[-1].str),  false, false)); }
    break;

  case 44:
#line 147 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, NULL,(yyvsp[-1].str),  false, true)); }
    break;

  case 45:
#line 148 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[-1].str),  NULL,true,  false)); }
    break;

  case 46:
#line 149 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[-1].str),  NULL,false, false)); }
    break;

  case 47:
#line 150 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, NULL,(yyvsp[0].str),  false, false)); }
    break;

  case 48:
#line 151 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, NULL,(yyvsp[0].str),  false, true)); }
    break;

  case 49:
#line 152 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[0].str),  NULL,true,  false)); }
    break;

  case 50:
#line 153 "src/q_parser.y"
    { FLDS((yyval.query), get_range_q(field, (yyvsp[0].str),  NULL,false, false)); }
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 1468 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (qp, yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (qp, YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (qp, YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (qp, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 155 "src/q_parser.y"


const char *special_char = "&:()[]{}!+\"~^-|<>=*?";
const char *not_word =  " \t&:()[]{}!+\"~^-|<>=";

static int get_word(YYSTYPE *lvalp, QParser *qp)
{
    bool is_wild = false;
    int len;
    char c;
    char *buf = qp->buf[qp->buf_index];
    char *bufp = buf;
    qp->buf_index = (qp->buf_index + 1) % QP_CONC_WORDS;

    qp->qstrp--; /* need to back up one character */
    
    while (!strchr(not_word, (c=*qp->qstrp++))) {
        switch (c) {
            case '\\':
                if ((c=*qp->qstrp) == ' ' && c != '\t' && c != '\0') {
                    *bufp++ = '\\';
                }
                else {
                    *bufp++ = c;
                    qp->qstrp++;
                }
                break;
            case '*': case '?':
                is_wild = true;
                /* fall through */
            default:
                *bufp++ = c;
        }
    }
    qp->qstrp--;
    /* check for keywords. There are only four so we have a bit of a hack which
     * just checks for all of them. */
    *bufp = '\0';
    len = (int)(bufp - buf);
    if (len == 3) {
        if (buf[0] == 'A' && buf[1] == 'N' && buf[2] == 'D') return AND;
        if (buf[0] == 'N' && buf[1] == 'O' && buf[2] == 'T') return NOT;
        if (buf[0] == 'R' && buf[1] == 'E' && buf[2] == 'Q') return REQ;
    }
    if (len == 2 && buf[0] == 'O' && buf[1] == 'R') return OR;

    /* found a word so return it. */
    lvalp->str = buf;
    if (is_wild) return WILD_STR;
    return WORD;
}

static int yylex(YYSTYPE *lvalp, QParser *qp)
{
    char c, nc;

    while ((c=*qp->qstrp++) == ' ' || c == '\t') {
    }

    if (c == '\0') return 0;

    if (strchr(special_char, c)) {   /* comment */
        nc = *qp->qstrp;
        switch (c) {
            case '-': case '!': return NOT;
            case '+': return REQ;
            case '*':
                if (nc == ':') return c;
                break;
            case '&':
                if (nc == '&') {
                    qp->qstrp++;
                    return AND;
                }
                break; /* Don't return single & character. Use in word. */
            case '|':
                if (nc == '|') {
                    qp->qstrp++;
                    return OR;
                }
            default:
                return c;
        }
    }

    return get_word(lvalp, qp);
}

static int yyerror(QParser *qp, char const *msg)
{
    if (!qp->handle_parse_errors) {
        if (qp->clean_str) {
            free(qp->qstr);
        }
        mutex_unlock(&qp->mutex);
        RAISE(PARSE_ERROR, "couldn't parse query ``%s''. Error message "
              " was %s.", qp->qstr, (char *)msg);
    }
    return 0;
}

#define BQ(query) ((BooleanQuery *)(query))

static TokenStream *get_cached_ts(QParser *qp, char *field, char *text)
{
    TokenStream *ts = h_get(qp->ts_cache, field);
    if (!ts) {
        ts = a_get_ts(qp->analyzer, field, text);
        h_set(qp->ts_cache, field, ts);
    }
    else {
        ts->reset(ts, text);
    }
    return ts;
}

static char *get_cached_field(HashTable *field_cache, const char *field)
{
    char *cached_field = h_get(field_cache, field);
    if (!cached_field) {
        cached_field = estrdup(field);
        h_set(field_cache, cached_field, cached_field);
    }
    return cached_field;
}

static Query *get_bool_q(BCArray *bca)
{
    Query *q;
    const int clause_count = bca->size;

    if (clause_count == 0) {
        q = NULL;
        free(bca->clauses);
    }
    else if (clause_count == 1) {
        BooleanClause *bc = bca->clauses[0];
        q = bc->query;
        free(bc);
        free(bca->clauses);
    }
    else {
        q = bq_new(false);
        /* copy clauses into query */

        BQ(q)->clause_cnt = clause_count;
        BQ(q)->clause_capa = bca->capa;
        free(BQ(q)->clauses);
        BQ(q)->clauses = bca->clauses;
    }
    free(bca);
    return q;
}

static void bca_add_clause(BCArray *bca, BooleanClause *clause)
{
    if (bca->size >= bca->capa) {
        bca->capa <<= 1;
        REALLOC_N(bca->clauses, BooleanClause *, bca->capa);
    }
    bca->clauses[bca->size] = clause;
    bca->size++;
}

static BCArray *first_cls(BooleanClause *clause)
{
    BCArray *bca = ALLOC_AND_ZERO(BCArray);
    bca->capa = BCA_INIT_CAPA;
    bca->clauses = ALLOC_N(BooleanClause *, BCA_INIT_CAPA);
    if (clause) {
        bca_add_clause(bca, clause);
    }
    return bca;
}

static BCArray *add_and_cls(BCArray *bca, BooleanClause *clause)
{
    if (clause) {
        if (bca->size == 1) {
            if (!bca->clauses[0]->is_prohibited) {
                bc_set_occur(bca->clauses[0], BC_MUST);
            }
        }
        if (!clause->is_prohibited) {
            bc_set_occur(clause, BC_MUST);
        }
        bca_add_clause(bca, clause);
    }
    return bca;
}

static BCArray *add_or_cls(BCArray *bca, BooleanClause *clause)
{
    if (clause) {
        bca_add_clause(bca, clause);
    }
    return bca;
}

static BCArray *add_default_cls(QParser *qp, BCArray *bca,
                                BooleanClause *clause)
{
    if (qp->or_default) {
        add_or_cls(bca, clause);
    }
    else {
        add_and_cls(bca, clause);
    }
    return bca;
}

static BooleanClause *get_bool_cls(Query *q, unsigned int occur)
{
    if (q) {
        return bc_new(q, occur);
    }
    else {
        return NULL;
    }
}

static Query *get_term_q(QParser *qp, char *field, char *word)
{
    Query *q;
    Token *token;
    TokenStream *stream = get_cached_ts(qp, field, word);

    if ((token = ts_next(stream)) == NULL) {
        q = NULL;
    }
    else {
        q = tq_new(field, token->text);
        if ((token = ts_next(stream)) != NULL) {
            /* Less likely case, destroy the term query and create a 
             * phrase query instead */
            Query *phq = phq_new(field);
            phq_add_term(phq, ((TermQuery *)q)->term, 0);
            q->destroy_i(q);
            q = phq;
            do {
                phq_add_term(q, token->text, token->pos_inc);
            } while ((token = ts_next(stream)) != NULL);
        }
    }
    return q;
}

static Query *get_fuzzy_q(QParser *qp, char *field, char *word, char *slop_str)
{
    Query *q;
    Token *token;
    TokenStream *stream = get_cached_ts(qp, field, word);

    if ((token = ts_next(stream)) == NULL) {
        q = NULL;
    }
    else {
        /* it only makes sense to find one term in a fuzzy query */
        float slop = DEF_MIN_SIM;
        if (slop_str) {
            sscanf(slop_str, "%f", &slop);
        }
        q = fuzq_new_conf(field, token->text, slop, DEF_PRE_LEN,
                          qp->max_clauses);
    }
    return q;
}

static char *lower_str(char *str)
{
    const int max_len = (int)strlen(str) + 1;
    int cnt;
    wchar_t *wstr = ALLOC_N(wchar_t, max_len);
    if ((cnt = mbstowcs(wstr, str, max_len)) > 0) {
        wchar_t *w = wstr;
        while (*w) {
            *w = towlower(*w);
            w++;
        }
        wcstombs(str, wstr, max_len);
    }
    else {
        char *s = str;
        while (*s) {
            *s = tolower(*s);
            s++;
        }
    }
    free(wstr);
    str[max_len] = '\0';
    return str;
}

static Query *get_wild_q(QParser *qp, char *field, char *pattern)
{
    Query *q;
    bool is_prefix = false;
    char *p;
    int len = (int)strlen(pattern);

    if (qp->wild_lower) {
        lower_str(pattern);
    }
    
    /* simplify the wildcard query to a prefix query if possible. Basically a
     * prefix query is any wildcard query that has a '*' as the last character
     * and no other wildcard characters before it. */
    if (pattern[len - 1] == '*') {
        is_prefix = true;
        for (p = &pattern[len - 2]; p >= pattern; p--) {
            if (*p == '*' || *p == '?') {
                is_prefix = false;
                break;
            }
        }
    }
    if (is_prefix) {
        /* chop off the '*' temporarily to create the query */
        pattern[len - 1] = 0;
        q = prefixq_new(field, pattern);;
        pattern[len - 1] = '*';
    }
    else {
        q = wcq_new(field, pattern);;
    }
    return q;
}

static HashSet *add_field(QParser *qp, char *field)
{
    if (qp->allow_any_fields || hs_exists(qp->all_fields, field)) {
        hs_add(qp->fields, get_cached_field(qp->field_cache, field));
    }
    return qp->fields;
}

static HashSet *first_field(QParser *qp, char *field)
{
    qp->fields = qp->fields_buf;
    qp->fields->size = 0;
    h_clear(qp->fields->ht);
    return add_field(qp, field);
}

static void ph_destroy(Phrase *self)
{
    int i;
    for (i = 0; i < self->size; i++) {
        ary_destroy(self->positions[i].terms, &free);
    }
    free(self->positions);
    free(self);
}


static Phrase *ph_new()
{
  Phrase *self = ALLOC_AND_ZERO(Phrase);
  self->capa = PHRASE_INIT_CAPA;
  self->positions = ALLOC_AND_ZERO_N(PhrasePosition, PHRASE_INIT_CAPA);
  return self;
}

static Phrase *ph_first_word(char *word)
{
    Phrase *self = ph_new();
    if (word) { /* no point in adding NULL in start */
        self->positions[0].terms = ary_new_type_capa(char *, 1);
        ary_push(self->positions[0].terms, estrdup(word));
        self->size = 1;
    }
    return self;
}

static Phrase *ph_add_word(Phrase *self, char *word)
{
    if (word) {
        const int index = self->size;
        PhrasePosition *pp = self->positions;
        if (index >= self->capa) {
            self->capa <<= 1;
            REALLOC_N(pp, PhrasePosition, self->capa);
            self->positions = pp;
        }
        pp[index].pos = self->pos_inc;
        pp[index].terms = ary_new_type_capa(char *, 1);
        ary_push(pp[index].terms, estrdup(word));
        self->size++;
        self->pos_inc = 0;
    }
    else {
        self->pos_inc++;
    }
    return self;
}

static Phrase *ph_add_multi_word(Phrase *self, char *word)
{
    const int index = self->size - 1;
    PhrasePosition *pp = self->positions;

    if (word) {
        ary_push(pp[index].terms, estrdup(word));
    }
    return self;
}

static Query *get_phrase_query(QParser *qp, char *field,
                           Phrase *phrase, char *slop_str)
{
    const int pos_cnt = phrase->size;
    Query *q = NULL;

    if (pos_cnt == 1) {
        char **words = phrase->positions[0].terms;
        const int word_count = ary_size(words);
        if (word_count == 1) {
            q = get_term_q(qp, field, words[0]);
        }
        else {
            int i;
            q = bq_new(false);
            for (i = 0; i < word_count; i++) {
                bq_add_query_nr(q, get_term_q(qp, field, words[i]), BC_SHOULD);
            }
        }
    }
    else if (pos_cnt > 1) {
        Token *token;
        TokenStream *stream;
        int i, j;
        q = phq_new(field);
        if (slop_str) {
            float slop;
            sscanf(slop_str,"%f",&slop);
            ((PhraseQuery *)q)->slop = slop;
        }

        for (i = 0; i < pos_cnt; i++) {
            int pos_inc = phrase->positions[i].pos; /* Actually holds pos_inc */
            char **words = phrase->positions[i].terms;
            const int word_count = ary_size(words);
            
            if (word_count == 1) {
                stream = get_cached_ts(qp, field, words[0]);
                while ((token = ts_next(stream))) {
                    phq_add_term(q, token->text, token->pos_inc + pos_inc);
                    pos_inc = 0;
                }
            }
            else {
                bool added_position = false;

                for (j = 0; j < word_count; j++) {
                    stream = get_cached_ts(qp, field, words[j]);
                    if ((token = ts_next(stream))) {
                        if (!added_position) {
                            phq_add_term(q, token->text, token->pos_inc + pos_inc);
                            added_position = true;
                        }
                        else {
                            phq_append_multi_term(q, token->text);
                        }
                    }
                }
            }
        }
    }
    return q;
}

static Query *get_phrase_q(QParser *qp, Phrase *phrase, char *slop_str)
{
    Query *q;
    FLDS(q, get_phrase_query(qp, field, phrase, slop_str));
    ph_destroy(phrase);
    return q;
}

static Query *get_range_q(const char *field, const char *from, const char *to,
                          bool inc_lower, bool inc_upper)
{
    return rq_new(field, from, to, inc_lower, inc_upper);
}

void qp_destroy(QParser *self)
{
    if (self->close_def_fields) {
        hs_destroy(self->def_fields);
    }
    hs_destroy(self->all_fields);
    hs_destroy(self->fields_buf);
    h_destroy(self->field_cache);
    h_destroy(self->ts_cache);
    a_deref(self->analyzer);
    free(self);
}

QParser *qp_new(HashSet *all_fields, HashSet *def_fields, Analyzer *analyzer)
{
    int i;
    QParser *self = ALLOC(QParser);
    self->or_default = true;
    self->wild_lower = true;
    self->clean_str = false;
    self->max_clauses = QP_MAX_CLAUSES;
    self->handle_parse_errors = false;
    self->allow_any_fields = false;
    self->def_slop = 0;
    self->fields_buf = hs_str_new(NULL);
    self->all_fields = all_fields;
    if (def_fields) {
        self->def_fields = def_fields;
        for (i = 0; i < self->def_fields->size; i++) {
            if (!hs_exists(self->all_fields, self->def_fields->elems[i])) {
                hs_add(self->all_fields, estrdup(self->def_fields->elems[i]));
            }
        }
        self->close_def_fields = true;
    }
    else {
        self->def_fields = all_fields;
        self->close_def_fields = false;
    }
    self->field_cache = h_new_str((free_ft)NULL, &free);
    for (i = 0; i < self->all_fields->size; i++) {
        char *field = estrdup(self->all_fields->elems[i]);
        h_set(self->field_cache, field, field);
    }
    self->fields = self->def_fields;
    /* make sure all_fields contains the default fields */
    self->analyzer = analyzer;
    self->ts_cache = h_new_str(NULL, (free_ft)&ts_deref);
    self->buf_index = 0;
    mutex_init(&self->mutex, NULL);
    return self;
}

/* these chars have meaning within phrases */
static const char *PHRASE_CHARS = "<>|\"";

static void str_insert(char *str, int len, char chr)
{
    memmove(str+1, str, len*sizeof(char));
    *str = chr;
}

char *qp_clean_str(char *str)
{
    int b, pb = -1;
    int br_cnt = 0;
    bool quote_open = false;
    char *sp, *nsp;

    /* leave a little extra */
    char *new_str = ALLOC_N(char, strlen(str)*2 + 1);

    for (sp = str, nsp = new_str; *sp; sp++) {
        b = *sp;
        /* ignore escaped characters */
        if (pb == '\\') {
            if (quote_open && strrchr(PHRASE_CHARS, b)) {
                *nsp++ = '\\'; /* this was left off the first time through */
            }
            *nsp++ = b;
            /* \\ has escaped itself so has no power. Assign pb random char : */
            pb = ((b == '\\') ? ':' : b);
            continue;
        }
        switch (b) {
            case '\\':
                if (!quote_open) { /* We do our own escaping below */
                    *nsp++ = b;
                }
                break;
            case '"':
                quote_open = !quote_open;
                *nsp++ = b;
                break;
            case '(':
              if (!quote_open) {
                  br_cnt++;
              }
              else {
                  *nsp++ = '\\';
              }
              *nsp++ = b;
              break;
            case ')':
                if (!quote_open) {
                    if (br_cnt == 0) {
                        str_insert(new_str, (int)(nsp - new_str), '(');
                        nsp++;
                    }
                    else {
                        br_cnt--;
                    }
                }
                else {
                    *nsp++ = '\\';
                }
                *nsp++ = b;
                break;
            case '>':
                if (quote_open) {
                    if (pb == '<') {
                        /* remove the escape character */
                        nsp--;
                        nsp[-1] = '<';
                    }
                    else {
                        *nsp++ = '\\';
                    }
                }
                *nsp++ = b;
                break;
            default:
                if (quote_open) {
                    if (strrchr(special_char, b) && b != '|') {
                        *nsp++ = '\\';
                    }
                }
                *nsp++ = b;
        }
        pb = b;
    }
    if (quote_open) {
        *nsp++ = '"';
    }
    for (;br_cnt > 0; br_cnt--) {
      *nsp++ = ')';
    }
    *nsp = '\0';
    return new_str;  
}

Query *qp_get_bad_query(QParser *qp, char *str)
{
    Query *q;
    FLDS(q, get_term_q(qp, field, str));
    return q;
}

Query *qp_parse(QParser *self, char *qstr)
{
    Query *result;
    mutex_lock(&self->mutex);
    if (self->clean_str) {
        self->qstrp = self->qstr = qp_clean_str(qstr);
    }
    else {
        self->qstrp = self->qstr = qstr;
    }
    self->fields = self->def_fields;
    self->result = NULL;

    yyparse(self);

    result = self->result;
    if (!result && self->handle_parse_errors) {
        result = qp_get_bad_query(self, self->qstr);
    }
    if (!result) {
        result = bq_new(false);
    }
    if (self->clean_str) {
        free(self->qstr);
    }

    mutex_unlock(&self->mutex);
    return result;
}



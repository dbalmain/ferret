/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse frt_parse
#define yylex   frt_lex
#define yyerror frt_error
#define yylval  frt_lval
#define yychar  frt_char
#define yydebug frt_debug
#define yynerrs frt_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     QWRD = 258,
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
#define QWRD 258
#define WILD_STR 259
#define LOW 260
#define OR 261
#define AND 262
#define NOT 263
#define REQ 264
#define HIGH 265




/* Copy the first part of user declarations.  */
#line 84 "src/q_parser.y"

#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <assert.h>
#include "except.h"
#include "search.h"
#include "array.h"
#include "symbol.h"
#include "internal.h"

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

float qp_default_fuzzy_min_sim = 0.5;
int qp_default_fuzzy_pre_len = 0;



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

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 113 "src/q_parser.y"
{
    Query *query;
    BooleanClause *bcls;
    BCArray *bclss;
    HashSet *hashset;
    Phrase *phrase;
    char *str;
}
/* Line 187 of yacc.c.  */
#line 163 "src/q_parser.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 121 "src/q_parser.y"

static int yylex(YYSTYPE *lvalp, QParser *qp);
static int yyerror(QParser *qp, char const *msg);

#define PHRASE_INIT_CAPA 4
static Query *get_bool_q(BCArray *bca);

static BCArray *first_cls(BooleanClause *boolean_clause);
static BCArray *add_and_cls(BCArray *bca, BooleanClause *clause);
static BCArray *add_or_cls(BCArray *bca, BooleanClause *clause);
static BCArray *add_default_cls(QParser *qp, BCArray *bca,
                                BooleanClause *clause);
static void bca_destroy(BCArray *bca);

static BooleanClause *get_bool_cls(Query *q, BCType occur);

static Query *get_term_q(QParser *qp, Symbol field, char *word);
static Query *get_fuzzy_q(QParser *qp, Symbol field, char *word,
                          char *slop);
static Query *get_wild_q(QParser *qp, Symbol field, char *pattern);

static HashSet *first_field(QParser *qp, const char *field);
static HashSet *add_field(QParser *qp, const char *field);

static Query *get_phrase_q(QParser *qp, Phrase *phrase, char *slop);

static Phrase *ph_first_word(char *word);
static Phrase *ph_add_word(Phrase *self, char *word);
static Phrase *ph_add_multi_word(Phrase *self, char *word);
static void ph_destroy(Phrase *self);

static Query *get_r_q(QParser *qp, Symbol field, char *from, char *to,
                      bool inc_lower, bool inc_upper);

static void qp_push_fields(QParser *self, HashSet *fields, bool destroy);
static void qp_pop_fields(QParser *self);

/**
 * +FLDS+ calls +func+ for all fields on top of the field stack. +func+
 * must return a query. If there is more than one field on top of FieldStack
 * then +FLDS+ will combing all the queries returned by +func+ into a single
 * BooleanQuery which it than assigns to +q+. If there is only one field, the
 * return value of +func+ is assigned to +q+ directly.
 */
#define FLDS(q, func) do {\
    TRY {\
        Symbol field;\
        if (qp->fields->size == 0) {\
            q = NULL;\
        } else if (qp->fields->size == 1) {\
            field = (Symbol)qp->fields->first->elem;\
            q = func;\
        } else {\
            Query *volatile sq; HashSetEntry *volatile hse;\
            q = bq_new_max(false, qp->max_clauses);\
            for (hse = qp->fields->first; hse; hse = hse->next) {\
                field = (Symbol)hse->elem;\
                sq = func;\
                TRY\
                  if (sq) bq_add_query_nr(q, sq, BC_SHOULD);\
                XCATCHALL\
                  if (sq) q_deref(sq);\
                XENDTRY\
            }\
            if (((BooleanQuery *)q)->clause_cnt == 0) {\
                q_deref(q);\
                q = NULL;\
            }\
        }\
    } XCATCHALL\
        qp->destruct = true;\
        HANDLED();\
    XENDTRY\
    if (qp->destruct && !qp->recovering && q) {q_deref(q); q = NULL;}\
} while (0)

#define Y if (qp->destruct) goto yyerrorlab;
#define T TRY
#define E\
  XCATCHALL\
    qp->destruct = true;\
    HANDLED();\
  XENDTRY\
  if (qp->destruct) Y;


/* Line 216 of yacc.c.  */
#line 261 "src/q_parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
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
      while (YYID (0))
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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  39
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   126

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  26
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  51
/* YYNRULES -- Number of states.  */
#define YYNSTATES  80

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   265

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    12,    16,    19,    22,
      25,    27,    29,    33,    35,    38,    42,    44,    46,    48,
      50,    52,    56,    59,    61,    62,    67,    68,    69,    75,
      77,    81,    85,    91,    94,    99,   101,   104,   107,   111,
     115,   120,   125,   130,   135,   139,   143,   147,   151,   154,
     158,   162
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      27,     0,    -1,    -1,    28,    -1,    29,    -1,    28,     7,
      29,    -1,    28,     6,    29,    -1,    28,    29,    -1,     9,
      30,    -1,     8,    30,    -1,    30,    -1,    31,    -1,    31,
      12,     3,    -1,    32,    -1,    13,    14,    -1,    13,    28,
      14,    -1,    34,    -1,    39,    -1,    41,    -1,    33,    -1,
       3,    -1,     3,    15,     3,    -1,     3,    15,    -1,     4,
      -1,    -1,    38,    10,    31,    35,    -1,    -1,    -1,    16,
      36,    10,    31,    37,    -1,     3,    -1,    38,    17,     3,
      -1,    18,    40,    18,    -1,    18,    40,    18,    15,     3,
      -1,    18,    18,    -1,    18,    18,    15,     3,    -1,     3,
      -1,    19,    20,    -1,    40,     3,    -1,    40,    19,    20,
      -1,    40,    17,     3,    -1,    21,     3,     3,    22,    -1,
      21,     3,     3,    23,    -1,    24,     3,     3,    22,    -1,
      24,     3,     3,    23,    -1,    19,     3,    23,    -1,    19,
       3,    22,    -1,    21,     3,    20,    -1,    24,     3,    20,
      -1,    19,     3,    -1,    19,    25,     3,    -1,    20,    25,
       3,    -1,    20,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   226,   226,   227,   229,   230,   231,   232,   234,   235,
     236,   238,   239,   241,   242,   243,   244,   245,   246,   247,
     249,   250,   251,   253,   255,   255,   257,   257,   257,   260,
     261,   263,   264,   265,   266,   268,   269,   270,   271,   272,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "QWRD", "WILD_STR", "LOW", "OR", "AND",
  "NOT", "REQ", "':'", "HIGH", "'^'", "'('", "')'", "'~'", "'*'", "'|'",
  "'\"'", "'<'", "'>'", "'['", "']'", "'}'", "'{'", "'='", "$accept",
  "bool_q", "bool_clss", "bool_cls", "boosted_q", "q", "term_q", "wild_q",
  "field_q", "@1", "@2", "@3", "field", "phrase_q", "ph_words", "range_q", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
      58,   265,    94,    40,    41,   126,    42,   124,    34,    60,
      62,    91,    93,   125,   123,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    26,    27,    27,    28,    28,    28,    28,    29,    29,
      29,    30,    30,    31,    31,    31,    31,    31,    31,    31,
      32,    32,    32,    33,    35,    34,    36,    37,    34,    38,
      38,    39,    39,    39,    39,    40,    40,    40,    40,    40,
      41,    41,    41,    41,    41,    41,    41,    41,    41,    41,
      41,    41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     3,     3,     2,     2,     2,
       1,     1,     3,     1,     2,     3,     1,     1,     1,     1,
       1,     3,     2,     1,     0,     4,     0,     0,     5,     1,
       3,     3,     5,     2,     4,     1,     2,     2,     3,     3,
       4,     4,     4,     4,     3,     3,     3,     3,     2,     3,
       3,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    20,    23,     0,     0,     0,    26,     0,     0,     0,
       0,     0,     0,     3,     4,    10,    11,    13,    19,    16,
       0,    17,    18,    22,     9,     8,    14,     0,     0,    35,
      33,     0,     0,    48,     0,    51,     0,     0,     0,     1,
       0,     0,     7,     0,     0,     0,    21,    15,     0,     0,
      36,    37,     0,    31,     0,    45,    44,    49,    50,     0,
      46,     0,    47,     6,     5,    12,    24,    30,    27,    34,
      39,     0,    38,    40,    41,    42,    43,    25,    28,    32
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    77,
      28,    78,    20,    21,    32,    22
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -30
static const yytype_int8 yypact[] =
{
      83,    -4,   -30,   102,   102,    64,   -30,     7,    -2,    -1,
       6,    15,    31,    45,   -30,   -30,    29,   -30,   -30,   -30,
      -5,   -30,   -30,    40,   -30,   -30,   -30,    26,    47,   -30,
      55,    42,    19,   -15,    68,   -30,    71,     0,     1,   -30,
      83,    83,   -30,    72,   102,    73,   -30,   -30,   102,    76,
     -30,   -30,    78,    74,    70,   -30,   -30,   -30,   -30,    -6,
     -30,    33,   -30,   -30,   -30,   -30,   -30,   -30,   -30,   -30,
     -30,    90,   -30,   -30,   -30,   -30,   -30,   -30,   -30,   -30
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -30,   -30,    89,   -13,    56,   -29,   -30,   -30,   -30,   -30,
     -30,   -30,   -30,   -30,   -30,   -30
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -30
static const yytype_int8 yytable[] =
{
      42,    33,    35,    59,    61,    44,   -29,    55,    56,    37,
      29,    23,    45,   -29,    42,    66,    73,    74,    38,    68,
      60,    62,    51,    34,    36,    30,    31,    63,    64,     1,
       2,    39,    40,    41,     3,     4,    52,    53,    54,     5,
      47,    43,     6,    46,     7,     8,     9,    10,     1,     2,
      11,    40,    41,     3,     4,    75,    76,    48,     5,    24,
      25,     6,    50,     7,     8,     9,    10,     1,     2,    11,
      49,    57,     3,     4,    58,    65,    67,     5,    26,    69,
       6,    70,     7,     8,     9,    10,     1,     2,    11,    71,
      72,     3,     4,    79,    27,     0,     5,     0,     0,     6,
       0,     7,     8,     9,    10,     1,     2,    11,     0,     0,
       0,     0,     0,     0,     0,     5,     0,     0,     6,     0,
       7,     8,     9,    10,     0,     0,    11
};

static const yytype_int8 yycheck[] =
{
      13,     3,     3,     3,     3,    10,    10,    22,    23,     3,
       3,    15,    17,    17,    27,    44,    22,    23,     3,    48,
      20,    20,     3,    25,    25,    18,    19,    40,    41,     3,
       4,     0,     6,     7,     8,     9,    17,    18,    19,    13,
      14,    12,    16,     3,    18,    19,    20,    21,     3,     4,
      24,     6,     7,     8,     9,    22,    23,    10,    13,     3,
       4,    16,    20,    18,    19,    20,    21,     3,     4,    24,
      15,     3,     8,     9,     3,     3,     3,    13,    14,     3,
      16,     3,    18,    19,    20,    21,     3,     4,    24,    15,
      20,     8,     9,     3,     5,    -1,    13,    -1,    -1,    16,
      -1,    18,    19,    20,    21,     3,     4,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    16,    -1,
      18,    19,    20,    21,    -1,    -1,    24
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     8,     9,    13,    16,    18,    19,    20,
      21,    24,    27,    28,    29,    30,    31,    32,    33,    34,
      38,    39,    41,    15,    30,    30,    14,    28,    36,     3,
      18,    19,    40,     3,    25,     3,    25,     3,     3,     0,
       6,     7,    29,    12,    10,    17,     3,    14,    10,    15,
      20,     3,    17,    18,    19,    22,    23,     3,     3,     3,
      20,     3,    20,    29,    29,     3,    31,     3,    31,     3,
       3,    15,    20,    22,    23,    22,    23,    35,    37,     3
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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (qp, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
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
    while (YYID (0))
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, qp); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, QParser *qp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, qp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    QParser *qp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (qp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, QParser *qp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, qp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    QParser *qp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, qp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, QParser *qp)
#else
static void
yy_reduce_print (yyvsp, yyrule, qp)
    YYSTYPE *yyvsp;
    int yyrule;
    QParser *qp;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , qp);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, qp); \
} while (YYID (0))

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
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
      YYSIZE_T yyn = 0;
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
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
      int yychecklim = YYLAST - yyn + 1;
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
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
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
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, QParser *qp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, qp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    QParser *qp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (qp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 27: /* "bool_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1230 "src/q_parser.c"
	break;
      case 28: /* "bool_clss" */
#line 223 "src/q_parser.y"
	{ if ((yyvaluep->bclss) && qp->destruct) bca_destroy((yyvaluep->bclss)); };
#line 1235 "src/q_parser.c"
	break;
      case 29: /* "bool_cls" */
#line 222 "src/q_parser.y"
	{ if ((yyvaluep->bcls) && qp->destruct) bc_deref((yyvaluep->bcls)); };
#line 1240 "src/q_parser.c"
	break;
      case 30: /* "boosted_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1245 "src/q_parser.c"
	break;
      case 31: /* "q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1250 "src/q_parser.c"
	break;
      case 32: /* "term_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1255 "src/q_parser.c"
	break;
      case 33: /* "wild_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1260 "src/q_parser.c"
	break;
      case 34: /* "field_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1265 "src/q_parser.c"
	break;
      case 39: /* "phrase_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1270 "src/q_parser.c"
	break;
      case 40: /* "ph_words" */
#line 224 "src/q_parser.y"
	{ if ((yyvaluep->phrase) && qp->destruct) ph_destroy((yyvaluep->phrase)); };
#line 1275 "src/q_parser.c"
	break;
      case 41: /* "range_q" */
#line 221 "src/q_parser.y"
	{ if ((yyvaluep->query) && qp->destruct) q_deref((yyvaluep->query)); };
#line 1280 "src/q_parser.c"
	break;

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (QParser *qp);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


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
	yytype_int16 *yyss1 = yyss;
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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
#line 226 "src/q_parser.y"
    {   qp->result = (yyval.query) = NULL; }
    break;

  case 3:
#line 227 "src/q_parser.y"
    { T qp->result = (yyval.query) = get_bool_q((yyvsp[(1) - (1)].bclss)); E }
    break;

  case 4:
#line 229 "src/q_parser.y"
    { T (yyval.bclss) = first_cls((yyvsp[(1) - (1)].bcls)); E }
    break;

  case 5:
#line 230 "src/q_parser.y"
    { T (yyval.bclss) = add_and_cls((yyvsp[(1) - (3)].bclss), (yyvsp[(3) - (3)].bcls)); E }
    break;

  case 6:
#line 231 "src/q_parser.y"
    { T (yyval.bclss) = add_or_cls((yyvsp[(1) - (3)].bclss), (yyvsp[(3) - (3)].bcls)); E }
    break;

  case 7:
#line 232 "src/q_parser.y"
    { T (yyval.bclss) = add_default_cls(qp, (yyvsp[(1) - (2)].bclss), (yyvsp[(2) - (2)].bcls)); E }
    break;

  case 8:
#line 234 "src/q_parser.y"
    { T (yyval.bcls) = get_bool_cls((yyvsp[(2) - (2)].query), BC_MUST); E }
    break;

  case 9:
#line 235 "src/q_parser.y"
    { T (yyval.bcls) = get_bool_cls((yyvsp[(2) - (2)].query), BC_MUST_NOT); E }
    break;

  case 10:
#line 236 "src/q_parser.y"
    { T (yyval.bcls) = get_bool_cls((yyvsp[(1) - (1)].query), BC_SHOULD); E }
    break;

  case 12:
#line 239 "src/q_parser.y"
    { T if ((yyvsp[(1) - (3)].query)) sscanf((yyvsp[(3) - (3)].str),"%f",&((yyvsp[(1) - (3)].query)->boost));  (yyval.query)=(yyvsp[(1) - (3)].query); E }
    break;

  case 14:
#line 242 "src/q_parser.y"
    { T (yyval.query) = bq_new_max(true, qp->max_clauses); E }
    break;

  case 15:
#line 243 "src/q_parser.y"
    { T (yyval.query) = get_bool_q((yyvsp[(2) - (3)].bclss)); E }
    break;

  case 20:
#line 249 "src/q_parser.y"
    { FLDS((yyval.query), get_term_q(qp, field, (yyvsp[(1) - (1)].str))); Y}
    break;

  case 21:
#line 250 "src/q_parser.y"
    { FLDS((yyval.query), get_fuzzy_q(qp, field, (yyvsp[(1) - (3)].str), (yyvsp[(3) - (3)].str))); Y}
    break;

  case 22:
#line 251 "src/q_parser.y"
    { FLDS((yyval.query), get_fuzzy_q(qp, field, (yyvsp[(1) - (2)].str), NULL)); Y}
    break;

  case 23:
#line 253 "src/q_parser.y"
    { FLDS((yyval.query), get_wild_q(qp, field, (yyvsp[(1) - (1)].str))); Y}
    break;

  case 24:
#line 255 "src/q_parser.y"
    { qp_pop_fields(qp); }
    break;

  case 25:
#line 256 "src/q_parser.y"
    { (yyval.query) = (yyvsp[(3) - (4)].query); }
    break;

  case 26:
#line 257 "src/q_parser.y"
    { qp_push_fields(qp, qp->all_fields, false); }
    break;

  case 27:
#line 257 "src/q_parser.y"
    { qp_pop_fields(qp); }
    break;

  case 28:
#line 258 "src/q_parser.y"
    { (yyval.query) = (yyvsp[(4) - (5)].query); }
    break;

  case 29:
#line 260 "src/q_parser.y"
    { (yyval.hashset) = first_field(qp, (yyvsp[(1) - (1)].str)); }
    break;

  case 30:
#line 261 "src/q_parser.y"
    { (yyval.hashset) = add_field(qp, (yyvsp[(3) - (3)].str));}
    break;

  case 31:
#line 263 "src/q_parser.y"
    { (yyval.query) = get_phrase_q(qp, (yyvsp[(2) - (3)].phrase), NULL); }
    break;

  case 32:
#line 264 "src/q_parser.y"
    { (yyval.query) = get_phrase_q(qp, (yyvsp[(2) - (5)].phrase), (yyvsp[(5) - (5)].str)); }
    break;

  case 33:
#line 265 "src/q_parser.y"
    { (yyval.query) = NULL; }
    break;

  case 34:
#line 266 "src/q_parser.y"
    { (yyval.query) = NULL; (void)(yyvsp[(4) - (4)].str);}
    break;

  case 35:
#line 268 "src/q_parser.y"
    { (yyval.phrase) = ph_first_word((yyvsp[(1) - (1)].str)); }
    break;

  case 36:
#line 269 "src/q_parser.y"
    { (yyval.phrase) = ph_first_word(NULL); }
    break;

  case 37:
#line 270 "src/q_parser.y"
    { (yyval.phrase) = ph_add_word((yyvsp[(1) - (2)].phrase), (yyvsp[(2) - (2)].str)); }
    break;

  case 38:
#line 271 "src/q_parser.y"
    { (yyval.phrase) = ph_add_word((yyvsp[(1) - (3)].phrase), NULL); }
    break;

  case 39:
#line 272 "src/q_parser.y"
    { (yyval.phrase) = ph_add_multi_word((yyvsp[(1) - (3)].phrase), (yyvsp[(3) - (3)].str));  }
    break;

  case 40:
#line 274 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (4)].str),  (yyvsp[(3) - (4)].str),  true,  true)); Y}
    break;

  case 41:
#line 275 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (4)].str),  (yyvsp[(3) - (4)].str),  true,  false)); Y}
    break;

  case 42:
#line 276 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (4)].str),  (yyvsp[(3) - (4)].str),  false, true)); Y}
    break;

  case 43:
#line 277 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (4)].str),  (yyvsp[(3) - (4)].str),  false, false)); Y}
    break;

  case 44:
#line 278 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, NULL,(yyvsp[(2) - (3)].str),  false, false)); Y}
    break;

  case 45:
#line 279 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, NULL,(yyvsp[(2) - (3)].str),  false, true)); Y}
    break;

  case 46:
#line 280 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (3)].str),  NULL,true,  false)); Y}
    break;

  case 47:
#line 281 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (3)].str),  NULL,false, false)); Y}
    break;

  case 48:
#line 282 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, NULL,(yyvsp[(2) - (2)].str),  false, false)); Y}
    break;

  case 49:
#line 283 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, NULL,(yyvsp[(3) - (3)].str),  false, true)); Y}
    break;

  case 50:
#line 284 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(3) - (3)].str),  NULL,true,  false)); Y}
    break;

  case 51:
#line 285 "src/q_parser.y"
    { FLDS((yyval.query), get_r_q(qp, field, (yyvsp[(2) - (2)].str),  NULL,false, false)); Y}
    break;


/* Line 1267 of yacc.c.  */
#line 1810 "src/q_parser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (qp, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (qp, yymsg);
	  }
	else
	  {
	    yyerror (qp, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
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
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, qp);
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
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, qp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
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
		 yytoken, &yylval, qp);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, qp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 287 "src/q_parser.y"


static const char *special_char = "&:()[]{}!\"~^|<>=*?+-";
static const char *not_word =   " \t()[]{}!\"~^|<>=";

/**
 * +get_word+ gets the next query-word from the query string. A query-word is
 * basically a string of non-special or escaped special characters. It is
 * Analyzer agnostic. It is up to the get_*_q methods to tokenize the word and
 * turn it into a +Query+. See the documentation for each get_*_q method to
 * see how it handles tokenization.
 *
 * Note that +get_word+ is also responsible for returning field names and
 * matching the special tokens 'AND', 'NOT', 'REQ' and 'OR'.
 */
static int get_word(YYSTYPE *lvalp, QParser *qp)
{
    bool is_wild = false;
    int len;
    char c;
    char *buf = qp->buf[qp->buf_index];
    char *bufp = buf;
    qp->buf_index = (qp->buf_index + 1) % QP_CONC_WORDS;

    if (qp->dynbuf) {
        free(qp->dynbuf);
        qp->dynbuf = NULL;
    }

    qp->qstrp--; /* need to back up one character */
    
    while (!strchr(not_word, (c = *qp->qstrp++))) {
        switch (c) {
            case '\\':
                if ((c = *qp->qstrp) == '\0') {
                    *bufp++ = '\\';
                }
                else {
                    *bufp++ = c;
                    qp->qstrp++;
                }
                break;
            case ':':
                if ((*qp->qstrp) == ':') {
                    qp->qstrp++;
                    *bufp++ = ':';
                    *bufp++ = ':';
                }
                else {
                   goto get_word_done;
                }
                break;
            case '*': case '?':
                is_wild = true;
                /* fall through */
            default:
                *bufp++ = c;
        }
        /* we've exceeded the static buffer. switch to the dynamic one. The
         * dynamic buffer is allocated enough space to hold the whole query
         * string so it's capacity doesn't need to be checked again once
         * allocated. */
        if (!qp->dynbuf && ((bufp - buf) == MAX_WORD_SIZE)) {
            qp->dynbuf = ALLOC_AND_ZERO_N(char, strlen(qp->qstr) + 1);
            strncpy(qp->dynbuf, buf, MAX_WORD_SIZE);
            buf = qp->dynbuf;
            bufp = buf + MAX_WORD_SIZE;
        }
    }
get_word_done:
    qp->qstrp--;
    /* check for keywords. There are only four so we have a bit of a hack
     * which just checks for all of them. */
    *bufp = '\0';
    len = (int)(bufp - buf);
    if (qp->use_keywords) {
        if (len == 3) {
            if (buf[0] == 'A' && buf[1] == 'N' && buf[2] == 'D') return AND;
            if (buf[0] == 'N' && buf[1] == 'O' && buf[2] == 'T') return NOT;
            if (buf[0] == 'R' && buf[1] == 'E' && buf[2] == 'Q') return REQ;
        }
        if (len == 2 && buf[0] == 'O' && buf[1] == 'R') return OR;
    }

    /* found a word so return it. */
    lvalp->str = buf;
    if (is_wild) {
        return WILD_STR;
    }
    return QWRD;
}

/**
 * +yylex+ is the lexing method called by the QueryParser. It breaks the
 * query up into special characters;
 * 
 *     ( "&:()[]{}!\"~^|<>=*?+-" )
 *
 * and tokens; 
 * 
 *   - QWRD
 *   - WILD_STR
 *   - AND['AND', '&&']
 *   - OR['OR', '||']
 *   - REQ['REQ', '+']
 *   - NOT['NOT', '-', '~']
 *
 * QWRD tokens are query word tokens which are made up of characters other
 * than the special characters. They can also contain special characters when
 * escaped with a backslash '\'. WILD_STR is the same as QWRD except that it
 * may also contain '?' and '*' characters.
 *
 * If any of the special chars are seen they will usually be returned straight
 * away. The exceptions are the wild chars '*' and '?', and '&' which will be
 * treated as a plain old word character unless followed by another '&'.
 * 
 * If no special characters or tokens are found then yylex delegates to
 * +get_word+ which will fetch the next query-word.
 */
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
            case '?':
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

/**
 * yyerror gets called if there is an parse error with the yacc parser.
 * It is responsible for clearing any memory that was allocated during the
 * parsing process.
 */
static int yyerror(QParser *qp, char const *msg)
{
    qp->destruct = true;
    if (!qp->handle_parse_errors) {
        char buf[1024];
        buf[1023] = '\0';
        strncpy(buf, qp->qstr, 1023);
        if (qp->clean_str) {
            free(qp->qstr);
        }
        mutex_unlock(&qp->mutex);
        snprintf(xmsg_buffer, XMSG_BUFFER_SIZE,
                 "couldn't parse query ``%s''. Error message "
                 " was %s", buf, (char *)msg);
    }
    while (qp->fields_top->next != NULL) {
        qp_pop_fields(qp);
    }
    return 0;
}

#define BQ(query) ((BooleanQuery *)(query))

/**
 * The QueryParser caches a tokenizer for each field so that it doesn't need
 * to allocate a new tokenizer for each term in the query. This would be quite
 * expensive as tokenizers use quite a large hunk of memory.
 *
 * This method returns the query parser for a particular field and sets it up
 * with the text to be tokenized.
 */
static TokenStream *get_cached_ts(QParser *qp, Symbol field, char *text)
{
    TokenStream *ts;
    if (hs_exists(qp->tokenized_fields, field)) {
        ts = (TokenStream *)h_get(qp->ts_cache, field);
        if (!ts) {
            ts = a_get_ts(qp->analyzer, field, text);
            h_set(qp->ts_cache, field, ts);
        }
        else {
            ts->reset(ts, text);
        }
    }
    else {
        ts = qp->non_tokenizer;
        ts->reset(ts, text);
    }
    return ts;
}

/**
 * Turns a BooleanClause array into a BooleanQuery. It will optimize the query
 * if 0 or 1 clauses are present to NULL or the actual query in the clause
 * respectively.
 */
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
        if (bc->is_prohibited) {
            q = bq_new(false);
            bq_add_query_nr(q, bc->query, BC_MUST_NOT);
            bq_add_query_nr(q, maq_new(), BC_MUST);
        }
        else {
            q = bc->query;
        }
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

/**
 * Base method for appending BooleanClauses to a BooleanClause array. This
 * method doesn't care about the type of clause (MUST, SHOULD, MUST_NOT).
 */
static void bca_add_clause(BCArray *bca, BooleanClause *clause)
{
    if (bca->size >= bca->capa) {
        bca->capa <<= 1;
        REALLOC_N(bca->clauses, BooleanClause *, bca->capa);
    }
    bca->clauses[bca->size] = clause;
    bca->size++;
}

/**
 * Add the first clause to a BooleanClause array. This method is also
 * responsible for allocating a new BooleanClause array.
 */
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

/**
 * Add AND clause to the BooleanClause array. The means that it will set the
 * clause being added and the previously added clause from SHOULD clauses to
 * MUST clauses. (If they are currently MUST_NOT clauses they stay as they
 * are.)
 */
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

/**
 * Add SHOULD clause to the BooleanClause array.
 */
static BCArray *add_or_cls(BCArray *bca, BooleanClause *clause)
{
    if (clause) {
        bca_add_clause(bca, clause);
    }
    return bca;
}

/**
 * Add AND or OR clause to the BooleanClause array, depending on the default
 * clause type.
 */
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

/**
 * destroy array of BooleanClauses
 */
static void bca_destroy(BCArray *bca)
{
    int i;
    for (i = 0; i < bca->size; i++) {
        bc_deref(bca->clauses[i]);
    }
    free(bca->clauses);
    free(bca);
}

/**
 * Turn a query into a BooleanClause for addition to a BooleanQuery.
 */
static BooleanClause *get_bool_cls(Query *q, BCType occur)
{
    if (q) {
        return bc_new(q, occur);
    }
    else {
        return NULL;
    }
}

/**
 * Create a TermQuery. The word will be tokenized and if the tokenization
 * produces more than one token, a PhraseQuery will be returned. For example,
 * if the word is dbalmain@gmail.com and a LetterTokenizer is used then a
 * PhraseQuery "dbalmain gmail com" will be returned which is actually exactly
 * what we want as it will match any documents containing the same email
 * address and tokenized with the same tokenizer.
 */
static Query *get_term_q(QParser *qp, Symbol field, char *word)
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
                if (token->pos_inc) {
                    phq_add_term(q, token->text, token->pos_inc);
                    /* add some slop since single term  was expected */
                    ((PhraseQuery *)q)->slop++;
                }
                else {
                    phq_append_multi_term(q, token->text);
                }
            } while ((token = ts_next(stream)) != NULL);
        }
    }
    return q;
}

/**
 * Create a FuzzyQuery. The word will be tokenized and only the first token
 * will be used. If there are any more tokens after tokenization, they will be
 * ignored.
 */
static Query *get_fuzzy_q(QParser *qp, Symbol field, char *word,
                          char *slop_str)
{
    Query *q;
    Token *token;
    TokenStream *stream = get_cached_ts(qp, field, word);

    if ((token = ts_next(stream)) == NULL) {
        q = NULL;
    }
    else {
        /* it only makes sense to find one term in a fuzzy query */
        float slop = qp_default_fuzzy_min_sim;
        if (slop_str) {
            sscanf(slop_str, "%f", &slop);
        }
        q = fuzq_new_conf(field, token->text, slop, qp_default_fuzzy_pre_len,
                          qp->max_clauses);
    }
    return q;
}

/**
 * Downcase a string taking locale into account and works for multibyte
 * character sets.
 */
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

/**
 * Create a WildCardQuery. No tokenization will be performed on the pattern
 * but the pattern will be downcased if +qp->wild_lower+ is set to true and
 * the field in question is a tokenized field.
 *
 * Note: this method will not always return a WildCardQuery. It could be
 * optimized to a MatchAllQuery if the pattern is '*' or a PrefixQuery if the
 * only wild char (*, ?) in the pattern is a '*' at the end of the pattern.
 */
static Query *get_wild_q(QParser *qp, Symbol field, char *pattern)
{
    Query *q;
    bool is_prefix = false;
    char *p;
    int len = (int)strlen(pattern);

    if (qp->wild_lower
        && (!qp->tokenized_fields || hs_exists(qp->tokenized_fields, field))) {
        lower_str(pattern);
    }
    
    /* simplify the wildcard query to a prefix query if possible. Basically a
     * prefix query is any wildcard query that has a '*' as the last character
     * and no other wildcard characters before it. "*" by itself will expand
     * to a MatchAllQuery */
    if (strcmp(pattern, "*") == 0) {
        return maq_new();
    }
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
        q = prefixq_new(field, pattern);
        pattern[len - 1] = '*';
    }
    else {
        q = wcq_new(field, pattern);
    }
    MTQMaxTerms(q) = qp->max_clauses;
    return q;
}

/**
 * Adds another field to the top of the FieldStack.
 */
static HashSet *add_field(QParser *qp, const char *field_name)
{
    Symbol field = intern(field_name);
    if (qp->allow_any_fields || hs_exists(qp->all_fields, field)) {
        hs_add(qp->fields, field);
    }
    return qp->fields;
}

/**
 * The method gets called when a field modifier ("field1|field2:") is seen. It
 * will push a new FieldStack object onto the stack and add +field+ to its
 * fields set.
 */
static HashSet *first_field(QParser *qp, const char *field)
{
    qp_push_fields(qp, hs_new_ptr(NULL), true);
    return add_field(qp, field);
}

/**
 * Destroy a phrase object freeing all allocated memory.
 */
static void ph_destroy(Phrase *self)
{
    int i;
    for (i = 0; i < self->size; i++) {
        ary_destroy(self->positions[i].terms, &free);
    }
    free(self->positions);
    free(self);
}


/**
 * Allocate a new Phrase object
 */
static Phrase *ph_new()
{
  Phrase *self = ALLOC_AND_ZERO(Phrase);
  self->capa = PHRASE_INIT_CAPA;
  self->positions = ALLOC_AND_ZERO_N(PhrasePosition, PHRASE_INIT_CAPA);
  return self;
}

/**
 * Add the first word to the phrase. This method is also in charge of
 * allocating a new Phrase object.
 */
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

/**
 * Add a new word to the Phrase
 */
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

/**
 * Adds a word to the Phrase object in the same position as the previous word
 * added to the Phrase. This will later be turned into a multi-PhraseQuery.
 */
static Phrase *ph_add_multi_word(Phrase *self, char *word)
{
    const int index = self->size - 1;
    PhrasePosition *pp = self->positions;

    if (word) {
        ary_push(pp[index].terms, estrdup(word));
    }
    return self;
}

/**
 * Build a phrase query for a single field. It might seem like a better idea
 * to build the PhraseQuery once and duplicate it for each field but this
 * would be buggy in the case of PerFieldAnalyzers in which case a different
 * tokenizer could be used for each field.
 *
 * Note that the query object returned by this method is not always a
 * PhraseQuery. If there is only one term in the query then the query is
 * simplified to a TermQuery. If there are multiple terms but only a single
 * position, then a MultiTermQuery is retured.
 *
 * Note that each word in the query gets tokenized. Unlike get_term_q, if the
 * word gets tokenized into more than one token, the rest of the tokens are
 * ignored. For example, if you have the phrase;
 *
 *      "email: dbalmain@gmail.com"
 * 
 * the Phrase object will contain to positions with the words 'email:' and
 * 'dbalmain@gmail.com'. Now, if you are using a LetterTokenizer then the
 * second word will be tokenized into the tokens ['dbalmain', 'gmail', 'com']
 * and only the first token will be used, so the resulting phrase query will
 * actually look like this;
 *
 *      "email dbalmain"
 *
 * This problem can easily be solved by using the StandardTokenizer or any
 * custom tokenizer which will leave dbalmain@gmail.com as a single token.
 */
static Query *get_phrase_query(QParser *qp, Symbol field,
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
            int term_cnt = 0;
            Token *token;
            char *last_word = NULL;

            for (i = 0; i < word_count; i++) {
                token = ts_next(get_cached_ts(qp, field, words[i]));
                if (token) {
                    free(words[i]);
                    last_word = words[i] = estrdup(token->text);
                    ++term_cnt;
                }
                else {
                    /* empty words will later be ignored */
                    words[i][0] = '\0';
                }
            }

            switch (term_cnt) {
                case 0:
                    q = bq_new(false);
                    break;
                case 1:
                    q = tq_new(field, last_word);
                    break;
                default:
                    q = multi_tq_new_conf(field, term_cnt, 0.0);
                    for (i = 0; i < word_count; i++) {
                        /* ignore empty words */
                        if (words[i][0]) {
                            multi_tq_add_term(q, words[i]);
                        }
                    }
                    break;
            }
        }
    }
    else if (pos_cnt > 1) {
        Token *token;
        TokenStream *stream;
        int i, j;
        int pos_inc = 0;
        q = phq_new(field);
        if (slop_str) {
            int slop;
            sscanf(slop_str,"%d",&slop);
            ((PhraseQuery *)q)->slop = slop;
        }

        for (i = 0; i < pos_cnt; i++) {
            char **words = phrase->positions[i].terms;
            const int word_count = ary_size(words);
            if (pos_inc) {
                ((PhraseQuery *)q)->slop++;
            }
            pos_inc += phrase->positions[i].pos + 1; /* Actually holds pos_inc*/
            
            if (word_count == 1) {
                stream = get_cached_ts(qp, field, words[0]);
                while ((token = ts_next(stream))) {
                    if (token->pos_inc) {
                        phq_add_term(q, token->text,
                                     pos_inc ? pos_inc : token->pos_inc);
                    }
                    else {
                        phq_append_multi_term(q, token->text);
                        ((PhraseQuery *)q)->slop++;
                    }
                    pos_inc = 0;
                }
            }
            else {
                bool added_position = false;

                for (j = 0; j < word_count; j++) {
                    stream = get_cached_ts(qp, field, words[j]);
                    if ((token = ts_next(stream))) {
                        if (!added_position) {
                            phq_add_term(q, token->text,
                                         pos_inc ? pos_inc : token->pos_inc);
                            added_position = true;
                            pos_inc = 0;
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

/**
 * Get a phrase query from the Phrase object. The Phrase object is built up by
 * the query parser as the all PhraseQuery didn't work well for this. Once the
 * PhraseQuery has been built the Phrase object needs to be destroyed.
 */
static Query *get_phrase_q(QParser *qp, Phrase *phrase, char *slop_str)
{
    Query *volatile q = NULL;
    FLDS(q, get_phrase_query(qp, field, phrase, slop_str));
    ph_destroy(phrase);
    return q;
}

/**
 * Gets a RangeQuery object.
 *
 * Just like with WildCardQuery, RangeQuery needs to downcase its terms if the
 * tokenizer also downcased its terms.
 */
static Query *get_r_q(QParser *qp, Symbol field, char *from, char *to,
                      bool inc_lower, bool inc_upper)
{
    Query *rq;
    if (qp->wild_lower
        && (!qp->tokenized_fields || hs_exists(qp->tokenized_fields, field))) {
        if (from) {
            lower_str(from);
        }
        if (to) {
            lower_str(to);
        }
    }
/*
 * terms don't get tokenized as it doesn't really make sense to do so for
 * range queries.

    if (from) {
        TokenStream *stream = get_cached_ts(qp, field, from);
        Token *token = ts_next(stream);
        from = token ? estrdup(token->text) : NULL;
    }
    if (to) {
        TokenStream *stream = get_cached_ts(qp, field, to);
        Token *token = ts_next(stream);
        to = token ? estrdup(token->text) : NULL;
    }
*/

    rq = qp->use_typed_range_query ?
        trq_new(field, from, to, inc_lower, inc_upper) :
        rq_new(field, from, to, inc_lower, inc_upper);
    return rq;
}

/**
 * Every time the query parser sees a new field modifier ("field1|field2:")
 * it pushes a new FieldStack object onto the stack and sets its fields to the
 * fields specified in the fields modifier. If the field modifier is '*',
 * fs->fields is set to all_fields. fs->fields is set to +qp->def_field+ at
 * the bottom of the stack (ie the very first set of fields pushed onto the
 * stack).
 */
static void qp_push_fields(QParser *self, HashSet *fields, bool destroy)
{
    FieldStack *fs = ALLOC(FieldStack); 

    fs->next    = self->fields_top;
    fs->fields  = fields;
    fs->destroy = destroy;

    self->fields_top = fs;
    self->fields = fields;
}

/**
 * Pops the top of the fields stack and frees any memory used by it. This will
 * get called when query modified by a field modifier ("field1|field2:") has
 * been fully parsed and the field specifier no longer applies.
 */
static void qp_pop_fields(QParser *self)
{
    FieldStack *fs = self->fields_top; 

    if (fs->destroy) {
        hs_destroy(fs->fields);
    }
    self->fields_top = fs->next;
    if (self->fields_top) {
        self->fields = self->fields_top->fields;
    }
    free(fs);
}

/**
 * Free all memory allocated by the QueryParser.
 */
void qp_destroy(QParser *self)
{
    if (self->tokenized_fields != self->all_fields) {
        hs_destroy(self->tokenized_fields);
    }
    if (self->def_fields != self->all_fields) {
        hs_destroy(self->def_fields);
    }
    hs_destroy(self->all_fields);

    qp_pop_fields(self);
    assert(NULL == self->fields_top);

    h_destroy(self->ts_cache);
    tk_destroy(self->non_tokenizer);
    a_deref(self->analyzer);
    free(self);
}

/**
 * Creates a new QueryParser setting all boolean parameters to their defaults.
 * If +def_fields+ is NULL then +all_fields+ is used in place of +def_fields+.
 * Not also that this method ensures that all fields that exist in
 * +def_fields+ must also exist in +all_fields+. This should make sense.
 */
QParser *qp_new(Analyzer *analyzer)
{
    QParser *self = ALLOC(QParser);
    self->or_default = true;
    self->wild_lower = true;
    self->clean_str = false;
    self->max_clauses = QP_MAX_CLAUSES;
    self->handle_parse_errors = false;
    self->allow_any_fields = false;
    self->use_keywords = true;
    self->use_typed_range_query = false;
    self->def_slop = 0;

    self->tokenized_fields = hs_new_ptr(NULL);
    self->all_fields = hs_new_ptr(NULL);
    self->def_fields = hs_new_ptr(NULL);

    self->fields_top = NULL;
    qp_push_fields(self, self->def_fields, false);

    /* make sure all_fields contains the default fields */
    self->analyzer = analyzer;
    self->ts_cache = h_new_ptr((free_ft)&ts_deref);
    self->buf_index = 0;
    self->dynbuf = NULL;
    self->non_tokenizer = non_tokenizer_new();
    mutex_init(&self->mutex, NULL);
    return self;
}

void qp_add_field(QParser *self,
                  Symbol field,
                  bool is_default,
                  bool is_tokenized)
{
    hs_add(self->all_fields, field);
    if (is_default) {
        hs_add(self->def_fields, field);
    }
    if (is_tokenized) {
        hs_add(self->tokenized_fields, field);
    }
}

/* these chars have meaning within phrases */
static const char *PHRASE_CHARS = "<>|\"";

/**
 * +str_insert_char+ inserts a character at the beginning of a string by
 * shifting the rest of the string right.
 */
static void str_insert_char(char *str, int len, char chr)
{
    memmove(str+1, str, len*sizeof(char));
    *str = chr;
}

/**
 * +qp_clean_str+ basically scans the query string and ensures that all open
 * and close parentheses '()' and quotes '"' are balanced. It does this by
 * inserting or appending extra parentheses or quotes to the string. This
 * obviously won't necessarily be exactly what the user wanted but we are
 * never going to know that anyway. The main job of this method is to help the
 * query at least parse correctly.
 *
 * It also checks that all special characters within phrases (ie between
 * quotes) are escaped correctly unless they have meaning within a phrase
 * ( <>,|," ). Note that '<' and '>' will also be escaped unless the appear
 * together like so; '<>'.
 */
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
            /* \ has escaped itself so has no power. Assign pb random char 'r' */
            pb = ((b == '\\') ? 'r' : b);
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
                        str_insert_char(new_str, (int)(nsp - new_str), '(');
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

/**
 * Takes a string and finds whatever tokens it can using the QueryParser's
 * analyzer. It then turns these tokens (if any) into a boolean query. If it
 * fails to find any tokens, this method will return NULL.
 */
static Query *qp_get_bad_query(QParser *qp, char *str)
{
    Query *volatile q = NULL;
    qp->recovering = true;
    assert(qp->fields_top->next == NULL);
    FLDS(q, get_term_q(qp, field, str));
    return q;
}

/**
 * +qp_parse+ takes a string and turns it into a Query object using Ferret's
 * query language. It must either raise an error or return a query object. It
 * must not return NULL. If the yacc parser fails it will use a very basic
 * boolean query parser which takes whatever tokens it can find in the query
 * and terns them into a boolean query on the default fields.
 */
Query *qp_parse(QParser *self, char *qstr)
{
    Query *result = NULL;
    mutex_lock(&self->mutex);
    /* if qp->fields_top->next is not NULL we have a left over field-stack
     * object that was not popped during the last query parse */
    assert(NULL == self->fields_top->next);

    self->recovering = self->destruct = false;
    if (self->clean_str) {
        self->qstrp = self->qstr = qp_clean_str(qstr);
    }
    else {
        self->qstrp = self->qstr = qstr;
    }
    self->fields = self->def_fields;
    self->result = NULL;

    if (0 == yyparse(self)) result = self->result;
    if (!result && self->handle_parse_errors) {
        self->destruct = false;
        result = qp_get_bad_query(self, self->qstr);
    }
    if (self->destruct && !self->handle_parse_errors) {
        xraise(PARSE_ERROR, xmsg_buffer);
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


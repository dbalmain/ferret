/*****************************************************************************
 * QueryParser
 * ===========
 *
 * Synopsis
 * --------
 *
 * === qp_parse
 *
 * The main QueryParser method is +qp_parse+. It gets called with a the query
 * string. The first thing it does is to clean the query string if
 * ((QueryParser *)self)->clean_str is set to true. The cleaning is done with
 * the qp_clean_str.
 * 
 * It then calls the yacc parser which will set self->result to the parsed
 * query. If parsing fails in anyway, self->result should be set to NULL, in
 * which case qp_parse does one of two things, depending on the value of
 * self->handle_parse_errors. If it is set to true, qp_parse attempts to do a
 * very basic parsing of the query by ignoring all special characters and
 * parsing the query as a plain boolean query. If it is set to false, qp_parse
 * will raise a PARSE_ERROR.
 * 
 * === The Lexer
 *
 * yylex is the lexing method called by the QueryParser. It breaks the query
 * up into special characters ( "&:()[]{}!\"~^|<>=*?+-" ) and tokens (QWRD,
 * WILD_STR, AND['AND', '&&'], OR['OR', '||'], REQ['REQ', '+'], NOT['NOT',
 * '-', '~']). QWRD tokens are query word tokens which are made up of
 * characters other than the special characters. They can also contain special
 * characters when escaped with a backslash '\'. WILD_STR is the same as QWRD
 * except that it may also contain '?' and '*' characters.
 *
 * === The Parser
 *
 * For a better understanding of the how the query parser works, it is a good
 * idea to study the Ferret Query Language (FQL) described below. Once you
 * understand FQL the one tricky part that needs to be mentioned is how fields
 * are handled. The QueryParser knows about two sets of fields, the default
 * search fields and the set of all fields in the index. When no fields are
 * specified then the default fields are used. The '*:' field specifier will
 * search all fields contained in the all_fields set. Otherwise all fields
 * specified in the field descripter separated by '|' will be searched. For
 * example 'title|content:' will search the title and content fields. When
 * fields are specified like this, the parser will push the fields onto a
 * stack and all queries modified by the field specifier will be applied to
 * the fields on top of the stack. This is where the FLDS macro comes into
 * place. It takes the current query building function in the parser and calls
 * it for all fields on top of the stack.
 * 
 * Ferret Query Language (FQL)
 * ===========================
 *
 * FIXME to be continued...
 *****************************************************************************/
%{
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <assert.h>
#include "except.h"
#include "search.h"
#include "array.h"
#include "intern.h"
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

%}
%union {
    Query *query;
    BooleanClause *bcls;
    BCArray *bclss;
    HashSet *hashset;
    Phrase *phrase;
    char *str;
}
%{
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

static Query *get_term_q(QParser *qp, char *field, char *word);
static Query *get_fuzzy_q(QParser *qp, char *field, char *word, char *slop);
static Query *get_wild_q(QParser *qp, char *field, char *pattern);

static HashSet *first_field(QParser *qp, char *field);
static HashSet *add_field(QParser *qp, char *field);

static Query *get_phrase_q(QParser *qp, Phrase *phrase, char *slop);

static Phrase *ph_first_word(char *word);
static Phrase *ph_add_word(Phrase *self, char *word);
static Phrase *ph_add_multi_word(Phrase *self, char *word);
static void ph_destroy(Phrase *self);

static Query *get_r_q(QParser *qp, char *field, char *from, char *to,
                      bool inc_lower, bool inc_upper);

static void qp_push_fields(QParser *self, HashSet *fields, bool destroy);
static void qp_pop_fields(QParser *self);

#define FLDS(q, func) do {\
    TRY {\
        char *field;\
        if (qp->fields->size == 0) {\
            q = NULL;\
        } else if (qp->fields->size == 1) {\
            field = (char *)qp->fields->first->elem;\
            q = func;\
        } else {\
            Query *volatile sq;HashSetEntry *volatile hse;\
            q = bq_new_max(false, qp->max_clauses);\
            for (hse = qp->fields->first; hse; hse = hse->next) {\
                field = (char *)hse->elem;\
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
%}
%expect 1
%pure-parser
%parse-param { QParser *qp }
%lex-param   { QParser *qp }
%token <str>    QWRD WILD_STR
%type <query>   q bool_q boosted_q term_q wild_q field_q phrase_q range_q
%type <bcls>    bool_cls
%type <bclss>   bool_clss
%type <hashset> field
%type <phrase>  ph_words
%nonassoc LOW
%left AND OR
%nonassoc REQ NOT
%left ':'
%nonassoc HIGH
%destructor { if ($$ && qp->destruct) q_deref($$); } q bool_q boosted_q term_q wild_q field_q phrase_q range_q
%destructor { if ($$ && qp->destruct) bc_deref($$); } bool_cls
%destructor { if ($$ && qp->destruct) bca_destroy($$); } bool_clss
%destructor { if ($$ && qp->destruct) ph_destroy($$); } ph_words
%%
bool_q    : /* Nothing */             {   qp->result = $$ = NULL; }
          | bool_clss                 { T qp->result = $$ = get_bool_q($1); E }
          ;
bool_clss : bool_cls                  { T $$ = first_cls($1); E }
          | bool_clss AND bool_cls    { T $$ = add_and_cls($1, $3); E }
          | bool_clss OR bool_cls     { T $$ = add_or_cls($1, $3); E }
          | bool_clss bool_cls        { T $$ = add_default_cls(qp, $1, $2); E }
          ;
bool_cls  : REQ boosted_q             { T $$ = get_bool_cls($2, BC_MUST); E }
          | NOT boosted_q             { T $$ = get_bool_cls($2, BC_MUST_NOT); E }
          | boosted_q                 { T $$ = get_bool_cls($1, BC_SHOULD); E }
          ;
boosted_q : q
          | q '^' QWRD                { T if ($1) sscanf($3,"%f",&($1->boost));  $$=$1; E }
          ;
q         : term_q
          | '(' ')'                   { T $$ = bq_new_max(true, qp->max_clauses); E }
          | '(' bool_clss ')'         { T $$ = get_bool_q($2); E }
          | field_q
          | phrase_q
          | range_q
          | wild_q
          ;
term_q    : QWRD                      { FLDS($$, get_term_q(qp, field, $1)); Y}
          | QWRD '~' QWRD %prec HIGH  { FLDS($$, get_fuzzy_q(qp, field, $1, $3)); Y}
          | QWRD '~' %prec LOW        { FLDS($$, get_fuzzy_q(qp, field, $1, NULL)); Y}
          ;
wild_q    : WILD_STR                  { FLDS($$, get_wild_q(qp, field, $1)); Y}
          ;
field_q   : field ':' q { qp_pop_fields(qp); }
                                      { $$ = $3; }
          | '*' { qp_push_fields(qp, qp->all_fields, false); } ':' q { qp_pop_fields(qp); }
                                      { $$ = $4; }
          ;
field     : QWRD                      { $$ = first_field(qp, $1); }
          | field '|' QWRD            { $$ = add_field(qp, $3);}
          ;
phrase_q  : '"' ph_words '"'          { $$ = get_phrase_q(qp, $2, NULL); }
          | '"' ph_words '"' '~' QWRD { $$ = get_phrase_q(qp, $2, $5); }
          | '"' '"'                   { $$ = NULL; }
          | '"' '"' '~' QWRD          { $$ = NULL; (void)$4;}
          ;
ph_words  : QWRD              { $$ = ph_first_word($1); }
          | '<' '>'           { $$ = ph_first_word(NULL); }
          | ph_words QWRD     { $$ = ph_add_word($1, $2); }
          | ph_words '<' '>'  { $$ = ph_add_word($1, NULL); }
          | ph_words '|' QWRD { $$ = ph_add_multi_word($1, $3);  }
          ;
range_q   : '[' QWRD QWRD ']' { FLDS($$, get_r_q(qp, field, $2,  $3,  true,  true)); Y}
          | '[' QWRD QWRD '}' { FLDS($$, get_r_q(qp, field, $2,  $3,  true,  false)); Y}
          | '{' QWRD QWRD ']' { FLDS($$, get_r_q(qp, field, $2,  $3,  false, true)); Y}
          | '{' QWRD QWRD '}' { FLDS($$, get_r_q(qp, field, $2,  $3,  false, false)); Y}
          | '<' QWRD '}'      { FLDS($$, get_r_q(qp, field, NULL,$2,  false, false)); Y}
          | '<' QWRD ']'      { FLDS($$, get_r_q(qp, field, NULL,$2,  false, true)); Y}
          | '[' QWRD '>'      { FLDS($$, get_r_q(qp, field, $2,  NULL,true,  false)); Y}
          | '{' QWRD '>'      { FLDS($$, get_r_q(qp, field, $2,  NULL,false, false)); Y}
          | '<' QWRD          { FLDS($$, get_r_q(qp, field, NULL,$2,  false, false)); Y}
          | '<' '=' QWRD      { FLDS($$, get_r_q(qp, field, NULL,$3,  false, true)); Y}
          | '>' '='  QWRD     { FLDS($$, get_r_q(qp, field, $3,  NULL,true,  false)); Y}
          | '>' QWRD          { FLDS($$, get_r_q(qp, field, $2,  NULL,false, false)); Y}
          ;
%%

static const char *special_char = "&:()[]{}!\"~^|<>=*?+-";
static const char *not_word =   " \t()[]{}!\"~^|<>=";

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
        /* we've exceeded the static buffer. switch to the dynamic
           one. */
        if (!qp->dynbuf && ((bufp - buf) == MAX_WORD_SIZE)) {
            qp->dynbuf = ALLOC_AND_ZERO_N(char, strlen(qp->qstr) + 1);
            strncpy(qp->dynbuf, buf, MAX_WORD_SIZE);
            buf = qp->dynbuf;
            bufp = buf + MAX_WORD_SIZE;
        }
    }
get_word_done:
    qp->qstrp--;
    /* check for keywords. There are only four so we have a bit of a hack which
     * just checks for all of them. */
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
    return 0;
}

#define BQ(query) ((BooleanQuery *)(query))

static TokenStream *get_cached_ts(QParser *qp, char *field, char *text)
{
    TokenStream *ts;
    if (!qp->tokenized_fields || hs_exists(qp->tokenized_fields, field)) {
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

static void bca_destroy(BCArray *bca)
{
    int i;
    for (i = 0; i < bca->size; i++) {
        bc_deref(bca->clauses[i]);
    }
    free(bca->clauses);
    free(bca);
}

static BooleanClause *get_bool_cls(Query *q, BCType occur)
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
        float slop = qp_default_fuzzy_min_sim;
        if (slop_str) {
            sscanf(slop_str, "%f", &slop);
        }
        q = fuzq_new_conf(field, token->text, slop, qp_default_fuzzy_pre_len,
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

static HashSet *add_field(QParser *qp, char *field)
{
    if (qp->allow_any_fields || hs_exists(qp->all_fields, field)) {
        hs_add(qp->fields, (char *)intern(field));
    }
    return qp->fields;
}

// XXX
static HashSet *first_field(QParser *qp, char *field)
{
    qp_push_fields(qp, hs_new_str(NULL), true);
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
            int term_cnt = 0;
            Token *token;
            char *last_word = NULL;

            for (i = 0; i < word_count; i++) {
                token = ts_next(get_cached_ts(qp, field, words[i]));
                free(words[i]);
                if (token) {
                    last_word = words[i] = estrdup(token->text);
                    ++term_cnt;
                }
                else {
                    words[i] = estrdup("");
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

static Query *get_phrase_q(QParser *qp, Phrase *phrase, char *slop_str)
{
    Query *volatile q = NULL;
    FLDS(q, get_phrase_query(qp, field, phrase, slop_str));
    ph_destroy(phrase);
    return q;
}

static Query *get_r_q(QParser *qp, char *field, char *from, char *to,
                      bool inc_lower, bool inc_upper)
{
    Query *rq;
    if (qp->wild_lower) {
        if (from) {
            lower_str(from);
        }
        if (to) {
            lower_str(to);
        }
    }
/*
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

static void qp_push_fields(QParser *self, HashSet *fields, bool destroy)
{
    FieldStack *fs = ALLOC(FieldStack); 

    fs->next    = self->fields_top;
    fs->fields  = fields;
    fs->destroy = destroy;

    self->fields_top = fs;
    self->fields = fields;
}

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

void qp_destroy(QParser *self)
{
    if (self->def_fields != self->all_fields) {
        hs_destroy(self->def_fields);
    }
    if (self->tokenized_fields) {
        hs_destroy(self->tokenized_fields);
    }
    if (self->dynbuf) {
        free(self->dynbuf);
    }
    hs_destroy(self->all_fields);

    qp_pop_fields(self);
    assert(NULL == self->fields_top);

    h_destroy(self->ts_cache);
    tk_destroy(self->non_tokenizer);
    a_deref(self->analyzer);
    free(self);
}

QParser *qp_new(HashSet *all_fields, HashSet *def_fields,
                HashSet *tokenized_fields, Analyzer *analyzer)
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
    self->all_fields = all_fields;
    self->tokenized_fields = tokenized_fields;
    if (def_fields) {
        HashSetEntry *hse;
        self->def_fields = def_fields;
        for (hse = def_fields->first; hse; hse = hse->next) {
            if (!hs_exists(self->all_fields, hse->elem)) {
                hs_add(self->all_fields, hse->elem);
            }
        }
    }
    else {
        self->def_fields = all_fields;
    }
    self->fields_top = NULL;
    qp_push_fields(self, self->def_fields, false);

    /* make sure all_fields contains the default fields */
    self->analyzer = analyzer;
    self->ts_cache = h_new_str(NULL, (free_ft)&ts_deref);
    self->buf_index = 0;
    self->dynbuf = 0;
    self->non_tokenizer = non_tokenizer_new();
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

/*****************************************************************************
 * qp_clean_str method which basically scans the query string and ensures that
 * all open and close parentheses '()' and quotes '"' are balanced. It does
 * this be inserting or appending extra parentheses or quotes which is not
 * necessarily going to be exactly what the user wanted but it will help
 * prevent the parser from failing so it's the best we can do at this stage.
 * It also checks 
 *****************************************************************************/
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

static Query *qp_get_bad_query(QParser *qp, char *str)
{
    Query *volatile q = NULL;
    qp->recovering = true;
    FLDS(q, get_term_q(qp, field, str));
    return q;
}

Query *qp_parse(QParser *self, char *qstr)
{
    Query *result = NULL;
    mutex_lock(&self->mutex);
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

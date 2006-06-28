%{
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
%}
%expect 1
%pure-parser
%parse-param { QParser *qp }
%lex-param   { QParser *qp }
%token <str>    WORD WILD_STR
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
%%
bool_q    : /* Nothing */             { qp->result = $$ = NULL; }
          | bool_clss                 { qp->result = $$ = get_bool_q($1); }
          ;
bool_clss : bool_cls                  { $$ = first_cls($1); }
          | bool_clss AND bool_cls    { $$ = add_and_cls($1, $3); }
          | bool_clss OR bool_cls     { $$ = add_or_cls($1, $3); }
          | bool_clss bool_cls        { $$ = add_default_cls(qp, $1, $2); }
          ;
bool_cls  : REQ boosted_q             { $$ = get_bool_cls($2, BC_MUST); }
          | NOT boosted_q             { $$ = get_bool_cls($2, BC_MUST_NOT); }
          | boosted_q                 { $$ = get_bool_cls($1, BC_SHOULD); }
          ;
boosted_q : q
          | q '^' WORD                { if ($1) sscanf($3,"%f",&($1->boost)); $$=$1; }
          ;
q         : term_q
          | '(' bool_clss ')'         { $$ = get_bool_q($2); }
          | field_q
          | phrase_q
          | range_q
          | wild_q
          ;
term_q    : WORD                      { FLDS($$, get_term_q(qp, field, $1)); }
          | WORD '~' WORD %prec HIGH  { FLDS($$, get_fuzzy_q(qp, field, $1, $3)); }
          | WORD '~' %prec LOW        { FLDS($$, get_fuzzy_q(qp, field, $1, NULL)); }
          ;
wild_q    : WILD_STR                  { FLDS($$, get_wild_q(qp, field, $1)); }
          ;
field_q   : field ':' q { qp->fields = qp->def_fields; }
                                      { $$ = $3; }
          | '*' { qp->fields = qp->all_fields; } ':' q {qp->fields = qp->def_fields;}
                                      { $$ = $4; }
          ;
field     : WORD                      { $$ = first_field(qp, $1); }
          | field '|' WORD            { $$ = add_field(qp, $3);}
          ;
phrase_q  : '"' ph_words '"'          { $$ = get_phrase_q(qp, $2, NULL); }
          | '"' ph_words '"' '~' WORD { $$ = get_phrase_q(qp, $2, $5); }
          | '"' '"'                   { $$ = NULL; }
          | '"' '"' '~' WORD          { $$ = NULL; }
          ;
ph_words  : WORD              { $$ = ph_first_word($1); }
          | '<' '>'           { $$ = ph_first_word(NULL); }
          | ph_words WORD     { $$ = ph_add_word($1, $2); }
          | ph_words '<' '>'  { $$ = ph_add_word($1, NULL); }
          | ph_words '|' WORD { $$ = ph_add_multi_word($1, $3);  }
          ;
range_q   : '[' WORD WORD ']' { FLDS($$, get_range_q(field, $2,  $3,  true,  true)); }
          | '[' WORD WORD '}' { FLDS($$, get_range_q(field, $2,  $3,  true,  false)); }
          | '{' WORD WORD ']' { FLDS($$, get_range_q(field, $2,  $3,  false, true)); }
          | '{' WORD WORD '}' { FLDS($$, get_range_q(field, $2,  $3,  false, false)); }
          | '<' WORD '}'      { FLDS($$, get_range_q(field, NULL,$2,  false, false)); }
          | '<' WORD ']'      { FLDS($$, get_range_q(field, NULL,$2,  false, true)); }
          | '[' WORD '>'      { FLDS($$, get_range_q(field, $2,  NULL,true,  false)); }
          | '{' WORD '>'      { FLDS($$, get_range_q(field, $2,  NULL,false, false)); }
          | '<' WORD          { FLDS($$, get_range_q(field, NULL,$2,  false, false)); }
          | '<' '=' WORD      { FLDS($$, get_range_q(field, NULL,$3,  false, true)); }
          | '>' '='  WORD     { FLDS($$, get_range_q(field, $3,  NULL,true,  false)); }
          | '>' WORD          { FLDS($$, get_range_q(field, $2,  NULL,false, false)); }
          ;
%%

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

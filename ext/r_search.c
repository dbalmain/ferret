#include "ferret.h"
#include <st.h>
#include <rubysig.h>
#include "search.h"

extern VALUE cTerm;
extern VALUE cDirectory;

extern VALUE cIndexReader;
extern void frt_ir_free(void *p);
extern void frt_ir_mark(void *p);


static VALUE cScoreDoc;
static VALUE cTopDocs;
static VALUE cExplanation;
static VALUE cSearcher;
static VALUE cIndexSearcher;
static VALUE cMultiSearcher;
static VALUE cSortField;
static VALUE cSortType;
static VALUE cSort;
static VALUE cIndex;

/* Queries */
static VALUE cQuery;
static VALUE cTermQuery;
static VALUE cBooleanQuery;
static VALUE cBooleanClause;
static VALUE cBCOccur;
static VALUE cRangeQuery;
static VALUE cPhraseQuery;
static VALUE cMultiPhraseQuery;
static VALUE cPrefixQuery;
static VALUE cWildCardQuery;
static VALUE cFuzzyQuery;
static VALUE cMatchAllQuery;
static VALUE cConstantScoreQuery;
static VALUE cFilteredQuery;
static VALUE cSpanTermQuery;
static VALUE cSpanFirstQuery;
static VALUE cSpanNearQuery;
static VALUE cSpanOrQuery;
static VALUE cSpanNotQuery;

/* Filters */
static VALUE cFilter;
static VALUE cRangeFilter;
static VALUE cQueryFilter;

/* Option hash keys */
static VALUE rfirst_doc_key;
static VALUE rnum_docs_key;
static VALUE rfilter_key;
static VALUE rsort_key;

static VALUE rsort_type_key;
static VALUE rreverse_key;
static VALUE rcomparator_key;

static VALUE rpath_key;
static VALUE rcreate_key;
static VALUE rcreate_if_missing_key;
static VALUE rid_field_key;
static VALUE rdefault_field_key;
static VALUE rdefault_search_field_key;
static VALUE rdir_key;
static VALUE rclose_dir_key;
static VALUE rkey_key;
static VALUE ruse_compound_file_key;
static VALUE rauto_flush_key;
static VALUE rcheck_latest_key;
/* from r_qparser.c */
extern VALUE rhandle_parse_errors_key;
extern VALUE roccur_default_key;
extern VALUE rwild_lower_key;
extern VALUE rdefault_slop_key;
extern VALUE rclean_str_key;
extern VALUE rallow_any_fields_key;
extern VALUE ranalyzer_key;

/* Class variable ids */
static VALUE rdefault_min_similarity_id;
static VALUE rdefault_prefix_length_id;

extern void frt_set_term(VALUE rterm, Term *t);
extern Term *frt_get_term(VALUE rterm);
extern VALUE frt_get_analyzer(Analyzer *a);
extern HashSet *frt_get_fields(VALUE rfields);
extern Analyzer *frt_get_cwrapped_analyzer(VALUE ranalyzer);

/****************************************************************************
 *
 * ScoreDoc Methods
 *
 ****************************************************************************/

static void
frt_sd_free(void *p)
{
  object_del(p);
  free(p);
}

static VALUE
frt_get_sd(Hit *hit)
{
  VALUE self = Data_Wrap_Struct(cScoreDoc, NULL, &frt_sd_free, hit);
  object_add(hit, self);
  return self;
}

#define GET_HIT Hit *hit; Data_Get_Struct(self, Hit, hit)
static VALUE
frt_sd_score(VALUE self)
{
  GET_HIT;
  return rb_float_new(hit->score);
}

static VALUE
frt_sd_doc(VALUE self)
{
  GET_HIT;
  return INT2FIX(hit->doc);
}

/****************************************************************************
 *
 * TopDocs Methods
 *
 ****************************************************************************/

static void
frt_td_free(void *p)
{
  TopDocs *td = (TopDocs *)p;
  object_del(td);
  if (td->hits) {
    object_del(td->hits);
    free(td->hits);
  }
  free(td);
}

static void
frt_td_mark(void *p)
{
  TopDocs *td = (TopDocs *)p;
  frt_gc_mark(td->hits);
}

#define GET_TD TopDocs *td = (TopDocs *)DATA_PTR(self)

static VALUE
frt_get_td(TopDocs *td)
{
  int i;
  VALUE self = Data_Wrap_Struct(cTopDocs, &frt_td_mark, &frt_td_free, td);
  object_add(td, self);
  if (td->hits) {
    VALUE hits = rb_ary_new2(td->size);
    for (i = 0; i < td->size; i++) {
      rb_ary_store(hits, i, frt_get_sd(td->hits[i]));
    }
    object_add(td->hits, hits);
  }
  return self;
}

static VALUE
frt_td_hits(VALUE self)
{
  GET_TD;
  if (td->hits) {
    return object_get(td->hits);
  } else {
    return rb_ary_new();
  }
}

static VALUE
frt_td_size(VALUE self)
{
  GET_TD;
  return INT2FIX(td->size);
}

static VALUE
frt_td_total_hits(VALUE self)
{
  GET_TD;
  return INT2FIX(td->total_hits);
}

static VALUE
frt_td_fields(VALUE self)
{
  rb_raise(rb_eNotImpError, "not implemented in the c extension version");
  return Qnil;
}

static VALUE
frt_td_each(VALUE self)
{
  int i;
  Hit *hit;
  GET_TD;
  for (i = 0; i < td->size; i++) {
    hit = td->hits[i];
    rb_yield_values(2, INT2FIX(hit->doc), rb_float_new(hit->score));
  }
  return INT2FIX(td->size);
}

/****************************************************************************
 *
 * Explanation Methods
 *
 ****************************************************************************/

#define GET_EXPL Explanation *expl = (Explanation *)DATA_PTR(self)

static VALUE
frt_expl_to_s(VALUE self)
{
  GET_EXPL;
  char *str = expl_to_s(expl, 0);
  VALUE rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

static VALUE
frt_expl_to_html(VALUE self)
{
  GET_EXPL;
  char *str = expl_to_html(expl);
  VALUE rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

static VALUE
frt_expl_value(VALUE self)
{
  GET_EXPL;
  return rb_float_new((double)expl->value);
}

/****************************************************************************
 *
 * Query Methods
 *
 ****************************************************************************/

static void
frt_q_free(void *p)
{
  object_del(p);
  q_deref((Query *)p);
}

#define GET_Q Query *q = (Query *)DATA_PTR(self)


static VALUE
frt_q_to_s(int argc, VALUE *argv, VALUE self)
{
  VALUE rfield, rstr;
  char *str, *field = "";
  GET_Q;
  if (rb_scan_args(argc, argv, "01", &rfield)) {
    rfield = rb_obj_as_string(rfield);
    field = RSTRING(rfield)->ptr;
  }
  str = q->to_s(q, field);
  rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

static VALUE
frt_q_get_boost(VALUE self)
{
  GET_Q;
  return rb_float_new((double)q->boost);
}

static VALUE
frt_q_set_boost(VALUE self, VALUE rboost)
{
  GET_Q;
  q->boost = NUM2DBL(rboost);
  return rboost;
}

#define MK_QUERY(klass, q) Data_Wrap_Struct(klass, NULL, &frt_q_free, q)
VALUE
frt_get_q(Query *q)
{
  VALUE self = object_get(q);

  if (self == Qnil) {
    switch (q->type) {
      case TERM_QUERY:
        self = MK_QUERY(cTermQuery, q);
        break;
      case BOOLEAN_QUERY:
        self = MK_QUERY(cBooleanQuery, q);
        break;
      case PHRASE_QUERY:
        self = MK_QUERY(cPhraseQuery, q);
        break;
      case MULTI_PHRASE_QUERY:
        self = MK_QUERY(cMultiPhraseQuery, q);
        break;
      case CONSTANT_QUERY:
        self = MK_QUERY(cConstantScoreQuery, q);
        break;
      case MATCH_ALL_QUERY:
        self = MK_QUERY(cMatchAllQuery, q);
        break;
      case RANGE_QUERY:
        self = MK_QUERY(cRangeQuery, q);
        break;
      case WILD_CARD_QUERY:
        self = MK_QUERY(cWildCardQuery, q);
        break;
      case FUZZY_QUERY:
        self = MK_QUERY(cFuzzyQuery, q);
        break;
      case PREFIX_QUERY:
        self = MK_QUERY(cPrefixQuery, q);
        break;
      case FILTERED_QUERY:
        self = MK_QUERY(cFilteredQuery, q);
        break;
      case SPAN_TERM_QUERY:
        self = MK_QUERY(cSpanTermQuery, q);
        break;
      case SPAN_FIRST_QUERY:
        self = MK_QUERY(cSpanFirstQuery, q);
        break;
      case SPAN_OR_QUERY:
        self = MK_QUERY(cSpanOrQuery, q);
        break;
      case SPAN_NOT_QUERY:
        self = MK_QUERY(cSpanNotQuery, q);
        break;
      case SPAN_NEAR_QUERY:
        self = MK_QUERY(cSpanNearQuery, q);
        break;
      default:
        self = MK_QUERY(cQuery, q);
        break;
    }
    object_add(q, self);
  }
  return self;
}

/****************************************************************************
 *
 * TermQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_tq_init(VALUE self, VALUE rterm)
{
  Term *t = frt_get_term(rterm);
  Query *q = tq_create(t);
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * BooleanClause Methods
 *
 ****************************************************************************/

static void
frt_bc_mark(void *p)
{
  BooleanClause *bc = (BooleanClause *)p;
  frt_gc_mark(bc->query);
}

static void
frt_bc_free(void *p)
{
  object_del(p);
  bc_deref((BooleanClause *)p);  
}

static VALUE
frt_get_bc(BooleanClause *bc)
{
  VALUE self = Data_Wrap_Struct(cBooleanClause, &frt_bc_mark, &frt_bc_free, bc);
  ref(bc);
  object_add(bc, self);
  return self;
}

static VALUE
frt_bc_init(int argc, VALUE *argv, VALUE self)
{
  BooleanClause *bc;
  VALUE rquery, roccur;
  unsigned int occur = BC_SHOULD;
  Query *sub_q;
  if (rb_scan_args(argc, argv, "11", &rquery, &roccur) == 2) {
    occur = FIX2INT(roccur);
  }
  Data_Get_Struct(rquery, Query, sub_q);
  ref(sub_q);
  bc = bc_create(sub_q, occur);
  Frt_Wrap_Struct(self, &frt_bc_mark, &frt_bc_free, bc);
  object_add(bc, self);
  return self;
}

#define GET_BC BooleanClause *bc = (BooleanClause *)DATA_PTR(self)
static VALUE
frt_bc_get_query(VALUE self)
{
  GET_BC;
  return object_get(bc->query);
}

static VALUE
frt_bc_set_query(VALUE self, VALUE rquery)
{
  GET_BC;
  Data_Get_Struct(rquery, Query, bc->query);
  return rquery;
}

static VALUE
frt_bc_is_required(VALUE self)
{
  GET_BC;
  return bc->is_required ? Qtrue : Qfalse;
}

static VALUE
frt_bc_is_prohibited(VALUE self)
{
  GET_BC;
  return bc->is_prohibited ? Qtrue : Qfalse;
}

static VALUE
frt_bc_set_occur(VALUE self, VALUE roccur)
{
  GET_BC;
  bc_set_occur(bc, FIX2INT(roccur));
  return roccur;
}

static VALUE
frt_bc_to_s(VALUE self)
{
  VALUE rstr;
  char *qstr, *ostr = "", *str;
  int len;
  GET_BC;
  qstr = bc->query->to_s(bc->query, "");
  switch (bc->occur) {
    case BC_SHOULD:
      ostr = "Should";
      break;
    case BC_MUST:
      ostr = "Must";
      break;
    case BC_MUST_NOT:
      ostr = "Must Not";
      break;
  }
  len = strlen(ostr) + strlen(qstr) + 2;
  str = ALLOC_N(char, len);
  sprintf(str, "%s:%s", ostr, qstr);
  rstr = rb_str_new(str, len);
  free(qstr);
  free(str);
  return rstr;
}

/****************************************************************************
 *
 * BooleanQuery Methods
 *
 ****************************************************************************/

static void
frt_bq_mark(void *p)
{
  int i;
  Query *q = (Query *)p;
  BooleanQuery *bq = (BooleanQuery *)q->data;
  for (i = 0; i < bq->clause_cnt; i++) {
    frt_gc_mark(bq->clauses[i]);
  }
}

static VALUE
frt_bq_init(int argc, VALUE *argv, VALUE self)
{
  VALUE rcoord_disabled;
  bool coord_disabled = false;
  Query *q;
  if (rb_scan_args(argc, argv, "01", &rcoord_disabled)) {
    coord_disabled = RTEST(rcoord_disabled);
  }
  q = bq_create(coord_disabled);
  Frt_Wrap_Struct(self, &frt_bq_mark, &frt_q_free, q);
  object_add(q, self);

  /* don't destroy sub_queries as they are held by ruby objects */
  q->destroy_all = false;
  return self;
}

static VALUE
frt_bq_add_query(int argc, VALUE *argv, VALUE self)
{
  GET_Q;
  VALUE rquery, roccur;
  int occur = BC_SHOULD;
  Query *sub_q;
  if (rb_scan_args(argc, argv, "11", &rquery, &roccur) == 2) {
    occur = FIX2INT(roccur);
  }
  Data_Get_Struct(rquery, Query, sub_q);
  return frt_get_bc(bq_add_query(q, sub_q, occur));
}

static VALUE
frt_bq_add_clause(VALUE self, VALUE rclause)
{
  BooleanClause *bc;
  GET_Q;
  Data_Get_Struct(rclause, BooleanClause, bc);
  bq_add_clause(q, bc);
  return rclause;
}

/****************************************************************************
 *
 * RangeQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_rq_init(VALUE self, VALUE rfield, VALUE rlterm, VALUE ruterm,
    VALUE rincl, VALUE rincu)
{
  Query *q;
  char *lterm = NIL_P(rlterm) ? NULL : RSTRING(rb_obj_as_string(rlterm))->ptr;
  char *uterm = NIL_P(ruterm) ? NULL : RSTRING(rb_obj_as_string(ruterm))->ptr;
  if (!lterm && !uterm) 
    rb_raise(rb_eArgError, "The bounds of a range should not both be nil");
  if (RTEST(rincl) && !lterm) 
    rb_raise(rb_eArgError, "The lower bound should not be nil if it is inclusive");
  if (RTEST(rincu) && !uterm) 
    rb_raise(rb_eArgError, "The upper bound should not be nil if it is inclusive");
  if (uterm && lterm && (strcmp(uterm, lterm) < 0))
    rb_raise(rb_eArgError, "The upper bound should greater than the lower bound."
       " %s > %s", lterm, uterm);
  rfield = rb_obj_as_string(rfield);
  q = rq_create(RSTRING(rfield)->ptr, lterm, uterm, RTEST(rincl), RTEST(rincu));
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

static VALUE
frt_rq_new_more(VALUE klass, VALUE rfield, VALUE rlterm, VALUE rincl)
{
  Query *q;
  VALUE self;
  char *lterm = NIL_P(rlterm) ? NULL : RSTRING(rb_obj_as_string(rlterm))->ptr;
  rfield = rb_obj_as_string(rfield);
  if (!lterm) {
    rb_raise(rb_eArgError, "The lower term must not be nil in a more "
                           "than query");
  }
  q = rq_create_more(RSTRING(rfield)->ptr, lterm, RTEST(rincl));
  self = Data_Wrap_Struct(klass, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

static VALUE
frt_rq_new_less(VALUE klass, VALUE rfield, VALUE ruterm, VALUE rincu)
{
  Query *q;
  VALUE self;
  char *uterm = NIL_P(ruterm) ? NULL : RSTRING(rb_obj_as_string(ruterm))->ptr;
  rfield = rb_obj_as_string(rfield);
  if (!uterm) {
    rb_raise(rb_eArgError, "The upper term must not be nil in a less "
                           "than query");
  }
  q = rq_create_less(RSTRING(rfield)->ptr, uterm, RTEST(rincu));
  self = Data_Wrap_Struct(klass, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * PhraseQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_phq_alloc(VALUE klass)
{
  Query *q = phq_create();
  VALUE self = Data_Wrap_Struct(klass, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

static VALUE
frt_phq_init(VALUE self)
{
  return self;
}

static VALUE
frt_phq_add(int argc, VALUE *argv, VALUE self)
{
  VALUE rterm, rpos_inc;
  int pos_inc = 1;
  Term *t;
  GET_Q;
  if (rb_scan_args(argc, argv, "11", &rterm, &rpos_inc) == 2) {
    pos_inc = FIX2INT(rpos_inc);
  }
  t = frt_get_term(rterm);
  phq_add_term(q, t, pos_inc);
  return self;
}

static VALUE
frt_phq_shift(VALUE self, VALUE rterm)
{
  return frt_phq_add(1, &rterm, self);
}

static VALUE
frt_phq_get_slop(VALUE self)
{
  GET_Q;
  PhraseQuery *pq = (PhraseQuery *)q->data;
  return INT2FIX(pq->slop);
}

static VALUE
frt_phq_set_slop(VALUE self, VALUE rslop)
{
  GET_Q;
  PhraseQuery *pq = (PhraseQuery *)q->data;
  pq->slop = FIX2INT(rslop);
  return self;
}

/****************************************************************************
 *
 * MultiPhraseQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_mphq_alloc(VALUE klass)
{
  Query *q = mphq_create();
  VALUE self = Data_Wrap_Struct(klass, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

static VALUE
frt_mphq_init(VALUE self)
{
  return self;
}

static VALUE
frt_mphq_add(int argc, VALUE *argv, VALUE self)
{
  VALUE rterms, rpos_inc;
  int pos_inc = 1;
  int t_cnt;
  Term **terms;
  GET_Q;
  if (rb_scan_args(argc, argv, "11", &rterms, &rpos_inc) == 2) {
    pos_inc = FIX2INT(rpos_inc);
  }
  if (TYPE(rterms) == T_ARRAY) {
    int i;
    t_cnt = RARRAY(rterms)->len;
    terms = ALLOC_N(Term *, t_cnt);
    for (i = 0; i < t_cnt; i++) {
      terms[i] = frt_get_term(RARRAY(rterms)->ptr[i]);
    }
  } else {
    Check_Type(rterms, T_DATA);
    if (RBASIC(rterms)->klass != cTerm) {
      rb_raise(rb_eArgError, "passed an unknown type to add");
    }
    t_cnt = 1;
    terms = ALLOC_N(Term *, t_cnt);
    terms[0] = frt_get_term(rterms);
  }
  mphq_add_terms(q, terms, t_cnt, pos_inc);
  return self;
}

static VALUE
frt_mphq_shift(VALUE self, VALUE rterm)
{
  return frt_mphq_add(1, &rterm, self);
}

static VALUE
frt_mphq_get_slop(VALUE self)
{
  GET_Q;
  MultiPhraseQuery *pq = (MultiPhraseQuery *)q->data;
  return INT2FIX(pq->slop);
}

static VALUE
frt_mphq_set_slop(VALUE self, VALUE rslop)
{
  GET_Q;
  MultiPhraseQuery *pq = (MultiPhraseQuery *)q->data;
  pq->slop = FIX2INT(rslop);
  return self;
}

/****************************************************************************
 *
 * PrefixQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_prq_init(VALUE self, VALUE rterm)
{
  Term *t = frt_get_term(rterm);
  Query *q = prefixq_create(t);
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * WildCardQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_wcq_init(VALUE self, VALUE rterm)
{
  Term *t = frt_get_term(rterm);
  Query *q = wcq_create(t);
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * FuzzyQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_fq_init(int argc, VALUE *argv, VALUE self)
{
  Term *t;
  Query *q;
  VALUE rterm, rmin_sim, rpre_len;
  FuzzyQuery *fq;
  float min_sim;
  int pre_len;
  rb_scan_args(argc, argv, "12", &rterm, &rmin_sim, &rpre_len);
  switch (argc) {
    case 1:
      rmin_sim = rb_cvar_get(cFuzzyQuery, rdefault_min_similarity_id);
    case 2:
      rpre_len = rb_cvar_get(cFuzzyQuery, rdefault_prefix_length_id);
  }
  min_sim = NUM2DBL(rmin_sim);
  if (min_sim >= 1.0) {
    rb_raise(rb_eArgError, "minimum_similarity cannot be greater than or equal to 1");
  } else if (min_sim < 0.0) {
    rb_raise(rb_eArgError, "minimum_similarity cannot be less than 0");
  }
  pre_len = INT2FIX(rpre_len);
  if (pre_len < 0) {
    rb_raise(rb_eArgError, "prefix_length cannot be less than 0");
  }

  t = frt_get_term(rterm);
  q = fuzq_create(t);
  fq = (FuzzyQuery *)q->data;

  fq->pre_len = FIX2INT(rpre_len);
  fq->min_sim = (float)NUM2DBL(rmin_sim);
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

static VALUE
frt_fq_get_dms(VALUE self)
{
  return rb_cvar_get(cFuzzyQuery, rdefault_min_similarity_id);
}

static VALUE
frt_fq_set_dms(VALUE self, VALUE val)
{
  double min_sim = NUM2DBL(val);
  if (min_sim >= 1.0) {
    rb_raise(rb_eArgError, "minimum_similarity cannot be greater than or equal to 1");
  } else if (min_sim < 0.0) {
    rb_raise(rb_eArgError, "minimum_similarity cannot be less than 0");
  }
  rb_cvar_set(cFuzzyQuery, rdefault_min_similarity_id, val, Qfalse);
  return val;
}

static VALUE
frt_fq_get_dpl(VALUE self)
{
  return rb_cvar_get(cFuzzyQuery, rdefault_prefix_length_id);
}

static VALUE
frt_fq_set_dpl(VALUE self, VALUE val)
{
  int pre_len = INT2FIX(val);
  if (pre_len < 0) {
    rb_raise(rb_eArgError, "prefix_length cannot be less than 0");
  }
  rb_cvar_set(cFuzzyQuery, rdefault_prefix_length_id, val, Qfalse);
  return val;
}

/****************************************************************************
 *
 * MatchAllQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_maq_alloc(VALUE klass)
{
  Query *q = maq_create();
  VALUE self = Data_Wrap_Struct(klass, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

static VALUE
frt_maq_init(VALUE self)
{
  return self;
}

/****************************************************************************
 *
 * ConstantScoreQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_csq_init(VALUE self, VALUE rfilter)
{
  Query *q;
  Filter *filter;
  Data_Get_Struct(rfilter, Filter, filter);
  q = csq_create(filter);

  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * FilteredQuery Methods
 *
 ****************************************************************************/

static void
frt_fqq_mark(void *p)
{
  FilteredQuery *fq = (FilteredQuery *)((Query *)p)->data;
  frt_gc_mark(fq->query);
  frt_gc_mark(fq->filter);
}

static VALUE
frt_fqq_init(VALUE self, VALUE rquery, VALUE rfilter)
{
  Query *sq, *q;
  Filter *f;
  Data_Get_Struct(rquery, Query, sq);
  Data_Get_Struct(rfilter, Filter, f);
  q = fq_create(sq, f);
  q->destroy_all = false;
  Frt_Wrap_Struct(self, &frt_fqq_mark, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * SpanTermQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_spantq_init(VALUE self, VALUE rterm)
{
  Term *t = frt_get_term(rterm);
  Query *q = spantq_create(t);
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * SpanFirstQuery Methods
 *
 ****************************************************************************/

static VALUE
frt_spanfq_init(VALUE self, VALUE rmatch, VALUE rend)
{
  Query *q;
  Query *match;
  Data_Get_Struct(rmatch, Query, match);
  q = spanfq_create(match, FIX2INT(rend));
  q->destroy_all = false;
  Frt_Wrap_Struct(self, NULL, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * SpanNearQuery Methods
 *
 ****************************************************************************/

static void
frt_spannq_free(void *p)
{
  SpanNearQuery *snq = (SpanNearQuery *)((SpanQuery *)((Query *)p)->data)->data;
  free(snq->clauses);
  frt_q_free(p);
}

static void
frt_spannq_mark(void *p)
{
  int i;
  SpanNearQuery *snq = (SpanNearQuery *)((SpanQuery *)((Query *)p)->data)->data;
  for (i = 0; i < snq->c_cnt; i++) {
    frt_gc_mark(snq->clauses[i]);
  }
}

static VALUE
frt_spannq_init(VALUE self, VALUE rclauses, VALUE rslop, VALUE rin_order)
{
  int i;
  Query *q;
  Query **clauses;
  Check_Type(rclauses, T_ARRAY);
  clauses = ALLOC_N(Query *, RARRAY(rclauses)->len);
  for (i = 0; i < RARRAY(rclauses)->len; i++) {
    Data_Get_Struct(RARRAY(rclauses)->ptr[i], Query, clauses[i]);
  }

  q = spannq_create(clauses, RARRAY(rclauses)->len,
      FIX2INT(rslop), RTEST(rin_order));
  q->destroy_all = false;
  Frt_Wrap_Struct(self, &frt_spannq_mark, &frt_spannq_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * SpanOrQuery Methods
 *
 ****************************************************************************/

static void
frt_spanoq_free(void *p)
{
  SpanOrQuery *soq = (SpanOrQuery *)((SpanQuery *)((Query *)p)->data)->data;
  free(soq->clauses);
  frt_q_free(p);
}

static void
frt_spanoq_mark(void *p)
{
  int i;
  SpanOrQuery *soq = (SpanOrQuery *)((SpanQuery *)((Query *)p)->data)->data;
  for (i = 0; i < soq->c_cnt; i++) {
    frt_gc_mark(soq->clauses[i]);
  }
}

static VALUE
frt_spanoq_init(VALUE self, VALUE rclauses)
{
  int i;
  Query *q;
  Query **clauses;
  Check_Type(rclauses, T_ARRAY);
  clauses = ALLOC_N(Query *, RARRAY(rclauses)->len);
  for (i = 0; i < RARRAY(rclauses)->len; i++) {
    Data_Get_Struct(RARRAY(rclauses)->ptr[i], Query, clauses[i]);
  }

  q = spanoq_create(clauses, RARRAY(rclauses)->len);
  q->destroy_all = false;
  Frt_Wrap_Struct(self, &frt_spanoq_mark, &frt_spanoq_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * SpanNotQuery Methods
 *
 ****************************************************************************/

static void
frt_spanxq_mark(void *p)
{
  SpanNotQuery *sxq = (SpanNotQuery *)((SpanQuery *)((Query *)p)->data)->data;
  frt_gc_mark(sxq->inc);
  frt_gc_mark(sxq->exc);
}

static VALUE
frt_spanxq_init(VALUE self, VALUE rinc, VALUE rexc)
{
  Query *q;
  Check_Type(rinc, T_DATA);
  Check_Type(rexc, T_DATA);
  q = spanxq_create(DATA_PTR(rinc), DATA_PTR(rexc));
  q->destroy_all = false;
  Frt_Wrap_Struct(self, &frt_spanxq_mark, &frt_q_free, q);
  object_add(q, self);
  return self;
}

/****************************************************************************
 *
 * Filter Methods
 *
 ****************************************************************************/

static void
frt_f_free(void *p)
{
  Filter *f = (Filter *)p;
  object_del(p);
  f->destroy(f);
}

#define GET_F Filter *f = (Filter *)DATA_PTR(self)

static VALUE
frt_f_to_s(VALUE self)
{
  VALUE rstr;
  char *str;
  GET_F;
  str = f->to_s(f);
  rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

/****************************************************************************
 *
 * RangeFilter Methods
 *
 ****************************************************************************/

static VALUE
frt_rf_init(VALUE self, VALUE rfield, VALUE rlterm, VALUE ruterm,
    VALUE rincl, VALUE rincu)
{
  Filter *f;
  char *lterm = NIL_P(rlterm) ? NULL : RSTRING(rb_obj_as_string(rlterm))->ptr;
  char *uterm = NIL_P(ruterm) ? NULL : RSTRING(rb_obj_as_string(ruterm))->ptr;
  if (!lterm && !uterm) 
    rb_raise(rb_eArgError, "The bounds of a range should not both be nil");
  if (RTEST(rincl) && !lterm) 
    rb_raise(rb_eArgError, "The lower bound should not be nil if it is inclusive");
  if (RTEST(rincu) && !uterm) 
    rb_raise(rb_eArgError, "The upper bound should not be nil if it is inclusive");
  if (uterm && lterm && (strcmp(uterm, lterm) < 0))
    rb_raise(rb_eArgError, "The upper bound should greater than the lower bound."
       " %s > %s", lterm, uterm);
  rfield = rb_obj_as_string(rfield);
  f = rfilt_create(RSTRING(rfield)->ptr, lterm, uterm, RTEST(rincl), RTEST(rincu));
  Frt_Wrap_Struct(self, NULL, &frt_f_free, f);
  object_add(f, self);
  return self;
}

static VALUE
frt_rf_new_more(int argc, VALUE *argv, VALUE klass)
{
  Filter *f;
  VALUE self;
  VALUE rfield, rlterm, rincl;
  char *lterm;
  rb_scan_args(argc, argv, "21", &rfield, &rlterm, &rincl);
  rfield = rb_obj_as_string(rfield);
  lterm = NIL_P(rlterm) ? NULL : RSTRING(rb_obj_as_string(rlterm))->ptr;
  if (!lterm) {
    rb_raise(rb_eArgError, "The lower term must not be nil in a more "
                           "than filter");
  }
  f = rfilt_create(RSTRING(rfield)->ptr, lterm, NULL, rincl != Qfalse, false);
  self = Data_Wrap_Struct(klass, NULL, &frt_f_free, f);
  object_add(f, self);
  return self;
}

static VALUE
frt_rf_new_less(int argc, VALUE *argv, VALUE klass)
{
  Filter *f;
  VALUE self;
  VALUE rfield, ruterm, rincu;
  char *uterm;
  rb_scan_args(argc, argv, "21", &rfield, &ruterm, &rincu);
  rfield = rb_obj_as_string(rfield);
  uterm = NIL_P(ruterm) ? NULL : RSTRING(rb_obj_as_string(ruterm))->ptr;
  if (!uterm) {
    rb_raise(rb_eArgError, "The upper term must not be nil in a less "
                           "than filter");
  }
  f = rfilt_create(RSTRING(rfield)->ptr, NULL, uterm, false, rincu != Qfalse);
  self = Data_Wrap_Struct(klass, NULL, &frt_f_free, f);
  object_add(f, self);
  return self;
}

/****************************************************************************
 *
 * QueryFilter Methods
 *
 ****************************************************************************/

static VALUE
frt_qf_init(VALUE self, VALUE rquery)
{
  Query *q;
  Filter *f;
  Data_Get_Struct(rquery, Query, q);
  f = qfilt_create(q);
  Frt_Wrap_Struct(self, NULL, &frt_f_free, f);
  object_add(f, self);
  return self;
}

/****************************************************************************
 *
 * SortField Methods
 *
 ****************************************************************************/

static void 
frt_sf_free(void *p)
{
  object_del(p);
  sort_field_destroy((SortField *)p);
}

static VALUE
frt_get_sf(SortField *sf)
{
  VALUE self = object_get(sf);
  if (self == Qnil) {
    self = Data_Wrap_Struct(cSortField, NULL, &frt_sf_free, sf);
    object_add(sf, self);
  }
  return self;
}

static VALUE
frt_sf_init(int argc, VALUE *argv, VALUE self)
{
  SortField *sf;
  VALUE rfield, roptions;
  VALUE rval;
  int sort_type = SORT_TYPE_AUTO;
  int is_reverse = false;
  if (rb_scan_args(argc, argv, "11", &rfield, &roptions) == 2) {
    if (Qnil != (rval = rb_hash_aref(roptions, rsort_type_key))) {
      sort_type = FIX2INT(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rreverse_key))) {
      is_reverse = RTEST(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rcomparator_key))) {
      rb_raise(rb_eArgError, "Unsupported argument ':comparator'");
    }
  }
  if (NIL_P(rfield)) rb_raise(rb_eArgError, "must pass a valid field name");
  rfield = rb_obj_as_string(rfield);

  sf = sort_field_create(RSTRING(rfield)->ptr, sort_type, is_reverse);
  
  Frt_Wrap_Struct(self, NULL, &frt_sf_free, sf);
  object_add(sf, self);
  return self;
}

#define GET_SF SortField *sf = (SortField *)DATA_PTR(self)
static VALUE
frt_sf_is_reverse(VALUE self)
{
  GET_SF;
  return sf->reverse ? Qtrue : Qfalse;
}

static VALUE
frt_sf_get_name(VALUE self)
{
  GET_SF;
  return sf->field ? rb_str_new2(sf->field) : Qnil;
}

static VALUE
frt_sf_get_sort_type(VALUE self)
{
  GET_SF;
  return INT2FIX(sf->type);
}

static VALUE
frt_sf_get_comparator(VALUE self)
{
  return Qnil;
}

static VALUE
frt_sf_to_s(VALUE self)
{
  GET_SF;
  char *str = sort_field_to_s(sf);
  VALUE rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

/****************************************************************************
 *
 * Sort Methods
 *
 ****************************************************************************/

static void 
frt_sort_free(void *p)
{
  Sort *sort = (Sort *)p;
  object_del(sort);
  object_del(sort->sort_fields);
  sort_destroy(sort);
}

static void 
frt_sort_mark(void *p)
{
  Sort *sort = (Sort *)p;
  /*
  int i;
  for (i = 0; i < sort->sf_cnt; i++) {
    frt_get_sf(sort->sort_fields[i]);
  }
  */
  frt_gc_mark(sort->sort_fields);
}

static VALUE
frt_sort_alloc(VALUE klass)
{
  VALUE self;
  Sort *sort = sort_create();
  sort->destroy_all = false;
  self = Data_Wrap_Struct(klass, &frt_sort_mark, &frt_sort_free, sort);
  object_add(sort, self);
  return self;
}

static void
frt_sort_add(Sort *sort, VALUE rsf, bool reverse)
{
  SortField *sf;
  switch (TYPE(rsf)) {
    case T_DATA:
      Data_Get_Struct(rsf, SortField, sf);
      if (reverse) sf->reverse = !sf->reverse;
      break;
    case T_SYMBOL:
      rsf = rb_obj_as_string(rsf);
    case T_STRING:
      sf = sort_field_auto_create(RSTRING(rsf)->ptr, reverse);
      /* need to give it a ruby object so it'll be freed when the
       * sort is garbage collected */
      rsf = frt_get_sf(sf);
      break;
    default:
      rb_raise(rb_eArgError, "Unknown SortField Type");
      break;
  }
  sort_add_sort_field(sort, sf);
}

#define GET_SORT Sort *sort = (Sort *)DATA_PTR(self)
static VALUE
frt_sort_init(int argc, VALUE *argv, VALUE self)
{
  int i;
  VALUE rfields, rreverse;
  bool reverse = false;
  bool has_sfd = false;
  GET_SORT;
  switch (rb_scan_args(argc, argv, "02", &rfields, &rreverse)) {
    case 2: reverse = RTEST(rreverse);
    case 1: 
      if (TYPE(rfields) == T_ARRAY) {
        int i;
        for (i = 0; i < RARRAY(rfields)->len; i++) {
          frt_sort_add(sort, RARRAY(rfields)->ptr[i], reverse);
        }
      } else {
        frt_sort_add(sort, rfields, reverse);
      }
      for (i = 0; i < sort->sf_cnt; i++) {
        if (sort->sort_fields[i] == &SORT_FIELD_DOC) has_sfd = true;
      }
      if (!has_sfd) {
        sort_add_sort_field(sort, &SORT_FIELD_DOC);
      }
      break;
    case 0:
      sort_add_sort_field(sort, &SORT_FIELD_SCORE);
      sort_add_sort_field(sort, &SORT_FIELD_DOC);
  }
  rfields = rb_ary_new2(sort->sf_cnt);
  for (i = 0; i < sort->sf_cnt; i++) {
    rb_ary_store(rfields, i, object_get(sort->sort_fields[i]));
  }
  object_add(sort->sort_fields, rfields);
  return self;
}
  
static VALUE
frt_sort_get_fields(VALUE self)
{
  GET_SORT;
  return object_get(sort->sort_fields);
}


static VALUE
frt_sort_to_s(VALUE self)
{
  GET_SORT;
  char *str = sort_to_s(sort);
  VALUE rstr = rb_str_new2(str);
  free(str);
  return rstr;
}
/****************************************************************************
 *
 * Searcher Methods
 *
 ****************************************************************************/

static void
frt_sea_free(void *p)
{
  Searcher *sea = (Searcher *)p;
  object_del(sea);
  sea_close(sea);
}

#define GET_SEA Searcher *sea = (Searcher *)DATA_PTR(self)

static VALUE
frt_sea_close(VALUE self)
{
  GET_SEA;
  Frt_Unwrap_Struct(self);
  sea->close(sea);
  return Qnil;
}

static VALUE
frt_sea_get_reader(VALUE self, VALUE rterm)
{
  GET_SEA;
  return object_get(sea->ir);
}

static VALUE
frt_sea_doc_freq(VALUE self, VALUE rterm)
{
  GET_SEA;
  Term t;
  frt_set_term(rterm, &t);
  return INT2FIX(sea->doc_freq(sea, &t));
}

static VALUE
frt_sea_doc_freqs(VALUE self, VALUE rterms)
{
  int i;
  GET_SEA;
  Term t;
  VALUE freqs;
  Check_Type(rterms, T_ARRAY);

  freqs = rb_ary_new2(RARRAY(rterms)->len);
  for (i = 0; i < RARRAY(rterms)->len; i++) {
    frt_set_term(RARRAY(rterms)->ptr[i], &t);
    rb_ary_store(freqs, i, INT2FIX(sea->doc_freq(sea, &t)));
  }
  return freqs;
}

static VALUE
frt_sea_doc(VALUE self, VALUE rdoc_num)
{
  GET_SEA;
  return frt_get_doc(sea->get_doc(sea, FIX2INT(rdoc_num)));
}

static VALUE
frt_sea_max_doc(VALUE self)
{
  GET_SEA;
  return INT2FIX(sea->max_doc(sea));
}

static TopDocs *
frt_sea_search_internal(Query *query, VALUE roptions, Searcher *sea)
{
  VALUE rval;
  int first_doc = 0, num_docs = 10;
  Filter *filter = NULL;
  Sort *sort = NULL;
  
  if (Qnil != roptions) {
    if (Qnil != (rval = rb_hash_aref(roptions, rfirst_doc_key))) {
      first_doc = FIX2INT(rval);
      if (first_doc < 0)
        rb_raise(rb_eArgError, ":first_doc must be >= 0");
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rnum_docs_key))) {
      num_docs = FIX2INT(rval);
      if (num_docs <= 0)
        rb_raise(rb_eArgError, ":num_docs must be > 0");
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rfilter_key))) {
      Data_Get_Struct(rval, Filter, filter);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rsort_key))) {
      if (TYPE(rval) != T_DATA) {
        rval = frt_sort_init(1, &rval, frt_sort_alloc(cSort));
      }
      Data_Get_Struct(rval, Sort, sort);
    }
  }

  return sea->search(sea, query, first_doc, num_docs, filter, sort);
}

static VALUE
frt_sea_search(int argc, VALUE *argv, VALUE self)
{
  GET_SEA;
  VALUE rquery, roptions;
  Query *query;
  rb_scan_args(argc, argv, "11", &rquery, &roptions);
  Data_Get_Struct(rquery, Query, query);
  return frt_get_td(frt_sea_search_internal(query, roptions, sea));
}

static VALUE
frt_sea_search_each(VALUE self, VALUE rquery, VALUE roptions)
{
  return Qnil;
}

static VALUE
frt_sea_explain(VALUE self, VALUE rquery, VALUE rdoc_num)
{
  GET_SEA;
  Query *query;
  Explanation *expl;
  Data_Get_Struct(rquery, Query, query);
  expl = sea->explain(sea, query, FIX2INT(rdoc_num));
  return Data_Wrap_Struct(cExplanation, NULL, &expl_destoy, expl);
}

/****************************************************************************
 *
 * IndexSearcher Methods
 *
 ****************************************************************************/

static void
frt_is_mark(void *p)
{
  Searcher *sea = (Searcher *)p;
  frt_gc_mark(sea->ir);
  frt_gc_mark(sea->ir->store);
}

#define FRT_GET_IR(rir, ir) do {\
  rir = Data_Wrap_Struct(cIndexReader, &frt_ir_mark, &frt_ir_free, ir);\
  object_add(ir, rir);\
} while (0)

static VALUE
frt_is_init(VALUE self, VALUE obj)
{
  Store *store = NULL;
  IndexReader *ir = NULL;
  Searcher *sea;
  if (TYPE(obj) == T_STRING) {
    store = open_fs_store(StringValueCStr(obj));
    ir = ir_open(store);
    deref(store);
    FRT_GET_IR(obj, ir);
  } else {
    Check_Type(obj, T_DATA);
    if (rb_obj_is_kind_of(obj, cDirectory) == Qtrue) {
      Data_Get_Struct(obj, Store, store);
      ir = ir_open(store);
      FRT_GET_IR(obj, ir);
    } else if (rb_obj_is_kind_of(obj, cIndexReader) == Qtrue) {
      Data_Get_Struct(obj, IndexReader, ir);
    } else {
      rb_raise(rb_eArgError, "Unknown type for argument to IndexSearcher.new");
    }
  }

  sea = sea_create(ir);
  sea->close_ir = false;
  Frt_Wrap_Struct(self, &frt_is_mark, &frt_sea_free, sea);
  object_add(sea, self);
  return self;
}

/****************************************************************************
 *
 * MultiSearcher Methods
 *
 ****************************************************************************/

static void
frt_ms_free(void *p)
{
  Searcher *sea = (Searcher *)p;
  MultiSearcher *msea = (MultiSearcher *)sea->data;
  free(msea->searchers);
  object_del(sea);
  sea_close(sea);
}

static void
frt_ms_mark(void *p)
{
  int i;
  Searcher *sea = (Searcher *)p;
  MultiSearcher *msea = (MultiSearcher *)sea->data;
  for (i = 0; i < msea->s_cnt; i++) {
    frt_gc_mark(msea->searchers[i]);
  }
}

static VALUE
frt_ms_init(int argc, VALUE *argv, VALUE self)
{
  int i, j;

  VALUE rsearcher;
  Array *searchers = ary_create(argc, (free_ft)NULL);
  Searcher *s;

  for (i = 0; i < argc; i++) {
    rsearcher = argv[i];
    switch (TYPE(rsearcher)) {
      case T_ARRAY:
        for (j = 0; j < RARRAY(rsearcher)->len; j++) {
          VALUE rs = RARRAY(rsearcher)->ptr[j];
          Data_Get_Struct(rs, Searcher, s);
          ary_append(searchers, s);
        }
      break;
      case T_DATA:
        Data_Get_Struct(rsearcher, Searcher, s);
        ary_append(searchers, s);
        break;
      default:
        rb_raise(rb_eArgError, "Can't add class %s to MultiSearcher",
            rb_obj_classname(rsearcher));
        break;
    }
  }
  s = msea_create((Searcher **)searchers->elems, searchers->size, false);
  free(searchers); /* only free the Array, not the elems array holding the searchers */
  Frt_Wrap_Struct(self, &frt_ms_mark, &frt_ms_free, s);
  object_add(s, self);
  return self;
}

/****************************************************************************
 *
 * Index Methods
 *
 ****************************************************************************/

/*
static void
frt_ind_free_store_i(Index *self)
{
  VALUE rval;
  if (self->close_store && (Qnil != (rval = object_get(self->store)))) {
    // user passed close_dir option so unwrap it 
    Frt_Unwrap_Struct(rval);
    object_del(self->store);
  }
}
*/

static void
frt_ind_free(void *p)
{
  Index *ind = (Index *)p;
  object_del(ind);
  index_destroy(ind);
}

static void
frt_ind_mark(void *p)
{
  Index *ind = (Index *)p;
  frt_gc_mark(ind->store);
  frt_gc_mark(ind->analyzer);
}

static VALUE
frt_ind_init(int argc, VALUE *argv, VALUE self)
{
  VALUE roptions;
  Index *ind;
  if (rb_scan_args(argc, argv, "01", &roptions)) {
    VALUE rval;
    Store *store = NULL;
    Analyzer *analyzer = NULL;
    bool create = false;
    HashSet *def_fields = NULL;

    if (Qnil != (rval = rb_hash_aref(roptions, rpath_key))) {
      rval = rb_obj_as_string(rval);
      /* TODO: create the directory if it is missing */
      store = open_fs_store(RSTRING(rval)->ptr);
      deref(store);
    } else if (Qnil != (rval = rb_hash_aref(roptions, rdir_key))) {
      Data_Get_Struct(rval, Store, store);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rcreate_key))) {
      create = RTEST(rval);
    }
    if (store) {
      if (!store->exists(store, "segments")) {
        if (Qfalse == rb_hash_aref(roptions, rcreate_if_missing_key)) {
          rb_raise(rb_eStandardError, "Index does not exist. Set either "
              ":create or :create_if_missing to true");
        } else {
          /* if create_if_missing and segments doesn't exist, create := true */
          create = true;
        }
      }
    } else {
      create = true;
    }

    if (Qnil != (rval = rb_hash_aref(roptions, ranalyzer_key))) {
      analyzer = frt_get_cwrapped_analyzer(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rdefault_search_field_key))) {
      def_fields = frt_get_fields(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rclose_dir_key))) {
      /* No need to do anything here. Let the GC do the work. 
       * if (RTEST(rval) && !close_store) close_store = true;
       */
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rdefault_field_key))) {
      if (!def_fields) def_fields = frt_get_fields(rval);
    }
    ind = index_create(store, analyzer, def_fields, create);
    if (analyzer) a_deref(analyzer);
    
    /* QueryParser options */
    if (Qnil != (rval = rb_hash_aref(roptions, rhandle_parse_errors_key))) {
      ind->qp->handle_parse_errors = RTEST(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rallow_any_fields_key))) {
      ind->qp->allow_any_fields = RTEST(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rwild_lower_key))) {
      ind->qp->wild_lower = RTEST(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, roccur_default_key))) {
      ind->qp->or_default = (FIX2INT(rval) == BC_MUST) ? false : true;
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rdefault_slop_key))) {
      ind->qp->def_slop = FIX2INT(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rclean_str_key))) {
      ind->qp->clean_str = RTEST(rval);
    }

    /* IndexWriter options */
    if (Qnil != (rval = rb_hash_aref(roptions, ruse_compound_file_key))) {
      ind->use_compound_file = RTEST(rval);
      if (ind->iw) ind->iw->use_compound_file = ind->use_compound_file;
    }

    /* other options */
    if (Qnil != (rval = rb_hash_aref(roptions, rkey_key))) {
      ind->key = frt_get_fields(rval);
    }
    /* by default id_field and def_field are the same but they can differ */
    if (Qnil != (rval = rb_hash_aref(roptions, rid_field_key))) {
      ind->id_field = estrdup(RSTRING(rb_obj_as_string(rval))->ptr);
    } else if (Qnil != (rval = rb_hash_aref(roptions, rdefault_field_key))) {
      ind->id_field = estrdup(RSTRING(rb_obj_as_string(rval))->ptr);
    } else if (ind->key && (ind->key->size == 1)) {
      /* if neither are set and key has only one field then use that field */
      ind->id_field = estrdup(ind->key->elems[0]);
      ind->def_field = estrdup(ind->key->elems[0]);
    }

    if (Qnil != (rval = rb_hash_aref(roptions, rdefault_field_key))) {
      ind->def_field = estrdup(RSTRING(rb_obj_as_string(rval))->ptr);
    } else if (Qnil != (rval = rb_hash_aref(roptions, rid_field_key))) {
      ind->def_field = estrdup(RSTRING(rb_obj_as_string(rval))->ptr);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rauto_flush_key))) {
      ind->auto_flush = RTEST(rval);
    }
    if (Qnil != (rval = rb_hash_aref(roptions, rcheck_latest_key))) {
      ind->check_latest = RTEST(rval);
    }

  } else {
    ind = index_create(NULL, NULL, NULL, true);
  }
  Frt_Wrap_Struct(self, &frt_ind_mark, &frt_ind_free, ind);
  object_add(ind, self);
  return self;
}

#define GET_IND Index *ind = (Index *)DATA_PTR(self);\
  if (!ind) rb_raise(rb_eStandardError, "Called method on closed Index object")
static VALUE
frt_ind_close(VALUE self)
{
  GET_IND;
  //frt_ind_free_store_i(ind);
  Frt_Unwrap_Struct(self);
  object_del(ind);
  index_destroy(ind);
  return Qnil;
}

typedef struct HashToDocArg {
  Document *doc;
  Index *ind;
} HashToDocArg;

static int
frt_hash_to_doc_i(VALUE key, VALUE value, HashToDocArg *htda)
{
  VALUE rfield, rdata;
  if (key == Qundef) return ST_CONTINUE;
  rfield = rb_obj_as_string(key);
  rdata = rb_obj_as_string(value);
  if (htda->ind->key && hs_exists(htda->ind->key, RSTRING(rfield)->ptr)) {
    doc_add_field(htda->doc, df_create(RSTRING(rfield)->ptr,
          estrdup(RSTRING(rdata)->ptr),
          DF_STORE_YES, DF_INDEX_UNTOKENIZED, DF_TERM_VECTOR_NO));
  } else {
    doc_add_field(htda->doc, df_create(RSTRING(rfield)->ptr,
          estrdup(RSTRING(rdata)->ptr),
          DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
  }
  return ST_CONTINUE;
}

static Document *
frt_rdoc_to_doc(Index *ind, VALUE rdoc, bool *close_doc)
{
  Document *doc = NULL;
  VALUE rstr;
  int i;
  HashToDocArg htda;

  switch (TYPE(rdoc)) {
    case T_SYMBOL:
      rdoc = rb_obj_as_string(rdoc);
    case T_STRING:
      doc = doc_create();
      *close_doc = true;
      doc_add_field(doc, df_create(ind->def_field, estrdup(RSTRING(rdoc)->ptr),
            DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
      *close_doc = true;
      break;
    case T_ARRAY:
      doc = doc_create();
      *close_doc = true;
      for (i = 0; i < RARRAY(rdoc)->len; i++) {
        rstr = rb_obj_as_string(RARRAY(rdoc)->ptr[i]);
        doc_add_field(doc, df_create(ind->def_field, estrdup(RSTRING(rstr)->ptr),
              DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
      }
      break;
    case T_HASH:
      htda.doc = doc = doc_create();
      htda.ind = ind;
      *close_doc = true;
      rb_hash_foreach(rdoc, frt_hash_to_doc_i, (VALUE)&htda);
      break;
    case T_DATA:
      Data_Get_Struct(rdoc, Document, doc);
      break;
    default:
      rb_raise(rb_eArgError, "Can't add class %s to an Index", rb_obj_classname(rdoc));
      break;
  }
  return doc;
}

static VALUE
frt_ind_add_doc(int argc, VALUE *argv, VALUE self)
{
  VALUE rdoc, ranalyzer;
  Document *doc;
  bool close_doc = false;
  GET_IND;
  rb_scan_args(argc, argv, "11", &rdoc, &ranalyzer);

  doc = frt_rdoc_to_doc(ind, rdoc, &close_doc);

  if (argc == 2) {
    Analyzer *analyzer = frt_get_cwrapped_analyzer(ranalyzer);
    index_add_doc_a(ind, doc, analyzer);
    a_deref(analyzer);
  } else {
    index_add_doc(ind, doc);
  }
  if (close_doc) doc_destroy(doc);
  return self;
}

static Query *
frt_get_query_i(Index *ind, VALUE rquery)
{
  Query *q = NULL;

  switch (TYPE(rquery)) {
    case T_SYMBOL:
      rquery = rb_obj_as_string(rquery);
    case T_STRING:
      q = index_get_query(ind, RSTRING(rquery)->ptr);
      break;
    case T_DATA:
      Data_Get_Struct(rquery, Query, q);
      ref(q);
      break;
    default:
      rb_raise(rb_eArgError, "Can only handle a String or a Query.");
      break;
  }
  //printf(">>>>>%s<<<<<\n", q->to_s(q, "file_name"));
  return q;
}

static VALUE
frt_ind_search(int argc, VALUE *argv, VALUE self)
{
  Query *q;
  VALUE rquery, roptions, rtd;
  GET_IND;
  rb_scan_args(argc, argv, "11", &rquery, &roptions);
  ensure_searcher_open(ind);

  q = frt_get_query_i(ind, rquery);
  rtd = frt_get_td(frt_sea_search_internal(q, roptions, ind->sea));
  q_deref(q);

  return rtd;
}

static VALUE
frt_ind_search_each(int argc, VALUE *argv, VALUE self)
{
  int i;
  Query *q;
  TopDocs *td;
  VALUE rquery, roptions, rtotal_hits;
  GET_IND;


  rb_scan_args(argc, argv, "11", &rquery, &roptions);

  rb_thread_critical = Qtrue;

  ensure_searcher_open(ind);

  q = frt_get_query_i(ind, rquery);
  //printf(">>>>>%s<<<<<\n", q->to_s(q, "file_name"));
  td = frt_sea_search_internal(q, roptions, ind->sea);
  q_deref(q);

  rtotal_hits = INT2FIX(td->total_hits);

  for (i = 0; i < td->size; i++) {
    rb_yield_values(2, INT2FIX(td->hits[i]->doc),
        rb_float_new(td->hits[i]->score));
  }

  rb_thread_critical = 0;

  td_destroy(td);

  return rtotal_hits;
}

static Document *
frt_ind_doc_i(VALUE self, VALUE rid)
{
  Document *doc = NULL;
  Term t;
  GET_IND;
  switch (TYPE(rid)) {
    case T_FIXNUM:
      doc = index_get_doc(ind, FIX2INT(rid));
      break;
    case T_SYMBOL:
      rid = rb_obj_as_string(rid);
    case T_STRING:
      doc = index_get_doc_id(ind, RSTRING(rid)->ptr);
      break;
    case T_DATA:
      frt_set_term(rid, &t);
      doc = index_get_doc_term(ind, &t);
      break;
    default:
      rb_raise(rb_eArgError, "cannot find id of type %s", rb_obj_classname(rid));
      break;
  }
  return doc;
}

static VALUE
frt_ind_doc(VALUE self, VALUE rid)
{
  return frt_get_doc(frt_ind_doc_i(self, rid));
}

static VALUE
frt_ind_delete(VALUE self, VALUE rid)
{
  Term t;
  GET_IND;
  switch (TYPE(rid)) {
    case T_FIXNUM:
      index_delete(ind, FIX2INT(rid));
      break;
    case T_SYMBOL:
      rid = rb_obj_as_string(rid);
    case T_STRING:
      index_delete_id(ind, RSTRING(rid)->ptr);
      break;
    case T_DATA:
      frt_set_term(rid, &t);
      index_delete_term(ind, &t);
      break;
    default:
      rb_raise(rb_eArgError, "cannot delete id of type %s", rb_obj_classname(rid));
      break;
  }

  return INT2FIX(1);
}

static VALUE
frt_ind_query_delete(VALUE self, VALUE rquery)
{
  Query *q;
  GET_IND;
  switch (TYPE(rquery)) {
    case T_SYMBOL:
      rquery = rb_obj_as_string(rquery);
    case T_STRING:
      index_delete_query_str(ind, RSTRING(rquery)->ptr, NULL);
      break;
    case T_DATA:
      Data_Get_Struct(rquery, Query, q);
      index_delete_query(ind, q, NULL);
      break;
    default:
      rb_raise(rb_eArgError, "Can only handle a String or a Query.");
      break;
  }

  return self;
}

static VALUE
frt_ind_is_deleted(VALUE self, VALUE rdoc_num)
{
  GET_IND;
  return index_is_deleted(ind, FIX2INT(rdoc_num)) ? Qtrue : Qfalse;
}

static int
frt_ind_get_doc_num_i(Index *ind, VALUE rid)
{
  Term t;
  int doc_num = -1;
  switch (TYPE(rid)) {
    case T_FIXNUM:
      doc_num = FIX2INT(rid);
      break;
    case T_SYMBOL:
      rid = rb_obj_as_string(rid);
    case T_STRING:
      t.field = ind->id_field;
      t.text = RSTRING(rid)->ptr;
      doc_num = index_term_id(ind, &t);
      break;
    case T_DATA:
      frt_set_term(rid, &t);
      doc_num = index_term_id(ind, &t);
      break;
    default:
      rb_raise(rb_eArgError, "cannot find id of type %s", rb_obj_classname(rid));
      break;
  }
  return doc_num;
}


static int
frt_hash_update_doc_i(VALUE key, VALUE value, Document *doc)
{
  VALUE rfield, rdata;
  if (key == Qundef) return ST_CONTINUE;
  rfield = rb_obj_as_string(key);
  rdata = rb_obj_as_string(value);
  doc_delete_fields(doc, RSTRING(rfield)->ptr);
  doc_add_field(doc, df_create(RSTRING(rfield)->ptr, estrdup(RSTRING(rdata)->ptr),
        DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
  return ST_CONTINUE;
}

static Document *
frt_rdoc_update_doc(Index *ind, VALUE rdoc, Document *doc)
{
  int i;
  VALUE rstr;
  Document *odoc;
  switch (TYPE(rdoc)) {
    case T_SYMBOL:
      rdoc = rb_obj_as_string(rdoc);
    case T_STRING:
      doc_delete_fields(doc, ind->def_field);
      doc_add_field(doc, df_create(ind->def_field, estrdup(RSTRING(rdoc)->ptr),
            DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
      break;
    case T_ARRAY:
      doc_delete_fields(doc, ind->def_field);
      for (i = 0; i < RARRAY(rdoc)->len; i++) {
        rstr = rb_obj_as_string(RARRAY(rdoc)->ptr[i]);
        doc_add_field(doc, df_create(ind->def_field, estrdup(RSTRING(rstr)->ptr),
              DF_STORE_YES, DF_INDEX_TOKENIZED, DF_TERM_VECTOR_NO));
      }
      break;
    case T_HASH:
      rb_hash_foreach(rdoc, frt_hash_update_doc_i, (VALUE)doc);
      break;
    case T_DATA:
      Data_Get_Struct(rdoc, Document, odoc);
      for (i = 0; i < odoc->fcnt; i++) {
        int j;
        char *field;
        Array *dfs = odoc->field_arr[i];
        field = ((DocField *)dfs->elems[0])->name;
        doc_delete_fields(doc, field);
        for (j = 0; j < dfs->size; j++) {
          doc_add_field(doc, df_clone((DocField *)dfs->elems[j]));
        }
      }
      break;
    default:
      rb_raise(rb_eArgError, "Can't add class %s to an Index", rb_obj_classname(rdoc));
      break;
  }
  return doc;
}

static VALUE
frt_ind_update(VALUE self, VALUE rid, VALUE rdoc)
{
  int doc_num;
  GET_IND;
  doc_num = frt_ind_get_doc_num_i(ind, rid);
  if (doc_num >= 0) {
    Document *doc = index_get_doc(ind, doc_num);
    doc = frt_rdoc_update_doc(ind, rdoc, doc);
    index_delete(ind, doc_num);
    index_add_doc(ind, doc);
    doc_destroy(doc);
    index_auto_flush_iw(ind);
  }
  return self;
}

struct QueryUpdateArg {
  Array *docs;
  VALUE rdoc;
  Index *ind;
};

static void frt_ind_qupd_i(Searcher *sea, int doc_num, float score, void *arg)
{
  struct QueryUpdateArg *qua = (struct QueryUpdateArg *)arg;
  Document *doc = sea->ir->get_doc(sea->ir, doc_num);
  ir_delete_doc(sea->ir, doc_num);
  doc = frt_rdoc_update_doc(qua->ind, qua->rdoc, doc);
  ary_append(qua->docs, doc);
}

static VALUE
frt_ind_query_update(VALUE self, VALUE rquery, VALUE rdoc)
{
  int i;
  Query *q;
  struct QueryUpdateArg qua;
  GET_IND;

  ensure_searcher_open(ind);  
  qua.rdoc = rdoc;
  qua.docs = ary_create(8, (free_ft)&doc_destroy);
  qua.ind = ind;

  q = frt_get_query_i(ind, rquery);
  sea_search_each(ind->sea, q, NULL, &frt_ind_qupd_i, &qua);
  q_deref(q);
  
  for (i = 0; i < qua.docs->size; i++) {
    index_add_doc(ind, qua.docs->elems[i]);
  }

  ary_destroy(qua.docs);

  index_auto_flush_ir(ind);

  return self;
}

static VALUE
frt_ind_has_deletions(VALUE self)
{
  GET_IND;
  return index_has_del(ind) ? Qtrue : Qfalse;
}

static VALUE
frt_ind_has_writes(VALUE self)
{
  GET_IND;
  return ind->has_writes ? Qtrue : Qfalse;
}

static VALUE
frt_ind_flush(VALUE self)
{
  GET_IND;
  index_flush(ind);
  return Qnil;
}

static VALUE
frt_ind_optimize(VALUE self)
{
  GET_IND;
  index_optimize(ind);
  return Qnil;
}

static VALUE
frt_ind_size(VALUE self)
{
  GET_IND;
  return INT2FIX(index_size(ind));
}

#define INCONSISTANT_TYPES_MSG "Inconsistant arguments. All members "\
            "of indexes array must be of the same type"
static void
frt_ind_add_indexes_i(Index *ind, VALUE *indexes, int cnt)
{
  int i;
  if (cnt == 0) return;
  Check_Type(indexes[0], T_DATA);
  if (rb_obj_is_kind_of(indexes[0], cDirectory) == Qtrue) {
    Store **stores = ALLOC_N(Store *, cnt);
    for (i = 0; i < cnt; i++) {
      if (rb_obj_is_kind_of(indexes[i], cDirectory) != Qtrue)
        rb_raise(rb_eArgError, INCONSISTANT_TYPES_MSG);
      Data_Get_Struct(indexes[i], Store, stores[i]);
    }
    ensure_writer_open(ind);
    iw_add_indexes(ind->iw, stores, cnt);
    free(stores);
  } else if (rb_obj_is_kind_of(indexes[0], cIndexReader) == Qtrue) {
    IndexReader **readers = ALLOC_N(IndexReader *, cnt);
    for (i = 0; i < cnt; i++) {
      if (rb_obj_is_kind_of(indexes[i], cIndexReader) != Qtrue)
        rb_raise(rb_eArgError, INCONSISTANT_TYPES_MSG);
      Data_Get_Struct(indexes[i], IndexReader, readers[i]);
    }
    ensure_writer_open(ind);
    iw_add_readers(ind->iw, readers, cnt);
    free(readers);
  } else if (CLASS_OF(indexes[0]) == cIndex) {
    Index *index;
    IndexReader **readers = ALLOC_N(IndexReader *, cnt);
    for (i = 0; i < cnt; i++) {
      if (CLASS_OF(indexes[i]) != cIndex)
        rb_raise(rb_eArgError, INCONSISTANT_TYPES_MSG);
      Data_Get_Struct(indexes[i], Index, index);
      ensure_reader_open(index);
      readers[i] = index->ir;
    }
    ensure_writer_open(ind);
    iw_add_readers(ind->iw, readers, cnt);
    free(readers);
  } else {
    rb_raise(rb_eArgError, "can't add class of type %s to index",
        rb_obj_classname(indexes[0]));
  }

  index_auto_flush_iw(ind);
}

static VALUE
frt_ind_add_indexes(VALUE self, VALUE rindexes)
{
  GET_IND;
  switch (TYPE(rindexes)) {
    case T_ARRAY:
      frt_ind_add_indexes_i(ind, RARRAY(rindexes)->ptr, RARRAY(rindexes)->len);
      break;
    default:  
      frt_ind_add_indexes_i(ind, &rindexes, 1);
      break;
  }
  return self;
}

static VALUE
frt_ind_persist(int argc, VALUE *argv, VALUE self)
{
  VALUE rdir, rcreate;
  bool create;
  Store *old_store;
  GET_IND;

  index_flush(ind);
  //frt_ind_free_store_i(ind);
  old_store = ind->store;

  rb_scan_args(argc, argv, "11", &rdir, &rcreate);
  create = RTEST(rcreate);

  if (T_DATA == TYPE(rdir)) {
    Data_Get_Struct(rdir, Store, ind->store);
    ref(ind->store);
  } else {
    rdir = rb_obj_as_string(rdir);
    ind->store = open_fs_store(RSTRING(rdir)->ptr);
  }

  if (!create && !ind->store->exists(ind->store, "segments")) create = true;

  if (create) {
    ind->iw = iw_open(ind->store, ind->analyzer, create);
    ref(ind->analyzer);
    ind->iw->use_compound_file = ind->use_compound_file;
  }

  ensure_writer_open(ind);
  iw_add_indexes(ind->iw, &old_store, 1);

  store_deref(old_store);

  index_auto_flush_iw(ind);

  return self;
}

static VALUE
frt_ind_explain(VALUE self, VALUE rquery, VALUE rdoc_num)
{
  Query *q;
  Explanation *expl;
  GET_IND;
  q = frt_get_query_i(ind, rquery);
  expl = index_explain(ind, q, FIX2INT(rdoc_num));
  q_deref(q);
  return Data_Wrap_Struct(cExplanation, NULL, &expl_destoy, expl);
}


/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_search(void)
{
  VALUE oSORT_FIELD_DOC;

  /* option hash keys for IndexSearcher#search */
  rfirst_doc_key = ID2SYM(rb_intern("first_doc"));
  rnum_docs_key  = ID2SYM(rb_intern("num_docs"));
  rfilter_key    = ID2SYM(rb_intern("filter"));
  rsort_key      = ID2SYM(rb_intern("sort"));


  /* option hash keys for SortField#initialize */
  rsort_type_key  = ID2SYM(rb_intern("sort_type"));
  rreverse_key    = ID2SYM(rb_intern("reverse"));
  rcomparator_key = ID2SYM(rb_intern("comparator"));

  /* option hash keys for Index#initialize */
  rpath_key                 = ID2SYM(rb_intern("path"));
  rcreate_key               = ID2SYM(rb_intern("create"));
  rcreate_if_missing_key    = ID2SYM(rb_intern("create_if_missing"));
  rid_field_key             = ID2SYM(rb_intern("id_field"));
  rdefault_field_key        = ID2SYM(rb_intern("default_field"));
  rdefault_search_field_key = ID2SYM(rb_intern("default_search_field"));
  ranalyzer_key             = ID2SYM(rb_intern("analyzer"));
  rdir_key                  = ID2SYM(rb_intern("dir"));
  rclose_dir_key            = ID2SYM(rb_intern("close_dir"));
  rkey_key                  = ID2SYM(rb_intern("key"));
  ruse_compound_file_key    = ID2SYM(rb_intern("use_compound_file"));
  rhandle_parse_errors_key  = ID2SYM(rb_intern("handle_parse_errors"));
  rauto_flush_key           = ID2SYM(rb_intern("auto_flush"));
  rcheck_latest_key         = ID2SYM(rb_intern("check_latest"));

  /* ids */
  rdefault_min_similarity_id = rb_intern("default_min_similarity");
  rdefault_prefix_length_id = rb_intern("default_prefix_length");

  /* Explanation */
  cExplanation = rb_define_class_under(mSearch, "Explanation", rb_cObject);
  rb_define_alloc_func(cExplanation, frt_data_alloc);

  rb_define_method(cExplanation, "to_s", frt_expl_to_s, 0);
  rb_define_method(cExplanation, "to_html", frt_expl_to_html, 0);
  rb_define_method(cExplanation, "value", frt_expl_value, 0);

  /* ScoreDoc */
  cScoreDoc = rb_define_class_under(mSearch, "ScoreDoc", rb_cObject);
  rb_define_alloc_func(cScoreDoc, frt_data_alloc);

  rb_define_method(cScoreDoc, "score", frt_sd_score, 0);
  rb_define_method(cScoreDoc, "doc", frt_sd_doc, 0);

  /* TopDocs */
  cTopDocs = rb_define_class_under(mSearch, "TopDocs", rb_cObject);
  rb_define_alloc_func(cTopDocs, frt_data_alloc);

  rb_define_method(cTopDocs, "score_docs", frt_td_hits, 0);
  rb_define_method(cTopDocs, "hits", frt_td_hits, 0);
  rb_define_method(cTopDocs, "size", frt_td_size, 0);
  rb_define_method(cTopDocs, "total_hits", frt_td_total_hits, 0);
  rb_define_method(cTopDocs, "fields", frt_td_fields, 0);
  rb_define_method(cTopDocs, "each", frt_td_each, 0);

  /* Query */
  cQuery = rb_define_class_under(mSearch, "Query", rb_cObject);

  rb_define_method(cQuery, "to_s", frt_q_to_s, -1);
  rb_define_method(cQuery, "boost", frt_q_get_boost, 0);
  rb_define_method(cQuery, "boost=", frt_q_set_boost, 1);

  /* TermQuery */
  cTermQuery = rb_define_class_under(mSearch, "TermQuery", cQuery);
  rb_define_alloc_func(cTermQuery, frt_data_alloc);

  rb_define_method(cTermQuery, "initialize", frt_tq_init, 1);

  /* BooleanQueryOccur */
  cBooleanClause = rb_define_class_under(mSearch, "BooleanClause", rb_cObject);
  rb_define_alloc_func(cBooleanClause, frt_data_alloc);
  
  rb_define_method(cBooleanClause, "initialize", frt_bc_init, -1);
  rb_define_method(cBooleanClause, "query", frt_bc_get_query, 0);
  rb_define_method(cBooleanClause, "query=", frt_bc_set_query, 1);
  rb_define_method(cBooleanClause, "required?", frt_bc_is_required, 0);
  rb_define_method(cBooleanClause, "prohibited?", frt_bc_is_prohibited, 0);
  rb_define_method(cBooleanClause, "occur=", frt_bc_set_occur, 1);
  rb_define_method(cBooleanClause, "to_s", frt_bc_to_s, 0);

  /* BooleanQuery */
  cBooleanQuery = rb_define_class_under(mSearch, "BooleanQuery", cQuery);
  rb_define_alloc_func(cBooleanQuery, frt_data_alloc);

  rb_define_method(cBooleanQuery, "initialize", frt_bq_init, -1);
  rb_define_method(cBooleanQuery, "add_query", frt_bq_add_query, -1);
  rb_define_method(cBooleanQuery, "add_clause", frt_bq_add_clause, 1);

  rb_define_const(cBooleanQuery, "MUST", INT2FIX(BC_MUST));
  rb_define_const(cBooleanQuery, "MUST_NOT", INT2FIX(BC_MUST_NOT));
  rb_define_const(cBooleanQuery, "SHOULD", INT2FIX(BC_SHOULD));

  /* BooleanQueryOccur */
  cBCOccur = rb_define_class_under(cBooleanClause, "Occur", cQuery);
  rb_define_const(cBCOccur, "MUST", INT2FIX(BC_MUST));
  rb_define_const(cBCOccur, "MUST_NOT", INT2FIX(BC_MUST_NOT));
  rb_define_const(cBCOccur, "SHOULD", INT2FIX(BC_SHOULD));

  /* RangeQuery */
  cRangeQuery = rb_define_class_under(mSearch, "RangeQuery", cQuery);
  rb_define_alloc_func(cRangeQuery, frt_data_alloc);

  rb_define_method(cRangeQuery, "initialize", frt_rq_init, 5);
  rb_define_singleton_method(cRangeQuery, "new_more", frt_rq_new_more, 3);
  rb_define_singleton_method(cRangeQuery, "new_less", frt_rq_new_less, 3);

  /* PhraseQuery */
  cPhraseQuery = rb_define_class_under(mSearch, "PhraseQuery", cQuery);
  rb_define_alloc_func(cPhraseQuery, frt_phq_alloc);

  rb_define_method(cPhraseQuery, "initialize", frt_phq_init, 0);
  rb_define_method(cPhraseQuery, "add", frt_phq_add, -1);
  rb_define_method(cPhraseQuery, "<<", frt_phq_shift, 1);
  rb_define_method(cPhraseQuery, "slop", frt_phq_get_slop, 0);
  rb_define_method(cPhraseQuery, "slop=", frt_phq_set_slop, 1);

  /* MultiPhraseQuery */
  cMultiPhraseQuery = rb_define_class_under(mSearch, "MultiPhraseQuery", cQuery);
  rb_define_alloc_func(cMultiPhraseQuery, frt_mphq_alloc);

  rb_define_method(cMultiPhraseQuery, "initialize", frt_mphq_init, 0);
  rb_define_method(cMultiPhraseQuery, "add", frt_mphq_add, -1);
  rb_define_method(cMultiPhraseQuery, "<<", frt_mphq_shift, 1);
  rb_define_method(cMultiPhraseQuery, "slop", frt_mphq_get_slop, 0);
  rb_define_method(cMultiPhraseQuery, "slop=", frt_mphq_set_slop, 1);

  /* PrefixQuery */
  cPrefixQuery = rb_define_class_under(mSearch, "PrefixQuery", cQuery);
  rb_define_alloc_func(cPrefixQuery, frt_data_alloc);

  rb_define_method(cPrefixQuery, "initialize", frt_prq_init, 1);

  /* WildCardQuery */
  cWildCardQuery = rb_define_class_under(mSearch, "WildcardQuery", cQuery);
  rb_define_alloc_func(cWildCardQuery, frt_data_alloc);

  rb_define_method(cWildCardQuery, "initialize", frt_wcq_init, 1);

  /* FuzzyQuery */
  cFuzzyQuery = rb_define_class_under(mSearch, "FuzzyQuery", cQuery);
  rb_define_alloc_func(cFuzzyQuery, frt_data_alloc);
  rb_cvar_set(cFuzzyQuery, rdefault_min_similarity_id, rb_float_new(0.5), Qfalse);
  rb_cvar_set(cFuzzyQuery, rdefault_prefix_length_id, INT2FIX(0), Qfalse);

  rb_define_singleton_method(cFuzzyQuery, "default_min_similarity", frt_fq_get_dms, 0);
  rb_define_singleton_method(cFuzzyQuery, "default_min_similarity=", frt_fq_set_dms, 1);
  rb_define_singleton_method(cFuzzyQuery, "default_prefix_length", frt_fq_get_dpl, 0);
  rb_define_singleton_method(cFuzzyQuery, "default_prefix_length=", frt_fq_set_dpl, 1);

  rb_define_method(cFuzzyQuery, "initialize", frt_fq_init, -1);

  /* MatchAllQuery */
  cMatchAllQuery = rb_define_class_under(mSearch, "MatchAllQuery", cQuery);
  rb_define_alloc_func(cMatchAllQuery, frt_maq_alloc);

  rb_define_method(cMatchAllQuery, "initialize", frt_maq_init, 0);

  /* ConstantScoreQuery */
  cConstantScoreQuery = rb_define_class_under(mSearch, "ConstantScoreQuery", cQuery);
  rb_define_alloc_func(cConstantScoreQuery, frt_data_alloc);

  rb_define_method(cConstantScoreQuery, "initialize", frt_csq_init, 1);

  /* FilteredQuery */
  cFilteredQuery = rb_define_class_under(mSearch, "FilteredQuery", cQuery);
  rb_define_alloc_func(cFilteredQuery, frt_data_alloc);

  rb_define_method(cFilteredQuery, "initialize", frt_fqq_init, 2);

  /* SpanTermQuery */
  cSpanTermQuery = rb_define_class_under(mSpans, "SpanTermQuery", cQuery);
  rb_define_alloc_func(cSpanTermQuery, frt_data_alloc);

  rb_define_method(cSpanTermQuery, "initialize", frt_spantq_init, 1);

  /* SpanFirstQuery */
  cSpanFirstQuery = rb_define_class_under(mSpans, "SpanFirstQuery", cQuery);
  rb_define_alloc_func(cSpanFirstQuery, frt_data_alloc);

  rb_define_method(cSpanFirstQuery, "initialize", frt_spanfq_init, 2);

  /* SpanNearQuery */
  cSpanNearQuery = rb_define_class_under(mSpans, "SpanNearQuery", cQuery);
  rb_define_alloc_func(cSpanNearQuery, frt_data_alloc);

  rb_define_method(cSpanNearQuery, "initialize", frt_spannq_init, 3);

  /* SpanOrQuery */
  cSpanOrQuery = rb_define_class_under(mSpans, "SpanOrQuery", cQuery);
  rb_define_alloc_func(cSpanOrQuery, frt_data_alloc);

  rb_define_method(cSpanOrQuery, "initialize", frt_spanoq_init, 1);

  /* SpanNotQuery */
  cSpanNotQuery = rb_define_class_under(mSpans, "SpanNotQuery", cQuery);
  rb_define_alloc_func(cSpanNotQuery, frt_data_alloc);

  rb_define_method(cSpanNotQuery, "initialize", frt_spanxq_init, 2);


  /* Filter */
  cFilter = rb_define_class_under(mSearch, "Filter", rb_cObject);
  rb_define_alloc_func(cConstantScoreQuery, frt_data_alloc);

  rb_define_method(cFilter, "to_s", frt_f_to_s, 0);

  /* RangeFilter */
  cRangeFilter = rb_define_class_under(mSearch, "RangeFilter", cFilter);
  rb_define_alloc_func(cRangeFilter, frt_data_alloc);

  rb_define_method(cRangeFilter, "initialize", frt_rf_init, 5);
  rb_define_singleton_method(cRangeFilter, "new_more", frt_rf_new_more, -1);
  rb_define_singleton_method(cRangeFilter, "new_less", frt_rf_new_less, -1);

  /* QueryFilter */
  cQueryFilter = rb_define_class_under(mSearch, "QueryFilter", cFilter);
  rb_define_alloc_func(cQueryFilter, frt_data_alloc);

  rb_define_method(cQueryFilter, "initialize", frt_qf_init, 1);

  /* SortField */
  cSortField = rb_define_class_under(mSearch, "SortField", rb_cObject);
  rb_define_alloc_func(cSortField, frt_data_alloc);

  rb_define_method(cSortField, "initialize", frt_sf_init, -1);
  rb_define_method(cSortField, "reverse?", frt_sf_is_reverse, 0);
  rb_define_method(cSortField, "name", frt_sf_get_name, 0);
  rb_define_method(cSortField, "sort_type", frt_sf_get_sort_type, 0);
  rb_define_method(cSortField, "comparator", frt_sf_get_comparator, 0);
  rb_define_method(cSortField, "to_s", frt_sf_to_s, 0);

  /* SortType */
  cSortType = rb_define_class_under(cSortField, "SortType", rb_cObject);
  rb_define_alloc_func(cSortType, frt_data_alloc);

  rb_define_const(cSortType, "SCORE", INT2FIX(SORT_TYPE_SCORE));
  rb_define_const(cSortType, "DOC", INT2FIX(SORT_TYPE_DOC));
  rb_define_const(cSortType, "AUTO", INT2FIX(SORT_TYPE_AUTO));
  rb_define_const(cSortType, "STRING", INT2FIX(SORT_TYPE_STRING));
  rb_define_const(cSortType, "INTEGER", INT2FIX(SORT_TYPE_INTEGER));
  rb_define_const(cSortType, "FLOAT", INT2FIX(SORT_TYPE_FLOAT));

  rb_define_const(cSortField, "FIELD_SCORE",
      Data_Wrap_Struct(cSortField, NULL, &frt_deref_free, &SORT_FIELD_SCORE));
  object_add(&SORT_FIELD_SCORE, rb_const_get(cSortField, rb_intern("FIELD_SCORE")));

  rb_define_const(cSortField, "FIELD_SCORE_REV",
      Data_Wrap_Struct(cSortField, NULL, &frt_deref_free, &SORT_FIELD_SCORE_REV));
  object_add(&SORT_FIELD_SCORE_REV,
      rb_const_get(cSortField, rb_intern("FIELD_SCORE_REV")));

  rb_define_const(cSortField, "FIELD_DOC",
      Data_Wrap_Struct(cSortField, NULL, &frt_deref_free, &SORT_FIELD_DOC));

  oSORT_FIELD_DOC = rb_const_get(cSortField, rb_intern("FIELD_DOC"));
  object_add(&SORT_FIELD_DOC, oSORT_FIELD_DOC);

  rb_define_const(cSortField, "FIELD_DOC_REV",
      Data_Wrap_Struct(cSortField, NULL, &frt_deref_free, &SORT_FIELD_DOC_REV));
  object_add(&SORT_FIELD_DOC_REV,
      rb_const_get(cSortField, rb_intern("FIELD_DOC_REV")));

  /* Sort */
  cSort = rb_define_class_under(mSearch, "Sort", rb_cObject);
  rb_define_alloc_func(cSort, frt_sort_alloc);

  rb_define_method(cSort, "initialize", frt_sort_init, -1);
  rb_define_method(cSort, "fields", frt_sort_get_fields, 0);
  rb_define_method(cSort, "to_s", frt_sort_to_s, 0);

  rb_define_const(cSort, "RELEVANCE",
      frt_sort_init(0, NULL, frt_sort_alloc(cSort)));
  rb_define_const(cSort, "INDEX_ORDER",
      frt_sort_init(1, &oSORT_FIELD_DOC, frt_sort_alloc(cSort)));

  /* Searcher */
  cSearcher = rb_define_class_under(mSearch, "Searcher", rb_cObject);
  rb_define_method(cSearcher, "close", frt_sea_close, 0);
  rb_define_method(cSearcher, "reader", frt_sea_get_reader, 0);
  rb_define_method(cSearcher, "doc_freq", frt_sea_doc_freq, 1);
  rb_define_method(cSearcher, "doc_freqs", frt_sea_doc_freqs, 1);
  rb_define_method(cSearcher, "doc", frt_sea_doc, 1);
  rb_define_method(cSearcher, "[]", frt_sea_doc, 1);
  rb_define_method(cSearcher, "max_doc", frt_sea_max_doc, 0);
  rb_define_method(cSearcher, "search", frt_sea_search, -1);
  rb_define_method(cSearcher, "search_each", frt_sea_search_each, 2);
  rb_define_method(cSearcher, "explain", frt_sea_explain, 2);

  /* IndexSearcher */
  cIndexSearcher = rb_define_class_under(mSearch, "IndexSearcher", cSearcher);
  rb_define_alloc_func(cIndexSearcher, frt_data_alloc);
  rb_define_method(cIndexSearcher, "initialize", frt_is_init, 1);

  /* MultiSearcher */
  cMultiSearcher = rb_define_class_under(mSearch, "MultiSearcher", cSearcher);
  rb_define_alloc_func(cMultiSearcher, frt_data_alloc);
  rb_define_method(cMultiSearcher, "initialize", frt_ms_init, -1);


  /* Index */
  cIndex = rb_define_class_under(mIndex, "Index", rb_cObject);
  rb_define_alloc_func(cIndex, frt_data_alloc);

  rb_define_method(cIndex, "initialize", frt_ind_init, -1);
  rb_define_method(cIndex, "close", frt_ind_close, 0);
  rb_define_method(cIndex, "add_document", frt_ind_add_doc, -1);
  rb_define_method(cIndex, "add_doc", frt_ind_add_doc, -1);
  rb_define_method(cIndex, "<<", frt_ind_add_doc, -1);
  rb_define_method(cIndex, "search", frt_ind_search, -1);
  rb_define_method(cIndex, "search_each", frt_ind_search_each, -1);
  rb_define_method(cIndex, "doc", frt_ind_doc, 1);
  rb_define_method(cIndex, "[]", frt_ind_doc, 1);
  rb_define_method(cIndex, "delete", frt_ind_delete, 1);
  rb_define_method(cIndex, "query_delete", frt_ind_query_delete, 1);
  rb_define_method(cIndex, "deleted?", frt_ind_is_deleted, 1);
  rb_define_method(cIndex, "update", frt_ind_update, 2);
  rb_define_method(cIndex, "query_update", frt_ind_query_update, 2);
  rb_define_method(cIndex, "has_deletions?", frt_ind_has_deletions, 0);
  rb_define_method(cIndex, "has_writes?", frt_ind_has_writes, 0);
  rb_define_method(cIndex, "flush", frt_ind_flush, 0);
  rb_define_method(cIndex, "optimize", frt_ind_optimize, 0);
  rb_define_method(cIndex, "size", frt_ind_size, 0);
  rb_define_method(cIndex, "num_docs", frt_ind_size, 0);
  rb_define_method(cIndex, "add_indexes", frt_ind_add_indexes, 1);
  rb_define_method(cIndex, "persist", frt_ind_persist, -1);
  rb_define_method(cIndex, "explain", frt_ind_explain, 2);
}

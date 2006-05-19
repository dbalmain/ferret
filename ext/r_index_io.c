#include "ferret.h"
#include "index.h"

VALUE cTVOffsetInfo;
VALUE cTermVector;
VALUE cTermDocEnum;
VALUE cIndexWriter;
VALUE cIndexReader;
VALUE cTermEnum;

VALUE ranalyzer_key;
VALUE rclose_dir_key;
VALUE rcreate_key;
VALUE rcreate_if_missing_key;
VALUE ruse_compound_file_key;
VALUE rmerge_factor_key;
VALUE rmin_merge_docs_key;
VALUE rmax_merge_docs_key;
VALUE rmax_field_length_key;
VALUE rterm_index_interval_key;

extern void frt_set_term(VALUE rterm, Term *t);
extern VALUE frt_get_rterm(char *field, char *text);
extern Analyzer *frt_get_cwrapped_analyzer(VALUE ranalyzer);

/****************************************************************************
 *
 * TermEnum Methods
 *
 ****************************************************************************/

static void
frt_te_free(void *p)
{
  TermEnum *te = (TermEnum *)p;
  te->close(te);
}

#define GET_TE TermEnum *te = (TermEnum *)DATA_PTR(self)
static VALUE
frt_te_next(VALUE self)
{
  GET_TE;
  return te->next(te) ? Qtrue : Qfalse;
}

static VALUE
frt_te_term(VALUE self)
{
  GET_TE;
  if (!te->tb_curr) return Qnil;
  return frt_get_rterm(te->tb_curr->field, te->tb_curr->text);
}

static VALUE
frt_te_doc_freq(VALUE self)
{
  GET_TE;
  if (!te->tb_curr) return Qnil;
  return INT2FIX(te->ti_curr->doc_freq);
}

static VALUE
frt_te_close(VALUE self)
{
  GET_TE;
  Frt_Unwrap_Struct(self);
  te->close(te);
  return Qnil;
}

static VALUE
frt_te_skip_to(VALUE self, VALUE rterm)
{
  GET_TE;
  Term t;
  frt_set_term(rterm, &t);

  return te_skip_to(te, &t) ? Qtrue : Qfalse;
}

/****************************************************************************
 *
 * TermVectorOffsetInfo Methods
 *
 ****************************************************************************/

void
frt_tvoi_free(void *p)
{
  object_del(p);
  tvoi_destroy(p);
}

static VALUE
frt_tvoi_init(VALUE self, VALUE rstart, VALUE rend)
{
  TVOffsetInfo *tvoi = tvoi_create(FIX2INT(rstart), FIX2INT(rend));
  Frt_Wrap_Struct(self, NULL, &frt_tvoi_free, tvoi);
  object_add(tvoi, self);
  return self;
}

#define GET_TVOI TVOffsetInfo *tvoi = (TVOffsetInfo *)DATA_PTR(self)

static VALUE
frt_tvoi_set_start(VALUE self, VALUE rstart)
{
  GET_TVOI;
  tvoi->start = FIX2INT(rstart);
  return Qnil;
}

static VALUE
frt_tvoi_get_start(VALUE self)
{
  GET_TVOI;
  return INT2FIX(tvoi->start);
}

static VALUE
frt_tvoi_set_end(VALUE self, VALUE rend)
{
  GET_TVOI;
  tvoi->end = FIX2INT(rend);
  return Qnil;
}

static VALUE
frt_tvoi_get_end(VALUE self)
{
  GET_TVOI;
  return INT2FIX(tvoi->end);
}

static VALUE
frt_tvoi_eql(VALUE self, VALUE rother)
{
  GET_TVOI;
  TVOffsetInfo *other;
  if (TYPE(rother) != T_DATA) return Qfalse;
  Data_Get_Struct(rother, TVOffsetInfo, other);

  return ((tvoi->start == other->start) && (tvoi->end == other->end))
    ? Qtrue : Qfalse;
}

static VALUE
frt_tvoi_hash(VALUE self, VALUE rother)
{
  GET_TVOI;
  return INT2FIX(29 * tvoi->start + tvoi->end);
}

static VALUE
frt_tvoi_to_s(VALUE self)
{
  char buf[60];
  GET_TVOI;
  sprintf(buf, "TermVectorOffsetInfo(%d:%d)", tvoi->start, tvoi->end);
  return rb_str_new2(buf);
}

/****************************************************************************
 *
 * TermVector Methods
 *
 ****************************************************************************/

void
frt_tv_free(void *p)
{
  int i;
  TermVector *tv = (TermVector *)p;
  for (i = 0; i < tv->tcnt; i++) {
    free(tv->terms[i]);
  }
  free(tv->terms);
  if (tv->positions) {
    for (i = 0; i < tv->tcnt; i++) {
      free(tv->positions[i]);
    }
    free(tv->positions);
  }
  if (tv->offsets) {
    for (i = 0; i < tv->tcnt; i++) {
      free(tv->offsets[i]);
    }
    free(tv->offsets);
  }
  free(tv->freqs);
  object_del(p);
  free(p);
}

void
frt_tv_mark(void *p)
{
  int i, j;
  TermVector *tv = (TermVector *)p;
  if (tv->offsets != NULL) {
    for (i = 0; i < tv->tcnt; i++) {
      for (j = 0; j < tv->freqs[i]; j++) {
        frt_gc_mark(tv->offsets[i][j]);
      }
    }
  }
}
  
static VALUE
frt_get_tv(TermVector *tv)
{
  VALUE self = Qnil;
  if (tv) {
    self = object_get(tv);
    if (self == Qnil) {
      self = Data_Wrap_Struct(cTermVector, &frt_tv_mark, &frt_tv_free, tv);
      if (tv->offsets) {
        TVOffsetInfo *tvoi;
        VALUE rtvoi;
        int i, j;
        for (i = 0; i < tv->tcnt; i++) {
          for (j = 0; j < tv->freqs[i]; j++) {
            tvoi = tv->offsets[i][j];
            if (object_get(tvoi) == Qnil) {
              rtvoi = Data_Wrap_Struct(cTVOffsetInfo, NULL, &frt_tvoi_free, tvoi);
              object_add(tvoi, rtvoi);
            }
          }
        }
      }
      object_add(tv, self);
    }
  }
  return self;
}

#define GET_TV TermVector *tv = (TermVector *)DATA_PTR(self)

static VALUE
frt_tv_get_field(VALUE self)
{
  GET_TV;
  return rb_str_new2(tv->field);
}

static VALUE
frt_tv_get_terms(VALUE self)
{
  int i;
  GET_TV;
  VALUE rterms = rb_ary_new2(tv->tcnt);
  for (i = 0; i < tv->tcnt; i++) {
    rb_ary_push(rterms, rb_str_new2(tv->terms[i]));
  }
  return rterms;
}

static VALUE
frt_tv_get_freqs(VALUE self)
{
  int i;
  GET_TV;
  VALUE rfreqs = rb_ary_new2(tv->tcnt);
  for (i = 0; i < tv->tcnt; i++) {
    rb_ary_push(rfreqs, INT2FIX(tv->freqs[i]));
  }
  return rfreqs;
}

static VALUE
frt_tv_get_positions(VALUE self)
{
  int i, j, freq;
  GET_TV;
  VALUE rpositions, rpositionss;

  if (!tv->positions) return Qnil;
  rpositionss = rb_ary_new2(tv->tcnt);
  for (i = 0; i < tv->tcnt; i++) {
    freq = tv->freqs[i];
    rpositions = rb_ary_new2(freq);
    for (j = 0; j < freq; j++) {
      rb_ary_push(rpositions, INT2FIX(tv->positions[i][j]));
    }
    rb_ary_push(rpositionss, rpositions);
  }
  return rpositionss;
}

static VALUE
frt_tv_get_offsets(VALUE self)
{
  int i, j, freq;
  GET_TV;
  VALUE roffsetss, roffsets, roffset;
  if (!tv->offsets) return Qnil;
  roffsetss = rb_ary_new2(tv->tcnt);

  for (i = 0; i < tv->tcnt; i++) {
    freq = tv->freqs[i];
    roffsets = rb_ary_new2(freq);
    for (j = 0; j < freq; j++) {
      roffset = object_get(tv->offsets[i][j]);
      rb_ary_push(roffsets, roffset);
    }
    rb_ary_push(roffsetss, roffsets);
  }
  return roffsetss;
}

/****************************************************************************
 *
 * TermDocEnum Methods
 *
 ****************************************************************************/

void
frt_tde_free(void *p)
{
  TermDocEnum *tde = (TermDocEnum *)p;
  tde->close(tde);
}

static VALUE
frt_get_tde(TermDocEnum *tde)
{
  return Data_Wrap_Struct(cTermDocEnum, NULL, &frt_tde_free, tde);
}

#define GET_TDE TermDocEnum *tde = (TermDocEnum *)DATA_PTR(self)

static VALUE
frt_tde_close(VALUE self)
{
  GET_TDE;
  Frt_Unwrap_Struct(self);
  tde->close(tde);
  return Qnil;
}

static VALUE
frt_tde_seek(VALUE self, VALUE rterm)
{
  GET_TDE;
  Term t;
  frt_set_term(rterm, &t);
  tde->seek(tde, &t);
  return Qnil;
}

static VALUE
frt_tde_doc(VALUE self)
{
  GET_TDE;
  return INT2FIX(tde->doc_num(tde));
}

static VALUE
frt_tde_freq(VALUE self)
{
  GET_TDE;
  return INT2FIX(tde->freq(tde));
}

static VALUE
frt_tde_next(VALUE self)
{
  GET_TDE;
  return tde->next(tde) ? Qtrue : Qfalse;
}

static VALUE
frt_tde_next_position(VALUE self)
{
  GET_TDE;
  return INT2FIX(tde->next_position(tde));
}

static VALUE
frt_tde_read(VALUE self, VALUE rdocs, VALUE rfreqs)
{
  int i, req_num, cnt;
  GET_TDE;
  Check_Type(rdocs, T_ARRAY);
  Check_Type(rfreqs, T_ARRAY);
  req_num = MIN(RARRAY(rdocs)->len, RARRAY(rfreqs)->len);
  cnt = tde->read(tde, (int *)RARRAY(rdocs)->ptr,
      (int *)RARRAY(rfreqs)->ptr, req_num);
  for (i = 0; i < cnt; i++) {
    RARRAY(rdocs)->ptr[i] = INT2FIX(RARRAY(rdocs)->ptr[i]);
    RARRAY(rfreqs)->ptr[i] = INT2FIX(RARRAY(rfreqs)->ptr[i]);
  }
  return INT2FIX(cnt);
}

static VALUE
frt_tde_skip_to(VALUE self, VALUE rtarget)
{
  GET_TDE;
  return tde->skip_to(tde, FIX2INT(rtarget)) ? Qtrue : Qfalse;
}

/****************************************************************************
 *
 * IndexWriter Methods
 *
 ****************************************************************************/

void
frt_iw_free(void *p)
{
  IndexWriter *iw = (IndexWriter *)p;
  iw_close(iw);
}

void
frt_iw_mark(void *p)
{
  IndexWriter *iw = (IndexWriter *)p;
  frt_gc_mark(iw->analyzer);
  frt_gc_mark(iw->store);
}

#define SET_INT_ATTR(attr) \
  if (RTEST(rval = rb_hash_aref(roptions, r##attr##_key)))\
    iw->attr = FIX2INT(rval);

static VALUE
frt_iw_init(int argc, VALUE *argv, VALUE self)
{
  VALUE rdir, roptions, rval;
  bool create = false;
  bool use_compound_file = true;
  Store *store;
  Analyzer *analyzer = NULL;
  IndexWriter *iw;
  rb_scan_args(argc, argv, "02", &rdir, &roptions);
  if (argc > 0) {
    if (TYPE(rdir) == T_DATA) {
      store = DATA_PTR(rdir);
      ref(store);
    } else {
      StringValue(rdir);
      frt_create_dir(rdir);
      store = open_fs_store(RSTRING(rdir)->ptr);
    }
  } else {
    store = open_ram_store();
  }
  if (argc == 2) {
    Check_Type(roptions, T_HASH);
    /* Let ruby's GC handle the closing of the store
    if (!close_dir) {
      close_dir = RTEST(rb_hash_aref(roptions, rclose_dir_key));
    }
    */
    /* use_compound_file defaults to true */
    use_compound_file = 
      (rb_hash_aref(roptions, ruse_compound_file_key) == Qfalse) ? false : true;

    rval = rb_hash_aref(roptions, ranalyzer_key);
    if (rval == Qnil) {
      analyzer = mb_standard_analyzer_create(true);
    } else {
      analyzer = frt_get_cwrapped_analyzer(rval);
    }
    create = RTEST(rb_hash_aref(roptions, rcreate_key));
    if (!create && RTEST(rb_hash_aref(roptions, rcreate_if_missing_key))) {
      if (!store->exists(store, "segments")) {
        create = true;
      }
    }
  }
  iw = iw_open(store, analyzer, create);
  store_deref(store);
  iw->use_compound_file = use_compound_file;

  SET_INT_ATTR(merge_factor);
  SET_INT_ATTR(min_merge_docs);
  SET_INT_ATTR(max_merge_docs);
  SET_INT_ATTR(max_field_length);
  SET_INT_ATTR(term_index_interval);

  Frt_Wrap_Struct(self, &frt_iw_mark, &frt_iw_free, iw);
  return self;
}

#define GET_IW IndexWriter *iw = (IndexWriter *)DATA_PTR(self)

static VALUE
frt_iw_close(VALUE self)
{
  GET_IW;
  Frt_Unwrap_Struct(self);
  iw_close(iw);
  return Qnil;
}

static VALUE
frt_iw_add_doc(VALUE self, VALUE rdoc)
{
  GET_IW;
  Document *doc;
  Data_Get_Struct(rdoc, Document, doc);
  iw_add_doc(iw, doc);
  return Qnil;
}

static VALUE
frt_iw_set_merge_factor(VALUE self, VALUE val)
{
  GET_IW;
  iw->merge_factor = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_iw_set_min_merge_docs(VALUE self, VALUE val)
{
  GET_IW;
  iw->min_merge_docs = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_iw_set_max_merge_docs(VALUE self, VALUE val)
{
  GET_IW;
  iw->max_merge_docs = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_iw_set_max_field_length(VALUE self, VALUE val)
{
  GET_IW;
  iw->max_field_length = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_iw_set_term_index_interval(VALUE self, VALUE val)
{
  GET_IW;
  iw->term_index_interval = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_iw_set_use_compound_file(VALUE self, VALUE val)
{
  GET_IW;
  iw->use_compound_file = FIX2INT(val);
  return Qnil;
}

static VALUE
frt_iw_get_doc_count(VALUE self)
{
  GET_IW;
  return INT2FIX(iw_doc_count(iw));
}

static VALUE
frt_iw_get_merge_factor(VALUE self)
{
  GET_IW;
  return INT2FIX(iw->merge_factor);
}

static VALUE
frt_iw_get_min_merge_docs(VALUE self)
{
  GET_IW;
  return INT2FIX(iw->min_merge_docs);
}

static VALUE
frt_iw_get_max_merge_docs(VALUE self)
{
  GET_IW;
  return INT2FIX(iw->max_merge_docs);
}

static VALUE
frt_iw_get_max_field_length(VALUE self)
{
  GET_IW;
  return INT2FIX(iw->max_field_length);
}

static VALUE
frt_iw_get_term_index_interval(VALUE self)
{
  GET_IW;
  return INT2FIX(iw->term_index_interval);
}

static VALUE
frt_iw_get_use_compound_file(VALUE self)
{
  GET_IW;
  return INT2FIX(iw->use_compound_file);
}

static VALUE
frt_iw_optimize(VALUE self)
{
  GET_IW;
  iw_optimize(iw);
  return Qnil;
}

/****************************************************************************
 *
 * IndexReader Methods
 *
 ****************************************************************************/

void
frt_ir_free(void *p)
{
  object_del(p);
  ir_close((IndexReader *)p);
}

void
frt_ir_mark(void *p)
{
  IndexReader *ir = (IndexReader *)p;
  frt_gc_mark(ir->store);
}

static VALUE
frt_ir_init(int argc, VALUE *argv, VALUE self)
{
  VALUE rdir, rclose_dir;
  //bool close_dir = false;
  Store *store = NULL;
  IndexReader *ir;
  switch (rb_scan_args(argc, argv, "11", &rdir, &rclose_dir)) {
    case 2: //close_dir = RTEST(rclose_dir);
    case 1: 
      if (TYPE(rdir) == T_DATA) {
        store = DATA_PTR(rdir);
      } else {
        rdir = rb_obj_as_string(rdir);
        frt_create_dir(rdir);
        store = open_fs_store(RSTRING(rdir)->ptr);
        deref(store);
      }
  }
  ir = ir_open(store);
  Frt_Wrap_Struct(self, &frt_ir_mark, &frt_ir_free, ir);
  object_add(ir, self);
  return self;
}

static VALUE
frt_ir_open(int argc, VALUE *argv, VALUE klass)
{
  VALUE self = Frt_Make_Struct(klass);
  return frt_ir_init(argc, argv, self);
}

#define GET_IR IndexReader *ir = (IndexReader *)DATA_PTR(self)

static VALUE
frt_ir_set_norm(VALUE self, VALUE rdoc_num, VALUE rfield, VALUE rval)
{
  GET_IR;
  rfield = rb_obj_as_string(rfield);
  ir_set_norm(ir, FIX2INT(rdoc_num), RSTRING(rfield)->ptr, NUM2CHR(rval));
  return Qnil;
}
  
static VALUE
frt_ir_get_norms(VALUE self, VALUE rfield)
{
  GET_IR;
  uchar *norms;
  rfield = rb_obj_as_string(rfield);
  norms = ir->get_norms(ir, RSTRING(rfield)->ptr);
  if (norms) {
    return rb_str_new((char *)norms, ir->max_doc(ir));
  } else {
    return Qnil;
  }
}

static VALUE
frt_ir_get_norms_into(VALUE self, VALUE rfield, VALUE rnorms, VALUE roffset)
{
  GET_IR;
  int offset;
  rfield = rb_obj_as_string(rfield);
  offset = FIX2INT(roffset);
  Check_Type(rnorms, T_STRING);
  if (RSTRING(rnorms)->len < offset + ir->max_doc(ir)) {
    rb_raise(rb_eArgError, "supplied a string of length:%d to IndexReader#get_norms_into but needed a string of length offset:%d + maxdoc:%d", RSTRING(rnorms)->len, offset, ir->max_doc(ir));
  }

  ir->get_norms_into(ir, RSTRING(rfield)->ptr, (uchar *)RSTRING(rnorms)->ptr, offset);
  return Qnil;
}

static VALUE
frt_ir_commit(VALUE self)
{
  GET_IR;
  ir_commit(ir);
  return Qnil;
}

static VALUE
frt_ir_close(VALUE self)
{
  GET_IR;
  object_del(ir);
  Frt_Unwrap_Struct(self);
  ir_close(ir);
  return Qnil;
}

static VALUE
frt_ir_has_deletions(VALUE self)
{
  GET_IR;
  return ir->has_deletions(ir) ? Qtrue : Qfalse;
}

static VALUE
frt_ir_delete(VALUE self, VALUE rdoc_num)
{
  GET_IR;
  int doc_num = FIX2INT(rdoc_num);
  ir_delete_doc(ir, doc_num);
  return Qnil;
}

static VALUE
frt_ir_is_deleted(VALUE self, VALUE rdoc_num)
{
  GET_IR;
  int doc_num = FIX2INT(rdoc_num);
  return ir->is_deleted(ir, doc_num) ? Qtrue : Qfalse;
}

static VALUE
frt_ir_max_doc(VALUE self)
{
  GET_IR;
  return INT2FIX(ir->max_doc(ir));
}

static VALUE
frt_ir_num_docs(VALUE self)
{
  GET_IR;
  return INT2FIX(ir->num_docs(ir));
}

static VALUE
frt_ir_undelete_all(VALUE self)
{
  GET_IR;
  ir_undelete_all(ir);
  return Qnil;
}

static VALUE
frt_ir_get_doc(VALUE self, VALUE rdoc_num)
{
  GET_IR;
  Document *doc = ir->get_doc(ir, FIX2INT(rdoc_num));
  return frt_get_doc(doc);
}

static VALUE
frt_ir_is_latest(VALUE self)
{
  GET_IR;
  return ir_is_latest(ir) ? Qtrue : Qfalse;
}

static VALUE
frt_ir_get_term_vector(VALUE self, VALUE rdoc_num, VALUE rfield)
{
  GET_IR;
  TermVector *tv;
  rfield = rb_obj_as_string(rfield);
  tv = ir->get_term_vector(ir, FIX2INT(rdoc_num), RSTRING(rfield)->ptr);
  return frt_get_tv(tv);
}

static VALUE
frt_ir_get_term_vectors(VALUE self, VALUE rdoc_num)
{
  int i;
  GET_IR;
  Array *tvs = ir->get_term_vectors(ir, FIX2INT(rdoc_num));
  VALUE rtvs = rb_ary_new2(tvs->size);
  VALUE rtv;
  for (i = 0; i < tvs->size; i++) {
    rtv = frt_get_tv(tvs->elems[i]); 
    rb_ary_push(rtvs, rtv);
  }
  tvs->free_elem = NULL;
  ary_destroy(tvs);

  return rtvs;
}

static VALUE
frt_ir_term_docs(VALUE self)
{
  GET_IR;
  return frt_get_tde(ir->term_docs(ir));
}

static VALUE
frt_ir_term_docs_for(VALUE self, VALUE rterm)
{
  GET_IR;
  Term t;
  frt_set_term(rterm, &t);
  return frt_get_tde(ir_term_docs_for(ir, &t));
}

static VALUE
frt_ir_term_positions(VALUE self)
{
  GET_IR;
  return frt_get_tde(ir->term_positions(ir));
}

static VALUE
frt_ir_term_positions_for(VALUE self, VALUE rterm)
{
  GET_IR;
  Term t;
  frt_set_term(rterm, &t);
  return frt_get_tde(ir_term_positions_for(ir, &t));
}

static VALUE
frt_ir_doc_freq(VALUE self, VALUE rterm)
{
  GET_IR;
  Term t;
  frt_set_term(rterm, &t);
  return INT2FIX(ir->doc_freq(ir, &t));
}

static VALUE
frt_ir_terms(VALUE self)
{
  TermEnum *te;
  GET_IR;
  te = ir->terms(ir);
  return Data_Wrap_Struct(cTermEnum, NULL, &frt_te_free, te);
}

static VALUE
frt_ir_terms_from(VALUE self, VALUE rterm)
{
  TermEnum *te;
  Term t;
  GET_IR;
  frt_set_term(rterm, &t);
  te = ir->terms_from(ir, &t);
  return Data_Wrap_Struct(cTermEnum, NULL, &frt_te_free, te);
}

static VALUE
frt_ir_get_field_names(VALUE self)
{
  GET_IR;
  VALUE rfnames;
  HashSet *fnames = ir->get_field_names(ir, IR_ALL);
  rfnames = frt_hs_to_rb_ary(fnames);
  hs_destroy(fnames);
  return rfnames;
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_index_io(void)
{
  ranalyzer_key = ID2SYM(rb_intern("analyzer"));
  rclose_dir_key = ID2SYM(rb_intern("close_dir"));
  rcreate_key = ID2SYM(rb_intern("create"));
  rcreate_if_missing_key = ID2SYM(rb_intern("create_if_missing"));
  ruse_compound_file_key = ID2SYM(rb_intern("use_compound_file"));
  rmerge_factor_key = ID2SYM(rb_intern("merge_factor"));
  rmin_merge_docs_key = ID2SYM(rb_intern("min_merge_docs"));
  rmax_merge_docs_key = ID2SYM(rb_intern("max_merge_docs"));
  rmax_field_length_key = ID2SYM(rb_intern("max_field_length"));
  rterm_index_interval_key = ID2SYM(rb_intern("term_index_interval"));

  /* TermEnum */
  cTermEnum = rb_define_class_under(mIndex, "TermEnum", rb_cObject);
  rb_define_alloc_func(cTermEnum, frt_data_alloc);

  rb_define_method(cTermEnum, "next?", frt_te_next, 0);
  rb_define_method(cTermEnum, "term", frt_te_term, 0);
  rb_define_method(cTermEnum, "doc_freq", frt_te_doc_freq, 0);
  rb_define_method(cTermEnum, "skip_to", frt_te_skip_to, 1);
  rb_define_method(cTermEnum, "close", frt_te_close, 0);

  /* TermVectorOffsetInfo */
  cTVOffsetInfo = rb_define_class_under(mIndex, "TermVectorOffsetInfo", rb_cObject);
  rb_define_alloc_func(cTVOffsetInfo, frt_data_alloc);

  rb_define_method(cTVOffsetInfo, "initialize", frt_tvoi_init, 2);
  rb_define_method(cTVOffsetInfo, "start=", frt_tvoi_set_start, 1);
  rb_define_method(cTVOffsetInfo, "start", frt_tvoi_get_start, 0);
  rb_define_method(cTVOffsetInfo, "end=", frt_tvoi_set_end, 1);
  rb_define_method(cTVOffsetInfo, "end", frt_tvoi_get_end, 0);
  rb_define_method(cTVOffsetInfo, "eql?", frt_tvoi_eql, 1);
  rb_define_method(cTVOffsetInfo, "==", frt_tvoi_eql, 1);
  rb_define_method(cTVOffsetInfo, "hash", frt_tvoi_hash, 0);
  rb_define_method(cTVOffsetInfo, "to_s", frt_tvoi_to_s, 0);

  /* TermVector */
  cTermVector = rb_define_class_under(mIndex, "TermVector", rb_cObject);
  rb_define_alloc_func(cTermVector, frt_data_alloc);
  rb_define_method(cTermVector, "field", frt_tv_get_field, 0);
  rb_define_method(cTermVector, "terms", frt_tv_get_terms, 0);
  rb_define_method(cTermVector, "freqs", frt_tv_get_freqs, 0);
  rb_define_method(cTermVector, "positions", frt_tv_get_positions, 0);
  rb_define_method(cTermVector, "offsets", frt_tv_get_offsets, 0);

  /* TermDocEnum */
  cTermDocEnum = rb_define_class_under(mIndex, "TermDocEnum", rb_cObject);
  rb_define_alloc_func(cTermDocEnum, frt_data_alloc);
  rb_define_method(cTermDocEnum, "close", frt_tde_close, 0);
  rb_define_method(cTermDocEnum, "seek", frt_tde_seek, 1);
  rb_define_method(cTermDocEnum, "doc", frt_tde_doc, 0);
  rb_define_method(cTermDocEnum, "freq", frt_tde_freq, 0);
  rb_define_method(cTermDocEnum, "next?", frt_tde_next, 0);
  rb_define_method(cTermDocEnum, "next_position", frt_tde_next_position, 0);
  rb_define_method(cTermDocEnum, "read", frt_tde_read, 2);
  rb_define_method(cTermDocEnum, "skip_to", frt_tde_skip_to, 1);

  /* IndexWriter */
  cIndexWriter = rb_define_class_under(mIndex, "IndexWriter", rb_cObject);
  rb_define_alloc_func(cIndexWriter, frt_data_alloc);

  rb_define_const(cIndexWriter, "WRITE_LOCK_TIMEOUT", INT2FIX(1));
  rb_define_const(cIndexWriter, "COMMIT_LOCK_TIMEOUT", INT2FIX(10));
  rb_define_const(cIndexWriter, "WRITE_LOCK_NAME",
      rb_str_new2(WRITE_LOCK_NAME));
  rb_define_const(cIndexWriter, "COMMIT_LOCK_NAME",
      rb_str_new2(COMMIT_LOCK_NAME));
  rb_define_const(cIndexWriter, "DEFAULT_MERGE_FACTOR",
      INT2FIX(config.merge_factor)); 
  rb_define_const(cIndexWriter, "DEFAULT_MIN_MERGE_DOCS",
      INT2FIX(config.min_merge_docs));
  rb_define_const(cIndexWriter, "DEFAULT_MAX_MERGE_DOCS",
      INT2FIX(config.max_merge_docs));
  rb_define_const(cIndexWriter, "DEFAULT_MAX_FIELD_LENGTH",
      INT2FIX(config.max_field_length));
  rb_define_const(cIndexWriter, "DEFAULT_TERM_INDEX_INTERVAL",
      INT2FIX(config.term_index_interval));

  rb_define_method(cIndexWriter, "initialize", frt_iw_init, -1);
  rb_define_method(cIndexWriter, "close", frt_iw_close, 0);
  rb_define_method(cIndexWriter, "add_document", frt_iw_add_doc, 1);
  rb_define_method(cIndexWriter, "<<", frt_iw_add_doc, 1);
  rb_define_method(cIndexWriter, "merge_factor", frt_iw_get_merge_factor, 0);
  rb_define_method(cIndexWriter, "min_merge_docs", frt_iw_get_min_merge_docs, 0);
  rb_define_method(cIndexWriter, "max_merge_docs", frt_iw_get_max_merge_docs, 0);
  rb_define_method(cIndexWriter, "max_field_length", frt_iw_get_max_field_length, 0);
  rb_define_method(cIndexWriter, "term_index_interval", frt_iw_get_term_index_interval, 0);
  rb_define_method(cIndexWriter, "use_compound_file", frt_iw_get_use_compound_file, 0);
  rb_define_method(cIndexWriter, "doc_count", frt_iw_get_doc_count, 0);
  rb_define_method(cIndexWriter, "merge_factor=", frt_iw_set_merge_factor, 1);
  rb_define_method(cIndexWriter, "min_merge_docs=", frt_iw_set_min_merge_docs, 1);
  rb_define_method(cIndexWriter, "max_merge_docs=", frt_iw_set_max_merge_docs, 1);
  rb_define_method(cIndexWriter, "max_field_length=", frt_iw_set_max_field_length, 1);
  rb_define_method(cIndexWriter, "term_index_interval=", frt_iw_set_term_index_interval, 1);
  rb_define_method(cIndexWriter, "use_compound_file=", frt_iw_set_use_compound_file, 1);
  rb_define_method(cIndexWriter, "optimize", frt_iw_optimize, 0);

  /* IndexReader */
  cIndexReader = rb_define_class_under(mIndex, "IndexReader", rb_cObject);
  rb_define_alloc_func(cIndexReader, frt_data_alloc);
  rb_define_singleton_method(cIndexReader, "open", frt_ir_open, -1);
  rb_define_method(cIndexReader, "initialize", frt_ir_init, -1);
  rb_define_method(cIndexReader, "set_norm", frt_ir_set_norm, 3);
  rb_define_method(cIndexReader, "get_norms", frt_ir_get_norms, 1);
  rb_define_method(cIndexReader, "get_norms_into", frt_ir_get_norms_into, 3);
  rb_define_method(cIndexReader, "commit", frt_ir_commit, 0);
  rb_define_method(cIndexReader, "close", frt_ir_close, 0);
  rb_define_method(cIndexReader, "has_deletions?", frt_ir_has_deletions, 0);
  rb_define_method(cIndexReader, "delete", frt_ir_delete, 1);
  rb_define_method(cIndexReader, "deleted?", frt_ir_is_deleted, 1);
  rb_define_method(cIndexReader, "max_doc", frt_ir_max_doc, 0);
  rb_define_method(cIndexReader, "num_docs", frt_ir_num_docs, 0);
  rb_define_method(cIndexReader, "undelete_all", frt_ir_undelete_all, 0);
  rb_define_method(cIndexReader, "latest?", frt_ir_is_latest, 0);
  rb_define_method(cIndexReader, "get_document", frt_ir_get_doc, 1);
  rb_define_method(cIndexReader, "[]", frt_ir_get_doc, 1);
  rb_define_method(cIndexReader, "get_term_vector", frt_ir_get_term_vector, 2);
  rb_define_method(cIndexReader, "get_term_vectors", frt_ir_get_term_vectors, 1);
  rb_define_method(cIndexReader, "term_docs", frt_ir_term_docs, 0);
  rb_define_method(cIndexReader, "term_positions", frt_ir_term_positions, 0);
  rb_define_method(cIndexReader, "term_docs_for", frt_ir_term_docs_for, 1);
  rb_define_method(cIndexReader, "term_positions_for", frt_ir_term_positions_for, 1);
  rb_define_method(cIndexReader, "doc_freq", frt_ir_doc_freq, 1);
  rb_define_method(cIndexReader, "terms", frt_ir_terms, 0);
  rb_define_method(cIndexReader, "terms_from", frt_ir_terms_from, 1);
  rb_define_method(cIndexReader, "get_field_names", frt_ir_get_field_names, 0);
}

#include "ferret.h"
#include "document.h"

VALUE cDocument;
VALUE cField;
VALUE cFieldStore;
VALUE cFieldIndex;
VALUE cFieldTermVector;

/****************************************************************************
 *
 * Field Methods
 *
 ****************************************************************************/

void
frt_field_free(void *p)
{
  object_del(p);
  df_destroy_data(p);
}

static VALUE
frt_field_alloc(VALUE klass)
{
  VALUE self;
  DocField *df = ALLOC(DocField);
  df->name = NULL;
  df->data = NULL;
  self = Data_Wrap_Struct(klass, NULL, &frt_field_free, df);
  object_add(df, self);
  return self;
}

#define GET_DF DocField *df; Data_Get_Struct(self, DocField, df)
static VALUE
frt_field_init(int argc, VALUE *argv, VALUE self)
{
  GET_DF;
  VALUE rname, rdata, rstored, rindexed, rstore_tv, rbinary, rboost;
  float boost = 1.0;
  int stored = 0, indexed = 0, store_tv = 0;
  bool binary = false;
  switch (rb_scan_args(argc, argv, "25", &rname, &rdata, &rstored,
            &rindexed, &rstore_tv, &rbinary, &rboost)) {
    case 7: boost = (float)rb_num2dbl(rboost);
    case 6: binary = RTEST(rbinary);
    case 5: store_tv = FIX2INT(rstore_tv);
    case 4: indexed = FIX2INT(rindexed);
    case 3: stored = FIX2INT(rstored);
    case 2:
      rname = rb_obj_as_string(rname);
      rdata = rb_obj_as_string(rdata);
      break;
  }
  char *name = RSTRING(rname)->ptr;
  int len = RSTRING(rdata)->len;
  char *data = ALLOC_N(char, len + 1);
  MEMCPY(data, RSTRING(rdata)->ptr, char, len);
  data[len] = 0;
  df_set(df, name, data, stored, indexed, store_tv);
  df->blen = len;
  df->is_binary = binary;
  df->boost = boost;
  return Qnil;
}

static VALUE
frt_field_get_name(VALUE self)
{
  GET_DF;
  return rb_str_new2(df->name);
}

static VALUE
frt_field_set_name(VALUE self, VALUE rname)
{
  int len;
  GET_DF;
  rname = rb_obj_as_string(rname);
  len = RSTRING(rname)->len;
  REALLOC_N(df->name, char, len);
  MEMCPY(df->name, RSTRING(rname)->ptr, char, len);
  return Qnil;
}

static VALUE
frt_field_get_data(VALUE self)
{
  GET_DF;
  return rb_str_new(df->data, df->blen);
}

static VALUE
frt_field_set_data(VALUE self, VALUE rdata)
{
  int len;
  GET_DF;
  rdata = rb_obj_as_string(rdata);
  len = RSTRING(rdata)->len;
  REALLOC_N(df->data, char, len);
  MEMCPY(df->data, RSTRING(rdata)->ptr, char, len);
  df->blen = len;
  return Qnil;
}

static VALUE
frt_field_get_boost(VALUE self)
{
  GET_DF;
  return rb_float_new((double)df->boost);
}

static VALUE
frt_field_set_boost(VALUE self, VALUE rboost)
{
  GET_DF;
  df->boost = (float)rb_num2dbl(rboost);
  return Qnil;
}

static VALUE
frt_field_is_stored(VALUE self)
{
  GET_DF;
  return df->is_stored ? Qtrue : Qfalse;
}

static VALUE
frt_field_is_indexed(VALUE self)
{
  GET_DF;
  return df->is_indexed ? Qtrue : Qfalse;
}

static VALUE
frt_field_is_tokenized(VALUE self)
{
  GET_DF;
  return df->is_tokenized ? Qtrue : Qfalse;
}

static VALUE
frt_field_is_binary(VALUE self)
{
  GET_DF;
  return df->is_binary ? Qtrue : Qfalse;
}

static VALUE
frt_field_is_compressed(VALUE self)
{
  GET_DF;
  return df->is_compressed ? Qtrue : Qfalse;
}

static VALUE
frt_field_store_tv(VALUE self)
{
  GET_DF;
  return df->store_tv ? Qtrue : Qfalse;
}

static VALUE
frt_field_store_pos(VALUE self)
{
  GET_DF;
  return df->store_pos ? Qtrue : Qfalse;
}

static VALUE
frt_field_store_offset(VALUE self)
{
  GET_DF;
  return df->store_offset ? Qtrue : Qfalse;
}

static VALUE
frt_field_omit_norms(VALUE self)
{
  GET_DF;
  return df->omit_norms ? Qtrue : Qfalse;
}

static VALUE
frt_field_to_s(VALUE self)
{
  VALUE rstr;
  char *str;
  GET_DF;

  str = df_to_s(df);
  rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

static VALUE
frt_field_new_binary(VALUE klass, VALUE rname, VALUE rdata, VALUE rstore)
{
  char *data;
  int len;
  DocField *df;
  int store = FIX2INT(rstore);
  rname = rb_obj_as_string(rname);
  rdata = rb_obj_as_string(rdata);
  len = RSTRING(rdata)->len;
  data = ALLOC_N(char, len);
  MEMCPY(data, RSTRING(rdata)->ptr, char, len);

  df = df_create_binary(RSTRING(rname)->ptr, data, len, store);
  return Data_Wrap_Struct(klass, NULL, &df_destroy_data, df);
}

static VALUE
frt_field_set_store(VALUE self, VALUE rstore)
{
  GET_DF;
  int store = FIX2INT(rstore);
  df_set_store(df, store);
  return Qnil;
}

static VALUE
frt_field_set_term_vector(VALUE self, VALUE rterm_vector)
{
  GET_DF;
  int term_vector = FIX2INT(rterm_vector);
  df_set_term_vector(df, term_vector);
  return Qnil;
}

static VALUE
frt_field_set_index(VALUE self, VALUE rindex)
{
  GET_DF;
  int index = FIX2INT(rindex);
  df_set_index(df, index);
  return Qnil;
}

/****************************************************************************
 *
 * Document Methods
 *
 ****************************************************************************/

void
frt_doc_free(void *p)
{
  object_del(p);
  doc_destroy(p);
}

void
frt_doc_mark(void *p)
{
  int i;
  DocField *df;
  Document *doc = (Document *)p;
  for (i = 0; i < doc->dfcnt; i++) {
    df = doc->df_arr[i];
    frt_gc_mark(df);
  }
}

static VALUE
frt_doc_alloc(VALUE klass)
{
  Document *doc = doc_create();
  doc->free_data = NULL;
  VALUE self = Data_Wrap_Struct(klass, &frt_doc_mark, &frt_doc_free, doc);
  object_add(doc, self);
  return self;
}

VALUE
frt_get_doc(Document *doc)
{
  VALUE rfield, self;
  DocField *df;
  int i;
  HshEntry *he;
  if (!doc || (self = object_get(doc)) != Qnil) return Qnil;

  doc->free_data = NULL;
  /* Set all fields to not free their data */
  for (i = 0; i <= doc->fields->mask; i++) {
    he = &doc->fields->table[i];
    if (he->key != NULL && he->key != dummy_key) {
      ((Array *)he->value)->free_elem = NULL;
    }
  }
  for (i = 0; i < doc->dfcnt; i++) {
    df = doc->df_arr[i];
    rfield = Data_Wrap_Struct(cField, NULL, &frt_field_free, df);
    object_add(df, rfield);
  }
  self = Data_Wrap_Struct(cDocument, &frt_doc_mark, &frt_doc_free, doc);
  object_add(doc, self);
  return self;
}

#define GET_DOC Document *doc; Data_Get_Struct(self, Document, doc)
static VALUE
frt_doc_init(VALUE self) 
{
  return self;
}

static VALUE
frt_doc_all_fields(VALUE self)
{
  int i;
  GET_DOC;
  VALUE values = rb_ary_new2(doc->dfcnt);
  for (i = 0; i < doc->dfcnt; i++) {
    rb_ary_push(values, object_get(doc->df_arr[i])); 
  }
  return values;
}

static VALUE
frt_doc_field_count(VALUE self)
{
  GET_DOC;
  return INT2FIX(doc->fcnt);
}

static VALUE
frt_doc_entry_count(VALUE self)
{
  GET_DOC;
  return INT2FIX(doc->dfcnt);
}

static VALUE
frt_doc_add_field(VALUE self, VALUE rfield)
{
  DocField *df;
  GET_DOC;
  Data_Get_Struct(rfield, DocField, df);
  doc_add_field(doc, df);
  return Qnil;
}

/* TODO: return the removed fields as an array */
static VALUE
frt_doc_remove_fields(VALUE self, VALUE rname)
{
  Array *fields;
  GET_DOC;
  rname = rb_obj_as_string(rname);
  fields = doc_remove_fields(doc, RSTRING(rname)->ptr);
  ary_destroy(fields);
  return Qnil;
}

static VALUE
frt_doc_remove_field(VALUE self, VALUE rname)
{
  DocField *df;
  GET_DOC;
  rname = rb_obj_as_string(rname);
  df = doc_remove_field(doc, RSTRING(rname)->ptr);
  return object_get(df);
}

static VALUE
frt_doc_field(VALUE self, VALUE rname)
{
  GET_DOC;
  DocField *df;
  rname = rb_obj_as_string(rname);
  df = doc_get_field(doc, RSTRING(rname)->ptr);
  return object_get(df);
}

static VALUE
frt_doc_fields(VALUE self, VALUE rname)
{
  int i;
  VALUE fields;
  GET_DOC;
  Array *dfs;
  rname = rb_obj_as_string(rname);
  dfs = doc_get_fields(doc, RSTRING(rname)->ptr);
  if (!dfs) return Qnil;
  fields = rb_ary_new2(dfs->size);
  for (i = 0; i < dfs->size; i++) {
    rb_ary_push(fields, object_get(dfs->elems[i]));
  }

  return fields;
}

static VALUE
frt_doc_values(VALUE self, VALUE rname)
{
  int i, len = 0, vindex = 0;
  VALUE rvalues;
  char *values = NULL;
  GET_DOC;
  Array *dfs;
  DocField *df;
  rname = rb_obj_as_string(rname);
  dfs = doc_get_fields(doc, RSTRING(rname)->ptr);
  if (!dfs) return Qnil;

  for (i = 0; i < dfs->size; i++) {
    df = (DocField *)dfs->elems[i];
    if (df->is_binary) continue;
    len += df->blen + 1;
    REALLOC_N(values, char, len);
    MEMCPY(values + vindex, df->data, char, df->blen);
    vindex = len;
    values[vindex-1] = ' ';
  }
  if (len) {
    values[len-1] = '\0';
    rvalues = rb_str_new(values, len-1);
    free(values);
  } else {
    rvalues = Qnil;
  }

  return rvalues;
}

static VALUE
frt_doc_binaries(VALUE self, VALUE rname)
{
  int i;
  VALUE rvalues;
  GET_DOC;
  Array *dfs;
  DocField *df;
  rname = rb_obj_as_string(rname);
  dfs = doc_get_fields(doc, RSTRING(rname)->ptr);
  if (!dfs) return Qnil;

  rvalues = rb_ary_new2(dfs->size);
  for (i = 0; i < dfs->size; i++) {
    df = (DocField *)dfs->elems[i];
    if (!df->is_binary) continue;
    rb_ary_push(rvalues, rb_str_new(df->data, df->blen));
  }
  return rvalues;
}

static VALUE
frt_doc_set(VALUE self, VALUE rname, VALUE rdata)
{
  DocField *df;
  GET_DOC;
  VALUE rfield;
  rname = rb_obj_as_string(rname);
  rdata = rb_obj_as_string(rdata);

  df = doc_get_field(doc, RSTRING(rname)->ptr);
  if (df) {
    free(df->data);
    df->data = estrdup(RSTRING(rdata)->ptr);
    rfield = object_get(df);
  } else {
    rfield = rb_funcall(cField, id_new, 2, rname, rdata);
    Data_Get_Struct(rfield, DocField, df);
    doc_add_field(doc, df);
  }
  return rfield;
}
static VALUE
frt_doc_to_s(VALUE self)
{
  char *str;
  VALUE rstr;
  GET_DOC;
  str = doc_to_s(doc);
  rstr = rb_str_new2(str);
  free(str);
  return rstr;
}

static VALUE
frt_doc_get_boost(VALUE self)
{
  GET_DOC;
  return rb_float_new((double)doc->boost);
}

static VALUE
frt_doc_set_boost(VALUE self, VALUE rboost)
{
  GET_DOC;
  doc->boost = (float)rb_num2dbl(rboost);
  return Qnil;
}


/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_doc(void)
{
  /* Field */
  cField = rb_define_class_under(mDocument, "Field", rb_cObject);
  rb_define_alloc_func(cField, frt_field_alloc);

  rb_define_method(cField, "initialize", frt_field_init, -1);
  rb_define_singleton_method(cField, "new_binary_field",
      frt_field_new_binary, 3);
  rb_define_method(cField, "name", frt_field_get_name, 0);
  rb_define_method(cField, "name=", frt_field_set_name, 1);
  rb_define_method(cField, "data", frt_field_get_data, 0);
  rb_define_method(cField, "data=", frt_field_set_data, 1);
  rb_define_method(cField, "boost", frt_field_get_boost, 0);
  rb_define_method(cField, "boost=", frt_field_set_boost, 1);
  rb_define_method(cField, "stored?", frt_field_is_stored, 0);
  rb_define_method(cField, "indexed?", frt_field_is_indexed, 0);
  rb_define_method(cField, "tokenized?", frt_field_is_tokenized, 0);
  rb_define_method(cField, "binary?", frt_field_is_binary, 0);
  rb_define_method(cField, "compressed?", frt_field_is_compressed, 0);
  rb_define_method(cField, "store_term_vector?", frt_field_store_tv, 0);
  rb_define_method(cField, "store_positions?", frt_field_store_pos, 0);
  rb_define_method(cField, "store_offsets?", frt_field_store_offset, 0);
  rb_define_method(cField, "omit_norms?", frt_field_omit_norms, 0);
  rb_define_method(cField, "to_s", frt_field_to_s, 0);
  rb_define_method(cField, "store=", frt_field_set_store, 1);
  rb_define_method(cField, "index=", frt_field_set_index, 1);
  rb_define_method(cField, "term_vector=", frt_field_set_term_vector, 1);

  /* Field Constants */
  cFieldStore = rb_define_class_under(cField, "Store", rb_cObject);
    rb_define_const(cFieldStore, "YES", INT2FIX(DF_STORE_YES));
    rb_define_const(cFieldStore, "NO", INT2FIX(DF_STORE_NO));
    rb_define_const(cFieldStore, "COMPRESS", INT2FIX(DF_STORE_COMPRESS));
  cFieldIndex = rb_define_class_under(cField, "Index", rb_cObject);
    rb_define_const(cFieldIndex, "UNTOKENIZED", INT2FIX(DF_INDEX_UNTOKENIZED));
    rb_define_const(cFieldIndex, "TOKENIZED", INT2FIX(DF_INDEX_TOKENIZED));
    rb_define_const(cFieldIndex, "NO", INT2FIX(DF_INDEX_NO));
    rb_define_const(cFieldIndex, "NO_NORMS", INT2FIX(DF_INDEX_NO_NORMS));
  cFieldTermVector = rb_define_class_under(cField, "TermVector", rb_cObject);
    rb_define_const(cFieldTermVector, "NO", INT2FIX(DF_TERM_VECTOR_NO));
    rb_define_const(cFieldTermVector, "YES", INT2FIX(DF_TERM_VECTOR_YES));
    rb_define_const(cFieldTermVector, "WITH_POSITIONS",
        INT2FIX(DF_TERM_VECTOR_WITH_POSITIONS));
    rb_define_const(cFieldTermVector, "WITH_OFFSETS",
        INT2FIX(DF_TERM_VECTOR_WITH_OFFSETS));
    rb_define_const(cFieldTermVector, "WITH_POSITIONS_OFFSETS",
        INT2FIX(DF_TERM_VECTOR_WITH_POSITIONS_OFFSETS));

  /* Document */
  cDocument = rb_define_class_under(mDocument, "Document", rb_cObject);
  rb_define_alloc_func(cDocument, frt_doc_alloc);

  rb_define_method(cDocument, "initialize", frt_doc_init, 0);
  rb_define_method(cDocument, "all_fields", frt_doc_all_fields, 0);
  rb_define_method(cDocument, "field_count", frt_doc_field_count, 0);
  rb_define_method(cDocument, "entry_count", frt_doc_entry_count, 0);
  rb_define_method(cDocument, "add_field", frt_doc_add_field, 1);
  rb_define_method(cDocument, "<<", frt_doc_add_field, 1);
  rb_define_method(cDocument, "remove_fields", frt_doc_remove_fields, 1);
  rb_define_method(cDocument, "remove_field", frt_doc_remove_field, 1);
  rb_define_method(cDocument, "field", frt_doc_field, 1);
  rb_define_method(cDocument, "fields", frt_doc_fields, 1);
  rb_define_method(cDocument, "values", frt_doc_values, 1);
  rb_define_method(cDocument, "binaries", frt_doc_binaries, 1);
  rb_define_method(cDocument, "[]", frt_doc_values, 1);
  rb_define_method(cDocument, "set", frt_doc_set, 2);
  rb_define_method(cDocument, "[]=", frt_doc_set, 2);
  rb_define_method(cDocument, "to_s", frt_doc_to_s, 0);
  rb_define_method(cDocument, "boost", frt_doc_get_boost, 0);
  rb_define_method(cDocument, "boost=", frt_doc_set_boost, 1);
}

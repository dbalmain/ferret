#include "ferret.h"
#include "hash.h"

/* Object Map */
static HshTable *object_map;

/* IDs */
ID id_new;

/* Modules */
VALUE mFerret;
VALUE mAnalysis;
VALUE mDocument;
VALUE mIndex;
VALUE mSearch;
VALUE mStore;
VALUE mStringHelper;
VALUE mUtils;
VALUE mSpans;

/* Classes */
/*
*/


unsigned int
object_hash(const void *key)
{
  return (unsigned int)key;
}

int
object_eq(const void *key1, const void *key2)
{
  return key1 == key2;
}

VALUE
object_get(void *key)
{
  VALUE val = (VALUE)h_get(object_map, key);
  if (!val) val = Qnil;
  return val;
}

//static int hash_cnt = 0;
void
//object_add(void *key, VALUE obj)
object_add2(void *key, VALUE obj, const char *file, int line, const char *func)
{
  if (h_get(object_map, key))
    printf("failed adding %d. %s:%d:%s\n", (int)key, file, line, func);
  //printf("adding %d. now contains %d %s:%d:%s\n", (int)key, ++hash_cnt, file, line, func);
  h_set(object_map, key, (void *)obj);
}

void
//object_del(void *key)
object_del2(void *key, const char *file, int line, const char *func)
{
  if (object_get(key) == Qnil) 
    printf("failed deleting %d. %s:%d:%s\n", (int)key, file, line, func);
  //printf("deleting %d. now contains %d, %s:%d:%s\n", (int)key, --hash_cnt, file, line, func);
  h_del(object_map, key);
}

void
frt_gc_mark(void *key)
{
  VALUE val = (VALUE)h_get(object_map, key);
  if (val)
    rb_gc_mark(val);
}

VALUE
frt_data_alloc(VALUE klass)
{
  return Frt_Make_Struct(klass);
}

void
frt_deref_free(void *p)
{
  object_del(p);
}

void
Init_ferret_ext(void)
{
  /* initialize object map */
  object_map = h_new(&object_hash, &object_eq, NULL, NULL);

  /* IDs */
	id_new = rb_intern("new");

  /* Modules */
  mFerret = rb_define_module("Ferret");
  mAnalysis = rb_define_module_under(mFerret, "Analysis");
  mDocument = rb_define_module_under(mFerret, "Document");
  mIndex = rb_define_module_under(mFerret, "Index");
  mSearch = rb_define_module_under(mFerret, "Search");
  mStore = rb_define_module_under(mFerret, "Store");
  mUtils = rb_define_module_under(mFerret, "Utils");
  mSpans = rb_define_module_under(mSearch, "Spans");

  /* Inits */
  Init_term();
  Init_analysis();
  Init_doc();
  Init_dir();
  Init_index_io();
  Init_search();
  Init_qparser();
}

#include "ferret.h"
#include "except.h"
#include "hash.h"

/* Object Map */
static HshTable *object_map;

/* IDs */
ID id_new;
ID id_call;

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

xcontext_t *xtop_context = NULL;

unsigned int
value_hash(const void *key)
{
  return (unsigned int)key;
}

int
value_eq(const void *key1, const void *key2)
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
object_add2(void *key, VALUE obj, const char *file, int line)
{
  if (h_get(object_map, key))
    printf("failed adding %d. %s:%d\n", (int)key, file, line);
  //printf("adding %d. now contains %d %s:%d\n", (int)key, ++hash_cnt, file, line);
  h_set(object_map, key, (void *)obj);
}

void
//object_set(void *key, VALUE obj)
object_set2(void *key, VALUE obj, const char *file, int line)
{
  //if (!h_get(object_map, key))
    //printf("seting %d. now contains %d %s:%d\n", (int)key, ++hash_cnt, file, line);
  h_set(object_map, key, (void *)obj);
}

void
//object_del(void *key)
object_del2(void *key, const char *file, int line)
{
  if (object_get(key) == Qnil) 
    printf("failed deleting %d. %s:%d\n", (int)key, file, line);
  //printf("deleting %d. now contains %d, %s:%d\n", (int)key, --hash_cnt, file, line);
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
frt_thread_once(int *once_control, void (*init_routine) (void))
{
  if (*once_control) {
    init_routine();
    *once_control = 0;
  }
}

void
frt_thread_key_create(thread_key_t *key, void (*destr_function) (void *))
{
  *key = h_new(&value_hash, &value_eq, NULL, destr_function);
}

void
frt_thread_key_delete(thread_key_t key)
{
  h_destroy(key);
}

void
frt_thread_setspecific(thread_key_t key, const void *pointer)
{
  h_set(key, (void *)rb_thread_current(), (void *)pointer);
}

void *
frt_thread_getspecific(thread_key_t key)
{
  return h_get(key, (void *)rb_thread_current());
}

void
Init_ferret_ext(void)
{
  /* initialize object map */
  object_map = h_new(&value_hash, &value_eq, NULL, NULL);

  /* IDs */
	id_new = rb_intern("new");
	id_call = rb_intern("call");

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

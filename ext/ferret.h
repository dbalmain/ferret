#ifndef __FERRET_H_
#define __FERRET_H_

#include "global.h"
#include "document.h"

/* IDs */
extern ID id_new;
extern ID id_call;

/* Modules */
extern VALUE mFerret;
extern VALUE mAnalysis;
extern VALUE mDocument;
extern VALUE mIndex;
extern VALUE mSearch;
extern VALUE mStore;
extern VALUE mStringHelper;
extern VALUE mUtils;
extern VALUE mSpans;

/* Classes */
extern VALUE cDirectory;

/* Ferret Inits */
extern void Init_term();
extern void Init_dir();
extern void Init_analysis();
extern void Init_doc();
extern void Init_index_io();
extern void Init_search();
extern void Init_qparser();
//extern void object_add(void *key, VALUE obj);
#define object_add(key, obj) object_add2(key, obj,  __FILE__, __LINE__)
extern void object_add2(void *key, VALUE obj, const char *file, int line);
//extern void object_set(void *key, VALUE obj);
#define object_set(key, obj) object_set2(key, obj,  __FILE__, __LINE__)
extern void object_set2(void *key, VALUE obj, const char *file, int line);
//extern void object_del(void *key);
#define object_del(key) object_del2(key,  __FILE__, __LINE__)
extern void object_del2(void *key, const char *file, int line);
extern void frt_gc_mark(void *key);
extern VALUE object_get(void *key);
extern VALUE frt_data_alloc(VALUE klass);
extern VALUE frt_get_doc(Document *doc);
extern void frt_deref_free(void *p);


#define Frt_Make_Struct(klass)\
  rb_data_object_alloc(klass,NULL,(RUBY_DATA_FUNC)NULL,(RUBY_DATA_FUNC)NULL)

#define Frt_Wrap_Struct(self,mmark,mfree,mdata)\
  do {\
    ((struct RData *)(self))->data = mdata;\
    ((struct RData *)(self))->dmark = (RUBY_DATA_FUNC)mmark;\
    ((struct RData *)(self))->dfree = (RUBY_DATA_FUNC)mfree;\
  } while (0)

#define Frt_Unwrap_Struct(self)\
  do {\
    ((struct RData *)(self))->data = NULL;\
    ((struct RData *)(self))->dmark = NULL;\
    ((struct RData *)(self))->dfree = NULL;\
  } while (0)

#endif

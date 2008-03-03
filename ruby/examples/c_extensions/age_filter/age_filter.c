#include <ruby.h>
#include <math.h>
#include <search.h>

static VALUE mFerretExt;
static VALUE cAgeFilter;

static VALUE age_filter_alloc(VALUE klass)
{
    PostFilter *post_filter = ALLOC(PostFilter);
    return Data_Wrap_Struct(klass, NULL, &free, post_filter);
}

static float age_filter(int doc_num, float score, Searcher *searcher, void *arg)
{
    long today = (long)arg;
    LazyDoc *lazy_doc = searcher_get_lazy_doc(searcher, doc_num);
    LazyDocField *lazy_df = h_get(lazy_doc->field_dict, "day");
    char *day_str = lazy_df_get_data(lazy_df, 0);
    long day, age;
    if (sscanf(day_str, "%ld", &day) != 1) {
        rb_raise(rb_eStandardError, "Couldn't parse <day> field in index as an"
                " integer. <day> field was '%s'.", day_str);
    }
    lazy_doc_close(lazy_doc);
    age = today - day;
    return 1.0/pow(2.0, age/50.0);
}

static VALUE age_filter_init(VALUE self, VALUE rdate_int)
{
    PostFilter *post_filter = DATA_PTR(self);
    Check_Type(rdate_int, T_FIXNUM);
    post_filter->filter_func = &age_filter;
    post_filter->arg = (void *)FIX2LONG(rdate_int);
    return self;
}

void Init_age_filter(void)
{
    mFerretExt = rb_define_module("FerretExt");
    cAgeFilter = rb_define_class_under(mFerretExt, "AgeFilter", rb_cObject);
    rb_define_alloc_func(cAgeFilter, age_filter_alloc);
    
    rb_define_method(cAgeFilter, "initialize", age_filter_init, 1);
}

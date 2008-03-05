#include <ruby.h>
#include <math.h>
#include <search.h>

static VALUE mFerretExt;
static VALUE cAgeFilter;
static VALUE cCachedAgeFilter;

/******************************************************************************
 * This is an example implementation of a C-extension for Ferret. Ferret is
 * already an extremely fast search library so sometimes it makes sense to
 * extend it with C instead of slowing it down with a bunch of Ruby code.
 *
 * Situation
 * =========
 * We have a large Ferret index (more than a million documents) and we'd like
 * to sort our search results based on the age of the documents, the more
 * recent being the most relevant.
 *
 * Solution 1 - A Naive Ruby Implementation
 * ----------------------------------------
 * The initial solution to this would be to write a filter_proc that adjusts
 * the score based on the age of the document. To do this we'll add a field
 * to each of our documents containing the date. To make things simple for
 * ourselves we'll store the date as an integer as in the following example;
 *
 *   DAY = 60*60*24
 *   $index = Ferret::I.new(:default_field => 'content',
 *                          :dir => 'index')
 *   documents.each do |document|
 *     $index << {
 *       :day => document.time.to_i / DAY,
 *       :content => document.content
 *     }
 *   end
 *
 * The :day field in the index will store the number of days after epoch that
 * the document was created. So to get the age (in days) of the 5th document
 * in the index you would do this;
 *
 *   TODAY = Time.now/DAY
 *   document5_age_in_days = TODAY - $index[5][:day].to_i
 *
 * Now to filter our search results. We'll use a half life of 50 days. That
 * is, a document will be twice as relevant as a similar document produced 50
 * days previously and 4 times as relevant as a document dated 100 days
 * before. (You can play with this value to see what works best for you).
 * Creating the filter_proc to do this turns out to be quite easy.
 *
 *   age_filter = lambda do |doc, score, searcher|
 *     1.0 / 2 ** ((TODAY - searcher[doc][:day].to_i)/50.0)
 *   end
 *
 * Our testing shows that this filter works correctly. However, we now have a
 * slight problem. It's too slow! (see benchmark-results.txt). Originally, it
 * took 0.04 seconds per query but now our queries are taking 45 seconds each.
 * This obviously isn't good enough. So your first thought might be to write a
 * C-extension to speed things up.
 *
 * Solution 2 - A Naive C extension
 * --------------------------------
 * The naive C implementation below is basically just a rewrite of the
 * filter_proc in C.
 *
 ******************************************************************************/
/***********************************************************************
 * A naive implementation of an age filter.
 ***********************************************************************/
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

/***********************************************************************
 * A cached implementation of an age filter.
 ***********************************************************************/

static void caf_free(void *p)
{
    PostFilter *post_filter = (PostFilter *)p;
    free(post_filter->arg);
    free(post_filter);
}

static VALUE cached_age_filter_alloc(VALUE klass)
{
    PostFilter *post_filter = ALLOC(PostFilter);
    return Data_Wrap_Struct(klass, NULL, &caf_free, post_filter);
}

static float cached_age_filter(int doc_num, float score, Searcher *searcher,
                                void *arg)
{
    long *age_cache = (long *)arg;
    return 1.0/pow(2.0, age_cache[doc_num]/50.0);
}

static VALUE cached_age_filter_init(VALUE self, VALUE rdate_int,
                                     VALUE rindex_reader)
{
    PostFilter *post_filter = DATA_PTR(self);
    IndexReader *index_reader = DATA_PTR(rindex_reader);
    TermDocEnum *term_doc_enum = index_reader->term_docs(index_reader);
    TermEnum *term_enum = ir_terms(index_reader, "day");
    char *day_str;
    long today, day, age;
    long *age_cache;
    Check_Type(rdate_int, T_FIXNUM);
    post_filter->filter_func = &cached_age_filter;
    post_filter->arg = 
        age_cache = ALLOC_AND_ZERO_N(long ,index_reader->max_doc(index_reader));
    today = FIX2LONG(rdate_int);
    while ((day_str = term_enum->next(term_enum)) != NULL) {
        if (sscanf(day_str, "%ld", &day) != 1) {
            rb_raise(rb_eStandardError, "Couldn't parse <day> field in index as"
                     " an integer. <day> field was '%s'.", day_str);
        }
        age = today - day;

        term_doc_enum->seek_te(term_doc_enum, term_enum);
        while (term_doc_enum->next(term_doc_enum)) {
            age_cache[term_doc_enum->doc_num(term_doc_enum)] = age;
        }
    }
    return self;
}

/***********************************************************************
 * Binding Initialization
 ***********************************************************************/
void Init_age_filter(void)
{
    mFerretExt = rb_define_module("FerretExt");
    cAgeFilter = rb_define_class_under(mFerretExt, "AgeFilter", rb_cObject);
    rb_define_alloc_func(cAgeFilter, age_filter_alloc);
    rb_define_method(cAgeFilter, "initialize", age_filter_init, 1);

    cCachedAgeFilter = rb_define_class_under(mFerretExt, "CachedAgeFilter",
                                              rb_cObject);
    rb_define_alloc_func(cCachedAgeFilter, cached_age_filter_alloc);
    
    rb_define_method(cCachedAgeFilter, "initialize", cached_age_filter_init, 2);
}

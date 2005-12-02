#include "ferret.h"
#include <math.h>

/****************************************************************************
 *
 * Similarity Methods
 *
 ****************************************************************************/

/*
static VALUE
frt_sim_c_byte_to_float(VALUE self, VALUE rbyte)
{
  int b = FIX2INT(rbyte);
  if (b == 0)
    return rb_float_new(0.0);
  int mantissa = b & 0x07;           // 0x07 =  7 = 0b00000111
  int exponent = (b >> 3) & 0x1F;    // 0x1f = 31 = 0b00011111
  int val = (mantissa << 21) | ((exponent + 48) << 24);
  void *tmp = &val;
  return rb_float_new(*(float *)tmp);
}
*/

static VALUE
frt_dsim_tf(VALUE self, VALUE freq)
{
  return rb_float_new(sqrt(NUM2DBL(freq)));
}

static VALUE
frt_dsim_idf(VALUE self, VALUE rdoc_freq, VALUE rnum_docs)
{
  int doc_freq;
  int num_docs = FIX2INT(rnum_docs);
  if (num_docs == 0) return rb_float_new(0.0);

  doc_freq = FIX2INT(rdoc_freq);
  return rb_float_new(log((double)num_docs/(double)(doc_freq+1)) + 1.0);
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_similarity(void)
{
  VALUE cDefaultSimilarity;
  /* Similarity */
  cSimilarity = rb_define_class_under(mSearch, "Similarity", rb_cObject);
  cDefaultSimilarity = rb_define_class_under(mSearch, "DefaultSimilarity", cSimilarity);

  //rb_define_singleton_method(cSimilarity, "byte_to_float", frt_sim_c_byte_to_float, 1);
  rb_define_method(cDefaultSimilarity, "tf", frt_dsim_tf, 1);
  rb_define_method(cDefaultSimilarity, "idf", frt_dsim_idf, 2);
}

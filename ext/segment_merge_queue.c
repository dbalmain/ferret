#include "ferret.h"

ID eq, lt;
/****************************************************************************
 *
 * SegmentMergeQueue Methods
 *
 ****************************************************************************/

static VALUE
frt_smq_less_than(VALUE self, VALUE rsti1, VALUE rsti2)
{
  int base1, base2;
  int cmp = frt_term_compare_to_int(rb_iv_get(rsti1, "@term"), rb_iv_get(rsti2, "@term"));
  if (cmp == 0) {
    base1 = FIX2INT(rb_iv_get(rsti1, "@base"));
    base2 = FIX2INT(rb_iv_get(rsti2, "@base"));
    return base1 < base2 ? Qtrue : Qfalse;
  } else {
    return cmp < 0 ? Qtrue : Qfalse;
  }
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_segment_merge_queue(void)
{
  // IDs
  eq = rb_intern("==");
  lt = rb_intern("<");

  // SegmentMergeQueue
	cSegmentMergeQueue = rb_define_class_under(mIndex, "SegmentMergeQueue", cPriorityQueue);

	rb_define_method(cSegmentMergeQueue, "less_than", frt_smq_less_than, 2);
}

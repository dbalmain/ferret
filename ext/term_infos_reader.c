#include "ferret.h"

static ID frt_id_index_terms;
/****************************************************************************
 *
 * TermInfosReader Methods
 *
 ****************************************************************************/

static VALUE
frt_tir_get_index_offset(VALUE self, VALUE rterm)
{
  VALUE index_terms = rb_ivar_get(self, frt_id_index_terms);

  register int lo = 0;            // binary search @index_terms[]
  register int hi = RARRAY(index_terms)->len - 1;
  register int mid, delta;

  Term *term, *tmp_term; 
  Data_Get_Struct(rterm, Term, term);

  while (hi >= lo) {
    mid = (lo + hi) >> 1;

    Data_Get_Struct(RARRAY(index_terms)->ptr[mid], Term, tmp_term);
    delta = frt_term_cmp(term, tmp_term);
    if (delta < 0) {
      hi = mid - 1;
    } else if (delta > 0) {
      lo = mid + 1;
    } else {
      return INT2FIX(mid);
    }
  }
  return INT2FIX(hi);
}

/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_term_infos_reader(void)
{
  /* IDs */
  frt_id_index_terms = rb_intern("@index_terms");

  /* TermInfosReader */
  cTermInfosReader = rb_define_class_under(mIndex, "TermInfosReader", rb_cObject);

  rb_define_method(cTermInfosReader, "get_index_offset", frt_tir_get_index_offset, 1);
}

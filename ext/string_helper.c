#include "ferret.h"

/****************************************************************************
 *
 * StringHelper Methods
 *
 ****************************************************************************/
inline int
frt_sh_string_difference_int(VALUE self, VALUE rstr1, VALUE rstr2)
{
  int i;
  struct RString *str1 = RSTRING(rstr1), *str2 = RSTRING(rstr2);
  char *s1 = str1->ptr, *s2 = str2->ptr;
  int len = str1->len < str2->len ? str1->len : str2->len;
  for (i = 0; i < len; i++) {
    if (s1[i] != s2[i]) 
      return i;
  }
  return len;
}

static VALUE
frt_sh_string_difference(VALUE self, VALUE rstr1, VALUE rstr2)
{
  return INT2FIX(frt_sh_string_difference_int(self, rstr1, rstr2));
}


/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_string_helper(void)
{
	// StringHelper
  mStringHelper = rb_define_module_under(mUtils, "StringHelper");

	rb_define_method(mStringHelper, "string_difference", frt_sh_string_difference, 2);
}

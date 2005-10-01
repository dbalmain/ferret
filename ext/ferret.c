#include "ferret.h"

void
Init_extensions(void)
{
  // IDs
	frt_newobj = rb_intern("new");

  // Modules
  mFerret = rb_define_module("Ferret");
  mStore = rb_define_module_under(mFerret, "Store");
  mIndex = rb_define_module_under(mFerret, "Index");
  mUtils = rb_define_module_under(mFerret, "Utils");

  // Inits
  Init_indexio();
  Init_term();
  Init_term_buffer();
  Init_priority_queue();
  Init_segment_merge_queue();
  Init_ram_directory();
  Init_string_helper();
}

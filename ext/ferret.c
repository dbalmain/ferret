#include "ferret.h"

// IDs
ID frt_newobj;

// Modules
VALUE mFerret;
VALUE mStore;
VALUE mIndex;
VALUE mUtils;
VALUE mStringHelper;

// Classes
VALUE cRAMDirectory;
VALUE cIndexIn;
VALUE cBufferedIndexIn;
VALUE cFSIndexIn;
VALUE cIndexOut;
VALUE cBufferedIndexOut;
VALUE cFSIndexOut;
VALUE cRAMIndexOut;
VALUE cRAMIndexIn;
VALUE cTerm;
VALUE cTermBuffer;
VALUE cPriorityQueue;
VALUE cSegmentMergeQueue;

void
Init_ferret_ext(void)
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

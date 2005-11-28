#include "ferret.h"

/* IDs */
ID id_new;
ID id_close;
ID id_size;
ID id_iv_size;

/* Modules */
VALUE mFerret;
VALUE mStore;
VALUE mIndex;
VALUE mUtils;
VALUE mAnalysis;
VALUE mStringHelper;

/* Classes */
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
VALUE cTermInfo;
VALUE cToken;
VALUE cPriorityQueue;
VALUE cSegmentMergeQueue;
VALUE cSegmentTermEnum;
VALUE cTermEnum;
VALUE cTermInfosReader;

void
Init_ferret_ext(void)
{
  /* IDs */
	id_new = rb_intern("new");
	id_close = rb_intern("close");
	id_size = rb_intern("size");
	id_iv_size = rb_intern("@size");

  /* Modules */
  mFerret = rb_define_module("Ferret");
  mStore = rb_define_module_under(mFerret, "Store");
  mIndex = rb_define_module_under(mFerret, "Index");
  mUtils = rb_define_module_under(mFerret, "Utils");
  mAnalysis = rb_define_module_under(mFerret, "Analysis");

  /* Classes */
  cTermEnum = rb_define_class_under(mIndex, "TermEnum", rb_cObject);

  /* Inits */
  Init_indexio();
  Init_term();
  Init_term_buffer();
  Init_term_info();
  Init_term_infos_reader();
  Init_token();
  Init_priority_queue();
  Init_segment_merge_queue();
  Init_segment_term_enum();
  Init_ram_directory();
  Init_string_helper();
}

#ifndef __FERRET_H_
#define __FERRET_H_

#include <ruby.h>

#define BUFFER_SIZE 1024

typedef unsigned char byte_t;

typedef struct IndexBuffer {
  long start;
  int len;
  int pos;
  byte_t *buffer;
} IndexBuffer;

typedef struct Term {
	VALUE field;
	char *text;
	int  tlen;
} Term;

typedef struct PriorityQueue {
	VALUE *heap;
	int len;
	int size;
} PriorityQueue;

typedef struct TermInfo {
  int doc_freq;
  long freq_pointer;
  long prox_pointer;
  int skip_offset;
} TermInfo;

typedef struct RAMFile {
  void  **buffers;
  int   bufcnt;
  VALUE mtime;
  char  *name;
  int   length;
} RAMFile;

typedef struct SegmentTermEnum {
  VALUE input;
  IndexBuffer *buf;
  VALUE field_infos;
  VALUE rtb_curr;
  Term *tb_curr;
  VALUE rtb_prev;
  Term *tb_prev;
  TermInfo *ti;
  int is_index;
  int size;
  int position;
  int index_pointer;
  int index_interval;
  int skip_interval;
  int format;
  int format_m1skip_interval;
} SegmentTermEnum;

/* IDs */
extern ID id_new;
extern ID id_close;
extern ID id_size;
extern ID id_iv_size;

/* Modules */
extern VALUE mFerret;
extern VALUE mStore;
extern VALUE mIndex;
extern VALUE mUtils;
extern VALUE mAnalysis;
extern VALUE mSearch;
extern VALUE mStringHelper;

/* Classes */
extern VALUE cRAMDirectory;
extern VALUE cIndexIn;
extern VALUE cBufferedIndexIn;
extern VALUE cFSIndexIn;
extern VALUE cIndexOut;
extern VALUE cBufferedIndexOut;
extern VALUE cFSIndexOut;
extern VALUE cRAMIndexOut;
extern VALUE cRAMIndexIn;
extern VALUE cTerm;
extern VALUE cTermBuffer;
extern VALUE cTermInfo;
extern VALUE cToken;
extern VALUE cPriorityQueue;
extern VALUE cSegmentMergeQueue;
extern VALUE cTermEnum;
extern VALUE cTermInfosReader;
extern VALUE cSegmentTermEnum;
extern VALUE cSimilarity;
extern VALUE cDefaultSimilarity;

/* Ferret Inits */
extern void Init_indexio();
extern void Init_term();
extern void Init_term_info();
extern void Init_term_infos_reader();
extern void Init_term_buffer();
extern void Init_priority_queue();
extern void Init_token();
extern void Init_segment_merge_queue();
extern void Init_segment_term_enum();
extern void Init_ram_directory();
extern void Init_string_helper();
extern void Init_similarity();

/* External functions */
extern int frt_hash(register char *p, register int len);
extern unsigned long long frt_read_vint(VALUE self, IndexBuffer *my_buf);
extern VALUE frt_indexin_read_long(VALUE self);
extern VALUE frt_indexin_read_int(VALUE self);
extern VALUE frt_indexin_seek(VALUE self, VALUE pos);
extern VALUE frt_termbuffer_to_term(VALUE self);
extern void frt_read_chars(VALUE self, char *buf, int offset, int len);
extern void frt_write_bytes(VALUE self, byte_t *buf, int len);
extern int frt_term_compare_to_int(VALUE self, VALUE rother);
extern VALUE frt_termbuffer_init_copy(VALUE self, VALUE rother);
extern VALUE frt_termbuffer_read(VALUE self, VALUE input, VALUE info);
extern inline int frt_term_cmp(Term *t1, Term *t2);

#endif

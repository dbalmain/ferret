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
	char *field;
	char *text;
	int  flen;
	int  tlen;
} Term;

typedef struct PriorityQueue {
	VALUE *heap;
	int len;
	int size;
} PriorityQueue;

typedef struct TermBuffer {
	char *field;
	char *text;
	int flen;
	int tlen;
} TermBuffer;

typedef struct RAMFile {
  void  **buffers;
  int   bufcnt;
  VALUE mtime;
  char  *name;
  int   length;
} RAMFile;

/* IDs */
extern ID frt_newobj;

/* Modules */
extern VALUE mFerret;
extern VALUE mStore;
extern VALUE mIndex;
extern VALUE mUtils;
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
extern VALUE cPriorityQueue;
extern VALUE cSegmentMergeQueue;

/* Ferret Inits */
extern void Init_indexio();
extern void Init_term();
extern void Init_priority_queue();
extern void Init_term_buffer();
extern void Init_segment_merge_queue();
extern void Init_ram_directory();
extern void Init_string_helper();

/* External functions */
extern int frt_hash(register char *p, register int len);
extern unsigned long long frt_read_vint(VALUE self);
extern void frt_read_chars(VALUE self, char *buf, int offset, int len);
extern void frt_write_bytes(VALUE self, byte_t *buf, int len);
extern int frt_term_compare_to_int(VALUE self, VALUE rother);
#endif

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

// Ferret Inits
extern void Init_indexio();
extern void Init_term();
extern void Init_priority_queue();
extern void Init_term_buffer();
extern void Init_segment_merge_queue();
extern void Init_ram_directory();
extern void Init_string_helper();

// External functions
extern int frt_hash(register char *p, register int len);
extern unsigned long long frt_read_vint(VALUE self);
extern void frt_read_chars(VALUE self, char *buf, int offset, int len);
extern void frt_write_bytes(VALUE self, byte_t *buf, int len);
extern int frt_term_compare_to_int(VALUE self, VALUE rother);
#endif

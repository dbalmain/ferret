#ifndef FRT_VECTOR_H
#define FRT_VECTOR_H

/****************************************************************************
 *
 * TVOffsetInfo
 *
 ****************************************************************************/

typedef struct TVOffsetInfo {
  int start;
  int end;
} TVOffsetInfo;

#define tvoi_set(tvoi, start, end) do {\
    tvoi->start = start;\
    tvoi->end = end;\
} while (0)

/****************************************************************************
 *
 * TVField
 *
 ****************************************************************************/

typedef struct TVField {
  int tvf_pointer;
  int number;
} TVField;

TVField *tvf_new(int number, int store_positions, int store_offsets);
void tvf_destroy(void *p);

/****************************************************************************
 *
 * TVTerm
 *
 ****************************************************************************/

typedef struct TVTerm {
  char *text;
  int freq;
  int *positions;
  TVOffsetInfo **offsets;
} TVTerm;

TVTerm *tvt_new(char *text,
                   int freq,
                   int *positions,
                   TVOffsetInfo **offsets);
void tvt_destroy(void *p);

/****************************************************************************
 *
 * TermVector
 *
 ****************************************************************************/

typedef struct TermVector {
  char *field;
  char **terms;
  int tcnt;
  int *freqs;
  int **positions;
  TVOffsetInfo ***offsets;
} TermVector;

TermVector *tv_new(const char *field,
                      char **terms,
                      int  tcnt,
                      int  *freqs,
                      int  **positions,
                      TVOffsetInfo ***offsets);
void tv_destroy(TermVector *tv);

/****************************************************************************
 *
 * TermVectorsWriter
 *
 ****************************************************************************/

#define STORE_POSITIONS_WITH_TERMVECTOR 0x1
#define STORE_OFFSET_WITH_TERMVECTOR 0x2
    
#define FORMAT_VERSION 2
#define FORMAT_SIZE 4
    
#define TVX_EXTENSION ".tvx"
#define TVD_EXTENSION ".tvd"
#define TVF_EXTENSION ".tvf"

typedef struct TermVectorsWriter {
  TVField *curr_field;
  int curr_doc_pointer;
  OutStream *tvx;
  OutStream *tvd;
  OutStream *tvf;
  FieldInfos *fis;
  TVField **fields;
  int fcnt;
  int fsize;
  TVTerm **terms;
  int tcnt;
  int tsize;
} TermVectorsWriter;

TermVectorsWriter *tvw_open(Store *store, char *segment, FieldInfos *fis);
void tvw_close(TermVectorsWriter *tvw);
void tvw_open_doc(TermVectorsWriter *tvw);
void tvw_close_doc(TermVectorsWriter *tvw);
void tvw_open_field(TermVectorsWriter *tvw, char *field);
void tvw_close_field(TermVectorsWriter *tvw);
void tvw_add_term(TermVectorsWriter *tvw, char *text, int freq, int *positions, TVOffsetInfo **offsets);
void tvw_add_all_doc_vectors(TermVectorsWriter *tvw, Array *vectors);


/****************************************************************************
 *
 * TermVectorsReader
 *
 ****************************************************************************/

typedef struct TermVectorsReader {
  int size;
  InStream *tvx;
  InStream *tvd;
  InStream *tvf;
  FieldInfos *fis;
  int tvd_format;
  int tvf_format;
} TermVectorsReader;

TermVectorsReader *tvr_open(Store *store, char *segment, FieldInfos *fis);
TermVectorsReader *tvr_clone(TermVectorsReader *orig);
void tvr_close(TermVectorsReader *tvr);
TermVector *tvr_read_term_vector(TermVectorsReader *tvr,
    char *field, int tvf_pointer);
Array *tvr_get_tv(TermVectorsReader *tvr, int doc_num);
TermVector *tvr_get_field_tv(TermVectorsReader *tvr, int doc_num, char *field);

#endif

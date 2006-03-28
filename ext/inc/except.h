#ifndef FRT_EXCEPT_H
#define FRT_EXCEPT_H

#include <setjmp.h>
#include <ruby.h>

#define BODY 0
#define FINALLY -1
#define EXCEPTION 1
#define ERROR 1
#define IO_ERROR 2
#define ARG_ERROR 3
#define EOF_ERROR 4
#define UNSUPPORTED_ERROR 5
#define STATE_ERROR 6
#define PARSE_ERROR 7
#define MEM_ERROR 8

typedef struct xcontext_t {
  jmp_buf jbuf;
  struct xcontext_t *next;
  char *msg;
  volatile int excode;
  int handled : 1;
  int in_finally : 1;
} xcontext_t;

RUBY_EXTERN int rb_thread_critical;
extern xcontext_t *xtop_context;

#define TRY\
  xcontext_t xcontext;\
  rb_thread_critical = Qtrue;\
  xcontext.next = xtop_context;\
  xtop_context = &xcontext;\
  xcontext.handled = true;\
  xcontext.in_finally = false;\
  switch (setjmp(xcontext.jbuf)) {\
    case BODY:


#define XENDTRY\
  }\
  xtop_context = xcontext.next;\
  if (!xcontext.handled) {\
    RAISE(xcontext.excode, xcontext.msg);\
  }\
  rb_thread_critical = 0;

#define ENDTRY\
  }\
  if (!xcontext.in_finally) {\
    xtop_context = xcontext.next;\
    if (!xcontext.handled) {\
      RAISE(xcontext.excode, xcontext.msg);\
    }\
    xcontext.in_finally = 1;\
    longjmp(xcontext.jbuf, FINALLY);\
  }\
  rb_thread_critical = 0;

#define XFINALLY default: xcontext.in_finally = 1;

#define XCATCHALL break; default: xcontext.in_finally = 1;

#define RAISE(xexcode, xmsg) \
  do {\
    fprintf(stderr,"Error occured in %s, %d: %s\n", __FILE__, __LINE__, __func__);\
    if (!xtop_context) {\
      eprintf(EXCEPTION_CODE, "Error: exception %d not handled: %s", xexcode, xmsg);\
    } else if (!xtop_context->in_finally) {\
      xtop_context->msg = xmsg;\
      xtop_context->excode = xexcode;\
      xtop_context->handled = false;\
      longjmp(xtop_context->jbuf, xexcode);\
    } else if (xtop_context->handled) {\
      xtop_context->msg = xmsg;\
      xtop_context->excode = xexcode;\
      xtop_context->handled = false;\
    }\
  } while (0)

#define HANDLED() xcontext.handled = 1 /* true */

extern char * const UNSUPPORTED_ERROR_MSG;
extern char * const EOF_ERROR_MSG;

#endif

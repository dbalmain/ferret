/**
 * Exception Handling Framework
 *
 * Exception Handling looks something like this;
 *
 * <pre>
 *   TRY
 *       RAISE(EXCEPTION, msg1);
 *       break;
 *   case EXCEPTION:
 *       // This should be called
 *       exception_handled = true;
 *       HANDLED();
 *       break;
 *   default:
 *       // shouldn't enter here
 *       break;
 *   XFINALLY
 *       // this code will always be run
 *       if (close_widget_one(arg) == 0) {
 *           RAISE(EXCEPTION_CODE, msg);
 *       }
 *       // this code will also always run, even if the above exception is
 *       // raised
 *       if (close_widget_two(arg) == 0) {
 *           RAISE(EXCEPTION_CODE, msg);
 *       }
 *   ENDTRY
 * </pre>
 *
 * Basically exception handling uses the following macros;
 *
 * TRY 
 *   Sets up the exception handler and need be placed before any expected
 *   exceptions would be raised.
 *
 * case <EXCEPTION_CODE>:
 *   Internally the exception handling uses a switch statement so use the case
 *   statement with the appropriate error code to catch Exceptions. Hence, if
 *   you want to catch all exceptions, use the default keyword.
 *
 * HANDLED
 *   If you catch and handle an exception you need to explicitely call
 *   HANDLED(); or the exeption will be re-raised once the current exception
 *   handling context is left.
 *
 * case FINALLY:
 *   Code in this block is always called. Use this block to close any
 *   resources opened in the Exception handling body.
 *
 * XFINALLY
 *   Similar to case FINALLY: except that any exceptions thrown in this block
 *   are ignored until the end of the block is reached at which time the first
 *   exception which was raise will be re-raised. This is useful for closing a
 *   number of resources that all might raise exceptions so as much as
 *   possible will be successfully closed.
 *
 * ENDTRY
 *   Must be placed at the end of all exception handling code.
 */

#ifndef FRT_EXCEPT_H
#define FRT_EXCEPT_H

#include <setjmp.h>
#include "defines.h"

#define BODY 0
#define FINALLY 1
#define EXCEPTION 2
#define ERROR 2
#define IO_ERROR 3
#define ARG_ERROR 4
#define EOF_ERROR 5
#define UNSUPPORTED_ERROR 5
#define STATE_ERROR 6
#define PARSE_ERROR 7
#define MEM_ERROR 8

extern char *const UNSUPPORTED_ERROR_MSG;
extern char *const EOF_ERROR_MSG;
extern bool except_show_pos;

typedef struct xcontext_t
{
    jmp_buf jbuf;
    struct xcontext_t *next;
    const char *msg;
    volatile int excode;
    unsigned int handled : 1;
    unsigned int in_finally : 1;
} xcontext_t;

#define TRY\
  do {\
    xcontext_t xcontext;\
    xpush_context(&xcontext);\
    switch (setjmp(xcontext.jbuf)) {\
      case BODY:


#define XENDTRY\
    }\
    xpop_context();\
  } while (0);

#define ENDTRY\
    }\
    if (!xcontext.in_finally) {\
      xpop_context();\
      xcontext.in_finally = 1;\
      longjmp(xcontext.jbuf, FINALLY);\
    }\
  } while (0);

#define XFINALLY default: xcontext.in_finally = 1;

#define XCATCHALL break; default: xcontext.in_finally = 1;

#ifdef FRT_HAS_ISO_VARARGS
# define RAISE(excode, ...) do {\
  sprintf(xmsg_buffer, __VA_ARGS__);\
  sprintf(xmsg_buffer_final, "Error occured in %s:%d - %s\n\t%s\n",\
          __FILE__, __LINE__, __func__, xmsg_buffer);\
  xraise(excode, xmsg_buffer_final);\
} while (0)
#elif defined(FRT_HAS_GNUC_VARARGS)
# define RAISE(excode, args...) do {\
  sprintf(xmsg_buffer, ##args);\
  sprintf(xmsg_buffer_final, "Error occured in %s:%d - %s\n\t%s\n",\
          __FILE__, __LINE__, __func__, xmsg_buffer);\
  xraise(excode, xmsg_buffer_final);\
} while (0)

#else
extern void RAISE(int excode, const char *const fmt, ...);
#endif

#define HANDLED() xcontext.handled = 1; /* true */

extern void xraise(int excode, const char *const msg);
extern void xpush_context(xcontext_t *context);
extern void xpop_context();

extern char xmsg_buffer[2048];
extern char xmsg_buffer_final[2048];

#endif

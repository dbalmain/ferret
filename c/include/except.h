/**
 * Exception Handling Framework
 *
 * Exception Handling looks something like this;
 *
 * ### NOTE ###
 * You must use a FINALLY block if you use "default:" block. Otherwise the
 * default: block will get called in place of the FINALLY block.
 *
 *
 * <pre>
 *   FRT_TRY
 *       FRT_RAISE(FRT_EXCEPTION, msg1);
 *       break;
 *   case FRT_EXCEPTION:
 *       // This should be called
 *       exception_handled = true;
 *       FRT_HANDLED();
 *       break;
 *   default:
 *       // shouldn't enter here
 *       break;
 *   FRT_XFINALLY
 *       // this code will always be run
 *       if (close_widget_one(arg) == 0) {
 *           FRT_RAISE(EXCEPTION_CODE, msg);
 *       }
 *       // this code will also always run, even if the above exception is
 *       // raised
 *       if (close_widget_two(arg) == 0) {
 *           FRT_RAISE(EXCEPTION_CODE, msg);
 *       }
 *   FRT_XENDTRY
 * </pre>
 *
 * Basically exception handling uses the following macros;
 *
 * FRT_TRY
 *   Sets up the exception handler and need be placed before any expected
 *   exceptions would be raised.
 *
 * case <EXCEPTION_CODE>:
 *   Internally the exception handling uses a switch statement so use the case
 *   statement with the appropriate error code to catch Exceptions. Hence, if
 *   you want to catch all exceptions, use the default keyword.
 *
 * FRT_HANDLED
 *   If you catch and handle an exception you need to explicitely call
 *   FRT_HANDLED(); or the exeption will be re-raised once the current exception
 *   handling context is left.
 *
 * case FRT_FINALLY:
 *   Code in this block is always called. Use this block to close any
 *   resources opened in the Exception handling body.
 *
 * FRT_ENDTRY
 *   Must be placed at the end of all exception handling code.
 *
 * FRT_XFINALLY
 *   Similar to case FRT_FINALLY: except that it uses a fall through (ie, you must
 *   not use a break before it) instead of a jump to get to it. This saves a
 *   jump. It must be used in combination with FRT_XENDTRY and must not have any
 *   other catches. This is an optimization so should probably be not be used
 *   in most cases.
 *
 * FRT_XCATCHALL
 *   Like FRT_XFINALLY but the block is only called when an exception is raised.
 *   Must use in combination with FRT_XENDTRY and do not have any other FRT_FINALLY or
 *   catch block.
 *
 * FRT_XENDTRY
 *   Must use in combination with FRT_XFINALLY or FRT_XCATCHALL. Simply, it doesn't
 *   jump to FRT_FINALLY, making it more efficient.
 */
#ifndef FRT_EXCEPT_H
#define FRT_EXCEPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <setjmp.h>
#include "config.h"

/* TODO make this an enum */
#define FRT_BODY 0
#define FRT_FINALLY 1
#define FRT_EXCEPTION 2
#define FRT_FERRET_ERROR 2
#define FRT_IO_ERROR 3
#define FRT_FILE_NOT_FOUND_ERROR 4
#define FRT_ARG_ERROR 5
#define FRT_EOF_ERROR 6
#define FRT_UNSUPPORTED_ERROR 7
#define FRT_STATE_ERROR 8
#define FRT_PARSE_ERROR 9
#define FRT_MEM_ERROR 10
#define FRT_INDEX_ERROR 11
#define FRT_LOCK_ERROR 12

extern char *const FRT_UNSUPPORTED_ERROR_MSG;
extern char *const FRT_EOF_ERROR_MSG;

typedef struct frt_xcontext_t
{
    jmp_buf jbuf;
    struct frt_xcontext_t *next;
    const char *msg;
    volatile int excode;
    unsigned int handled : 1;
    unsigned int in_finally : 1;
} frt_xcontext_t;

#define FRT_TRY\
  do {\
    frt_xcontext_t xcontext;\
    frt_xpush_context(&xcontext);\
    switch (setjmp(xcontext.jbuf)) {\
      case FRT_BODY:


#define FRT_XENDTRY\
    }\
    frt_xpop_context();\
  } while (0);

#define FRT_ENDTRY\
    }\
    if (!xcontext.in_finally) {\
      frt_xpop_context();\
      xcontext.in_finally = 1;\
      longjmp(xcontext.jbuf, FRT_FINALLY);\
    }\
  } while (0);

#define FRT_RETURN_EARLY() frt_xpop_context()


#define FRT_XFINALLY default: xcontext.in_finally = 1;

#define FRT_XCATCHALL break; default: xcontext.in_finally = 1;

#define FRT_HANDLED() xcontext.handled = 1; /* true */

#define FRT_XMSG_BUFFER_SIZE 2048

#ifdef FRT_HAS_ISO_VARARGS
# define FRT_RAISE(excode, ...) do {\
  snprintf(frt_xmsg_buffer, FRT_XMSG_BUFFER_SIZE, __VA_ARGS__);\
  snprintf(frt_xmsg_buffer_final, FRT_XMSG_BUFFER_SIZE,\
          "Error occurred in %s:%d - %s\n\t%s",\
          __FILE__, __LINE__, __func__, frt_xmsg_buffer);\
  frt_xraise(excode, frt_xmsg_buffer_final);\
} while (0)
#elif defined(FRT_HAS_GNUC_VARARGS)
# define FRT_RAISE(excode, args...) do {\
  snprintf(frt_xmsg_buffer, FRT_XMSG_BUFFER_SIZE, ##args);\
  snprintf(frt_xmsg_buffer_final, FRT_XMSG_BUFFER_SIZE,\
          "Error occurred in %s:%d - %s\n\t%s\n",\
          __FILE__, __LINE__, __func__, frt_xmsg_buffer);\
  frt_xraise(excode, frt_xmsg_buffer_final);\
} while (0)

#else
extern void FRT_RAISE(int excode, const char *fmt, ...);
#endif
#define RAISE_HELL() FRT_RAISE(FRT_FERRET_ERROR, "Hell")


extern void frt_xraise(int excode, const char *const msg);
extern void frt_xpush_context(frt_xcontext_t *context);
extern void frt_xpop_context();

extern char frt_xmsg_buffer[FRT_XMSG_BUFFER_SIZE];
extern char frt_xmsg_buffer_final[FRT_XMSG_BUFFER_SIZE];

#ifdef __cplusplus
} // extern "C"
#endif

#endif

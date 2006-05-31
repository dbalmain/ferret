#include <stdarg.h>
#include "global.h"
#include "except.h"
#include "threading.h"

/**
 * Set this to false if you don't want to print the error location when an
 * exception occurs.
 */
bool except_show_pos = true;

const char *const FRT_ERROR_TYPES[] = {
  "Body",
  "Finally",
  "Exception",
  "IO Error",
  "Argument Error",
  "End-of-File Error",
  "Unsupported Function Error",
  "State Error",
  "Parse Error",
  "Memory Error"
};

char *const UNSUPPORTED_ERROR_MSG = "Unsupported operation";
char *const EOF_ERROR_MSG = "Read past end of file";
char xmsg_buffer[2048];
char xmsg_buffer_final[2048];

static thread_key_t exception_stack_key;
static thread_once_t exception_stack_key_once = THREAD_ONCE_INIT;

static void exception_stack_alloc()
{
    thread_key_create(&exception_stack_key, NULL);
}

void xpush_context(xcontext_t *context)
{
    xcontext_t *top_context;
    thread_once(&exception_stack_key_once, *exception_stack_alloc);
    top_context = thread_getspecific(exception_stack_key);
    context->next = top_context;
    thread_setspecific(exception_stack_key, context);
    context->handled = true;
    context->in_finally = false;
}

static inline void xraise_context(xcontext_t *context,
                                  volatile int excode,
                                  const char *const msg)
{
    context->msg = msg;
    context->excode = excode;
    context->handled = false;
    longjmp(context->jbuf, excode);
}

#ifndef FRT_HAS_VARARGS
void RAISE(int excode, const char *const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    xraise(excode, vsprintf(xmsg_buffer, excode, fmt, args));
    va_end(args);
}
#endif

void xraise(int excode, const char *const msg)
{
    xcontext_t *top_context = thread_getspecific(exception_stack_key);

    if (!top_context) {
        FRT_EXIT(FRT_ERROR_TYPES[excode], msg);
    }
    else if (!top_context->in_finally) {
        xraise_context(top_context, excode, msg);
    }
    else if (top_context->handled) {
        top_context->msg = msg;
        top_context->excode = excode;
        top_context->handled = false;
    }
}

void xpop_context()
{
    xcontext_t *top_cxt, *context;
    thread_once(&exception_stack_key_once, *exception_stack_alloc);
    top_cxt = thread_getspecific(exception_stack_key);
    context = top_cxt->next;
    thread_setspecific(exception_stack_key, context);
    if (!top_cxt->handled) {
        if (context) {
            xraise_context(context, top_cxt->excode, top_cxt->msg);
        }
        else {
            FRT_EXIT(FRT_ERROR_TYPES[top_cxt->excode], top_cxt->msg);
        }
    }
}

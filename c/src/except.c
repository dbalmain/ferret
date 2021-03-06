#include <stdarg.h>
#include "global.h"
#include "except.h"
#include "threading.h"
#include "internal.h"

static const char *const ERROR_TYPES[] = {
    "Body",
    "Finally",
    "Exception",
    "IO Error",
    "File Not Found Error",
    "Argument Error",
    "End-of-File Error",
    "Unsupported Function Error",
    "State Error",
    "Parse Error",
    "Memory Error",
    "Index Error",
    "Lock Error"
};

const char *const UNSUPPORTED_ERROR_MSG = "Unsupported operation";
const char *const EOF_ERROR_MSG = "Read past end of file";
char xmsg_buffer[XMSG_BUFFER_SIZE];
char xmsg_buffer_final[XMSG_BUFFER_SIZE];

static thread_key_t exception_stack_key;
static thread_once_t exception_stack_key_once = THREAD_ONCE_INIT;

static void exception_stack_alloc(void)
{
    thread_key_create(&exception_stack_key, NULL);
}

void xpush_context(xcontext_t *context)
{
    xcontext_t *top_context;
    thread_once(&exception_stack_key_once, *exception_stack_alloc);
    top_context = (xcontext_t *)thread_getspecific(exception_stack_key);
    context->next = top_context;
    thread_setspecific(exception_stack_key, context);
    context->handled = true;
    context->in_finally = false;
}

static INLINE void xraise_context(xcontext_t *context,
                                    volatile int excode,
                                    const char *const msg)
{
    context->msg = msg;
    context->excode = excode;
    context->handled = false;
    longjmp(context->jbuf, excode);
}

#ifndef FRT_HAS_VARARGS
void RAISE(int excode, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(xmsg_buffer, XMSG_BUFFER_SIZE, fmt, args);
    xraise(excode, xmsg_buffer);
    va_end(args);
}
#endif

void xraise(int excode, const char *const msg)
{
    xcontext_t *top_context;
    thread_once(&exception_stack_key_once, *exception_stack_alloc);
    top_context = (xcontext_t *)thread_getspecific(exception_stack_key);

    if (!top_context) {
        XEXIT(ERROR_TYPES[excode], msg);
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
    top_cxt = (xcontext_t *)thread_getspecific(exception_stack_key);
    context = top_cxt->next;
    thread_setspecific(exception_stack_key, context);
    if (!top_cxt->handled) {
        if (context) {
            xraise_context(context, top_cxt->excode, top_cxt->msg);
        }
        else {
            XEXIT(ERROR_TYPES[top_cxt->excode], top_cxt->msg);
        }
    }
}

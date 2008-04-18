#include "except.h"
#include "test.h"

static char *msg1 = "Message One";
static char *msg2 = "Message Two";

static void raise_exception()
{
    RAISE(EXCEPTION, msg1);
}

static void inner_try(tst_case *tc)
{
    volatile bool exception_handled = false;
    volatile bool ioerror_called = false;

    TRY
        raise_exception();
        Assert(false, "Exception should have been raised");
        break;
    case EXCEPTION:
        /* This should be called */
        Astrstr(xcontext.msg, msg1);
        exception_handled = true;
        HANDLED();
        RAISE(IO_ERROR, msg2);
        Assert(false, "Exception should have been raised");
        break;
    case IO_ERROR:
        ioerror_called = true;
        break;
    default:
        Assert(false, "Exception should have been known");
        break;
    case FINALLY:
        Assert(exception_handled, "Exception wasn't handled");
        Assert(ioerror_called, "IO_ERROR wasn't called");
    ENDTRY
}

static void test_nested_except(tst_case *tc, void *data)
{
    volatile bool ioerror_handled = false;
    bool finally_handled = false;
    (void)data;

    TRY
        inner_try(tc);
        Assert(false, "Exception should have been raised");
        break;
    case IO_ERROR:
        /* This should be called */
        Astrstr(xcontext.msg, msg2);
        ioerror_handled = true;
        HANDLED();
        break;
    case EXCEPTION:
        /* This should be called */
        Assert(false, "Exception should not have been raised");
        break;
    default:
        Assert(false, "Exception should have been known");
        break;
    case FINALLY:
        finally_handled = true;
    ENDTRY
    Assert(ioerror_handled, "Exception wasn't handled");
    Assert(finally_handled, "Finally wasn't handled");
}

static void test_function_except(tst_case *tc, void *data)
{
    volatile bool exception_handled = false;
    bool finally_handled = false;
    (void)data; /* suppress warning */

    TRY
        raise_exception();
        Assert(false, "Exception should have been raised");
        break;
    case EXCEPTION:
        /* This should be called */
        Astrstr(xcontext.msg, msg1);
#if defined(__func__) && defined(FRT_HAS_VARARGS)
        Astrstr("raise_exception", msg1);
#endif
        exception_handled = true;
        HANDLED();
        break;
    default:
        Assert(false, "Exception should have been known");
        break;
    case FINALLY:
        finally_handled = true;
    ENDTRY
    Assert(exception_handled, "Exception wasn't handled");
    Assert(finally_handled, "Finally wasn't handled");
}

static void test_simple_except(tst_case *tc, void *data)
{
    volatile bool exception_handled = false;
    bool finally_handled = false;
    (void)data; /* suppress warning */

    TRY
        RAISE(EXCEPTION, "error message %s %d", "string", 20);
        Assert(false, "Exception should have been raised");
        break;
    case EXCEPTION:
        /* This should be called */
        Astrstr(xcontext.msg, "error message string 20");
#if defined(__func__) && defined(FRT_HAS_VARARGS)
        Astrstr(xcontext.msg, __func__);
#endif
#if defined(FRT_HAS_VARARGS)
        Astrstr(xcontext.msg, __FILE__);
#endif
        exception_handled = true;
        HANDLED();
        break;
    default:
        Assert(false, "Exception should have been known");
        break;
    case FINALLY:
        finally_handled = true;
    ENDTRY
    Assert(exception_handled, "Exception wasn't handled");
    Assert(finally_handled, "Finally wasn't handled");
}

static void try_xfinally1(tst_case *tc)
{
    bool finally_handled = false;

    TRY
        Assert(true, "No exception raised");
    XFINALLY
        RAISE(EXCEPTION, msg1);
        finally_handled = true;
    XENDTRY
    Assert(finally_handled, "Finally wasn't handled");
    Atrue(finally_handled);
}

static void try_xfinally2(tst_case *tc)
{
    bool finally_handled = false;

    TRY
        RAISE(EXCEPTION, msg1);
        Assert(false, "Exception should have been raised");
    XFINALLY
        RAISE(EXCEPTION, msg1);
        finally_handled = true;
    XENDTRY
    Assert(finally_handled, "Finally wasn't handled");
    Atrue(finally_handled);
}

static void try_xcatchall(tst_case *tc)
{
    bool catchall_handled = false;

    TRY
        RAISE(EXCEPTION, msg1);
        Assert(false, "Exception should have been raised");
    XCATCHALL
        HANDLED();
        RAISE(EXCEPTION, msg1);
        catchall_handled = true;
    XENDTRY
    Assert(catchall_handled, "Finally wasn't handled");
    Atrue(catchall_handled);
}

static void test_xfinally(tst_case *tc, void *data)
{
    volatile bool exception_handled = false;
    bool finally_handled = false;
    (void)data; /* suppress warning */

    TRY
        try_xfinally1(tc);
        try_xfinally2(tc);
        try_xcatchall(tc);
        Assert(false, "Exception should have been raised");
        break;
    case EXCEPTION:
        /* This should be called */
        Astrstr(xcontext.msg, msg1);
        exception_handled = true;
        HANDLED();
        break;
    default:
        Assert(false, "Exception should have been known");
        break;
    case FINALLY:
        finally_handled = true;
    ENDTRY
    Assert(exception_handled, "Exception wasn't handled");
    Assert(finally_handled, "Finally wasn't handled");
}

static void test_uncaught_except(tst_case *tc, void *data)
{
    bool old_abort_setting = x_abort_on_exception;
    FILE *old_stream_setting = x_exception_stream;
    FILE *exception_output = tmpfile();
    (void)data; /* suppress warning */


    x_abort_on_exception = false;
    x_exception_stream = exception_output;

    /* Unhandled exception in try block */
    TRY
        raise_exception();
    ENDTRY
    Assert(x_has_aborted, "Unhandled exception in try block didn't abort");

    /* Unhandled exception outside of try block */
    x_has_aborted = false;
    raise_exception();
    Assert(x_has_aborted, "Unhandled exception didn't cause an abort");

    x_abort_on_exception = old_abort_setting;
    x_exception_stream = old_stream_setting;
    fclose(exception_output);
}

tst_suite *ts_except(tst_suite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_simple_except, NULL);
    tst_run_test(suite, test_function_except, NULL);
    tst_run_test(suite, test_nested_except, NULL);
    tst_run_test(suite, test_xfinally, NULL);
    tst_run_test(suite, test_uncaught_except, NULL);
    return suite;
}

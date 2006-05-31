#include "test.h"
#include "except.h"

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
        RAISE(EXCEPTION, msg1);
        Assert(false, "Exception should have been raised");
        break;
    case EXCEPTION:
        /* This should be called */
        Astrstr(xcontext.msg, msg1);
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

static void test_xfinally(tst_case *tc, void *data)
{
    volatile bool exception_handled = false;
    bool finally_handled = false;
    (void)data; /* suppress warning */

    TRY
        try_xfinally1(tc);
        try_xfinally2(tc);
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

tst_suite *ts_except(tst_suite *suite)
{
    suite = ADD_SUITE(suite);

    except_show_pos = false;

    tst_run_test(suite, test_simple_except, NULL);
    tst_run_test(suite, test_function_except, NULL);
    tst_run_test(suite, test_nested_except, NULL);
    tst_run_test(suite, test_xfinally, NULL);

    except_show_pos = true;

    return suite;
}

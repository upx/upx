/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */


#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


#if (ACC_H_WINDOWS_H) && defined(acc_int64l_t)
   /* See also: KB Q274323: PRB: Performance Counter Value May
    *   Unexpectedly Leap Forward */
#  define __ACCLIB_UCLOCK_USE_QPC 1
#endif

#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#elif (ACC_OS_DOS32 && defined(__DJGPP__))
#elif (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_TOS)
#  define __ACCLIB_UCLOCK_USE_CLOCK 1
#elif (ACC_H_WINDOWS_H) && (ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (ACC_CC_DMC || ACC_CC_LCC)
     /* winmm.lib is missing */
#    define __ACCLIB_UCLOCK_USE_CLOCK 1
#  else
#    define __ACCLIB_UCLOCK_USE_MMSYSTEM 1
#    if (ACC_CC_MSC && (_MSC_VER >= 1000))
       /* avoid -W4 warnings in <mmsystem.h> */
#      pragma warning(disable: 4201)
#    endif
#    include <mmsystem.h>
#    if (ACC_CC_INTELC || ACC_CC_MSC)
#      pragma comment(lib,"winmm")
#    endif
#  endif
#endif

#if (__ACCLIB_UCLOCK_USE_CLOCK) && !defined(CLOCKS_PER_SEC)
#  if defined(CLK_TCK)
#    define CLOCKS_PER_SEC CLK_TCK
#  else
#    undef __ACCLIB_UCLOCK_USE_CLOCK
#  endif
#endif


/*************************************************************************
//
**************************************************************************/

ACCLIB_PUBLIC(int, acc_uclock_open) (acc_uclock_handle_p h)
{
    acc_uclock_t c;
    int i;

#if (__ACCLIB_UCLOCK_USE_QPC)
    LARGE_INTEGER li;
    h->qpf = 0.0;
    if (QueryPerformanceFrequency(&li) != 0) {
        h->qpf = (double) li.QuadPart;
        if (h->qpf <= 0.0 || QueryPerformanceCounter(&li) == 0) {
            h->qpf = 0.0;
        }
    }
#endif
    h->h = 1;
    h->mode = 0;

    /* warm up */
    i = 100;
    do {
        acc_uclock_read(h, &c);
#if (__ACCLIB_UCLOCK_USE_QPC)
        if (h->qpf > 0.0 && c.qpc == 0) { h->qpf = 0.0; i = 100; }
#endif
    } while (--i > 0);

    return 0;
}


ACCLIB_PUBLIC(int, acc_uclock_close) (acc_uclock_handle_p h)
{
    h->h = 0;
    h->mode = -1;
    return 0;
}


ACCLIB_PUBLIC(void, acc_uclock_read) (acc_uclock_handle_p h, acc_uclock_p c)
{
#if (__ACCLIB_UCLOCK_USE_QPC)
    if (h->qpf > 0.0) {
        LARGE_INTEGER li;
        if (QueryPerformanceCounter(&li) != 0) {
            c->qpc = (acc_int64l_t) li.QuadPart;
            if (c->qpc > 0)
                return;
        }
        c->qpc = 0; /* failed */
    }
#endif

    {
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
# if (ACC_CC_AZTECC)
    c->ticks.t32 = 0;
# else
    union REGS ri, ro;
    ri.x.ax = 0x2c00; int86(0x21, &ri, &ro);
    c->ticks.t32 = ro.h.ch*60UL*60UL*100UL + ro.h.cl*60UL*100UL + ro.h.dh*100UL + ro.h.dl;
# endif
#elif (ACC_OS_DOS32 && defined(__DJGPP__))
    c->ticks.t64 = uclock();
#elif (__ACCLIB_UCLOCK_USE_CLOCK) && defined(acc_int64l_t)
    c->ticks.t64 = clock();
#elif (__ACCLIB_UCLOCK_USE_CLOCK)
    c->ticks.t32 = clock();
#elif (__ACCLIB_UCLOCK_USE_MMSYSTEM)
    c->ticks.t32 = timeGetTime();
#elif (__ACCLIB_UCLOCK_USE_GETRUSAGE)
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0) c->ticks.rd = 0;
    else c->ticks.td = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
#elif (HAVE_GETTIMEOFDAY)
    struct timeval tv;
    if (gettimeofday(&tv, 0) != 0) c->ticks.td = 0;
    else c->ticks.td = tv.tv_sec + tv.tv_usec / 1000000.0;
#else
    ACC_UNUSED(c);
#endif
    }
    ACC_UNUSED(h);
}


ACCLIB_PUBLIC(double, acc_uclock_get_elapsed) (acc_uclock_handle_p h, const acc_uclock_p start, const acc_uclock_p stop)
{
    double d;

    if (!h->h) {
        h->mode = -1;
        return 0.0;
    }

#if (__ACCLIB_UCLOCK_USE_QPC)
    if (h->qpf > 0.0) {
        h->mode = 1;
        if (start->qpc == 0 || stop->qpc == 0) return 0.0;
        return (double) (stop->qpc - start->qpc) / h->qpf;
    }
#endif

#if (ACC_OS_DOS16 || ACC_OS_WIN16)
    h->mode = 2;
    if (stop->ticks.t32 < start->ticks.t32) /* midnight passed */
        d = 86400.0 - (start->ticks.t32 - stop->ticks.t32) / 100.0;
    else
        d = (stop->ticks.t32 - start->ticks.t32) / 100.0;
#elif (ACC_OS_DOS32 && defined(__DJGPP__))
    h->mode = 3;
    d = (double) (stop->ticks.t64 - start->ticks.t64) / (UCLOCKS_PER_SEC);
#elif (__ACCLIB_UCLOCK_USE_CLOCK) && defined(acc_int64l_t)
    h->mode = 4;
    d = (double) (stop->ticks.t64 - start->ticks.t64) / (CLOCKS_PER_SEC);
#elif (__ACCLIB_UCLOCK_USE_CLOCK)
    h->mode = 5;
    d = (double) (stop->ticks.t32 - start->ticks.t32) / (CLOCKS_PER_SEC);
#elif (__ACCLIB_UCLOCK_USE_MMSYSTEM)
    h->mode = 6;
    d = (double) (stop->ticks.t32 - start->ticks.t32) / 1000.0;
#elif (__ACCLIB_UCLOCK_USE_GETRUSAGE)
    h->mode = 7;
    d = stop->ticks.td - start->ticks.td;
#elif (HAVE_GETTIMEOFDAY)
    h->mode = 8;
    d = stop->ticks.td - start->ticks.td;
#else
    h->mode = 0;
    d = 0.0;
#endif
    return d;
}


/*
vi:ts=4:et
*/

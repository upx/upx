/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */


#define __ACCLIB_HALLOC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


#if (ACC_HAVE_MM_HUGE_PTR)
#if 1 && (ACC_OS_DOS16 && defined(BLX286))
#  define __ACCLIB_HALLOC_USE_DAH 1
#elif 1 && (ACC_OS_DOS16 && defined(DOSX286))
#  define __ACCLIB_HALLOC_USE_DAH 1
#elif 1 && (ACC_OS_OS216)
#  define __ACCLIB_HALLOC_USE_DAH 1
#elif 1 && (ACC_OS_WIN16)
#  define __ACCLIB_HALLOC_USE_GA 1
#elif 1 && (ACC_OS_DOS16) && (ACC_CC_BORLANDC) && defined(__DPMI16__)
#  define __ACCLIB_HALLOC_USE_GA 1
#endif
#endif


#if (__ACCLIB_HALLOC_USE_DAH)
#if 0 && (ACC_OS_OS216)
#include <os2.h>
#else
ACC_EXTERN_C unsigned short __far __pascal DosAllocHuge(unsigned short, unsigned short, unsigned short __far *, unsigned short, unsigned short);
ACC_EXTERN_C unsigned short __far __pascal DosFreeSeg(unsigned short);
#endif
#endif

#if (__ACCLIB_HALLOC_USE_GA)
#if 0
#define STRICT 1
#include <windows.h>
#else
ACC_EXTERN_C const void __near* __far __pascal GlobalAlloc(unsigned, unsigned long);
ACC_EXTERN_C const void __near* __far __pascal GlobalFree(const void __near*);
ACC_EXTERN_C unsigned long __far __pascal GlobalHandle(unsigned);
ACC_EXTERN_C void __far* __far __pascal GlobalLock(const void __near*);
ACC_EXTERN_C int __far __pascal GlobalUnlock(const void __near*);
#endif
#endif


/***********************************************************************
// halloc
************************************************************************/

ACCLIB_PUBLIC(acc_hvoid_p, acc_halloc) (acc_hsize_t size)
{
    acc_hvoid_p p = 0;

    if (size <= 0)
        return p;

#if 0 && defined(__palmos__)
    p = MemPtrNew(size);
#elif !defined(ACC_HAVE_MM_HUGE_PTR)
    if (size < (size_t) -1)
        p = malloc((size_t) size);
#else
    if ((long)size <= 0)
        return p;
{
#if (__ACCLIB_HALLOC_USE_DAH)
    unsigned short sel = 0;
    if (DosAllocHuge((unsigned short)(size >> 16), (unsigned short)size, &sel, 0, 0) == 0)
        p = (acc_hvoid_p) ACC_MK_FP(sel, 0);
#elif (__ACCLIB_HALLOC_USE_GA)
    const void __near* h = GlobalAlloc(2, size);
    if (h) {
        p = GlobalLock(h);
        if (p && ACC_FP_OFF(p) != 0) {
            GlobalUnlock(h);
            p = 0;
        }
        if (!p)
            GlobalFree(h);
    }
#elif (ACC_CC_MSC && (_MSC_VER >= 700))
    p = _halloc(size, 1);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    p = halloc(size, 1);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    p = farmalloc(size);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    p = farmalloc(size);
#elif defined(ACC_CC_AZTECC)
    p = lmalloc(size);
#else
    if (size < (size_t) -1)
        p = malloc((size_t) size);
#endif
}
#endif

    return p;
}


ACCLIB_PUBLIC(void, acc_hfree) (acc_hvoid_p p)
{
    if (!p)
        return;

#if 0 && defined(__palmos__)
    MemPtrFree(p);
#elif !defined(ACC_HAVE_MM_HUGE_PTR)
    free(p);
#else
#if (__ACCLIB_HALLOC_USE_DAH)
    if (ACC_FP_OFF(p) == 0)
        DosFreeSeg((unsigned short) ACC_FP_SEG(p));
#elif (__ACCLIB_HALLOC_USE_GA)
    if (ACC_FP_OFF(p) == 0) {
        const void __near* h = (const void __near*) (unsigned) GlobalHandle(ACC_FP_SEG(p));
        if (h) {
            GlobalUnlock(h);
            GlobalFree(h);
        }
    }
#elif (ACC_CC_MSC && (_MSC_VER >= 700))
    _hfree(p);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    hfree(p);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    farfree((void __far*) p);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    farfree((void __far*) p);
#elif defined(ACC_CC_AZTECC)
    lfree(p);
#else
    free(p);
#endif
#endif
}



/*
vi:ts=4:et
*/

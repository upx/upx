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


#define __ACCLIB_DOSALLOC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


#if (ACC_OS_OS216)
ACC_EXTERN_C unsigned short __far __pascal DosAllocHuge(unsigned short, unsigned short, unsigned short __far *, unsigned short, unsigned short);
ACC_EXTERN_C unsigned short __far __pascal DosFreeSeg(unsigned short);
#endif


/***********************************************************************
// dos_alloc
************************************************************************/

#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#if !defined(ACC_CC_AZTECC)

ACCLIB_PUBLIC(void __far*, acc_dos_alloc) (unsigned long size)
{
    void __far* p = 0;
    union REGS ri, ro;

    if ((long)size <= 0)
        return p;
    size = (size + 15) >> 4;
    if (size > 0xffffu)
        return p;
    ri.x.ax = 0x4800;
    ri.x.bx = (unsigned short) size;
    int86(0x21, &ri, &ro);
    if ((ro.x.cflag & 1) == 0)  /* if carry flag not set */
        p = (void __far*) ACC_MK_FP(ro.x.ax, 0);
    return p;
}


ACCLIB_PUBLIC(int, acc_dos_free) (void __far* p)
{
    union REGS ri, ro;
    struct SREGS rs;

    if (!p)
        return 0;
    if (ACC_FP_OFF(p) != 0)
        return -1;
    segread(&rs);
    ri.x.ax = 0x4900;
    rs.es = ACC_FP_SEG(p);
    int86x(0x21, &ri, &ro, &rs);
    if (ro.x.cflag & 1)         /* if carry flag set */
        return -1;
    return 0;
}

#endif
#endif


#if (ACC_OS_OS216)

ACCLIB_PUBLIC(void __far*, acc_dos_alloc) (unsigned long size)
{
    void __far* p = 0;
    unsigned short sel = 0;

    if ((long)size <= 0)
        return p;
    if (DosAllocHuge((unsigned short)(size >> 16), (unsigned short)size, &sel, 0, 0) == 0)
        p = (void __far*) ACC_MK_FP(sel, 0);
    return p;
}

ACCLIB_PUBLIC(int, acc_dos_free) (void __far* p)
{
    if (!p)
        return 0;
    if (ACC_FP_OFF(p) != 0)
        return -1;
    if (DosFreeSeg(ACC_FP_SEG(p)) != 0)
        return -1;
    return 0;
}

#endif


/*
vi:ts=4:et
*/

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


/***********************************************************************
//
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrpbrk) (const acc_hchar_p s, const acc_hchar_p accept)
{
    for ( ; *s; ++s) {
        const acc_hchar_p a = accept;
        for ( ; *a; ++a) {
            if (*s == *a) {
#if (ACC_CC_GNUC)
                union { acc_hchar_p p; const acc_hchar_p c; } u;
                u.c = s; return u.p; /* UNCONST */
#else
                return (acc_hchar_p)s; /* UNCONST */
#endif
            }
        }
    }
    return (acc_hchar_p)0;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_hstrsep) (acc_hchar_pp ss, const acc_hchar_p delim)
{
    acc_hchar_p s = *ss;
    if (s) {
        acc_hchar_p p = __ACCLIB_FUNCNAME(acc_hstrpbrk)(s, delim);
        if (p) *p++ = 0;
        *ss = p;
    }
    return s;
}


/*
vi:ts=4:et
*/

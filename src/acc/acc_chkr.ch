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


/*************************************************************************
//
**************************************************************************/

#if !defined(__ACCCHKR_FUNCNAME)
#  define __ACCCHKR_FUNCNAME(f) accchkr_ ## f
#endif



#if !defined(ACCCHKR_ASSERT)

#if 0 || defined(ACCCHKR_CONFIG_DEBUG)

#include <stdio.h>
static int __ACCCHKR_FUNCNAME(assert_fail)(const char* s, unsigned l)
{
    fprintf(stderr, "ACCCHKR assertion failed in line %u: `%s'\n", l, s);
    return 0;
}
#define ACCCHKR_ASSERT(expr)    ((expr) ? 1 : __ACCCHKR_FUNCNAME(assert_fail)(#expr,__LINE__))

#else

#define ACCCHKR_ASSERT(expr)    ((expr) ? 1 : 0)

#endif

#endif



/* avoid inlining */
static int __ACCCHKR_FUNCNAME(schedule_insns_bug)(void);
static int __ACCCHKR_FUNCNAME(strength_reduce_bug)(int*);


/*************************************************************************
// main entry
**************************************************************************/

static int __ACCCHKR_FUNCNAME(check)(int r)
{
#if defined(ACC_ENDIAN_BIG_ENDIAN)
    {
        union { long l; unsigned char c[sizeof(long)]; } u;
        u.l = 0; u.c[sizeof(long)-1] = 0x80;
        r &= ACCCHKR_ASSERT(u.l == 128);
        u.l = 0; u.c[0] = 0x80;
        r &= ACCCHKR_ASSERT(u.l < 0);
        ACC_UNUSED(u);
    }
#endif
#if defined(ACC_ENDIAN_LITTLE_ENDIAN)
    {
        union { long l; unsigned char c[sizeof(long)]; } u;
        u.l = 0; u.c[0] = 0x80;
        r &= ACCCHKR_ASSERT(u.l == 128);
        u.l = 0; u.c[sizeof(long)-1] = 0x80;
        r &= ACCCHKR_ASSERT(u.l < 0);
        ACC_UNUSED(u);
    }
#endif

    /* check for the gcc schedule-insns optimization bug */
    if (r == 1)
    {
        r &= ACCCHKR_ASSERT(!__ACCCHKR_FUNCNAME(schedule_insns_bug()));
    }

    /* check for the gcc strength-reduce optimization bug */
    if (r == 1)
    {
        static int x[3];
        static unsigned xn = 3;
        register unsigned j;

        for (j = 0; j < xn; j++)
            x[j] = (int)j - 3;
        r &= ACCCHKR_ASSERT(!__ACCCHKR_FUNCNAME(strength_reduce_bug(x)));
    }

    return r;
}


/*************************************************************************
//
**************************************************************************/

static int __ACCCHKR_FUNCNAME(schedule_insns_bug)(void)
{
    const int a[] = {1, 2, 0}; const int* b;
    b = a; return (*b) ? 0 : 1;
}


static int __ACCCHKR_FUNCNAME(strength_reduce_bug)(int* x)
{
#if 0 && (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    ACC_UNUSED(x); return 0;
#else
    return x[0] != -3 || x[1] != -2 || x[2] != -1;
#endif
}



/*
vi:ts=4:et
*/

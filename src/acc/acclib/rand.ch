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


/*************************************************************************
// some linear congruential pseudo random number generators (PRNG)
**************************************************************************/

ACCLIB_PUBLIC(void, acc_srand31) (acc_rand31_t* r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32L_C(0xffffffff);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand31) (acc_rand31_t* r)
{
    r->seed = (r->seed * ACC_UINT32L_C(1103515245)) + 12345;
    r->seed &= ACC_UINT32L_C(0x7fffffff);
    return r->seed;
}


#if defined(acc_uint64l_t)

ACCLIB_PUBLIC(void, acc_srand48) (acc_rand48_t* r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32L_C(0xffffffff);
    r->seed <<= 16; r->seed |= 0x330e;
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand48) (acc_rand48_t* r)
{
    r->seed = (r->seed * ACC_UINT64L_C(25214903917)) + 11;
    r->seed &= ACC_UINT64L_C(0xffffffffffff);
    return (acc_uint32l_t) (r->seed >> 17);
}

#endif /* defined(acc_uint64l_t) */


#if defined(acc_uint64l_t)

ACCLIB_PUBLIC(void, acc_srand64) (acc_rand64_t* r, acc_uint64l_t seed)
{
    r->seed = seed & ACC_UINT64L_C(0xffffffffffffffff);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand64) (acc_rand64_t* r)
{
    r->seed = (r->seed * ACC_UINT64L_C(6364136223846793005)) + 1;
    r->seed &= ACC_UINT64L_C(0xffffffffffffffff);
    return (acc_uint32l_t) (r->seed >> 33);
}

#endif /* defined(acc_uint64l_t) */


/*
vi:ts=4:et
*/

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


#define __ACCLIB_RAND_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/*************************************************************************
// some linear congruential pseudo random number generators (PRNG)
**************************************************************************/

ACCLIB_PUBLIC(void, acc_srand31) (acc_rand31_p r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32L_C(0xffffffff);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand31) (acc_rand31_p r)
{
    r->seed = r->seed * ACC_UINT32L_C(1103515245) + 12345;
    r->seed &= ACC_UINT32L_C(0x7fffffff);
    return r->seed;
}


#if defined(acc_uint64l_t)

ACCLIB_PUBLIC(void, acc_srand48) (acc_rand48_p r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32L_C(0xffffffff);
    r->seed <<= 16; r->seed |= 0x330e;
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand48) (acc_rand48_p r)
{
    r->seed = r->seed * ACC_UINT64L_C(25214903917) + 11;
    r->seed &= ACC_UINT64L_C(0xffffffffffff);
    return (acc_uint32l_t) (r->seed >> 17);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand48_r32) (acc_rand48_p r)
{
    r->seed = r->seed * ACC_UINT64L_C(25214903917) + 11;
    r->seed &= ACC_UINT64L_C(0xffffffffffff);
    return (acc_uint32l_t) (r->seed >> 16);
}

#endif /* acc_uint64l_t */


#if defined(acc_uint64l_t)

ACCLIB_PUBLIC(void, acc_srand64) (acc_rand64_p r, acc_uint64l_t seed)
{
    r->seed = seed & ACC_UINT64L_C(0xffffffffffffffff);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand64) (acc_rand64_p r)
{
    r->seed = r->seed * ACC_UINT64L_C(6364136223846793005) + 1;
#if (SIZEOF_ACC_INT64L_T > 8)
    r->seed &= ACC_UINT64L_C(0xffffffffffffffff);
#endif
    return (acc_uint32l_t) (r->seed >> 33);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_rand64_r32) (acc_rand64_p r)
{
    r->seed = r->seed * ACC_UINT64L_C(6364136223846793005) + 1;
#if (SIZEOF_ACC_INT64L_T > 8)
    r->seed &= ACC_UINT64L_C(0xffffffffffffffff);
#endif
    return (acc_uint32l_t) (r->seed >> 32);
}

#endif /* acc_uint64l_t */


/*************************************************************************
// mersenne twister PRNG
**************************************************************************/

ACCLIB_PUBLIC(void, acc_srandmt) (acc_randmt_p r, acc_uint32l_t seed)
{
    unsigned i = 0;
    do {
        r->s[i++] = (seed &= ACC_UINT32L_C(0xffffffff));
        seed ^= seed >> 30;
        seed = seed * ACC_UINT32L_C(0x6c078965) + i;
    } while (i != 624);
    r->n = i;
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_randmt) (acc_randmt_p r)
{
    return (__ACCLIB_FUNCNAME(acc_randmt_r32)(r)) >> 1;
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_randmt_r32) (acc_randmt_p r)
{
    acc_uint32l_t v;
    if (r->n == 624) {
        int i = 0, j;
        r->n = 0;
        do {
            j = i - 623; if (j < 0) j += 624;
            v = (r->s[i] & ACC_UINT32L_C(0x80000000)) ^ (r->s[j] & ACC_UINT32L_C(0x7fffffff));
            j = i - 227; if (j < 0) j += 624;
            r->s[i] = r->s[j] ^ (v >> 1);
            if (v & 1) r->s[i] ^= ACC_UINT32L_C(0x9908b0df);
        } while (++i != 624);
    }
    v = r->s[r->n++];
    v ^= v >> 11; v ^= (v & ACC_UINT32L_C(0x013a58ad)) << 7;
    v ^= (v & ACC_UINT32L_C(0x0001df8c)) << 15; v ^= v >> 18;
    return v;
}


/*
vi:ts=4:et
*/

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


#define __ACCLIB_RDTSC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif

#if defined(acc_int32e_t)


#if ((ACC_ARCH_AMD64 || ACC_ARCH_IA32) && ACC_CC_GNUC)
#  if (ACC_CC_GNUC >= 0x020000ul)
#    define __ACCLIB_RDTSC_REGS : : "r" (t) : "cc", "memory", "eax", "edx"
#  else
#    define __ACCLIB_RDTSC_REGS : : "r" (t) : "ax", "dx"
#  endif
#elif (ACC_ARCH_IA32 && ACC_CC_INTELC) && defined(__linux__)
#  define __ACCLIB_RDTSC_REGS   : : "r" (t) : "memory", "eax", "edx"
#endif


/*************************************************************************
// read TSC
**************************************************************************/

ACCLIB_PUBLIC(int, acc_tsc_read) (acc_uint32e_t* t)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32) && defined(__ACCLIB_RDTSC_REGS)
    __asm__ __volatile__(
        "clc \n" ".byte 0x0f, 0x31\n"
        "movl %%eax,(%0)\n" "movl %%edx,4(%0)\n"
        __ACCLIB_RDTSC_REGS
    );
    return 0;
#elif (ACC_ARCH_IA32 && (ACC_OS_DOS32 || ACC_OS_WIN32) && (ACC_CC_DMC || ACC_CC_INTELC || ACC_CC_MSC || ACC_CC_PELLESC))
    ACC_UNUSED(t);
    __asm {
        mov ecx, t
        clc
#  if (ACC_CC_MSC && (_MSC_VER < 1200))
        _emit 0x0f
        _emit 0x31
#  else
        rdtsc
#  endif
        mov [ecx], eax
        mov [ecx+4], edx
    }
    return 0;
#else
    t[0] = t[1] = 0;
    return -1;
#endif
}


/*************************************************************************
// read and add TSC
**************************************************************************/

ACCLIB_PUBLIC(int, acc_tsc_read_add) (acc_uint32e_t* t)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32) && defined(__ACCLIB_RDTSC_REGS)
    __asm__ __volatile__(
        "clc \n" ".byte 0x0f, 0x31\n"
        "addl %%eax,(%0)\n" "adcl $0,%%edx\n" "addl %%edx,4(%0)\n"
        __ACCLIB_RDTSC_REGS
    );
    return 0;
#elif (ACC_ARCH_IA32 && (ACC_OS_DOS32 || ACC_OS_WIN32) && (ACC_CC_DMC || ACC_CC_INTELC || ACC_CC_MSC || ACC_CC_PELLESC))
    ACC_UNUSED(t);
    __asm {
        mov ecx, t
        clc
#  if (ACC_CC_MSC && (_MSC_VER < 1200))
        _emit 0x0f
        _emit 0x31
#  else
        rdtsc
#  endif
        add [ecx], eax
        adc edx, 0
        add [ecx+4], edx
    }
    return 0;
#else
    acc_uint32e_t v[2];
    int r;
    r = acc_tsc_read(v);
    t[0] += v[0];
    if (t[0] < v[0]) t[1] += 1;
    t[1] += v[1];
    return r;
#endif
}


#endif /* defined(acc_int32e_t) */


/*
vi:ts=4:et
*/

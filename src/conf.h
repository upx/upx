/* conf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_CONF_H
#define __UPX_CONF_H 1

#include "version.h"


/*************************************************************************
// ACC
**************************************************************************/

#if (defined(_WIN32) || defined(_WIN64)) && defined(_MSC_VER)
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif

#include "miniacc.h"
#if ((ACC_OS_WIN32 || ACC_OS_WIN64) && ACC_CC_MWERKS) && defined(__MSL__)
#  undef HAVE_UTIME_H /* this pulls in <windows.h> */
#endif
// FIXME - quick hack for arm-wince-gcc-3.4 (Debian pocketpc-*.deb packages)
#if 1 && (ACC_ARCH_ARM) && defined(__pe__) && !defined(_WIN32)
#  undef HAVE_CHMOD
#  undef HAVE_CHOWN
#  undef HAVE_LSTAT
#  undef HAVE_UTIME
#endif


// pragmas
#if (ACC_CC_BORLANDC)
#  if (__BORLANDC__ < 0x0500)
#    error "need Borland C++ 5.0 or newer"
#  endif
#  pragma warn -aus     // 8004: 'x' is assigned a value that is never used
#  pragma warn -inl     // 8026+8027: Function not expanded inline
   // Borland compilers typically produce a number of bogus warnings, and
   // the actual diagnostics vary from version to version...
#  if (__BORLANDC__ < 0x0530)
#    pragma warn -csu   // 8012: Comparing signed and unsigned values
#  endif
#  if (__BORLANDC__ >= 0x0530 && __BORLANDC__ < 0x0560)
#    pragma warn -osh   // 8055: Possible overflow in shift operation
#  endif
#  if (__BORLANDC__ >= 0x0560)
#    pragma warn -use   // 8080: 'x' is declared but never used
#  endif
#elif (ACC_CC_DMC)
#  if (__DMC__ < 0x829)
#    error "need Digital Mars C++ 8.29 or newer"
#  endif
#elif (ACC_CC_INTELC)
#  if (__INTEL_COMPILER < 450)
#    error "need Intel C++ 4.5 or newer"
#  endif
#  if (ACC_OS_WIN32 || ACC_OS_WIN64)
#  elif defined(__linux__)
#    pragma warning(error: 424)         // #424: extra ";" ignored
//#    pragma warning(disable: 128)       // #128: loop is not reachable from preceding code
//#    pragma warning(disable: 181)       // #181: argument is incompatible with corresponding format string conversion
#    pragma warning(disable: 193)       // #193: zero used for undefined preprocessing identifier
#    pragma warning(disable: 810)       // #810: conversion from "A" to "B" may lose significant bits
#    pragma warning(disable: 981)       // #981: operands are evaluated in unspecified order
#    pragma warning(disable: 1418)      // #1418: external function definition with no prior declaration
//#    pragma warning(disable: 1419)      // #1419: external declaration in primary source file
#  else
#    error "untested platform"
#  endif
#elif (ACC_CC_MSC)
#  if (_MSC_VER < 1100)
#    error "need Visual C++ 5.0 or newer"
#  endif
#  pragma warning(error: 4096)          // W2: '__cdecl' must be used with '...'
#  pragma warning(error: 4138)          // Wx: '*/' found outside of comment
#  pragma warning(disable: 4097)        // W3: typedef-name 'A' used as synonym for class-name 'B'
#  pragma warning(disable: 4511)        // W3: 'class': copy constructor could not be generated
#  pragma warning(disable: 4512)        // W4: 'class': assignment operator could not be generated
#  pragma warning(disable: 4514)        // W4: 'function': unreferenced inline function has been removed
#  pragma warning(disable: 4710)        // W4: 'function': function not inlined
#  if (_MSC_VER >= 1300)
#    pragma warning(disable: 4625)      // W4: 'class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#    pragma warning(disable: 4626)      // W4: 'class' : assignment operator could not be generated because a base class assignment operator is inaccessible
#    pragma warning(disable: 4711)      // W4: 'function' selected for automatic inline expansion
#    pragma warning(disable: 4820)      // W4: 'struct' : 'x' bytes padding added after member 'member'
#  endif
#  if (_MSC_VER >= 1400)
#    pragma warning(disable: 4996)      // W1: 'function': was declared deprecated
#  endif
#elif (ACC_CC_SUNPROC)
//#  pragma error_messages(off,"badargtype2w")    // FIXME
#elif (ACC_CC_WATCOMC)
#  if (__WATCOMC__ < 1280)
#    error "need Open Watcom C++ 1.8 or newer"  // because earlier versions do not support nested classes
#  endif
#  if defined(__cplusplus)
#    pragma warning 367 9               // w3: conditional expression in if statement is always true
#    pragma warning 368 9               // w3: conditional expression in if statement is always false
#    pragma warning 389 9               // w3: integral value may be truncated
#    pragma warning 656 9               // w5: define this function inside its class definition (may improve code quality)
#  endif
#endif


#define ACC_WANT_ACC_INCD_H 1
#define ACC_WANT_ACC_INCE_H 1
#define ACC_WANT_ACC_LIB_H 1
#define ACC_WANT_ACC_CXX_H 1
#include "miniacc.h"
#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if defined(INVALID_HANDLE_VALUE) || defined(MAKEWORD) || defined(RT_CURSOR)
#    error "something pulled in <windows.h>"
#  endif
#endif

/* intergral types */
typedef acc_int8_t      upx_int8_t;
typedef acc_uint8_t     upx_uint8_t;
typedef acc_int16_t     upx_int16_t;
typedef acc_uint16_t    upx_uint16_t;
typedef acc_int32_t     upx_int32_t;
typedef acc_uint32_t    upx_uint32_t;
typedef acc_int64_t     upx_int64_t;
typedef acc_uint64_t    upx_uint64_t;
typedef acc_uintptr_t   upx_uintptr_t;
#define UPX_INT16_C     ACC_INT16_C
#define UPX_UINT16_C    ACC_UINT16_C
#define UPX_INT32_C     ACC_INT32_C
#define UPX_UINT32_C    ACC_UINT32_C
#define UPX_INT64_C     ACC_INT64_C
#define UPX_UINT64_C    ACC_UINT64_C


/*************************************************************************
//
**************************************************************************/

#if defined(__linux__) && !defined(__unix__)
#  define __unix__ 1
#endif

// just in case
#undef _
#undef __
#undef ___
#undef NDEBUG
#undef dos
#undef linux
#undef small
#undef tos
#if (ACC_CC_DMC)
#  undef tell
#endif
#if !(ACC_CC_PGI)
#  undef unix
#endif
#if defined(__DJGPP__)
#  undef sopen
#  undef __unix__
#  undef __unix
#endif

#if !defined(WITH_UCL)
#  define WITH_UCL 1
#endif
#if 0 && !defined(WITH_LZMA)
#  define WITH_LZMA 1
#endif
#if 1 && (ACC_CC_WATCOMC)
#  undef WITH_LZMA
#endif
#if defined(UPX_OFFICIAL_BUILD)
#  if !(WITH_LZMA) || !(WITH_NRV) || !(WITH_UCL)
#    error
#  endif
#endif
#if (WITH_NRV)
#  include <nrv/nrvconf.h>
#endif
#if (WITH_UCL)
#  define ucl_compress_config_t REAL_ucl_compress_config_t
#  include <ucl/uclconf.h>
#  include <ucl/ucl.h>
#  if !defined(UCL_VERSION) || (UCL_VERSION < 0x010300L)
#    error "please upgrade your UCL installation"
#  endif
#  undef ucl_compress_config_t
#  undef ucl_compress_config_p
#endif
#if !defined(UINT_MAX) || (UINT_MAX < 0xffffffffL)
#  error "UINT_MAX"
#endif

#define upx_byte                    unsigned char
#define upx_bytep                   upx_byte *


/*************************************************************************
// system includes
**************************************************************************/

// malloc debuggers
#if (WITH_VALGRIND)
#  include <valgrind/memcheck.h>
#endif
#if !defined(VALGRIND_MAKE_MEM_DEFINED)
#  define VALGRIND_MAKE_MEM_DEFINED(addr,len)   0
#endif
#if !defined(VALGRIND_MAKE_MEM_NOACCESS)
#  define VALGRIND_MAKE_MEM_NOACCESS(addr,len)  0
#endif
#if !defined(VALGRIND_MAKE_MEM_UNDEFINED)
#  define VALGRIND_MAKE_MEM_UNDEFINED(addr,len) 0
#endif


// unconditionally turn on assertions
#undef NDEBUG
#include <assert.h>


/*************************************************************************
// portab
**************************************************************************/

#ifndef STDIN_FILENO
#  define STDIN_FILENO      (fileno(stdin))
#endif
#ifndef STDOUT_FILENO
#  define STDOUT_FILENO     (fileno(stdout))
#endif
#ifndef STDERR_FILENO
#  define STDERR_FILENO     (fileno(stderr))
#endif


#if !(HAVE_STRCASECMP) && (HAVE_STRICMP)
#  define strcasecmp        stricmp
#endif
#if !(HAVE_STRNCASECMP) && (HAVE_STRNICMP)
#  define strncasecmp       strnicmp
#endif


#if !defined(S_IWUSR) && defined(_S_IWUSR)
#  define S_IWUSR           _S_IWUSR
#elif !defined(S_IWUSR) && defined(_S_IWRITE)
#  define S_IWUSR           _S_IWRITE
#endif

#if !defined(S_IFMT) && defined(_S_IFMT)
#  define S_IFMT            _S_IFMT
#endif
#if !defined(S_IFREG) && defined(_S_IFREG)
#  define S_IFREG           _S_IFREG
#endif
#if !defined(S_IFDIR) && defined(_S_IFDIR)
#  define S_IFDIR           _S_IFDIR
#endif
#if !defined(S_IFCHR) && defined(_S_IFCHR)
#  define S_IFCHR           _S_IFCHR
#endif

#if !defined(S_ISREG)
#  if defined(S_IFMT) && defined(S_IFREG)
#    define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#  else
#    error "S_ISREG"
#  endif
#endif
#if !defined(S_ISDIR)
#  if defined(S_IFMT) && defined(S_IFDIR)
#    define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#  else
#    error "S_ISDIR"
#  endif
#endif
#if !defined(S_ISCHR)
#  if defined(S_IFMT) && defined(S_IFCHR)
#    define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#  endif
#endif


// avoid warnings about shadowing global functions
#undef index
#define basename            upx_basename
#define index               upx_index
#define outp                upx_outp

#undef PAGE_MASK
#undef PAGE_SIZE

#if !defined(O_BINARY)
#  define O_BINARY  0
#endif

#ifndef OPTIONS_VAR
#  define OPTIONS_VAR   "UPX"
#endif


#if (ACC_CC_INTELC && (__INTEL_COMPILER < 800))
#elif (0 && (ACC_ARCH_AMD64 || ACC_ARCH_I386))
#elif (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_INTELC_GNUC || ACC_CC_PATHSCALE)
#  define __packed_struct(s)        struct s {
#  define __packed_struct_end()     } __attribute__((__packed__,__aligned__(1)));
#elif (ACC_CC_WATCOMC)
#  define __packed_struct(s)        _Packed struct s {
#  define __packed_struct_end()     };
#endif
#if !defined(__packed_struct)
#  define __packed_struct(s)        struct s {
#  define __packed_struct_end()     };
#endif


/*************************************************************************
//
**************************************************************************/

#define UNUSED(var)              ACC_UNUSED(var)
#define COMPILE_TIME_ASSERT(e)   ACC_COMPILE_TIME_ASSERT(e)

#define __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b) { \
     typedef a acc_tmp_a_t; typedef b acc_tmp_b_t; \
     __packed_struct(acc_tmp_t) acc_tmp_b_t x; acc_tmp_a_t y; acc_tmp_b_t z; __packed_struct_end() \
     COMPILE_TIME_ASSERT(sizeof(struct acc_tmp_t) == 2*sizeof(b)+sizeof(a)) \
     COMPILE_TIME_ASSERT(sizeof(((acc_tmp_t*)0)->x)+sizeof(((acc_tmp_t*)0)->y)+sizeof(((acc_tmp_t*)0)->z) == 2*sizeof(b)+sizeof(a)) \
   }
#if defined(__acc_alignof)
#  define __COMPILE_TIME_ASSERT_ALIGNOF(a,b) \
     __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b) \
     COMPILE_TIME_ASSERT(__acc_alignof(a) == sizeof(b))
#else
#  define __COMPILE_TIME_ASSERT_ALIGNOF(a,b) \
     __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b)
#endif
#define COMPILE_TIME_ASSERT_ALIGNED1(a)     __COMPILE_TIME_ASSERT_ALIGNOF(a,char)

#define TABLESIZE(table)    ((sizeof(table)/sizeof((table)[0])))


#if 0
#define ALIGN_DOWN(a,b)     (((a) / (b)) * (b))
#define ALIGN_UP(a,b)       ALIGN_DOWN((a) + ((b) - 1), b)
#define ALIGN_GAP(a,b)      (ALIGN_UP(a,b) - (a))
#elif 1
template <class T>
inline T ALIGN_DOWN(const T& a, const T& b) { T r; r = (a / b) * b; return r; }
template <class T>
inline T ALIGN_UP  (const T& a, const T& b) { T r; r = ((a + b - 1) / b) * b; return r; }
template <class T>
inline T ALIGN_GAP (const T& a, const T& b) { T r; r = ALIGN_UP(a, b) - a; return r; }
#else
inline unsigned ALIGN_DOWN(unsigned a, unsigned b) { return (a / b) * b; }
inline unsigned ALIGN_UP  (unsigned a, unsigned b) { return ((a + b - 1) / b) * b; }
inline unsigned ALIGN_GAP (unsigned a, unsigned b) { return ALIGN_UP(a, b) - a; }
#endif

#if 0
#define UPX_MAX(a,b)        ((a) >= (b) ? (a) : (b))
#define UPX_MIN(a,b)        ((a) <= (b) ? (a) : (b))
#elif 1
template <class T>
inline const T& UPX_MAX(const T& a, const T& b) { if (a < b) return b; return a; }
template <class T>
inline const T& UPX_MIN(const T& a, const T& b) { if (a < b) return a; return b; }
#else
inline unsigned UPX_MAX(unsigned a, unsigned b) { return a < b ? b : a; }
inline unsigned UPX_MIN(unsigned a, unsigned b) { return a < b ? a : b; }
#endif


// An Array allocates memory on the heap, but automatically
// gets destructed when leaving scope or on exceptions.
#define Array(type, var, size) \
    assert((int)(size) > 0); \
    MemBuffer var ## _membuf((size)*(sizeof(type))); \
    type * const var = ((type *) var ## _membuf.getVoidPtr())

#define ByteArray(var, size)    Array(unsigned char, var, size)

struct noncopyable
{
protected:
    inline noncopyable() {}
    inline ~noncopyable() {}
private:
    noncopyable(const noncopyable &); // undefined
    const noncopyable& operator=(const noncopyable &); // undefined
};


/*************************************************************************
// constants
**************************************************************************/

/* exit codes of this program: 0 ok, 1 error, 2 warning */
#define EXIT_OK         0
#define EXIT_ERROR      1
#define EXIT_WARN       2

#define EXIT_USAGE      1
#define EXIT_FILE_READ  1
#define EXIT_FILE_WRITE 1
#define EXIT_MEMORY     1
#define EXIT_CHECKSUM   1
#define EXIT_INIT       1
#define EXIT_INTERNAL   1


// magic constants for patching
#define UPX_MAGIC_LE32          0x21585055      /* "UPX!" */
#define UPX_MAGIC2_LE32         0xD5D0D8A1


// upx_compress() error codes
#define UPX_E_OK                    (0)
#define UPX_E_ERROR                 (-1)
#define UPX_E_OUT_OF_MEMORY         (-2)
#define UPX_E_NOT_COMPRESSIBLE      (-3)
#define UPX_E_INPUT_OVERRUN         (-4)
#define UPX_E_OUTPUT_OVERRUN        (-5)
#define UPX_E_LOOKBEHIND_OVERRUN    (-6)
#define UPX_E_EOF_NOT_FOUND         (-7)
#define UPX_E_INPUT_NOT_CONSUMED    (-8)
#define UPX_E_NOT_YET_IMPLEMENTED   (-9)
#define UPX_E_INVALID_ARGUMENT      (-10)


// Executable formats. Note: big endian types are >= 128.
#define UPX_F_DOS_COM           1
#define UPX_F_DOS_SYS           2
#define UPX_F_DOS_EXE           3
#define UPX_F_DJGPP2_COFF       4
#define UPX_F_WATCOM_LE         5
#define UPX_F_VXD_LE            6
#define UPX_F_DOS_EXEH          7               /* OBSOLETE */
#define UPX_F_TMT_ADAM          8
#define UPX_F_WIN32_PE          9
#define UPX_F_LINUX_i386        10
#define UPX_F_WIN16_NE          11
#define UPX_F_LINUX_ELF_i386    12
#define UPX_F_LINUX_SEP_i386    13
#define UPX_F_LINUX_SH_i386     14
#define UPX_F_VMLINUZ_i386      15
#define UPX_F_BVMLINUZ_i386     16
#define UPX_F_ELKS_8086         17
#define UPX_F_PS1_EXE           18
#define UPX_F_VMLINUX_i386      19
#define UPX_F_LINUX_ELFI_i386   20
#define UPX_F_WINCE_ARM_PE      21
#define UPX_F_LINUX_ELF64_AMD   22
#define UPX_F_LINUX_ELF32_ARMEL 23
#define UPX_F_BSD_i386          24
#define UPX_F_BSD_ELF_i386      25
#define UPX_F_BSD_SH_i386       26

#define UPX_F_VMLINUX_AMD64     27
#define UPX_F_VMLINUX_ARMEL     28
#define UPX_F_MACH_i386         29
#define UPX_F_LINUX_ELF32_MIPSEL 30
#define UPX_F_VMLINUZ_ARMEL     31
#define UPX_F_MACH_ARMEL        32

#define UPX_F_DYLIB_i386        33
#define UPX_F_MACH_AMD64        34
#define UPX_F_DYLIB_AMD64       35

#define UPX_F_WIN64_PEP         36

#define UPX_F_PLAIN_TEXT        127

#define UPX_F_ATARI_TOS         129
#define UPX_F_SOLARIS_SPARC     130
#define UPX_F_MACH_PPC32        131
#define UPX_F_LINUX_ELFPPC32    132
#define UPX_F_LINUX_ELF32_ARMEB 133
#define UPX_F_MACH_FAT          134
#define UPX_F_VMLINUX_ARMEB     135
#define UPX_F_VMLINUX_PPC32     136
#define UPX_F_LINUX_ELF32_MIPSEB 137
#define UPX_F_DYLIB_PPC32       138


// compression methods
#define M_ALL           (-1)
#define M_END           (-2)
#define M_NONE          (-3)
#define M_SKIP          (-4)
#define M_ULTRA_BRUTE   (-5)
// compression methods - DO NOT CHANGE
#define M_NRV2B_LE32    2
#define M_NRV2B_8       3
#define M_NRV2B_LE16    4
#define M_NRV2D_LE32    5
#define M_NRV2D_8       6
#define M_NRV2D_LE16    7
#define M_NRV2E_LE32    8
#define M_NRV2E_8       9
#define M_NRV2E_LE16    10
//#define M_CL1B_LE32     11
//#define M_CL1B_8        12
//#define M_CL1B_LE16     13
#define M_LZMA          14
#define M_DEFLATE       15      /* zlib */

#define M_IS_NRV2B(x)   ((x) >= M_NRV2B_LE32 && (x) <= M_NRV2B_LE16)
#define M_IS_NRV2D(x)   ((x) >= M_NRV2D_LE32 && (x) <= M_NRV2D_LE16)
#define M_IS_NRV2E(x)   ((x) >= M_NRV2E_LE32 && (x) <= M_NRV2E_LE16)
//#define M_IS_CL1B(x)    ((x) >= M_CL1B_LE32  && (x) <= M_CL1B_LE16)
#define M_IS_LZMA(x)    (((x) & 255) == M_LZMA)
#define M_IS_DEFLATE(x) ((x) == M_DEFLATE)


// filters
#define FT_END          (-1)
#define FT_NONE         (-2)
#define FT_SKIP         (-3)
#define FT_ULTRA_BRUTE  (-4)


/*************************************************************************
// compression - callback_t
**************************************************************************/

struct upx_callback_t;
#define upx_callback_p upx_callback_t *
#if 0
typedef void* (__acc_cdecl *upx_alloc_func_t)
    (upx_callback_p self, unsigned items, unsigned size);
typedef void      (__acc_cdecl *upx_free_func_t)
    (upx_callback_p self, void* ptr);
#endif
typedef void (__acc_cdecl *upx_progress_func_t)
    (upx_callback_p, unsigned, unsigned);

struct upx_callback_t
{
    upx_progress_func_t nprogress;
    void *user;

    void reset() { memset(this, 0, sizeof(*this)); }
};


/*************************************************************************
// compression - config_t
**************************************************************************/

template <class T, T default_value, T min_value, T max_value>
struct OptVar
{
    typedef T Type;
    static const T default_value_c = default_value;
    static const T min_value_c = min_value;
    static const T max_value_c = max_value;

    void assertValue() {
        // FIXME: this generates annoying warnings "unsigned >= 0 is always true"
        //assert((v >= min_value) && (v <= max_value));
    }

    OptVar() : v(default_value), is_set(0) { }
    OptVar& operator= (const T other) {
        v = other; is_set += 1;
        assertValue();
        return *this;
    }
#if 0
#error
    // there is too much implicit magic in this copy operator;
    // better introduce an explicit "oassign" function.
    OptVar& operator= (const OptVar& other) {
        if (other.is_set) { v = other.v; is_set += 1; }
        assertValue(); return *this;
    }
#endif

    void reset() { v = default_value; is_set = 0; }
    operator T () const { return v; }

    T v;
    unsigned is_set;
};


// optional assignments
template <class T> inline void oassign(T& self, const T& other) {
    if (other.is_set) { self.v = other.v; self.is_set += 1; }
}
#if 0
template <class V, class T> inline void oassign(V& v, const T& other) {
    if (other.is_set) { v = other.v; }
}
#else
template <class T> inline void oassign(unsigned& v, const T& other) {
    if (other.is_set) { v = other.v; }
}
#endif


struct lzma_compress_config_t
{
    typedef OptVar<unsigned,  2u, 0u,   4u> pos_bits_t;             // pb
    typedef OptVar<unsigned,  0u, 0u,   4u> lit_pos_bits_t;         // lb
    typedef OptVar<unsigned,  3u, 0u,   8u> lit_context_bits_t;     // lc
    typedef OptVar<unsigned, (1u<<22), 1u, (1u<<30) > dict_size_t;
    typedef OptVar<unsigned, 64u, 5u, 273u> num_fast_bytes_t;

    pos_bits_t          pos_bits;           // pb
    lit_pos_bits_t      lit_pos_bits;       // lp
    lit_context_bits_t  lit_context_bits;   // lc
    dict_size_t         dict_size;
    unsigned            fast_mode;
    num_fast_bytes_t    num_fast_bytes;
    unsigned            match_finder_cycles;

    unsigned            max_num_probs;

    void reset();
};


struct ucl_compress_config_t : public REAL_ucl_compress_config_t
{
    void reset() { memset(this, 0xff, sizeof(*this)); }
};


struct zlib_compress_config_t
{
    typedef OptVar<unsigned,  8u, 1u,   9u> mem_level_t;            // ml
    typedef OptVar<unsigned, 15u, 9u,  15u> window_bits_t;          // wb
    typedef OptVar<unsigned,  0u, 0u,   4u> strategy_t;             // st

    mem_level_t         mem_level;          // ml
    window_bits_t       window_bits;        // wb
    strategy_t          strategy;           // st

    void reset();
};


struct upx_compress_config_t
{
    lzma_compress_config_t  conf_lzma;
    ucl_compress_config_t   conf_ucl;
    zlib_compress_config_t  conf_zlib;
    void reset() { conf_lzma.reset(); conf_ucl.reset(); conf_zlib.reset(); }
};

#define NULL_cconf  ((upx_compress_config_t *) NULL)


/*************************************************************************
// compression - result_t
**************************************************************************/

struct lzma_compress_result_t
{
    unsigned pos_bits;              // pb
    unsigned lit_pos_bits;          // lp
    unsigned lit_context_bits;      // lc
    unsigned dict_size;
    unsigned fast_mode;
    unsigned num_fast_bytes;
    unsigned match_finder_cycles;
    unsigned num_probs;             // (computed result)

    void reset() { memset(this, 0, sizeof(*this)); }
};


struct ucl_compress_result_t
{
    ucl_uint result[16];

    void reset() { memset(this, 0, sizeof(*this)); }
};


struct zlib_compress_result_t
{
    unsigned dummy;

    void reset() { memset(this, 0, sizeof(*this)); }
};


struct upx_compress_result_t
{
    // debug
    int method, level;
    unsigned u_len, c_len;

    lzma_compress_result_t  result_lzma;
    ucl_compress_result_t   result_ucl;
    zlib_compress_result_t  result_zlib;

    void reset() {
        memset(this, 0, sizeof(*this));
        result_lzma.reset(); result_ucl.reset(); result_zlib.reset();
    }
};


/*************************************************************************
// globals
**************************************************************************/

#include "snprintf.h"

#if defined(__cplusplus)

#include "stdcxx.h"
#include "options.h"
#include "except.h"
#include "bele.h"
#include "util.h"
#include "console.h"


// classes
class ElfLinker;
typedef ElfLinker Linker;


// main.cpp
extern const char *progname;
bool set_ec(int ec);
#if (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
void e_exit(int ec) __attribute__((__noreturn__));
#else
void e_exit(int ec);
#endif


// msg.cpp
void printSetNl(int need_nl);
void printClearLine(FILE *f = NULL);
void printErr(const char *iname, const Throwable *e);
void printUnhandledException(const char *iname, const std::exception *e);
#if (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
void __acc_cdecl_va printErr(const char *iname, const char *format, ...)
        __attribute__((__format__(__printf__,2,3)));
void __acc_cdecl_va printWarn(const char *iname, const char *format, ...)
        __attribute__((__format__(__printf__,2,3)));
#else
void __acc_cdecl_va printErr(const char *iname, const char *format, ...);
void __acc_cdecl_va printWarn(const char *iname, const char *format, ...);
#endif

#if (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
void __acc_cdecl_va infoWarning(const char *format, ...)
        __attribute__((__format__(__printf__,1,2)));
void __acc_cdecl_va infoHeader(const char *format, ...)
        __attribute__((__format__(__printf__,1,2)));
void __acc_cdecl_va info(const char *format, ...)
        __attribute__((__format__(__printf__,1,2)));
#else
void __acc_cdecl_va infoWarning(const char *format, ...);
void __acc_cdecl_va infoHeader(const char *format, ...);
void __acc_cdecl_va info(const char *format, ...);
#endif
void infoHeader();
void infoWriting(const char *what, long size);


// work.cpp
void do_one_file(const char *iname, char *oname);
void do_files(int i, int argc, char *argv[]);


// help.cpp
void show_head(void);
void show_help(int verbose=0);
void show_license(void);
void show_usage(void);
void show_version(int);


// compress.cpp
unsigned upx_adler32(const void *buf, unsigned len, unsigned adler=1);
unsigned upx_crc32(const void *buf, unsigned len, unsigned crc=0);

int upx_compress           ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult );
int upx_decompress         ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
int upx_test_overlap       ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );


#endif /* __cplusplus */


#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if defined(INVALID_HANDLE_VALUE) || defined(MAKEWORD) || defined(RT_CURSOR)
#    error "something pulled in <windows.h>"
#  endif
#endif


#endif /* already included */


/*
vi:ts=4:et
*/


/* conf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_CONF_H
#define __UPX_CONF_H

#include "version.h"


/*************************************************************************
// ACC
**************************************************************************/

#include "acc/acc.h"
#if ((ACC_OS_WIN32 || ACC_OS_WIN64) && ACC_CC_MWERKS) && defined(__MSL__)
# undef HAVE_UTIME_H /* this pulls in <windows.h> */
#endif
#include "acc/acc_incd.h"
#include "acc/acc_ince.h"
#include "acc/acc_lib.h"
#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if defined(INVALID_HANDLE_VALUE) || defined(MAKEWORD) || defined(RT_CURSOR)
#    error "something pulled in <windows.h>"
#  endif
#endif


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
#    pragma warning(disable: 193)       // #193: zero used for undefined preprocessing identifier
#    pragma warning(disable: 810)       // #810: conversion from "A" to "B" may lose significant bits
#    pragma warning(disable: 981)       // #981: operands are evaluated in unspecified order
#    pragma warning(disable: 1418)      // #1418: external definition with no prior declaration
#  else
#    error "untested platform"
#  endif
#elif (ACC_CC_MSC)
#  if (_MSC_VER < 1100)
#    error "need Visual C++ 5.0 or newer"
#  endif
#  pragma warning(error: 4096)          // W2: '__cdecl' must be used with '...'
#  pragma warning(disable: 4097)        // W3: typedef-name 'A' used as synonym for class-name 'B'
#  pragma warning(disable: 4511)        // W3: 'class': copy constructor could not be generated
#  pragma warning(disable: 4512)        // W4: 'class': assignment operator could not be generated
#  pragma warning(disable: 4514)        // W4: 'function': unreferenced inline function has been removed
#  pragma warning(disable: 4710)        // W4: 'function': function not inlined
#elif (ACC_CC_WATCOMC)
#  if (__WATCOMC__ < 1100)
#    error "need Watcom C++ 11.0c or newer"
#  endif
#  if defined(__cplusplus)
#    pragma warning 656 9               // w5: define this function inside its class definition (may improve code quality)
#  endif
#endif


/*************************************************************************
//
**************************************************************************/

#if defined(__linux__) && !defined(__unix__)
#  define __unix__ 1
#endif

// just in case
#undef NDEBUG
#undef dos
#undef linux
#undef small
#undef tos
#if defined(ACC_CC_DMC)
#  undef tell
#endif
#if !defined(ACC_CC_PGI)
#  undef unix
#endif
#if defined(__DJGPP__)
#  undef sopen
#  undef __unix__
#  undef __unix
#endif


#if !defined(WITH_UCL)
#  error "please set UCLDIR in the Makefile"
#endif
#if defined(WITH_UCL)
#  include <ucl/uclconf.h>
#  include <ucl/ucl.h>
#  if !defined(UCL_VERSION) || (UCL_VERSION < 0x010200L)
#    error "please upgrade your UCL installation"
#  endif
#  if !defined(UPX_UINT_MAX)
#    define UPX_UINT_MAX  UCL_UINT_MAX
#    define upx_uint      ucl_uint
#    define upx_voidp     ucl_voidp
#    define upx_uintp     ucl_uintp
#    define upx_byte      ucl_byte
#    define upx_bytep     ucl_bytep
#    define upx_bool      ucl_bool
#    define upx_compress_config_t   ucl_compress_config_t
#    define upx_progress_callback_t ucl_progress_callback_t
#    define UPX_E_OK      UCL_E_OK
#    define UPX_E_ERROR   UCL_E_ERROR
#    define UPX_E_OUT_OF_MEMORY UCL_E_OUT_OF_MEMORY
#    define __UPX_CDECL   __UCL_CDECL
#  endif
#endif
#if defined(WITH_NRV)
#  include <nrv/nrvconf.h>
#endif
#if !defined(__UPX_CHECKER)
#  if defined(__UCL_CHECKER) || defined(__NRV_CHECKER)
#    define __UPX_CHECKER
#  endif
#endif
#if !defined(UINT_MAX) || (UINT_MAX < 0xffffffffL)
#  error "UINT_MAX"
#endif


/*************************************************************************
// system includes
**************************************************************************/

// malloc debuggers
#if defined(WITH_VALGRIND)
#  include <valgrind/memcheck.h>
#elif defined(WITH_DMALLOC)
#  define DMALLOC_FUNC_CHECK
#  include <dmalloc.h>
#elif defined(WITH_GC)
#  define GC_DEBUG
#  include <gc/gc.h>
#  undef malloc
#  undef realloc
#  undef free
#  define malloc            GC_MALLOC
#  define realloc           GC_REALLOC
#  define free              GC_FREE
#endif

#if !defined(VALGRIND_MAKE_WRITABLE)
#  define VALGRIND_MAKE_WRITABLE(addr,len)      0
#endif
#if !defined(VALGRIND_MAKE_READABLE)
#  if 0
#    define VALGRIND_MAKE_READABLE(addr,len)    (memset(addr,0,len), 0)
#  else
#    define VALGRIND_MAKE_READABLE(addr,len)    0
#  endif
#endif
#if !defined(VALGRIND_DISCARD)
#  define VALGRIND_DISCARD(handle)              ((void)(&handle))
#endif


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


#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
#  define strcasecmp      stricmp
#endif
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
#  define strncasecmp     strnicmp
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
#define basename            upx_basename
#define index               upx_index
#define outp                upx_outp


#undef __attribute_packed
#if (ACC_CC_GNUC || ACC_CC_INTELC)
#  if (1 && (ACC_ARCH_IA32))
#    define __attribute_packed
#  else
#    define __attribute_packed    __attribute__((__packed__,__aligned__(1)))
#  endif
#else
#  define __attribute_packed
#endif

#if !defined(O_BINARY)
#  define O_BINARY  0
#endif

#ifndef OPTIONS_VAR
#  define OPTIONS_VAR   "UPX"
#endif


/*************************************************************************
//
**************************************************************************/

#define UNUSED(var)              ACC_UNUSED(var)
#define COMPILE_TIME_ASSERT(e)   ACC_COMPILE_TIME_ASSERT(e)

#if 1
#  define __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b) { \
     typedef a acc_tmp_a_t; typedef b acc_tmp_b_t; \
     struct acc_tmp_t { acc_tmp_b_t x; acc_tmp_a_t y; acc_tmp_b_t z[7]; }; \
     COMPILE_TIME_ASSERT(sizeof(struct acc_tmp_t) == 8*sizeof(b)+sizeof(a)) \
   }
#else
#  define __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b) { \
     struct acc_tmp_t { b x; a y; b z[7]; }; \
     COMPILE_TIME_ASSERT(sizeof(struct acc_tmp_t) == 8*sizeof(b)+sizeof(a)) \
   }
#endif

#if defined(acc_alignof)
#  define COMPILE_TIME_ASSERT_ALIGNOF(a,b) \
     __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b) \
     COMPILE_TIME_ASSERT(acc_alignof(a) == sizeof(b))
#else
#  define COMPILE_TIME_ASSERT_ALIGNOF(a,b) \
     __COMPILE_TIME_ASSERT_ALIGNOF_SIZEOF(a,b)
#endif

#define TABLESIZE(table)    ((sizeof(table)/sizeof((table)[0])))

#define ALIGN_DOWN(a,b)     (((a) / (b)) * (b))
#define ALIGN_UP(a,b)       ALIGN_DOWN((a) + ((b) - 1), b)
#define ALIGN_GAP(a,b)      (ALIGN_UP(a,b) - (a))

#define UPX_MAX(a,b)        ((a) >= (b) ? (a) : (b))
#define UPX_MIN(a,b)        ((a) <= (b) ? (a) : (b))
#define UPX_MAX3(a,b,c)     ((a) >= (b) ? UPX_MAX(a,c) : UPX_MAX(b,c))
#define UPX_MIN3(a,b,c)     ((a) <= (b) ? UPX_MIN(a,c) : UPX_MIN(b,c))


#if 0 && defined(__cplusplus) && !defined(new) && !defined(delete)
// global operators - debug
inline void *operator new(size_t l)
{
    void *p = malloc(l);
    printf("new   %6ld %p\n",(long)l,p);
    fflush(stdout);
    return p;
}
inline void *operator new[](size_t l)
{
    void *p = malloc(l);
    printf("new[] %6ld %p\n",(long)l,p);
    fflush(stdout);
    return p;
}
inline void operator delete(void *p)
{
    printf("delete       %p\n",p);
    fflush(stdout);
    if (p) free(p);
}
inline void operator delete[](void *p)
{
    printf("delete[]     %p\n",p);
    fflush(stdout);
    if (p) free(p);
}
#endif


// An Array allocates memory on the heap, but automatically
// gets destructed when leaving scope or on exceptions.
#define Array(type, var, size) \
    assert((int)(size) > 0); \
    MemBuffer var ## _membuf((size)*(sizeof(type))); \
    type * const var = ((type *) var ## _membuf.getVoidPtr())

#define ByteArray(var, size)    Array(unsigned char, var, size)


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

#define M_IS_NRV2B(x)   ((x) >= M_NRV2B_LE32 && (x) <= M_NRV2B_LE16)
#define M_IS_NRV2D(x)   ((x) >= M_NRV2D_LE32 && (x) <= M_NRV2D_LE16)
#define M_IS_NRV2E(x)   ((x) >= M_NRV2E_LE32 && (x) <= M_NRV2E_LE16)


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
#define UPX_F_ATARI_TOS         129
#define UPX_F_SOLARIS_SPARC     130


#define UPX_MAGIC_LE32      0x21585055          /* "UPX!" */
#define UPX_MAGIC2_LE32     0xD5D0D8A1


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


// main.cpp
extern const char *progname;
void init_options(struct options_t *o);
bool set_ec(int ec);
#if defined(__GNUC__)
void e_exit(int ec) __attribute__((__noreturn__));
#else
void e_exit(int ec);
#endif


// msg.cpp
void printSetNl(int need_nl);
void printClearLine(FILE *f = NULL);
void printErr(const char *iname, const Throwable *e);
void printUnhandledException(const char *iname, const std::exception *e);
#if defined(__GNUC__)
void __acc_cdecl_va printErr(const char *iname, const char *format, ...)
        __attribute__((__format__(printf,2,3)));
void __acc_cdecl_va printWarn(const char *iname, const char *format, ...)
        __attribute__((__format__(printf,2,3)));
#else
void __acc_cdecl_va printErr(const char *iname, const char *format, ...);
void __acc_cdecl_va printWarn(const char *iname, const char *format, ...);
#endif

#if defined(__GNUC__)
void __acc_cdecl_va infoWarning(const char *format, ...)
        __attribute__((__format__(printf,1,2)));
void __acc_cdecl_va infoHeader(const char *format, ...)
        __attribute__((__format__(printf,1,2)));
void __acc_cdecl_va info(const char *format, ...)
        __attribute__((__format__(printf,1,2)));
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
void show_help(int x = 0);
void show_license(void);
void show_usage(void);
void show_version(int);


// compress.cpp
unsigned upx_adler32(const void *buf, unsigned len, unsigned adler=1);
unsigned upx_crc32(const void *buf, unsigned len, unsigned crc=0);

int upx_compress           ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_progress_callback_t *cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf,
                                   upx_uintp result );
int upx_decompress         ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   int method );
int upx_test_overlap       ( const upx_bytep buf, upx_uint src_off,
                                   upx_uint  src_len, upx_uintp dst_len,
                                   int method );


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


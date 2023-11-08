/* conf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */

#pragma once

/*************************************************************************
// init
**************************************************************************/

#include "headers.h"
#include "version.h"

#if !defined(__has_attribute)
#define __has_attribute(x) 0
#endif
#if !defined(__has_builtin)
#define __has_builtin(x) 0
#endif
#if !defined(__has_feature)
#define __has_feature(x) 0
#endif

// reserve name "upx" for namespace
namespace upx {}

ACC_COMPILE_TIME_ASSERT_HEADER(CHAR_BIT == 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(short) == 2)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(int) == 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(long long) == 8)
// check sane compiler mandatory flags
ACC_COMPILE_TIME_ASSERT_HEADER(-1 == ~0)      // two's complement - see http://wg21.link/P0907R4
ACC_COMPILE_TIME_ASSERT_HEADER(0u - 1 == ~0u) // two's complement - see http://wg21.link/P0907R4
ACC_COMPILE_TIME_ASSERT_HEADER((1u << 31) << 1 == 0)
ACC_COMPILE_TIME_ASSERT_HEADER(((int) (1u << 31)) >> 31 == -1) // arithmetic right shift
ACC_COMPILE_TIME_ASSERT_HEADER((-1) >> 31 == -1)               // arithmetic right shift
ACC_COMPILE_TIME_ASSERT_HEADER(CHAR_MAX == 255)                // -funsigned-char
ACC_COMPILE_TIME_ASSERT_HEADER((char) (-1) == 255)

// enable some more strict warnings for Git developer builds
#if defined(UPX_CONFIG_DISABLE_WSTRICT) && (UPX_CONFIG_DISABLE_WSTRICT + 0 == 0)
#if defined(UPX_CONFIG_DISABLE_WERROR) && (UPX_CONFIG_DISABLE_WERROR + 0 == 0)
#if (ACC_CC_CLANG >= 0x0b0000)
#pragma clang diagnostic error "-Wsuggest-override"
#elif (ACC_CC_GNUC >= 0x0a0000)
// don't enable before gcc-10 because of gcc bug #78010
#pragma GCC diagnostic error "-Wsuggest-override"
#endif
#if (ACC_CC_CLANG >= 0x080000)
// don't enable before clang-8 because of stddef.h issues
#pragma clang diagnostic error "-Wzero-as-null-pointer-constant"
#elif (ACC_CC_GNUC >= 0x040700) && defined(__GLIBC__)
// Some non-GLIBC toolchains do not use 'nullptr' everywhere when C++:
// openwrt-sdk-x86-64_gcc-11.2.0_musl.Linux-x86_64/staging_dir/
//   toolchain-x86_64_gcc-11.2.0_musl/include/fortify/stdlib.h:
//   51:32: error: zero as null pointer constant
#pragma GCC diagnostic error "-Wzero-as-null-pointer-constant"
#endif
#endif // UPX_CONFIG_DISABLE_WERROR
#endif // UPX_CONFIG_DISABLE_WSTRICT

// multithreading (UPX currently does not use multithreading)
#if (WITH_THREADS) && defined(_WIN32) && defined(__GLIBCXX__)
#if !defined(_GLIBCXX_HAS_GTHREADS)
#error "broken MinGW installation: missing _GLIBCXX_HAS_GTHREADS"
#undef WITH_THREADS
#endif
#endif
#if (WITH_THREADS)
#define upx_thread_local     thread_local
#define upx_std_atomic(Type) std::atomic<Type>
#define upx_std_once_flag    std::once_flag
#define upx_std_call_once    std::call_once
#else
#define upx_thread_local     /*empty*/
#define upx_std_atomic(Type) Type
#define upx_std_once_flag    upx_std_atomic(size_t)
template <class NoexceptCallable>
inline void upx_std_call_once(upx_std_once_flag &flag, NoexceptCallable &&f) {
    if (__acc_unlikely(!flag)) {
        flag = 1;
        f();
    }
}
#endif // WITH_THREADS

// <type_traits> upx_is_integral is overloaded for BE16 & friends; see bele.h
template <class T>
struct upx_is_integral : public std::is_integral<T> {};
template <class T>
inline constexpr bool upx_is_integral_v = upx_is_integral<T>::value;

#if (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC) && defined(__MINT__)
// horrible hack for broken compiler / ABI
#define upx_fake_alignas_1    __attribute__((__aligned__(1), __packed__))
#define upx_fake_alignas_2    __attribute__((__aligned__(2)))
#define upx_fake_alignas_4    __attribute__((__aligned__(2))) // object file maximum 2 ???
#define upx_fake_alignas_16   __attribute__((__aligned__(2))) // object file maximum 2 ???
#define upx_fake_alignas__(x) upx_fake_alignas_##x
#define alignas(x)            upx_fake_alignas__(x)
#define upx_alignas_max       upx_fake_alignas_4
#endif
#ifndef upx_alignas_max
#define upx_alignas_max alignas(std::max_align_t)
#endif

/*************************************************************************
// core
**************************************************************************/

// protect against integer overflows and malicious header fields
// see C 11 standard, Annex K
//
// this limits uncompressed_size to about 682 MiB (715_128_832 bytes)
typedef size_t upx_rsize_t;
#define UPX_RSIZE_MAX     UPX_RSIZE_MAX_MEM
#define UPX_RSIZE_MAX_MEM (768 * 1024 * 1024) // DO NOT CHANGE !!!
#define UPX_RSIZE_MAX_STR (256 * 1024)

// integral types
typedef acc_int8_t upx_int8_t;
typedef acc_uint8_t upx_uint8_t;
typedef acc_int16_t upx_int16_t;
typedef acc_uint16_t upx_uint16_t;
typedef acc_int32_t upx_int32_t;
typedef acc_uint32_t upx_uint32_t;
typedef acc_int64_t upx_int64_t;
typedef acc_uint64_t upx_uint64_t;
typedef acc_uintptr_t upx_uintptr_t;

// convention: use "byte" when dealing with data; use "char/uchar" when dealing
// with strings; use "upx_uint8_t" when dealing with small integers
typedef unsigned char byte;
#define upx_byte  byte
#define upx_bytep byte *
typedef unsigned char uchar;
// convention: use "charptr" when dealing with abstract pointer arithmetics
#define charptr upx_charptr_unit_type *
// upx_charptr_unit_type is some opaque type with sizeof(type) == 1
struct alignas(1) upx_charptr_unit_type final { char hidden__; };
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(upx_charptr_unit_type) == 1)

// using the system off_t was a bad idea even back in 199x...
typedef upx_int64_t upx_off_t;
#undef off_t
#if 0
// TODO cleanup: at some future point we can do this:
#define off_t DO_NOT_USE_off_t
#else
#define off_t upx_off_t
#endif

// shortcuts
#define forceinline __acc_forceinline
#if _MSC_VER
#define noinline __declspec(noinline)
#undef __acc_noinline
#define __acc_noinline noinline
#else
#define noinline __acc_noinline
#endif
#define forceinline_constexpr forceinline constexpr
#define likely                __acc_likely
#define unlikely              __acc_unlikely
#define very_likely           __acc_very_likely
#define very_unlikely         __acc_very_unlikely
#define may_throw             noexcept(false)

#define COMPILE_TIME_ASSERT(e) ACC_COMPILE_TIME_ASSERT(e)
#define DELETED_FUNCTION       = delete
#define UNUSED(var)            ACC_UNUSED(var)

/*************************************************************************
// portab
**************************************************************************/

// some system headers may define these, so undef just in case
#undef _
#undef __
#undef ___
#undef dos
#undef large
#undef linux
#undef small
#undef SP
#undef SS
#undef tos
#undef unix
#if (ACC_OS_POSIX) && !defined(__unix__)
#define __unix__ 1
#endif
#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 ||  \
     ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#undef __unix
#undef __unix__
#endif
#if (ACC_OS_DOS32) && defined(__DJGPP__)
#undef sopen
#endif

#if defined(HAVE_DUP) && (HAVE_DUP + 0 == 0)
#undef dup
#define dup(x) (-1)
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO (fileno(stdin))
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO (fileno(stdout))
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO (fileno(stderr))
#endif

#if !(HAVE_STRCASECMP) && (HAVE_STRICMP) && !defined(strcasecmp)
#define strcasecmp stricmp
#endif
#if !(HAVE_STRNCASECMP) && (HAVE_STRNICMP) && !defined(strncasecmp)
#define strncasecmp strnicmp
#endif

#if !defined(S_IWUSR) && defined(_S_IWUSR)
#define S_IWUSR _S_IWUSR
#elif !defined(S_IWUSR) && defined(_S_IWRITE)
#define S_IWUSR _S_IWRITE
#endif

#if !defined(S_IFMT) && defined(_S_IFMT)
#define S_IFMT _S_IFMT
#endif
#if !defined(S_IFREG) && defined(_S_IFREG)
#define S_IFREG _S_IFREG
#endif
#if !defined(S_IFDIR) && defined(_S_IFDIR)
#define S_IFDIR _S_IFDIR
#endif
#if !defined(S_IFCHR) && defined(_S_IFCHR)
#define S_IFCHR _S_IFCHR
#endif

#if !defined(S_ISREG)
#if defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) &S_IFMT) == S_IFREG)
#else
#error "S_ISREG"
#endif
#endif
#if !defined(S_ISDIR)
#if defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) &S_IFMT) == S_IFDIR)
#else
#error "S_ISDIR"
#endif
#endif
#if !defined(S_ISCHR)
#if defined(S_IFMT) && defined(S_IFCHR)
#define S_ISCHR(m) (((m) &S_IFMT) == S_IFCHR)
#endif
#endif

// avoid warnings about shadowing global symbols
#undef _base
#undef basename
#undef index
#undef outp
#define _base    upx_renamed__base
#define basename upx_renamed_basename
#define index    upx_renamed_index
#define outp     upx_renamed_outp

#undef PAGE_MASK
#undef PAGE_SIZE

#if !defined(O_BINARY) || (O_BINARY + 0 == 0)
#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 ||  \
     ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#error "missing O_BINARY"
#endif
#endif
#if !defined(O_BINARY)
#define O_BINARY 0
#endif

/*************************************************************************
// util
**************************************************************************/

#define CLANG_FORMAT_DUMMY_STATEMENT /*empty*/

// malloc debuggers
#if !defined(VALGRIND_CHECK_MEM_IS_ADDRESSABLE)
#define VALGRIND_CHECK_MEM_IS_ADDRESSABLE(addr, len) 0
#endif
#if !defined(VALGRIND_CHECK_MEM_IS_DEFINED)
#define VALGRIND_CHECK_MEM_IS_DEFINED(addr, len) 0
#endif
#if !defined(VALGRIND_MAKE_MEM_DEFINED)
#define VALGRIND_MAKE_MEM_DEFINED(addr, len) 0
#endif
#if !defined(VALGRIND_MAKE_MEM_NOACCESS)
#define VALGRIND_MAKE_MEM_NOACCESS(addr, len) 0
#endif
#if !defined(VALGRIND_MAKE_MEM_UNDEFINED)
#define VALGRIND_MAKE_MEM_UNDEFINED(addr, len) 0
#endif

#if defined(_WIN32) && defined(__MINGW32__) && defined(__GNUC__) && !defined(__clang__)
#define attribute_format(a, b) __attribute__((__format__(__gnu_printf__, a, b)))
#elif (ACC_CC_CLANG || ACC_CC_GNUC)
#define attribute_format(a, b) __attribute__((__format__(__printf__, a, b)))
#else
#define attribute_format(a, b) /*empty*/
#endif

// for no-op debug output
inline void NO_printf(const char *, ...) noexcept attribute_format(1, 2);
inline void NO_fprintf(FILE *, const char *, ...) noexcept attribute_format(2, 3);
inline void NO_printf(const char *, ...) noexcept {}
inline void NO_fprintf(FILE *, const char *, ...) noexcept {}

#if __has_builtin(__builtin_memcpy_inline)
#define upx_memcpy_inline __builtin_memcpy_inline
#elif __has_builtin(__builtin_memcpy)
#define upx_memcpy_inline __builtin_memcpy
#elif defined(__GNUC__)
#define upx_memcpy_inline __builtin_memcpy
#else
#define upx_memcpy_inline memcpy
#endif

#if __has_builtin(__builtin_return_address)
#define upx_return_address() __builtin_return_address(0)
#elif defined(__GNUC__)
#define upx_return_address() __builtin_return_address(0)
#elif (ACC_CC_MSC)
#define upx_return_address() _ReturnAddress()
#else
#define upx_return_address() nullptr
#endif

// TODO cleanup: we now require C++17, so remove all packed_struct usage
#define packed_struct(s) struct alignas(1) s

#define COMPILE_TIME_ASSERT_ALIGNOF_USING_SIZEOF__(a, b)                                           \
    {                                                                                              \
        typedef a acc_tmp_a_t;                                                                     \
        typedef b acc_tmp_b_t;                                                                     \
        struct alignas(1) acc_tmp_t {                                                              \
            acc_tmp_b_t x;                                                                         \
            acc_tmp_a_t y;                                                                         \
            acc_tmp_b_t z;                                                                         \
        };                                                                                         \
        COMPILE_TIME_ASSERT(sizeof(struct acc_tmp_t) == 2 * sizeof(b) + sizeof(a))                 \
        COMPILE_TIME_ASSERT(sizeof(((acc_tmp_t *) nullptr)->x) +                                   \
                                sizeof(((acc_tmp_t *) nullptr)->y) +                               \
                                sizeof(((acc_tmp_t *) nullptr)->z) ==                              \
                            2 * sizeof(b) + sizeof(a))                                             \
    }
#define COMPILE_TIME_ASSERT_ALIGNOF__(a, b)                                                        \
    COMPILE_TIME_ASSERT_ALIGNOF_USING_SIZEOF__(a, b)                                               \
    COMPILE_TIME_ASSERT(__acc_alignof(a) == sizeof(b))                                             \
    COMPILE_TIME_ASSERT(alignof(a) == sizeof(b))
#define COMPILE_TIME_ASSERT_ALIGNED1(a) COMPILE_TIME_ASSERT_ALIGNOF__(a, char)

#define TABLESIZE(table) ((sizeof(table) / sizeof((table)[0])))

template <class T>
inline T ALIGN_DOWN(const T &a, const T &b) {
    T r;
    r = (a / b) * b;
    return r;
}
template <class T>
inline T ALIGN_UP(const T &a, const T &b) {
    T r;
    r = ((a + b - 1) / b) * b;
    return r;
}
template <class T>
inline T ALIGN_GAP(const T &a, const T &b) {
    T r;
    r = ALIGN_UP(a, b) - a;
    return r;
}

template <class T>
inline const T &UPX_MAX(const T &a, const T &b) {
    if (a < b)
        return b;
    return a;
}
template <class T>
inline const T &UPX_MIN(const T &a, const T &b) {
    if (a < b)
        return a;
    return b;
}

template <class T>
inline void mem_clear(T *object) noexcept {
    static_assert(std::is_class_v<T>); // UPX convention
    static_assert(std::is_standard_layout_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);
    constexpr size_t size = sizeof(*object);
    static_assert(size >= 1 && size <= UPX_RSIZE_MAX_MEM);
    memset((void *) object, 0, size);
}
#if defined(__clang__) || __GNUC__ != 7
template <class T>
inline void mem_clear(T (&array)[]) noexcept DELETED_FUNCTION;
#endif
template <class T, size_t N>
inline void mem_clear(T (&array)[N]) noexcept DELETED_FUNCTION;

// An Array allocates memory on the heap, and automatically
// gets destructed when leaving scope or on exceptions.
#define Array(type, var, n)                                                                        \
    MemBuffer var##_membuf(mem_size(sizeof(type), (n)));                                           \
    type *const var = ACC_STATIC_CAST(type *, var##_membuf.getVoidPtr())

#define ByteArray(var, n) Array(byte, var, (n))

// assert_noexcept()
noinline void assertFailed(const char *expr, const char *file, int line, const char *func) noexcept;
noinline void throwAssertFailed(const char *expr, const char *file, int line, const char *func);
#if defined(__GNUC__)
#undef assert
#if DEBUG || 0
// generate a warning if assert() is used inside a "noexcept" context
#define assert(e)                                                                                  \
    ((void) (__acc_cte(e) || (assertFailed(#e, __FILE__, __LINE__, __func__), throw 1, 0)))
#else
// turn assertion failures into exceptions
#define assert(e)                                                                                  \
    ((void) (__acc_cte(e) || (throwAssertFailed(#e, __FILE__, __LINE__, __func__), throw 1, 0)))
#endif
#define assert_noexcept(e)                                                                         \
    ((void) (__acc_cte(e) || (assertFailed(#e, __FILE__, __LINE__, __func__), 0)))
#else
#define assert_noexcept assert
#endif

#include "util/cxxlib.h"
using upx::is_same_any_v;
using upx::noncopyable;
using upx::OptVar;
using upx::tribool;
#define usizeof(expr) (upx::UnsignedSizeOf<sizeof(expr)>::value)

/*************************************************************************
// constants
**************************************************************************/

/* exit codes of this program: 0 ok, 1 error, 2 warning */
#define EXIT_OK         0
#define EXIT_ERROR      1
#define EXIT_WARN       2
//
#define EXIT_USAGE      1
#define EXIT_FILE_READ  1
#define EXIT_FILE_WRITE 1
#define EXIT_MEMORY     1
#define EXIT_CHECKSUM   1
#define EXIT_INIT       1
#define EXIT_INTERNAL   1

// magic constants for patching
#define UPX_MAGIC_LE32  0x21585055 /* "UPX!" */
#define UPX_MAGIC2_LE32 0xD5D0D8A1

// upx_compress() error codes
#define UPX_E_OK                  (0)
#define UPX_E_ERROR               (-1)
#define UPX_E_OUT_OF_MEMORY       (-2)
#define UPX_E_NOT_COMPRESSIBLE    (-3)
#define UPX_E_INPUT_OVERRUN       (-4)
#define UPX_E_OUTPUT_OVERRUN      (-5)
#define UPX_E_LOOKBEHIND_OVERRUN  (-6)
#define UPX_E_EOF_NOT_FOUND       (-7)
#define UPX_E_INPUT_NOT_CONSUMED  (-8)
#define UPX_E_NOT_YET_IMPLEMENTED (-9)
#define UPX_E_INVALID_ARGUMENT    (-10)

// Executable formats (info: big endian types are >= 128); DO NOT CHANGE
#define UPX_F_DOS_COM             1
#define UPX_F_DOS_SYS             2
#define UPX_F_DOS_EXE             3
#define UPX_F_DJGPP2_COFF         4
#define UPX_F_WATCOM_LE           5
// #define UPX_F_VXD_LE              6 // NOT IMPLEMENTED
#define UPX_F_DOS_EXEH            7 // OBSOLETE
#define UPX_F_TMT_ADAM            8
#define UPX_F_W32PE_I386          9
#define UPX_F_LINUX_i386          10
// #define UPX_F_WIN16_NE            11 // NOT IMPLEMENTED
#define UPX_F_LINUX_ELF_i386      12
// #define UPX_F_LINUX_SEP_i386      13 // NOT IMPLEMENTED
#define UPX_F_LINUX_SH_i386       14
#define UPX_F_VMLINUZ_i386        15
#define UPX_F_BVMLINUZ_i386       16
#define UPX_F_ELKS_8086           17 // NOT IMPLEMENTED
#define UPX_F_PS1_EXE             18
#define UPX_F_VMLINUX_i386        19
#define UPX_F_LINUX_ELFI_i386     20
#define UPX_F_WINCE_ARM           21 // Windows CE
#define UPX_F_LINUX_ELF64_AMD64   22
#define UPX_F_LINUX_ELF32_ARM     23
#define UPX_F_BSD_i386            24
#define UPX_F_BSD_ELF_i386        25
#define UPX_F_BSD_SH_i386         26
#define UPX_F_VMLINUX_AMD64       27
#define UPX_F_VMLINUX_ARM         28
#define UPX_F_MACH_i386           29
#define UPX_F_LINUX_ELF32_MIPSEL  30
#define UPX_F_VMLINUZ_ARM         31
#define UPX_F_MACH_ARM            32
#define UPX_F_DYLIB_i386          33
#define UPX_F_MACH_AMD64          34
#define UPX_F_DYLIB_AMD64         35
#define UPX_F_W64PE_AMD64         36
#define UPX_F_MACH_ARM64          37
// #define UPX_F_MACH_PPC64LE        38 // DOES NOT EXIST
#define UPX_F_LINUX_ELF64_PPC64LE 39
#define UPX_F_VMLINUX_PPC64LE     40
// #define UPX_F_DYLIB_PPC64LE       41 // DOES NOT EXIST
#define UPX_F_LINUX_ELF64_ARM64   42
#define UPX_F_W64PE_ARM64         43 // NOT YET IMPLEMENTED
#define UPX_F_W64PE_ARM64EC       44 // NOT YET IMPLEMENTED

#define UPX_F_ATARI_TOS         129
// #define UPX_F_SOLARIS_SPARC     130 // NOT IMPLEMENTED
#define UPX_F_MACH_PPC32        131
#define UPX_F_LINUX_ELF32_PPC32 132
#define UPX_F_LINUX_ELF32_ARMEB 133
#define UPX_F_MACH_FAT          134
#define UPX_F_VMLINUX_ARMEB     135
#define UPX_F_VMLINUX_PPC32     136
#define UPX_F_LINUX_ELF32_MIPS  137
#define UPX_F_DYLIB_PPC32       138
#define UPX_F_MACH_PPC64        139
#define UPX_F_LINUX_ELF64_PPC64 140
#define UPX_F_VMLINUX_PPC64     141
#define UPX_F_DYLIB_PPC64       142

// compression methods; DO NOT CHANGE
#define M_NRV2B_LE32  2
#define M_NRV2B_8     3
#define M_NRV2B_LE16  4
#define M_NRV2D_LE32  5
#define M_NRV2D_8     6
#define M_NRV2D_LE16  7
#define M_NRV2E_LE32  8
#define M_NRV2E_8     9
#define M_NRV2E_LE16  10
// #define M_CL1B_LE32   11
// #define M_CL1B_8      12
// #define M_CL1B_LE16   13
#define M_LZMA        14
#define M_DEFLATE     15 // zlib
#define M_ZSTD        16
#define M_BZIP2       17
// compression methods internal usage
#define M_ALL         (-1)
#define M_END         (-2)
#define M_NONE        (-3)
#define M_SKIP        (-4)
#define M_ULTRA_BRUTE (-5)

#define M_IS_NRV2B(x)   ((x) >= M_NRV2B_LE32 && (x) <= M_NRV2B_LE16)
#define M_IS_NRV2D(x)   ((x) >= M_NRV2D_LE32 && (x) <= M_NRV2D_LE16)
#define M_IS_NRV2E(x)   ((x) >= M_NRV2E_LE32 && (x) <= M_NRV2E_LE16)
// #define M_IS_CL1B(x)    ((x) >= M_CL1B_LE32  && (x) <= M_CL1B_LE16)
#define M_IS_LZMA(x)    (((x) &255) == M_LZMA)
#define M_IS_DEFLATE(x) ((x) == M_DEFLATE)
#define M_IS_ZSTD(x)    ((x) == M_ZSTD)
#define M_IS_BZIP2(x)   ((x) == M_BZIP2)

// filters internal usage
#define FT_END         (-1)
#define FT_NONE        (-2)
#define FT_SKIP        (-3)
#define FT_ULTRA_BRUTE (-4)

/*************************************************************************
// compression - setup and callback_t
**************************************************************************/

#define WITH_LZMA 1
#define WITH_UCL  1
#define WITH_ZLIB 1
#if (WITH_UCL)
#define ucl_compress_config_t REAL_ucl_compress_config_t
#include <ucl/include/ucl/uclconf.h>
#include <ucl/include/ucl/ucl.h>
#undef ucl_compress_config_t
#undef ucl_compress_config_p
#endif

struct upx_callback_t;
typedef void(__acc_cdecl *upx_progress_func_t)(upx_callback_t *, unsigned, unsigned);

struct upx_callback_t final {
    upx_progress_func_t nprogress;
    void *user;

    void reset() noexcept { mem_clear(this); }
};

/*************************************************************************
// compression - config_t
**************************************************************************/

struct bzip2_compress_config_t final {
    unsigned dummy;

    void reset() noexcept;
};

struct lzma_compress_config_t final {
    typedef OptVar<unsigned, 2u, 0u, 4u> pos_bits_t;         // pb
    typedef OptVar<unsigned, 0u, 0u, 4u> lit_pos_bits_t;     // lp
    typedef OptVar<unsigned, 3u, 0u, 8u> lit_context_bits_t; // lc
    typedef OptVar<unsigned, (1u << 22), 1u, (1u << 30)> dict_size_t;
    typedef OptVar<unsigned, 64u, 5u, 273u> num_fast_bytes_t;

    pos_bits_t pos_bits;                 // pb
    lit_pos_bits_t lit_pos_bits;         // lp
    lit_context_bits_t lit_context_bits; // lc
    dict_size_t dict_size;
    unsigned fast_mode;
    num_fast_bytes_t num_fast_bytes;
    unsigned match_finder_cycles;

    unsigned max_num_probs;

    void reset() noexcept;
};

struct ucl_compress_config_t final : public REAL_ucl_compress_config_t {
    void reset() noexcept { memset(this, 0xff, sizeof(*this)); }
};

struct zlib_compress_config_t final {
    typedef OptVar<unsigned, 8u, 1u, 9u> mem_level_t;     // ml
    typedef OptVar<unsigned, 15u, 9u, 15u> window_bits_t; // wb
    typedef OptVar<unsigned, 0u, 0u, 4u> strategy_t;      // st

    mem_level_t mem_level;     // ml
    window_bits_t window_bits; // wb
    strategy_t strategy;       // st

    void reset() noexcept;
};

struct zstd_compress_config_t final {
    unsigned dummy;

    void reset() noexcept;
};

struct upx_compress_config_t final {
    bzip2_compress_config_t conf_bzip2;
    lzma_compress_config_t conf_lzma;
    ucl_compress_config_t conf_ucl;
    zlib_compress_config_t conf_zlib;
    zstd_compress_config_t conf_zstd;

    void reset() noexcept {
        conf_bzip2.reset();
        conf_lzma.reset();
        conf_ucl.reset();
        conf_zlib.reset();
        conf_zstd.reset();
    }
};

#define NULL_cconf ((upx_compress_config_t *) nullptr)

/*************************************************************************
// compression - result_t
**************************************************************************/

struct bzip2_compress_result_t final {
    unsigned dummy;

    void reset() noexcept { mem_clear(this); }
};

struct lzma_compress_result_t final {
    unsigned pos_bits;         // pb
    unsigned lit_pos_bits;     // lp
    unsigned lit_context_bits; // lc
    unsigned dict_size;
    unsigned fast_mode;
    unsigned num_fast_bytes;
    unsigned match_finder_cycles;
    unsigned num_probs; // (computed result)

    void reset() noexcept { mem_clear(this); }
};

struct ucl_compress_result_t final {
    ucl_uint result[16];

    void reset() noexcept { mem_clear(this); }
};

struct zlib_compress_result_t final {
    unsigned dummy;

    void reset() noexcept { mem_clear(this); }
};

struct zstd_compress_result_t final {
    unsigned dummy;

    void reset() noexcept { mem_clear(this); }
};

struct upx_compress_result_t final {
    // debugging aid
    struct Debug {
        int method, level;
        unsigned u_len, c_len;
        void reset() noexcept { mem_clear(this); }
    };
    Debug debug;

    bzip2_compress_result_t result_bzip2;
    lzma_compress_result_t result_lzma;
    ucl_compress_result_t result_ucl;
    zlib_compress_result_t result_zlib;
    zstd_compress_result_t result_zstd;

    void reset() noexcept {
        debug.reset();
        result_bzip2.reset();
        result_lzma.reset();
        result_ucl.reset();
        result_zlib.reset();
        result_zstd.reset();
    }
};

/*************************************************************************
// globals
**************************************************************************/

// classes
class ElfLinker;
typedef ElfLinker Linker;
class Throwable;

// util/membuffer.h
class MemBuffer;
void *membuffer_get_void_ptr(MemBuffer &mb) noexcept;
unsigned membuffer_get_size(MemBuffer &mb) noexcept;

// util/dt_check.cpp
noinline void upx_compiler_sanity_check() noexcept;
noinline int upx_doctest_check(int argc, char **argv);
int upx_doctest_check();

// main.cpp
extern const char *progname;
bool main_set_exit_code(int ec);
int main_get_options(int argc, char **argv);
void main_get_envoptions();
int upx_main(int argc, char *argv[]) may_throw;

// msg.cpp
void printSetNl(int need_nl) noexcept;
void printClearLine(FILE *f = nullptr) noexcept;
void printErr(const char *iname, const Throwable &e) noexcept;
void printUnhandledException(const char *iname, const std::exception *e) noexcept;
void printErr(const char *iname, const char *format, ...) noexcept attribute_format(2, 3);
void printWarn(const char *iname, const char *format, ...) noexcept attribute_format(2, 3);

void infoWarning(const char *format, ...) attribute_format(1, 2);
void infoHeader(const char *format, ...) attribute_format(1, 2);
void info(const char *format, ...) attribute_format(1, 2);
void infoHeader();
void infoWriting(const char *what, upx_int64_t size);

// work.cpp
void do_one_file(const char *iname, char *oname) may_throw;
int do_files(int i, int argc, char *argv[]) may_throw;

// help.cpp
extern const char gitrev[];
void show_header();
void show_help(int verbose = 0);
void show_license();
void show_sysinfo(const char *options_var);
void show_usage();
void show_version(bool one_line = false);

// compress/compress.cpp
// clang-format off
unsigned upx_adler32(const void *buf, unsigned len, unsigned adler = 1);
unsigned upx_crc32  (const void *buf, unsigned len, unsigned crc = 0);

int upx_compress           ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned *dst_len,
                                   upx_callback_t *cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult );
int upx_decompress         ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned *dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
int upx_test_overlap       ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned *dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
// clang-format on

#include "util/snprintf.h" // must get included first!
#include "util/util.h"
#include "options.h"
#include "except.h"
#include "bele.h"
#include "console/console.h"
// xspan
#include "util/raw_bytes.h"
#include "util/xspan.h"

/* vim:set ts=4 sw=4 et: */

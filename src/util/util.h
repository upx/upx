/* util.h --

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
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
// upx_rsize_t and mem_size: assert sane memory buffer sizes to protect
// against integer overflows and malicious header fields
// see C 11 standard, Annex K
**************************************************************************/

inline bool mem_size_valid_bytes(upx_uint64_t bytes) noexcept { return bytes <= UPX_RSIZE_MAX; }

noinline bool mem_size_valid(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1 = 0,
                             upx_uint64_t extra2 = 0) noexcept;

// will throw on invalid size
noinline upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1,
                              upx_uint64_t extra2 = 0) may_throw;

noinline upx_rsize_t mem_size_ptr(const void *ptr, upx_uint64_t element_size, upx_uint64_t n,
                                  upx_uint64_t extra1 = 0, upx_uint64_t extra2 = 0) may_throw;

//
// inline fast paths:
//

// will throw on invalid size
inline upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n) may_throw {
    upx_uint64_t bytes = element_size * n;
    if very_unlikely (element_size == 0 || element_size > UPX_RSIZE_MAX || n > UPX_RSIZE_MAX ||
                      bytes > UPX_RSIZE_MAX)
        return mem_size(element_size, n, 0, 0); // this will throw
    return ACC_ICONV(upx_rsize_t, bytes);
}

// will throw on invalid size
inline upx_rsize_t mem_size_get_n(upx_uint64_t element_size, upx_uint64_t n) may_throw {
    (void) mem_size(element_size, n); // assert size
    return ACC_ICONV(upx_rsize_t, n); // and return n
}

// will throw on invalid size
inline void mem_size_assert(upx_uint64_t element_size, upx_uint64_t n) may_throw {
    (void) mem_size(element_size, n); // assert size
}
inline void mem_size_assert_noexcept(upx_uint64_t element_size, upx_uint64_t n) noexcept {
    assert_noexcept(mem_size_valid(element_size, n)); // assert size
}

// "new" with asserted size; will throw on invalid size
template <class T>
inline T *NewT(upx_uint64_t n) may_throw {
    COMPILE_TIME_ASSERT(std::is_standard_layout<T>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_copyable<T>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_default_constructible<T>::value)
    const upx_rsize_t bytes = mem_size(sizeof(T), n); // assert size
    T *array = new T[size_t(n)];
#if !defined(__SANITIZE_MEMORY__)
    if likely (array != nullptr && bytes > 0) {
        memset(array, 0xfb, bytes); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
        (void) VALGRIND_MAKE_MEM_UNDEFINED(array, bytes);
    }
#endif
    UNUSED(bytes);
    return array;
}
#if DEBUG || 1
#define New(type, n) (NewT<type>((n)))
#else
#define New(type, n) new type[mem_size_get_n(sizeof(type), (n))]
#endif

template <class T>
inline T *New0T(upx_uint64_t n) may_throw {
    COMPILE_TIME_ASSERT(std::is_standard_layout<T>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_copyable<T>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_default_constructible<T>::value)
    const upx_rsize_t bytes = mem_size(sizeof(T), n); // assert size
    T *array = new T[size_t(n)];
    if likely (array != nullptr && bytes > 0)
        memset(array, 0, bytes); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    return array;
}
#define New0(type, n) (New0T<type>((n)))

/*************************************************************************
// ptr util
**************************************************************************/

#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
forceinline upx_ptraddr_t ptr_get_address(const void *p) noexcept {
    return __builtin_cheri_address_get(p);
}
forceinline upx_ptraddr_t ptr_get_address(upx_uintptr_t p) noexcept {
    return __builtin_cheri_address_get(p);
}
#else
forceinline upx_ptraddr_t ptr_get_address(const void *p) noexcept { return (upx_uintptr_t) p; }
forceinline upx_ptraddr_t ptr_get_address(upx_uintptr_t p) noexcept { return p; }
#endif

forceinline upx_sptraddr_t ptraddr_diff(const void *a, const void *b) noexcept {
    return ptr_get_address(a) - ptr_get_address(b);
}

template <size_t Alignment>
forceinline bool ptr_is_aligned(const void *p) noexcept {
    static_assert(upx::has_single_bit(Alignment));
    return (ptr_get_address(p) & (Alignment - 1)) == 0;
}
forceinline bool ptr_is_aligned(const void *p, size_t alignment) noexcept {
    assert_noexcept(upx::has_single_bit(alignment));
    return (ptr_get_address(p) & (alignment - 1)) == 0;
}

template <class T>
forceinline T *ptr_align_down(T *p, size_t alignment) noexcept {
    typedef std::conditional_t<std::is_const_v<T>, const char *, char *> CharPtr;
    return upx::ptr_static_cast<T *>(upx::ptr_static_cast<CharPtr>(p) -
                                     size_t(ptr_get_address(p) & (alignment - 1)));
}
template <class T>
forceinline T *ptr_align_up(T *p, size_t alignment) noexcept {
    typedef std::conditional_t<std::is_const_v<T>, const char *, char *> CharPtr;
    return upx::ptr_static_cast<T *>(
        upx::ptr_static_cast<CharPtr>(p) +
        size_t((upx_ptraddr_t(0) - ptr_get_address(p)) & (alignment - 1)));
}

// ptrdiff_t with nullptr checks and asserted size; will throw on failure
// NOTE: returns size_in_bytes, not number of elements!
noinline int ptr_diff_bytes(const void *a, const void *b) may_throw;
noinline unsigned ptr_udiff_bytes(const void *a, const void *b) may_throw; // asserts a >= b

// short names "ptr_diff" and "ptr_udiff" for types with sizeof(X) == 1
template <class T, class U>
forceinline typename std::enable_if<sizeof(T) == 1 && sizeof(U) == 1, int>::type
ptr_diff(const T *a, const U *b) may_throw {
    return ptr_diff_bytes(a, b);
}
template <class T, class U>
forceinline typename std::enable_if<sizeof(T) == 1 && sizeof(U) == 1, unsigned>::type
ptr_udiff(const T *a, const U *b) may_throw {
    return ptr_udiff_bytes(a, b);
}

// check that buffers do not overlap; will throw on error
noinline void ptraddr_check_no_overlap(upx_ptraddr_t a, size_t a_size, upx_ptraddr_t b,
                                       size_t b_size) may_throw;
noinline void ptraddr_check_no_overlap(upx_ptraddr_t a, size_t a_size, upx_ptraddr_t b,
                                       size_t b_size, upx_ptraddr_t c, size_t c_size) may_throw;

forceinline void ptr_check_no_overlap(const void *a, size_t a_size, const void *b, size_t b_size)
    may_throw {
    ptraddr_check_no_overlap(ptr_get_address(a), a_size, ptr_get_address(b), b_size);
}
forceinline void ptr_check_no_overlap(const void *a, size_t a_size, const void *b, size_t b_size,
                                      const void *c, size_t c_size) may_throw {
    ptraddr_check_no_overlap(ptr_get_address(a), a_size, ptr_get_address(b), b_size,
                             ptr_get_address(c), c_size);
}

// invalidate and poison a pointer: point to a non-null invalid address
// - resulting pointer should crash on dereference
// - this should be efficient, so no mmap() guard page etc.
// - this should play nice with runtime checkers like ASAN, MSAN, UBSAN, valgrind, etc.
// - this should play nice with static analyzers like clang-tidy etc.
// NOTE: this is clearly UB (Undefined Behaviour), and stricter compilers or
//   architectures may need a more advanced/costly implementation in the future
template <class T>
inline void ptr_invalidate_and_poison(T *(&ptr)) noexcept {
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
    // TODO later: examine __builtin_cheri_bounds_set()
    ptr = upx::ptr_static_cast<T *>(&ptr); // FIXME: HACK: point to address of self
#else
    // pretend there is some memory-mapped I/O at address 0x00000200
    ptr = (T *) (void *) 512; // NOLINT(performance-no-int-to-ptr)
#endif
}

/*************************************************************************
// stdlib
**************************************************************************/

noinline void *upx_calloc(size_t n, size_t element_size) may_throw;

noinline const char *upx_getenv(const char *envvar) noexcept;

noinline void upx_memswap(void *a, void *b, size_t bytes) noexcept;

noinline void upx_rand_init() noexcept;
noinline int upx_rand() noexcept;

typedef int(__acc_cdecl_qsort *upx_compare_func_t)(const void *, const void *);
typedef void (*upx_sort_func_t)(void *array, size_t n, size_t element_size, upx_compare_func_t);

void upx_gnomesort(void *array, size_t n, size_t element_size, upx_compare_func_t compare);
void upx_shellsort_memswap(void *array, size_t n, size_t element_size, upx_compare_func_t compare);
void upx_shellsort_memcpy(void *array, size_t n, size_t element_size, upx_compare_func_t compare);

// this wraps std::stable_sort()
template <size_t ElementSize>
void upx_std_stable_sort(void *array, size_t n, upx_compare_func_t compare);

// #define UPX_CONFIG_USE_STABLE_SORT 1
#if UPX_CONFIG_USE_STABLE_SORT
// use std::stable_sort(); NOTE: requires that "element_size" is constexpr!
#define upx_qsort(a, n, element_size, compare)                                                     \
    upx_std_stable_sort<(element_size)>((a), (n), (compare))
#else
                              // use libc qsort(); good enough for our use cases
#define upx_qsort ::qsort
#endif

/*************************************************************************
// misc support functions
**************************************************************************/

int find(const void *b, int blen, const void *what, int wlen) noexcept;
int find_be16(const void *b, int blen, unsigned what) noexcept;
int find_be32(const void *b, int blen, unsigned what) noexcept;
int find_be64(const void *b, int blen, upx_uint64_t what) noexcept;
int find_le16(const void *b, int blen, unsigned what) noexcept;
int find_le32(const void *b, int blen, unsigned what) noexcept;
int find_le64(const void *b, int blen, upx_uint64_t what) noexcept;

int mem_replace(void *b, int blen, const void *what, int wlen, const void *r) noexcept;

char *fn_basename(const char *name);
int fn_strcmp(const char *n1, const char *n2);
char *fn_strlwr(char *n);
bool fn_has_ext(const char *name, const char *ext, bool ignore_case = true);

bool file_exists(const char *name);
bool maketempname(char *ofilename, size_t size, const char *ifilename, const char *ext,
                  bool force = true);
bool makebakname(char *ofilename, size_t size, const char *ifilename, bool force = true);

noinline bool is_envvar_true(const char *envvar, const char *alternate_name = nullptr) noexcept;

unsigned get_ratio(upx_uint64_t u_len, upx_uint64_t c_len);
bool set_method_name(char *buf, size_t size, int method, int level);
void center_string(char *buf, size_t size, const char *s);

/* vim:set ts=4 sw=4 et: */

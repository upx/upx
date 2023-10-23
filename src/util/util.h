/* util.h --

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
// assert sane memory buffer sizes to protect against integer overflows
// and malicious header fields
// see C 11 standard, Annex K
**************************************************************************/

inline bool mem_size_valid_bytes(upx_uint64_t bytes) noexcept { return bytes <= UPX_RSIZE_MAX; }

bool mem_size_valid(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1 = 0,
                    upx_uint64_t extra2 = 0) noexcept;

// will throw on invalid size
upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1,
                     upx_uint64_t extra2 = 0);

//
// inline fast paths:
//

// will throw on invalid size
inline upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n) {
    upx_uint64_t bytes = element_size * n;
    if very_unlikely (element_size == 0 || element_size > UPX_RSIZE_MAX || n > UPX_RSIZE_MAX ||
                      bytes > UPX_RSIZE_MAX)
        return mem_size(element_size, n, 0, 0); // this will throw
    return ACC_ICONV(upx_rsize_t, bytes);
}

// will throw on invalid size
inline upx_rsize_t mem_size_get_n(upx_uint64_t element_size, upx_uint64_t n) {
    (void) mem_size(element_size, n); // assert size
    return ACC_ICONV(upx_rsize_t, n); // and return n
}

// will throw on invalid size
inline void mem_size_assert(upx_uint64_t element_size, upx_uint64_t n) {
    (void) mem_size(element_size, n); // assert size
}

// "new" with asserted size; will throw on invalid size
#if DEBUG
template <class T>
T *NewArray(upx_uint64_t n) {
    COMPILE_TIME_ASSERT(std::is_standard_layout<T>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_copyable<T>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_default_constructible<T>::value)
    size_t bytes = mem_size(sizeof(T), n); // assert size
    T *array = new T[size_t(n)];
#if !defined(__SANITIZE_MEMORY__)
    if (array != nullptr && bytes > 0) {
        memset(array, 0xfb, bytes);
        (void) VALGRIND_MAKE_MEM_UNDEFINED(array, bytes);
    }
#endif
    UNUSED(bytes);
    return array;
}
#define New(type, n) (NewArray<type>((n)))
#else
#define New(type, n) new type[mem_size_get_n(sizeof(type), (n))]
#endif

/*************************************************************************
// ptr util
**************************************************************************/

// ptrdiff_t with nullptr checks and asserted size; will throw on failure
// NOTE: returns size_in_bytes, not number of elements!
int ptr_diff_bytes(const void *a, const void *b);
unsigned ptr_udiff_bytes(const void *a, const void *b); // asserts a >= b

// short names "ptr_diff" and "ptr_udiff" for types with sizeof(X) == 1
template <class T, class U>
inline typename std::enable_if<sizeof(T) == 1 && sizeof(U) == 1, int>::type ptr_diff(const T *a,
                                                                                     const U *b) {
    return ptr_diff_bytes(a, b);
}
template <class T, class U>
inline typename std::enable_if<sizeof(T) == 1 && sizeof(U) == 1, unsigned>::type
ptr_udiff(const T *a, const U *b) {
    return ptr_udiff_bytes(a, b);
}

// check that buffers do not overlap; will throw on error
noinline void uintptr_check_no_overlap(upx_uintptr_t a, size_t a_size, upx_uintptr_t b,
                                       size_t b_size);
noinline void uintptr_check_no_overlap(upx_uintptr_t a, size_t a_size, upx_uintptr_t b,
                                       size_t b_size, upx_uintptr_t c, size_t c_size);

forceinline void ptr_check_no_overlap(const void *a, size_t a_size, const void *b, size_t b_size) {
    uintptr_check_no_overlap((upx_uintptr_t) a, a_size, (upx_uintptr_t) b, b_size);
}
forceinline void ptr_check_no_overlap(const void *a, size_t a_size, const void *b, size_t b_size,
                                      const void *c, size_t c_size) {
    uintptr_check_no_overlap((upx_uintptr_t) a, a_size, (upx_uintptr_t) b, b_size,
                             (upx_uintptr_t) c, c_size);
}

// invalidate and poison a pointer: point to a non-null invalid address
// - resulting pointer should crash on dereference
// - this should be efficient, so no mmap() guard page etc.
// - this should play nice with runtime checkers like ASAN, MSAN, valgrind, etc.
// - this should play nice with static analyzers like clang-tidy etc.
template <class T>
inline void ptr_invalidate_and_poison(T *(&ptr)) noexcept {
    ptr = (T *) (void *) 251; // 0x000000fb // NOLINT(performance-no-int-to-ptr)
}

/*************************************************************************
// stdlib
**************************************************************************/

void *upx_calloc(size_t n, size_t element_size);

void upx_memswap(void *a, void *b, size_t n);

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
#define upx_qsort qsort
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

unsigned get_ratio(upx_uint64_t u_len, upx_uint64_t c_len);
bool set_method_name(char *buf, size_t size, int method, int level);
void center_string(char *buf, size_t size, const char *s);

/* vim:set ts=4 sw=4 et: */

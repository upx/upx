/* xspan -- a minimally invasive checked memory smart pointer

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
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

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */

// manually forward a number of well-known functions using a checked "raw_bytes()" call
//
// uses C
// uses optional D, E and XSPAN_FWD_C_IS_MEMBUFFER
// needs XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION
//
// example for a default implementation:
//   #define XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(A, B, RType)
//       std::enable_if_t<std::is_convertible_v<A *, B *> || std::is_convertible_v<B *, A *>, RType>

// requires convertible T* to U* or U* to T*
#define XSPAN_FWD_TU_CONVERTIBLE(RType)                                                            \
    template <class T, class U>                                                                    \
    inline XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(T, U, RType)
// requires convertible to void* (i.e. any pointer type matches)
#define XSPAN_FWD_TU_VOIDPTR(RType)                                                                \
    template <class T, class U>                                                                    \
    inline RType

/*************************************************************************
// overloads of global operators
**************************************************************************/

#ifndef XSPAN_FWD_C_IS_MEMBUFFER

// global operator: disallow "n + C" => force using "C + n" (member function) instead
template <class T, class U>
inline typename std::enable_if<std::is_integral<U>::value, void *>::type operator+(U, const C<T> &)
    XSPAN_DELETED_FUNCTION;

#endif // XSPAN_FWD_C_IS_MEMBUFFER

/*************************************************************************
// overloads for standard functions
**************************************************************************/

template <class T>
inline void *memchr(const C<T> &a, int v, size_t n) {
    return memchr(a.raw_bytes(n), v, n);
}
template <class T>
inline const void *memchr(const C<const T> &a, int v, size_t n) {
    return memchr(a.raw_bytes(n), v, n);
}

template <class T>
inline int memcmp(const C<T> &a, const void *b, size_t n) {
    return memcmp(a.raw_bytes(n), b, n);
}
template <class T>
inline int memcmp(const void *a, const C<T> &b, size_t n) {
    return memcmp(a, b.raw_bytes(n), n);
}
XSPAN_FWD_TU_VOIDPTR(int) memcmp(const C<T> &a, const C<U> &b, size_t n) {
    return memcmp(a.raw_bytes(n), b.raw_bytes(n), n);
}
#ifdef D
XSPAN_FWD_TU_VOIDPTR(int) memcmp(const C<T> &a, const D<U> &b, size_t n) {
    return memcmp(a.raw_bytes(n), b.raw_bytes(n), n);
}
#endif
#ifdef E
XSPAN_FWD_TU_VOIDPTR(int) memcmp(const C<T> &a, const E<U> &b, size_t n) {
    return memcmp(a.raw_bytes(n), b.raw_bytes(n), n);
}
#endif

template <class T>
inline void *memcpy(C<T> a, const void *b, size_t n) {
    return memcpy(a.raw_bytes(n), b, n);
}
template <class T>
inline void *memcpy(void *a, const C<T> &b, size_t n) {
    return memcpy(a, b.raw_bytes(n), n);
}
XSPAN_FWD_TU_VOIDPTR(void *) memcpy(const C<T> &a, const C<U> &b, size_t n) {
    return memcpy(a.raw_bytes(n), b.raw_bytes(n), n);
}
#ifdef D
XSPAN_FWD_TU_VOIDPTR(void *) memcpy(const C<T> &a, const D<U> &b, size_t n) {
    return memcpy(a.raw_bytes(n), b.raw_bytes(n), n);
}
#endif
#ifdef E
XSPAN_FWD_TU_VOIDPTR(void *) memcpy(const C<T> &a, const E<U> &b, size_t n) {
    return memcpy(a.raw_bytes(n), b.raw_bytes(n), n);
}
#endif

template <class T>
inline void *memmove(C<T> a, const void *b, size_t n) {
    return memmove(a.raw_bytes(n), b, n);
}
template <class T>
inline void *memmove(void *a, const C<T> &b, size_t n) {
    return memmove(a, b.raw_bytes(n), n);
}
XSPAN_FWD_TU_VOIDPTR(void *) memmove(const C<T> &a, const C<U> &b, size_t n) {
    return memmove(a.raw_bytes(n), b.raw_bytes(n), n);
}
#ifdef D
XSPAN_FWD_TU_VOIDPTR(void *) memmove(const C<T> &a, const D<U> &b, size_t n) {
    return memmove(a.raw_bytes(n), b.raw_bytes(n), n);
}
#endif
#ifdef E
XSPAN_FWD_TU_VOIDPTR(void *) memmove(const C<T> &a, const E<U> &b, size_t n) {
    return memmove(a.raw_bytes(n), b.raw_bytes(n), n);
}
#endif

template <class T>
inline void *memset(const C<T> &a, int v, size_t n) {
    return memset(a.raw_bytes(n), v, n);
}

/*************************************************************************
// overloads for UPX extras 1
**************************************************************************/

template <class T>
inline int ptr_diff_bytes(const C<T> &a, const void *b) {
    return ptr_diff_bytes(a.raw_bytes(0), b);
}
template <class T>
inline int ptr_diff_bytes(const void *a, const C<T> &b) {
    return ptr_diff_bytes(a, b.raw_bytes(0));
}
XSPAN_FWD_TU_VOIDPTR(int) ptr_diff_bytes(const C<T> &a, const C<U> &b) {
    return ptr_diff_bytes(a.raw_bytes(0), b.raw_bytes(0));
}
#ifdef D
XSPAN_FWD_TU_VOIDPTR(int) ptr_diff_bytes(const C<T> &a, const D<U> &b) {
    return ptr_diff_bytes(a.raw_bytes(0), b.raw_bytes(0));
}
#endif
#ifdef E
XSPAN_FWD_TU_VOIDPTR(int) ptr_diff_bytes(const C<T> &a, const E<U> &b) {
    return ptr_diff_bytes(a.raw_bytes(0), b.raw_bytes(0));
}
#endif

template <class T>
inline unsigned ptr_udiff_bytes(const C<T> &a, const void *b) {
    return ptr_udiff_bytes(a.raw_bytes(0), b);
}
template <class T>
inline unsigned ptr_udiff_bytes(const void *a, const C<T> &b) {
    return ptr_udiff_bytes(a, b.raw_bytes(0));
}
XSPAN_FWD_TU_VOIDPTR(unsigned) ptr_udiff_bytes(const C<T> &a, const C<U> &b) {
    return ptr_udiff_bytes(a.raw_bytes(0), b.raw_bytes(0));
}
#ifdef D
XSPAN_FWD_TU_VOIDPTR(unsigned) ptr_udiff_bytes(const C<T> &a, const D<U> &b) {
    return ptr_udiff_bytes(a.raw_bytes(0), b.raw_bytes(0));
}
#endif
#ifdef E
XSPAN_FWD_TU_VOIDPTR(unsigned) ptr_udiff_bytes(const C<T> &a, const E<U> &b) {
    return ptr_udiff_bytes(a.raw_bytes(0), b.raw_bytes(0));
}
#endif

/*************************************************************************
// overloads for UPX extras 2
**************************************************************************/

#ifdef UPX_VERSION_HEX

template <class T>
inline unsigned upx_adler32(const C<T> &a, unsigned n, unsigned adler = 1) {
    return upx_adler32(a.raw_bytes(n), n, adler);
}

template <class T>
inline unsigned get_ne16(const C<T> &a) {
    return get_ne16(a.raw_bytes(2));
}
template <class T>
inline unsigned get_ne32(const C<T> &a) {
    return get_ne32(a.raw_bytes(4));
}
template <class T>
inline upx_uint64_t get_ne64(const C<T> &a) {
    return get_ne64(a.raw_bytes(8));
}

template <class T>
inline unsigned get_be16(const C<T> &a) {
    return get_be16(a.raw_bytes(2));
}
template <class T>
inline unsigned get_be32(const C<T> &a) {
    return get_be32(a.raw_bytes(4));
}
template <class T>
inline upx_uint64_t get_be64(const C<T> &a) {
    return get_be64(a.raw_bytes(8));
}

template <class T>
inline unsigned get_le16(const C<T> &a) {
    return get_le16(a.raw_bytes(2));
}
template <class T>
inline unsigned get_le24(const C<T> &a) {
    return get_le24(a.raw_bytes(3));
}
template <class T>
inline unsigned get_le32(const C<T> &a) {
    return get_le32(a.raw_bytes(4));
}
template <class T>
inline upx_uint64_t get_le64(const C<T> &a) {
    return get_le64(a.raw_bytes(8));
}

template <class T>
inline void set_ne16(const C<T> &a, unsigned v) {
    return set_ne16(a.raw_bytes(2), v);
}
template <class T>
inline void set_ne32(const C<T> &a, unsigned v) {
    return set_ne32(a.raw_bytes(4), v);
}
template <class T>
inline void set_ne64(const C<T> &a, upx_uint64_t v) {
    return set_ne64(a.raw_bytes(8), v);
}

template <class T>
inline void set_be16(const C<T> &a, unsigned v) {
    return set_be16(a.raw_bytes(2), v);
}
template <class T>
inline void set_be32(const C<T> &a, unsigned v) {
    return set_be32(a.raw_bytes(4), v);
}
template <class T>
inline void set_be64(const C<T> &a, upx_uint64_t v) {
    return set_be64(a.raw_bytes(8), v);
}

template <class T>
inline void set_le16(const C<T> &a, unsigned v) {
    return set_le16(a.raw_bytes(2), v);
}
template <class T>
inline void set_le24(const C<T> &a, unsigned v) {
    return set_le24(a.raw_bytes(3), v);
}
template <class T>
inline void set_le32(const C<T> &a, unsigned v) {
    return set_le32(a.raw_bytes(4), v);
}
template <class T>
inline void set_le64(const C<T> &a, upx_uint64_t v) {
    return set_le64(a.raw_bytes(8), v);
}

#ifndef XSPAN_FWD_C_IS_MEMBUFFER
template <class T>
inline C<T> operator+(const C<T> &a, const BE16 &v) {
    return a + unsigned(v);
}
template <class T>
inline C<T> operator+(const C<T> &a, const BE32 &v) {
    return a + unsigned(v);
}
template <class T>
inline C<T> operator+(const C<T> &a, const LE16 &v) {
    return a + unsigned(v);
}
template <class T>
inline C<T> operator+(const C<T> &a, const LE32 &v) {
    return a + unsigned(v);
}

template <class T>
inline C<T> operator-(const C<T> &a, const BE16 &v) {
    return a - unsigned(v);
}
template <class T>
inline C<T> operator-(const C<T> &a, const BE32 &v) {
    return a - unsigned(v);
}
template <class T>
inline C<T> operator-(const C<T> &a, const LE16 &v) {
    return a - unsigned(v);
}
template <class T>
inline C<T> operator-(const C<T> &a, const LE32 &v) {
    return a - unsigned(v);
}
#endif // XSPAN_FWD_C_IS_MEMBUFFER

template <class T>
typename std::enable_if<sizeof(T) == 1, upx_rsize_t>::type upx_safe_strlen(const C<T> &a) {
    // not fully checked, but can require at least 1 byte
    upx_rsize_t len = upx_safe_strlen(a.raw_bytes(1));
    (void) a.raw_bytes(len + 1); // now can do a full check
    return len;
}

#endif // UPX_VERSION_HEX

#undef XSPAN_FWD_TU_CONVERTIBLE
#undef XSPAN_FWD_TU_VOIDPTR

/* vim:set ts=4 sw=4 et: */

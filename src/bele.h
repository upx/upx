/* bele.h -- access memory in BigEndian and LittleEndian byte order

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2005 Laszlo Molnar
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


#ifndef __UPX_BELE_H
#define __UPX_BELE_H


/*************************************************************************
// core
**************************************************************************/

inline unsigned get_be16(const void *p)
{
#if defined(ACC_UA_GET_BE16)
    return ACC_UA_GET_BE16(p);
#else
    return acc_ua_get_be16(p);
#endif
}

inline void set_be16(void *p, unsigned v)
{
#if defined(ACC_UA_SET_BE16)
    ACC_UA_SET_BE16(p, v);
#else
    acc_ua_set_be16(p, v);
#endif
}

inline unsigned get_be24(const void *p)
{
#if defined(ACC_UA_GET_BE24)
    return ACC_UA_GET_BE24(p);
#else
    return acc_ua_get_be24(p);
#endif
}

inline void set_be24(void *p, unsigned v)
{
#if defined(ACC_UA_SET_BE24)
    ACC_UA_SET_BE24(p, v);
#else
    acc_ua_set_be24(p, v);
#endif
}

inline unsigned get_be32(const void *p)
{
#if defined(ACC_UA_GET_BE32)
    return ACC_UA_GET_BE32(p);
#else
    return acc_ua_get_be32(p);
#endif
}

inline void set_be32(void *p, unsigned v)
{
#if defined(ACC_UA_SET_BE32)
    ACC_UA_SET_BE32(p, v);
#else
    acc_ua_set_be32(p, v);
#endif
}

inline acc_uint64l_t get_be64(const void *p)
{
#if defined(ACC_UA_GET_BE64)
    return ACC_UA_GET_BE64(p);
#else
    return acc_ua_get_be64(p);
#endif
}

inline void set_be64(void *p, acc_uint64l_t v)
{
#if defined(ACC_UA_SET_BE64)
    ACC_UA_SET_BE64(p, v);
#else
    acc_ua_set_be64(p, v);
#endif
}

inline unsigned get_le16(const void *p)
{
#if defined(ACC_UA_GET_LE16)
    return ACC_UA_GET_LE16(p);
#else
    return acc_ua_get_le16(p);
#endif
}

inline void set_le16(void *p, unsigned v)
{
#if defined(ACC_UA_SET_LE16)
    ACC_UA_SET_LE16(p, v);
#else
    acc_ua_set_le16(p, v);
#endif
}

inline unsigned get_le24(const void *p)
{
#if defined(ACC_UA_GET_LE24)
    return ACC_UA_GET_LE24(p);
#else
    return acc_ua_get_le24(p);
#endif
}

inline void set_le24(void *p, unsigned v)
{
#if defined(ACC_UA_SET_LE24)
    ACC_UA_SET_LE24(p, v);
#else
    acc_ua_set_le24(p, v);
#endif
}

inline unsigned get_le32(const void *p)
{
#if defined(ACC_UA_GET_LE32)
    return ACC_UA_GET_LE32(p);
#else
    return acc_ua_get_le32(p);
#endif
}

inline void set_le32(void *p, unsigned v)
{
#if defined(ACC_UA_SET_LE32)
    ACC_UA_SET_LE32(p, v);
#else
    acc_ua_set_le32(p, v);
#endif
}

inline acc_uint64l_t get_le64(const void *p)
{
#if defined(ACC_UA_GET_LE64)
    return ACC_UA_GET_LE64(p);
#else
    return acc_ua_get_le64(p);
#endif
}

inline void set_le64(void *p, acc_uint64l_t v)
{
#if defined(ACC_UA_SET_LE64)
    ACC_UA_SET_LE64(p, v);
#else
    acc_ua_set_le64(p, v);
#endif
}


/*************************************************************************
// get signed values, i.e. sign-extend
**************************************************************************/

inline int sign_extend(int v, int bits)
{
    const unsigned sign_bit = 1u << (bits - 1);
    v |= 0u - (v & sign_bit);
    return v;
}

inline acc_int64l_t sign_extend(acc_int64l_t v, int bits)
{
    const acc_uint64l_t sign_bit = ACC_UINT64_C(1) << (bits - 1);
    v |= ACC_UINT64_C(0) - (v & sign_bit);
    return v;
}

inline int get_be16_signed(const void *p)
{
    int v = get_be16(p);
    return sign_extend(v, 16);
}

inline int get_be24_signed(const void *p)
{
    int v = get_be24(p);
    return sign_extend(v, 24);
}

inline int get_be32_signed(const void *p)
{
    int v = get_be32(p);
    return sign_extend(v, 32);
}

inline acc_int64l_t get_be64_signed(const void *p)
{
    acc_int64l_t v = get_be64(p);
    return sign_extend(v, 64);
}

inline int get_le16_signed(const void *p)
{
    int v = get_le16(p);
    return sign_extend(v, 16);
}

inline int get_le24_signed(const void *p)
{
    int v = get_le24(p);
    return sign_extend(v, 24);
}

inline int get_le32_signed(const void *p)
{
    int v = get_le32(p);
    return sign_extend(v, 32);
}

inline int get_le64_signed(const void *p)
{
    acc_int64l_t v = get_le64(p);
    return sign_extend(v, 64);
}


/*************************************************************************
// classes for portable unaligned access
//
// Important: these classes must be PODs (Plain Old Data), i.e. no
//   constructor, no destructor, no virtual functions and no default
//   assignment operator, and all fields must be public(!).
**************************************************************************/

struct BE16
{
    unsigned char d[2];

    BE16& operator =  (unsigned v) { set_be16(d, v); return *this; }
    BE16& operator += (unsigned v) { set_be16(d, get_be16(d) + v); return *this; }
    BE16& operator -= (unsigned v) { set_be16(d, get_be16(d) - v); return *this; }
    BE16& operator *= (unsigned v) { set_be16(d, get_be16(d) * v); return *this; }
    BE16& operator /= (unsigned v) { set_be16(d, get_be16(d) / v); return *this; }
    BE16& operator &= (unsigned v) { set_be16(d, get_be16(d) & v); return *this; }
    BE16& operator |= (unsigned v) { set_be16(d, get_be16(d) | v); return *this; }
    BE16& operator ^= (unsigned v) { set_be16(d, get_be16(d) ^ v); return *this; }
    BE16& operator <<= (unsigned v) { set_be16(d, get_be16(d) << v); return *this; }
    BE16& operator >>= (unsigned v) { set_be16(d, get_be16(d) >> v); return *this; }

    operator unsigned () const  { return get_be16(d); }
}
__attribute_packed;


struct BE32
{
    unsigned char d[4];

    BE32& operator =  (unsigned v) { set_be32(d, v); return *this; }
    BE32& operator += (unsigned v) { set_be32(d, get_be32(d) + v); return *this; }
    BE32& operator -= (unsigned v) { set_be32(d, get_be32(d) - v); return *this; }
    BE32& operator *= (unsigned v) { set_be32(d, get_be32(d) * v); return *this; }
    BE32& operator /= (unsigned v) { set_be32(d, get_be32(d) / v); return *this; }
    BE32& operator &= (unsigned v) { set_be32(d, get_be32(d) & v); return *this; }
    BE32& operator |= (unsigned v) { set_be32(d, get_be32(d) | v); return *this; }
    BE32& operator ^= (unsigned v) { set_be32(d, get_be32(d) ^ v); return *this; }
    BE32& operator <<= (unsigned v) { set_be32(d, get_be32(d) << v); return *this; }
    BE32& operator >>= (unsigned v) { set_be32(d, get_be32(d) >> v); return *this; }

    operator unsigned () const  { return get_be32(d); }
}
__attribute_packed;


struct BE64
{
    unsigned char d[8];

    BE64& operator =  (acc_uint64l_t v) { set_be64(d, v); return *this; }
    BE64& operator += (acc_uint64l_t v) { set_be64(d, get_be64(d) + v); return *this; }
    BE64& operator -= (acc_uint64l_t v) { set_be64(d, get_be64(d) - v); return *this; }
    BE64& operator *= (acc_uint64l_t v) { set_be64(d, get_be64(d) * v); return *this; }
    BE64& operator /= (acc_uint64l_t v) { set_be64(d, get_be64(d) / v); return *this; }
    BE64& operator &= (acc_uint64l_t v) { set_be64(d, get_be64(d) & v); return *this; }
    BE64& operator |= (acc_uint64l_t v) { set_be64(d, get_be64(d) | v); return *this; }
    BE64& operator ^= (acc_uint64l_t v) { set_be64(d, get_be64(d) ^ v); return *this; }
    BE64& operator <<= (unsigned v) { set_be64(d, get_be64(d) << v); return *this; }
    BE64& operator >>= (unsigned v) { set_be64(d, get_be64(d) >> v); return *this; }

    operator acc_uint64l_t () const  { return get_be64(d); }
}
__attribute_packed;


struct LE16
{
    unsigned char d[2];

    LE16& operator =  (unsigned v) { set_le16(d, v); return *this; }
    LE16& operator += (unsigned v) { set_le16(d, get_le16(d) + v); return *this; }
    LE16& operator -= (unsigned v) { set_le16(d, get_le16(d) - v); return *this; }
    LE16& operator *= (unsigned v) { set_le16(d, get_le16(d) * v); return *this; }
    LE16& operator /= (unsigned v) { set_le16(d, get_le16(d) / v); return *this; }
    LE16& operator &= (unsigned v) { set_le16(d, get_le16(d) & v); return *this; }
    LE16& operator |= (unsigned v) { set_le16(d, get_le16(d) | v); return *this; }
    LE16& operator ^= (unsigned v) { set_le16(d, get_le16(d) ^ v); return *this; }
    LE16& operator <<= (unsigned v) { set_le16(d, get_le16(d) << v); return *this; }
    LE16& operator >>= (unsigned v) { set_le16(d, get_le16(d) >> v); return *this; }

    operator unsigned () const  { return get_le16(d); }
}
__attribute_packed;


struct LE32
{
    unsigned char d[4];

    LE32& operator =  (unsigned v) { set_le32(d, v); return *this; }
    LE32& operator += (unsigned v) { set_le32(d, get_le32(d) + v); return *this; }
    LE32& operator -= (unsigned v) { set_le32(d, get_le32(d) - v); return *this; }
    LE32& operator *= (unsigned v) { set_le32(d, get_le32(d) * v); return *this; }
    LE32& operator /= (unsigned v) { set_le32(d, get_le32(d) / v); return *this; }
    LE32& operator &= (unsigned v) { set_le32(d, get_le32(d) & v); return *this; }
    LE32& operator |= (unsigned v) { set_le32(d, get_le32(d) | v); return *this; }
    LE32& operator ^= (unsigned v) { set_le32(d, get_le32(d) ^ v); return *this; }
    LE32& operator <<= (unsigned v) { set_le32(d, get_le32(d) << v); return *this; }
    LE32& operator >>= (unsigned v) { set_le32(d, get_le32(d) >> v); return *this; }

    operator unsigned () const  { return get_le32(d); }
}
__attribute_packed;


struct LE64
{
    unsigned char d[8];

    LE64& operator =  (acc_uint64l_t v) { set_le64(d, v); return *this; }
    LE64& operator += (acc_uint64l_t v) { set_le64(d, get_le64(d) + v); return *this; }
    LE64& operator -= (acc_uint64l_t v) { set_le64(d, get_le64(d) - v); return *this; }
    LE64& operator *= (acc_uint64l_t v) { set_le64(d, get_le64(d) * v); return *this; }
    LE64& operator /= (acc_uint64l_t v) { set_le64(d, get_le64(d) / v); return *this; }
    LE64& operator &= (acc_uint64l_t v) { set_le64(d, get_le64(d) & v); return *this; }
    LE64& operator |= (acc_uint64l_t v) { set_le64(d, get_le64(d) | v); return *this; }
    LE64& operator ^= (acc_uint64l_t v) { set_le64(d, get_le64(d) ^ v); return *this; }
    LE64& operator <<= (unsigned v) { set_le64(d, get_le64(d) << v); return *this; }
    LE64& operator >>= (unsigned v) { set_le64(d, get_le64(d) >> v); return *this; }

    operator acc_uint64l_t () const  { return get_le64(d); }
}
__attribute_packed;


/*************************************************************************
// global operators
**************************************************************************/

template <class T>
inline T* operator + (T* ptr, const BE16& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator + (const BE16& v, T* ptr) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const BE16& v) { return ptr - (unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const BE32& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator + (const BE32& v, T* ptr) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const BE32& v) { return ptr - (unsigned) v; }

// these are not implemented on purpose and will cause link-time errors
template <class T> T* operator + (T* ptr, const BE64& v);
template <class T> T* operator + (const BE64& v, T* ptr);
template <class T> T* operator - (T* ptr, const BE64& v);

template <class T>
inline T* operator + (T* ptr, const LE16& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator + (const LE16& v, T* ptr) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const LE16& v) { return ptr - (unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const LE32& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator + (const LE32& v, T* ptr) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const LE32& v) { return ptr - (unsigned) v; }

// these are not implemented on purpose and will cause link-time errors
template <class T> T* operator + (T* ptr, const LE64& v);
template <class T> T* operator + (const LE64& v, T* ptr);
template <class T> T* operator - (T* ptr, const LE64& v);


/*************************************************************************
// misc
**************************************************************************/

// for use with qsort()
int __acc_cdecl_qsort be16_compare(const void *, const void *);
int __acc_cdecl_qsort be24_compare(const void *, const void *);
int __acc_cdecl_qsort be32_compare(const void *, const void *);
int __acc_cdecl_qsort be64_compare(const void *, const void *);
int __acc_cdecl_qsort le16_compare(const void *, const void *);
int __acc_cdecl_qsort le24_compare(const void *, const void *);
int __acc_cdecl_qsort le32_compare(const void *, const void *);
int __acc_cdecl_qsort le64_compare(const void *, const void *);
int __acc_cdecl_qsort be16_compare_signed(const void *, const void *);
int __acc_cdecl_qsort be24_compare_signed(const void *, const void *);
int __acc_cdecl_qsort be32_compare_signed(const void *, const void *);
int __acc_cdecl_qsort be64_compare_signed(const void *, const void *);
int __acc_cdecl_qsort le16_compare_signed(const void *, const void *);
int __acc_cdecl_qsort le24_compare_signed(const void *, const void *);
int __acc_cdecl_qsort le32_compare_signed(const void *, const void *);
int __acc_cdecl_qsort le64_compare_signed(const void *, const void *);


// just for testing...
#if !defined(ACC_CFG_NO_UNALIGNED)
#if 0 && (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_CC_GNUC >= 0x030200)
   typedef acc_uint16e_t LE16_unaligned __attribute__((__aligned__(1)));
   typedef acc_uint32e_t LE32_unaligned __attribute__((__aligned__(1)));
#  ifndef LE16
#    define LE16    LE16_unaligned
#  endif
#  ifndef LE32
#    define LE32    LE32_unaligned
#  endif
#endif
#if 0 && (ACC_ARCH_I386) && (ACC_CC_INTELC)
   typedef __declspec(align(1)) acc_uint16e_t LE16_unaligned;
   typedef __declspec(align(1)) acc_uint32e_t LE32_unaligned;
#  ifndef LE16
#    define LE16    LE16_unaligned
#  endif
#  ifndef LE32
#    define LE32    LE32_unaligned
#  endif
#endif
#if 0 && (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_CC_MSC) && (_MSC_VER >= 1200)
   typedef __declspec(align(1)) acc_uint16e_t LE16_unaligned;
   typedef __declspec(align(1)) acc_uint32e_t LE32_unaligned;
#  ifndef LE16
#    define LE16    LE16_unaligned
#  endif
#  ifndef LE32
#    define LE32    LE32_unaligned
#  endif
#  pragma warning(disable: 4244)        // Wx: conversion, possible loss of data
#endif
#endif


#endif /* already included */


/*
vi:ts=4:et:nowrap
*/


/* bele.h -- access memory in BigEndian and LittleEndian byte order

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


#ifndef __UPX_BELE_H
#define __UPX_BELE_H


/*************************************************************************
// core
**************************************************************************/

inline unsigned get_be16(const void *bb)
{
    const unsigned char* b = (const unsigned char*) bb;
    unsigned v;
    v  = (unsigned) b[1] <<  0;
    v |= (unsigned) b[0] <<  8;
    return v;
}

inline void set_be16(void *bb, unsigned v)
{
    unsigned char* b = (unsigned char*) bb;
    b[1] = (unsigned char) (v >>  0);
    b[0] = (unsigned char) (v >>  8);
}


inline unsigned get_be24(const void *bb)
{
    const unsigned char* b = (const unsigned char*) bb;
    unsigned v;
    v  = (unsigned) b[2] <<  0;
    v |= (unsigned) b[1] <<  8;
    v |= (unsigned) b[0] << 16;
    return v;
}

inline void set_be24(void *bb, unsigned v)
{
    unsigned char* b = (unsigned char*) bb;
    b[2] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
    b[0] = (unsigned char) (v >> 16);
}


inline unsigned get_be32(const void *bb)
{
    const unsigned char* b = (const unsigned char*) bb;
    unsigned v;
    v  = (unsigned) b[3] <<  0;
    v |= (unsigned) b[2] <<  8;
    v |= (unsigned) b[1] << 16;
    v |= (unsigned) b[0] << 24;
    return v;
}

inline void set_be32(void *bb, unsigned v)
{
    unsigned char* b = (unsigned char*) bb;
    b[3] = (unsigned char) (v >>  0);
    b[2] = (unsigned char) (v >>  8);
    b[1] = (unsigned char) (v >> 16);
    b[0] = (unsigned char) (v >> 24);
}


inline unsigned get_le16(const void *bb)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
    return * (const unsigned short *) bb;
#else
    const unsigned char* b = (const unsigned char*) bb;
    unsigned v;
    v  = (unsigned) b[0] <<  0;
    v |= (unsigned) b[1] <<  8;
    return v;
#endif
}

inline void set_le16(void *bb, unsigned v)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
    (* (unsigned short *) bb) = (unsigned short) (v & 0xffff);
#else
    unsigned char* b = (unsigned char*) bb;
    b[0] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
#endif
}


inline unsigned get_le24(const void *bb)
{
    const unsigned char* b = (const unsigned char*) bb;
    unsigned v;
    v  = (unsigned) b[0] <<  0;
    v |= (unsigned) b[1] <<  8;
    v |= (unsigned) b[2] << 16;
    return v;
}

inline void set_le24(void *bb, unsigned v)
{
    unsigned char* b = (unsigned char*) bb;
    b[0] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
    b[2] = (unsigned char) (v >> 16);
}


inline unsigned get_le32(const void *bb)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
    return * (const acc_uint32e_t *) bb;
#else
    const unsigned char* b = (const unsigned char*) bb;
    unsigned v;
    v  = (unsigned) b[0] <<  0;
    v |= (unsigned) b[1] <<  8;
    v |= (unsigned) b[2] << 16;
    v |= (unsigned) b[3] << 24;
    return v;
#endif
}

inline void set_le32(void *bb, unsigned v)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
    (* (acc_uint32e_t *) bb) = v;
#else
    unsigned char* b = (unsigned char*) bb;
    b[0] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
    b[2] = (unsigned char) (v >> 16);
    b[3] = (unsigned char) (v >> 24);
#endif
}


/*************************************************************************
// get signed values, i.e. sign-extend
**************************************************************************/

inline int sign_extend(int v, int bits)
{
    const int sign_bit = 1 << (bits - 1);
    v |= -(v & sign_bit);
    return v;
}

inline int get_be16_signed(const void *bb)
{
    int v = get_be16(bb);
    return sign_extend(v, 16);
}

inline int get_be24_signed(const void *bb)
{
    int v = get_be24(bb);
    return sign_extend(v, 24);
}

inline int get_be32_signed(const void *bb)
{
    int v = get_be32(bb);
    return sign_extend(v, 32);
}

inline int get_le16_signed(const void *bb)
{
    int v = get_le16(bb);
    return sign_extend(v, 16);
}

inline int get_le24_signed(const void *bb)
{
    int v = get_le24(bb);
    return sign_extend(v, 24);
}

inline int get_le32_signed(const void *bb)
{
    int v = get_le32(bb);
    return sign_extend(v, 32);
}


/*************************************************************************
// classes for portable unaligned access
**************************************************************************/

class BE16
{
    unsigned char d[2];

public:
    BE16() { }
    BE16& operator =  (const BE16 &v) {
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
        * (acc_uint32e_t *) d = * (const acc_uint32e_t *) v.d;
#else
        memcpy(d, v.d, sizeof(d));
#endif
        return *this;
    }

    BE16& operator =  (unsigned v)    { set_be16(d, v); return *this; }
    BE16& operator += (unsigned v)    { set_be16(d, get_be16(d) + v); return *this; }
    BE16& operator -= (unsigned v)    { set_be16(d, get_be16(d) - v); return *this; }
    BE16& operator &= (unsigned v)    { set_be16(d, get_be16(d) & v); return *this; }
    BE16& operator |= (unsigned v)    { set_be16(d, get_be16(d) | v); return *this; }

    operator unsigned () const  { return get_be16(d); }
}
__attribute_packed;


class BE32
{
    unsigned char d[4];

public:
    BE32() { }
    BE32& operator =  (const BE32 &v) {
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
        * (unsigned int *) d = * (const unsigned int *) v.d;
#else
        memcpy(d, v.d, sizeof(d));
#endif
        return *this;
    }

    BE32& operator =  (unsigned v)    { set_be32(d, v); return *this; }
    BE32& operator += (unsigned v)    { set_be32(d, get_be32(d) + v); return *this; }
    BE32& operator -= (unsigned v)    { set_be32(d, get_be32(d) - v); return *this; }
    BE32& operator &= (unsigned v)    { set_be32(d, get_be32(d) & v); return *this; }
    BE32& operator |= (unsigned v)    { set_be32(d, get_be32(d) | v); return *this; }

    operator unsigned () const  { return get_be32(d); }
}
__attribute_packed;


class LE16
{
    unsigned char d[2];

public:
    LE16() { }
    LE16& operator =  (const LE16 &v) {
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
        * (unsigned short *) d = * (const unsigned short *) v.d;
#else
        memcpy(d, v.d, sizeof(d));
#endif
        return *this;
    }

    LE16& operator =  (unsigned v)    { set_le16(d, v); return *this; }
    LE16& operator += (unsigned v)    { set_le16(d, get_le16(d) + v); return *this; }
    LE16& operator -= (unsigned v)    { set_le16(d, get_le16(d) - v); return *this; }
    LE16& operator &= (unsigned v)    { set_le16(d, get_le16(d) & v); return *this; }
    LE16& operator |= (unsigned v)    { set_le16(d, get_le16(d) | v); return *this; }

    operator unsigned () const  { return get_le16(d); }
}
__attribute_packed;


class LE32
{
    unsigned char d[4];

public:
    LE32() { }
    LE32& operator =  (const LE32 &v) {
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
        * (acc_uint32e_t *) d = * (const acc_uint32e_t *) v.d;
#else
        memcpy(d, v.d, sizeof(d));
#endif
        return *this;
    }

    LE32& operator =  (unsigned v)    { set_le32(d, v); return *this; }
    LE32& operator += (unsigned v)    { set_le32(d, get_le32(d) + v); return *this; }
    LE32& operator -= (unsigned v)    { set_le32(d, get_le32(d) - v); return *this; }
    LE32& operator &= (unsigned v)    { set_le32(d, get_le32(d) & v); return *this; }
    LE32& operator |= (unsigned v)    { set_le32(d, get_le32(d) | v); return *this; }

    operator unsigned () const  { return get_le32(d); }
}
__attribute_packed;


/*************************************************************************
// global operators
**************************************************************************/

inline bool operator < (const BE16& v1, const BE16& v2)
{
    return (unsigned)v1 < (unsigned)v2;
}

inline bool operator < (const BE32& v1, const BE32& v2)
{
    return (unsigned)v1 < (unsigned)v2;
}

inline bool operator < (const LE16& v1, const LE16& v2)
{
    return (unsigned)v1 < (unsigned)v2;
}

inline bool operator < (const LE32& v1, const LE32& v2)
{
    return (unsigned)v1 < (unsigned)v2;
}


template <class T>
inline T* operator + (T* ptr, const BE16& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const BE16& v) { return ptr - (unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const BE32& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const BE32& v) { return ptr - (unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const LE16& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const LE16& v) { return ptr - (unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const LE32& v) { return ptr + (unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const LE32& v) { return ptr - (unsigned) v; }


/*************************************************************************
// misc
**************************************************************************/

// for use with qsort()
int __acc_cdecl_qsort be16_compare(const void *e1, const void *e2);
int __acc_cdecl_qsort be24_compare(const void *e1, const void *e2);
int __acc_cdecl_qsort be32_compare(const void *e1, const void *e2);
int __acc_cdecl_qsort le16_compare(const void *e1, const void *e2);
int __acc_cdecl_qsort le24_compare(const void *e1, const void *e2);
int __acc_cdecl_qsort le32_compare(const void *e1, const void *e2);
int __acc_cdecl_qsort be16_compare_signed(const void *e1, const void *e2);
int __acc_cdecl_qsort be24_compare_signed(const void *e1, const void *e2);
int __acc_cdecl_qsort be32_compare_signed(const void *e1, const void *e2);
int __acc_cdecl_qsort le16_compare_signed(const void *e1, const void *e2);
int __acc_cdecl_qsort le24_compare_signed(const void *e1, const void *e2);
int __acc_cdecl_qsort le32_compare_signed(const void *e1, const void *e2);


// just for testing...
#if 1 && (ACC_ARCH_AMD64 || ACC_ARCH_IA32) && (ACC_CC_GNUC >= 0x030200)
   typedef unsigned short LE16_unaligned __attribute__((__aligned__(1)));
   typedef acc_uint32e_t  LE32_unaligned __attribute__((__aligned__(1)));
#  define LE16      LE16_unaligned
#  define LE32      LE32_unaligned
#endif
#if 0 && (ACC_ARCH_IA32) && (ACC_CC_INTELC)
   typedef __declspec(align(1)) unsigned short LE16_unaligned;
   typedef __declspec(align(1)) acc_uint32e_t  LE32_unaligned;
#  define LE16      LE16_unaligned
#  define LE32      LE32_unaligned
#endif
#if 0 && (ACC_ARCH_AMD64 || ACC_ARCH_IA32) && (ACC_CC_MSC) && (_MSC_VER >= 1200)
   typedef __declspec(align(1)) unsigned short LE16_unaligned;
   typedef __declspec(align(1)) acc_uint32e_t  LE32_unaligned;
#  define LE16      LE16_unaligned
#  define LE32      LE32_unaligned
#  pragma warning(disable: 4244)        // Wx: conversion, possible loss of data
#endif


#endif /* already included */


/*
vi:ts=4:et:nowrap
*/


/* bele.h -- access memory in BigEndian and LittleEndian byte order

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2002 Laszlo Molnar
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
    const upx_bytep b = (const upx_bytep) bb;
    unsigned v;
    v  = (unsigned) b[1] <<  0;
    v |= (unsigned) b[0] <<  8;
    return v;
}

inline void set_be16(void *bb, unsigned v)
{
    upx_bytep b = (upx_bytep) bb;
    b[1] = (unsigned char) (v >>  0);
    b[0] = (unsigned char) (v >>  8);
}


inline unsigned get_be24(const void *bb)
{
    const upx_bytep b = (const upx_bytep) bb;
    unsigned v;
    v  = (unsigned) b[2] <<  0;
    v |= (unsigned) b[1] <<  8;
    v |= (unsigned) b[0] << 16;
    return v;
}

inline void set_be24(void *bb, unsigned v)
{
    upx_bytep b = (upx_bytep) bb;
    b[2] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
    b[0] = (unsigned char) (v >> 16);
}


inline unsigned get_be32(const void *bb)
{
    const upx_bytep b = (const upx_bytep) bb;
    unsigned v;
    v  = (unsigned) b[3] <<  0;
    v |= (unsigned) b[2] <<  8;
    v |= (unsigned) b[1] << 16;
    v |= (unsigned) b[0] << 24;
    return v;
}

inline void set_be32(void *bb, unsigned v)
{
    upx_bytep b = (upx_bytep) bb;
    b[3] = (unsigned char) (v >>  0);
    b[2] = (unsigned char) (v >>  8);
    b[1] = (unsigned char) (v >> 16);
    b[0] = (unsigned char) (v >> 24);
}


inline unsigned get_le16(const void *bb)
{
#if defined(__i386__)
    return * (const unsigned short *) bb;
#else
    const upx_bytep b = (const upx_bytep) bb;
    unsigned v;
    v  = (unsigned) b[0] <<  0;
    v |= (unsigned) b[1] <<  8;
    return v;
#endif
}

inline void set_le16(void *bb, unsigned v)
{
#if defined(__i386__)
    (* (unsigned short *) bb) = (unsigned short) (v & 0xffff);
#else
    upx_bytep b = (upx_bytep) bb;
    b[0] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
#endif
}


inline unsigned get_le24(const void *bb)
{
    const upx_bytep b = (const upx_bytep) bb;
    unsigned v;
    v  = (unsigned) b[0] <<  0;
    v |= (unsigned) b[1] <<  8;
    v |= (unsigned) b[2] << 16;
    return v;
}

inline void set_le24(void *bb, unsigned v)
{
    upx_bytep b = (upx_bytep) bb;
    b[0] = (unsigned char) (v >>  0);
    b[1] = (unsigned char) (v >>  8);
    b[2] = (unsigned char) (v >> 16);
}


inline unsigned get_le32(const void *bb)
{
#if defined(__i386__)
    return * (const unsigned *) bb;
#else
    const upx_bytep b = (const upx_bytep) bb;
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
#if defined(__i386__)
    (* (unsigned *) bb) = v;
#else
    upx_bytep b = (upx_bytep) bb;
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
    BE16& operator =  (const BE16 &v) { memcpy(d, v.d, sizeof(d)); return *this; }

    BE16& operator =  (unsigned v)    { set_be16(d, v); return *this; }
    BE16& operator += (unsigned v)    { set_be16(d, get_be16(d) + v); return *this; }
    BE16& operator -= (unsigned v)    { set_be16(d, get_be16(d) - v); return *this; }
    BE16& operator &= (unsigned v)    { set_be16(d, get_be16(d) & v); return *this; }
    BE16& operator |= (unsigned v)    { set_be16(d, get_be16(d) | v); return *this; }

    operator const unsigned () const  { return get_be16(d); }
}
__attribute_packed;


class BE32
{
    unsigned char d[4];

public:
    BE32& operator =  (const BE32 &v) { memcpy(d, v.d, sizeof(d)); return *this; }

    BE32& operator =  (unsigned v)    { set_be32(d, v); return *this; }
    BE32& operator += (unsigned v)    { set_be32(d, get_be32(d) + v); return *this; }
    BE32& operator -= (unsigned v)    { set_be32(d, get_be32(d) - v); return *this; }
    BE32& operator &= (unsigned v)    { set_be32(d, get_be32(d) & v); return *this; }
    BE32& operator |= (unsigned v)    { set_be32(d, get_be32(d) | v); return *this; }

    operator const unsigned () const  { return get_be32(d); }
}
__attribute_packed;


class LE16
{
    unsigned char d[2];

public:
    LE16& operator =  (const LE16 &v) { memcpy(d, v.d, sizeof(d)); return *this; }

    LE16& operator =  (unsigned v)    { set_le16(d, v); return *this; }
    LE16& operator += (unsigned v)    { set_le16(d, get_le16(d) + v); return *this; }
    LE16& operator -= (unsigned v)    { set_le16(d, get_le16(d) - v); return *this; }
    LE16& operator &= (unsigned v)    { set_le16(d, get_le16(d) & v); return *this; }
    LE16& operator |= (unsigned v)    { set_le16(d, get_le16(d) | v); return *this; }

    operator const unsigned () const  { return get_le16(d); }
}
__attribute_packed;


class LE32
{
    unsigned char d[4];

public:
    LE32& operator =  (const LE32 &v) { memcpy(d, v.d, sizeof(d)); return *this; }

    LE32& operator =  (unsigned v)    { set_le32(d, v); return *this; }
    LE32& operator += (unsigned v)    { set_le32(d, get_le32(d) + v); return *this; }
    LE32& operator -= (unsigned v)    { set_le32(d, get_le32(d) - v); return *this; }
    LE32& operator &= (unsigned v)    { set_le32(d, get_le32(d) & v); return *this; }
    LE32& operator |= (unsigned v)    { set_le32(d, get_le32(d) | v); return *this; }

    operator const unsigned () const  { return get_le32(d); }
}
__attribute_packed;


/*************************************************************************
// global operators
**************************************************************************/

inline bool operator < (const BE16& v1, const BE16& v2)
{
    return (const unsigned)v1 < (const unsigned)v2;
}

inline bool operator < (const BE32& v1, const BE32& v2)
{
    return (const unsigned)v1 < (const unsigned)v2;
}

inline bool operator < (const LE16& v1, const LE16& v2)
{
    return (const unsigned)v1 < (const unsigned)v2;
}

inline bool operator < (const LE32& v1, const LE32& v2)
{
    return (const unsigned)v1 < (const unsigned)v2;
}


template <class T>
inline T* operator + (T* ptr, const BE16& v) { return ptr + (const unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const BE16& v) { return ptr - (const unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const BE32& v) { return ptr + (const unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const BE32& v) { return ptr - (const unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const LE16& v) { return ptr + (const unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const LE16& v) { return ptr - (const unsigned) v; }

template <class T>
inline T* operator + (T* ptr, const LE32& v) { return ptr + (const unsigned) v; }
template <class T>
inline T* operator - (T* ptr, const LE32& v) { return ptr - (const unsigned) v; }


/*************************************************************************
// misc
**************************************************************************/

// for use with qsort()
int be16_compare(const void *e1, const void *e2);
int be24_compare(const void *e1, const void *e2);
int be32_compare(const void *e1, const void *e2);
int le16_compare(const void *e1, const void *e2);
int le24_compare(const void *e1, const void *e2);
int le32_compare(const void *e1, const void *e2);
int be16_compare_signed(const void *e1, const void *e2);
int be24_compare_signed(const void *e1, const void *e2);
int be32_compare_signed(const void *e1, const void *e2);
int le16_compare_signed(const void *e1, const void *e2);
int le24_compare_signed(const void *e1, const void *e2);
int le32_compare_signed(const void *e1, const void *e2);


// just for testing...
#if 0 && defined(__i386__) && defined(__GNUC_VERSION_HEX__)
# if (__GNUC_VERSION_HEX__ >= 0x030100)
   typedef unsigned short LE16_unaligned __attribute__((__packed__,__aligned__(1)));
   typedef unsigned int   LE32_unaligned __attribute__((__packed__,__aligned__(1)));
#  define LE16      LE16_unaligned
#  define LE32      LE32_unaligned
# endif
#endif


#endif /* already included */


/*
vi:ts=4:et:nowrap
*/


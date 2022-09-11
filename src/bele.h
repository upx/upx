/* bele.h -- access memory in BigEndian and LittleEndian byte order

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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
#ifndef UPX_BELE_H__
#define UPX_BELE_H__ 1

// BE - Big Endian
// LE - Little Endian
// NE - Native Endianness (aka host endianness)
// TE - Target Endianness (not used here, see various packers)

/*************************************************************************
// core - NE
**************************************************************************/

__acc_static_forceinline unsigned get_ne16(const void *p) {
    upx_uint16_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}

__acc_static_forceinline unsigned get_ne32(const void *p) {
    upx_uint32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}

__acc_static_forceinline upx_uint64_t get_ne64(const void *p) {
    upx_uint64_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}

__acc_static_forceinline void set_ne16(void *p, unsigned vv) {
    upx_uint16_t v = (upx_uint16_t)(vv & 0xffff);
    upx_memcpy_inline(p, &v, sizeof(v));
}

__acc_static_forceinline void set_ne32(void *p, unsigned vv) {
    upx_uint32_t v = vv;
    upx_memcpy_inline(p, &v, sizeof(v));
}

__acc_static_forceinline void set_ne64(void *p, upx_uint64_t vv) {
    upx_uint64_t v = vv;
    upx_memcpy_inline(p, &v, sizeof(v));
}

/*************************************************************************
// core - bswap
**************************************************************************/

#if (ACC_CC_MSC)

ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(long) == 4)

__acc_static_forceinline unsigned bswap16(unsigned v) {
    return (unsigned) _byteswap_ulong(v << 16);
}
__acc_static_forceinline unsigned bswap32(unsigned v) { return (unsigned) _byteswap_ulong(v); }
__acc_static_forceinline upx_uint64_t bswap64(upx_uint64_t v) { return _byteswap_uint64(v); }

#else

__acc_static_forceinline constexpr unsigned bswap16(unsigned v) {
    // return __builtin_bswap16((upx_uint16_t) (v & 0xffff));
    // return (unsigned) __builtin_bswap64((upx_uint64_t) v << 48);
    return __builtin_bswap32(v << 16);
}
__acc_static_forceinline constexpr unsigned bswap32(unsigned v) {
    // return (unsigned) __builtin_bswap64((upx_uint64_t) v << 32);
    return __builtin_bswap32(v);
}
__acc_static_forceinline constexpr upx_uint64_t bswap64(upx_uint64_t v) {
    return __builtin_bswap64(v);
}

#endif

__acc_static_forceinline constexpr unsigned no_bswap16(unsigned v) {
    return v & 0xffff; // needed so that this is equivalent to bswap16() above
}

__acc_static_forceinline constexpr unsigned no_bswap32(unsigned v) { return v; }

__acc_static_forceinline constexpr upx_uint64_t no_bswap64(upx_uint64_t v) { return v; }

#if (ACC_ABI_BIG_ENDIAN)
#define ne16_to_be16(v) no_bswap16(v)
#define ne32_to_be32(v) no_bswap32(v)
#define ne64_to_be64(v) no_bswap64(v)
#define ne16_to_le16(v) bswap16(v)
#define ne32_to_le32(v) bswap32(v)
#define ne64_to_le64(v) bswap64(v)
#else
#define ne16_to_be16(v) bswap16(v)
#define ne32_to_be32(v) bswap32(v)
#define ne64_to_be64(v) bswap64(v)
#define ne16_to_le16(v) no_bswap16(v)
#define ne32_to_le32(v) no_bswap32(v)
#define ne64_to_le64(v) no_bswap64(v)
#endif

/*************************************************************************
// get/set 16/32/64
**************************************************************************/

inline unsigned get_be16(const void *p) { return ne16_to_be16(get_ne16(p)); }
inline unsigned get_be32(const void *p) { return ne32_to_be32(get_ne32(p)); }
inline upx_uint64_t get_be64(const void *p) { return ne64_to_be64(get_ne64(p)); }
inline unsigned get_le16(const void *p) { return ne16_to_le16(get_ne16(p)); }
inline unsigned get_le32(const void *p) { return ne32_to_le32(get_ne32(p)); }
inline upx_uint64_t get_le64(const void *p) { return ne64_to_le64(get_ne64(p)); }
inline void set_be16(void *p, unsigned v) { set_ne16(p, ne16_to_be16(v)); }
inline void set_be32(void *p, unsigned v) { set_ne32(p, ne32_to_be32(v)); }
inline void set_be64(void *p, upx_uint64_t v) { set_ne64(p, ne64_to_be64(v)); }
inline void set_le16(void *p, unsigned v) { set_ne16(p, ne16_to_le16(v)); }
inline void set_le32(void *p, unsigned v) { set_ne32(p, ne32_to_le32(v)); }
inline void set_le64(void *p, upx_uint64_t v) { set_ne64(p, ne64_to_le64(v)); }

/*************************************************************************
// get/set 24/26
**************************************************************************/

inline unsigned get_be24(const void *p) {
    const unsigned char *b = ACC_CCAST(const unsigned char *, p);
    return (b[0] << 16) | (b[1] << 8) | (b[2] << 0);
}

inline unsigned get_le24(const void *p) {
    const unsigned char *b = ACC_CCAST(const unsigned char *, p);
    return (b[0] << 0) | (b[1] << 8) | (b[2] << 16);
}

inline void set_be24(void *p, unsigned v) {
    unsigned char *b = ACC_PCAST(unsigned char *, p);
    b[0] = ACC_ICONV(unsigned char, (v >> 16) & 0xff);
    b[1] = ACC_ICONV(unsigned char, (v >> 8) & 0xff);
    b[2] = ACC_ICONV(unsigned char, (v >> 0) & 0xff);
}

inline void set_le24(void *p, unsigned v) {
    unsigned char *b = ACC_PCAST(unsigned char *, p);
    b[0] = ACC_ICONV(unsigned char, (v >> 0) & 0xff);
    b[1] = ACC_ICONV(unsigned char, (v >> 8) & 0xff);
    b[2] = ACC_ICONV(unsigned char, (v >> 16) & 0xff);
}

inline unsigned get_le26(const void *p) { return get_le32(p) & 0x03ffffff; }

inline void set_le26(void *p, unsigned v) {
    // preserve the top 6 bits
    // set_le32(p, (get_le32(p) & 0xfc000000) | (v & 0x03ffffff));
    // optimized version, saving a runtime bswap32
    set_ne32(p, (get_ne32(p) & ne32_to_le32(0xfc000000)) |
                    (ne32_to_le32(v) & ne32_to_le32(0x03ffffff)));
}

/*************************************************************************
// get signed values
**************************************************************************/

__acc_static_forceinline int sign_extend(unsigned v, unsigned bits) {
    const unsigned sign_bit = 1u << (bits - 1);
    v &= sign_bit | (sign_bit - 1);
    v |= 0 - (v & sign_bit);
    return ACC_ICAST(int, v);
}

__acc_static_forceinline upx_int64_t sign_extend(upx_uint64_t v, unsigned bits) {
    const upx_uint64_t sign_bit = 1ull << (bits - 1);
    v &= sign_bit | (sign_bit - 1);
    v |= 0 - (v & sign_bit);
    return ACC_ICAST(upx_int64_t, v);
}

inline int get_be16_signed(const void *p) {
    unsigned v = get_be16(p);
    return sign_extend(v, 16);
}

inline int get_be24_signed(const void *p) {
    unsigned v = get_be24(p);
    return sign_extend(v, 24);
}

inline int get_be32_signed(const void *p) {
    unsigned v = get_be32(p);
    return sign_extend(v, 32);
}

inline upx_int64_t get_be64_signed(const void *p) {
    upx_uint64_t v = get_be64(p);
    return sign_extend(v, 64);
}

inline int get_le16_signed(const void *p) {
    unsigned v = get_le16(p);
    return sign_extend(v, 16);
}

inline int get_le24_signed(const void *p) {
    unsigned v = get_le24(p);
    return sign_extend(v, 24);
}

inline int get_le32_signed(const void *p) {
    unsigned v = get_le32(p);
    return sign_extend(v, 32);
}

inline upx_int64_t get_le64_signed(const void *p) {
    upx_uint64_t v = get_le64(p);
    return sign_extend(v, 64);
}

/*************************************************************************
// classes for portable unaligned access
//
// Important: these classes must be PODs (Plain Old Data), i.e. no
//   constructor, no destructor, no virtual functions and no default
//   assignment operator, and all fields must be public(!).
//
// [Actually we _can_ use a safe non-POD subset, but for this we need
//  to have gcc bug 17519 fixed - see http://gcc.gnu.org/PR17519 ]
**************************************************************************/

struct alignas(1) BE16 {
    unsigned char d[2];

    BE16 &operator=(unsigned v) {
        set_be16(d, v);
        return *this;
    }
    BE16 &operator+=(unsigned v) {
        set_be16(d, get_be16(d) + v);
        return *this;
    }
    BE16 &operator-=(unsigned v) {
        set_be16(d, get_be16(d) - v);
        return *this;
    }
    BE16 &operator*=(unsigned v) {
        set_be16(d, get_be16(d) * v);
        return *this;
    }
    BE16 &operator/=(unsigned v) {
        set_be16(d, get_be16(d) / v);
        return *this;
    }
    BE16 &operator&=(unsigned v) {
        set_be16(d, get_be16(d) & v);
        return *this;
    }
    BE16 &operator|=(unsigned v) {
        set_be16(d, get_be16(d) | v);
        return *this;
    }
    BE16 &operator^=(unsigned v) {
        set_be16(d, get_be16(d) ^ v);
        return *this;
    }
    BE16 &operator<<=(unsigned v) {
        set_be16(d, get_be16(d) << v);
        return *this;
    }
    BE16 &operator>>=(unsigned v) {
        set_be16(d, get_be16(d) >> v);
        return *this;
    }

    operator unsigned() const { return get_be16(d); }

    bool operator<(const BE16 &v) const { return unsigned(*this) < unsigned(v); }
};

struct alignas(1) BE32 {
    unsigned char d[4];

    BE32 &operator=(unsigned v) {
        set_be32(d, v);
        return *this;
    }
    BE32 &operator+=(unsigned v) {
        set_be32(d, get_be32(d) + v);
        return *this;
    }
    BE32 &operator-=(unsigned v) {
        set_be32(d, get_be32(d) - v);
        return *this;
    }
    BE32 &operator*=(unsigned v) {
        set_be32(d, get_be32(d) * v);
        return *this;
    }
    BE32 &operator/=(unsigned v) {
        set_be32(d, get_be32(d) / v);
        return *this;
    }
    BE32 &operator&=(unsigned v) {
        set_be32(d, get_be32(d) & v);
        return *this;
    }
    BE32 &operator|=(unsigned v) {
        set_be32(d, get_be32(d) | v);
        return *this;
    }
    BE32 &operator^=(unsigned v) {
        set_be32(d, get_be32(d) ^ v);
        return *this;
    }
    BE32 &operator<<=(unsigned v) {
        set_be32(d, get_be32(d) << v);
        return *this;
    }
    BE32 &operator>>=(unsigned v) {
        set_be32(d, get_be32(d) >> v);
        return *this;
    }

    operator unsigned() const { return get_be32(d); }

    bool operator<(const BE32 &v) const { return unsigned(*this) < unsigned(v); }
};

struct alignas(1) BE64 {
    unsigned char d[8];

    BE64 &operator=(upx_uint64_t v) {
        set_be64(d, v);
        return *this;
    }
    BE64 &operator+=(upx_uint64_t v) {
        set_be64(d, get_be64(d) + v);
        return *this;
    }
    BE64 &operator-=(upx_uint64_t v) {
        set_be64(d, get_be64(d) - v);
        return *this;
    }
    BE64 &operator*=(upx_uint64_t v) {
        set_be64(d, get_be64(d) * v);
        return *this;
    }
    BE64 &operator/=(upx_uint64_t v) {
        set_be64(d, get_be64(d) / v);
        return *this;
    }
    BE64 &operator&=(upx_uint64_t v) {
        set_be64(d, get_be64(d) & v);
        return *this;
    }
    BE64 &operator|=(upx_uint64_t v) {
        set_be64(d, get_be64(d) | v);
        return *this;
    }
    BE64 &operator^=(upx_uint64_t v) {
        set_be64(d, get_be64(d) ^ v);
        return *this;
    }
    BE64 &operator<<=(unsigned v) {
        set_be64(d, get_be64(d) << v);
        return *this;
    }
    BE64 &operator>>=(unsigned v) {
        set_be64(d, get_be64(d) >> v);
        return *this;
    }

    operator upx_uint64_t() const { return get_be64(d); }

    bool operator<(const BE64 &v) const { return upx_uint64_t(*this) < upx_uint64_t(v); }
};

struct alignas(1) LE16 {
    unsigned char d[2];

    LE16 &operator=(unsigned v) {
        set_le16(d, v);
        return *this;
    }
    LE16 &operator+=(unsigned v) {
        set_le16(d, get_le16(d) + v);
        return *this;
    }
    LE16 &operator-=(unsigned v) {
        set_le16(d, get_le16(d) - v);
        return *this;
    }
    LE16 &operator*=(unsigned v) {
        set_le16(d, get_le16(d) * v);
        return *this;
    }
    LE16 &operator/=(unsigned v) {
        set_le16(d, get_le16(d) / v);
        return *this;
    }
    LE16 &operator&=(unsigned v) {
        set_le16(d, get_le16(d) & v);
        return *this;
    }
    LE16 &operator|=(unsigned v) {
        set_le16(d, get_le16(d) | v);
        return *this;
    }
    LE16 &operator^=(unsigned v) {
        set_le16(d, get_le16(d) ^ v);
        return *this;
    }
    LE16 &operator<<=(unsigned v) {
        set_le16(d, get_le16(d) << v);
        return *this;
    }
    LE16 &operator>>=(unsigned v) {
        set_le16(d, get_le16(d) >> v);
        return *this;
    }

    operator unsigned() const { return get_le16(d); }

    bool operator<(const LE16 &v) const { return unsigned(*this) < unsigned(v); }
};

struct alignas(1) LE32 {
    unsigned char d[4];

    LE32 &operator=(unsigned v) {
        set_le32(d, v);
        return *this;
    }
    LE32 &operator+=(unsigned v) {
        set_le32(d, get_le32(d) + v);
        return *this;
    }
    LE32 &operator-=(unsigned v) {
        set_le32(d, get_le32(d) - v);
        return *this;
    }
    LE32 &operator*=(unsigned v) {
        set_le32(d, get_le32(d) * v);
        return *this;
    }
    LE32 &operator/=(unsigned v) {
        set_le32(d, get_le32(d) / v);
        return *this;
    }
    LE32 &operator&=(unsigned v) {
        set_le32(d, get_le32(d) & v);
        return *this;
    }
    LE32 &operator|=(unsigned v) {
        set_le32(d, get_le32(d) | v);
        return *this;
    }
    LE32 &operator^=(unsigned v) {
        set_le32(d, get_le32(d) ^ v);
        return *this;
    }
    LE32 &operator<<=(unsigned v) {
        set_le32(d, get_le32(d) << v);
        return *this;
    }
    LE32 &operator>>=(unsigned v) {
        set_le32(d, get_le32(d) >> v);
        return *this;
    }

    operator unsigned() const { return get_le32(d); }

    bool operator<(const LE32 &v) const { return unsigned(*this) < unsigned(v); }
};

struct alignas(1) LE64 {
    unsigned char d[8];

    LE64 &operator=(upx_uint64_t v) {
        set_le64(d, v);
        return *this;
    }
    LE64 &operator+=(upx_uint64_t v) {
        set_le64(d, get_le64(d) + v);
        return *this;
    }
    LE64 &operator-=(upx_uint64_t v) {
        set_le64(d, get_le64(d) - v);
        return *this;
    }
    LE64 &operator*=(upx_uint64_t v) {
        set_le64(d, get_le64(d) * v);
        return *this;
    }
    LE64 &operator/=(upx_uint64_t v) {
        set_le64(d, get_le64(d) / v);
        return *this;
    }
    LE64 &operator&=(upx_uint64_t v) {
        set_le64(d, get_le64(d) & v);
        return *this;
    }
    LE64 &operator|=(upx_uint64_t v) {
        set_le64(d, get_le64(d) | v);
        return *this;
    }
    LE64 &operator^=(upx_uint64_t v) {
        set_le64(d, get_le64(d) ^ v);
        return *this;
    }
    LE64 &operator<<=(unsigned v) {
        set_le64(d, get_le64(d) << v);
        return *this;
    }
    LE64 &operator>>=(unsigned v) {
        set_le64(d, get_le64(d) >> v);
        return *this;
    }

    operator upx_uint64_t() const { return get_le64(d); }

    bool operator<(const LE64 &v) const { return upx_uint64_t(*this) < upx_uint64_t(v); }
};

// native types
#if (ACC_ABI_BIG_ENDIAN)
typedef BE16 NE16;
typedef BE32 NE32;
typedef BE64 NE64;
#else
typedef LE16 NE16;
typedef LE32 NE32;
typedef LE64 NE64;
#endif

/*************************************************************************
// global operators (pointer addition/subtraction)
**************************************************************************/

template <class T>
inline T *operator+(T *ptr, const BE16 &v) {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const BE16 &v) {
    return ptr - unsigned(v);
}

template <class T>
inline T *operator+(T *ptr, const BE32 &v) {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const BE32 &v) {
    return ptr - unsigned(v);
}

// these are not implemented on purpose and will cause link-time errors
template <class T>
T *operator+(T *ptr, const BE64 &v);
template <class T>
T *operator-(T *ptr, const BE64 &v);

template <class T>
inline T *operator+(T *ptr, const LE16 &v) {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const LE16 &v) {
    return ptr - unsigned(v);
}

template <class T>
inline T *operator+(T *ptr, const LE32 &v) {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const LE32 &v) {
    return ptr - unsigned(v);
}

// these are not implemented on purpose and will cause link-time errors
template <class T>
T *operator+(T *ptr, const LE64 &v);
template <class T>
T *operator-(T *ptr, const LE64 &v);

/*************************************************************************
// global overloads
**************************************************************************/

inline unsigned ALIGN_DOWN(unsigned a, const BE32 &b) { return ALIGN_DOWN(a, unsigned(b)); }
inline unsigned ALIGN_DOWN(const BE32 &a, unsigned b) { return ALIGN_DOWN(unsigned(a), b); }
inline unsigned ALIGN_UP(unsigned a, const BE32 &b) { return ALIGN_UP(a, unsigned(b)); }
inline unsigned ALIGN_UP(const BE32 &a, unsigned b) { return ALIGN_UP(unsigned(a), b); }

inline unsigned ALIGN_DOWN(unsigned a, const LE32 &b) { return ALIGN_DOWN(a, unsigned(b)); }
inline unsigned ALIGN_DOWN(const LE32 &a, unsigned b) { return ALIGN_DOWN(unsigned(a), b); }
inline unsigned ALIGN_UP(unsigned a, const LE32 &b) { return ALIGN_UP(a, unsigned(b)); }
inline unsigned ALIGN_UP(const LE32 &a, unsigned b) { return ALIGN_UP(unsigned(a), b); }

inline unsigned UPX_MAX(unsigned a, const BE16 &b) { return UPX_MAX(a, unsigned(b)); }
inline unsigned UPX_MAX(const BE16 &a, unsigned b) { return UPX_MAX(unsigned(a), b); }
inline unsigned UPX_MIN(unsigned a, const BE16 &b) { return UPX_MIN(a, unsigned(b)); }
inline unsigned UPX_MIN(const BE16 &a, unsigned b) { return UPX_MIN(unsigned(a), b); }

inline unsigned UPX_MAX(unsigned a, const BE32 &b) { return UPX_MAX(a, unsigned(b)); }
inline unsigned UPX_MAX(const BE32 &a, unsigned b) { return UPX_MAX(unsigned(a), b); }
inline unsigned UPX_MIN(unsigned a, const BE32 &b) { return UPX_MIN(a, unsigned(b)); }
inline unsigned UPX_MIN(const BE32 &a, unsigned b) { return UPX_MIN(unsigned(a), b); }

inline unsigned UPX_MAX(unsigned a, const LE16 &b) { return UPX_MAX(a, unsigned(b)); }
inline unsigned UPX_MAX(const LE16 &a, unsigned b) { return UPX_MAX(unsigned(a), b); }
inline unsigned UPX_MIN(unsigned a, const LE16 &b) { return UPX_MIN(a, unsigned(b)); }
inline unsigned UPX_MIN(const LE16 &a, unsigned b) { return UPX_MIN(unsigned(a), b); }

inline unsigned UPX_MAX(unsigned a, const LE32 &b) { return UPX_MAX(a, unsigned(b)); }
inline unsigned UPX_MAX(const LE32 &a, unsigned b) { return UPX_MAX(unsigned(a), b); }
inline unsigned UPX_MIN(unsigned a, const LE32 &b) { return UPX_MIN(a, unsigned(b)); }
inline unsigned UPX_MIN(const LE32 &a, unsigned b) { return UPX_MIN(unsigned(a), b); }

/*************************************************************************
// misc support
**************************************************************************/

// for use with qsort()
extern "C" {
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
} // extern "C"

/*************************************************************************
// Provide namespaces and classes to abstract endianness policies.
//
// CTP - Compile-Time Polymorphism (templates)
// RTP - Run-Time Polymorphism (virtual functions)
**************************************************************************/

// forward declarations
namespace N_BELE_CTP {
struct BEPolicy;
struct LEPolicy;
extern const BEPolicy be_policy;
extern const LEPolicy le_policy;
} // namespace N_BELE_CTP
namespace N_BELE_RTP {
struct AbstractPolicy;
struct BEPolicy;
struct LEPolicy;
extern const BEPolicy be_policy;
extern const LEPolicy le_policy;
} // namespace N_BELE_RTP

// implementation
namespace N_BELE_CTP {
#define BELE_CTP 1
#include "bele_policy.h"
#undef BELE_CTP
} // namespace N_BELE_CTP
namespace N_BELE_RTP {
#define BELE_RTP 1
#include "bele_policy.h"
#undef BELE_RTP
} // namespace N_BELE_RTP

// util
namespace N_BELE_CTP {
inline const N_BELE_RTP::AbstractPolicy *getRTP(const BEPolicy * /*dummy*/) {
    return &N_BELE_RTP::be_policy;
}
inline const N_BELE_RTP::AbstractPolicy *getRTP(const LEPolicy * /*dummy*/) {
    return &N_BELE_RTP::le_policy;
}
} // namespace N_BELE_CTP

#endif /* already included */

/* vim:set ts=4 sw=4 et: */

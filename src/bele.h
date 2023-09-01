/* bele.h -- access memory in BigEndian and LittleEndian byte order

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

// BE - Big Endian
// LE - Little Endian
// NE - Native Endianness (aka Host Endianness aka CPU Endianness)
// TE - Target Endianness (not used here, see various packers)

#if 1
// some platforms may provide their own system bswapXX() functions, so rename to avoid conflicts
#undef bswap16
#undef bswap32
#undef bswap64
#define bswap16 upx_bswap16
#define bswap32 upx_bswap32
#define bswap64 upx_bswap64
#endif

/*************************************************************************
// XE - eXtended Endian compatibility
// try to detect XX16 vs XX32 vs XX64 size mismatches
**************************************************************************/

#if 0 // permissive version using "void *"

#define REQUIRE_XE16 /*empty*/
#define REQUIRE_XE24 /*empty*/
#define REQUIRE_XE32 /*empty*/
#define REQUIRE_XE64 /*empty*/
typedef void XE16;
typedef void XE24;
typedef void XE32;
typedef void XE64;

#else // permissive version

// forward declarations
struct BE16;
struct BE32;
struct BE64;
struct LE16;
struct LE32;
struct LE64;

// Note:
//   void is explicitly allowed (but there is no automatic pointer conversion because of template!)
//   char is explicitly allowed
//   byte is explicitly allowed
template <class T>
static inline constexpr bool is_xe16_type =
    upx::is_same_any_v<T, void, char, byte, upx_int16_t, upx_uint16_t, BE16, LE16>;
template <class T>
static inline constexpr bool is_xe24_type = upx::is_same_any_v<T, void, char, byte>;
template <class T>
static inline constexpr bool is_xe32_type =
    upx::is_same_any_v<T, void, char, byte, upx_int32_t, upx_uint32_t, BE32, LE32>;
template <class T>
static inline constexpr bool is_xe64_type =
    upx::is_same_any_v<T, void, char, byte, upx_int64_t, upx_uint64_t, BE64, LE64>;

template <class T>
using enable_if_xe16 = std::enable_if_t<is_xe16_type<T>, T>;
template <class T>
using enable_if_xe24 = std::enable_if_t<is_xe24_type<T>, T>;
template <class T>
using enable_if_xe32 = std::enable_if_t<is_xe32_type<T>, T>;
template <class T>
using enable_if_xe64 = std::enable_if_t<is_xe64_type<T>, T>;

#define REQUIRE_XE16 template <class XE16, class = enable_if_xe16<XE16> >
#define REQUIRE_XE24 template <class XE24, class = enable_if_xe24<XE24> >
#define REQUIRE_XE32 template <class XE32, class = enable_if_xe32<XE32> >
#define REQUIRE_XE64 template <class XE64, class = enable_if_xe64<XE64> >

#endif // permissive version

/*************************************************************************
// core - NE
**************************************************************************/

REQUIRE_XE16
static forceinline unsigned get_ne16(const XE16 *p) noexcept {
    upx_uint16_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}

REQUIRE_XE32
static forceinline unsigned get_ne32(const XE32 *p) noexcept {
    upx_uint32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}

REQUIRE_XE64
static forceinline upx_uint64_t get_ne64(const XE64 *p) noexcept {
    upx_uint64_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}

REQUIRE_XE16
static forceinline void set_ne16(XE16 *p, unsigned vv) noexcept {
    upx_uint16_t v = (upx_uint16_t) (vv & 0xffff);
    upx_memcpy_inline(p, &v, sizeof(v));
}

REQUIRE_XE32
static forceinline void set_ne32(XE32 *p, unsigned vv) noexcept {
    upx_uint32_t v = vv;
    upx_memcpy_inline(p, &v, sizeof(v));
}

REQUIRE_XE64
static forceinline void set_ne64(XE64 *p, upx_uint64_t vv) noexcept {
    upx_uint64_t v = vv;
    upx_memcpy_inline(p, &v, sizeof(v));
}

/*************************************************************************
// core - bswap
**************************************************************************/

#if (ACC_CC_MSC)

ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(long) == 4)

// unfortunately *not* constexpr with current MSVC
static forceinline unsigned bswap16(unsigned v) noexcept {
    return (unsigned) _byteswap_ulong(v << 16);
}
static forceinline unsigned bswap32(unsigned v) noexcept { return (unsigned) _byteswap_ulong(v); }
static forceinline upx_uint64_t bswap64(upx_uint64_t v) noexcept { return _byteswap_uint64(v); }

#else

static forceinline constexpr unsigned bswap16(unsigned v) noexcept {
#if defined(__riscv) && __riscv_xlen == 64
    return (unsigned) __builtin_bswap64((upx_uint64_t) v << 48);
#else
    // return __builtin_bswap16((upx_uint16_t) (v & 0xffff));
    return __builtin_bswap32(v << 16);
#endif
}
static forceinline constexpr unsigned bswap32(unsigned v) noexcept {
#if defined(__riscv) && __riscv_xlen == 64
    return (unsigned) __builtin_bswap64((upx_uint64_t) v << 32);
#else
    return __builtin_bswap32(v);
#endif
}
static forceinline constexpr upx_uint64_t bswap64(upx_uint64_t v) noexcept {
    return __builtin_bswap64(v);
}

#endif

static forceinline constexpr unsigned no_bswap16(unsigned v) noexcept {
    // mask is needed so that for all v: bswap16(bswap16(v)) == no_bswap16(v)
    return v & 0xffff;
}
static forceinline constexpr unsigned no_bswap32(unsigned v) noexcept { return v; }
static forceinline constexpr upx_uint64_t no_bswap64(upx_uint64_t v) noexcept { return v; }

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

REQUIRE_XE16
inline unsigned get_be16(const XE16 *p) noexcept { return ne16_to_be16(get_ne16(p)); }
REQUIRE_XE32
inline unsigned get_be32(const XE32 *p) noexcept { return ne32_to_be32(get_ne32(p)); }
REQUIRE_XE64
inline upx_uint64_t get_be64(const XE64 *p) noexcept { return ne64_to_be64(get_ne64(p)); }
REQUIRE_XE16
inline unsigned get_le16(const XE16 *p) noexcept { return ne16_to_le16(get_ne16(p)); }
REQUIRE_XE32
inline unsigned get_le32(const XE32 *p) noexcept { return ne32_to_le32(get_ne32(p)); }
REQUIRE_XE64
inline upx_uint64_t get_le64(const XE64 *p) noexcept { return ne64_to_le64(get_ne64(p)); }

REQUIRE_XE16
inline void set_be16(XE16 *p, unsigned v) noexcept { set_ne16(p, ne16_to_be16(v)); }
REQUIRE_XE32
inline void set_be32(XE32 *p, unsigned v) noexcept { set_ne32(p, ne32_to_be32(v)); }
REQUIRE_XE64
inline void set_be64(XE64 *p, upx_uint64_t v) noexcept { set_ne64(p, ne64_to_be64(v)); }
REQUIRE_XE16
inline void set_le16(XE16 *p, unsigned v) noexcept { set_ne16(p, ne16_to_le16(v)); }
REQUIRE_XE32
inline void set_le32(XE32 *p, unsigned v) noexcept { set_ne32(p, ne32_to_le32(v)); }
REQUIRE_XE64
inline void set_le64(XE64 *p, upx_uint64_t v) noexcept { set_ne64(p, ne64_to_le64(v)); }

/*************************************************************************
// get/set 24/26
**************************************************************************/

REQUIRE_XE24
inline unsigned get_be24(const XE24 *p) noexcept {
    const byte *b = ACC_CCAST(const byte *, p);
    return (b[0] << 16) | (b[1] << 8) | (b[2] << 0);
}

REQUIRE_XE24
inline unsigned get_le24(const XE24 *p) noexcept {
    const byte *b = ACC_CCAST(const byte *, p);
    return (b[0] << 0) | (b[1] << 8) | (b[2] << 16);
}

REQUIRE_XE24
inline void set_be24(XE24 *p, unsigned v) noexcept {
    byte *b = ACC_PCAST(byte *, p);
    b[0] = ACC_ICONV(byte, (v >> 16) & 0xff);
    b[1] = ACC_ICONV(byte, (v >> 8) & 0xff);
    b[2] = ACC_ICONV(byte, (v >> 0) & 0xff);
}

REQUIRE_XE24
inline void set_le24(XE24 *p, unsigned v) noexcept {
    byte *b = ACC_PCAST(byte *, p);
    b[0] = ACC_ICONV(byte, (v >> 0) & 0xff);
    b[1] = ACC_ICONV(byte, (v >> 8) & 0xff);
    b[2] = ACC_ICONV(byte, (v >> 16) & 0xff);
}

inline unsigned get_le26(const void *p) noexcept { return get_le32(p) & 0x03ffffff; }
inline unsigned get_le19_5(const void *p) noexcept { return (get_le32(p) >> 5) & 0x0007ffff; }
inline unsigned get_le14_5(const void *p) noexcept { return (get_le32(p) >> 5) & 0x00003fff; }

inline void set_le26(void *p, unsigned v) noexcept {
    // preserve the top 6 bits
    //   set_le32(p, (get_le32(p) & 0xfc000000) | (v & 0x03ffffff));
    // optimized version, saving a runtime bswap32
    set_ne32(p, (get_ne32(p) & ne32_to_le32(0xfc000000)) |
                    (ne32_to_le32(v) & ne32_to_le32(0x03ffffff)));
}
inline void set_le19_5(void *p, unsigned v) noexcept {
    set_le32(p, (get_le32(p) & 0xff00001f) | ((v & 0x0007ffff) << 5));
}
inline void set_le14_5(void *p, unsigned v) noexcept {
    set_le32(p, (get_le32(p) & 0xfff8001f) | ((v & 0x00003fff) << 5));
}

/*************************************************************************
// get signed values
**************************************************************************/

static forceinline constexpr int sign_extend(unsigned v, unsigned bits) noexcept {
    const unsigned sign_bit = 1u << (bits - 1);
    v &= sign_bit | (sign_bit - 1);
    v |= 0u - (v & sign_bit);
    return ACC_ICAST(int, v);
}

static forceinline constexpr upx_int64_t sign_extend(upx_uint64_t v, unsigned bits) noexcept {
    const upx_uint64_t sign_bit = 1ull << (bits - 1);
    v &= sign_bit | (sign_bit - 1);
    v |= 0ull - (v & sign_bit);
    return ACC_ICAST(upx_int64_t, v);
}

REQUIRE_XE16
inline int get_be16_signed(const XE16 *p) noexcept {
    unsigned v = get_be16(p);
    return sign_extend(v, 16);
}

REQUIRE_XE24
inline int get_be24_signed(const XE24 *p) noexcept {
    unsigned v = get_be24(p);
    return sign_extend(v, 24);
}

REQUIRE_XE32
inline int get_be32_signed(const XE32 *p) noexcept {
    unsigned v = get_be32(p);
    return sign_extend(v, 32);
}

REQUIRE_XE64
inline upx_int64_t get_be64_signed(const XE64 *p) noexcept {
    upx_uint64_t v = get_be64(p);
    return sign_extend(v, 64);
}

REQUIRE_XE16
inline int get_le16_signed(const XE16 *p) noexcept {
    unsigned v = get_le16(p);
    return sign_extend(v, 16);
}

REQUIRE_XE24
inline int get_le24_signed(const XE24 *p) noexcept {
    unsigned v = get_le24(p);
    return sign_extend(v, 24);
}

REQUIRE_XE32
inline int get_le32_signed(const XE32 *p) noexcept {
    unsigned v = get_le32(p);
    return sign_extend(v, 32);
}

REQUIRE_XE64
inline upx_int64_t get_le64_signed(const XE64 *p) noexcept {
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

struct alignas(1) BE16 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[2];

    forceinline operator unsigned() const noexcept { return get_be16(d); }

    forceinline BE16 &operator=(unsigned v) noexcept {
        set_be16(d, v);
        return *this;
    }
    BE16 &operator+=(unsigned v) noexcept {
        set_be16(d, get_be16(d) + v);
        return *this;
    }
    BE16 &operator-=(unsigned v) noexcept {
        set_be16(d, get_be16(d) - v);
        return *this;
    }
    BE16 &operator*=(unsigned v) noexcept {
        set_be16(d, get_be16(d) * v);
        return *this;
    }
    BE16 &operator/=(unsigned v) noexcept {
        set_be16(d, get_be16(d) / v);
        return *this;
    }
    BE16 &operator&=(unsigned v) noexcept {
        set_be16(d, get_be16(d) & v);
        return *this;
    }
    BE16 &operator|=(unsigned v) noexcept {
        set_be16(d, get_be16(d) | v);
        return *this;
    }
    BE16 &operator^=(unsigned v) noexcept {
        set_be16(d, get_be16(d) ^ v);
        return *this;
    }
    BE16 &operator<<=(unsigned v) noexcept {
        set_be16(d, get_be16(d) << v);
        return *this;
    }
    BE16 &operator>>=(unsigned v) noexcept {
        set_be16(d, get_be16(d) >> v);
        return *this;
    }

    bool operator==(const BE16 &x) const noexcept { return memcmp(d, x.d, sizeof(d)) == 0; }
    bool operator<(const BE16 &x) const noexcept { return unsigned(*this) < unsigned(x); }
};

struct alignas(1) BE32 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[4];

    forceinline operator unsigned() const noexcept { return get_be32(d); }

    forceinline BE32 &operator=(unsigned v) noexcept {
        set_be32(d, v);
        return *this;
    }
    BE32 &operator+=(unsigned v) noexcept {
        set_be32(d, get_be32(d) + v);
        return *this;
    }
    BE32 &operator-=(unsigned v) noexcept {
        set_be32(d, get_be32(d) - v);
        return *this;
    }
    BE32 &operator*=(unsigned v) noexcept {
        set_be32(d, get_be32(d) * v);
        return *this;
    }
    BE32 &operator/=(unsigned v) noexcept {
        set_be32(d, get_be32(d) / v);
        return *this;
    }
    BE32 &operator&=(unsigned v) noexcept {
        set_be32(d, get_be32(d) & v);
        return *this;
    }
    BE32 &operator|=(unsigned v) noexcept {
        set_be32(d, get_be32(d) | v);
        return *this;
    }
    BE32 &operator^=(unsigned v) noexcept {
        set_be32(d, get_be32(d) ^ v);
        return *this;
    }
    BE32 &operator<<=(unsigned v) noexcept {
        set_be32(d, get_be32(d) << v);
        return *this;
    }
    BE32 &operator>>=(unsigned v) noexcept {
        set_be32(d, get_be32(d) >> v);
        return *this;
    }

    bool operator==(const BE32 &x) const noexcept { return memcmp(d, x.d, sizeof(d)) == 0; }
    bool operator<(const BE32 &x) const noexcept { return unsigned(*this) < unsigned(x); }
};

struct alignas(1) BE64 final {
    typedef upx_uint64_t integral_conversion_type; // automatic conversion to upx_uint64_t
    byte d[8];

    forceinline operator upx_uint64_t() const noexcept { return get_be64(d); }

    forceinline BE64 &operator=(upx_uint64_t v) noexcept {
        set_be64(d, v);
        return *this;
    }
    BE64 &operator+=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) + v);
        return *this;
    }
    BE64 &operator-=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) - v);
        return *this;
    }
    BE64 &operator*=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) * v);
        return *this;
    }
    BE64 &operator/=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) / v);
        return *this;
    }
    BE64 &operator&=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) & v);
        return *this;
    }
    BE64 &operator|=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) | v);
        return *this;
    }
    BE64 &operator^=(upx_uint64_t v) noexcept {
        set_be64(d, get_be64(d) ^ v);
        return *this;
    }
    BE64 &operator<<=(unsigned v) noexcept {
        set_be64(d, get_be64(d) << v);
        return *this;
    }
    BE64 &operator>>=(unsigned v) noexcept {
        set_be64(d, get_be64(d) >> v);
        return *this;
    }

    bool operator==(const BE64 &x) const noexcept { return memcmp(d, x.d, sizeof(d)) == 0; }
    bool operator<(const BE64 &x) const noexcept { return upx_uint64_t(*this) < upx_uint64_t(x); }
};

struct alignas(1) LE16 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[2];

    forceinline operator unsigned() const noexcept { return get_le16(d); }

    forceinline LE16 &operator=(unsigned v) noexcept {
        set_le16(d, v);
        return *this;
    }
    LE16 &operator+=(unsigned v) noexcept {
        set_le16(d, get_le16(d) + v);
        return *this;
    }
    LE16 &operator-=(unsigned v) noexcept {
        set_le16(d, get_le16(d) - v);
        return *this;
    }
    LE16 &operator*=(unsigned v) noexcept {
        set_le16(d, get_le16(d) * v);
        return *this;
    }
    LE16 &operator/=(unsigned v) noexcept {
        set_le16(d, get_le16(d) / v);
        return *this;
    }
    LE16 &operator&=(unsigned v) noexcept {
        set_le16(d, get_le16(d) & v);
        return *this;
    }
    LE16 &operator|=(unsigned v) noexcept {
        set_le16(d, get_le16(d) | v);
        return *this;
    }
    LE16 &operator^=(unsigned v) noexcept {
        set_le16(d, get_le16(d) ^ v);
        return *this;
    }
    LE16 &operator<<=(unsigned v) noexcept {
        set_le16(d, get_le16(d) << v);
        return *this;
    }
    LE16 &operator>>=(unsigned v) noexcept {
        set_le16(d, get_le16(d) >> v);
        return *this;
    }

    bool operator==(const LE16 &x) const noexcept { return memcmp(d, x.d, sizeof(d)) == 0; }
    bool operator<(const LE16 &x) const noexcept { return unsigned(*this) < unsigned(x); }
};

struct alignas(1) LE32 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[4];

    forceinline operator unsigned() const noexcept { return get_le32(d); }

    forceinline LE32 &operator=(unsigned v) noexcept {
        set_le32(d, v);
        return *this;
    }
    LE32 &operator+=(unsigned v) noexcept {
        set_le32(d, get_le32(d) + v);
        return *this;
    }
    LE32 &operator-=(unsigned v) noexcept {
        set_le32(d, get_le32(d) - v);
        return *this;
    }
    LE32 &operator*=(unsigned v) noexcept {
        set_le32(d, get_le32(d) * v);
        return *this;
    }
    LE32 &operator/=(unsigned v) noexcept {
        set_le32(d, get_le32(d) / v);
        return *this;
    }
    LE32 &operator&=(unsigned v) noexcept {
        set_le32(d, get_le32(d) & v);
        return *this;
    }
    LE32 &operator|=(unsigned v) noexcept {
        set_le32(d, get_le32(d) | v);
        return *this;
    }
    LE32 &operator^=(unsigned v) noexcept {
        set_le32(d, get_le32(d) ^ v);
        return *this;
    }
    LE32 &operator<<=(unsigned v) noexcept {
        set_le32(d, get_le32(d) << v);
        return *this;
    }
    LE32 &operator>>=(unsigned v) noexcept {
        set_le32(d, get_le32(d) >> v);
        return *this;
    }

    bool operator==(const LE32 &x) const noexcept { return memcmp(d, x.d, sizeof(d)) == 0; }
    bool operator<(const LE32 &x) const noexcept { return unsigned(*this) < unsigned(x); }
};

struct alignas(1) LE64 final {
    typedef upx_uint64_t integral_conversion_type; // automatic conversion to upx_uint64_t
    byte d[8];

    forceinline operator upx_uint64_t() const noexcept { return get_le64(d); }

    forceinline LE64 &operator=(upx_uint64_t v) noexcept {
        set_le64(d, v);
        return *this;
    }
    LE64 &operator+=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) + v);
        return *this;
    }
    LE64 &operator-=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) - v);
        return *this;
    }
    LE64 &operator*=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) * v);
        return *this;
    }
    LE64 &operator/=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) / v);
        return *this;
    }
    LE64 &operator&=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) & v);
        return *this;
    }
    LE64 &operator|=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) | v);
        return *this;
    }
    LE64 &operator^=(upx_uint64_t v) noexcept {
        set_le64(d, get_le64(d) ^ v);
        return *this;
    }
    LE64 &operator<<=(unsigned v) noexcept {
        set_le64(d, get_le64(d) << v);
        return *this;
    }
    LE64 &operator>>=(unsigned v) noexcept {
        set_le64(d, get_le64(d) >> v);
        return *this;
    }

    bool operator==(const LE64 &x) const noexcept { return memcmp(d, x.d, sizeof(d)) == 0; }
    bool operator<(const LE64 &x) const noexcept { return upx_uint64_t(*this) < upx_uint64_t(x); }
};

/*************************************************************************
// global operators (pointer addition/subtraction)
**************************************************************************/

template <class T>
inline T *operator+(T *ptr, const BE16 &v) noexcept {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const BE16 &v) noexcept {
    return ptr - unsigned(v);
}
template <class T>
inline T *operator+(T *ptr, const BE32 &v) noexcept {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const BE32 &v) noexcept {
    return ptr - unsigned(v);
}
template <class T>
inline T *operator+(T *ptr, const LE16 &v) noexcept {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const LE16 &v) noexcept {
    return ptr - unsigned(v);
}
template <class T>
inline T *operator+(T *ptr, const LE32 &v) noexcept {
    return ptr + unsigned(v);
}
template <class T>
inline T *operator-(T *ptr, const LE32 &v) noexcept {
    return ptr - unsigned(v);
}

// these are not implemented on purpose and will cause errors
template <class T>
T *operator+(T *ptr, const BE64 &v) noexcept DELETED_FUNCTION;
template <class T>
T *operator-(T *ptr, const BE64 &v) noexcept DELETED_FUNCTION;
template <class T>
T *operator+(T *ptr, const LE64 &v) noexcept DELETED_FUNCTION;
template <class T>
T *operator-(T *ptr, const LE64 &v) noexcept DELETED_FUNCTION;

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

// TODO: introduce upx::umax() and upx::umin()
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

// native types
#if (ACC_ABI_BIG_ENDIAN)
typedef BE16 NE16;
typedef BE32 NE32;
typedef BE64 NE64;
#define ne16_compare        be16_compare
#define ne32_compare        be32_compare
#define ne64_compare        be64_compare
#define ne16_compare_signed be16_compare_signed
#define ne32_compare_signed be32_compare_signed
#define ne64_compare_signed be64_compare_signed
#else
typedef LE16 NE16;
typedef LE32 NE32;
typedef LE64 NE64;
#define ne16_compare        le16_compare
#define ne32_compare        le32_compare
#define ne64_compare        le64_compare
#define ne16_compare_signed le16_compare_signed
#define ne32_compare_signed le32_compare_signed
#define ne64_compare_signed le64_compare_signed
#endif

// <type_traits> upx_is_integral
#define TT_IS_INTEGRAL(T)                                                                          \
    template <>                                                                                    \
    struct upx_is_integral<T> : public std::true_type {};                                          \
    template <>                                                                                    \
    struct upx_is_integral<const T> : public std::true_type {};                                    \
    template <>                                                                                    \
    struct upx_is_integral<volatile T> : public std::true_type {};                                 \
    template <>                                                                                    \
    struct upx_is_integral<const volatile T> : public std::true_type {}
TT_IS_INTEGRAL(BE16);
TT_IS_INTEGRAL(BE32);
TT_IS_INTEGRAL(BE64);
TT_IS_INTEGRAL(LE16);
TT_IS_INTEGRAL(LE32);
TT_IS_INTEGRAL(LE64);
#undef TT_IS_INTEGRAL

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
inline const N_BELE_RTP::AbstractPolicy *getRTP(const BEPolicy * /*dummy*/) noexcept {
    return &N_BELE_RTP::be_policy;
}
inline const N_BELE_RTP::AbstractPolicy *getRTP(const LEPolicy * /*dummy*/) noexcept {
    return &N_BELE_RTP::le_policy;
}
} // namespace N_BELE_CTP

/* vim:set ts=4 sw=4 et: */

/* bele.h -- access memory in BigEndian and LittleEndian byte order

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

// BE - Big Endian
// LE - Little Endian
// NE - Native Endianness (aka Host Endianness aka CPU Endianness)
// TE - Target Endianness (not used here, see various packers)

static_assert(std::is_same_v<byte, unsigned char>);
static_assert(std::is_same_v<upx_int32_t, int>);
static_assert(std::is_same_v<upx_uint32_t, unsigned>);

#if defined(upx_is_constant_evaluated)
#define bele_constexpr constexpr
#else
#define bele_constexpr /*empty*/
#endif

// forward declarations
struct BE16;
struct BE32;
struct BE64;
struct LE16;
struct LE32;
struct LE64;

/*************************************************************************
// XE - eXtended Endian compatibility
// try to detect XX16 vs XX32 vs XX64 size mismatches
**************************************************************************/

#if !defined(upx_is_constant_evaluated) && !(DEBUG)

// permissive version using "void *"
#define REQUIRE_XE16 /*empty*/
#define REQUIRE_XE24 /*empty*/
#define REQUIRE_XE32 /*empty*/
#define REQUIRE_XE64 /*empty*/
typedef void XE16;
typedef void XE24;
typedef void XE32;
typedef void XE64;

#else // permissive version

namespace bele_detail {

// Note:
//   void is explicitly allowed (but there is no automatic pointer conversion because of template!)
//   char is explicitly allowed
//   byte is explicitly allowed
template <class T>
inline constexpr bool is_xe16_type =
    upx::is_same_any_v<T, void, char, byte, upx_int16_t, upx_uint16_t, BE16, LE16>;
template <class T>
inline constexpr bool is_xe24_type = upx::is_same_any_v<T, void, char, byte>;
template <class T>
inline constexpr bool is_xe32_type =
    upx::is_same_any_v<T, void, char, byte, upx_int32_t, upx_uint32_t, BE32, LE32>;
template <class T>
inline constexpr bool is_xe64_type =
    upx::is_same_any_v<T, void, char, byte, upx_int64_t, upx_uint64_t, BE64, LE64>;

template <class T>
using enable_if_xe16 = std::enable_if_t<is_xe16_type<T>, T>;
template <class T>
using enable_if_xe24 = std::enable_if_t<is_xe24_type<T>, T>;
template <class T>
using enable_if_xe32 = std::enable_if_t<is_xe32_type<T>, T>;
template <class T>
using enable_if_xe64 = std::enable_if_t<is_xe64_type<T>, T>;

} // namespace bele_detail

#define REQUIRE_XE16 template <class XE16, class = bele_detail::enable_if_xe16<XE16> >
#define REQUIRE_XE24 template <class XE24, class = bele_detail::enable_if_xe24<XE24> >
#define REQUIRE_XE32 template <class XE32, class = bele_detail::enable_if_xe32<XE32> >
#define REQUIRE_XE64 template <class XE64, class = bele_detail::enable_if_xe64<XE64> >

#endif // permissive version

/*************************************************************************
// core - NE
**************************************************************************/

forceinline bele_constexpr unsigned get_ne16(const byte *p) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        return upx::compile_time::get_ne16(p);
    else
#endif
    {
        upx_uint16_t v = 0;
        upx_memcpy_inline(&v, p, sizeof(v));
        return v;
    }
}

forceinline bele_constexpr unsigned get_ne32(const byte *p) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        return upx::compile_time::get_ne32(p);
    else
#endif
    {
        upx_uint32_t v = 0;
        upx_memcpy_inline(&v, p, sizeof(v));
        return v;
    }
}

forceinline bele_constexpr upx_uint64_t get_ne64(const byte *p) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        return upx::compile_time::get_ne64(p);
    else
#endif
    {
        upx_uint64_t v = 0;
        upx_memcpy_inline(&v, p, sizeof(v));
        return v;
    }
}

forceinline bele_constexpr void set_ne16(byte *p, unsigned v) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        upx::compile_time::set_ne16(p, upx_uint16_t(v & 0xffff));
    else
#endif
    {
        upx_uint16_t vv = upx_uint16_t(v & 0xffff);
        upx_memcpy_inline(p, &vv, sizeof(vv));
    }
}

forceinline bele_constexpr void set_ne32(byte *p, unsigned v) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        upx::compile_time::set_ne32(p, v);
    else
#endif
    {
        upx_uint32_t vv = v;
        upx_memcpy_inline(p, &vv, sizeof(vv));
    }
}

forceinline bele_constexpr void set_ne64(byte *p, upx_uint64_t v) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        upx::compile_time::set_ne64(p, v);
    else
#endif
    {
        upx_uint64_t vv = v;
        upx_memcpy_inline(p, &vv, sizeof(vv));
    }
}

/*************************************************************************
// core - NE
**************************************************************************/

REQUIRE_XE16
forceinline bele_constexpr unsigned get_ne16(const XE16 *p) noexcept {
    return get_ne16(upx::ptr_static_cast<const byte *>(p));
}

REQUIRE_XE32
forceinline bele_constexpr unsigned get_ne32(const XE32 *p) noexcept {
    return get_ne32(upx::ptr_static_cast<const byte *>(p));
}

REQUIRE_XE64
forceinline bele_constexpr upx_uint64_t get_ne64(const XE64 *p) noexcept {
    return get_ne64(upx::ptr_static_cast<const byte *>(p));
}

REQUIRE_XE16
forceinline bele_constexpr void set_ne16(XE16 *p, unsigned v) noexcept {
    set_ne16(upx::ptr_static_cast<byte *>(p), v);
}

REQUIRE_XE32
forceinline bele_constexpr void set_ne32(XE32 *p, unsigned v) noexcept {
    set_ne32(upx::ptr_static_cast<byte *>(p), v);
}

REQUIRE_XE64
forceinline bele_constexpr void set_ne64(XE64 *p, upx_uint64_t v) noexcept {
    set_ne64(upx::ptr_static_cast<byte *>(p), v);
}

/*************************************************************************
// core - bswap
**************************************************************************/

#if __cpp_lib_byteswap >= 202110L

forceinline constexpr unsigned bswap16(unsigned v) noexcept {
    return std::byteswap(upx_uint16_t(v & 0xffff));
}
forceinline constexpr unsigned bswap32(unsigned v) noexcept { return std::byteswap(v); }
forceinline constexpr upx_uint64_t bswap64(upx_uint64_t v) noexcept { return std::byteswap(v); }

#elif (ACC_CC_MSC)

static_assert(sizeof(long) == 4);

// _byteswap_XXX is unfortunately *not* constexpr with current MSVC
forceinline bele_constexpr unsigned bswap16(unsigned v) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        return upx::compile_time::bswap16(upx_uint16_t(v & 0xffff));
    else
#endif
        return (unsigned) _byteswap_ulong(v << 16);
}
forceinline bele_constexpr unsigned bswap32(unsigned v) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        return upx::compile_time::bswap32(v);
    else
#endif
        return (unsigned) _byteswap_ulong(v);
}
forceinline bele_constexpr upx_uint64_t bswap64(upx_uint64_t v) noexcept {
#if defined(upx_is_constant_evaluated)
    if (upx_is_constant_evaluated())
        return upx::compile_time::bswap64(v);
    else
#endif
        return _byteswap_uint64(v);
}

#else

forceinline constexpr unsigned bswap16(unsigned v) noexcept {
    // return __builtin_bswap32(v << 16);
    return __builtin_bswap16(upx_uint16_t(v & 0xffff));
}
forceinline constexpr unsigned bswap32(unsigned v) noexcept { return __builtin_bswap32(v); }
forceinline constexpr upx_uint64_t bswap64(upx_uint64_t v) noexcept { return __builtin_bswap64(v); }

#endif

forceinline constexpr unsigned no_bswap16(unsigned v) noexcept {
    // mask is needed so that for all v: bswap16(bswap16(v)) == no_bswap16(v)
    return v & 0xffff;
}
forceinline constexpr unsigned no_bswap32(unsigned v) noexcept { return v; }
forceinline constexpr upx_uint64_t no_bswap64(upx_uint64_t v) noexcept { return v; }

#if (ACC_ABI_BIG_ENDIAN)
#define ne16_to_be16(v) no_bswap16(v)
#define ne32_to_be32(v) no_bswap32(v)
#define ne64_to_be64(v) no_bswap64(v)
#define ne16_to_le16(v) bswap16(v)
#define ne32_to_le32(v) bswap32(v)
#define ne64_to_le64(v) bswap64(v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#define ne16_to_be16(v) bswap16(v)
#define ne32_to_be32(v) bswap32(v)
#define ne64_to_be64(v) bswap64(v)
#define ne16_to_le16(v) no_bswap16(v)
#define ne32_to_le32(v) no_bswap32(v)
#define ne64_to_le64(v) no_bswap64(v)
#else
#error "ACC_ABI_ENDIAN"
#endif

/*************************************************************************
// get/set 16/32/64
**************************************************************************/

REQUIRE_XE16
inline bele_constexpr unsigned get_be16(const XE16 *p) noexcept {
    return ne16_to_be16(get_ne16(p));
}
REQUIRE_XE32
inline bele_constexpr unsigned get_be32(const XE32 *p) noexcept {
    return ne32_to_be32(get_ne32(p));
}
REQUIRE_XE64
inline bele_constexpr upx_uint64_t get_be64(const XE64 *p) noexcept {
    return ne64_to_be64(get_ne64(p));
}
REQUIRE_XE16
inline bele_constexpr unsigned get_le16(const XE16 *p) noexcept {
    return ne16_to_le16(get_ne16(p));
}
REQUIRE_XE32
inline bele_constexpr unsigned get_le32(const XE32 *p) noexcept {
    return ne32_to_le32(get_ne32(p));
}
REQUIRE_XE64
inline bele_constexpr upx_uint64_t get_le64(const XE64 *p) noexcept {
    return ne64_to_le64(get_ne64(p));
}

REQUIRE_XE16
inline bele_constexpr void set_be16(XE16 *p, unsigned v) noexcept { set_ne16(p, ne16_to_be16(v)); }
REQUIRE_XE32
inline bele_constexpr void set_be32(XE32 *p, unsigned v) noexcept { set_ne32(p, ne32_to_be32(v)); }
REQUIRE_XE64
inline bele_constexpr void set_be64(XE64 *p, upx_uint64_t v) noexcept {
    set_ne64(p, ne64_to_be64(v));
}
REQUIRE_XE16
inline bele_constexpr void set_le16(XE16 *p, unsigned v) noexcept { set_ne16(p, ne16_to_le16(v)); }
REQUIRE_XE32
inline bele_constexpr void set_le32(XE32 *p, unsigned v) noexcept { set_ne32(p, ne32_to_le32(v)); }
REQUIRE_XE64
inline bele_constexpr void set_le64(XE64 *p, upx_uint64_t v) noexcept {
    set_ne64(p, ne64_to_le64(v));
}

/*************************************************************************
// get/set 24/26
**************************************************************************/

inline constexpr unsigned get_be24(const byte *p) noexcept {
    return upx::compile_time::get_be24(p);
}
inline constexpr unsigned get_le24(const byte *p) noexcept {
    return upx::compile_time::get_le24(p);
}
inline constexpr unsigned get_ne24(const byte *p) noexcept {
    return upx::compile_time::get_ne24(p);
}
inline constexpr void set_be24(byte *p, unsigned v) noexcept { upx::compile_time::set_be24(p, v); }
inline constexpr void set_le24(byte *p, unsigned v) noexcept { upx::compile_time::set_le24(p, v); }
inline constexpr void set_ne24(byte *p, unsigned v) noexcept { upx::compile_time::set_ne24(p, v); }

REQUIRE_XE24
forceinline bele_constexpr unsigned get_be24(const XE24 *p) noexcept {
    return get_be24(upx::ptr_static_cast<const byte *>(p));
}
REQUIRE_XE24
forceinline bele_constexpr unsigned get_le24(const XE24 *p) noexcept {
    return get_le24(upx::ptr_static_cast<const byte *>(p));
}
REQUIRE_XE24
forceinline bele_constexpr unsigned get_ne24(const XE24 *p) noexcept {
    return get_ne24(upx::ptr_static_cast<const byte *>(p));
}
REQUIRE_XE24
forceinline bele_constexpr void set_be24(XE24 *p, unsigned v) noexcept {
    set_be24(upx::ptr_static_cast<byte *>(p), v);
}
REQUIRE_XE24
forceinline bele_constexpr void set_le24(XE24 *p, unsigned v) noexcept {
    set_le24(upx::ptr_static_cast<byte *>(p), v);
}
REQUIRE_XE24
forceinline bele_constexpr void set_ne24(XE24 *p, unsigned v) noexcept {
    set_ne24(upx::ptr_static_cast<byte *>(p), v);
}

REQUIRE_XE32
inline bele_constexpr unsigned get_le26(const XE32 *p) noexcept { return get_le32(p) & 0x03ffffff; }
REQUIRE_XE32
inline bele_constexpr unsigned get_le19_5(const XE32 *p) noexcept {
    return (get_le32(p) >> 5) & 0x0007ffff;
}
REQUIRE_XE32
inline bele_constexpr unsigned get_le14_5(const XE32 *p) noexcept {
    return (get_le32(p) >> 5) & 0x00003fff;
}

REQUIRE_XE32
inline bele_constexpr void set_le26(XE32 *p, unsigned v) noexcept {
    // preserve the top 6 bits
    //   set_le32(p, (get_le32(p) & 0xfc000000) | (v & 0x03ffffff));
    // optimized version, saving a runtime bswap32
    set_ne32(p, (get_ne32(p) & ne32_to_le32(0xfc000000)) |
                    (ne32_to_le32(v) & ne32_to_le32(0x03ffffff)));
}
REQUIRE_XE32
inline bele_constexpr void set_le19_5(XE32 *p, unsigned v) noexcept {
    set_le32(p, (get_le32(p) & 0xff00001f) | ((v & 0x0007ffff) << 5));
}
REQUIRE_XE32
inline bele_constexpr void set_le14_5(XE32 *p, unsigned v) noexcept {
    set_le32(p, (get_le32(p) & 0xfff8001f) | ((v & 0x00003fff) << 5));
}

/*************************************************************************
// get signed values
**************************************************************************/

forceinline constexpr int sign_extend32(unsigned v, unsigned bits) noexcept {
#if (ACC_ARCH_M68K) // no barrel shifter
    const unsigned sign_bit = 1u << (bits - 1);
    return ACC_ICAST(int, (v & (sign_bit - 1)) - (v & sign_bit));
#else
    return ACC_ICAST(int, v << (32 - bits)) >> (32 - bits);
#endif
}

forceinline constexpr upx_int64_t sign_extend64(upx_uint64_t v, unsigned bits) noexcept {
#if (ACC_ARCH_M68K) // no barrel shifter
    const upx_uint64_t sign_bit = upx_uint64_t(1) << (bits - 1);
    return ACC_ICAST(upx_int64_t, (v & (sign_bit - 1)) - (v & sign_bit));
#else
    return ACC_ICAST(upx_int64_t, v << (64 - bits)) >> (64 - bits);
#endif
}

REQUIRE_XE16
inline bele_constexpr int get_be16_signed(const XE16 *p) noexcept {
    unsigned v = get_be16(p);
    return sign_extend32(v, 16);
}

REQUIRE_XE24
inline bele_constexpr int get_be24_signed(const XE24 *p) noexcept {
    unsigned v = get_be24(p);
    return sign_extend32(v, 24);
}

REQUIRE_XE32
inline bele_constexpr int get_be32_signed(const XE32 *p) noexcept {
    unsigned v = get_be32(p);
    return sign_extend32(v, 32);
}

REQUIRE_XE64
inline bele_constexpr upx_int64_t get_be64_signed(const XE64 *p) noexcept {
    upx_uint64_t v = get_be64(p);
    return sign_extend64(v, 64);
}

REQUIRE_XE16
inline bele_constexpr int get_le16_signed(const XE16 *p) noexcept {
    unsigned v = get_le16(p);
    return sign_extend32(v, 16);
}

REQUIRE_XE24
inline bele_constexpr int get_le24_signed(const XE24 *p) noexcept {
    unsigned v = get_le24(p);
    return sign_extend32(v, 24);
}

REQUIRE_XE32
inline bele_constexpr int get_le32_signed(const XE32 *p) noexcept {
    unsigned v = get_le32(p);
    return sign_extend32(v, 32);
}

REQUIRE_XE64
inline bele_constexpr upx_int64_t get_le64_signed(const XE64 *p) noexcept {
    upx_uint64_t v = get_le64(p);
    return sign_extend64(v, 64);
}

/*************************************************************************
// classes for portable unaligned access
//
// Note: these classes must be PODs (Plain Old Data), i.e. no
//   constructor, no destructor, no virtual functions and no default
//   assignment operator, and all fields must be public
**************************************************************************/

struct alignas(1) BE16 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[2];

    static forceinline constexpr BE16 make(const BE16 &x) noexcept { return x; }
    static forceinline bele_constexpr BE16 make(unsigned v) noexcept {
        BE16 x = {};
        set_be16(x.d, v);
        return x;
    }

    forceinline bele_constexpr operator unsigned() const noexcept { return get_be16(d); }

    forceinline bele_constexpr BE16 &operator=(unsigned v) noexcept {
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

    bele_constexpr bool operator==(const BE16 &x) const noexcept {
#if defined(upx_is_constant_evaluated)
        if (upx_is_constant_evaluated())
            return upx::compile_time::mem_eq(d, x.d, sizeof(d));
        else
#endif
            return upx_memcmp_inline(d, x.d, sizeof(d)) == 0;
    }
    bele_constexpr bool operator<(const BE16 &x) const noexcept {
        return unsigned(*this) < unsigned(x);
    }
};

struct alignas(1) BE32 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[4];

    static forceinline constexpr BE32 make(const BE32 &x) noexcept { return x; }
    static forceinline bele_constexpr BE32 make(unsigned v) noexcept {
        BE32 x = {};
        set_be32(x.d, v);
        return x;
    }

    forceinline bele_constexpr operator unsigned() const noexcept { return get_be32(d); }

    forceinline bele_constexpr BE32 &operator=(unsigned v) noexcept {
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

    bele_constexpr bool operator==(const BE32 &x) const noexcept {
#if defined(upx_is_constant_evaluated)
        if (upx_is_constant_evaluated())
            return upx::compile_time::mem_eq(d, x.d, sizeof(d));
        else
#endif
            return upx_memcmp_inline(d, x.d, sizeof(d)) == 0;
    }
    bele_constexpr bool operator<(const BE32 &x) const noexcept {
        return unsigned(*this) < unsigned(x);
    }
};

struct alignas(1) BE64 final {
    typedef upx_uint64_t integral_conversion_type; // automatic conversion to upx_uint64_t
    byte d[8];

    static forceinline constexpr BE64 make(const BE64 &x) noexcept { return x; }
    static forceinline bele_constexpr BE64 make(upx_uint64_t v) noexcept {
        BE64 x = {};
        set_be64(x.d, v);
        return x;
    }

    forceinline bele_constexpr operator upx_uint64_t() const noexcept { return get_be64(d); }

    forceinline bele_constexpr BE64 &operator=(upx_uint64_t v) noexcept {
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

    bele_constexpr bool operator==(const BE64 &x) const noexcept {
#if defined(upx_is_constant_evaluated)
        if (upx_is_constant_evaluated())
            return upx::compile_time::mem_eq(d, x.d, sizeof(d));
        else
#endif
            return upx_memcmp_inline(d, x.d, sizeof(d)) == 0;
    }
    bele_constexpr bool operator<(const BE64 &x) const noexcept {
        return upx_uint64_t(*this) < upx_uint64_t(x);
    }
};

struct alignas(1) LE16 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[2];

    static forceinline constexpr LE16 make(const LE16 &x) noexcept { return x; }
    static forceinline bele_constexpr LE16 make(unsigned v) noexcept {
        LE16 x = {};
        set_le16(x.d, v);
        return x;
    }

    forceinline bele_constexpr operator unsigned() const noexcept { return get_le16(d); }

    forceinline bele_constexpr LE16 &operator=(unsigned v) noexcept {
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

    bele_constexpr bool operator==(const LE16 &x) const noexcept {
#if defined(upx_is_constant_evaluated)
        if (upx_is_constant_evaluated())
            return upx::compile_time::mem_eq(d, x.d, sizeof(d));
        else
#endif
            return upx_memcmp_inline(d, x.d, sizeof(d)) == 0;
    }
    bele_constexpr bool operator<(const LE16 &x) const noexcept {
        return unsigned(*this) < unsigned(x);
    }
};

struct alignas(1) LE32 final {
    typedef unsigned integral_conversion_type; // automatic conversion to unsigned
    byte d[4];

    static forceinline constexpr LE32 make(const LE32 &x) noexcept { return x; }
    static forceinline bele_constexpr LE32 make(unsigned v) noexcept {
        LE32 x = {};
        set_le32(x.d, v);
        return x;
    }

    forceinline bele_constexpr operator unsigned() const noexcept { return get_le32(d); }

    forceinline bele_constexpr LE32 &operator=(unsigned v) noexcept {
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

    bele_constexpr bool operator==(const LE32 &x) const noexcept {
#if defined(upx_is_constant_evaluated)
        if (upx_is_constant_evaluated())
            return upx::compile_time::mem_eq(d, x.d, sizeof(d));
        else
#endif
            return upx_memcmp_inline(d, x.d, sizeof(d)) == 0;
    }
    bele_constexpr bool operator<(const LE32 &x) const noexcept {
        return unsigned(*this) < unsigned(x);
    }
};

struct alignas(1) LE64 final {
    typedef upx_uint64_t integral_conversion_type; // automatic conversion to upx_uint64_t
    byte d[8];

    static forceinline constexpr LE64 make(const LE64 &x) noexcept { return x; }
    static forceinline bele_constexpr LE64 make(upx_uint64_t v) noexcept {
        LE64 x = {};
        set_le64(x.d, v);
        return x;
    }

    forceinline bele_constexpr operator upx_uint64_t() const noexcept { return get_le64(d); }

    forceinline bele_constexpr LE64 &operator=(upx_uint64_t v) noexcept {
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

    bele_constexpr bool operator==(const LE64 &x) const noexcept {
#if defined(upx_is_constant_evaluated)
        if (upx_is_constant_evaluated())
            return upx::compile_time::mem_eq(d, x.d, sizeof(d));
        else
#endif
            return upx_memcmp_inline(d, x.d, sizeof(d)) == 0;
    }
    bele_constexpr bool operator<(const LE64 &x) const noexcept {
        return upx_uint64_t(*this) < upx_uint64_t(x);
    }
};

/*************************************************************************
// global operators (pointer addition/subtraction)
**************************************************************************/

template <class T>
inline bele_constexpr T *operator+(T *ptr, const BE16 &v) noexcept {
    return ptr + size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator-(T *ptr, const BE16 &v) noexcept {
    return ptr - size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator+(T *ptr, const BE32 &v) noexcept {
    return ptr + size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator-(T *ptr, const BE32 &v) noexcept {
    return ptr - size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator+(T *ptr, const LE16 &v) noexcept {
    return ptr + size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator-(T *ptr, const LE16 &v) noexcept {
    return ptr - size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator+(T *ptr, const LE32 &v) noexcept {
    return ptr + size_t(unsigned(v));
}
template <class T>
inline bele_constexpr T *operator-(T *ptr, const LE32 &v) noexcept {
    return ptr - size_t(unsigned(v));
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

#if !ALLOW_INT_PLUS_MEMBUFFER
template <class T>
T *operator+(const BE16 &v, T *ptr) noexcept DELETED_FUNCTION;
template <class T>
T *operator+(const BE32 &v, T *ptr) noexcept DELETED_FUNCTION;
template <class T>
T *operator+(const LE16 &v, T *ptr) noexcept DELETED_FUNCTION;
template <class T>
T *operator+(const LE32 &v, T *ptr) noexcept DELETED_FUNCTION;
#endif // ALLOW_INT_PLUS_MEMBUFFER

template <class T>
T *operator+(const BE64 &v, T *ptr) noexcept DELETED_FUNCTION;
template <class T>
T *operator+(const LE64 &v, T *ptr) noexcept DELETED_FUNCTION;

/*************************************************************************
// some global overloads
**************************************************************************/

namespace upx {

#define REQUIRE_UINT32                                                                             \
    template <class T, class = std::enable_if_t<std::is_same_v<T, upx_uint32_t>, T> >
#define REQUIRE_UINT64                                                                             \
    template <class T, class = std::enable_if_t<std::is_same_v<T, upx_uint64_t>, T> >

REQUIRE_UINT32
inline bele_constexpr T align_down(const T &a, const BE32 &b) noexcept {
    return align_down(a, T(b));
}
REQUIRE_UINT32
inline bele_constexpr T align_down(const BE32 &a, const T &b) noexcept {
    return align_down(T(a), b);
}
REQUIRE_UINT32
inline bele_constexpr T align_down(const T &a, const LE32 &b) noexcept {
    return align_down(a, T(b));
}
REQUIRE_UINT32
inline bele_constexpr T align_down(const LE32 &a, const T &b) noexcept {
    return align_down(T(a), b);
}

REQUIRE_UINT32
inline bele_constexpr T align_up(const T &a, const BE32 &b) noexcept { return align_up(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T align_up(const BE32 &a, const T &b) noexcept { return align_up(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T align_up(const T &a, const LE32 &b) noexcept { return align_up(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T align_up(const LE32 &a, const T &b) noexcept { return align_up(T(a), b); }

REQUIRE_UINT32
inline bele_constexpr T min(const T &a, const BE16 &b) noexcept { return min(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T min(const BE16 &a, const T &b) noexcept { return min(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T min(const T &a, const BE32 &b) noexcept { return min(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T min(const BE32 &a, const T &b) noexcept { return min(T(a), b); }
REQUIRE_UINT64
inline bele_constexpr T min(const T &a, const BE64 &b) noexcept { return min(a, T(b)); }
REQUIRE_UINT64
inline bele_constexpr T min(const BE64 &a, const T &b) noexcept { return min(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T min(const T &a, const LE16 &b) noexcept { return min(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T min(const LE16 &a, const T &b) noexcept { return min(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T min(const T &a, const LE32 &b) noexcept { return min(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T min(const LE32 &a, const T &b) noexcept { return min(T(a), b); }
REQUIRE_UINT64
inline bele_constexpr T min(const T &a, const LE64 &b) noexcept { return min(a, T(b)); }
REQUIRE_UINT64
inline bele_constexpr T min(const LE64 &a, const T &b) noexcept { return min(T(a), b); }

REQUIRE_UINT32
inline bele_constexpr T max(const T &a, const BE16 &b) noexcept { return max(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T max(const BE16 &a, const T &b) noexcept { return max(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T max(const T &a, const BE32 &b) noexcept { return max(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T max(const BE32 &a, const T &b) noexcept { return max(T(a), b); }
REQUIRE_UINT64
inline bele_constexpr T max(const T &a, const BE64 &b) noexcept { return max(a, T(b)); }
REQUIRE_UINT64
inline bele_constexpr T max(const BE64 &a, const T &b) noexcept { return max(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T max(const T &a, const LE16 &b) noexcept { return max(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T max(const LE16 &a, const T &b) noexcept { return max(T(a), b); }
REQUIRE_UINT32
inline bele_constexpr T max(const T &a, const LE32 &b) noexcept { return max(a, T(b)); }
REQUIRE_UINT32
inline bele_constexpr T max(const LE32 &a, const T &b) noexcept { return max(T(a), b); }
REQUIRE_UINT64
inline bele_constexpr T max(const T &a, const LE64 &b) noexcept { return max(a, T(b)); }
REQUIRE_UINT64
inline bele_constexpr T max(const LE64 &a, const T &b) noexcept { return max(T(a), b); }

#undef REQUIRE_UINT32
#undef REQUIRE_UINT64

} // namespace upx

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

// <type_traits> upx_is_integral; see conf.h
#define TT_UPX_IS_INTEGRAL(T)                                                                      \
    template <>                                                                                    \
    struct upx_is_integral<T> : public std::true_type {};                                          \
    template <>                                                                                    \
    struct upx_is_integral<const T> : public std::true_type {};                                    \
    template <>                                                                                    \
    struct upx_is_integral<volatile T> : public std::true_type {};                                 \
    template <>                                                                                    \
    struct upx_is_integral<const volatile T> : public std::true_type {}
TT_UPX_IS_INTEGRAL(BE16);
TT_UPX_IS_INTEGRAL(BE32);
TT_UPX_IS_INTEGRAL(BE64);
TT_UPX_IS_INTEGRAL(LE16);
TT_UPX_IS_INTEGRAL(LE32);
TT_UPX_IS_INTEGRAL(LE64);
#undef TT_UPX_IS_INTEGRAL

// NE - Native Endianness (aka Host Endianness aka CPU Endianness)
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
#elif (ACC_ABI_LITTLE_ENDIAN)
typedef LE16 NE16;
typedef LE32 NE32;
typedef LE64 NE64;
#define ne16_compare        le16_compare
#define ne32_compare        le32_compare
#define ne64_compare        le64_compare
#define ne16_compare_signed le16_compare_signed
#define ne32_compare_signed le32_compare_signed
#define ne64_compare_signed le64_compare_signed
#else
#error "ACC_ABI_ENDIAN"
#endif

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

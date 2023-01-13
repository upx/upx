/* membuffer.cpp --

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

#include "../conf.h"
#include "membuffer.h"

// extra functions to reduce dependency on membuffer.h
void *membuffer_get_void_ptr(MemBuffer &mb) { return mb.getVoidPtr(); }
unsigned membuffer_get_size(MemBuffer &mb) { return mb.getSize(); }

MemBuffer::Stats MemBuffer::stats;

#if DEBUG
#define debug_set(var, expr) (var) = (expr)
#else
#define debug_set(var, expr) /*empty*/
#endif

/*************************************************************************
// bool use_simple_mcheck()
**************************************************************************/

#if defined(__SANITIZE_ADDRESS__)
__acc_static_forceinline constexpr bool use_simple_mcheck() { return false; }
#elif (WITH_VALGRIND) && defined(RUNNING_ON_VALGRIND)
static int use_simple_mcheck_flag = -1;
__acc_static_noinline void use_simple_mcheck_init() {
    use_simple_mcheck_flag = 1;
    if (RUNNING_ON_VALGRIND) {
        use_simple_mcheck_flag = 0;
        // fprintf(stderr, "upx: detected RUNNING_ON_VALGRIND\n");
    }
}
__acc_static_forceinline bool use_simple_mcheck() {
    if __acc_unlikely (use_simple_mcheck_flag < 0)
        use_simple_mcheck_init();
    return (bool) use_simple_mcheck_flag;
}
#else
__acc_static_forceinline constexpr bool use_simple_mcheck() { return true; }
#endif

/*************************************************************************
//
**************************************************************************/

MemBuffer::MemBuffer(upx_uint64_t size_in_bytes) {
    alloc(size_in_bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

MemBuffer::~MemBuffer() { this->dealloc(); }

// similar to BoundedPtr, except checks only at creation
// skip == offset, take == size_in_bytes
void *MemBuffer::subref_impl(const char *errfmt, size_t skip, size_t take) {
    debug_set(debug.last_return_address_subref, upx_return_address());
    // check overrun and wrap-around
    if (skip + take > b_size_in_bytes || skip + take < skip) {
        char buf[100];
        // printf is using unsigned formatting
        if (!errfmt || !errfmt[0])
            errfmt = "bad subref %#x %#x";
        snprintf(buf, sizeof(buf), errfmt, (unsigned) skip, (unsigned) take);
        throwCantPack(buf);
    }
    return &b[skip];
}

static unsigned width(unsigned x) {
    unsigned w = 0;
    if ((~0u << 16) & x) {
        w += 16;
        x >>= 16;
    }
    if ((~0u << 8) & x) {
        w += 8;
        x >>= 8;
    }
    if ((~0u << 4) & x) {
        w += 4;
        x >>= 4;
    }
    if ((~0u << 2) & x) {
        w += 2;
        x >>= 2;
    }
    if ((~0u << 1) & x) {
        w += 1;
        // x >>= 1;
    }
    return 1 + w;
}

static inline unsigned umax(unsigned a, unsigned b) { return (a >= b) ? a : b; }

unsigned MemBuffer::getSizeForCompression(unsigned uncompressed_size, unsigned extra) {
    unsigned const z = uncompressed_size;                 // fewer keystrokes and display columns
    unsigned const w = umax(8, width(z - 1));             // ignore tiny offsets
    unsigned bytes = ACC_ICONV(unsigned, mem_size(1, z)); // check
    // Worst matching: All match at max_offset, which implies 3==min_match
    // All literal: 1 bit overhead per literal byte
    bytes = umax(bytes, bytes + z / 8);
    // NRV2B: 1 byte plus 2 bits per width exceeding 8 ("ss11")
    bytes = umax(bytes, (z / 3 * (8 + 2 * (w - 8) / 1)) / 8);
    // NRV2E: 1 byte plus 3 bits per pair of width exceeding 7 ("ss12")
    bytes = umax(bytes, (z / 3 * (8 + 3 * (w - 7) / 2)) / 8);
    // zstd: ZSTD_COMPRESSBOUND
    bytes = umax(bytes, z + (z >> 8) + ((z < (128 << 10)) ? (((128 << 10) - z) >> 11) : 0));
    // extra + 256 safety for rounding
    bytes = mem_size(1, bytes, extra, 256);
    return bytes;
}

unsigned MemBuffer::getSizeForDecompression(unsigned uncompressed_size, unsigned extra) {
    size_t bytes = mem_size(1, uncompressed_size, extra); // check
    return ACC_ICONV(unsigned, bytes);
}

void MemBuffer::allocForCompression(unsigned uncompressed_size, unsigned extra) {
    unsigned size = getSizeForCompression(uncompressed_size, extra);
    alloc(size);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

void MemBuffer::allocForDecompression(unsigned uncompressed_size, unsigned extra) {
    unsigned size = getSizeForDecompression(uncompressed_size, extra);
    alloc(size);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

void MemBuffer::fill(unsigned off, unsigned len, int value) {
    debug_set(debug.last_return_address_fill, upx_return_address());
    checkState();
    assert((int) off >= 0);
    assert((int) len >= 0);
    assert(off <= b_size_in_bytes);
    assert(len <= b_size_in_bytes);
    assert(off + len <= b_size_in_bytes);
    if (len > 0)
        memset(b + off, value, len);
}

/*************************************************************************
//
**************************************************************************/

#define PTR_BITS(p) ((unsigned) ((upx_uintptr_t) (p) &0xffffffff))
#define MAGIC1(p) ((PTR_BITS(p) ^ 0xfefdbeeb) | 1)
#define MAGIC2(p) ((PTR_BITS(p) ^ 0xfefdbeeb ^ 0x80024011) | 1)

void MemBuffer::checkState() const {
    if (!b)
        throwInternalError("block not allocated");
    if (use_simple_mcheck()) {
        if (get_ne32(b - 4) != MAGIC1(b))
            throwInternalError("memory clobbered before allocated block 1");
        if (get_ne32(b - 8) != b_size_in_bytes)
            throwInternalError("memory clobbered before allocated block 2");
        if (get_ne32(b + b_size_in_bytes) != MAGIC2(b))
            throwInternalError("memory clobbered past end of allocated block");
    }
}

void MemBuffer::alloc(upx_uint64_t size) {
    // NOTE: we don't automatically free a used buffer
    assert(b == nullptr);
    assert(b_size_in_bytes == 0);
    //
    assert(size > 0);
    debug_set(debug.last_return_address_alloc, upx_return_address());
    size_t bytes = mem_size(1, size, use_simple_mcheck() ? 32 : 0);
    unsigned char *p = (unsigned char *) malloc(bytes);
    NO_printf("MemBuffer::alloc %llu: %p\n", size, p);
    if (!p)
        throwOutOfMemoryException();
    b = p;
    b_size_in_bytes = ACC_ICONV(unsigned, size);
    if (use_simple_mcheck()) {
        b = p + 16;
        // store magic constants to detect buffer overruns
        set_ne32(b - 8, b_size_in_bytes);
        set_ne32(b - 4, MAGIC1(b));
        set_ne32(b + b_size_in_bytes, MAGIC2(b));
        set_ne32(b + b_size_in_bytes + 4, stats.global_alloc_counter);
    }
#if !defined(__SANITIZE_ADDRESS__) && 0
    fill(0, b_size_in_bytes, (rand() & 0xff) | 1); // debug
    (void) VALGRIND_MAKE_MEM_UNDEFINED(b, b_size_in_bytes);
#endif
    stats.global_alloc_counter += 1;
    stats.global_total_bytes += b_size_in_bytes;
    stats.global_total_active_bytes += b_size_in_bytes;
}

void MemBuffer::dealloc() {
    if (b != nullptr) {
        debug_set(debug.last_return_address_dealloc, upx_return_address());
        checkState();
        stats.global_total_active_bytes -= b_size_in_bytes;
        if (use_simple_mcheck()) {
            // clear magic constants
            set_ne32(b - 8, 0);
            set_ne32(b - 4, 0);
            set_ne32(b + b_size_in_bytes, 0);
            set_ne32(b + b_size_in_bytes + 4, 0);
            //
            ::free(b - 16);
        } else
            ::free(b);
        b = nullptr;
        b_size_in_bytes = 0;
    } else {
        assert(b_size_in_bytes == 0);
    }
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("MemBuffer") {
    MemBuffer mb;
    CHECK_THROWS(mb.checkState());
    CHECK_THROWS(mb.alloc(0x30000000 + 1));
    CHECK(raw_bytes(mb, 0) == nullptr);
    CHECK_THROWS(raw_bytes(mb, 1));
    mb.alloc(64);
    mb.checkState();
    CHECK(raw_bytes(mb, 64) != nullptr);
    CHECK(raw_bytes(mb, 64) == mb.getVoidPtr());
    CHECK_THROWS(raw_bytes(mb, 65));
    if (use_simple_mcheck()) {
        upx_byte *b = raw_bytes(mb, 0);
        unsigned magic1 = get_ne32(b - 4);
        set_ne32(b - 4, magic1 ^ 1);
        CHECK_THROWS(mb.checkState());
        set_ne32(b - 4, magic1);
        mb.checkState();
    }
}

/* vim:set ts=4 sw=4 et: */

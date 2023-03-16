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

/*static*/ MemBuffer::Stats MemBuffer::stats;

#if DEBUG
#define debug_set(var, expr) (var) = (expr)
#else
#define debug_set(var, expr) /*empty*/
#endif

/*************************************************************************
// bool use_simple_mcheck()
**************************************************************************/

#if defined(__SANITIZE_ADDRESS__)
static forceinline constexpr bool use_simple_mcheck() { return false; }
#elif (WITH_VALGRIND) && defined(RUNNING_ON_VALGRIND)
static bool use_simple_mcheck_flag;
static noinline void init_use_simple_mcheck() noexcept {
    bool r = true;
    if (RUNNING_ON_VALGRIND) {
        r = false;
        NO_fprintf(stderr, "upx: detected RUNNING_ON_VALGRIND\n");
    }
    use_simple_mcheck_flag = r;
}
static bool use_simple_mcheck() {
    static upx_std_once_flag init_done;
    upx_std_call_once(init_done, init_use_simple_mcheck);
    return use_simple_mcheck_flag;
}
#else
static forceinline constexpr bool use_simple_mcheck() { return true; }
#endif

/*************************************************************************
//
**************************************************************************/

MemBuffer::MemBuffer(upx_uint64_t bytes) {
    alloc(bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

MemBuffer::~MemBuffer() noexcept { this->dealloc(); }

// similar to BoundedPtr, except checks only at creation
// skip == offset, take == size_in_bytes
void *MemBuffer::subref_impl(const char *errfmt, size_t skip, size_t take) {
    debug_set(debug.last_return_address_subref, upx_return_address());
    // check overrun and wrap-around
    if (skip + take > size_in_bytes || skip + take < skip) {
        char buf[100];
        // printf is using unsigned formatting
        if (!errfmt || !errfmt[0])
            errfmt = "bad subref %#x %#x";
        snprintf(buf, sizeof(buf), errfmt, (unsigned) skip, (unsigned) take);
        throwCantPack(buf);
    }
    return ptr + skip;
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

static forceinline size_t umax(size_t a, size_t b) { return (a >= b) ? a : b; }

unsigned MemBuffer::getSizeForCompression(unsigned uncompressed_size, unsigned extra) {
    if (uncompressed_size == 0)
        throwCantPack("invalid uncompressed_size");
    const size_t z = uncompressed_size;     // fewer keystrokes and display columns
    const size_t w = umax(8, width(z - 1)); // ignore tiny offsets
    size_t bytes = mem_size(1, z);          // check size
    // Worst matching: All match at max_offset, which implies 3==min_match
    // All literal: 1 bit overhead per literal byte; from UCL documentation
    bytes = umax(bytes, z + z / 8 + 256);
    // NRV2B: 1 byte plus 2 bits per width exceeding 8 ("ss11")
    bytes = umax(bytes, (z / 3 * (8 + 2 * (w - 8) / 1)) / 8);
    // NRV2E: 1 byte plus 3 bits per pair of width exceeding 7 ("ss12")
    bytes = umax(bytes, (z / 3 * (8 + 3 * (w - 7) / 2)) / 8);
    // zstd: ZSTD_COMPRESSBOUND
    bytes = umax(bytes, z + (z >> 8) + ((z < (128 << 10)) ? (((128 << 10) - z) >> 11) : 0));
    // add extra and 256 safety for various rounding/alignments
    bytes = mem_size(1, bytes, extra, 256);
    UNUSED(w);
    return ACC_ICONV(unsigned, bytes);
}

unsigned MemBuffer::getSizeForDecompression(unsigned uncompressed_size, unsigned extra) {
    if (uncompressed_size == 0)
        throwCantPack("invalid uncompressed_size");
    size_t bytes = mem_size(1, uncompressed_size, extra); // check size
    return ACC_ICONV(unsigned, bytes);
}

void MemBuffer::allocForCompression(unsigned uncompressed_size, unsigned extra) {
    unsigned bytes = getSizeForCompression(uncompressed_size, extra);
    alloc(bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

void MemBuffer::allocForDecompression(unsigned uncompressed_size, unsigned extra) {
    unsigned bytes = getSizeForDecompression(uncompressed_size, extra);
    alloc(bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

void MemBuffer::fill(unsigned off, unsigned len, int value) {
    debug_set(debug.last_return_address_fill, upx_return_address());
    checkState();
    if (off > size_in_bytes || len > size_in_bytes || off + len > size_in_bytes)
        throwCantPack("MemBuffer::fill out of range; take care!");
    if (len > 0)
        memset(ptr + off, value, len);
}

/*************************************************************************
//
**************************************************************************/

#define PTR_BITS(p) ((unsigned) ((upx_uintptr_t) (p) &0xffffffff))
#define MAGIC1(p) ((PTR_BITS(p) ^ 0xfefdbeeb) | 1)
#define MAGIC2(p) ((PTR_BITS(p) ^ 0xfefdbeeb ^ 0x80024011) | 1)

void MemBuffer::checkState() const {
    if (!ptr)
        throwInternalError("block not allocated");
    assert(size_in_bytes > 0);
    if (use_simple_mcheck()) {
        const byte *p = (const byte *) ptr;
        if (get_ne32(p - 4) != MAGIC1(p))
            throwInternalError("memory clobbered before allocated block 1");
        if (get_ne32(p - 8) != size_in_bytes)
            throwInternalError("memory clobbered before allocated block 2");
        if (get_ne32(p + size_in_bytes) != MAGIC2(p))
            throwInternalError("memory clobbered past end of allocated block");
    }
}

void MemBuffer::alloc(upx_uint64_t bytes) {
    // NOTE: we don't automatically free a used buffer
    assert(ptr == nullptr);
    assert(size_in_bytes == 0);
    //
    assert(bytes > 0);
    debug_set(debug.last_return_address_alloc, upx_return_address());
    size_t malloc_bytes = mem_size(1, bytes);
    if (use_simple_mcheck())
        malloc_bytes += 32;
    byte *p = (byte *) ::malloc(malloc_bytes);
    NO_printf("MemBuffer::alloc %llu: %p\n", bytes, p);
    if (!p)
        throwOutOfMemoryException();
    size_in_bytes = ACC_ICONV(unsigned, bytes);
    if (use_simple_mcheck()) {
        p += 16;
        // store magic constants to detect buffer overruns
        set_ne32(p - 8, size_in_bytes);
        set_ne32(p - 4, MAGIC1(p));
        set_ne32(p + size_in_bytes, MAGIC2(p));
        set_ne32(p + size_in_bytes + 4, stats.global_alloc_counter);
    }
    ptr = (pointer) (void *) p;
#if DEBUG
    memset(ptr, 0xff, size_in_bytes);
    (void) VALGRIND_MAKE_MEM_UNDEFINED(ptr, size_in_bytes);
#endif
    stats.global_alloc_counter += 1;
    stats.global_total_bytes += size_in_bytes;
    stats.global_total_active_bytes += size_in_bytes;
#if DEBUG || 1
    checkState();
#endif
}

void MemBuffer::dealloc() noexcept {
    if (ptr != nullptr) {
        debug_set(debug.last_return_address_dealloc, upx_return_address());
        checkState();
        stats.global_dealloc_counter += 1;
        stats.global_total_active_bytes -= size_in_bytes;
        if (use_simple_mcheck()) {
            byte *p = (byte *) ptr;
            // clear magic constants
            set_ne32(p - 8, 0);
            set_ne32(p - 4, 0);
            set_ne32(p + size_in_bytes, 0);
            set_ne32(p + size_in_bytes + 4, 0);
            //
            ::free(p - 16);
        } else {
            ::free(ptr);
        }
        ptr = nullptr;
        size_in_bytes = 0;
    } else {
        assert(size_in_bytes == 0);
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
    CHECK_NOTHROW(mb + 64);
    CHECK_THROWS(mb + 65);
#if ALLOW_INT_PLUS_MEMBUFFER
    CHECK_NOTHROW(64 + mb);
    CHECK_THROWS(65 + mb);
#endif
    CHECK_NOTHROW(mb.subref("", 0, 64));
    CHECK_NOTHROW(mb.subref("", 64, 0));
    CHECK_THROWS(mb.subref("", 1, 64));
    CHECK_THROWS(mb.subref("", 64, 1));
    if (use_simple_mcheck()) {
        byte *p = raw_bytes(mb, 0);
        unsigned magic1 = get_ne32(p - 4);
        set_ne32(p - 4, magic1 ^ 1);
        CHECK_THROWS(mb.checkState());
        set_ne32(p - 4, magic1);
        mb.checkState();
    }
}

TEST_CASE("MemBuffer unused") {
    MemBuffer mb;
    CHECK(mb.raw_ptr() == nullptr);
    CHECK(mb.raw_size_in_bytes() == 0);
}

TEST_CASE("MemBuffer::getSizeForCompression") {
    CHECK_THROWS(MemBuffer::getSizeForCompression(0));
    CHECK_THROWS(MemBuffer::getSizeForDecompression(0));
    CHECK(MemBuffer::getSizeForCompression(1) == 513);
    CHECK(MemBuffer::getSizeForCompression(256) == 800);
    CHECK(MemBuffer::getSizeForCompression(1024) == 1664);
    // CHECK(MemBuffer::getSizeForCompression(1024 * 1024) == 0); // TODO
    // CHECK(MemBuffer::getSizeForCompression(UPX_RSIZE_MAX) == 0); // TODO
}

/* vim:set ts=4 sw=4 et: */

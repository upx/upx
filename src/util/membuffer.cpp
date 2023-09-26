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

// A MemBuffer allocates memory on the heap, and automatically
// gets destructed when leaving scope or on exceptions.

#include "../conf.h"
#include "membuffer.h"

// extra functions to reduce dependency on membuffer.h
void *membuffer_get_void_ptr(MemBuffer &mb) noexcept { return mb.getVoidPtr(); }
unsigned membuffer_get_size(MemBuffer &mb) noexcept { return mb.getSize(); }

/*static*/ MemBuffer::Stats MemBuffer::stats;

#if DEBUG
#define debug_set(var, expr) (var) = (expr)
#else
#define debug_set(var, expr) /*empty*/
#endif

/*************************************************************************
// bool use_simple_mcheck()
**************************************************************************/

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_MEMORY__)
static forceinline constexpr bool use_simple_mcheck() noexcept { return false; }
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
static bool use_simple_mcheck() noexcept {
    static upx_std_once_flag init_done;
    upx_std_call_once(init_done, init_use_simple_mcheck);
    // NOTE: clang-analyzer-unix.Malloc does not know that this flag is "constant"; see below
    return use_simple_mcheck_flag;
}
#else
static forceinline constexpr bool use_simple_mcheck() noexcept { return true; }
#endif

/*************************************************************************
//
**************************************************************************/

MemBuffer::MemBuffer(upx_uint64_t bytes) : MemBufferBase<byte>() {
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

static forceinline constexpr size_t umax(size_t a, size_t b) { return (a >= b) ? a : b; }

/*static*/
unsigned MemBuffer::getSizeForCompression(unsigned uncompressed_size, unsigned extra) {
    if (uncompressed_size == 0)
        throwCantPack("invalid uncompressed_size");
    const size_t z = uncompressed_size; // fewer keystrokes and display columns
    size_t bytes = mem_size(1, z);      // check size
    // All literal: 1 bit overhead per literal byte; from UCL documentation
    bytes = umax(bytes, z + z / 8 + 256);
    // zstd: ZSTD_COMPRESSBOUND
    bytes = umax(bytes, z + (z >> 8) + ((z < (128 << 10)) ? (((128 << 10) - z) >> 11) : 0));
    // add extra and 256 safety for various rounding/alignments
    bytes = mem_size(1, bytes, extra, 256);
    return ACC_ICONV(unsigned, bytes);
}

/*static*/
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

// for use_simple_mcheck()
#define PTR_BITS32(p) ((unsigned) ((upx_uintptr_t) (p) &0xffffffff))
#define MAGIC1(p)     ((PTR_BITS32(p) ^ 0xfefdbeeb) | 1)
#define MAGIC2(p)     ((PTR_BITS32(p) ^ 0xfefdbeeb ^ 0x88224411) | 1)

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
    // INFO: we don't automatically free a used buffer
    assert(ptr == nullptr);
    assert(size_in_bytes == 0);
    //
    assert(bytes > 0);
    debug_set(debug.last_return_address_alloc, upx_return_address());
    size_t malloc_bytes = mem_size(1, bytes); // check size
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
        set_ne32(p + size_in_bytes + 0, MAGIC2(p));
        set_ne32(p + size_in_bytes + 4, stats.global_alloc_counter);
    }
    ptr = (pointer) (void *) p;
#if !defined(__SANITIZE_MEMORY__) && DEBUG
    memset(ptr, 0xfb, size_in_bytes);
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
#if DEBUG || 1
        // info: calling checkState() here violates "noexcept", so we need a try block
        bool shall_check = true;
        // bool shall_check = (std::uncaught_exceptions() == 0); // only if not unwinding
        // TODO later: add a priority() method to class Throwable
        if (shall_check) {
            try {
                checkState();
            } catch (const Throwable &e) {
                printErr("unknown", e);
                std::terminate();
            } catch (...) {
                std::terminate();
            }
        }
#endif
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
            ::free(p - 16); // NOLINT(clang-analyzer-unix.Malloc) // see NOTE above
        } else {
            ::free(ptr); // NOLINT(clang-analyzer-unix.Malloc) // see NOTE above
        }
        ptr = nullptr;
        size_in_bytes = 0;
    } else {
        assert_noexcept(size_in_bytes == 0);
    }
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("MemBuffer core") {
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

TEST_CASE("MemBuffer global overloads") {
    MemBuffer mb(1);
    MemBuffer mb4(4);
    mb.clear();
    mb4.clear();
    CHECK(memcmp(mb, "\x00", 1) == 0);
    CHECK_THROWS(memcmp(mb, "\x00\x00", 2)); // NOLINT(bugprone-unused-return-value)
    CHECK_THROWS(memcmp("\x00\x00", mb, 2)); // NOLINT(bugprone-unused-return-value)
    CHECK_THROWS(memcmp(mb, mb4, 2));        // NOLINT(bugprone-unused-return-value)
    CHECK_THROWS(memcmp(mb4, mb, 2));        // NOLINT(bugprone-unused-return-value)
    CHECK_NOTHROW(memset(mb, 255, 1));
    CHECK_THROWS(memset(mb, 254, 2));
    CHECK(mb[0] == 255);
    CHECK_THROWS(get_be16(mb));
    CHECK_THROWS(get_be32(mb));
    CHECK_THROWS(get_be64(mb));
    CHECK_THROWS(get_le16(mb));
    CHECK_THROWS(get_le32(mb));
    CHECK_THROWS(get_le64(mb));
    CHECK_NOTHROW(get_be16(mb4));
    CHECK_NOTHROW(get_be32(mb4));
    CHECK_THROWS(get_be64(mb4));
    CHECK_NOTHROW(get_le16(mb4));
    CHECK_NOTHROW(get_le32(mb4));
    CHECK_THROWS(get_le64(mb4));
    CHECK_NOTHROW(set_be32(mb4, 0));
    CHECK_THROWS(set_be64(mb4, 0));
    CHECK_NOTHROW(set_le32(mb4, 0));
    CHECK_THROWS(set_le64(mb4, 0));
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
    CHECK(MemBuffer::getSizeForCompression(1024 * 1024) == 1180160);         // 0x00100000
    CHECK(MemBuffer::getSizeForCompression(64 * 1024 * 1024) == 75497984);   // 0x04000000
    CHECK(MemBuffer::getSizeForCompression(512 * 1024 * 1024) == 603980288); // 0x20000000
    CHECK(MemBuffer::getSizeForCompression(640 * 1024 * 1024) == 754975232); // 0x28000000
    // "682 MiB Ought to be Enough for Anyone" --Markus F.X.J. Oberhumer, 2023 ;-)
    CHECK(MemBuffer::getSizeForCompression(682 * 1024 * 1024) == 804520448); // 0x2aa00000
    CHECK(MemBuffer::getSizeForCompression(715827428) == UPX_RSIZE_MAX);     // 0x2aaaa8e4
    CHECK_THROWS(MemBuffer::getSizeForCompression(715827428 + 1));           // 0x2aaaa8e4 + 1
}

/* vim:set ts=4 sw=4 et: */

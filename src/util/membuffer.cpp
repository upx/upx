/* membuffer.cpp --

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

// A MemBuffer allocates memory on the heap, and automatically
// gets destructed when leaving scope or on exceptions.

#include "../conf.h"
#include "membuffer.h"

// extra functions to reduce dependency on membuffer.h
void *membuffer_get_void_ptr(MemBuffer &mb) noexcept { return mb.getVoidPtr(); }
const void *membuffer_get_void_ptr(const MemBuffer &mb) noexcept { return mb.getVoidPtr(); }
unsigned membuffer_get_size_in_bytes(const MemBuffer &mb) noexcept { return mb.getSizeInBytes(); }

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
#elif defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
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
static noinline bool use_simple_mcheck() noexcept {
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

MemBuffer::MemBuffer(upx_uint64_t bytes) may_throw : MemBufferBase<byte>() {
    static_assert(element_size == 1);
    alloc(bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

MemBuffer::~MemBuffer() noexcept { this->dealloc(); }

/*static*/
unsigned MemBuffer::getSizeForCompression(unsigned uncompressed_size, unsigned extra) may_throw {
    if (uncompressed_size == 0)
        throwCantPack("invalid uncompressed_size");
    const size_t z = uncompressed_size; // fewer keystrokes and display columns
    size_t bytes = mem_size(1, z);      // check size
    // All literal: 1 bit overhead per literal byte; from UCL documentation
    bytes = upx::umax(bytes, z + z / 8 + 256);
    // zstd: ZSTD_COMPRESSBOUND
    bytes = upx::umax(bytes, z + (z >> 8) + ((z < (128 << 10)) ? (((128 << 10) - z) >> 11) : 0));
    // add extra and 256 safety for various rounding/alignments
    bytes = mem_size(1, bytes, extra, 256);
    return ACC_ICONV(unsigned, bytes);
}

/*static*/
unsigned MemBuffer::getSizeForDecompression(unsigned uncompressed_size, unsigned extra) may_throw {
    if (uncompressed_size == 0)
        throwCantPack("invalid uncompressed_size");
    size_t bytes = mem_size(1, uncompressed_size, extra); // check size
    return ACC_ICONV(unsigned, bytes);
}

void MemBuffer::allocForCompression(unsigned uncompressed_size, unsigned extra) may_throw {
    unsigned bytes = getSizeForCompression(uncompressed_size, extra);
    alloc(bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

void MemBuffer::allocForDecompression(unsigned uncompressed_size, unsigned extra) may_throw {
    unsigned bytes = getSizeForDecompression(uncompressed_size, extra);
    alloc(bytes);
    debug_set(debug.last_return_address_alloc, upx_return_address());
}

void MemBuffer::fill(size_t off, size_t bytes, int value) may_throw {
    debug_set(debug.last_return_address_fill, upx_return_address());
    checkState();
    // check overrun and wrap-around
    if very_unlikely (off + bytes > size_in_bytes || off + bytes < off)
        throwCantPack("MemBuffer::fill out of range; take care!");
    if (bytes > 0)
        memset(ptr + off, value, bytes);
}

// similar to BoundedPtr, except checks only at creation
// skip == offset, take == size_in_bytes
void *MemBuffer::subref_impl(const char *errfmt, size_t skip, size_t take) may_throw {
    debug_set(debug.last_return_address_subref, upx_return_address());
    checkState();
    // check overrun and wrap-around
    if very_unlikely (skip + take > size_in_bytes || skip + take < skip) {
        char buf[100];
        // printf is using unsigned formatting
        if (!errfmt || !errfmt[0])
            errfmt = "bad subref %#x %#x";
        upx_safe_snprintf(buf, sizeof(buf), errfmt, (unsigned) skip, (unsigned) take);
        throwCantPack(buf);
    }
    return ptr + skip;
}

/*************************************************************************
//
**************************************************************************/

// for use_simple_mcheck()
#define PTR_BITS32(p) ((upx_uint32_t) (ptr_get_address(p) & 0xffffffff))
#define MAGIC1(p)     ((PTR_BITS32(p) ^ 0xfefdbeeb) | 1)
#define MAGIC2(p)     ((PTR_BITS32(p) ^ 0xfefdbeeb ^ 0x88224411) | 1)

void MemBuffer::checkState() const may_throw {
    if very_unlikely (ptr == nullptr)
        throwInternalError("block not allocated");
    assert(size_in_bytes > 0);
    if (use_simple_mcheck()) {
        const byte *p = upx::ptr_static_cast<const byte *>(ptr);
        if very_unlikely (get_ne32(p - 4) != MAGIC1(p))
            throwInternalError("memory clobbered before allocated block 1");
        if very_unlikely (get_ne32(p - 8) != size_in_bytes)
            throwInternalError("memory clobbered before allocated block 2");
        if very_unlikely (get_ne32(p + size_in_bytes) != MAGIC2(p))
            throwInternalError("memory clobbered past end of allocated block");
    }
}

void MemBuffer::alloc(const upx_uint64_t bytes) may_throw {
    // INFO: we don't automatically free a used buffer
    assert(ptr == nullptr);
    assert(size_in_bytes == 0);
    //
    assert(bytes > 0);
    debug_set(debug.last_return_address_alloc, upx_return_address());
    upx_rsize_t malloc_bytes = mem_size(1, bytes); // check size
    if (use_simple_mcheck())
        malloc_bytes += 32;
    else
        malloc_bytes += 4;
    byte *p = (byte *) ::malloc(malloc_bytes);
    NO_printf("MemBuffer::alloc %llu: %p\n", bytes, p);
    if (!p)
        throwOutOfMemoryException();
    size_in_bytes = size_type(bytes);
    if (use_simple_mcheck()) {
        p += 16;
        // store magic constants to detect buffer overruns
        set_ne32(p - 8, size_in_bytes);
        set_ne32(p - 4, MAGIC1(p));
        set_ne32(p + size_in_bytes + 0, MAGIC2(p));
        set_ne32(p + size_in_bytes + 4, stats.global_alloc_counter);
    }
    ptr = upx::ptr_static_cast<pointer>(p);
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
    if (ptr == nullptr) {
        assert_noexcept(size_in_bytes == 0);
        return;
    }
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
        byte *p = upx::ptr_static_cast<byte *>(ptr);
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
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("MemBuffer unused 1") {
    MemBuffer mb;
    (void) mb;
}

TEST_CASE("MemBuffer unused 2") {
    MemBuffer mb;
    CHECK(mb.raw_ptr() == nullptr);
    CHECK(mb.raw_size_in_bytes() == 0);
}

TEST_CASE("MemBuffer core") {
    constexpr size_t N = 64;
    MemBuffer mb;
    CHECK_THROWS(mb.checkState());
    CHECK_THROWS(mb.alloc(0x30000000 + 1));
    CHECK(raw_bytes(mb, 0) == nullptr);
    CHECK_THROWS(raw_bytes(mb, 1));
    CHECK_THROWS(mb.begin());
    CHECK_THROWS(mb.end());
    CHECK_THROWS(mb.cbegin());
    CHECK_THROWS(mb.cend());
    mb.alloc(N);
    mb.checkState();
    CHECK(mb.begin() == mb.cbegin());
    CHECK(mb.end() == mb.cend());
    CHECK(mb.begin() == &mb[0]);
    CHECK(mb.end() == &mb[0] + N);
    CHECK(mb.cbegin() == &mb[0]);
    CHECK(mb.cend() == &mb[0] + N);
    CHECK(raw_bytes(mb, N) != nullptr);
    CHECK(raw_bytes(mb, N) == mb.getVoidPtr());
    CHECK_THROWS(raw_bytes(mb, N + 1));
    CHECK_NOTHROW(mb + N);
    CHECK_THROWS(mb + (N + 1));
#if ALLOW_INT_PLUS_MEMBUFFER
    CHECK_NOTHROW(N + mb);
    CHECK_THROWS((N + 1) + mb);
#endif
    CHECK_NOTHROW(mb.subref("", 0, N));
    CHECK_NOTHROW(mb.subref("", N, 0));
    CHECK_THROWS(mb.subref("", 1, N));
    CHECK_THROWS(mb.subref("", N, 1));
    if (use_simple_mcheck()) {
        byte *p = upx::ptr_static_cast<byte *>(raw_bytes(mb, N));
        upx_uint32_t magic1 = get_ne32(p - 4);
        set_ne32(p - 4, magic1 ^ 1);
        CHECK_THROWS(mb.checkState());
        set_ne32(p - 4, magic1);
        mb.checkState();
    }
}

TEST_CASE("MemBuffer global overloads") {
    {
        MemBuffer mb1(1);
        MemBuffer mb4(4);
        mb1.clear();
        mb4.clear();
        CHECK(memcmp(mb1, "\x00", 1) == 0);
        CHECK_THROWS(memcmp(mb1, "\x00\x00", 2)); // NOLINT(bugprone-unused-return-value)
        CHECK_THROWS(memcmp("\x00\x00", mb1, 2)); // NOLINT(bugprone-unused-return-value)
        CHECK_THROWS(memcmp(mb1, mb4, 2));        // NOLINT(bugprone-unused-return-value)
        CHECK_THROWS(memcmp(mb4, mb1, 2));        // NOLINT(bugprone-unused-return-value)
        CHECK_NOTHROW(memset(mb1, 255, 1));
        CHECK_THROWS(memset(mb1, 254, 2));
        CHECK(mb1[0] == 255);
    }

    for (size_t i = 1; i <= 16; i++) {
        MemBuffer mb(i);
        mb.clear();

        if (i < 2) {
            CHECK_THROWS(get_ne16(mb));
            CHECK_THROWS(get_be16(mb));
            CHECK_THROWS(get_le16(mb));
            CHECK_THROWS(set_ne16(mb, 0));
            CHECK_THROWS(set_be16(mb, 0));
            CHECK_THROWS(set_le16(mb, 0));
        } else {
            CHECK_NOTHROW(get_ne16(mb));
            CHECK_NOTHROW(get_be16(mb));
            CHECK_NOTHROW(get_le16(mb));
            CHECK_NOTHROW(set_ne16(mb, 0));
            CHECK_NOTHROW(set_be16(mb, 0));
            CHECK_NOTHROW(set_le16(mb, 0));
        }
        if (i < 3) {
            CHECK_THROWS(get_ne24(mb));
            CHECK_THROWS(get_be24(mb));
            CHECK_THROWS(get_le24(mb));
            CHECK_THROWS(set_ne24(mb, 0));
            CHECK_THROWS(set_be24(mb, 0));
            CHECK_THROWS(set_le24(mb, 0));
        } else {
            CHECK_NOTHROW(get_ne24(mb));
            CHECK_NOTHROW(get_be24(mb));
            CHECK_NOTHROW(get_le24(mb));
            CHECK_NOTHROW(set_ne24(mb, 0));
            CHECK_NOTHROW(set_be24(mb, 0));
            CHECK_NOTHROW(set_le24(mb, 0));
        }
        if (i < 4) {
            CHECK_THROWS(get_ne32(mb));
            CHECK_THROWS(get_be32(mb));
            CHECK_THROWS(get_le32(mb));
            CHECK_THROWS(set_ne32(mb, 0));
            CHECK_THROWS(set_be32(mb, 0));
            CHECK_THROWS(set_le32(mb, 0));
        } else {
            CHECK_NOTHROW(get_ne32(mb));
            CHECK_NOTHROW(get_be32(mb));
            CHECK_NOTHROW(get_le32(mb));
            CHECK_NOTHROW(set_ne32(mb, 0));
            CHECK_NOTHROW(set_be32(mb, 0));
            CHECK_NOTHROW(set_le32(mb, 0));
        }
        if (i < 8) {
            CHECK_THROWS(get_ne64(mb));
            CHECK_THROWS(get_be64(mb));
            CHECK_THROWS(get_le64(mb));
            CHECK_THROWS(set_ne64(mb, 0));
            CHECK_THROWS(set_be64(mb, 0));
            CHECK_THROWS(set_le64(mb, 0));
        } else {
            CHECK_NOTHROW(get_ne64(mb));
            CHECK_NOTHROW(get_be64(mb));
            CHECK_NOTHROW(get_le64(mb));
            CHECK_NOTHROW(set_ne64(mb, 0));
            CHECK_NOTHROW(set_be64(mb, 0));
            CHECK_NOTHROW(set_le64(mb, 0));
        }

        CHECK_NOTHROW(mb.subref("", 0, 0));
        CHECK_NOTHROW(mb.subref("", 0, i));
        CHECK_NOTHROW(mb.subref("", i, 0));
        CHECK_NOTHROW(mb.subref("", i - 1, 1));
        CHECK_THROWS(mb.subref("", 0, i + 1));
        CHECK_THROWS(mb.subref("", i + 1, 0));
        CHECK_THROWS(mb.subref("", i, 1));
        CHECK_THROWS(mb.subref("", (size_t) -1, 0));
        CHECK_THROWS(mb.subref("", (size_t) -1, i));

#if DEBUG || !(ACC_CC_CLANG && __PPC64__ && ACC_ABI_BIG_ENDIAN) || 0
        // @COMPILER_BUG @CLANG_BUG
        if (i < 2) {
            CHECK_THROWS(mb.subref("", 0, sizeof(NE16)));
            CHECK_THROWS(mb.subref("", 0, sizeof(BE16)));
            CHECK_THROWS(mb.subref("", 0, sizeof(LE16)));
        } else {
            CHECK_NOTHROW(mb.subref("", 0, sizeof(NE16)));
            CHECK_NOTHROW(mb.subref("", 0, sizeof(BE16)));
            CHECK_NOTHROW(mb.subref("", 0, sizeof(LE16)));
        }
        if (i < 4) {
            CHECK_THROWS(mb.subref("", 0, sizeof(NE32)));
            CHECK_THROWS(mb.subref("", 0, sizeof(BE32)));
            CHECK_THROWS(mb.subref("", 0, sizeof(LE32)));
        } else {
            CHECK_NOTHROW(mb.subref("", 0, sizeof(NE32)));
            CHECK_NOTHROW(mb.subref("", 0, sizeof(BE32)));
            CHECK_NOTHROW(mb.subref("", 0, sizeof(LE32)));
        }
        if (i < 8) {
            CHECK_THROWS(mb.subref("", 0, sizeof(NE64)));
            CHECK_THROWS(mb.subref("", 0, sizeof(BE64)));
            CHECK_THROWS(mb.subref("", 0, sizeof(LE64)));
        } else {
            CHECK_NOTHROW(mb.subref("", 0, sizeof(NE64)));
            CHECK_NOTHROW(mb.subref("", 0, sizeof(BE64)));
            CHECK_NOTHROW(mb.subref("", 0, sizeof(LE64)));
        }

        CHECK_NOTHROW(mb.subref_u<byte *>("", 0));
        CHECK_NOTHROW(mb.subref_u<byte *>("", i - 1));
        CHECK_THROWS(mb.subref_u<byte *>("", i));
        CHECK_THROWS(mb.subref_u<byte *>("", (size_t) -1));

        if (i < 2) {
            CHECK_THROWS(mb.subref_u<NE16 *>("", 0));
            CHECK_THROWS(mb.subref_u<BE16 *>("", 0));
            CHECK_THROWS(mb.subref_u<LE16 *>("", 0));
        } else {
            CHECK_NOTHROW(mb.subref_u<NE16 *>("", 0));
            CHECK_NOTHROW(mb.subref_u<BE16 *>("", 0));
            CHECK_NOTHROW(mb.subref_u<LE16 *>("", 0));
        }
        if (i < 4) {
            CHECK_THROWS(mb.subref_u<NE32 *>("", 0));
            CHECK_THROWS(mb.subref_u<BE32 *>("", 0));
            CHECK_THROWS(mb.subref_u<LE32 *>("", 0));
        } else {
            CHECK_NOTHROW(mb.subref_u<NE32 *>("", 0));
            CHECK_NOTHROW(mb.subref_u<BE32 *>("", 0));
            CHECK_NOTHROW(mb.subref_u<LE32 *>("", 0));
        }
        if (i < 8) {
            CHECK_THROWS(mb.subref_u<NE64 *>("", 0));
            CHECK_THROWS(mb.subref_u<BE64 *>("", 0));
            CHECK_THROWS(mb.subref_u<LE64 *>("", 0));
        } else {
            CHECK_NOTHROW(mb.subref_u<NE64 *>("", 0));
            CHECK_NOTHROW(mb.subref_u<BE64 *>("", 0));
            CHECK_NOTHROW(mb.subref_u<LE64 *>("", 0));
        }
#endif
    }
}

TEST_CASE("MemBuffer array access") {
    constexpr size_t N = 16;
    MemBuffer mb(N);
    mb.clear();
    for (size_t i = 0; i != N; ++i)
        mb[i] += 1;
    for (byte *ptr = mb; ptr != mb + N; ++ptr)
        *ptr += 1;
    for (byte *ptr = mb + 0; ptr < mb + N; ++ptr)
        *ptr += 1;
    for (byte *ptr = &mb[0]; ptr != mb.end(); ++ptr)
        *ptr += 1;
    for (byte *ptr = mb.begin(); ptr < mb.end(); ++ptr)
        *ptr += 1;
    for (size_t i = 0; i != N; ++i)
        assert(mb[i] == 5);
    CHECK_NOTHROW((void) &mb[N - 1]);
    CHECK_THROWS((void) &mb[N]); // NOT legal for containers like std::vector or MemBuffer
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

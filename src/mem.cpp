/* mem.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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


#include "conf.h"
#include "mem.h"


/*************************************************************************
// assert sane memory buffer sizes to protect against integer overflows
// and malicious header fields
**************************************************************************/

// DO NOT CHANGE
#define MAX_BUF_SIZE (768 * 1024 * 1024)
ACC_COMPILE_TIME_ASSERT_HEADER(2ull * MAX_BUF_SIZE * 9 / 8 + 16*1024*1024 < INT_MAX)

size_t mem_size(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra)
{
    assert(element_size > 0);
    if (element_size > MAX_BUF_SIZE) throwCantPack("mem_size 1; take care");
    if (n > MAX_BUF_SIZE) throwCantPack("mem_size 2; take care");
    if (extra > MAX_BUF_SIZE) throwCantPack("mem_size 3; take care");
    upx_uint64_t bytes = element_size * n + extra; // cannot overflow
    if (bytes > MAX_BUF_SIZE) throwCantPack("mem_size 4; take care");
    return ACC_ICONV(size_t, bytes);
}

size_t mem_size_get_n(upx_uint64_t element_size, upx_uint64_t n)
{
    (void) mem_size(element_size, n);   // check
    return ACC_ICONV(size_t, n);        // return n
}

bool mem_size_valid(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra)
{
    assert(element_size > 0);
    if (element_size > MAX_BUF_SIZE) return false;
    if (n > MAX_BUF_SIZE) return false;
    if (extra > MAX_BUF_SIZE) return false;
    upx_uint64_t bytes = element_size * n + extra; // cannot overflow
    if (bytes > MAX_BUF_SIZE) return false;
    return true;
}

bool mem_size_valid_bytes(upx_uint64_t bytes)
{
    if (bytes > MAX_BUF_SIZE) return false;
    return true;
}


int ptr_diff(const char *p1, const char *p2)
{
    assert(p1 != NULL);
    assert(p2 != NULL);
    ptrdiff_t d = p1 - p2;
    if (p1 >= p2)
        assert(mem_size_valid_bytes(d));
    else
        assert(mem_size_valid_bytes(-d));
    return ACC_ICONV(int, d);
}

#undef MAX_BUF_SIZE


/*************************************************************************
// bool use_mcheck()
**************************************************************************/

#if defined(__SANITIZE_ADDRESS__)
__acc_static_forceinline bool use_mcheck() { return false; }
#elif (WITH_VALGRIND) && defined(RUNNING_ON_VALGRIND)
static int use_mcheck_flag = -1;
__acc_static_noinline void use_mcheck_init()
{
    use_mcheck_flag = 1;
    if (RUNNING_ON_VALGRIND) {
        use_mcheck_flag = 0;
        //fprintf(stderr, "upx: detected RUNNING_ON_VALGRIND\n");
    }
}
__acc_static_forceinline bool use_mcheck()
{
    if __acc_unlikely(use_mcheck_flag < 0)
        use_mcheck_init();
    return (bool) use_mcheck_flag;
}
#else
__acc_static_forceinline bool use_mcheck() { return true; }
#endif


/*************************************************************************
//
**************************************************************************/

MemBuffer::MemBuffer(upx_uint64_t size) :
    b(NULL), b_size(0)
{
    alloc(size);
}


MemBuffer::~MemBuffer()
{
    this->dealloc();
}

void MemBuffer::dealloc()
{
    if (b != NULL)
    {
        checkState();
        if (use_mcheck())
        {
            // remove magic constants
            set_be32(b - 8, 0);
            set_be32(b - 4, 0);
            set_be32(b + b_size, 0);
            set_be32(b + b_size + 4, 0);
            //
            ::free(b - 16);
        }
        else
            ::free(b);
        b = NULL;
        b_size = 0;
    }
    else
        assert(b_size == 0);
}


unsigned MemBuffer::getSizeForCompression(unsigned uncompressed_size, unsigned extra)
{
    size_t bytes = mem_size(1, uncompressed_size, extra);
    bytes += uncompressed_size/8 + 256;
    return ACC_ICONV(unsigned, bytes);
}

unsigned MemBuffer::getSizeForUncompression(unsigned uncompressed_size, unsigned extra)
{
    size_t bytes = mem_size(1, uncompressed_size, extra);
    // INFO: 3 bytes are the allowed overrun for the i386 asm_fast decompressors
#if (ACC_ARCH_I386)
    bytes += 3;
#endif
    return ACC_ICONV(unsigned, bytes);
}


void MemBuffer::allocForCompression(unsigned uncompressed_size, unsigned extra)
{
    unsigned size = getSizeForCompression(uncompressed_size, extra);
    alloc(size);
}


void MemBuffer::allocForUncompression(unsigned uncompressed_size, unsigned extra)
{
    unsigned size = getSizeForUncompression(uncompressed_size, extra);
    alloc(size);
}


void MemBuffer::fill(unsigned off, unsigned len, int value)
{
    checkState();
    assert((int)off >= 0);
    assert((int)len >= 0);
    assert(off <= b_size);
    assert(len <= b_size);
    assert(off + len <= b_size);
    if (len > 0)
        memset(b + off, value, len);
}


/*************************************************************************
//
**************************************************************************/

#define PTR(p)      ((unsigned) ((upx_uintptr_t)(p) & 0xffffffff))
#define MAGIC1(p)   (PTR(p) ^ 0xfefdbeeb)
#define MAGIC2(p)   (PTR(p) ^ 0xfefdbeeb ^ 0x80024001)

unsigned MemBuffer::global_alloc_counter = 0;


void MemBuffer::checkState() const
{
    if (!b)
        throwInternalError("block not allocated");
    if (use_mcheck())
    {
        if (get_be32(b - 4) != MAGIC1(b))
            throwInternalError("memory clobbered before allocated block 1");
        if (get_be32(b - 8) != b_size)
            throwInternalError("memory clobbered before allocated block 2");
        if (get_be32(b + b_size) != MAGIC2(b))
            throwInternalError("memory clobbered past end of allocated block");
    }
    assert((int)b_size > 0);
}


void MemBuffer::alloc(upx_uint64_t size)
{
    // NOTE: we don't automatically free a used buffer
    assert(b == NULL);
    assert(b_size == 0);
    //
    assert(size > 0);
    size_t bytes = mem_size(1, size, use_mcheck() ? 32 : 0);
    unsigned char *p = (unsigned char *) malloc(bytes);
    if (!p)
        throwOutOfMemoryException();
    b_size = ACC_ICONV(unsigned, size);
    if (use_mcheck())
    {
        b = p + 16;
        // store magic constants to detect buffer overruns
        set_be32(b - 8, b_size);
        set_be32(b - 4, MAGIC1(b));
        set_be32(b + b_size, MAGIC2(b));
        set_be32(b + b_size + 4, global_alloc_counter++);
    }
    else
        b = p ;

    //fill(0, b_size, (rand() & 0xff) | 1); // debug
}

/* vim:set ts=4 sw=4 et: */

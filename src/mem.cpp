/* mem.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2020 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2020 Laszlo Molnar
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
// bool use_simple_mcheck()
**************************************************************************/

#if defined(__SANITIZE_ADDRESS__)
__acc_static_forceinline bool use_simple_mcheck() { return false; }
#elif (WITH_VALGRIND) && defined(RUNNING_ON_VALGRIND)
static int use_simple_mcheck_flag = -1;
__acc_static_noinline void use_simple_mcheck_init()
{
    use_simple_mcheck_flag = 1;
    if (RUNNING_ON_VALGRIND) {
        use_simple_mcheck_flag = 0;
        //fprintf(stderr, "upx: detected RUNNING_ON_VALGRIND\n");
    }
}
__acc_static_forceinline bool use_simple_mcheck()
{
    if __acc_unlikely(use_simple_mcheck_flag < 0)
        use_simple_mcheck_init();
    return (bool) use_simple_mcheck_flag;
}
#else
__acc_static_forceinline bool use_simple_mcheck() { return true; }
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

// similar to BoundedPtr, except checks only at creation
unsigned char *MemBuffer::subref(char const *errfmt, unsigned skip, unsigned take)
{
    if ((take + skip) < take  // wrap-around
    ||  (take + skip) > b_size  // overrun
    ) {
        char buf[100]; snprintf(buf, sizeof(buf), errfmt, skip, take);
        throwCantPack(buf);
    }
    return &b[skip];
}

void MemBuffer::dealloc()
{
    if (b != NULL)
    {
        checkState();
        if (use_simple_mcheck())
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
    if (use_simple_mcheck())
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
    size_t bytes = mem_size(1, size, use_simple_mcheck() ? 32 : 0);
    unsigned char *p = (unsigned char *) malloc(bytes);
    if (!p)
        throwOutOfMemoryException();
    b_size = ACC_ICONV(unsigned, size);
    if (use_simple_mcheck())
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

/* mem.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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


#include "conf.h"
#include "mem.h"


/*************************************************************************
//
**************************************************************************/

static int use_mcheck = -1;

static int mcheck_init()
{
    if (use_mcheck < 0)
    {
        use_mcheck = 1;
#if defined(WITH_VALGRIND) && defined(RUNNING_ON_VALGRIND)
        if (RUNNING_ON_VALGRIND)
        {
            //fprintf(stderr, "upx: detected RUNNING_ON_VALGRIND\n");
            use_mcheck = 0;
        }
#endif
    }
    return use_mcheck;
}


/*************************************************************************
//
**************************************************************************/

MemBuffer::MemBuffer() :
    b(NULL), b_size(0)
{
}

MemBuffer::MemBuffer(unsigned size) :
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
    if (b)
    {
        checkState();
        if (use_mcheck)
        {
            // remove magic constants
            set_be32(b - 8, 0);
            set_be32(b - 4, 0);
            set_be32(b + b_size, 0);
            set_be32(b + b_size + 4, 0);
            //
            ::free(b - 8);
        }
        else
            ::free(b);
        b = NULL;
        b_size = 0;
    }
    else
        assert(b_size == 0);
}


void MemBuffer::allocForCompression(unsigned uncompressed_size, unsigned extra)
{
    assert((int)uncompressed_size > 0);
    assert((int)extra >= 0);
    unsigned size = uncompressed_size + uncompressed_size/8 + 256 + extra;
    alloc(size);
}


void MemBuffer::allocForUncompression(unsigned uncompressed_size, unsigned extra)
{
    assert((int)uncompressed_size > 0);
    assert((int)extra >= 0);
    unsigned size = uncompressed_size + extra;
//    size += 512;   // 512 safety bytes
    // INFO: 3 bytes are the allowed overrun for the i386 asm_fast decompressors
#if (ACC_ARCH_IA32)
    size += 3;
#endif
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

#define PTR(p)      ((unsigned) ((acc_uintptr_t)(p) & 0xffffffff))
#define MAGIC1(p)   (PTR(p) ^ 0xfefdbeeb)
#define MAGIC2(p)   (PTR(p) ^ 0xfefdbeeb ^ 0x80024001)

unsigned MemBuffer::global_alloc_counter = 0;


void MemBuffer::checkState() const
{
    if (!b)
        throwInternalError("block not allocated");
    if (use_mcheck)
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


void MemBuffer::alloc(unsigned size)
{
    if (use_mcheck < 0)
        mcheck_init();

    // NOTE: we don't automatically free a used buffer
    assert(b == NULL);
    assert(b_size == 0);
    //
    assert((int)size > 0);
    unsigned total = use_mcheck ? size + 16 : size;
    assert((int)total > 0);
    unsigned char *p = (unsigned char *) malloc(total);
    if (!p)
    {
        //throw bad_alloc();
        throw OutOfMemoryException("out of memory");
        //exit(1);
    }
    b_size = size;
    if (use_mcheck)
    {
        b = p + 8;
        // store magic constants to detect buffer overruns
        set_be32(b - 8, b_size);
        set_be32(b - 4, MAGIC1(b));
        set_be32(b + b_size, MAGIC2(b));
        set_be32(b + b_size + 4, global_alloc_counter++);
    }
    else
        b = p ;
}


/*
vi:ts=4:et
*/


/* mem.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2003 Laszlo Molnar
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
    ptr(NULL), psize(0)
{
}

MemBuffer::MemBuffer(unsigned size) :
    ptr(NULL), psize(0)
{
    alloc(size);
}


MemBuffer::~MemBuffer()
{
    this->dealloc();
}

void MemBuffer::dealloc()
{
    if (ptr)
    {
        checkState();
        if (use_mcheck)
        {
            // clear magic constants
            set_be32(ptr - 8, 0);
            set_be32(ptr - 4, 0);
            set_be32(ptr + psize, 0);
            set_be32(ptr + psize + 4, 0);
            //
            ::free(ptr - 8);
        }
        else
            ::free(ptr);
        ptr = NULL;
        psize = 0;
    }
    else
        assert(psize == 0);
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
#if defined(__i386__)
    size += 3;
#endif
    alloc(size);
}


/*************************************************************************
//
**************************************************************************/

#define PTR(p)      ((unsigned)(p) & 0xffffffff)
#define MAGIC1(p)   (PTR(p) ^ 0xfefdbeeb)
#define MAGIC2(p)   (PTR(p) ^ 0xfefdbeeb ^ 0x80024001)

unsigned MemBuffer::global_alloc_counter = 0;


void MemBuffer::checkState() const
{
    if (!ptr)
        throwInternalError("block not allocated");
    if (use_mcheck)
    {
        if (get_be32(ptr - 4) != MAGIC1(ptr))
            throwInternalError("memory clobbered before allocated block 1");
        if (get_be32(ptr - 8) != psize)
            throwInternalError("memory clobbered before allocated block 2");
        if (get_be32(ptr + psize) != MAGIC2(ptr))
            throwInternalError("memory clobbered past end of allocated block");
    }
    assert((int)psize > 0);
}


void MemBuffer::alloc(unsigned size)
{
    if (use_mcheck < 0)
        mcheck_init();

    // NOTE: we don't automatically free a used buffer
    assert(ptr == NULL);
    assert(psize == 0);
    //
    assert((int)size > 0);
    unsigned total = use_mcheck ? size + 16 : size;
    assert((int)total > 0);
    unsigned char *p = (unsigned char *) malloc(total);
    if (!p)
    {
        //throw bad_alloc();
        throwCantPack("out of memory");
        //exit(1);
    }
    psize = size;
    if (use_mcheck)
    {
        ptr = p + 8;
        // store magic constants to detect buffer overruns
        set_be32(ptr - 8, psize);
        set_be32(ptr - 4, MAGIC1(ptr));
        set_be32(ptr + psize, MAGIC2(ptr));
        set_be32(ptr + psize + 4, global_alloc_counter++);
    }
    else
        ptr = p ;
}


/*
vi:ts=4:et
*/


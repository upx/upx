/* mem.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "mem.h"


/*************************************************************************
//
**************************************************************************/

MemBuffer::MemBuffer(unsigned size) :
    ptr(NULL), alloc_ptr(NULL), alloc_size(0)
{
    if (size > 0)
        alloc(size, 0);
}


MemBuffer::~MemBuffer()
{
    this->free();
}


void MemBuffer::free()
{
    if (alloc_ptr)
        ::free(alloc_ptr);
    alloc_ptr = ptr = NULL;
    alloc_size = 0;
}


unsigned MemBuffer::getSize() const
{
    if (!alloc_ptr)
        return 0;
    unsigned size = alloc_size - (ptr - alloc_ptr);
    assert((int)size > 0);
    return size;
}


void MemBuffer::alloc(unsigned size, unsigned base_offset)
{
#if 0
    // don't automaticlly free a used buffer
    this->free();
#endif
    assert(alloc_ptr == NULL);
    assert((int)size > 0);
    size = base_offset + size;
    alloc_ptr = (unsigned char *) malloc(size);
    if (!alloc_ptr)
    {
        throwCantPack("out of memory");
        //exit(1);
    }
    alloc_size = size;
    ptr = alloc_ptr + base_offset;
}


void MemBuffer::alloc(unsigned size)
{
    alloc(size, 0);
}


void MemBuffer::allocForCompression(unsigned uncompressed_size)
{
    // Idea:
    //   We allocate the buffer at an offset of 4096 so
    //   that we could do an in-place decompression for
    //   verifying our overlap_overhead at the end
    //   of packing.
    //
    // See Packer::verifyOverlappingDecompression().

    alloc(uncompressed_size + uncompressed_size/8 + 256, MAX_OVERLAP_OVERHEAD);
}


void MemBuffer::allocForUncompression(unsigned uncompressed_size)
{
    //alloc(uncompressed_size + 512, 0);  // 512 safety bytes
    alloc(uncompressed_size + 3, 0);  // 3 safety bytes for asm_fast
}


/*
vi:ts=4:et
*/


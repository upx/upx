/* mem.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar

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


#ifndef __UPX_MEM_H
#define __UPX_MEM_H


/*************************************************************************
//
**************************************************************************/

class MemBuffer
{
public:
    MemBuffer(unsigned size=0);
    ~MemBuffer();

    void alloc(unsigned size);
    void allocForCompression(unsigned uncompressed_size);
    void allocForUncompression(unsigned uncompressed_size);

    void free();

    const unsigned char *getBuf()     const { return ptr; }
    unsigned getSize() const ;

    operator       unsigned char * ()       { return ptr; }
    //operator const unsigned char * () const { return ptr; }

    enum { MAX_OVERLAP_OVERHEAD = 4096 };

private:
    void alloc(unsigned size, unsigned base_offset);

    unsigned char *ptr;
    unsigned char *alloc_ptr;
    unsigned alloc_size;

private:
    // disable copy and assignment
    MemBuffer(MemBuffer const &); // {}
    MemBuffer& operator= (MemBuffer const &); // { return *this; }

    // disable dynamic allocation
    static void *operator new (size_t); // {}
    static void *operator new[] (size_t); // {}
    //static void operator delete (void *) {}
    //static void operator delete[] (void *) {}
};


#endif /* already included */


/*
vi:ts=4:et
*/


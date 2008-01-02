/* mem.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_MEM_H
#define __UPX_MEM_H


/*************************************************************************
//
**************************************************************************/

class MemBuffer
{
public:
    MemBuffer();
    explicit MemBuffer(unsigned size);
    ~MemBuffer();

    static unsigned getSizeForCompression(unsigned uncompressed_size, unsigned extra=0);
    static unsigned getSizeForUncompression(unsigned uncompressed_size, unsigned extra=0);

    void alloc(unsigned size);
    void allocForCompression(unsigned uncompressed_size, unsigned extra=0);
    void allocForUncompression(unsigned uncompressed_size, unsigned extra=0);

    void dealloc();

    void checkState() const;

    unsigned getSize() const { return b_size; }

    operator       unsigned char * ()       { return b; }
    //operator const unsigned char * () const { return b; }
          void *getVoidPtr()                { return (void *) b; }
    const void *getVoidPtr() const          { return (const void *) b; }

    void fill(unsigned off, unsigned len, int value);
    void clear(unsigned off, unsigned len)  { fill(off, len, 0); }
    void clear()                            { fill(0, b_size, 0); }

private:
    unsigned char *b;
    unsigned b_size;

    static unsigned global_alloc_counter;

    // disable copy and assignment
    MemBuffer(const MemBuffer &); // {}
    MemBuffer& operator= (const MemBuffer &); // { return *this; }

    // disable dynamic allocation
    DISABLE_NEW_DELETE
};

#endif /* already included */


/*
vi:ts=4:et
*/


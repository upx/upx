/* p_vmlinux.cpp --

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

#include "file.h"
#include "packer.h"
#include "p_unix.h"
#include "p_elf.h"
#include <zlib.h>


/*************************************************************************
//
**************************************************************************/

PackBvmlinuxI386::PackBvmlinuxI386(InputFile *f) :
    super(f), elf_offset(0)
{
}

int PackBvmlinuxI386::getCompressionMethod() const
{
    return M_NRV2D_LE32;
}


bool PackBvmlinuxI386::canPack()
{
    Elf_LE32_Ehdr ehdr;
    Elf_LE32_Phdr text;
    Elf_LE32_Phdr data;
    unsigned char *buf = ehdr.e_ident;

    fi->seek(elf_offset, SEEK_SET);
    fi->readx(&ehdr, sizeof(ehdr));
    fi->readx(&text, sizeof(text));
    fi->readx(&data, sizeof(data));

    // check the ELF header
    if (memcmp(buf, "\x7f\x45\x4c\x46\x01\x01\x01", 7)) // ELF 32-bit LSB
        return false;
    if (!memcmp(buf+8, "FreeBSD", 7))               // branded
        return false;
    if (ehdr.e_type != 2)                           // executable
        return false;
    if (ehdr.e_machine != 3 && ehdr.e_machine != 6) // Intel 80[34]86
        return false;
    if (ehdr.e_version != 1)                        // version
        return false;

    // now check for bvmlinux
    if (ehdr.e_phoff != 52 || ehdr.e_ehsize != 52 || ehdr.e_phentsize != 32)
        return false;
    if (ehdr.e_entry != 0x100000 || ehdr.e_phnum != 2)
        return false;

    // check for bvmlinux - text segment
    if (text.p_type != 1 || text.p_offset != 0x1000)
        return false;
    if (text.p_vaddr != 0x100000 || text.p_paddr != 0x100000)
        return false;
    if (text.p_flags != 5)
        return false;

    // check for bvmlinux - data segment
    if (data.p_type != 1)
        return false;
    if (data.p_filesz < 200000)
        return false;
    if (data.p_flags != 6)
        return false;

    // check gzip data
    unsigned char magic[2];
    LE32 uncompressed_size;
    off_t gzip_offset = elf_offset + data.p_offset + 472;
    fi->seek(gzip_offset, SEEK_SET);
    fi->readx(magic, 2);
    if (memcmp(magic, "\037\213", 2))
        return false;
    fi->seek(data.p_filesz - 2 - 4 - 472, SEEK_CUR);
    fi->readx(&uncompressed_size, 4);
    if (uncompressed_size < data.p_filesz || uncompressed_size > 4*1024*1024)
        return false;

    // uncompress kernel with zlib
    fi->seek(gzip_offset, SEEK_SET);
    gzFile f = gzdopen(fi->getFd(), "r");
    if (f == NULL)
        return false;
    ibuf.alloc(uncompressed_size);
    unsigned ilen = gzread(f, ibuf, uncompressed_size);
    bool ok = ilen == uncompressed_size;
    gzclose(f);

    printf("decompressed kernel: %d -> %d\n", (int)data.p_filesz, ilen);
    return ok;
}


/*************************************************************************
//
**************************************************************************/

void PackBvmlinuxI386::pack(OutputFile *fo)
{
    fo = fo;
}


/*************************************************************************
//
**************************************************************************/

int PackBvmlinuxI386::canUnpack()
{
    return false;
}

void PackBvmlinuxI386::unpack(OutputFile *fo)
{
    fo = fo;
}


/*
vi:ts=4:et
*/


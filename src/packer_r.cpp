/* packer_r.cpp -- Packer relocation handling

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

#include "conf.h"
#include "packer.h"

/*************************************************************************
// sort and delta-compress relocations with optional bswap within image
// returns number of bytes written to 'out'
**************************************************************************/

/*static*/
unsigned Packer::optimizeReloc(unsigned relocnum, SPAN_P(byte) relocs, SPAN_S(byte) out,
                               SPAN_P(byte) image, unsigned image_size, int bits, bool bswap,
                               int *big) {
    assert(bits == 32 || bits == 64);
    mem_size_assert(1, image_size);
#if WITH_XSPAN >= 2
    ptr_check_no_overlap(relocs.data(), relocs.size_bytes(), image.data(image_size), image_size,
                         out.data(), out.size_bytes());
#endif
    SPAN_S_VAR(byte, fix, out);

    *big = 0;
    if (opt->exact)
        throwCantPackExact();
    if (relocnum == 0)
        return 0;
    upx_qsort(raw_bytes(relocs, 4 * relocnum), relocnum, 4, le32_compare);
    if (0) {
        printf("optimizeReloc: u_reloc %9u checksum=0x%08x\n", 4 * relocnum,
               upx_adler32(relocs, 4 * relocnum));
        printf("optimizeReloc: u_image %9u checksum=0x%08x\n", image_size,
               upx_adler32(image, image_size));
    }

    unsigned pc = (unsigned) -4;
    for (unsigned i = 0; i < relocnum; i++) {
        unsigned delta = get_le32(relocs + i * 4) - pc;
        if (delta == 0)
            continue;
        else if ((int) delta < 4)
            throwCantPack("overlapping fixups");
        else if (delta < 0xf0)
            *fix++ = (byte) delta;
        else if (delta < 0x100000) {
            *fix++ = (byte) (0xf0 + (delta >> 16));
            *fix++ = (byte) delta;
            *fix++ = (byte) (delta >> 8);
        } else {
            *big = 1;
            *fix++ = 0xf0;
            *fix++ = 0;
            *fix++ = 0;
            set_le32(fix, delta);
            fix += 4;
        }
        pc += delta;
        if (pc + 4 > image_size)
            throwCantPack("bad reloc[%#x] = %#x", i, pc);
        if (bswap) {
            if (bits == 32)
                set_be32(image + pc, get_le32(image + pc));
            else
                set_be64(image + pc, get_le64(image + pc));
        }
    }
    *fix++ = 0; // end marker
    const unsigned bytes = ptr_udiff_bytes(fix, out);
    if (0) {
        printf("optimizeReloc: c_reloc %9u checksum=0x%08x\n", bytes, upx_adler32(out, bytes));
        printf("optimizeReloc: c_image %9u checksum=0x%08x\n", image_size,
               upx_adler32(image, image_size));
    }
    return bytes;
}

/*************************************************************************
// delta-decompress relocations
// advances 'in'
// allocates 'out' and returns number of relocs written to 'out'
**************************************************************************/

/*static*/
unsigned Packer::unoptimizeReloc(SPAN_S(const byte) & in, MemBuffer &out, SPAN_P(byte) image,
                                 unsigned image_size, int bits, bool bswap) {
    assert(bits == 32 || bits == 64);
    mem_size_assert(1, image_size);
#if WITH_XSPAN >= 2
    ptr_check_no_overlap(in.data(), in.size_bytes(), image.data(image_size), image_size);
#endif
    SPAN_S_VAR(const byte, fix, in);

    // count
    unsigned relocnum = 0;
    for (fix = in; *fix; fix++, relocnum++) {
        if (*fix >= 0xf0) {
            if (*fix == 0xf0 && get_le16(fix + 1) == 0)
                fix += 4;
            fix += 2;
        }
    }
    NO_fprintf(stderr, "relocnum=%x\n", relocnum);
    if (0) {
        const unsigned bytes = ptr_udiff_bytes(fix + 1, in);
        printf("unoptimizeReloc: c_reloc %9u checksum=0x%08x\n", bytes, upx_adler32(in, bytes));
        printf("unoptimizeReloc: c_image %9u checksum=0x%08x\n", image_size,
               upx_adler32(image, image_size));
    }

    out.alloc(4 * (relocnum + 1)); // one extra entry
    SPAN_S_VAR(LE32, relocs, out);

    fix = in;
    unsigned pc = (unsigned) -4;
    for (unsigned i = 0; i < relocnum; i++) {
        unsigned delta;
        if (*fix < 0xf0)
            delta = *fix++;
        else {
            delta = (*fix & 0x0f) * 0x10000 + get_le16(fix + 1);
            fix += 3;
            if (delta == 0) {
                delta = get_le32(fix);
                fix += 4;
            }
        }
        if ((int) delta < 4)
            throwCantUnpack("overlapping fixups");
        pc += delta;
        if (pc + 4 > image_size)
            throwCantUnpack("bad reloc[%#x] = %#x", i, pc);
        *relocs++ = pc;
        if (bswap) {
            if (bits == 32)
                set_be32(image + pc, get_le32(image + pc));
            else
                set_be64(image + pc, get_le64(image + pc));
        }
    }
    in = fix + 1; // advance
    assert(relocnum == ptr_udiff_bytes(relocs, raw_bytes(out, 0)) / 4);
    if (0) {
        printf("unoptimizeReloc: u_reloc %9u checksum=0x%08x\n", 4 * relocnum,
               upx_adler32(out, 4 * relocnum));
        printf("unoptimizeReloc: u_image %9u checksum=0x%08x\n", image_size,
               upx_adler32(image, image_size));
    }
    return relocnum;
}

/* vim:set ts=4 sw=4 et: */

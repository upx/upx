/* p_tos.cpp --

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
#include "p_tos.h"

static const
#include "stub/l_t_n2b.h"
static const
#include "stub/l_t_n2bs.h"
static const
#include "stub/l_t_n2d.h"
static const
#include "stub/l_t_n2ds.h"

// #define TESTING


/*************************************************************************
//
**************************************************************************/

#define FH_SIZE         sizeof(tos_header_t)

PackTos::PackTos(InputFile *f) :
    super(f)
{
    COMPILE_TIME_ASSERT(FH_SIZE == 28);
}


int PackTos::getCompressionMethod() const
{
    if (M_IS_NRV2B(opt->method))
        return M_NRV2B_8;
    if (M_IS_NRV2D(opt->method))
        return M_NRV2D_8;
    return opt->level > 1 && file_size >= 512*1024 ? M_NRV2D_8 : M_NRV2B_8;
}


const int *PackTos::getFilters() const
{
    return NULL;
}


const upx_byte *PackTos::getLoader() const
{
    if (M_IS_NRV2B(opt->method))
        return opt->small ? nrv2b_loader_small : nrv2b_loader;
    if (M_IS_NRV2D(opt->method))
        return opt->small ? nrv2d_loader_small : nrv2d_loader;
    return NULL;
}


int PackTos::getLoaderSize() const
{
    if (M_IS_NRV2B(opt->method))
        return opt->small ? sizeof(nrv2b_loader_small) : sizeof(nrv2b_loader);
    if (M_IS_NRV2D(opt->method))
        return opt->small ? sizeof(nrv2d_loader_small) : sizeof(nrv2d_loader);
    return 0;
}


/*************************************************************************
//
**************************************************************************/

/* flags for curproc->memflags */
/* also used for program headers fh_flag */
#define F_FASTLOAD      0x01        // don't zero heap
#define F_ALTLOAD       0x02        // OK to load in alternate ram
#define F_ALTALLOC      0x04        // OK to malloc from alt. ram
#define F_SMALLTPA      0x08        // used in MagiC: TPA can be allocated
                                    // as specified in the program header
                                    // rather than the biggest free memory
                                    // block
#define F_MEMFLAGS      0xf0        // reserved for future use
#define F_SHTEXT        0x800       // program's text may be shared

#define F_MINALT        0xf0000000  // used to decide which type of RAM to load in

#define F_ALLOCZERO     0x2000      // zero mem, for bugged (GEM...) programs

/* Bit in Mxalloc's arg for "don't auto-free this memory" */
#define F_KEEP          0x4000

#define F_OS_SPECIAL    0x8000      // mark as a special process

/* flags for curproc->memflags (that is, fh_flag) and also Mxalloc mode.  */
/* (Actually, when users call Mxalloc, they add 0x10 to what you see here) */
#define F_PROTMODE      0xf0        // protection mode bits
#define F_PROT_P        0x00        // no read or write
#define F_PROT_G        0x10        // any access OK
#define F_PROT_S        0x20        // any super access OK
#define F_PROT_PR       0x30        // any read OK, no write
#define F_PROT_I        0x40        // invalid page


/*************************************************************************
// util
**************************************************************************/

bool PackTos::readFileHeader()
{
    fi->seek(0,SEEK_SET);
    fi->readx(&ih, FH_SIZE);
    if (ih.fh_magic != 0x601a)
        return false;
    if (FH_SIZE + ih.fh_text + ih.fh_data + ih.fh_sym > (unsigned) file_size)
        return false;
    return true;
}


bool PackTos::checkFileHeader()
{
    const unsigned f = ih.fh_flag;
    //printf("flags: 0x%x, text: %d, data: %d, bss: %d, sym: %d\n", f, (int)ih.fh_text, (int)ih.fh_data, (int)ih.fh_bss, (int)ih.fh_sym);
    if ((ih.fh_text & 1) || (ih.fh_data & 1))
        throwCantPack("odd size values in text/data");
    if (f & F_OS_SPECIAL)
        throwCantPack("I won't pack F_OS_SPECIAL programs");
    if ((f & F_PROTMODE) > F_PROT_I)
        throwCantPack("invalid protection mode");
    if ((f & F_PROTMODE) != F_PROT_P)
    {
        if (opt->force < 1)
            throwCantPack("no private memory protection; use option `-f' to force packing");
    }
    if (f & F_SHTEXT)
    {
        if (opt->force < 1)
            throwCantPack("shared text segment; use option `-f' to force packing");
    }
#if 0
    // fh_reserved seems to be unused
    if (ih.fh_reserved != 0)
    {
        if (opt->force < 1)
            throwCantPack("reserved header field set; use option `-f' to force packing");
    }
#endif
    return true;
}


/*************************************************************************
//
**************************************************************************/

unsigned PackTos::patch_d0_subq(void *b, int blen, unsigned d0,
                                const char *subq_marker)
{
    // patch a "subq.l #1,d0" or "subq.w #1,d0".
    // also convert into "dbra" if possible
    assert((int)d0 > 0);

    int boff = find_be16(b, blen, get_be16(subq_marker));
    if (boff < 0)
        throwBadLoader();

    unsigned char *p = (unsigned char *)b + boff;
    if (p[2] == 0x66)                   // bne.b XXX
        checkPatch(b, blen, boff, 4);
    else
        checkPatch(b, blen, boff, 2);

    if (d0 > 65536)
    {
        set_be16(p, 0x5380);            // subq.l #1,d0
    }
    else
    {
        if (p[2] == 0x66)               // bne.b XXX
        {
            set_be16(p, 0x51c8);        // dbra d0,XXX
            // adjust and extend branch from 8 to 16 bits
            int branch = (signed char) p[3];
            set_be16(p+2, branch+2);
            // adjust d0
            d0 -= 1;
        }
        else
        {
            set_be16(p, 0x5340);        // subq.w #1,d0
        }
        d0 &= 0xffff;
    }
    return d0;
}


unsigned PackTos::patch_d0_loop(void *b, int blen, unsigned d0,
                                const char *d0_marker, const char *subq_marker)
{
    d0 = patch_d0_subq(b, blen, d0, subq_marker);

    int boff = find_be32(b, blen, get_be32(d0_marker));
    checkPatch(b, blen, boff, 4);

    unsigned char *p = (unsigned char *)b + boff;
    assert(get_be16(p - 2) == 0x203c);  // move.l #XXXXXXXX,d0
    set_be32(p, d0);

    return d0;
}


/*************************************************************************
// relocs
**************************************************************************/

// Check relocation for errors to make sure our loader can handle it.
static int check_relocs(const upx_byte *relocs, unsigned rsize, unsigned isize,
                        unsigned *nrelocs, unsigned *relocsize, unsigned *overlay)
{
    unsigned fixup = get_be32(relocs);
    unsigned last_fixup = fixup;
    unsigned i = 4;

    assert(isize >= 4);
    assert(fixup > 0);

    *nrelocs = 1;
    for (;;)
    {
        if (fixup & 1)              // must be word-aligned
            return -1;
        if (fixup + 4 > isize)      // too far
            return -1;
        if (i >= rsize)             // premature EOF in relocs
            return -1;
        int c = relocs[i++];
        if (c == 0)                 // end marker
            break;
        else if (c == 1)            // increase fixup, no reloc
            fixup += 254;
        else if (c & 1)             // must be word-aligned
            return -1;
        else                        // next reloc is here
        {
            fixup += c;
            if (fixup - last_fixup < 4)     // overlapping relocation
                return -1;
            last_fixup = fixup;
            *nrelocs += 1;
        }
    }

    *relocsize = i;
    *overlay = rsize - i;
    return 0;
}


/*************************************************************************
//
**************************************************************************/

bool PackTos::canPack()
{
    if (!readFileHeader())
        return false;

    unsigned char buf[512];
    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(buf, sizeof(buf));

    if (!checkFileHeader())
        throwCantPack("unsupported header flags");
    if (file_size < 256)
        throwCantPack("program too small");
    return true;
}


void PackTos::fileInfo()
{
    if (!readFileHeader())
        return;
#if !defined(WITH_GUI)
    con_fprintf(stdout, "    text: %d, data: %d, sym: %d, bss: %d, flags=0x%x\n",
                (int)ih.fh_text, (int)ih.fh_data, (int)ih.fh_sym, (int)ih.fh_bss, (int)ih.fh_flag);
#endif /* !defined(WITH_GUI) */
}


/*************************************************************************
//
**************************************************************************/

void PackTos::pack(OutputFile *fo)
{
    unsigned t;
    unsigned nrelocs = 0;
    unsigned relocsize = 0;
    unsigned overlay = 0;

    const unsigned lsize = getLoaderSize();
    const unsigned e_len = get_be16(getLoader()+lsize-6);
    const unsigned d_len = get_be16(getLoader()+lsize-4);
    const unsigned decomp_offset = get_be16(getLoader()+lsize-2);
    assert(e_len + d_len == lsize - 6);
    assert((e_len & 3) == 0 && (d_len & 1) == 0);

    const unsigned i_text = ih.fh_text;
    const unsigned i_data = ih.fh_data;
    const unsigned i_sym = ih.fh_sym;
    const unsigned i_bss = ih.fh_bss;

    // read file
    const unsigned isize = file_size - i_sym;
    ibuf.alloc(isize);
    fi->seek(FH_SIZE, SEEK_SET);
    // read text + data
    t = i_text + i_data;
    fi->readx(ibuf,t);
    // skip symbols
    fi->seek(i_sym,SEEK_CUR);
    // read relocations + overlay
    overlay = file_size - (FH_SIZE + i_text + i_data + i_sym);
    fi->readx(ibuf+t,overlay);

#if 0 || defined(TESTING)
    printf("text: %d, data: %d, sym: %d, bss: %d, flags=0x%x\n",
           i_text, i_data, i_sym, i_bss, (int)ih.fh_flag);
    printf("xx1 reloc: %d, overlay: %d, fixup: %d\n", relocsize, overlay, overlay >= 4 ? (int)get_be32(ibuf+t) : -1);
#endif

    // Check relocs (see load_and_reloc() in mint/src/mem.c).
    // Must work around TOS bugs and lots of broken programs.
    int r = 0;
    if (overlay < 4)
    {
        // Bug workaround: Whatever this is, silently keep it in
        // the (unused) relocations for byte-identical unpacking.
        relocsize = overlay;
        overlay = 0;
    }
    else if (get_be32(ibuf+t) == 0)
    {
        // Bug workaround - check the empty fixup before testing fh_reloc.
        relocsize = 4;
        overlay -= 4;
    }
    else if (ih.fh_reloc != 0)
        relocsize = 0;
    else
        r = check_relocs(ibuf+t, overlay, t, &nrelocs, &relocsize, &overlay);

#if 0 || defined(TESTING)
    printf("xx2: %d relocs: %d, overlay: %d, t: %d\n", nrelocs, relocsize, overlay, t);
#endif

    if (r != 0)
        throwCantPack("bad relocation table");
    checkOverlay(overlay);

    // Append original fileheader.
    t += relocsize;
    ih.fh_sym = 0;                      // we stripped all symbols
    memcpy(ibuf+t, &ih, FH_SIZE);
    t += FH_SIZE;
#if 0 || defined(TESTING)
    printf("xx3 reloc: %d, overlay: %d, t: %d\n", relocsize, overlay, t);
#endif
    assert(t <= isize);

    // Now the data in ibuf[0..t] looks like this:
    //   text + data + relocs + original file header
    // After compression this will become the first part of the
    // data segement. The second part will be the decompressor.

    // alloc buffer
    obuf.allocForCompression(t + d_len + 512);

    // compress (max_match = 65535)
    ph.u_len = t;
    if (!compress(ibuf,obuf,0,65535))
        throwNotCompressible();

    // The decompressed data will now get placed at this offset:
    const unsigned overlapoh = findOverlapOverhead(obuf, 512);
    unsigned offset = (ph.u_len + overlapoh) - ph.c_len;

    // compute addresses
    unsigned o_text, o_data, o_bss;
    o_text = e_len;
    o_data = ph.c_len;
    o_bss = i_bss;

    // word align len of compressed data
    while (o_data & 1)
    {
        obuf[o_data++] = 0;
        offset++;
    }

    // append decompressor (part 2 of loader)
    const unsigned d_off = o_data;
    memcpy(obuf+d_off, getLoader()+e_len, d_len);
    o_data += d_len;

    // dword align the len of the final data segment
    while (o_data & 3)
    {
        obuf[o_data++] = 0;
        offset++;
    }
    // dword align offset
    while (offset & 3)
        offset++;

    // new bss
    if (i_text + i_data + i_bss > o_text + o_data + o_bss)
        o_bss = (i_text + i_data + i_bss) - (o_text + o_data);

    // dirty bss
    unsigned dirty_bss = (o_data + offset) - (i_text + i_data);
    //printf("real dirty_bss: %d\n", dirty_bss);
    // dword align (or 16 - for speedup when clearing the dirty bss)
    const unsigned dirty_bss_align = opt->small ? 4 : 16;
    while (dirty_bss & (dirty_bss_align - 1))
        dirty_bss++;
    // adjust bss, assert room for some stack
    if (dirty_bss + 256 > o_bss)
        o_bss = dirty_bss + 256;

    // dword align the len of the final bss segment
    while (o_bss & 3)
        o_bss++;

    // prepare loader
    MemBuffer loader(o_text);
    memcpy(loader,getLoader(),o_text);

    // patch loader
    patchPackHeader(loader,o_text);
    if (!opt->small)
        patchVersion(loader,o_text);
    //   patch "subq.l #1,d0" or "subq.w #1,d0" - see "up41" below
    const unsigned dirty_bss_d0 =
        patch_d0_subq(loader, o_text, dirty_bss / dirty_bss_align, "u4");
    patch_be32(loader,o_text,"up31",d_off + offset + decomp_offset);
    if (opt->small)
        patch_d0_loop(loader,o_text,o_data/4,"up22","u1");
    else
    {
        if (o_data <= 160)
            throwNotCompressible();
        unsigned loop1 = o_data / 160;
        unsigned loop2 = o_data % 160;
        if (loop2 == 0)
        {
            loop1--;
            loop2 = 160;
        }
        patch_be16(loader,o_text,"u2", 0x7000 + loop2/4-1); // moveq.l #X,d0
        patch_d0_loop(loader,o_text,loop1,"up22","u1");
    }
    patch_be32(loader,o_text,"up21",o_data + offset);
    patch_be32(loader,o_text,"up13",i_bss);               // p_blen
    patch_be32(loader,o_text,"up12",i_data);              // p_dlen
    patch_be32(loader,o_text,"up11",i_text);              // p_tlen

    // patch decompressor
    upx_byte *p = obuf + d_off;
    //   patch "moveq.l #1,d3" or "jmp (a5)"
    patch_be16(p,d_len,"u3", (nrelocs > 0) ? 0x7601 : 0x4ed5);
    patch_be32(p,d_len,"up41", dirty_bss_d0);

    // set new file_hdr
    memcpy(&oh, &ih, FH_SIZE);
    if (opt->tos.split_segments)
    {
        oh.fh_text = o_text;
        oh.fh_data = o_data;
    }
    else
    {
        // put everything into the text segment
        oh.fh_text = o_text + o_data;
        oh.fh_data = 0;
    }
    oh.fh_bss = o_bss;
    oh.fh_sym = 0;
    oh.fh_reserved = 0;
    // only keep the following flags:
    oh.fh_flag = ih.fh_flag & (F_FASTLOAD | F_ALTALLOC | F_SMALLTPA | F_ALLOCZERO | F_KEEP);
    // add an empty relocation fixup to workaround a bug in some TOS versions
    oh.fh_reloc = 0;

#if 0 || defined(TESTING)
    printf("old text: %6d, data: %6d, bss: %6d, reloc: %d, overlay: %d\n",
           i_text, i_data, i_bss, relocsize, overlay);
    printf("new text: %6d, data: %6d, bss: %6d, dirty_bss: %d, flag=0x%x\n",
           o_text, o_data, o_bss, dirty_bss, (int)oh.fh_flag);
#endif

    // write new file header, loader and compressed file
    fo->write(&oh, FH_SIZE);
    fo->write(loader, o_text);  // entry
    fo->write(obuf, o_data);    // compressed + decompressor

    // write empty relocation fixup
    fo->write("\x00\x00\x00\x00",4);

    // verify
    verifyOverlappingDecompression(&obuf, overlapoh);

    // copy the overlay
    copyOverlay(fo, overlay, &obuf);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
//
**************************************************************************/

int PackTos::canUnpack()
{
    if (!readFileHeader())
        return false;
    if (!readPackHeader(512))
        return false;
    // check header as set by packer
    if ((ih.fh_text & 3) != 0 || (ih.fh_data & 3) != 0 || (ih.fh_bss & 3) != 0
        || ih.fh_sym != 0 || ih.fh_reserved != 0 || ih.fh_reloc > 1)
        throwCantUnpack("program header damaged");
    if (!checkFileHeader())
        throwCantUnpack("unsupported header flags");
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackTos::unpack(OutputFile *fo)
{
    ibuf.alloc(ph.c_len);
    obuf.allocForUncompression(ph.u_len);

    fi->seek(ph.buf_offset + ph.getPackHeaderSize(),SEEK_SET);
    fi->readx(ibuf,ph.c_len);

    // decompress
    decompress(ibuf,obuf);

    // write original header & decompressed file
    if (fo)
    {
        unsigned overlay = file_size - (FH_SIZE + ih.fh_text + ih.fh_data);
        if (ih.fh_reloc == 0 && overlay >= 4)
            overlay -= 4;                           // this is our empty fixup
        checkOverlay(overlay);

        fo->write(obuf+ph.u_len-FH_SIZE, FH_SIZE);  // orig. file_hdr
        fo->write(obuf, ph.u_len-FH_SIZE);          // orig. text+data+relocs

        // copy any overlay
        copyOverlay(fo, overlay, &obuf);
    }
}


/*
vi:ts=4:et
*/


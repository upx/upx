/* p_tos.cpp --

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

#include "file.h"
#include "filter.h"
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
static const
#include "stub/l_t_n2e.h"
static const
#include "stub/l_t_n2es.h"

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


const int *PackTos::getCompressionMethods(int method, int level) const
{
    bool small = ih.fh_text + ih.fh_data <= 256*1024;
    return Packer::getDefaultCompressionMethods_8(method, level, small);
}


const int *PackTos::getFilters() const
{
    return NULL;
}


const upx_byte *PackTos::getLoader() const
{
    if (M_IS_NRV2B(ph.method))
        return opt->small ? nrv2b_loader_small : nrv2b_loader;
    if (M_IS_NRV2D(ph.method))
        return opt->small ? nrv2d_loader_small : nrv2d_loader;
    if (M_IS_NRV2E(ph.method))
        return opt->small ? nrv2e_loader_small : nrv2e_loader;
    return NULL;
}


int PackTos::getLoaderSize() const
{
    if (M_IS_NRV2B(ph.method))
        return opt->small ? sizeof(nrv2b_loader_small) : sizeof(nrv2b_loader);
    if (M_IS_NRV2D(ph.method))
        return opt->small ? sizeof(nrv2d_loader_small) : sizeof(nrv2d_loader);
    if (M_IS_NRV2E(ph.method))
        return opt->small ? sizeof(nrv2e_loader_small) : sizeof(nrv2e_loader);
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
//   readFileHeader() reads ih and checks for illegal values
//   checkFileHeader() checks ih for legal but unsupported values
**************************************************************************/

int PackTos::readFileHeader()
{
    fi->seek(0,SEEK_SET);
    fi->readx(&ih, FH_SIZE);
    if (ih.fh_magic != 0x601a)
        return 0;
    if (FH_SIZE + ih.fh_text + ih.fh_data + ih.fh_sym > (unsigned) file_size)
        return 0;
    return UPX_F_ATARI_TOS;
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
// some 68000 opcodes for patching
**************************************************************************/

enum m68k_reg_t {
    REG_D0, REG_D1, REG_D2, REG_D3, REG_D4, REG_D5, REG_D6, REG_D7,
    REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7
};

static unsigned OP_DBRA(int d_reg)
{
    assert(d_reg >= REG_D0 && d_reg <= REG_D7);
    return 0x51c8 | (d_reg & 7);
}

static unsigned OP_JMP(int a_reg)
{
    // jmp (a0)
    assert(a_reg >= REG_A0 && a_reg <= REG_A7);
    return 0x4ed0 | (a_reg & 7);
}

static unsigned OP_MOVEI_L(int d_reg)
{
    // movei.l #XXXXXXXX,d0
    assert(d_reg >= REG_D0 && d_reg <= REG_D7);
    return 0x203c | ((d_reg & 7) << 9);
}

static unsigned OP_MOVEQ(int value, int d_reg)
{
    // moveq.l #0,d0
    assert(d_reg >= REG_D0 && d_reg <= REG_D7);
    assert(value >= -128 && value <= 127);
    return 0x7000 | ((d_reg & 7) << 9) | (value & 0xff);
}

static unsigned OP_SUBQ_L(int value, int d_reg)
{
    // subq.l #X,d0
    assert(value >= 1 && value <= 8);
    assert(d_reg >= REG_D0 && d_reg <= REG_D7);
    return 0x5180 | ((value & 7) << 9) | (d_reg & 7);
}

static unsigned OP_SUBQ_W(int value, int d_reg)
{
    // subq.w #X,d0
    assert(value >= 1 && value <= 8);
    assert(d_reg >= REG_D0 && d_reg <= REG_D7);
    return 0x5140 | ((value & 7) << 9) | (d_reg & 7);
}


/*************************************************************************
//
**************************************************************************/

unsigned PackTos::patch_d_subq(void *b, int blen,
                               int d_reg, unsigned d_value,
                               const char *subq_marker)
{
    // patch a "subq.l #1,d0" or "subq.w #1,d0".
    // also convert into "dbra" if possible
    assert(d_reg >= REG_D0 && d_reg <= REG_D7);
    assert((int)d_value > 0);

    int boff =  find_be16(b, blen, get_be16(subq_marker));
    if (boff < 0)
        throwBadLoader();

    upx_byte *p = (upx_byte *)b + boff;
    if (p[2] == 0x66)                           // bne.b XXX
        checkPatch(b, blen, boff, 4);
    else
        checkPatch(b, blen, boff, 2);

    if (d_value > 65536)
    {
        set_be16(p, OP_SUBQ_L(1, d_reg));       // subq.l #1,d0
    }
    else
    {
        if (p[2] == 0x66)                       // bne.b XXX
        {
            set_be16(p, OP_DBRA(d_reg));        // dbra d0,XXX
            // adjust and extend branch from 8 to 16 bits
            int branch = (signed char) p[3];
            set_be16(p+2, branch+2);
            // adjust d0
            d_value -= 1;
        }
        else
        {
            set_be16(p, OP_SUBQ_W(1, d_reg));   // subq.w #1,d0
        }
        d_value &= 0xffff;
    }
    return d_value;
}


unsigned PackTos::patch_d_loop(void *b, int blen,
                               int d_reg, unsigned d_value,
                               const char *d_marker, const char *subq_marker)
{
    d_value = patch_d_subq(b, blen, d_reg, d_value, subq_marker);

    int boff = find_be32(b, blen, get_be32(d_marker));
    checkPatch(b, blen, boff, 4);
    upx_byte *p = (upx_byte *)b + boff;
    assert(get_be16(p - 2) == OP_MOVEI_L(d_reg));    // move.l #XXXXXXXX,d0
    set_be32(p, d_value);
    return d_value;
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
        unsigned c = relocs[i++];
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
#if 0 // debug
# define p(x)    printf("%-30s 0x%04x\n", #x, x)
    p(OP_DBRA(REG_D0));
    p(OP_MOVEI_L(REG_D0));
    p(OP_MOVEQ(-1, REG_D0));
    p(OP_MOVEQ(1, REG_D2));
    p(OP_MOVEQ(1, REG_D3));
    p(OP_SUBQ_W(1, REG_D0));
    p(OP_SUBQ_L(1, REG_D0));
# undef p
#endif

    if (!readFileHeader())
        return false;

    unsigned char buf[768];
    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(buf, sizeof(buf));

    if (!checkFileHeader())
        throwCantPack("unsupported header flags");
    if (file_size < 1024)
        throwCantPack("program is too small");

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

    // Check relocs (see load_and_reloc() in freemint/sys/memory.c).
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

    // alloc buffer (2048 is for decompressor and the various alignments)
    obuf.allocForCompression(t, 2048);

    // prepare packheader
    ph.u_len = t;
    // prepare filter
    Filter ft(ph.level);
    // compress (max_match = 65535)
    compressWithFilters(&ft, 512, 0, NULL, 0, 65535);

    // get loader
    const unsigned lsize = getLoaderSize();
    const unsigned e_len = get_be16(getLoader()+lsize-6);
    const unsigned d_len = get_be16(getLoader()+lsize-4);
    const unsigned decomp_offset = get_be16(getLoader()+lsize-2);
    assert(e_len + d_len == lsize - 6);
    assert((e_len & 3) == 0 && (d_len & 1) == 0);

    // The decompressed data will now get placed at this offset:
    unsigned offset = (ph.u_len + ph.overlap_overhead) - ph.c_len;

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
    if (dirty_bss + 512 > o_bss)
        o_bss = dirty_bss + 512;

    // dword align the len of the final bss segment
    while (o_bss & 3)
        o_bss++;

    // prepare loader
    MemBuffer loader(o_text);
    memcpy(loader, getLoader(), o_text);

    // patch loader
    int tmp = patchPackHeader(loader, o_text);
    assert(tmp + 32 == (int)o_text); UNUSED(tmp);
    patchVersionYear(loader, o_text);
    if (!opt->small)
        patchVersion(loader, o_text);
    //   patch "subq.l #1,d6" or "subq.w #1,d6" - see "up41" below
    const unsigned dirty_bss_d6 =
        patch_d_subq(loader, o_text, REG_D6, dirty_bss / dirty_bss_align, "u4");
    patch_be32(loader, o_text, "up31", d_off + offset + decomp_offset);
    if (opt->small)
        patch_d_loop(loader, o_text, REG_D0, o_data/4, "up22", "u1");
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
        patch_be16(loader, o_text, "u2", OP_MOVEQ(loop2/4-1, REG_D0)); // moveq.l #X,d0
        patch_d_loop(loader, o_text, REG_D0, loop1, "up22", "u1");
    }
    patch_be32(loader,o_text,"up21",o_data + offset);
    patch_be32(loader,o_text,"up13",i_bss);               // p_blen
    patch_be32(loader,o_text,"up12",i_data);              // p_dlen
    patch_be32(loader,o_text,"up11",i_text);              // p_tlen

    // patch decompressor
    upx_byte *p = obuf + d_off;
    //   patch "moveq.l #1,d5" or "jmp (ASTACK)"
    patch_be16(p, d_len, "u3", (nrelocs > 0) ? OP_MOVEQ(1, REG_D5) : OP_JMP(REG_A7));
    patch_be32(p, d_len, "up41", dirty_bss_d6);

    // set new file_hdr
    memcpy(&oh, &ih, FH_SIZE);
    if (opt->atari_tos.split_segments)
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
    verifyOverlappingDecompression();

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
    if (!readPackHeader(768))
        return false;
    // check header as set by packer
    if ((ih.fh_text & 3) != 0 || (ih.fh_data & 3) != 0 || (ih.fh_bss & 3) != 0
        || ih.fh_sym != 0 || ih.fh_reserved != 0 || ih.fh_reloc > 1)
        throwCantUnpack("program header damaged");
    // generic check
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

    fi->seek(FH_SIZE + ph.buf_offset + ph.getPackHeaderSize(), SEEK_SET);
    fi->readx(ibuf, ph.c_len);

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


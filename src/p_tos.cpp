/* p_tos.cpp --

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


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_tos.h"
#include "linker.h"

static const
#include "stub/m68k-atari.tos.h"

//#define TESTING 1


/*************************************************************************
//
**************************************************************************/

#define FH_SIZE         sizeof(tos_header_t)

PackTos::PackTos(InputFile *f) :
    super(f)
{
    bele = &N_BELE_RTP::be_policy;
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


Linker* PackTos::newLinker() const
{
    return new ElfLinkerM68k;
}


void PackTos::LinkerSymbols::LoopInfo::init(unsigned count_, bool allow_dbra)
{
    count = value = count_;
    if (count == 0)
        mode = LOOP_NONE;
    else if (count <= 65536 && allow_dbra)
    {
        mode = LOOP_DBRA;
        value -= 1;
        value &= 0xffff;
    }
    else if (count <= 65536)
    {
        mode = LOOP_SUBQ_W;
        value &= 0xffff;
    }
    else
        mode = LOOP_SUBQ_L;
}


unsigned PackTos::getDecomprOffset(int method, int small) const
{
    UNUSED(small);
    if (M_IS_NRV2B(method))
        return 2;   // FIXME: do not hardcode this value
    else if (M_IS_NRV2D(method))
        return 2;   // FIXME: do not hardcode this value
    else if (M_IS_NRV2E(method))
        return 2;   // FIXME: do not hardcode this value
    else if (M_IS_LZMA(method))
        return linker->getSectionSize("__mulsi3");
    else
        throwBadLoader();
    return 0;
}


void PackTos::buildLoader(const Filter *ft)
{
    assert(ft->id == 0);

    initLoader(stub_m68k_atari_tos, sizeof(stub_m68k_atari_tos));
    //linker->dumpSymbols();

    //
    // part 1a
    //

    addLoader("entry");

    if (symbols.up21_a6 <= 32767)
        addLoader("set_up21_a6.w");
    else if (symbols.up21_d4 <= 32767)
        addLoader("set_up21_d4.w");
    else
        addLoader("set_up21_d4.l");

    assert(symbols.loop1.count || symbols.loop2.count);
    if (symbols.loop1.count)
    {
        if (symbols.loop1.value <= 127)
            addLoader("loop1_set_count.b");
        else if (symbols.loop1.value <= 65535)
            addLoader("loop1_set_count.w");
        else
            addLoader("loop1_set_count.l");
        addLoader("loop1_label");
        addLoader(opt->small ? "loop1.small" : "loop1.fast");
        if (symbols.loop1.mode == symbols.LOOP_SUBQ_L)
            addLoader("loop1_subql");
        else if (symbols.loop1.mode == symbols.LOOP_SUBQ_W)
            addLoader("loop1_subqw");
        else if (symbols.loop1.mode == symbols.LOOP_DBRA)
            addLoader("loop1_dbra");
        else
            throwBadLoader();
    }
    if (symbols.loop2.count)
    {
        assert(symbols.loop2.mode == symbols.LOOP_DBRA);
        addLoader(opt->small ? "loop2.small" : "loop2.fast");
    }

    addLoader("copy_to_stack");

    if (M_IS_NRV2B(ph.method))
        addLoader("nrv2b.init");
    else if (M_IS_NRV2D(ph.method))
        addLoader("nrv2d.init");
    else if (M_IS_NRV2E(ph.method))
        addLoader("nrv2e.init");
    else if (M_IS_LZMA(ph.method))
        addLoader("lzma.init");
    else
        throwBadLoader();

    symbols.up31_d4 = symbols.up31_base_d4 + getDecomprOffset(ph.method, opt->small);
    symbols.up31_a6 = symbols.up31_base_a6 + getDecomprOffset(ph.method, opt->small);
    if (symbols.up31_a6 <= 32767)
        addLoader("jmp_decompressor_a6.w");
    else if (symbols.up31_d4 <= 32767)
        addLoader("jmp_decompressor_d4.w");
    else if (symbols.up31_a6 <= 65534)
        addLoader("jmp_decompressor_a6.w2");
    else
        addLoader("jmp_decompressor_d4.l");

    //
    // part 1b
    //

    addLoader("code_on_stack");

    addLoader("clear_dirty_bss");
    addLoader("loop3_label");
    addLoader(opt->small ? "loop3.small" : "loop3.fast");
    if (symbols.loop3.mode == symbols.LOOP_SUBQ_L)
        addLoader("loop3_subql");
    else if (symbols.loop3.mode == symbols.LOOP_SUBQ_W)
        addLoader("loop3_subqw");
    else if (symbols.loop3.mode == symbols.LOOP_DBRA)
        addLoader("loop3_dbra");
    else
        throwBadLoader();

    addLoader("flush_cache");
    addLoader("restore_stack");
#if 0
    addLoader("clear_dirty_stack");
#endif
    addLoader("start_program");

    addLoader("IDENTSTR,+40D,UPX1HEAD,CUTPOINT");

    //
    // part 2
    //

    if (M_IS_NRV2B(ph.method)) {
        addLoader(opt->small ? "nrv2b_8.small" : "nrv2b_8.fast");
    } else if (M_IS_NRV2D(ph.method)) {
        addLoader(opt->small ? "nrv2d_8.small" : "nrv2d_8.fast");
    } else if (M_IS_NRV2E(ph.method)) {
        addLoader(opt->small ? "nrv2e_8.small" : "nrv2e_8.fast");
    } else if (M_IS_LZMA(ph.method)) {
        addLoader("__mulsi3");
        addLoader(opt->small ? "lzma.small" : "lzma.fast");
        addLoader("lzma.finish");
    }
    else
        throwBadLoader();

    if (symbols.need_reloc)
        addLoader("reloc");

    assert(symbols.loop3.count);
    if (symbols.loop3.value <= 127)
        addLoader("loop3_set_count.b");
    else if (symbols.loop3.value <= 65535)
        addLoader("loop3_set_count.w");
    else
        addLoader("loop3_set_count.l");

    addLoader("jmp_stack");
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
            throwCantPack("no private memory protection; use option '-f' to force packing");
    }
    if (f & F_SHTEXT)
    {
        if (opt->force < 1)
            throwCantPack("shared text segment; use option '-f' to force packing");
    }
#if 0
    // fh_reserved seems to be unused
    if (ih.fh_reserved != 0)
    {
        if (opt->force < 1)
            throwCantPack("reserved header field set; use option '-f' to force packing");
    }
#endif
    return true;
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
    con_fprintf(stdout, "    text: %d, data: %d, sym: %d, bss: %d, flags=0x%x\n",
                (int)ih.fh_text, (int)ih.fh_data, (int)ih.fh_sym, (int)ih.fh_bss, (int)ih.fh_flag);
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

    symbols.reset();
    symbols.need_reloc = false;
    // prepare symbols for buildLoader() - worst case
    symbols.loop1.init(65536 + 1);
    symbols.loop2.init((160 - 1) / 4);
    symbols.loop3.init(65536 + 1);
    symbols.up21_d4 = 65536 + 1;
    symbols.up21_a6 = 65536 + 1;
    symbols.up31_base_d4 = 65536 + 1;
    symbols.up31_base_a6 = 65536 + 1;

    // read file
    const unsigned isize = file_size - i_sym;
    ibuf.alloc(isize);
    fi->seek(FH_SIZE, SEEK_SET);
    // read text + data
    t = i_text + i_data;
    fi->readx(ibuf,t);
    // skip symbols
    if (i_sym && opt->exact)
        throwCantPackExact();
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
    else {
        int r = check_relocs(ibuf+t, overlay, t, &nrelocs, &relocsize, &overlay);
        if (r != 0)
            throwCantPack("bad relocation table");
        symbols.need_reloc = true;
    }

#if 0 || defined(TESTING)
    printf("xx2: %d relocs: %d, overlay: %d, t: %d\n", nrelocs, relocsize, overlay, t);
#endif

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

    // alloc buffer (4096 is for decompressor and the various alignments)
    obuf.allocForCompression(t, 4096);

    // prepare packheader
    ph.u_len = t;
    // prepare filter
    Filter ft(ph.level);
    // compress (max_match = 65535)
    upx_compress_config_t cconf; cconf.reset();
    cconf.conf_ucl.max_match = 65535;
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 512, &cconf);

    //
    // multipass buildLoader()
    //

    // save initial loader
    const unsigned initial_lsize = getLoaderSize();
    unsigned last_lsize = initial_lsize;
    MemBuffer last_loader(last_lsize);
    memcpy(last_loader, getLoader(), last_lsize);

    unsigned o_text, o_data, o_bss;
    unsigned e_len, d_len, d_off;
    for (;;)
    {
        // The decompressed data will now get placed at this offset:
        unsigned offset = (ph.u_len + ph.overlap_overhead) - ph.c_len;

        // get loader
        const unsigned lsize = getLoaderSize();
        e_len = getLoaderSectionStart("CUTPOINT");
        d_len = lsize - e_len;
        assert((e_len & 3) == 0 && (d_len & 1) == 0);

        // compute section sizes
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
        d_off = o_data;
        ////memcpy(obuf + d_off, getLoader() + e_len, d_len); // must be done after relocation
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
        unsigned stack = 512 + getDecompressorWrkmemSize();
        if (dirty_bss + stack > o_bss)
            o_bss = dirty_bss + stack;

        // dword align the len of the final bss segment
        while (o_bss & 3)
            o_bss++;

        // update symbols for buildLoader()
        if (opt->small)
        {
            symbols.loop1.init(o_data / 4);
            symbols.loop2.init(0);
        }
        else
        {
            symbols.loop1.init(o_data / 160);
            symbols.loop2.init((o_data % 160) / 4);
        }
        symbols.loop3.init(dirty_bss / dirty_bss_align);

        symbols.up21_d4 = o_data + offset;
        symbols.up31_base_d4 = d_off + offset;
        symbols.up21_a6 = symbols.up21_d4 - (i_text + i_data);
        symbols.up31_base_a6 = symbols.up31_base_d4 - (i_text + i_data);
        assert((int)symbols.up21_a6 > 0);
        assert((int)symbols.up31_base_a6 > 0);

        const unsigned c = linker->getSymbolOffset("code_on_stack");
        unsigned d;
        d = linker->getSymbolOffset("flush_cache_rts") - c;
        symbols.flush_cache_rts_offset = d;
        d = linker->getSymbolOffset("clear_dirty_stack_loop") - c;
        symbols.clear_dirty_stack_len = (d + 3) / 4 + 32 - 1;
        d = linker->getSymbolOffset("code_on_stack_end") - c;
        symbols.copy_to_stack_len = d / 2 - 1;

        // now re-build loader
        buildLoader(&ft);
        unsigned new_lsize = getLoaderSize();
        //printf("buildLoader %d %d\n", new_lsize, initial_lsize);
        assert(new_lsize <= initial_lsize);
        if (new_lsize == last_lsize && memcmp(getLoader(), last_loader, last_lsize) == 0)
            break;
        last_lsize = new_lsize;
        memcpy(last_loader, getLoader(), last_lsize);
    }

    //
    // define symbols and reloc
    //

    defineDecompressorSymbols();

    linker->defineSymbol("loop1_count", symbols.loop1.value);
    linker->defineSymbol("loop2_count", symbols.loop2.value);
    linker->defineSymbol("loop3_count", symbols.loop3.value);

    linker->defineSymbol("orig_p_tlen", i_text);
    linker->defineSymbol("orig_p_dlen", i_data);
    linker->defineSymbol("orig_p_blen", i_bss);

    if (symbols.up21_a6 <= 32767)
        linker->defineSymbol("up21_a6", symbols.up21_a6);
    else
        linker->defineSymbol("up21_d4", symbols.up21_d4);

    if (symbols.up31_a6 <= 32767)
        linker->defineSymbol("up31_a6", symbols.up31_a6);
    else if (symbols.up31_d4 <= 32767)
        linker->defineSymbol("up31_d4", symbols.up31_d4);
    else if (symbols.up31_a6 <= 65534)
        linker->defineSymbol("up31_a6", symbols.up31_a6 - 32767);
    else
        linker->defineSymbol("up31_d4", symbols.up31_d4);
#if 0
    printf("relocsize = %d\n", relocsize);
    printf("upx21(d4) = %d\n", symbols.up21_d4);
    printf("upx21(a6) = %d\n", symbols.up21_a6);
    printf("upx31(d4) = %d\n", symbols.up31_d4);
    printf("upx31(a6) = %d\n", symbols.up31_a6);
#endif

    linker->defineSymbol("flush_cache_rts_offset", symbols.flush_cache_rts_offset);
    linker->defineSymbol("copy_to_stack_len", symbols.copy_to_stack_len);
    linker->defineSymbol("clear_dirty_stack_len", symbols.clear_dirty_stack_len);

    relocateLoader();

    //
    // write
    //

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
    printf("new text: %6d, data: %6d, bss: %6d, flag=0x%x\n",
           o_text, o_data, o_bss, (int)oh.fh_flag);
    linker->dumpSymbols();
#endif

    // prepare loader
    MemBuffer loader(o_text);
    memcpy(loader, getLoader(), o_text);
    patchPackHeader(loader, o_text);

    // write new file header, loader and compressed file
    fo->write(&oh, FH_SIZE);
    fo->write(loader, o_text);  // entry
    if (opt->debug.dump_stub_loader)
        OutputFile::dump(opt->debug.dump_stub_loader, loader, o_text);
    memcpy(obuf + d_off, getLoader() + e_len, d_len); // copy decompressor
    fo->write(obuf, o_data);    // compressed + decompressor

    // write empty relocation fixup
    fo->write("\x00\x00\x00\x00", 4);

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


/* p_exe.cpp -- dos/exe executable format

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
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_exe.h"
#include "linker.h"

static const CLANG_FORMAT_DUMMY_STATEMENT
#include "stub/i086-dos16.exe.h"

#define MAXMATCH     0x2000
#define MAXRELOCSIZE (0x8000 - MAXMATCH)

#define DI_LIMIT 0xff00 // see the assembly why

/*************************************************************************
//
**************************************************************************/

PackExe::PackExe(InputFile *f) : super(f) {
    bele = &N_BELE_RTP::le_policy;
    COMPILE_TIME_ASSERT(sizeof(exe_header_t) == 32)
    COMPILE_TIME_ASSERT_ALIGNED1(exe_header_t)
}

Linker *PackExe::newLinker() const { return new ElfLinkerX86(); }

const int *PackExe::getCompressionMethods(int method, int level) const {
    bool small = ih_imagesize <= 256 * 1024;
    // disable lzma for "--brute" unless explicitly given "--lzma"
    // (note that class PackMaster creates per-file local options)
    if (opt->all_methods_use_lzma == 1 && !opt->method_lzma_seen)
        opt->all_methods_use_lzma = 0;
    return Packer::getDefaultCompressionMethods_8(method, level, small);
}

const int *PackExe::getFilters() const { return nullptr; }

int PackExe::fillExeHeader(struct exe_header_t *eh) const {
#define oh (*eh)
    // fill new exe header
    int flag = 0;
    if (!opt->dos_exe.no_reloc && !M_IS_LZMA(ph.method))
        flag |= USEJUMP;
    if (ih.relocs == 0)
        flag |= NORELOC;

    mem_clear(&oh);
    oh.ident = 'M' + 'Z' * 256;
    oh.headsize16 = 2;

    unsigned minsp = 0x200;
    if (M_IS_LZMA(ph.method))
        minsp = stack_for_lzma;
    minsp = ALIGN_UP(minsp, 16u);
    assert(minsp < 0xff00);
    if (oh.sp > minsp)
        minsp = oh.sp;
    if (minsp < 0xff00 - 2)
        minsp = ALIGN_UP(minsp, 2u);
    oh.sp = minsp;

    unsigned destpara = (ph.u_len + ph.overlap_overhead - ph.c_len + 31) / 16;
    oh.ss = ph.c_len / 16 + destpara;
    if (ih.ss * 16 + ih.sp < 0x100000 && ih.ss > oh.ss && ih.sp > 0x200)
        oh.ss = ih.ss;
    if (oh.ss * 16 + 0x50 < ih.ss * 16 + ih.sp && oh.ss * 16 + 0x200 > ih.ss * 16 + ih.sp)
        oh.ss += 0x20;

    if (oh.ss != ih.ss)
        flag |= SS;
    if (oh.sp != ih.sp || M_IS_LZMA(ph.method))
        flag |= SP;
    return flag;
#undef oh
}

void PackExe::addLoaderEpilogue(int flag) {
    addLoader("EXEMAIN5");
    if (relocsize)
        addLoader(ph.u_len <= DI_LIMIT || (ph.u_len & 0x7fff) >= relocsize ? "EXENOADJ"
                                                                           : "EXEADJUS",
                  "EXERELO1", has_9a ? "EXEREL9A" : "", "EXERELO2",
                  ih_exesize > 0xFE00 ? "EXEREBIG" : "", "EXERELO3");
    addLoader("EXEMAIN8", device_driver ? "DEVICEEND" : "", (flag & SS) ? "EXESTACK" : "",
              (flag & SP) ? "EXESTASP" : "", (flag & USEJUMP) ? "EXEJUMPF" : "");
    if (!(flag & USEJUMP))
        addLoader(ih.cs ? "EXERCSPO" : "", "EXERETIP");

    linker->defineSymbol("original_cs", ih.cs);
    linker->defineSymbol("original_ip", ih.ip);
    linker->defineSymbol("original_sp", ih.sp);
    linker->defineSymbol("original_ss", ih.ss);
    linker->defineSymbol(
        "reloc_size",
        (ph.u_len <= DI_LIMIT || (ph.u_len & 0x7fff) >= relocsize ? 0 : MAXRELOCSIZE) - relocsize);
}

void PackExe::buildLoader(const Filter *) {
    // get flag
    exe_header_t dummy_oh;
    int flag = fillExeHeader(&dummy_oh);

    initLoader(stub_i086_dos16_exe, sizeof(stub_i086_dos16_exe));

    if (M_IS_LZMA(ph.method)) {
        addLoader("LZMA_DEC00", opt->small ? "LZMA_DEC10" : "LZMA_DEC20", "LZMA_DEC30",
                  use_clear_dirty_stack ? "LZMA_DEC31" : "", "LZMA_DEC32",
                  ph.u_len > 0xffff ? "LZMA_DEC33" : "");

        addLoaderEpilogue(flag);
        defineDecompressorSymbols();
        const unsigned lsize0 = getLoaderSize();

        // Lzma decompression code starts at ss:0x10, and its size is
        // lsize bytes. It also needs getDecompressorWrkmemSize() bytes
        // during uncompression. It also uses some stack, so 0x100
        // more bytes are allocated
        stack_for_lzma = 0x10 + lsize0 + getDecompressorWrkmemSize() + 0x100;
        stack_for_lzma = ALIGN_UP(stack_for_lzma, 16u);

        unsigned clear_dirty_stack_low = 0x10 + lsize0;
        clear_dirty_stack_low = ALIGN_UP(clear_dirty_stack_low, 2u);
        if (use_clear_dirty_stack)
            linker->defineSymbol("clear_dirty_stack_low", clear_dirty_stack_low);

        relocateLoader();
        const unsigned lsize = getLoaderSize();
        assert(lsize0 == lsize);
        MemBuffer loader(lsize);
        memcpy(loader, getLoader(), lsize);

        MemBuffer compressed_lzma;
        compressed_lzma.allocForCompression(lsize);
        unsigned c_len_lzma = MemBuffer::getSizeForCompression(lsize);
        int r = upx_compress(loader, lsize, compressed_lzma, &c_len_lzma, nullptr, M_NRV2B_LE16, 9,
                             nullptr, nullptr);
        assert(r == UPX_E_OK);
        assert(c_len_lzma < lsize);

        info("lzma+relocator code compressed: %u -> %u", lsize, c_len_lzma);
        // reinit the loader
        initLoader(stub_i086_dos16_exe, sizeof(stub_i086_dos16_exe));
        // prepare loader
        if (device_driver)
            addLoader("DEVICEENTRY,LZMADEVICE,DEVICEENTRY2");

        linker->addSection("COMPRESSED_LZMA", compressed_lzma, c_len_lzma, 0);
        addLoader("LZMAENTRY,NRV2B160,NRVDDONE,NRVDECO1,NRVGTD00,NRVDECO2");

    } else if (device_driver)
        addLoader("DEVICEENTRY,DEVICEENTRY2");

    addLoader("EXEENTRY", M_IS_LZMA(ph.method) && device_driver ? "LONGSUB" : "SHORTSUB",
              "JNCDOCOPY", relocsize ? "EXERELPU" : "", "EXEMAIN4",
              M_IS_LZMA(ph.method) ? "" : "EXEMAIN4B", "EXEMAIN4C",
              M_IS_LZMA(ph.method) ? "COMPRESSED_LZMA_START,COMPRESSED_LZMA" : "",
              "+G5DXXXX,UPX1HEAD,EXECUTPO");
    if (ph.method == M_NRV2B_8)
        addLoader("NRV2B16S", // decompressor
                  ph.u_len > DI_LIMIT ? "N2B64K01" : "", "NRV2BEX1",
                  opt->cpu_x86 == opt->CPU_8086 ? "N2BX8601" : "N2B28601", "NRV2BEX2",
                  opt->cpu_x86 == opt->CPU_8086 ? "N2BX8602" : "N2B28602", "NRV2BEX3",
                  ph.c_len > 0xffff ? "N2B64K02" : "", "NRV2BEX9");
    else if (ph.method == M_NRV2D_8)
        addLoader("NRV2D16S", ph.u_len > DI_LIMIT ? "N2D64K01" : "", "NRV2DEX1",
                  opt->cpu_x86 == opt->CPU_8086 ? "N2DX8601" : "N2D28601", "NRV2DEX2",
                  opt->cpu_x86 == opt->CPU_8086 ? "N2DX8602" : "N2D28602", "NRV2DEX3",
                  ph.c_len > 0xffff ? "N2D64K02" : "", "NRV2DEX9");
    else if (ph.method == M_NRV2E_8)
        addLoader("NRV2E16S", ph.u_len > DI_LIMIT ? "N2E64K01" : "", "NRV2EEX1",
                  opt->cpu_x86 == opt->CPU_8086 ? "N2EX8601" : "N2E28601", "NRV2EEX2",
                  opt->cpu_x86 == opt->CPU_8086 ? "N2EX8602" : "N2E28602", "NRV2EEX3",
                  ph.c_len > 0xffff ? "N2E64K02" : "", "NRV2EEX9");
    else if M_IS_LZMA (ph.method)
        return;
    else
        throwInternalError("unknown compression method");

    addLoaderEpilogue(flag);
}

/*************************************************************************
//
**************************************************************************/

int PackExe::readFileHeader() {
    ih_exesize = ih_imagesize = ih_overlay = 0;
    fi->readx(&ih, sizeof(ih));
    if (ih.ident != 'M' + 'Z' * 256 && ih.ident != 'Z' + 'M' * 256)
        return 0;
    ih_exesize = ih.m512 + ih.p512 * 512 - (ih.m512 ? 512 : 0);
    if (ih_exesize == 0)
        ih_exesize = file_size;
    ih_imagesize = ih_exesize - ih.headsize16 * 16;
    ih_overlay = file_size - ih_exesize;
    if (file_size_u < sizeof(ih) || ((ih.m512 | ih.p512) && ih.m512 + ih.p512 * 512u < sizeof(ih)))
        throwCantPack("illegal exe header");
    if (ih_exesize > file_size_u || ih_imagesize < 4 || ih_imagesize > ih_exesize)
        throwCantPack("exe header corrupted");
    NO_printf("dos/exe header: %d %d %d\n", ih_exesize, ih_imagesize, ih_overlay);
    return UPX_F_DOS_EXE;
}

tribool PackExe::canPack() {
    if (fn_has_ext(fi->getName(), "sys")) // dos/sys
        return false;
    if (!readFileHeader())
        return false;
    if (file_size < 1024 || ih_imagesize < 512)
        throwCantPack("file is too small for dos/exe");
    fi->seek(0x3c, SEEK_SET);
    LE32 offs;
    fi->readx(&offs, sizeof(offs));
    if (ih.relocoffs >= 0x40 && offs) {
        if (opt->dos_exe.force_stub)
            opt->overlay = opt->COPY_OVERLAY;
        else
            throwCantPack("dos/exe: can't pack new-exe");
    }
    return true;
}

/*************************************************************************
//
**************************************************************************/

static unsigned optimize_relocs(SPAN_S(byte) image, const unsigned image_size,
                                SPAN_S(const byte) relocs, const unsigned relocnum,
                                SPAN_S(byte) crel, bool *has_9a) {
#if WITH_XSPAN >= 2
    ptr_check_no_overlap(image.data(image_size), image_size, relocs.data(), relocs.size_bytes(),
                         crel.data(), crel.size_bytes());
#endif
    if (opt->exact)
        throwCantPackExact();

    SPAN_S_VAR(byte, const crel_start, crel);
    unsigned seg_high = 0;
#if 0
    unsigned seg_low = 0xffffffff;
    unsigned off_low = 0xffffffff;
    unsigned off_high = 0;
    unsigned linear_low = 0xffffffff;
    unsigned linear_high = 0;
#endif

    // pass 1 - find 0x9a bounds in image
    for (unsigned i = 0; i < relocnum; i++) {
        unsigned addr = get_le32(relocs + 4 * i);
        if (addr >= image_size - 1)
            throwCantPack("unexpected relocation 1");
        if (addr >= 3 && image[addr - 3] == 0x9a) {
            unsigned seg = get_le16(image + addr);
            if (seg > seg_high)
                seg_high = seg;
#if 0
            if (seg < seg_low)
                seg_low = seg;
            unsigned off = get_le16(image + addr - 2);
            if (off < off_low)
                off_low = off;
            if (off > off_high)
                off_high = off;
            unsigned l = (seg << 4) + off;
            if (l < linear_low)
                linear_low = l;
            if (l > linear_high)
                linear_high = l;
#endif
        }
    }
    // printf("%d %d\n", seg_low, seg_high);
    // printf("%d %d\n", off_low, off_high);
    // printf("%d %d\n", linear_low, linear_high);

    // pass 2 - reloc

    crel += 4; // to be filled in later

    unsigned ones = 0;
    unsigned es = 0;
    for (unsigned i = 0; i < relocnum;) {
        unsigned addr = get_le32(relocs + 4 * i);
        unsigned di = addr & 0x0f;
        set_le16(crel + 0, di);
        set_le16(crel + 2, (addr >> 4) - es);
        crel += 4;
        es = addr >> 4;

        for (++i; i < relocnum; i++) {
            unsigned t;
            addr = get_le32(relocs + 4 * i);
            NO_printf("%x\n", es * 16 + di);
            if ((addr - es * 16 > 0xfffe) || (i == relocnum - 1 && addr - es * 16 > 0xff00)) {
                // segment change
                t = 1 + (0xffff - di) / 254;
                memset(crel, 1, t);
                crel += t;
                ones += t - 1; // -1 is used to help the assembly stuff
                break;
            }
            unsigned offs = addr - es * 16;
            if (offs >= 3 && image[es * 16 + offs - 3] == 0x9a && offs > di + 3) {
                for (t = di; t < offs - 3; t++)
                    if (image[es * 16 + t] == 0x9a && get_le16(image + es * 16 + t + 3) <= seg_high)
                        break;
                if (t == offs - 3) {
                    // code 0: search for 0x9a
                    *crel++ = 0;
                    di = offs;
                    *has_9a = true;
                    continue;
                }
            }
            t = offs - di;
            if ((int) t < 2)
                throwCantPack("unexpected relocation 2");
            while (t >= 256) {
                // code 1: add 254, don't reloc
                *crel++ = 1;
                t -= 254;
                ones++;
            }
            *crel++ = (byte) t;
            di = offs;
        }
    }
    *crel++ = 1;
    ones++;
    set_le16(crel_start, ones);
    set_le16(crel_start + 2, seg_high);

    // OutputFile::dump("x.rel", crel_start, ptr_udiff_bytes(crel, crel_start));
    return ptr_udiff_bytes(crel, crel_start);
}

/*************************************************************************
//
**************************************************************************/

void PackExe::pack(OutputFile *fo) {
    unsigned ic;

    const unsigned relocnum = ih.relocs;
    if (relocnum > MAXRELOCSIZE) // early check
        throwCantPack("too many relocations");
    checkOverlay(ih_overlay);

    // read image
    // image + space for optimized relocs + safety/alignments
    ibuf.alloc(ih_imagesize + 4 * relocnum + 1024);
    fi->seek(ih.headsize16 * 16, SEEK_SET);
    fi->readx(ibuf, ih_imagesize);

    checkAlreadyPacked(ibuf, UPX_MIN(ih_imagesize, 127u));

    device_driver = get_le32(ibuf) == 0xffffffffu;

    // relocations
    relocsize = 0;
    has_9a = false;
    if (relocnum) {
        MemBuffer mb_relocs(4 * relocnum);
        SPAN_S_VAR(byte, relocs, mb_relocs);
        fi->seek(ih.relocoffs, SEEK_SET);
        fi->readx(relocs, 4 * relocnum);

        // dos/exe runs in real-mode, so convert to linear addresses
        for (ic = 0; ic < relocnum; ic++) {
            unsigned jc = get_le32(relocs + 4 * ic);
            set_le32(relocs + 4 * ic, ((jc >> 16) * 16 + (jc & 0xffff)) & 0xfffff);
        }
        upx_qsort(raw_bytes(relocs, 4 * relocnum), relocnum, 4, le32_compare);

        SPAN_S_VAR(byte, image, ibuf + 0, ih_imagesize);
        SPAN_S_VAR(byte, crel, ibuf + ih_imagesize, ibuf);
        relocsize = optimize_relocs(image, ih_imagesize, relocs, relocnum, crel, &has_9a);
        set_le16(crel + relocsize, relocsize + 2);
        relocsize += 2;
        assert(relocsize >= 11);
        if (relocsize > MAXRELOCSIZE) // optimize_relocs did not help
            throwCantPack("too many relocations");
#if TESTING && 0
        unsigned rout_len = MemBuffer::getSizeForCompression(relocsize);
        MemBuffer rout(rout_len);
        ucl_nrv2b_99_compress(raw_bytes(crel, relocsize), relocsize, rout, &rout_len, nullptr, 9,
                              nullptr, nullptr);
        printf("dos/exe reloc compress: %d -> %d\n", relocsize, rout_len);
#endif
    }

    // prepare packheader
    ph.u_len = ih_imagesize + relocsize;
    obuf.allocForCompression(ph.u_len);
    // prepare filter
    Filter ft(ph.level);
    // compress (max_match = 8192)
    upx_compress_config_t cconf;
    cconf.reset();
    cconf.conf_ucl.max_match = MAXMATCH;
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 32, &cconf);

    if (M_IS_NRV2B(ph.method) || M_IS_NRV2D(ph.method) || M_IS_NRV2E(ph.method))
        if (ph.max_run_found + ph.max_match_found > 0x8000)
            throwCantPack("decompressor limit exceeded, send a bugreport");

#if TESTING
    if (opt->debug.debug_level) {
        printf("image+relocs %d -> %d\n", ih_imagesize + relocsize, ph.c_len);
        printf("offsets: %d - %d\nmatches: %d - %d\nruns: %d - %d\n", 0 /*ph.min_offset_found*/,
               ph.max_offset_found, 0 /*ph.min_match_found*/, ph.max_match_found,
               0 /*ph.min_run_found*/, ph.max_run_found);
    }
#endif

    int flag = fillExeHeader(&oh);

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    // OutputFile::dump("xxloader.dat", loader, lsize);

    // patch loader
    const unsigned packedsize = ph.c_len;
    const unsigned e_len = getLoaderSectionStart("EXECUTPO");
    const unsigned d_len = lsize - e_len;
    assert((e_len & 15) == 0);

    const unsigned copysize = (1 + packedsize + d_len) & ~1;
    const unsigned firstcopy = copysize % 0x10000 ? copysize % 0x10000 : 0x10000;

    // set oh.min & oh.max
    ic = ih.min * 16 + ih_imagesize;
    if (ic < oh.ss * 16u + oh.sp)
        ic = oh.ss * 16u + oh.sp;
    oh.min = (ic - (packedsize + lsize)) / 16;
    ic = oh.min + (ih.max - ih.min);
    oh.max = ic < 0xffff && ih.max != 0xffff ? ic : 0xffff;

    // set extra info
    byte extra_info[9];
    unsigned eisize = 0;
    if (oh.ss != ih.ss) {
        set_le16(extra_info + eisize, ih.ss);
        eisize += 2;
        assert((flag & SS) != 0); // set in fillExeHeader()
    }
    if (oh.sp != ih.sp) {
        set_le16(extra_info + eisize, ih.sp);
        eisize += 2;
        assert((flag & SP) != 0); // set in fillExeHeader()
    }
    if (ih.min != oh.min) {
        set_le16(extra_info + eisize, ih.min);
        eisize += 2;
        flag |= MINMEM;
    }
    if (ih.max != oh.max) {
        set_le16(extra_info + eisize, ih.max);
        eisize += 2;
        flag |= MAXMEM;
    }
    extra_info[eisize++] = (byte) flag;

    if (M_IS_NRV2B(ph.method) || M_IS_NRV2D(ph.method) || M_IS_NRV2E(ph.method))
        linker->defineSymbol("bx_magic", 0x7FFF + 0x10 * ((packedsize & 15) + 1));

    unsigned decompressor_entry = 1 + (packedsize & 15);
    if (M_IS_LZMA(ph.method))
        decompressor_entry = 0x10;
    linker->defineSymbol("decompressor_entry", decompressor_entry);

    // patch loader
    if (flag & USEJUMP) {
        // I use a relocation entry to set the original cs
        unsigned n = getLoaderSectionStart("EXEJUMPF") + 1;
        n += packedsize + 2;
        oh.relocs = 1;
        oh.firstreloc = (n & 0xf) + ((n >> 4) << 16);
    } else {
        oh.relocs = 0;
        oh.firstreloc = ih.cs * 0x10000 + ih.ip;
    }

    oh.relocoffs = offsetof(exe_header_t, firstreloc);

    linker->defineSymbol("destination_segment", oh.ss - ph.c_len / 16 - e_len / 16);
    linker->defineSymbol("source_segment", e_len / 16 + (copysize - firstcopy) / 16);
    linker->defineSymbol("copy_offset", firstcopy - 2);
    linker->defineSymbol("words_to_copy", firstcopy / 2);

    linker->defineSymbol("exe_stack_sp", oh.sp);
    linker->defineSymbol("exe_stack_ss", oh.ss);
    linker->defineSymbol("interrupt", get_le16(ibuf + 8));
    linker->defineSymbol("attribute", get_le16(ibuf + 4));
    linker->defineSymbol("orig_strategy", get_le16(ibuf + 6));

    const unsigned outputlen = sizeof(oh) + e_len + packedsize + d_len + eisize;
    oh.m512 = outputlen & 511;
    oh.p512 = (outputlen + 511) >> 9;

    const char *exeentry = M_IS_LZMA(ph.method) ? "LZMAENTRY" : "EXEENTRY";
    oh.ip = device_driver ? getLoaderSection(exeentry) - 2 : 0;

    defineDecompressorSymbols();
    relocateLoader();
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, e_len);

    NO_fprintf(stderr, "\ne_len=%x d_len=%x c_len=%x oo=%x ulen=%x copysize=%x imagesize=%x", e_len,
               d_len, packedsize, ph.overlap_overhead, ph.u_len, copysize, ih_imagesize);

    // write header + write loader + compressed file
#if TESTING
    if (opt->debug.debug_level)
        printf("\n%d %d %d %d\n", (int) sizeof(oh), e_len, packedsize, d_len);
#endif
    fo->write(&oh, sizeof(oh));       // program header
    fo->write(loader, e_len);         // entry code
    fo->write(obuf, packedsize);      // compressed data
    fo->write(loader + e_len, d_len); // decompressor code
    fo->write(extra_info, eisize);    // extra info for unpacking
    assert(eisize <= 9);
    NO_printf("%-13s: program hdr  : %8u bytes\n", getName(), usizeof(oh));
    NO_printf("%-13s: entry        : %8u bytes\n", getName(), e_len);
    NO_printf("%-13s: compressed   : %8u bytes\n", getName(), packedsize);
    NO_printf("%-13s: decompressor : %8u bytes\n", getName(), d_len);
    NO_printf("%-13s: extra info   : %8u bytes\n", getName(), eisize);

    // verify
    verifyOverlappingDecompression();

    // copy the overlay
    copyOverlay(fo, ih_overlay, obuf);
    NO_fprintf(stderr, "dos/exe %x %x\n", relocsize, ph.u_len);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}

/*************************************************************************
//
**************************************************************************/

tribool PackExe::canUnpack() {
    if (!readFileHeader())
        return false;
    const unsigned off = ih.headsize16 * 16;
    fi->seek(off, SEEK_SET);
    bool b = readPackHeader(4096);
    return b && (off + ph.c_len <= file_size_u);
}

/*************************************************************************
//
**************************************************************************/

void PackExe::unpack(OutputFile *fo) {
    ibuf.alloc(file_size);
    obuf.allocForDecompression(ph.u_len);

    // read the file
    fi->seek(ih.headsize16 * 16, SEEK_SET);
    fi->readx(ibuf, ih_imagesize);

    // get compressed data offset
    unsigned e_len = ph.buf_offset + ph.getPackHeaderSize();
    if (ih_imagesize <= e_len + ph.c_len)
        throwCantUnpack("file damaged");

    checkOverlay(ih_overlay);

    // decompress
    decompress(ibuf + e_len, obuf);

    unsigned imagesize = ih_imagesize;
    imagesize -= 1;
    const byte flag = ibuf[imagesize];

    // relocations
    unsigned relocnum = 0;
    SPAN_S_VAR(const byte, relocstart, obuf + ph.u_len, obuf);
    MemBuffer mb_relocs;
    SPAN_0_VAR(byte, relocs, nullptr);
    if (!(flag & NORELOC)) {
        mb_relocs.alloc(4 * MAXRELOCSIZE);
        relocs = mb_relocs; // => now a SPAN_S

        relocsize = get_le16(obuf + ph.u_len - 2);
        ph.u_len -= 2;
        if (relocsize < 11 || relocsize > MAXRELOCSIZE || relocsize >= imagesize)
            throwCantUnpack("bad relocations");
        relocstart -= relocsize;

        // unoptimize_relocs
        unsigned ones = get_le16(relocstart);
        const unsigned seg_high = get_le16(relocstart + 2);
        SPAN_S_VAR(const byte, p, relocstart + 4);
        unsigned es = 0;
        while (ones) {
            unsigned di = get_le16(p);
            es += get_le16(p + 2);
            bool dorel = true;
            for (p += 4; ones && di < 0x10000; p++) {
                if (dorel) {
                    set_le16(relocs + (4 * relocnum + 0), di);
                    set_le16(relocs + (4 * relocnum + 2), es);
                    NO_printf("dos/exe unreloc %4d %6x\n", relocnum, es * 16 + di);
                    relocnum++;
                }
                dorel = true;
                if (*p == 0) {
                    SPAN_S_VAR(const byte, q, obuf + (es * 16 + di), obuf);
                    while (!(*q == 0x9a && get_le16(q + 3) <= seg_high))
                        q++;
                    di = ptr_udiff_bytes(q, obuf + (es * 16)) + 3;
                } else if (*p == 1) {
                    di += 254;
                    if (di < 0x10000)
                        ones--;
                    dorel = false;
                } else
                    di += *p;
            }
        }
    }

    // fill new exe header
    mem_clear(&oh);
    oh.ident = 'M' + 'Z' * 256;

    if (relocnum) {
        oh.relocs = relocnum;
        while (relocnum & 3) // paragraph align
            set_le32(relocs + (4 * relocnum++), 0);
    }

    unsigned outputlen = sizeof(oh) + 4 * relocnum + ptr_udiff_bytes(relocstart, obuf);
    oh.m512 = outputlen & 511;
    oh.p512 = (outputlen + 511) >> 9;
    oh.headsize16 = 2 + relocnum / 4;

    oh.max = ih.max;
    oh.min = ih.min;
    oh.sp = ih.sp;
    oh.ss = ih.ss;

    if (flag & MAXMEM) {
        imagesize -= 2;
        oh.max = get_le16(ibuf + imagesize);
    }
    if (flag & MINMEM) {
        imagesize -= 2;
        oh.min = get_le16(ibuf + imagesize);
    }
    if (flag & SP) {
        imagesize -= 2;
        oh.sp = get_le16(ibuf + imagesize);
    }
    if (flag & SS) {
        imagesize -= 2;
        oh.ss = get_le16(ibuf + imagesize);
    }

    unsigned ip = (flag & USEJUMP) ? get_le32(ibuf + imagesize - 4) : (unsigned) ih.firstreloc;
    oh.ip = ip & 0xffff;
    oh.cs = ip >> 16;

    oh.relocoffs = sizeof(oh);
    oh.firstreloc = 0;
    if (!fo)
        return;

    // write header + relocations + uncompressed file
    fo->write(&oh, sizeof(oh));
    if (relocnum)
        fo->write(relocs, 4 * relocnum);
    fo->write(obuf, ptr_udiff_bytes(relocstart, obuf));

    // copy the overlay
    copyOverlay(fo, ih_overlay, obuf);
}

/*

memory layout at decompression time
===================================

normal exe
----------

a, at load time

(e - copying code, C - compressed data, d - decompressor+relocator,
 x - not specified, U - uncompressed code+data, R uncompressed relocation)

eeCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCdddd
^ CS:0                                       ^ SS:0

b, after copying

xxxxxxxxxxxxxxxCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCdddd
^ES:DI=0       ^ DS:SI=0                     ^ CS=SS, IP in range 0..0xf

c, after uncompression

UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUURRdddd
                                           ^ ES:DI

device driver
-------------

the file has 2 entry points, CS:0 in device driver mode, and
CS:exe_as_device_entry in normal mode. the code in section DEVICEENTRY
sets up the same environment for section EXEENTRY, as it would see in normal
execution mode.

lzma uncompression for normal exes
----------------------------------

(n - nrv2b uncompressor, l - nrv2b compressed lzma + relocator code)

a, at load time

nneelllCCCCCCCCCCCCCCCCCCCCCCCCC

^ CS:0                                       ^ SS:0

b, after nrv2b

nneelllCCCCCCCCCCCCCCCCCCCCCCCCC             dddd
^ CS:0                                       ^ SS:0x10

after this, normal ee code runs

lzma + device driver
--------------------

(D - device driver adapter)

a, at load time

DDnneelllCCCCCCCCCCCCCCCCCCCCCCCCC

*/

/* vim:set ts=4 sw=4 et: */

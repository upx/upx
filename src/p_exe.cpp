/* p_exe.cpp --

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
#include "p_exe.h"
#include "linker.h"

static const
#include "stub/i086-dos16.exe.h"

#define RSFCRI          4096    // Reserved Space For Compressed Relocation Info
#define MAXMATCH        0x2000
#define MAXRELOCS       (0x8000-MAXMATCH)

#define DI_LIMIT        0xff00  // see the assembly why


/*************************************************************************
//
**************************************************************************/

PackExe::PackExe(InputFile *f) :
    super(f)
{
    bele = &N_BELE_RTP::le_policy;
    COMPILE_TIME_ASSERT(sizeof(exe_header_t) == 32);
    ih_exesize = ih_imagesize = ih_overlay = 0;
    stack_for_lzma = 0;
    use_clear_dirty_stack = false;

    // disable lzma for "--brute" unless explicitly given "--lzma"
    if (opt->all_methods_use_lzma && !opt->method_lzma_seen)
        opt->all_methods_use_lzma = false;
}


const int *PackExe::getCompressionMethods(int method, int level) const
{
    bool small = ih_imagesize <= 256*1024;
    return Packer::getDefaultCompressionMethods_8(method, level, small);
}


const int *PackExe::getFilters() const
{
    return NULL;
}


int PackExe::fillExeHeader(struct exe_header_t *eh) const
{
#define oh  (*eh)
    // fill new exe header
    int flag = 0;
    if (!opt->dos_exe.no_reloc && !M_IS_LZMA(ph.method))
        flag |= USEJUMP;
    if (ih.relocs == 0)
        flag |= NORELOC;

    memset(&oh,0,sizeof(oh));
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
    oh.ss = ph.c_len/16 + destpara;
    if (ih.ss*16 + ih.sp < 0x100000 && ih.ss > oh.ss && ih.sp > 0x200)
        oh.ss = ih.ss;
    if (oh.ss*16 + 0x50 < ih.ss*16 + ih.sp
        && oh.ss*16 + 0x200 > ih.ss*16 + ih.sp)
        oh.ss += 0x20;

    if (oh.ss != ih.ss)
        flag |= SS;
    if (oh.sp != ih.sp || M_IS_LZMA(ph.method))
        flag |= SP;
    return flag;
#undef oh
}

void PackExe::addLoaderEpilogue(int flag)
{
    addLoader("EXEMAIN5", NULL);
    if (relocsize)
        addLoader(ph.u_len <= DI_LIMIT || (ph.u_len & 0x7fff) >= relocsize ? "EXENOADJ" : "EXEADJUS",
                  "EXERELO1",
                  has_9a ? "EXEREL9A" : "",
                  "EXERELO2",
                  ih_exesize > 0xFE00 ? "EXEREBIG" : "",
                  "EXERELO3",
                  NULL
                 );
    addLoader("EXEMAIN8",
              device_driver ? "DEVICEEND" : "",
              (flag & SS) ? "EXESTACK" : "",
              (flag & SP) ? "EXESTASP" : "",
              (flag & USEJUMP) ? "EXEJUMPF" : "",
              NULL
             );
    if (!(flag & USEJUMP))
        addLoader(ih.cs ? "EXERCSPO" : "",
                  "EXERETIP",
                  NULL
                 );

    linker->defineSymbol("original_cs", ih.cs);
    linker->defineSymbol("original_ip", ih.ip);
    linker->defineSymbol("original_sp", ih.sp);
    linker->defineSymbol("original_ss", ih.ss);
    linker->defineSymbol("reloc_size",
                         (ph.u_len <= DI_LIMIT || (ph.u_len & 0x7fff)
                          >= relocsize ? 0 : MAXRELOCS) - relocsize);
}

void PackExe::buildLoader(const Filter *)
{
    // get flag
    exe_header_t dummy_oh;
    int flag = fillExeHeader(&dummy_oh);

    initLoader(stub_i086_dos16_exe, sizeof(stub_i086_dos16_exe));

    if (M_IS_LZMA(ph.method))
    {
        addLoader("LZMA_DEC00",
                  opt->small ? "LZMA_DEC10" : "LZMA_DEC20",
                  "LZMA_DEC30",
                  use_clear_dirty_stack ? "LZMA_DEC31" : "",
                  "LZMA_DEC32",
                  ph.u_len > 0xffff ? "LZMA_DEC33" : "",
                  NULL
                 );

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
        int r = upx_compress(loader, lsize, compressed_lzma, &c_len_lzma,
                             NULL, M_NRV2B_LE16, 9, NULL, NULL);
        assert(r == UPX_E_OK); assert(c_len_lzma < lsize);

        info("lzma+relocator code compressed: %u -> %u", lsize, c_len_lzma);
        // reinit the loader
        initLoader(stub_i086_dos16_exe, sizeof(stub_i086_dos16_exe));
        // prepare loader
        if (device_driver)
            addLoader("DEVICEENTRY,LZMADEVICE,DEVICEENTRY2", NULL);

        linker->addSection("COMPRESSED_LZMA", compressed_lzma, c_len_lzma, 0);
        addLoader("LZMAENTRY,NRV2B160,NRVDDONE,NRVDECO1,NRVGTD00,NRVDECO2",
                  NULL);

    }
    else if (device_driver)
        addLoader("DEVICEENTRY,DEVICEENTRY2", NULL);

    addLoader("EXEENTRY",
              M_IS_LZMA(ph.method) && device_driver ? "LONGSUB" : "SHORTSUB",
              "JNCDOCOPY",
              relocsize ? "EXERELPU" : "",
              "EXEMAIN4",
              M_IS_LZMA(ph.method) ? "" : "EXEMAIN4B",
              "EXEMAIN4C",
              M_IS_LZMA(ph.method) ? "COMPRESSED_LZMA_START,COMPRESSED_LZMA" : "",
              "+G5DXXXX,UPX1HEAD,EXECUTPO",
              NULL
             );
    if (ph.method == M_NRV2B_8)
        addLoader("NRV2B16S",               // decompressor
                  ph.u_len > DI_LIMIT ? "N2B64K01" : "",
                  "NRV2BEX1",
                  opt->cpu == opt->CPU_8086 ? "N2BX8601" : "N2B28601",
                  "NRV2BEX2",
                  opt->cpu == opt->CPU_8086 ? "N2BX8602" : "N2B28602",
                  "NRV2BEX3",
                  ph.c_len > 0xffff ? "N2B64K02" : "",
                  "NRV2BEX9",
                  NULL
                 );
    else if (ph.method == M_NRV2D_8)
        addLoader("NRV2D16S",
                  ph.u_len > DI_LIMIT ? "N2D64K01" : "",
                  "NRV2DEX1",
                  opt->cpu == opt->CPU_8086 ? "N2DX8601" : "N2D28601",
                  "NRV2DEX2",
                  opt->cpu == opt->CPU_8086 ? "N2DX8602" : "N2D28602",
                  "NRV2DEX3",
                  ph.c_len > 0xffff ? "N2D64K02" : "",
                  "NRV2DEX9",
                  NULL
                 );
    else if (ph.method == M_NRV2E_8)
        addLoader("NRV2E16S",
                  ph.u_len > DI_LIMIT ? "N2E64K01" : "",
                  "NRV2EEX1",
                  opt->cpu == opt->CPU_8086 ? "N2EX8601" : "N2E28601",
                  "NRV2EEX2",
                  opt->cpu == opt->CPU_8086 ? "N2EX8602" : "N2E28602",
                  "NRV2EEX3",
                  ph.c_len > 0xffff ? "N2E64K02" : "",
                  "NRV2EEX9",
                  NULL
                 );
    else if M_IS_LZMA(ph.method)
        return;
    else
        throwInternalError("unknown compression method");

    addLoaderEpilogue(flag);
}


/*************************************************************************
//
**************************************************************************/

int PackExe::readFileHeader()
{
    ih_exesize = ih_imagesize = ih_overlay = 0;
    fi->readx(&ih,sizeof(ih));
    if (ih.ident != 'M' + 'Z'*256 && ih.ident != 'Z' + 'M'*256)
        return 0;
    ih_exesize = ih.m512 + ih.p512*512 - (ih.m512 ? 512 : 0);
    ih_imagesize = ih_exesize - ih.headsize16*16;
    ih_overlay = file_size - ih_exesize;
    if (ih.m512+ih.p512*512u < sizeof (ih))
        throwCantPack("illegal exe header");
    if (file_size < (off_t)ih_exesize || ih_imagesize <= 0 || ih_imagesize > ih_exesize)
        throwCantPack("exe header corrupted");
#if 0
    printf("dos/exe header: %d %d %d\n", ih_exesize, ih_imagesize, ih_overlay);
#endif
    return UPX_F_DOS_EXE;
}


bool PackExe::canPack()
{
    if (fn_has_ext(fi->getName(),"sys"))
        return false;
    if (!readFileHeader())
        return false;
    if (file_size < 1024)
        throwCantPack("file is too small");
    fi->seek(0x3c,SEEK_SET);
    LE32 offs;
    fi->readx(&offs,sizeof (offs));
    if (ih.relocoffs >= 0x40 && offs)
    {
        if (opt->dos_exe.force_stub)
            opt->overlay = opt->COPY_OVERLAY;
        else
            throwCantPack("can't pack new-exe");
    }
    return true;
}


/*************************************************************************
//
**************************************************************************/

static
unsigned optimize_relocs(upx_byte *b, const unsigned size,
                         const upx_byte *relocs, const unsigned nrelocs,
                         upx_byte *crel, bool *has_9a)
{
    if (opt->exact)
        throwCantPackExact();

    upx_byte * const crel_save = crel;
    unsigned i;
    unsigned seg_high = 0;
#if 0
    unsigned seg_low = 0xffffffff;
    unsigned off_low = 0xffffffff;
    unsigned off_high = 0;
    unsigned linear_low = 0xffffffff;
    unsigned linear_high = 0;
#endif

    // pass 1 - find 0x9a bounds
    for (i = 0; i < nrelocs; i++)
    {
        unsigned addr = get_le32(relocs+4*i);
        if (addr >= size - 1)
            throwCantPack("unexpected relocation 1");
        if (addr >= 3 && b[addr-3] == 0x9a)
        {
            unsigned seg = get_le16(b+addr);
            if (seg > seg_high)
                seg_high = seg;
#if 0
            if (seg < seg_low)
                seg_low = seg;
            unsigned off = get_le16(b+addr-2);
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
    //printf("%d %d\n", seg_low, seg_high);
    //printf("%d %d\n", off_low, off_high);
    //printf("%d %d\n", linear_low, linear_high);


    // pass 2 - reloc

    crel += 4; // to be filled in later

    unsigned ones = 0;
    unsigned es = 0, di, t;
    i = 0;
    do
    {
        unsigned addr = get_le32(relocs+4*i);
        set_le16(crel,di = addr & 0x0f);
        set_le16(crel+2,(addr >> 4) - es);
        es = addr >> 4;
        crel += 4;

        for (++i; i < nrelocs; i++)
        {
            addr = get_le32(relocs+4*i);
            //printf ("%x\n",es*16+di);
            if ((addr - es*16 > 0xfffe)
                || (i == nrelocs - 1 && addr - es * 16 > 0xff00)
               )
            {
                // segment change
                t = 1+(0xffff-di)/254;
                memset(crel,1,t);
                crel += t;
                ones += t-1; // -1 is used to help the assembly stuff
                break;
            }
            unsigned offs = addr - es*16;
            if (offs >= 3 && b[es*16 + offs-3] == 0x9a && offs > di + 3)
            {
                for (t = di; t < offs-3; t++)
                    if (b[es*16+t] == 0x9a && get_le16(b+es*16+t+3) <= seg_high)
                        break;
                if (t == offs-3)
                {
                    // code 0: search for 0x9a
                    *crel++ = 0;
                    di = offs;
                    *has_9a = true;
                    continue;
                }
            }
            t = offs - di;
            if (t < 2)
                throwCantPack("unexpected relocation 2");

            while (t >= 256)
            {
                // code 1: add 254, don't reloc
                *crel++ = 1;
                t -= 254;
                ones++;
            }
            *crel++ = (unsigned char) t;
            di = offs;
        }
    } while (i < nrelocs);
    *crel++ = 1;
    ones++;
    set_le16 (crel_save,ones);
    set_le16 (crel_save+2,seg_high);

    //OutputFile::dump("x.rel", crel_save, crel - crel_save);
    return (unsigned) (crel - crel_save);
}


/*************************************************************************
//
**************************************************************************/

void PackExe::pack(OutputFile *fo)
{
    unsigned ic;

    if (ih.relocs > MAXRELOCS)
        throwCantPack("too many relocations");
    checkOverlay(ih_overlay);

    // alloc buffers
    relocsize = RSFCRI + 4*ih.relocs;
    ibuf.alloc(ih_imagesize+16+relocsize+2);
    obuf.allocForCompression(ih_imagesize+16+relocsize+2);

    // read image
    fi->seek(ih.headsize16*16,SEEK_SET);
    fi->readx(ibuf,ih_imagesize);

    checkAlreadyPacked(ibuf, UPX_MIN(ih_imagesize, 127u));

    device_driver = get_le32(ibuf) == 0xffffffffu;

    // relocations
    has_9a = false;
    upx_byte *w = ibuf + ih_imagesize;
    if (ih.relocs)
    {
        upx_byte *wr = w + RSFCRI;

        fi->seek(ih.relocoffs,SEEK_SET);
        fi->readx(wr,4*ih.relocs);

        for (ic = 0; ic < ih.relocs; ic++)
        {
            unsigned jc = get_le32(wr+4*ic);
            set_le32(wr+4*ic, ((jc>>16)*16+(jc&0xffff)) & 0xfffff);
        }
        qsort(wr,ih.relocs,4,le32_compare);
        relocsize = optimize_relocs(ibuf, ih_imagesize, wr, ih.relocs, w, &has_9a);
        set_le16(w+relocsize, relocsize+2);
        relocsize += 2;
        if (relocsize > MAXRELOCS)
            throwCantPack("too many relocations");
#if 0
        upx_byte out[9*relocsize/8+1024];
        unsigned in_len = relocsize;
        unsigned out_len = 0;
        ucl_nrv2b_99_compress(w, in_len, out, &out_len, NULL, 9, NULL, NULL);
        printf("reloc compress: %d -> %d\n", in_len, out_len);
#endif
    }
    else
    {
        relocsize = 0;
    }

    // prepare packheader
    ph.u_len = ih_imagesize + relocsize;
    // prepare filter
    Filter ft(ph.level);
    // compress (max_match = 8192)
    upx_compress_config_t cconf; cconf.reset();
    cconf.conf_ucl.max_match = MAXMATCH;
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 32, &cconf);

    if (M_IS_NRV2B(ph.method) || M_IS_NRV2D(ph.method) || M_IS_NRV2E(ph.method))
        if (ph.max_run_found + ph.max_match_found > 0x8000)
            throwCantPack("decompressor limit exceeded, send a bugreport");

#ifdef TESTING
    if (opt->debug.debug_level)
    {
        printf("image+relocs %d -> %d\n",ih_imagesize+relocsize,ph.c_len);
        printf("offsets: %d - %d\nmatches: %d - %d\nruns: %d - %d\n",
               0/*ph.min_offset_found*/,ph.max_offset_found,
               0/*ph.min_match_found*/,ph.max_match_found,
               0/*ph.min_run_found*/,ph.max_run_found);
    }
#endif

    int flag = fillExeHeader(&oh);

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    //OutputFile::dump("xxloader.dat", loader, lsize);

    // patch loader
    const unsigned packedsize = ph.c_len;
    const unsigned e_len = getLoaderSectionStart("EXECUTPO");
    const unsigned d_len = lsize - e_len;
    assert((e_len&15) == 0);

    const unsigned copysize = (1+packedsize+d_len) & ~1;
    const unsigned firstcopy = copysize%0x10000 ? copysize%0x10000 : 0x10000;

    // set oh.min & oh.max
    ic = ih.min*16 + ih_imagesize;
    if (ic < oh.ss*16u + oh.sp)
        ic = oh.ss*16u + oh.sp;
    oh.min = (ic - (packedsize + lsize)) / 16;
    ic = oh.min + (ih.max - ih.min);
    oh.max = ic < 0xffff && ih.max != 0xffff ? ic : 0xffff;

    // set extra info
    unsigned char extra_info[9];
    unsigned eisize = 0;
    if (oh.ss != ih.ss)
    {
        set_le16(extra_info+eisize,ih.ss);
        eisize += 2;
        assert((flag & SS) != 0);   // set in fillExeHeader()
    }
    if (oh.sp != ih.sp)
    {
        set_le16(extra_info+eisize,ih.sp);
        eisize += 2;
        assert((flag & SP) != 0);   // set in fillExeHeader()
    }
    if (ih.min != oh.min)
    {
        set_le16(extra_info+eisize,ih.min);
        eisize += 2;
        flag |= MINMEM;
    }
    if (ih.max != oh.max)
    {
        set_le16(extra_info+eisize,ih.max);
        eisize += 2;
        flag |= MAXMEM;
    }
    extra_info[eisize++] = (unsigned char) flag;

    if (M_IS_NRV2B(ph.method) || M_IS_NRV2D(ph.method) || M_IS_NRV2E(ph.method))
        linker->defineSymbol("bx_magic", 0x7FFF + 0x10 * ((packedsize & 15) + 1));

    unsigned decompressor_entry = 1 + (packedsize & 15);
    if (M_IS_LZMA(ph.method))
        decompressor_entry = 0x10;
    linker->defineSymbol("decompressor_entry", decompressor_entry);

    // patch loader
    if (flag & USEJUMP)
    {
        // I use a relocation entry to set the original cs
        unsigned n = getLoaderSectionStart("EXEJUMPF") + 1;
        n += packedsize + 2;
        oh.relocs = 1;
        oh.firstreloc = (n & 0xf) + ((n >> 4) << 16);
    }
    else
    {
        oh.relocs = 0;
        oh.firstreloc = ih.cs * 0x10000 + ih.ip;
    }

    // g++ 3.1 does not like the following line...
//    oh.relocoffs = offsetof(exe_header_t, firstreloc);
    oh.relocoffs = ptr_diff(&oh.firstreloc, &oh);

    linker->defineSymbol("destination_segment", oh.ss - ph.c_len / 16 - e_len / 16);
    linker->defineSymbol("source_segment", e_len / 16 + (copysize - firstcopy) / 16);
    linker->defineSymbol("copy_offset", firstcopy - 2);
    linker->defineSymbol("words_to_copy",firstcopy / 2);

    linker->defineSymbol("exe_stack_sp", oh.sp);
    linker->defineSymbol("exe_stack_ss", oh.ss);
    linker->defineSymbol("interrupt", get_le16(ibuf + 8));
    linker->defineSymbol("attribute", get_le16(ibuf + 4));
    linker->defineSymbol("orig_strategy", get_le16(ibuf + 6));

    const unsigned outputlen = sizeof(oh)+lsize+packedsize+eisize;
    oh.m512 = outputlen & 511;
    oh.p512 = (outputlen + 511) >> 9;

    const char *exeentry = M_IS_LZMA(ph.method) ? "LZMAENTRY" : "EXEENTRY";
    oh.ip = device_driver ? getLoaderSection(exeentry) - 2 : 0;

    defineDecompressorSymbols();
    relocateLoader();
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader,e_len);

//fprintf(stderr,"\ne_len=%x d_len=%x c_len=%x oo=%x ulen=%x destp=%x copys=%x images=%x",e_len,d_len,packedsize,ph.overlap_overhead,ph.u_len,destpara,copysize,ih_imagesize);

    // write header + write loader + compressed file
#ifdef TESTING
    if (opt->debug.debug_level)
        printf("\n%d %d %d %d\n",(int)sizeof(oh),e_len,packedsize,d_len);
#endif
    fo->write(&oh,sizeof(oh));
    fo->write(loader,e_len);            // entry
    fo->write(obuf,packedsize);
    fo->write(loader+e_len,d_len);      // decompressor
    fo->write(extra_info,eisize);
    assert(eisize <= 9);
#if 0
    printf("%-13s: program hdr  : %8ld bytes\n", getName(), (long) sizeof(oh));
    printf("%-13s: entry        : %8ld bytes\n", getName(), (long) e_len);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) packedsize);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) d_len);
    printf("%-13s: extra info   : %8ld bytes\n", getName(), (long) eisize);
#endif

    // verify
    verifyOverlappingDecompression();

    // copy the overlay
    copyOverlay(fo, ih_overlay, &obuf);
//fprintf (stderr,"%x %x\n",relocsize,ph.u_len);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
//
**************************************************************************/

int PackExe::canUnpack()
{
    if (!readFileHeader())
        return false;
    const off_t off = ih.headsize16 * 16;
    fi->seek(off, SEEK_SET);
    bool b = readPackHeader(4096);
    return b && (off + (off_t) ph.c_len <= file_size);
}


/*************************************************************************
//
**************************************************************************/

void PackExe::unpack(OutputFile *fo)
{
    ibuf.alloc(file_size);
    obuf.allocForUncompression(ph.u_len);

    // read the file
    fi->seek(ih.headsize16*16,SEEK_SET);
    fi->readx(ibuf,ih_imagesize);

    // get compressed data offset
    unsigned e_len = ph.buf_offset + ph.getPackHeaderSize();
    if (ih_imagesize <= e_len + ph.c_len)
        throwCantUnpack("file damaged");

    checkOverlay(ih_overlay);

    // decompress
    decompress(ibuf+e_len,obuf);

    unsigned imagesize = ih_imagesize;
    imagesize--;
    const unsigned char flag = ibuf[imagesize];

    unsigned relocn = 0;
    upx_byte *relocs = obuf + ph.u_len;

    MemBuffer wrkmem;
    if (!(flag & NORELOC))
    {
        relocs -= get_le16(obuf+ph.u_len-2);
        ph.u_len -= 2;

        wrkmem.alloc(4*MAXRELOCS);
        unsigned es = 0, ones = get_le16(relocs);
        const unsigned seghi = get_le16(relocs+2);
        const upx_byte *p = relocs + 4;

        while (ones)
        {
            unsigned di = get_le16(p);
            es += get_le16(p+2);
            bool dorel = true;
            for (p += 4; ones && di < 0x10000; p++)
            {
                if (dorel)
                {
                    set_le16(wrkmem+4*relocn,di);
                    set_le16(wrkmem+2+4*relocn++,es);
                    //printf ("%x\n",es*16+di);
                }
                dorel = true;
                if (*p == 0)
                {
                    const upx_byte *q;
                    for (q = obuf+es*16+di; !(*q == 0x9a && get_le16(q+3) <= seghi); q++)
                        ;
                    di = ptr_diff(q, obuf+es*16) + 3;
                }
                else if (*p == 1)
                {
                    di += 254;
                    if (di < 0x10000)
                        ones--;
                    dorel = false;
                }
                else
                    di += *p;
            }
        }
    }

    // fill new exe header
    memset(&oh,0,sizeof(oh));
    oh.ident = 'M' + 'Z'*256;

    if (relocn)
    {
        oh.relocs = relocn;
        while (relocn & 3)
            set_le32(wrkmem+4*relocn++,0);
    }

    unsigned outputlen = ptr_diff(relocs, obuf) + sizeof(oh) + relocn*4;
    oh.m512 = outputlen & 511;
    oh.p512 = (outputlen + 511) >> 9;
    oh.headsize16 = 2+relocn/4;

    oh.max = ih.max;
    oh.min = ih.min;
    oh.sp = ih.sp;
    oh.ss = ih.ss;

    if (flag & MAXMEM)
        { imagesize -= 2; oh.max = get_le16(ibuf+imagesize); }
    if (flag & MINMEM)
        { imagesize -= 2; oh.min = get_le16(ibuf+imagesize); }
    if (flag & SP)
        { imagesize -= 2; oh.sp = get_le16(ibuf+imagesize); }
    if (flag & SS)
        { imagesize -= 2; oh.ss = get_le16(ibuf+imagesize); }

    unsigned ip = (flag & USEJUMP) ? get_le32(ibuf+imagesize-4) : (unsigned) ih.firstreloc;
    oh.ip = ip & 0xffff;
    oh.cs = ip >> 16;

    oh.relocoffs = sizeof(oh);
    oh.firstreloc = 0;
    if (!fo)
        return;

    // write header + relocations + uncompressed file
    fo->write(&oh,sizeof(oh));
    if (relocn)
        fo->write(wrkmem,relocn*4);
    fo->write(obuf, ptr_diff(relocs, obuf));

    // copy the overlay
    copyOverlay(fo, ih_overlay, &obuf);
}


Linker* PackExe::newLinker() const
{
    return new ElfLinkerX86();
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


/*
vi:ts=4:et
*/


/* p_exe.cpp --

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
#include "filter.h"
#include "packer.h"
#include "p_exe.h"

static const
#include "stub/l_exe.h"

#define RSFCRI          4096    // Reserved Space For Compressed Relocation Info
#define MAXMATCH        0x2000
#define MAXRELOCS       (0x8000-MAXMATCH)


/*************************************************************************
//
**************************************************************************/

PackExe::PackExe(InputFile *f) :
    super(f)
{
    assert(sizeof(exe_header_t) == 32);
    ih_exesize = ih_imagesize = ih_overlay = 0;
}


int PackExe::getCompressionMethod() const
{
    if (M_IS_NRV2B(opt->method))
        return M_NRV2B_8;
    if (M_IS_NRV2D(opt->method))
        return M_NRV2D_8;
    return opt->level > 1 && ih_imagesize >= 300000 ? M_NRV2D_8 : M_NRV2B_8;
}


const int *PackExe::getFilters() const
{
    return NULL;
}


/*************************************************************************
//
**************************************************************************/

bool PackExe::readFileHeader()
{
    ih_exesize = ih_imagesize = ih_overlay = 0;
    fi->readx(&ih,sizeof(ih));
    if (ih.ident != 'M' + 'Z'*256 && ih.ident != 'Z' + 'M'*256)
        return false;
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
    return true;
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
        if (opt->dos.force_stub)
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
    upx_byte *crel_save = crel;
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
            if (addr - es*16 > 0xfffe)
            {
                // segment change
                t = 1+(0xffff-di)/254;
                memset(crel,1,t);
                crel += t;
                ones += t-1; // -1 is used to help the assembly stuff
                break;
            }
            unsigned offs = addr - es*16;
            if (offs >= 3 && b[es*16 + offs-3] == 0x9a)
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

#if 0 // def TESTING
    //if (opt->debug >= 3)
    {
        FILE *f1=fopen ("x.rel","wb");
        fwrite (crel_save,crel-crel_save,1,f1);
        fclose (f1);
    }
#endif
    return crel - crel_save;
}


/*************************************************************************
//
**************************************************************************/

void PackExe::pack(OutputFile *fo)
{
    unsigned ic;
    unsigned char flag = 0;

    char extra_info[32];
    unsigned eisize = 0;

    //
    const unsigned exesize = ih_exesize;
    const unsigned imagesize = ih_imagesize;
    const unsigned overlay = ih_overlay;
    if (ih.relocs > MAXRELOCS)
        throwCantPack("too many relocations");
    checkOverlay(overlay);

    // alloc buffers
    unsigned relocsize = RSFCRI + 4*ih.relocs;

    ibuf.alloc(imagesize+16+relocsize+2);
    obuf.allocForCompression(imagesize+16+relocsize+2);

    // read image
    fi->seek(ih.headsize16*16,SEEK_SET);
    fi->readx(ibuf,imagesize);

    if (find_le32(ibuf,imagesize < 127 ? imagesize : 127, UPX_MAGIC_LE32))
        throwAlreadyPacked();

    // relocations
    has_9a = false;
    upx_byte *w = ibuf + imagesize;
    if (ih.relocs)
    {
        upx_byte *wr = w + RSFCRI;

        fi->seek(ih.relocoffs,SEEK_SET);
        fi->readx(wr,4*ih.relocs);

        for (ic = 0; ic < ih.relocs; ic++)
        {
            unsigned jc = get_le32(wr+4*ic);
            set_le32(wr+4*ic, (jc>>16)*16+(jc&0xffff));
        }
        qsort(wr,ih.relocs,4,le32_compare);
        relocsize = optimize_relocs(ibuf, imagesize, wr, ih.relocs, w, &has_9a);
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
        flag |= NORELOC;
        relocsize = 0;
    }

    ph.u_len = imagesize + relocsize;
    if (!compress(ibuf,obuf,0,MAXMATCH))
        throwNotCompressible();
    const unsigned overlapoh = findOverlapOverhead(obuf,32);

    if (ph.max_run_found + ph.max_match_found > 0x8000)
        throwCantPack("decompressor limit exceeded, send a bugreport");
#ifdef TESTING
    if (opt->debug)
    {
        printf("image+relocs %d -> %d\n",imagesize+relocsize,ph.c_len);
        printf("offsets: %d - %d\nmatches: %d - %d\nruns: %d - %d\n",
               ph.min_offset_found,ph.max_offset_found,
               ph.min_match_found,ph.max_match_found,
               ph.min_run_found,ph.max_run_found);
    }
#endif
    const unsigned packedsize = ph.c_len;

    if (!opt->dos.no_reloc)
        flag |= USEJUMP;

    // fill new exe header
    memset(&oh,0,sizeof(oh));
    oh.ident = 'M' + 'Z' * 256;

    unsigned destpara = (ph.u_len+overlapoh-packedsize+31)/16;

    oh.ss = packedsize/16 + destpara;
    if (ih.ss*16 + ih.sp < 0x100000 && ih.ss > oh.ss && ih.sp > 0x200)
        oh.ss = ih.ss;
    oh.sp = ih.sp > 0x200 ? ih.sp : 0x200;
    if (oh.ss*16u + 0x50 < ih.ss*16u + ih.sp
        && oh.ss*16u + 0x200 > ih.ss*16u + ih.sp)
        oh.ss += 0x20;

    destpara = oh.ss - packedsize/16;
    if (oh.ss != ih.ss)
    {
        set_le16(extra_info+eisize,ih.ss);
        eisize += 2;
        flag |= SS;
    }
    if (oh.sp != ih.sp)
    {
        set_le16(extra_info+eisize,ih.sp);
        eisize += 2;
        flag |= SP;
    }

#define DI_LIMIT 0xff00  // see the assembly why
    // prepare loader
    initLoader(nrv_loader,sizeof(nrv_loader));
    addLoader("EXEENTRY",
              relocsize ? "EXERELPU" : "",
              "EXEMAIN4""+G5DXXXX""UPX1HEAD""EXECUTPO",
              NULL
             );
    if (ph.method == M_NRV2B_8)
        addLoader("NRV2B16S",               // decompressor
                  ph.u_len > DI_LIMIT ? "NDIGT64K" : "",
                  "NRV2BEX1",
                  opt->cpu == opt->CPU_8086 ? "N2BX8601" : "N2B28601",
                  "NRV2BEX2",
                  opt->cpu == opt->CPU_8086 ? "N2BX8602" : "N2B28602",
                  "NRV2BEX3",
                  packedsize > 0xffff ? "NSIGT64K" : "",
                  "NRV2BEX9""NRV2B16E",
                  NULL
                 );
    else if (ph.method == M_NRV2D_8)
        addLoader("NRV2D16S",
                  ph.u_len > DI_LIMIT ? "NDIGT64D" : "",
                  "NRV2DEX1",
                  opt->cpu == opt->CPU_8086 ? "N2DX8601" : "N2D28601",
                  "NRV2DEX2",
                  opt->cpu == opt->CPU_8086 ? "N2DX8602" : "N2D28602",
                  "NRV2DEX3",
                  packedsize > 0xffff ? "NSIGT64D" : "",
                  "NRV2DEX9""NRV2D16E",
                  NULL
                 );
    else
        throwInternalError("unknown compression method");
    addLoader("EXEMAIN5", NULL);
    if (relocsize)
        addLoader(ph.u_len <= DI_LIMIT || (ph.u_len & 0x7fff) >= relocsize ? "EXENOADJ" : "EXEADJUS",
                  "EXERELO1",
                  has_9a ? "EXEREL9A" : "",
                  "EXERELO2",
                  exesize > 0xFE00 ? "EXEREBIG" : "",
                  "EXERELO3",
                  NULL
                 );
    addLoader("EXEMAIN8",
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
    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    //OutputFile::dump("xxloader.dat", loader, lsize);

    // patch loader
    putPackHeader(loader,lsize);
    const unsigned e_len = getLoaderSection("EXECUTPO");
    const unsigned d_len = lsize - e_len;
    assert((e_len&15) == 0);

    const unsigned copysize = (1+packedsize+d_len) & ~1;
    const unsigned firstcopy = copysize%0x10000 ? copysize%0x10000 : 0x10000;

    oh.headsize16 = 2;
    oh.ip = 0;

    ic = ih.min*16+imagesize;
    if (ic < oh.ss*16u + oh.sp)
        ic = oh.ss*16u + oh.sp;

    oh.min = (ic - (packedsize + lsize)) / 16;
    ic = ((unsigned) oh.min) + (ih.max - ih.min);
    oh.max = ic < 0xffff && ih.max != 0xffff ? ic : 0xffff;

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

    putPackHeader(loader,lsize);
    upx_bytep p = find_le32(loader,lsize,get_le32("IPCS"));
    if (p == NULL)
        throwBadLoader();
    if (flag & USEJUMP)
    {
        memcpy(p,&ih.ip,4);
    }
    else
    {
        patch_le16(loader,lsize,"IP",ih.ip);
        if (ih.cs)
            patch_le16(loader,lsize,"CS",ih.cs);
    }
    if (flag & SP)
        patch_le16(loader,lsize,"SP",ih.sp);
    if (flag & SS)
        patch_le16(loader,lsize,"SS",ih.ss);
    if (relocsize)
        patch_le16(loader,lsize,"RS",(ph.u_len <= DI_LIMIT || (ph.u_len & 0x7fff) >= relocsize ? 0 : MAXRELOCS) - relocsize);

    patch_le16(loader,e_len,"BX",0x800F + 0x10*((packedsize&15)+1) - 0x10);
    patch_le16(loader,e_len,"BP",(packedsize&15)+1);

    patch_le16(loader,e_len,"ES",destpara-e_len/16);
    patch_le16(loader,e_len,"DS",e_len/16+(copysize-firstcopy)/16);
    patch_le16(loader,e_len,"SI",firstcopy-2);
    patch_le16(loader,e_len,"CX",firstcopy/2);

    // finish --stub support
    //if (ih.relocoffs >= 0x40 && memcmp(&ih.relocoffs,">TIPPACH",8))
    //    throwCantPack("FIXME");
    // I use a relocation entry to set the original cs
    oh.relocs = (flag & USEJUMP) ? 1 : 0;
    oh.relocoffs = (char*)(&oh.firstreloc)-(char*)&oh;
    oh.firstreloc = (p-loader) + packedsize + 2;
    oh.firstreloc = (oh.firstreloc&0xf)+((oh.firstreloc>>4)<<16);
    if (!(flag & USEJUMP))
        oh.firstreloc = ih.cs*0x10000 + ih.ip;

    extra_info[eisize++] = flag;
    const unsigned outputlen = sizeof(oh)+lsize+packedsize+eisize;
    oh.m512 = outputlen & 511;
    oh.p512 = (outputlen + 511) >> 9;

//fprintf(stderr,"\ne_len=%x d_len=%x clen=%x oo=%x ulen=%x destp=%x copys=%x images=%x",e_len,d_len,packedsize,overlapoh,ph.u_len,destpara,copysize,imagesize);

    // write header + write loader + compressed file
#ifdef TESTING
    if (opt->debug)
        printf("\n%d %d %d %d\n",(int)sizeof(oh),e_len,packedsize,d_len);
#endif
    fo->write(&oh,sizeof(oh));
    fo->write(loader,e_len);            // entry
    fo->write(obuf,packedsize);
    fo->write(loader+e_len,d_len);      // decompressor
    fo->write(extra_info,eisize);

    // verify
    verifyOverlappingDecompression(&obuf, overlapoh);

    // copy the overlay
    copyOverlay(fo, overlay, &obuf);
//fprintf (stderr,"%x %x\n",relocsize,ph.u_len);
}


/*************************************************************************
//
**************************************************************************/

int PackExe::canUnpack()
{
    if (!readFileHeader())
        return false;
    const off_t off = ih.headsize16*16;
    bool b = readPackHeader(128, off);
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
    unsigned imagesize = ih_imagesize;
    const unsigned overlay = ih_overlay;

    fi->seek(ih.headsize16*16,SEEK_SET);
    fi->readx(ibuf,imagesize);

    // get compressed data offset
    unsigned e_len = ph.buf_offset + ph.getPackHeaderSize();
    if (imagesize <= e_len + ph.c_len)
        throwCantUnpack("file damaged");

    checkOverlay(overlay);

    // decompress
    decompress(ibuf+e_len,obuf);

    const unsigned char flag = ibuf[imagesize-1];

    unsigned relocn = 0;
    upx_byte *relocs = obuf + ph.u_len;

    MemBuffer wrkmem;
    if (!(flag & NORELOC))
    {
        relocs -= get_le16(obuf+ph.u_len-2);
        ph.u_len -= 2;
        upx_byte *p;

        wrkmem.alloc(4*MAXRELOCS);
        unsigned es = 0,ones = get_le16(relocs);
        unsigned seghi = get_le16(relocs+2);
        p = relocs + 4;

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
                    upx_byte *q;
                    for (q = obuf+es*16+di; !(*q == 0x9a && get_le16(q+3) <= seghi); q++)
                        ;
                    di = q - (obuf+es*16) + 3;
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

    oh.relocs = relocn;
    while (relocn&3)
        set_le32(wrkmem+4*relocn++,0);

    unsigned outputlen = sizeof(oh)+relocn*4+relocs-obuf;
    oh.m512 = outputlen & 511;
    oh.p512 = (outputlen + 511) >> 9;
    oh.headsize16 = 2+relocn/4;

    imagesize--;
    oh.max = ih.max;
    oh.min = ih.min;
    oh.sp = ih.sp;
    oh.ss = ih.ss;

    if (flag & MAXMEM)
        imagesize -= 2, oh.max = get_le16(ibuf+imagesize);
    if (flag & MINMEM)
        imagesize -= 2, oh.min = get_le16(ibuf+imagesize);
    if (flag & SP)
        imagesize -= 2, oh.sp = get_le16(ibuf+imagesize);
    if (flag & SS)
        imagesize -= 2, oh.ss = get_le16(ibuf+imagesize);

    unsigned ip = (flag & USEJUMP) ? get_le32(ibuf+imagesize-4) : ih.firstreloc;
    oh.ip = ip & 0xffff;
    oh.cs = ip >> 16;

    oh.relocoffs = sizeof(oh);
    oh.firstreloc = 0;
    if (!fo)
        return;

    // write header + relocations + uncompressed file
    fo->write(&oh,sizeof(oh));
    fo->write(wrkmem,relocn*4);
    fo->write(obuf,relocs-obuf);

    // copy the overlay
    copyOverlay(fo, overlay, &obuf);
}


/*
vi:ts=4:et
*/


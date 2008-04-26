/* p_wcle.cpp --

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
#include "lefile.h"
#include "p_wcle.h"
#include "linker.h"

static const
#include "stub/i386-dos32.watcom.le.h"

#define LEOF_READ       (1<<0)
#define LEOF_WRITE      (1<<1)
#define LEOF_EXEC       (1<<2)
#define LEOF_PRELOAD    (1<<6)
#define LEOF_HUGE32     (1<<13)

#define IOT(x,y)        iobject_table[x].y
#define OOT(x,y)        oobject_table[x].y

#define LE_STUB_EDI     (1)

#ifdef TESTING
# define dputc(x,y)     do { if (opt->debug.debug_level) putc(x,y); } while (0)
# define Opt_debug      opt->debug.debug_level
#else
# define dputc(x,y)     ((void)0)
# define Opt_debug      0
#endif

#define my_base_address reserved
#define objects         ih.object_table_entries
#define pages           ih.memory_pages
#define mps             ih.memory_page_size
#define opages          oh.memory_pages


/*************************************************************************
//
**************************************************************************/

const int *PackWcle::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackWcle::getFilters() const
{
    static const int filters[] = {
        0x26, 0x24, 0x49, 0x46, 0x16, 0x13, 0x14, 0x11,
        FT_ULTRA_BRUTE, 0x25, 0x15, 0x12,
    FT_END };
    return filters;
}


Linker* PackWcle::newLinker() const
{
    return new ElfLinkerX86;
}


void PackWcle::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_i386_dos32_watcom_le, sizeof(stub_i386_dos32_watcom_le));
    addLoader("IDENTSTR,WCLEMAIN",
              ph.first_offset_found == 1 ? "WCLEMAIN02" : "",
              "WCLEMAIN03,UPX1HEAD,WCLECUTP", NULL);

    // fake alignment for the start of the decompressor
    linker->defineSymbol("WCLECUTP", 0x1000);

    addLoader(getDecompressorSections(), "WCLEMAI2", NULL);
    if (ft->id)
    {
        assert(ft->calls > 0);
        addLoader(ft->addvalue ? "WCCTTPOS" : "WCCTTNUL", NULL);
        addFilter32(ft->id);
    }
#if 1
    // FIXME: if (has_relocation)
    {
        addLoader("WCRELOC1,RELOC320",
                  big_relocs ? "REL32BIG" : "",
                  "RELOC32J",
                  NULL
                 );
    }
#endif
    addLoader(has_extra_code ? "WCRELSEL" : "",
              "WCLEMAI4",
              NULL
             );
}


/*************************************************************************
// util
**************************************************************************/

void PackWcle::handleStub(OutputFile *fo)
{
    if (fo && !opt->watcom_le.le)
        Packer::handleStub(fi,fo,le_offset);
}


bool PackWcle::canPack()
{
    if (!LeFile::readFileHeader())
        return false;
    return true;
}


/*************************************************************************
//
**************************************************************************/

// IDEA: as all the entries go into object #1, I could create bundles with 255
// elements (of course I still have to handle empty bundles)

void PackWcle::encodeEntryTable()
{
    unsigned count,object,n;
    upx_byte *p = ientries;
    n = 0;
    while (*p)
    {
        count = *p;
        n += count;
        if (p[1] == 0) // unused bundle
            p += 2;
        else if (p[1] == 3) // 32-bit bundle
        {
            object = get_le16(p+2)-1;
            set_le16(p+2,1);
            p += 4;
            for (; count; count--, p += 5)
                set_le32(p+1,IOT(object,my_base_address) + get_le32(p+1));
        }
        else
            throwCantPack("unsupported bundle type in entry table");
    }

    //if (Opt_debug) printf("%d entries encoded.\n",n);
    UNUSED(n);

    soentries = ptr_diff(p, ientries) + 1;
    oentries = ientries;
    ientries = NULL;
}


void PackWcle::readObjectTable()
{
    LeFile::readObjectTable();

    // temporary copy of the object descriptors
    iobject_desc.alloc(objects*sizeof(*iobject_table));
    memcpy(iobject_desc,iobject_table,objects*sizeof(*iobject_table));

    unsigned ic,jc,virtual_size;

    for (ic = jc = virtual_size = 0; ic < objects; ic++)
    {
        jc += IOT(ic,npages);
        IOT(ic,my_base_address) = virtual_size;
        virtual_size += (IOT(ic,virtual_size)+mps-1) &~ (mps-1);
    }
    if (pages != jc)
        throwCantPack("bad page number");
}


void PackWcle::encodeObjectTable()
{
    unsigned ic,jc;

    oobject_table = new le_object_table_entry_t[soobject_table = 2];
    memset(oobject_table,0,soobject_table * sizeof(*oobject_table));

    // object #1:
    OOT(0,base_address) = IOT(0,base_address);

    ic = IOT(objects-1,my_base_address)+IOT(objects-1,virtual_size);
    jc = pages*mps+sofixups+1024;
    if (ic < jc)
        ic = jc;

    unsigned csection = (ic + ph.overlap_overhead + mps-1) &~ (mps-1);

    OOT(0,virtual_size) = csection + mps;
    OOT(0,flags) = LEOF_READ|LEOF_EXEC|LEOF_HUGE32|LEOF_PRELOAD;
    OOT(0,pagemap_index) = 1;
    OOT(0,npages) = opages;

    // object #2: stack
    OOT(1,base_address) = (OOT(0,base_address)
                                      +OOT(0,virtual_size)+mps-1) & ~(mps-1);
    OOT(1,virtual_size) = mps + getDecompressorWrkmemSize();
    OOT(1,flags) = LEOF_READ|LEOF_HUGE32|LEOF_WRITE;
    OOT(1,pagemap_index) = 1;

    oh.init_cs_object = 1;
    oh.init_eip_offset = neweip;
    oh.init_ss_object = 2;
    oh.init_esp_offset = OOT(1,virtual_size);
    oh.automatic_data_object = 2;
}


void PackWcle::encodePageMap()
{
    opm_entries = new le_pagemap_entry_t[sopm_entries = opages];
    for (unsigned ic = 0; ic < sopm_entries; ic++)
    {
        opm_entries[ic].l = (unsigned char) (ic+1);
        opm_entries[ic].m = (unsigned char) ((ic+1)>>8);
        opm_entries[ic].h = 0;
        opm_entries[ic].type = 0;
    }
}


void PackWcle::encodeFixupPageTable()
{
    unsigned ic;
    ofpage_table = new unsigned[sofpage_table = 1 + opages];
    for (ofpage_table[0] = ic = 0; ic < opages; ic++)
        set_le32(ofpage_table+ic+1,sofixups-FIXUP_EXTRA);
}


void PackWcle::encodeFixups()
{
    ofixups = new upx_byte[sofixups = 1*7 + FIXUP_EXTRA];
    memset(ofixups,0,sofixups);
    ofixups[0] = 7;
    set_le16(ofixups+2,(LE_STUB_EDI + neweip) & (mps-1));
    ofixups[4] = 1;
}


void PackWcle::preprocessFixups()
{
    big_relocs = 0;

    unsigned ic,jc;

    Array(unsigned, counts, objects+2);
    countFixups(counts);

    for (ic = jc = 0; ic < objects; ic++)
        jc += counts[ic];

    if (jc == 0)
    {
        // FIXME: implement this
        throwCantPack("files without relocations are not supported");
    }

    ByteArray(rl, jc);
    ByteArray(srf, counts[objects+0]+1);
    ByteArray(slf, counts[objects+1]+1);

    upx_byte *selector_fixups = srf;
    upx_byte *selfrel_fixups = slf;
    unsigned rc = 0;

    upx_byte *fix = ifixups;
    for (ic = jc = 0; ic < pages; ic++)
    {
        while ((unsigned)(fix - ifixups) < get_le32(ifpage_table+ic+1))
        {
            const int fixp2 = get_le16_signed(fix+2);
            unsigned value;

            switch (*fix)
            {
                case 2:       // selector fixup
                    if (fixp2 < 0)
                    {
                        // cross page selector fixup
                        dputc('S',stdout);
                        fix += 5;
                        break;
                    }
                    dputc('s',stdout);
                    memcpy(selector_fixups,"\x8C\xCB\x66\x89\x9D",5); // mov bx, cs ; mov [xxx+ebp], bx
                    if (IOT(fix[4]-1,flags) & LEOF_WRITE)
                        selector_fixups[1] = 0xDB; // ds
                    set_le32(selector_fixups+5,jc+fixp2);
                    selector_fixups += 9;
                    fix += 5;
                    break;
                case 5:       // 16-bit offset
                    if ((unsigned)fixp2 < 4096 && IOT(fix[4]-1,my_base_address) == jc)
                        dputc('6',stdout);
                    else
                        throwCantPack("unsupported 16-bit offset relocation");
                    fix += (fix[1] & 0x10) ? 9 : 7;
                    break;
                case 6:       // 16:32 pointer
                    if (fixp2 < 0)
                    {
                        // cross page pointer fixup
                        dputc('P',stdout);
                        fix += (fix[1] & 0x10) ? 9 : 7;
                        break;
                    }
                    dputc('p',stdout);
                    memcpy(iimage+jc+fixp2,fix+5,(fix[1] & 0x10) ? 4 : 2);
                    set_le32(rl+4*rc++,jc+fixp2);
                    set_le32(iimage+jc+fixp2,get_le32(iimage+jc+fixp2)+IOT(fix[4]-1,my_base_address));

                    memcpy(selector_fixups,"\x8C\xCA\x66\x89\x95",5);
                    if (IOT(fix[4]-1,flags) & LEOF_WRITE)
                        selector_fixups[1] = 0xDA; // ds
                    set_le32(selector_fixups+5,jc+fixp2+4);
                    selector_fixups += 9;
                    fix += (fix[1] & 0x10) ? 9 : 7;
                    break;
                case 7:       // 32-bit offset
                    if (fixp2 < 0)
                    {
                        fix += (fix[1] & 0x10) ? 9 : 7;
                        break;
                    }
                    //if (memcmp(iimage+jc+fixp2,fix+5,(fix[1] & 0x10) ? 4 : 2))
                    //    throwCantPack("illegal fixup offset");

                    // work around an pmwunlite bug: remove duplicated fixups
                    // FIXME: fix the other cases too
                    if (rc == 0 || get_le32(rl+4*rc-4) != jc+fixp2)
                    {
                        set_le32(rl+4*rc++,jc+fixp2);
                        set_le32(iimage+jc+fixp2,get_le32(iimage+jc+fixp2)+IOT(fix[4]-1,my_base_address));
                    }
                    fix += (fix[1] & 0x10) ? 9 : 7;
                    break;
                case 8:       // 32-bit self relative fixup
                    if (fixp2 < 0)
                    {
                        // cross page self relative fixup
                        dputc('R',stdout);
                        fix += (fix[1] & 0x10) ? 9 : 7;
                        break;
                    }
                    value = get_le32(fix+5);
                    if (fix[1] == 0)
                        value &= 0xffff;
                    set_le32(iimage+jc+fixp2,(value+IOT(fix[4]-1,my_base_address))-jc-fixp2-4);
                    set_le32(selfrel_fixups,jc+fixp2);
                    selfrel_fixups += 4;
                    dputc('r',stdout);
                    fix += (fix[1] & 0x10) ? 9 : 7;
                    break;
                default:
                    throwCantPack("unsupported fixup record");
            }
        }
        jc += mps;
    }

    // resize ifixups if it's too small
    if (sofixups < 1000)
    {
        delete[] ifixups;
        ifixups = new upx_byte[1000];
    }
    fix = optimizeReloc32 (rl,rc,ifixups,iimage,1,&big_relocs);
    has_extra_code = srf != selector_fixups;
    // FIXME: this could be removed if has_extra_code = false
    // but then we'll need a flag
    *selector_fixups++ = 0xC3; // ret
    memcpy(fix,srf,selector_fixups-srf); // copy selector fixup code
    fix += selector_fixups-srf;

    memcpy(fix,slf,selfrel_fixups-slf); // copy self-relative fixup positions
    fix += selfrel_fixups-slf;
    set_le32(fix,0xFFFFFFFFUL);
    fix += 4;

    sofixups = ptr_diff(fix, ifixups);
}


#define RESERVED 0x1000
void PackWcle::encodeImage(Filter *ft)
{
    // concatenate image & preprocessed fixups
    unsigned isize = soimage + sofixups;
    ibuf.alloc(isize);
    memcpy(ibuf,iimage,soimage);
    memcpy(ibuf+soimage,ifixups,sofixups);

    delete[] ifixups; ifixups = NULL;

    oimage.allocForCompression(isize, RESERVED+512);
    // prepare packheader
    ph.u_len = isize;
    // prepare filter [already done]
    // compress
    upx_compress_config_t cconf; cconf.reset();
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(ibuf, isize,
                        oimage + RESERVED,
                        ibuf + ft->addvalue, ft->buf_len,
                        NULL, 0,
                        ft, 512, &cconf, 0);

    ibuf.dealloc();
    soimage = ph.c_len;
    while (soimage & 3)
        oimage[RESERVED + soimage++] = 0;
}


void PackWcle::pack(OutputFile *fo)
{
    handleStub(fo);

    if (ih.byte_order || ih.word_order
        || ih.exe_format_level
        || ih.cpu_type < 2 || ih.cpu_type > 5
        || ih.target_os != 1
        || ih.module_type != 0x200
        || ih.object_iterate_data_map_offset
        || ih.resource_entries
        || ih.module_directives_entries
        || ih.imported_modules_count
        || ih.object_table_entries > 255)
        throwCantPack("watcom/le: unexpected value in header");

    readObjectTable();
    readPageMap();
    readResidentNames();
    readEntryTable();
    readFixupPageTable();
    readFixups();
    readImage();
    readNonResidentNames();

//    if (find_le32(iimage,20,get_le32("UPX ")) >= 0)
    if (find_le32(iimage,UPX_MIN(soimage,256u),UPX_MAGIC_LE32) >= 0)
        throwAlreadyPacked();

    if (ih.init_ss_object != objects)
        throwCantPack("the stack is not in the last object");

    preprocessFixups();

    const unsigned text_size = IOT(ih.init_cs_object-1,npages) * mps;
    const unsigned text_vaddr = IOT(ih.init_cs_object-1,my_base_address);

    // attach some useful data at the end of preprocessed fixups
    ifixups[sofixups++] = (unsigned char) ih.automatic_data_object;
    unsigned ic = objects*sizeof(*iobject_table);
    memcpy(ifixups+sofixups,iobject_desc,ic);
    iobject_desc.dealloc();

    sofixups += ic;
    set_le32(ifixups+sofixups,ih.init_esp_offset+IOT(ih.init_ss_object-1,my_base_address)); // old stack pointer
    set_le32(ifixups+sofixups+4,ih.init_eip_offset+text_vaddr); // real entry point
    set_le32(ifixups+sofixups+8,mps*pages); // virtual address of unpacked relocations
    ifixups[sofixups+12] = (unsigned char) (unsigned) objects;
    sofixups += 13;

    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = text_size;
    ft.addvalue = text_vaddr;
    // compress
    encodeImage(&ft);

    const unsigned lsize = getLoaderSize();
    neweip = getLoaderSection("WCLEMAIN");
    int e_len = getLoaderSectionStart("WCLECUTP");
    const unsigned d_len = lsize - e_len;
    assert(e_len > 0 && e_len < RESERVED);

    memmove(oimage+e_len,oimage+RESERVED,soimage);
    soimage += lsize;

    opages = (soimage+mps-1)/mps;
    oh.bytes_on_last_page = soimage%mps;

    encodeObjectTable();
    encodeFixups();
    encodeFixupPageTable();
    encodePageMap();
    encodeEntryTable();

    encodeResidentNames();
    encodeNonResidentNames();

    // patch loader
    ic = (OOT(0,virtual_size) - d_len) &~ 15;
    assert(ic > ((ph.u_len + ph.overlap_overhead + 31) &~ 15));

    linker->defineSymbol("WCLECUTP", ic);

    linker->defineSymbol("original_entry", ih.init_eip_offset + text_vaddr);
    linker->defineSymbol("original_stack", ih.init_esp_offset +
                         IOT(ih.init_ss_object - 1, my_base_address));
    linker->defineSymbol("start_of_relocs", mps*pages);
    defineDecompressorSymbols();
    defineFilterSymbols(&ft);
    linker->defineSymbol("filter_buffer_start", text_vaddr);

    unsigned jpos = (((ph.c_len + 3) &~ 3) + d_len + 3) / 4;
    linker->defineSymbol("words_to_copy", jpos);
    linker->defineSymbol("copy_dest", ((ic + d_len + 3) &~ 3) - 4);
    linker->defineSymbol("copy_source", e_len + jpos * 4 - 4);

    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

    memcpy(oimage, loader, e_len);
    memcpy(oimage + soimage - d_len, loader + e_len, d_len);

    writeFile(fo, opt->watcom_le.le);

    // verify
    verifyOverlappingDecompression(oimage + e_len, oimage.getSize() - e_len);

    // copy the overlay
    const unsigned overlaystart = ih.data_pages_offset + exe_offset
        + getImageSize();
    const unsigned overlay = file_size - overlaystart - ih.non_resident_name_table_length;
    checkOverlay(overlay);
    copyOverlay(fo, overlay, &oimage);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
//
**************************************************************************/

void PackWcle::decodeFixups()
{
    upx_byte *p = oimage + soimage;

    iimage.dealloc();

    MemBuffer tmpbuf;
    unsigned fixupn = unoptimizeReloc32(&p,oimage,&tmpbuf,1);

    MemBuffer wrkmem(8*fixupn+8);
    unsigned ic,jc,o,r;
    for (ic=0; ic<fixupn; ic++)
    {
        jc=get_le32(tmpbuf+4*ic);
        set_le32(wrkmem+ic*8,jc);
        o = soobject_table;
        r = get_le32(oimage+jc);
        virt2rela(oobject_table,&o,&r);
        set_le32(wrkmem+ic*8+4,OOT(o-1,my_base_address));
        set_le32(oimage+jc,r);
    }
    set_le32(wrkmem+ic*8,0xFFFFFFFF);     // end of 32-bit offset fixups
    tmpbuf.dealloc();

    // selector fixups and self-relative fixups
    const upx_byte *selector_fixups = p;
    const upx_byte *selfrel_fixups = p;

    while (*selfrel_fixups != 0xC3)
        selfrel_fixups += 9;
    selfrel_fixups++;
    unsigned selectlen = ptr_diff(selfrel_fixups, selector_fixups)/9;

    ofixups = new upx_byte[fixupn*9+1000+selectlen*5];
    upx_bytep fp = ofixups;

    for (ic = 1, jc = 0; ic <= opages; ic++)
    {
        // self relative fixups
        while ((r = get_le32(selfrel_fixups))/mps == ic-1)
        {
            fp[0] = 8;
            set_le16(fp+2,r & (mps-1));
            o = 4+get_le32(oimage+r);
            set_le32(oimage+r,0);
            r += o;
            o = soobject_table;
            virt2rela(oobject_table,&o,&r);
            fp[4] = (unsigned char) o;
            set_le32(fp+5,r);
            fp[1] = (unsigned char) (r > 0xFFFF ? 0x10 : 0);
            fp += fp[1] ? 9 : 7;
            selfrel_fixups += 4;
            dputc('r',stdout);
        }
        // selector fixups
        while (selectlen && (r = get_le32(selector_fixups+5))/mps == ic-1)
        {
            fp[0] = 2;
            fp[1] = 0;
            set_le16(fp+2,r & (mps-1));
            unsigned x = selector_fixups[1] > 0xD0 ? oh.init_ss_object : oh.init_cs_object;
            fp[4] = (unsigned char) x;
            fp += 5;
            selector_fixups += 9;
            selectlen--;
            dputc('s',stdout);
        }
        // 32 bit offset fixups
        while (get_le32(wrkmem+4*jc) < ic*mps)
        {
            if (ic > 1 && ((get_le32(wrkmem+4*(jc-2))+3) & (mps-1)) < 3) // cross page fixup?
            {
                r = get_le32(oimage+get_le32(wrkmem+4*(jc-2)));
                fp[0] = 7;
                fp[1] = (unsigned char) (r > 0xFFFF ? 0x10 : 0);
                set_le16(fp+2,get_le32(wrkmem+4*(jc-2)) | ~3);
                set_le32(fp+5,r);
                o = soobject_table;
                r = get_le32(wrkmem+4*(jc-1));
                virt2rela(oobject_table,&o,&r);
                fp[4] = (unsigned char) o;
                fp += fp[1] ? 9 : 7;
                dputc('0',stdout);
            }
            o = soobject_table;
            r = get_le32(wrkmem+4*(jc+1));
            virt2rela(oobject_table,&o,&r);
            r = get_le32(oimage+get_le32(wrkmem+4*jc));
            fp[0] = 7;
            fp[1] = (unsigned char) (r > 0xFFFF ? 0x10 : 0);
            set_le16(fp+2,get_le32(wrkmem+4*jc) & (mps-1));
            fp[4] = (unsigned char) o;
            set_le32(fp+5,r);
            fp += fp[1] ? 9 : 7;
            jc += 2;
        }
        set_le32(ofpage_table+ic,ptr_diff(fp,ofixups));
    }
    for (ic=0; ic < FIXUP_EXTRA; ic++)
        *fp++ = 0;
    sofixups = ptr_diff(fp, ofixups);
}


void PackWcle::decodeFixupPageTable()
{
    ofpage_table = new unsigned[sofpage_table = 1 + opages];
    set_le32(ofpage_table,0);
    // the rest of ofpage_table is filled by decodeFixups()
}


void PackWcle::decodeObjectTable()
{
    soobject_table = oimage[ph.u_len - 1];
    oobject_table = new le_object_table_entry_t[soobject_table];
    unsigned jc, ic = soobject_table * sizeof(*oobject_table);

    const unsigned extradata = ph.version == 10 ? 17 : 13;
    memcpy(oobject_table,oimage + ph.u_len - extradata - ic,ic);
    if (ph.version >= 12)
        oh.automatic_data_object = oimage[ph.u_len - ic - 14];

    for (ic = jc = 0; ic < soobject_table; ic++)
    {
        OOT(ic,my_base_address) = jc;
        jc += (OOT(ic,virtual_size)+mps-1) &~ (mps-1);
    }

    // restore original cs:eip & ss:esp
    ic = soobject_table;
    jc = get_le32(oimage + ph.u_len - (ph.version < 11 ? 13 : 9));
    virt2rela(oobject_table,&ic,&jc);
    oh.init_cs_object = ic;
    oh.init_eip_offset = jc;

    ic = soobject_table;
    if (ph.version < 10)
        jc = ih.init_esp_offset;
    else
        jc = get_le32(oimage + ph.u_len - (ph.version == 10 ? 17 : 13));
    virt2rela(oobject_table,&ic,&jc);
    oh.init_ss_object = ic;
    oh.init_esp_offset = jc;
}


void PackWcle::decodeImage()
{
    oimage.allocForUncompression(ph.u_len);

    decompress(iimage + ph.buf_offset + ph.getPackHeaderSize(),oimage);
    soimage = get_le32(oimage + ph.u_len - 5);
    opages = soimage / mps;
    oh.memory_page_size = mps;
}


void PackWcle::decodeEntryTable()
{
    unsigned count,object,n,r;
    upx_byte *p = ientries;
    n = 0;
    while (*p)
    {
        count = *p;
        n += count;
        if (p[1] == 0) // unused bundle
            p += 2;
        else if (p[1] == 3) // 32-bit offset bundle
        {
            object = get_le16(p+2);
            if (object != 1)
                throwCantUnpack("corrupted entry found");
            object = soobject_table;
            r = get_le32(p+5);
            virt2rela(oobject_table,&object,&r);
            set_le16(p+2,object--);
            p += 4;
            for (; count; count--, p += 5)
                set_le32(p+1,get_le32(p+1) - OOT(object,my_base_address));
        }
        else
            throwCantUnpack("unsupported bundle type in entry table");
    }

    //if (Opt_debug) printf("\n%d entries decoded.\n",n);

    soentries = ptr_diff(p, ientries) + 1;
    oentries = ientries;
    ientries = NULL;
}


int PackWcle::canUnpack()
{
    if (!LeFile::readFileHeader())
        return false;
    fi->seek(exe_offset + ih.data_pages_offset, SEEK_SET);
    // FIXME: 1024 could be too large for some files
    //int len = 1024;
    int len = UPX_MIN(getImageSize(), 256u);
    return readPackHeader(len) ? 1 : -1;
}


void PackWcle::virt2rela(const le_object_table_entry_t *entr,unsigned *objn,unsigned *addr)
{
    for (; *objn > 1; objn[0]--)
    {
        if (entr[*objn-1].my_base_address > *addr)
            continue;
        *addr -= entr[*objn-1].my_base_address;
        break;
    }
}


/*************************************************************************
//
**************************************************************************/

void PackWcle::unpack(OutputFile *fo)
{
    handleStub(fo);

    readObjectTable();
    iobject_desc.dealloc();
    readPageMap();
    readResidentNames();
    readEntryTable();
    readFixupPageTable();
    readFixups();
    readImage();
    readNonResidentNames();

    decodeImage();
    decodeObjectTable();

    // unfilter
    if (ph.filter)
    {
        const unsigned text_size = OOT(oh.init_cs_object-1,npages) * mps;
        const unsigned text_vaddr = OOT(oh.init_cs_object-1,my_base_address);

        Filter ft(ph.level);
        ft.init(ph.filter, text_vaddr);
        ft.cto = (unsigned char) (ph.version < 11 ? (get_le32(oimage+ph.u_len-9) >> 24) : ph.filter_cto);
        ft.unfilter(oimage+text_vaddr, text_size);
    }

    decodeFixupPageTable();
    decodeFixups();
    decodeEntryTable();
    decodePageMap();
    decodeResidentNames();
    decodeNonResidentNames();

    for (unsigned ic = 0; ic < soobject_table; ic++)
        OOT(ic,my_base_address) = 0;

    while (oimage[soimage-1] == 0)
        soimage--;
    oh.bytes_on_last_page = soimage % mps;

    // write decompressed file
    if (fo)
        writeFile(fo, opt->watcom_le.le);

    // copy the overlay
    const unsigned overlaystart = ih.data_pages_offset + exe_offset
        + getImageSize();
    const unsigned overlay = file_size - overlaystart - ih.non_resident_name_table_length;
    checkOverlay(overlay);
    copyOverlay(fo, overlay, &oimage);
}


/*
vi:ts=4:et
*/


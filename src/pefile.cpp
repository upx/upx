/* pefile.cpp --

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
#include "pefile.h"
#include "linker.h"

#define FILLVAL         0

/*************************************************************************
//
**************************************************************************/

#if (WITH_SPAN >= 2) && 1
//#define IPTR(type, var)             Span<type> var(ibuf, ibuf.getSize(), ibuf)
//#define OPTR(type, var)             Span<type> var(obuf, obuf.getSize(), obuf)
#define IPTR_I_D(type, var, disp)   Span<type> var(ibuf + (disp), ibuf.getSize() - (disp), ibuf + (disp))
#define IPTR_I(type, var, first)    Span<type> var(first, ibuf)
#define OPTR_I(type, var, first)    Span<type> var(first, obuf)
#define IPTR_C(type, var, first)    const Span<type> var(first, ibuf)
#define OPTR_C(type, var, first)    const Span<type> var(first, obuf)
#else
#include "util/bptr.h"
//#define IPTR(type, var)             BoundedPtr<type> var(ibuf, ibuf.getSize())
//#define OPTR(type, var)             BoundedPtr<type> var(obuf, obuf.getSize())
#define IPTR_I_D(type, var, disp)   BoundedPtr<type> var(ibuf + (disp), ibuf.getSize() - (disp), ibuf + (disp))
#define IPTR_I(type, var, first)    BoundedPtr<type> var(ibuf, ibuf.getSize(), first)
#define OPTR_I(type, var, first)    BoundedPtr<type> var(obuf, obuf.getSize(), first)
#define IPTR_C(type, var, first)    const BoundedPtr<type> var(ibuf, ibuf.getSize(), first)
#define OPTR_C(type, var, first)    const BoundedPtr<type> var(obuf, obuf.getSize(), first)
#endif

static void xcheck(const void *p)
{
    if (!p)
        throwCantUnpack("xcheck unexpected nullptr pointer; take care!");
}
static void xcheck(const void *p, size_t plen, const void *b, size_t blen)
{
    const char *pp = (const char *) p;
    const char *bb = (const char *) b;
    if (pp < bb || pp > bb + blen || pp + plen > bb + blen)
        throwCantUnpack("xcheck pointer out of range; take care!");
}
#if 0
static void xcheck(size_t poff, size_t plen, const void *b, size_t blen)
{
    ACC_UNUSED(b);
    if (poff > blen || poff + plen > blen)
        throwCantUnpack("xcheck pointer out of range; take care!");
}
#endif
#define ICHECK(x, size)     xcheck(raw_bytes(x, 0), size, ibuf, ibuf.getSize())
#define OCHECK(x, size)     xcheck(raw_bytes(x, 0), size, obuf, obuf.getSize())

//#define imemset(a,b,c)      ICHECK(a,c), memset(a,b,c)
//#define omemset(a,b,c)      OCHECK(a,c), memset(a,b,c)
//#define imemcpy(a,b,c)      ICHECK(a,c), memcpy(a,b,c)
#define omemcpy(a,b,c)      OCHECK(a,c), memcpy(a,b,c)
#define omemmove(a,b,c)     OCHECK(a,c), memmove(a,b,c)


/*************************************************************************
//
**************************************************************************/

PeFile::PeFile(InputFile *f) : super(f)
{
    bele = &N_BELE_RTP::le_policy;
    COMPILE_TIME_ASSERT(sizeof(ddirs_t) == 8)
    COMPILE_TIME_ASSERT(sizeof(pe_section_t) == 40)
    COMPILE_TIME_ASSERT_ALIGNED1(ddirs_t)
    COMPILE_TIME_ASSERT_ALIGNED1(pe_section_t)
    COMPILE_TIME_ASSERT(RT_LAST == TABLESIZE(opt->win32_pe.compress_rt))

    isection = nullptr;
    oimport = nullptr;
    oimpdlls = nullptr;
    orelocs = nullptr;
    oexport = nullptr;
    otls = nullptr;
    oresources = nullptr;
    oxrelocs = nullptr;
    icondir_offset = 0;
    icondir_count = 0;
    importbyordinal = false;
    kernel32ordinal = false;
    tlsindex = 0;
    big_relocs = 0;
    sorelocs = 0;
    soxrelocs = 0;
    sotls = 0;
    isdll = false;
    ilinker = nullptr;
    use_tls_callbacks = false;
    oloadconf = nullptr;
    soloadconf = 0;

    use_dep_hack = true;
    use_clear_dirty_stack = true;
    use_stub_relocs = true;
    isrtm = false;
    isefi = false;
}


bool PeFile::testUnpackVersion(int version) const
{
    if (version != ph_version && ph_version != -1)
        throwCantUnpack("program has been modified; run a virus checker!");
    if (!canUnpackVersion(version))
        throwCantUnpack("this program is packed with an obsolete version and cannot be unpacked");
    return true;
}


/*************************************************************************
// util
**************************************************************************/

int PeFile::readFileHeader()
{
    struct alignas(1) exe_header_t {
        LE16 mz;
        LE16 m512;
        LE16 p512;
        char _[18];
        LE16 relocoffs;
        char __[34];
        LE32 nexepos;
    };

    COMPILE_TIME_ASSERT(sizeof(exe_header_t) == 64)
    COMPILE_TIME_ASSERT_ALIGNED1(exe_header_t)
    COMPILE_TIME_ASSERT(sizeof(((exe_header_t*)nullptr)->_)  == 18)
    COMPILE_TIME_ASSERT(sizeof(((exe_header_t*)nullptr)->__) == 34)

    exe_header_t h;
    int ic;
    pe_offset = 0;

    for (ic = 0; ic < 20; ic++)
    {
        fi->seek(pe_offset,SEEK_SET);
        fi->readx(&h,sizeof(h));

        if (h.mz == 'M' + 'Z'*256) // dos exe
        {
            if (h.nexepos && h.nexepos < sizeof(exe_header_t)) {
                // Overlapping MZ and PE headers by 'leanify', etc.
                char buf[64]; snprintf(buf, sizeof(buf),
                    "PE and MZ header overlap: %#x < %#x",
                    (unsigned)h.nexepos, (unsigned)sizeof(exe_header_t));
                throwCantPack(buf);
            }
            unsigned const delta = (h.relocoffs >= 0x40)
                ? h.nexepos // new format exe
                : (h.p512*512+h.m512 - h.m512 ? 512 : h.nexepos);

            if ((pe_offset + delta) < delta  // wrap-around
            ||  (pe_offset + delta) > (unsigned)file_size) {
                char buf[64]; snprintf(buf, sizeof(buf),
                    "bad PE delta %#x at offset %#x", delta, pe_offset);
                throwCantPack(buf);
            }
            pe_offset += delta;
        }
        else if (get_le32(&h) == 'P' + 'E'*256)
            break;
        else
            return 0;
    }
    if (ic == 20)
        return 0;
    fi->seek(pe_offset,SEEK_SET);
    readPeHeader();
    return getFormat();
}


/*************************************************************************
// interval handling
**************************************************************************/

PeFile::Interval::Interval(void *b) : capacity(0),base(b),ivarr(nullptr),ivnum(0)
{}

PeFile::Interval::~Interval()
{
    free(ivarr);
}

void PeFile::Interval::add(const void *start,unsigned len)
{
    add(ptr_diff_bytes(start,base),len);
}

void PeFile::Interval::add(const void *start,const void *end)
{
    add(ptr_diff_bytes(start,base),ptr_diff_bytes(end,start));
}

int __acc_cdecl_qsort PeFile::Interval::compare(const void *p1,const void *p2)
{
    const interval *i1 = (const interval*) p1;
    const interval *i2 = (const interval*) p2;
    if (i1->start < i2->start) return -1;
    if (i1->start > i2->start) return 1;
    if (i1->len < i2->len) return 1;
    if (i1->len > i2->len) return -1;
    return 0;
}

void PeFile::Interval::add(unsigned start,unsigned len)
{
    if (ivnum == capacity)
        ivarr = (interval*) realloc(ivarr,(capacity += 15) * sizeof (interval));
    ivarr[ivnum].start = start;
    ivarr[ivnum++].len = len;
}

void PeFile::Interval::add(const Interval *iv)
{
    for (unsigned ic = 0; ic < iv->ivnum; ic++)
        add(iv->ivarr[ic].start,iv->ivarr[ic].len);
}

void PeFile::Interval::flatten()
{
    if (!ivnum)
        return;
    qsort(ivarr,ivnum,sizeof (interval),Interval::compare);
    for (unsigned ic = 0; ic < ivnum - 1; ic++)
    {
        unsigned jc;
        for (jc = ic + 1; jc < ivnum && ivarr[ic].start + ivarr[ic].len >= ivarr[jc].start; jc++)
            if (ivarr[ic].start + ivarr[ic].len < ivarr[jc].start + ivarr[jc].len)
                ivarr[ic].len = ivarr[jc].start + ivarr[jc].len - ivarr[ic].start;
        if (jc > ic + 1)
        {
            memmove(ivarr + ic + 1, ivarr + jc,sizeof(interval) * (ivnum - jc));
            ivnum -= jc - ic - 1;
        }
    }
}

void PeFile::Interval::clear()
{
    for (unsigned ic = 0; ic < ivnum; ic++)
        memset((char*) base + ivarr[ic].start,0,ivarr[ic].len);
}

void PeFile::Interval::dump() const
{
    printf("%d intervals:\n",ivnum);
    for (unsigned ic = 0; ic < ivnum; ic++)
        printf("%x %x\n",ivarr[ic].start,ivarr[ic].len);
}


/*************************************************************************
// relocation handling
**************************************************************************/

struct alignas(1) PeFile::Reloc::reloc {
    LE32  pagestart;
    LE32  size;
};

void PeFile::Reloc::newRelocPos(void *p)
{
    rel = (reloc*) p;
    rel1 = (LE16*) ((char*) p + sizeof (reloc));
}

PeFile::Reloc::Reloc(upx_byte *s,unsigned si) :
    start(s), size(si), rel(nullptr), rel1(nullptr)
{
    COMPILE_TIME_ASSERT(sizeof(reloc) == 8)
    COMPILE_TIME_ASSERT_ALIGNED1(reloc)
    memset(counts,0,sizeof(counts));
    unsigned pos,type;
    while (next(pos,type))
        counts[type]++;
}

PeFile::Reloc::Reloc(unsigned rnum) :
    start(nullptr), size(0), rel(nullptr), rel1(nullptr)
{
    start = new upx_byte[mem_size(4, rnum, 8192)]; // => oxrelocs
    counts[0] = 0;
}

bool PeFile::Reloc::next(unsigned &pos,unsigned &type)
{
    if (!rel)
        newRelocPos(start);
    if (ptr_diff_bytes(rel, start) >= (int) size) {
        rel = nullptr; // rewind
        return false;
    }

    pos = rel->pagestart + (*rel1 & 0xfff);
    type = *rel1++ >> 12;
    //printf("%x %d\n",pos,type);
    if (ptr_diff_bytes(rel1,rel) >= (int) rel->size)
        newRelocPos(rel1);
    return type == 0 ? next(pos,type) : true;
}

void PeFile::Reloc::add(unsigned pos,unsigned type)
{
    set_le32(start + 1024 + 4 * counts[0]++,(pos << 4) + type);
}

void PeFile::Reloc::finish(upx_byte *&p,unsigned &siz)
{
    unsigned prev = 0xffffffff;
    set_le32(start + 1024 + 4 * counts[0]++,0xf0000000);
    qsort(start + 1024,counts[0],4,le32_compare);

    rel = (reloc*) start;
    rel1 = (LE16*) start;
    for (unsigned ic = 0; ic < counts[0]; ic++)
    {
        unsigned pos = get_le32(start + 1024 + 4 * ic);
        if ((pos ^ prev) >= 0x10000)
        {
            prev = pos;
            *rel1 = 0;
            rel->size = ALIGN_UP(ptr_diff_bytes(rel1,rel), 4);
            newRelocPos((char *)rel + rel->size);
            rel->pagestart = (pos >> 4) &~ 0xfff;
        }
        *rel1++ = (pos << 12) + ((pos >> 4) & 0xfff);
    }
    p = start;
    siz = ptr_udiff_bytes(rel1,start) &~ 3;
    siz -= 8;
    // siz can be 0 in 64-bit mode // assert(siz > 0);
    start = nullptr; // safety
}

void PeFile::processRelocs(Reloc *rel) // pass2
{
    rel->finish(oxrelocs,soxrelocs);
    if (opt->win32_pe.strip_relocs)
        soxrelocs = 0;
}

void PeFile32::processRelocs() // pass1
{
    big_relocs = 0;

    unsigned const take1 = IDSIZE(PEDIR_RELOC);
    unsigned const skip1 = IDADDR(PEDIR_RELOC);
    Reloc rel(ibuf.subref("bad reloc %#x", skip1, take1), take1);
    const unsigned *counts = rel.getcounts();
    unsigned rnum = 0;

    unsigned ic;
    for (ic = 1; ic < 16; ic++)
        rnum += counts[ic];

    if (opt->win32_pe.strip_relocs || rnum == 0)
    {
        if (IDSIZE(PEDIR_RELOC))
        {
            ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
            ih.objects = tryremove(IDADDR(PEDIR_RELOC), ih.objects);
        }
        mb_orelocs.alloc(1);
        orelocs = mb_orelocs;
        sorelocs = 0;
        return;
    }

    for (ic = 15; ic > 3; ic--)
        if (counts[ic])
            infoWarning("skipping unsupported relocation type %d (%d)",ic,counts[ic]);

    LE32 *fix[4];
    for (; ic; ic--)
        fix[ic] = New(LE32, counts[ic]);

    unsigned xcounts[4];
    memset(xcounts, 0, sizeof(xcounts));

    // prepare sorting
    unsigned pos,type;
    while (rel.next(pos,type))
    {
        if (pos >= ih.imagesize)
            continue;           // skip out-of-bounds record
        if (type < 4)
            fix[type][xcounts[type]++] = pos - rvamin;
    }

    // remove duplicated records
    for (ic = 1; ic <= 3; ic++)
    {
        qsort(fix[ic], xcounts[ic], 4, le32_compare);
        unsigned prev = ~0u;
        unsigned jc = 0;
        for (unsigned kc = 0; kc < xcounts[ic]; kc++)
            if (fix[ic][kc] != prev)
                prev = fix[ic][jc++] = fix[ic][kc];

        //printf("xcounts[%u] %u->%u\n", ic, xcounts[ic], jc);
        xcounts[ic] = jc;
    }

    // preprocess "type 3" relocation records
    for (ic = 0; ic < xcounts[3]; ic++)
    {
        pos = fix[3][ic] + rvamin;
        unsigned w = get_le32(ibuf.subref("bad reloc type 3 %#x", pos, sizeof(LE32)));
        set_le32(ibuf + pos, w - ih.imagebase - rvamin);
    }

    ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
    mb_orelocs.alloc(mem_size(4, rnum, 1024));  // 1024 - safety
    orelocs = mb_orelocs;
    sorelocs = optimizeReloc32((upx_byte*) fix[3], xcounts[3],
                            orelocs, ibuf + rvamin, ibufgood - rvamin, true, &big_relocs);
    delete [] fix[3];

    // Malware that hides behind UPX often has PE header info that is
    // deliberately corrupt.  Sometimes it is even tuned to cause us trouble!
    // Use an extra check to avoid AccessViolation (SIGSEGV) when appending
    // the relocs into one array.
    if ((rnum * 4 + 1024) < (sorelocs + 4*(2 + xcounts[2] + xcounts[1])))
        throwCantUnpack("Invalid relocs");

    // append relocs type "LOW" then "HIGH"
    for (ic = 2; ic ; ic--)
    {
        memcpy(orelocs + sorelocs,fix[ic],4 * xcounts[ic]);
        sorelocs += 4 * xcounts[ic];
        delete [] fix[ic];

        set_le32(orelocs + sorelocs,0);
        if (xcounts[ic])
        {
            sorelocs += 4;
            big_relocs |= 2 * ic;
        }
    }
    info("Relocations: original size: %u bytes, preprocessed size: %u bytes",(unsigned) IDSIZE(PEDIR_RELOC),sorelocs);
}

// FIXME - this is too similar to PeFile32::processRelocs
void PeFile64::processRelocs() // pass1
{
    big_relocs = 0;

    unsigned const take = IDSIZE(PEDIR_RELOC);
    unsigned const skip = IDADDR(PEDIR_RELOC);
    Reloc rel(ibuf.subref("bad reloc %#x", skip, take), take);
    const unsigned *counts = rel.getcounts();
    unsigned rnum = 0;

    unsigned ic;
    for (ic = 1; ic < 16; ic++)
        rnum += counts[ic];

    if (opt->win32_pe.strip_relocs || rnum == 0)
    {
        if (IDSIZE(PEDIR_RELOC))
        {
            ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
            ih.objects = tryremove(IDADDR(PEDIR_RELOC), ih.objects);
        }
        mb_orelocs.alloc(1);
        orelocs = mb_orelocs;
        sorelocs = 0;
        return;
    }

    for (ic = 15; ic; ic--)
        if (ic != 10 && counts[ic])
            infoWarning("skipping unsupported relocation type %d (%d)",ic,counts[ic]);

    LE32 *fix[16];
    for (ic = 15; ic; ic--)
        fix[ic] = New(LE32, counts[ic]);

    unsigned xcounts[16];
    memset(xcounts, 0, sizeof(xcounts));

    // prepare sorting
    unsigned pos,type;
    while (rel.next(pos,type))
    {
        // FIXME add check for relocations which try to modify the
        // PE header or other relocation records

        if (pos >= ih.imagesize)
            continue;           // skip out-of-bounds record
        if (type < 16)
            fix[type][xcounts[type]++] = pos - rvamin;
    }

    // remove duplicated records
    for (ic = 1; ic <= 15; ic++)
    {
        qsort(fix[ic], xcounts[ic], 4, le32_compare);
        unsigned prev = ~0u;
        unsigned jc = 0;
        for (unsigned kc = 0; kc < xcounts[ic]; kc++)
            if (fix[ic][kc] != prev)
                prev = fix[ic][jc++] = fix[ic][kc];

        //printf("xcounts[%u] %u->%u\n", ic, xcounts[ic], jc);
        xcounts[ic] = jc;
    }

    // preprocess "type 10" relocation records
    for (ic = 0; ic < xcounts[10]; ic++)
    {
        pos = fix[10][ic] + rvamin;
        upx_uint64_t w = get_le64(ibuf.subref("bad reloc 10 %#x", pos, sizeof(LE64)));
        set_le64(ibuf + pos, w - ih.imagebase - rvamin);
    }

    ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
    mb_orelocs.alloc(mem_size(4, rnum, 1024));  // 1024 - safety
    orelocs = mb_orelocs;
    sorelocs = optimizeReloc64((upx_byte*) fix[10], xcounts[10],
                            orelocs, ibuf + rvamin, ibufgood - rvamin, true, &big_relocs);

    for (ic = 15; ic; ic--)
        delete [] fix[ic];

#if 0
    // Malware that hides behind UPX often has PE header info that is
    // deliberately corrupt.  Sometimes it is even tuned to cause us trouble!
    // Use an extra check to avoid AccessViolation (SIGSEGV) when appending
    // the relocs into one array.
    if ((rnum * 4 + 1024) < (sorelocs + 4*(2 + xcounts[2] + xcounts[1])))
        throwCantUnpack("Invalid relocs");

    // append relocs type "LOW" then "HIGH"
    for (ic = 2; ic ; ic--)
    {
        memcpy(orelocs + sorelocs,fix[ic],4 * xcounts[ic]);
        sorelocs += 4 * xcounts[ic];
        delete [] fix[ic];

        set_le32(orelocs + sorelocs,0);
        if (xcounts[ic])
        {
            sorelocs += 4;
            big_relocs |= 2 * ic;
        }
    }
#endif
    info("Relocations: original size: %u bytes, preprocessed size: %u bytes",(unsigned) IDSIZE(PEDIR_RELOC),sorelocs);
}

/*************************************************************************
// import handling
**************************************************************************/

LE32& PeFile::IDSIZE(unsigned x) { return iddirs[x].size; }
LE32& PeFile::IDADDR(unsigned x) { return iddirs[x].vaddr; }
LE32& PeFile::ODSIZE(unsigned x) { return oddirs[x].size; }
LE32& PeFile::ODADDR(unsigned x) { return oddirs[x].vaddr; }

/*
 ImportLinker: 32 and 64 bit import table building.
 Import entries (dll name + proc name/ordinal pairs) can be
 added in arbitrary order.

 Internally it works by creating sections with special names,
 and adding relocation entries between those sections. The special
 names ensure that when the import table is built in the memory
 from those sections, a correct table can be generated simply by
 sorting the sections by name, and adding all of them to the output
 in the sorted order.
 */

class PeFile::ImportLinker : public ElfLinkerAMD64
{
    struct tstr : private ::noncopyable
    {
        char *s = nullptr;
        explicit tstr(char *str) : s(str) {}
        ~tstr() { delete [] s; }
        operator char *() const { return s; }
    };

    // encoding of dll and proc names are required, so that our special
    // control characters in the name of sections can work as intended
    static char *encode_name(const char *name, char *buf)
    {
        char *b = buf;
        while (*name)
        {
            *b++ = 'a' + ((*name >> 4) & 0xf);
            *b++ = 'a' + (*name  & 0xf);
            name++;
        }
        *b = 0;
        return buf;
    }

    static char *name_for_dll(const char *dll, char first_char)
    {
        assert(dll);
        unsigned l = strlen(dll);
        assert(l > 0);

        char *name = New(char, 3 * l + 2);
        assert(name);
        name[0] = first_char;
        char *n = name + 1 + 2 * l;
        do {
            *n++ = tolower(*dll);
        } while(*dll++);
        return encode_name(name + 1 + 2 * l, name + 1) - 1;
    }

    static char *name_for_proc(const char *dll, const char *proc,
                               char first_char, char separator)
    {
        unsigned len = 1 + 2 * strlen(dll) + 1 + 2 * strlen(proc) + 1 + 1;
        tstr dlln(name_for_dll(dll, first_char));
        char *procn = New(char, len);
        upx_safe_snprintf(procn, len, "%s%c", (const char*) dlln, separator);
        encode_name(proc, procn + strlen(procn));
        return procn;
    }

    static const char zeros[sizeof(import_desc)];

    enum {
        // the order of identifiers is very important below!!
        descriptor_id = 'D',
        thunk_id,
        dll_name_id,
        proc_name_id,
        ordinal_id,

        thunk_separator_first,
        thunk_separator,
        thunk_separator_last,
        procname_separator,
    };

    unsigned thunk_size; // 4 or 8 bytes

    void add(const char *dll, const char *proc, unsigned ordinal)
    {
        tstr sdll(name_for_dll(dll, dll_name_id));
        tstr desc_name(name_for_dll(dll, descriptor_id));

        char tsep = thunk_separator;
        if (findSection(sdll, false) == nullptr)
        {
            tsep = thunk_separator_first;
            addSection(sdll, dll, strlen(dll) + 1, 0); // name of the dll
            addSymbol(sdll, sdll, 0);

            addSection(desc_name, zeros, sizeof(zeros), 0); // descriptor
            addRelocation(desc_name, offsetof(import_desc, dllname),
                          "R_X86_64_32", sdll, 0);
        }
        tstr thunk(proc == nullptr ? name_for_dll(dll, thunk_id)
                                   : name_for_proc(dll, proc, thunk_id, tsep));

        if (findSection(thunk, false) != nullptr)
            return; // we already have this dll/proc
        addSection(thunk, zeros, thunk_size, 0);
        addSymbol(thunk, thunk, 0);
        if (tsep == thunk_separator_first)
        {
            addRelocation(desc_name, offsetof(import_desc, iat),
                          "R_X86_64_32", thunk, 0);

            tstr last_thunk(name_for_proc(dll, "X", thunk_id, thunk_separator_last));
            addSection(last_thunk, zeros, thunk_size, 0);
        }

        const char *reltype = thunk_size == 4 ? "R_X86_64_32" : "R_X86_64_64";
        if (ordinal != 0u)
        {
            addRelocation(thunk, 0, reltype, "*UND*",
                          ordinal | (1ull << (thunk_size * 8 - 1)));
        }
        else if (proc != nullptr)
        {
            tstr proc_name(name_for_proc(dll, proc, proc_name_id, procname_separator));
            addSection(proc_name, zeros, 2, 1); // 2 bytes of word aligned "hint"
            addSymbol(proc_name, proc_name, 0);
            addRelocation(thunk, 0, reltype, proc_name, 0);

            strcat(proc_name, "X");
            addSection(proc_name, proc, strlen(proc), 0); // the name of the symbol
        }
        else
            infoWarning("empty import: %s", dll);
    }

    static int __acc_cdecl_qsort compare(const void *p1, const void *p2)
    {
        const Section *s1 = * (const Section * const *) p1;
        const Section *s2 = * (const Section * const *) p2;
        return strcmp(s1->name, s2->name);
    }

    virtual void alignCode(unsigned len) override { alignWithByte(len, 0); }

    const Section *getThunk(const char *dll, const char *proc, char tsep) const
    {
        assert(dll);
        assert(proc);
        tstr thunk(name_for_proc(dll, proc, thunk_id, tsep));
        return findSection(thunk, false);
    }

public:
    explicit ImportLinker(unsigned thunk_size_) : thunk_size(thunk_size_)
    {
        assert(thunk_size == 4 || thunk_size == 8);
        addSection("*UND*", nullptr, 0, 0);
        addSymbol("*UND*", "*UND*", 0);
        addSection("*ZSTART", nullptr, 0, 0);
        addSymbol("*ZSTART", "*ZSTART", 0);
        Section *s = addSection("Dzero", zeros, sizeof(import_desc), 0);
        assert(s->name[0] == descriptor_id);

        // one trailing 00 byte after the last proc name
        addSection("Zzero", zeros, 1, 0);
    }

    template <typename C>
    void add(const C *dll, unsigned ordinal)
    {
        ACC_COMPILE_TIME_ASSERT(sizeof(C) == 1)  // "char" or "unsigned char"
        assert(ordinal < 0x10000);
        char ord[1+5+1];
        upx_safe_snprintf(ord, sizeof(ord), "%c%05u", ordinal_id, ordinal);
        add((const char*) dll, ordinal ? ord : nullptr, ordinal);
    }

    template <typename C1, typename C2>
    void add(const C1 *dll, const C2 *proc)
    {
        ACC_COMPILE_TIME_ASSERT(sizeof(C1) == 1)  // "char" or "unsigned char"
        ACC_COMPILE_TIME_ASSERT(sizeof(C2) == 1)  // "char" or "unsigned char"
        assert(proc);
        add((const char*) dll, (const char*) proc, 0);
    }

    unsigned build()
    {
        assert(output == nullptr);
        int osize = 4 + 2 * nsections; // upper limit for alignments
        for (unsigned ic = 0; ic < nsections; ic++)
            osize += sections[ic]->size;
        output = New(upx_byte, output_capacity = osize);
        outputlen = 0;

        // sort the sections by name before adding them all
        qsort(sections, nsections, sizeof (Section*), ImportLinker::compare);

        for (unsigned ic = 0; ic < nsections; ic++)
            addLoader(sections[ic]->name);
        addLoader("+40D");
        assert(outputlen <= osize);

        //OutputFile::dump("il0.imp", output, outputlen);
        return outputlen;
    }

    void relocate_import(unsigned myimport)
    {
        assert(nsections > 0);
        assert(output);
        defineSymbol("*ZSTART", /*0xffffffffff1000ull + 0 * */ myimport);
        ElfLinkerAMD64::relocate();
        //OutputFile::dump("il1.imp", output, outputlen);
    }

    template <typename C1, typename C2>
    upx_uint64_t getAddress(const C1 *dll, const C2 *proc) const
    {
        ACC_COMPILE_TIME_ASSERT(sizeof(C1) == 1)  // "char" or "unsigned char"
        ACC_COMPILE_TIME_ASSERT(sizeof(C2) == 1)  // "char" or "unsigned char"
        const Section *s = getThunk((const char*) dll, (const char*) proc,
                                    thunk_separator_first);
        if (s == nullptr && (s = getThunk((const char*) dll,(const char*) proc,
                                       thunk_separator)) == nullptr)
            throwInternalError("entry not found");
        return s->offset;
    }

    template <typename C>
    upx_uint64_t getAddress(const C *dll, unsigned ordinal) const
    {
        ACC_COMPILE_TIME_ASSERT(sizeof(C) == 1)  // "char" or "unsigned char"
        assert(ordinal > 0 && ordinal < 0x10000);
        char ord[1+5+1];
        upx_safe_snprintf(ord, sizeof(ord), "%c%05u", ordinal_id, ordinal);

        const Section *s = getThunk((const char*) dll, ord, thunk_separator_first);
        if (s == nullptr
            && (s = getThunk((const char*) dll, ord, thunk_separator)) == nullptr)
            throwInternalError("entry not found");
        return s->offset;
    }

    template <typename C>
    upx_uint64_t getAddress(const C *dll) const
    {
        ACC_COMPILE_TIME_ASSERT(sizeof(C) == 1)  // "char" or "unsigned char"
        tstr sdll(name_for_dll((const char*) dll, dll_name_id));
        return findSection(sdll, true)->offset;
    }

    template <typename C>
    upx_uint64_t hasDll(const C *dll) const
    {
        ACC_COMPILE_TIME_ASSERT(sizeof(C) == 1)  // "char" or "unsigned char"
        tstr sdll(name_for_dll((const char*) dll, dll_name_id));
        return findSection(sdll, false) != nullptr;
    }
};
const char PeFile::ImportLinker::zeros[sizeof(import_desc)] = { 0 };

void PeFile::addKernelImport(const char *name)
{
    ilinker->add(kernelDll(), name);
}

void PeFile::addStubImports()
{
    addKernelImport("LoadLibraryA");
    addKernelImport("GetProcAddress");
    if (!isdll)
        addKernelImport("ExitProcess");
    addKernelImport("VirtualProtect");
}

void PeFile::processImports2(unsigned myimport, unsigned) // pass 2
{
    COMPILE_TIME_ASSERT(sizeof(import_desc) == 20)

    if (!ilinker)
        return;

    ilinker->relocate_import(myimport);
    int len;
    oimpdlls = ilinker->getLoader(&len);
    assert(len == (int) soimpdlls);
    //OutputFile::dump("x1.imp", oimpdlls, soimpdlls);
}

template <typename LEXX, typename ord_mask_t>
unsigned PeFile::processImports0(ord_mask_t ord_mask) // pass 1
{
    if (isefi) {
        if (IDSIZE(PEDIR_IMPORT))
            throwCantPack("imports not supported on EFI");

        return 0;
    }

    unsigned dllnum = 0;
    unsigned const take = IDSIZE(PEDIR_IMPORT);
    unsigned const skip = IDADDR(PEDIR_IMPORT);
    import_desc *im = (import_desc*)ibuf.subref("bad import %#x", skip, take);
    import_desc * const im_save = im;
    if (IDADDR(PEDIR_IMPORT))
    {
        for (;; ++dllnum, ++im) {
            unsigned const skip2 = ptr_diff_bytes(im, ibuf);
            (void)ibuf.subref("bad import %#x", skip2, sizeof(*im));
            if (!im->dllname)
                break;
        }
        im = im_save;
    }

    struct udll
    {
        const upx_byte *name;
        const upx_byte *shname;
        unsigned   ordinal;
        unsigned   iat;
        LEXX       *lookupt;
        unsigned   original_position;
        bool       isk32;

        static int __acc_cdecl_qsort compare(const void *p1, const void *p2)
        {
            const udll *u1 = * (const udll * const *) p1;
            const udll *u2 = * (const udll * const *) p2;
            if (u1->isk32) return -1;
            if (u2->isk32) return 1;
            if (!*u1->lookupt) return 1;
            if (!*u2->lookupt) return -1;
            int rc = strcasecmp(u1->name,u2->name);
            if (rc) return rc;
            if (u1->ordinal) return -1;
            if (u2->ordinal) return 1;
            if (!u1->shname) return 1;
            if (!u2->shname) return -1;
            rc = (int) (upx_safe_strlen(u1->shname) - upx_safe_strlen(u2->shname));
            if (rc) return rc;
            return strcmp(u1->shname, u2->shname);
        }
    };

    // +1 for dllnum=0
    Array(struct udll, dlls, dllnum+1);
    Array(struct udll *, idlls, dllnum+1);

    soimport = 1024; // safety

    unsigned ic;
    for (ic = 0; dllnum && im->dllname; ic++, im++)
    {
        idlls[ic] = dlls + ic;
        dlls[ic].name = ibuf.subref("bad dllname %#x", im->dllname, 1);
        dlls[ic].shname = nullptr;
        dlls[ic].ordinal = 0;
        dlls[ic].iat = im->iat;
        unsigned const skip2 = (im->oft ? im->oft : im->iat);
        dlls[ic].lookupt = (LEXX*)ibuf.subref("bad dll lookupt %#x", skip2, sizeof(LEXX));
        dlls[ic].original_position = ic;
        dlls[ic].isk32 = strcasecmp(kernelDll(), dlls[ic].name) == 0;

        soimport += strlen(dlls[ic].name) + 1 + 4;

        for (IPTR_I(LEXX, tarr, dlls[ic].lookupt); *tarr; tarr += 1)
        {
            if (*tarr & ord_mask)
            {
                importbyordinal = true;
                soimport += 2; // ordinal num: 2 bytes
                dlls[ic].ordinal = *tarr & 0xffff;
            }
            else //it's an import by name
            {
                IPTR_I(const upx_byte, n, ibuf + *tarr + 2);
                unsigned len = strlen(n);
                soimport += len + 1;
                if (dlls[ic].shname == nullptr || len < strlen (dlls[ic].shname))
                    dlls[ic].shname = ibuf + *tarr + 2;
            }
            soimport++; // separator
        }
    }
    mb_oimport.alloc(soimport);
    mb_oimport.clear();
    oimport = mb_oimport;

    qsort(idlls,dllnum,sizeof (udll*),udll::compare);

    info("Processing imports: %d DLLs", dllnum);

    ilinker = new ImportLinker(sizeof(LEXX));
    // create the new import table
    addStubImports();

    for (ic = 0; ic < dllnum; ic++)
    {
        if (idlls[ic]->isk32)
        {
            // for kernel32.dll we need to put all the imported
            // ordinals into the output import table, as on
            // some versions of windows GetProcAddress does not resolve them
            if (strcasecmp(idlls[ic]->name, "kernel32.dll"))
                continue;
            if (idlls[ic]->ordinal)
                for (LEXX *tarr = idlls[ic]->lookupt; *tarr; tarr++)
                    if (*tarr & ord_mask)
                    {
                        ilinker->add(kernelDll(), *tarr & 0xffff);
                        kernel32ordinal = true;
                    }
        }
        else if (!ilinker->hasDll(idlls[ic]->name))
        {
            if (idlls[ic]->shname && !idlls[ic]->ordinal)
                ilinker->add(idlls[ic]->name, idlls[ic]->shname);
            else
                ilinker->add(idlls[ic]->name, idlls[ic]->ordinal);
        }
    }

    soimpdlls = ilinker->build();

    Interval names(ibuf),iats(ibuf),lookups(ibuf);

    // create the preprocessed data
    SPAN_P_VAR(upx_byte, ppi, oimport);  // preprocessed imports
    for (ic = 0; ic < dllnum; ic++)
    {
        LEXX *tarr = idlls[ic]->lookupt;
        set_le32(ppi, ilinker->getAddress(idlls[ic]->name));
        set_le32(ppi+4,idlls[ic]->iat - rvamin);
        ppi += 8;
        for (; *tarr; tarr++)
            if (*tarr & ord_mask)
            {
                unsigned ord = *tarr & 0xffff;
                if (idlls[ic]->isk32 && kernel32ordinal)
                {
                    *ppi++ = 0xfe; // signed + odd parity
                    set_le32(ppi, ilinker->getAddress(idlls[ic]->name, ord));
                    ppi += 4;
                }
                else
                {
                    *ppi++ = 0xff;
                    set_le16(ppi, ord);
                    ppi += 2;
                }
            }
            else
            {
                *ppi++ = 1;
                unsigned const skip2 = 2+ *tarr;
                unsigned const take2 = 1+ strlen(ibuf.subref("bad import name %#x", skip2, 1));
                memcpy(ppi,ibuf.subref("bad import name %#x", skip2, take2), take2);
                ppi += take2;
                names.add(*tarr, 2+ take2);
            }
        ppi++;

        unsigned esize = ptr_diff_bytes(tarr, idlls[ic]->lookupt);
        lookups.add(idlls[ic]->lookupt,esize);
        if (ptr_diff_bytes(ibuf.subref("bad import name %#x", idlls[ic]->iat, 1), (char *)idlls[ic]->lookupt) != 0)
        {
            memcpy(ibuf.subref("bad import name %#x", idlls[ic]->iat, esize), idlls[ic]->lookupt, esize);
            iats.add(idlls[ic]->iat,esize);
        }
        names.add(idlls[ic]->name,strlen(idlls[ic]->name) + 1 + 1);
    }
    ppi += 4;
    assert(ppi < oimport+soimport);
    soimport = ptr_diff_bytes(ppi,oimport);

    if (soimport == 4)
        soimport = 0;

    //OutputFile::dump("x0.imp", oimport, soimport);

    unsigned ilen = 0;
    names.flatten();
    if (names.ivnum > 1)
    {
        // The area occupied by the dll and imported names is not continuous
        // so to still support uncompression, I can't zero the iat area.
        // This decreases compression ratio, so FIXME somehow.
        infoWarning("can't remove unneeded imports");
        ilen += sizeof(import_desc) * dllnum;
#if TESTING
        if (opt->verbose > 3)
            names.dump();
#endif
        // do some work for the unpacker
        im = im_save;
        for (ic = 0; ic < dllnum; ic++, im++)
        {
            memset(im,FILLVAL,sizeof(*im));
            im->dllname = ptr_diff_bytes(dlls[idlls[ic]->original_position].name,ibuf);
        }
    }
    else
    {
        iats.add(im_save,sizeof(import_desc) * dllnum);
        // zero unneeded data
        iats.clear();
        lookups.clear();
    }
    names.clear();

    iats.add(&names);
    iats.add(&lookups);
    iats.flatten();
    for (ic = 0; ic < iats.ivnum; ic++)
        ilen += iats.ivarr[ic].len;

    info("Imports: original size: %u bytes, preprocessed size: %u bytes",ilen,soimport);
    return names.ivnum == 1 ? names.ivarr[0].start : 0;
}

/*************************************************************************
// export handling
**************************************************************************/

PeFile::Export::Export(char *_base) : base(_base), iv(_base)
{
    COMPILE_TIME_ASSERT(sizeof(export_dir_t) == 40)
    COMPILE_TIME_ASSERT_ALIGNED1(export_dir_t)
    ename = functionptrs = ordinals = nullptr;
    names = nullptr;
    memset(&edir,0,sizeof(edir));
    size = 0;
}

PeFile::Export::~Export()
{
    free(ename);
    delete [] functionptrs;
    delete [] ordinals;
    if (names) {
        unsigned const limit = edir.names + edir.functions;
        for (unsigned ic = 0; ic < limit; ic++)
            if (names[ic]) free(names[ic]);  // allocated by strdup()
        delete [] names;
    }
}

void PeFile::Export::convert(unsigned eoffs,unsigned esize)
{
    memcpy(&edir,base + eoffs,sizeof(export_dir_t));
    size = sizeof(export_dir_t);
    iv.add(eoffs,size);

    if (!edir.name || eoffs + esize <= (unsigned)edir.name) {
        char msg[50]; snprintf(msg, sizeof(msg),
                "bad export directory name RVA %#x", (unsigned)edir.name);
        throwInternalError(msg);
    }
    unsigned len = strlen(base + edir.name) + 1;
    ename = strdup(base + edir.name);
    size += len;
    iv.add(edir.name,len);

    len = 4 * edir.functions;
    functionptrs = New(char, len + 1);
    memcpy(functionptrs,base + edir.addrtable,len);
    size += len;
    iv.add(edir.addrtable,len);

    unsigned ic;
    names = New(char *, edir.names + edir.functions + 1);
    for (ic = 0; ic < edir.names; ic++)
    {
        char *n = base + get_le32(base + edir.nameptrtable + ic * 4);
        len = strlen(n) + 1;
        names[ic] = strdup(n);
        size += len;
        iv.add(get_le32(base + edir.nameptrtable + ic * 4),len);
    }
    iv.add(edir.nameptrtable,4 * edir.names);
    size += 4 * edir.names;

    LE32 *fp = (LE32*) functionptrs;
    // export forwarders
    for (ic = 0; ic < edir.functions; ic++)
        if (fp[ic] >= eoffs && fp[ic] < eoffs + esize)
        {
            char *forw = base + fp[ic];
            len = strlen(forw) + 1;
            iv.add(forw,len);
            size += len;
            names[ic + edir.names] = strdup(forw);
        }
        else
            names[ic + edir.names] = nullptr;

    len = 2 * edir.names;
    ordinals = New(char, len + 1);
    memcpy(ordinals,base + edir.ordinaltable,len);
    size += len;
    iv.add(edir.ordinaltable,len);
    iv.flatten();
    if (iv.ivnum == 1)
        iv.clear();
#if TESTING
    else
        iv.dump();
#endif
}

void PeFile::Export::build(char *newbase, unsigned newoffs)
{
    char * const functionp = newbase + sizeof(edir);
    char * const namep = functionp + 4 * edir.functions;
    char * const ordinalp = namep + 4 * edir.names;
    char * const enamep = ordinalp + 2 * edir.names;
    char * exports = enamep + strlen(ename) + 1;

    edir.addrtable = newoffs + ptr_diff_bytes(functionp, newbase);
    edir.ordinaltable = newoffs + ptr_diff_bytes(ordinalp, newbase);
    memcpy(ordinalp,ordinals,2 * edir.names);

    edir.name = newoffs + ptr_diff_bytes(enamep, newbase);
    strcpy(enamep,ename);
    edir.nameptrtable = newoffs + ptr_diff_bytes(namep, newbase);
    unsigned ic;
    for (ic = 0; ic < edir.names; ic++)
    {
        strcpy(exports,names[ic]);
        set_le32(namep + 4 * ic,newoffs + ptr_diff_bytes(exports, newbase));
        exports += strlen(exports) + 1;
    }

    memcpy(functionp,functionptrs,4 * edir.functions);
    for (ic = 0; ic < edir.functions; ic++)
        if (names[edir.names + ic])
        {
            strcpy(exports,names[edir.names + ic]);
            set_le32(functionp + 4 * ic,newoffs + ptr_diff_bytes(exports, newbase));
            exports += strlen(exports) + 1;
        }

    memcpy(newbase,&edir,sizeof(edir));
    assert(exports - newbase == (int) size);
}

void PeFile::processExports(Export *xport) // pass1
{
    soexport = ALIGN_UP(IDSIZE(PEDIR_EXPORT),4u);
    if (soexport == 0)
        return;
    if (!isdll && opt->win32_pe.compress_exports)
    {
        infoWarning("exports compressed, --compress-exports=0 might be needed");
        soexport = 0;
        return;
    }
    xport->convert(IDADDR(PEDIR_EXPORT),IDSIZE(PEDIR_EXPORT));
    soexport = ALIGN_UP(xport->getsize(), 4u);
    mb_oexport.alloc(soexport);
    mb_oexport.clear();
    oexport = mb_oexport;
}

void PeFile::processExports(Export *xport,unsigned newoffs) // pass2
{
    if (soexport)
        xport->build((char *) raw_bytes(oexport, 0), newoffs);
}


/*************************************************************************
// TLS handling
**************************************************************************/

// thanks for theowl for providing me some docs, so that now I understand
// what I'm doing here :)

// 1999-10-17: this was tricky to find:
// when the fixup records and the tls area are on the same page, then
// the tls area is not relocated, because the relocation is done by
// the virtual memory manager only for pages which are not yet loaded.
// of course it was impossible to debug this ;-)

template <>
struct PeFile::tls_traits<LE32>
{
    struct alignas(1) tls {
        LE32 datastart; // VA tls init data start
        LE32 dataend;   // VA tls init data end
        LE32 tlsindex;  // VA tls index
        LE32 callbacks; // VA tls callbacks
        char _[8];      // zero init, characteristics
    };

    static const unsigned sotls = 24;
    static const unsigned cb_size = 4;
    typedef unsigned cb_value_t;
    static const unsigned reloc_type = 3;
    static const int tls_handler_offset_reloc = 4;
};

template <>
struct PeFile::tls_traits<LE64>
{
    struct alignas(1) tls {
        LE64 datastart; // VA tls init data start
        LE64 dataend;   // VA tls init data end
        LE64 tlsindex;  // VA tls index
        LE64 callbacks; // VA tls callbacks
        char _[8];      // zero init, characteristics
    };

    static const unsigned sotls = 40;
    static const unsigned cb_size = 8;
    typedef upx_uint64_t cb_value_t;
    static const unsigned reloc_type = 10;
    static const int tls_handler_offset_reloc = -1;  // no need to relocate
};

template <typename LEXX>
void PeFile::processTls1(Interval *iv,
                         typename tls_traits<LEXX>::cb_value_t imagebase,
                         unsigned imagesize) // pass 1
{
    typedef typename tls_traits<LEXX>::tls tls;
    typedef typename tls_traits<LEXX>::cb_value_t cb_value_t;
    const unsigned cb_size = tls_traits<LEXX>::cb_size;

    COMPILE_TIME_ASSERT(sizeof(tls) == tls_traits<LEXX>::sotls)
    COMPILE_TIME_ASSERT_ALIGNED1(tls)

    if (isefi && IDSIZE(PEDIR_TLS))
        throwCantPack("TLS not supported on EFI");

    unsigned const take = ALIGN_UP(IDSIZE(PEDIR_TLS),4u);
    sotls = take;
    if (!sotls)
        return;

    unsigned const skip = IDADDR(PEDIR_TLS);
    const tls * const tlsp = (const tls*)ibuf.subref("bad tls %#x", skip, sizeof(tls));

    // note: TLS callbacks are not implemented in Windows 95/98/ME
    if (tlsp->callbacks)
    {
        if (tlsp->callbacks < imagebase)
            throwCantPack("invalid TLS callback");
        else if (tlsp->callbacks - imagebase + 4 >= imagesize)
            throwCantPack("invalid TLS callback");
        cb_value_t v = *(LEXX*)ibuf.subref("bad TLS %#x", (tlsp->callbacks - imagebase), sizeof(LEXX));

        if(v != 0)
        {
            //count number of callbacks, just for information string - Stefan Widmann
            unsigned num_callbacks = 0;
            unsigned callback_offset = 0;
            while (*(LEXX*)ibuf.subref("bad TLS %#x",
                tlsp->callbacks - imagebase + callback_offset, sizeof(LEXX)))
            {
                //increment number of callbacks
                num_callbacks++;
                callback_offset += cb_size;
            }
            info("TLS: %u callback(s) found, adding TLS callback handler", num_callbacks);
            //set flag to include necessary sections in loader
            use_tls_callbacks = true;
            //define linker symbols
            tlscb_ptr = tlsp->callbacks;
        }
    }

    const unsigned tlsdatastart = tlsp->datastart - imagebase;
    const unsigned tlsdataend = tlsp->dataend - imagebase;

    // now some ugly stuff: find the relocation entries in the tls data area
    unsigned const take2 = IDSIZE(PEDIR_RELOC);
    unsigned const skip2 = IDADDR(PEDIR_RELOC);
    Reloc rel(ibuf.subref("bad tls reloc %#x", skip2, take2), take2);
    unsigned pos,type;
    while (rel.next(pos,type))
        if (pos >= tlsdatastart && pos < tlsdataend)
            iv->add(pos,type);

    sotls = sizeof(tls) + tlsdataend - tlsdatastart;
    // if TLS callbacks are used, we need two more {D|Q}WORDS at the end of the TLS
    // ... and those dwords should be correctly aligned
    if (use_tls_callbacks)
        sotls = ALIGN_UP(sotls, cb_size) + 2 * cb_size;
    const unsigned aligned_sotls = ALIGN_UP(sotls, (unsigned)sizeof(LEXX));

    // the PE loader wants this stuff uncompressed
    mb_otls.alloc(aligned_sotls);
    mb_otls.clear();
    otls = mb_otls; // => otls now is a SPAN_S
    unsigned const take1 = sizeof(tls);
    unsigned const skip1 = IDADDR(PEDIR_TLS);
    memcpy(otls,ibuf.subref("bad tls %#x", skip1, take1), take1);
    // WARNING: this can acces data in BSS
    unsigned const take3 = sotls - sizeof(tls);
    memcpy(otls + sizeof(tls),ibuf.subref("bad tls %#x", tlsdatastart, take3), take3);
    tlsindex = tlsp->tlsindex - imagebase;
    //NEW: subtract two dwords if TLS callbacks are used - Stefan Widmann
    info("TLS: %u bytes tls data and %u relocations added",
         sotls - (unsigned) sizeof(tls) - (use_tls_callbacks ? 2 * cb_size : 0),iv->ivnum);

    // makes sure tls index is zero after decompression
    if (tlsindex && tlsindex < imagesize)
        set_le32(ibuf.subref("bad tlsindex %#x", tlsindex, sizeof(unsigned)), 0);
}

template <typename LEXX>
void PeFile::processTls2(Reloc *rel,const Interval *iv,unsigned newaddr,
                         typename tls_traits<LEXX>::cb_value_t imagebase) // pass 2
{
    typedef typename tls_traits<LEXX>::tls tls;
    typedef typename tls_traits<LEXX>::cb_value_t cb_value_t;
    const unsigned cb_size = tls_traits<LEXX>::cb_size;
    const unsigned reloc_type = tls_traits<LEXX>::reloc_type;
    const int tls_handler_offset_reloc = tls_traits<LEXX>::tls_handler_offset_reloc;

    if (sotls == 0)
        return;
    // add new relocation entries

    if __acc_cte(tls_handler_offset > 0 && tls_handler_offset_reloc > 0)
        rel->add(tls_handler_offset + tls_handler_offset_reloc, reloc_type);

    unsigned ic;
    //NEW: if TLS callbacks are used, relocate the VA of the callback chain, too - Stefan Widmann
    for (ic = 0; ic < (use_tls_callbacks ? 4 * cb_size : 3 * cb_size); ic += cb_size)
        rel->add(newaddr + ic, reloc_type);

    SPAN_S_VAR(tls, const tlsp, mb_otls);
    // now the relocation entries in the tls data area
    for (ic = 0; ic < iv->ivnum; ic += 4)
    {
        SPAN_S_VAR(upx_byte, pp, otls + (iv->ivarr[ic].start - (tlsp->datastart - imagebase) + sizeof(tls)));
        LEXX * const p = (LEXX *) raw_bytes(pp, sizeof(LEXX));
        cb_value_t kc = *p;
        if (kc < tlsp->dataend && kc >= tlsp->datastart)
        {
            kc +=  newaddr + sizeof(tls) - tlsp->datastart;
            *p = kc + imagebase;
            rel->add(kc,iv->ivarr[ic].len);
        }
        else
            rel->add(kc - imagebase,iv->ivarr[ic].len);
    }

    const unsigned tls_data_size = tlsp->dataend - tlsp->datastart;
    tlsp->datastart = newaddr + sizeof(tls) + imagebase;
    tlsp->dataend = tlsp->datastart + tls_data_size;

    // NEW: if we have TLS callbacks to handle, we create a pointer to the new callback chain - Stefan Widmann
    tlsp->callbacks = (use_tls_callbacks ? newaddr + sotls + imagebase - 2 * cb_size : 0);

    if (use_tls_callbacks)
    {
        // set handler offset
        SPAN_S_VAR(upx_byte, pp, otls);
        pp = otls + (sotls - 2 * cb_size);
        * (LEXX *) raw_bytes(pp, sizeof(LEXX)) = tls_handler_offset + imagebase;
        pp = otls + (sotls - 1 * cb_size);
        * (LEXX *) raw_bytes(pp, sizeof(LEXX)) = 0;  // end of one-item list
        // add relocation for TLS handler offset
        rel->add(newaddr + sotls - 2 * cb_size, reloc_type);
    }
}

/*************************************************************************
// Load Configuration handling
**************************************************************************/

void PeFile::processLoadConf(Interval *iv) // pass 1
{
    if (IDSIZE(PEDIR_LOADCONF) == 0)
        return;

    const unsigned lcaddr = IDADDR(PEDIR_LOADCONF);
    const upx_byte * const loadconf = ibuf.subref("bad loadconf %#x", lcaddr, 4);
    soloadconf = get_le32(loadconf);
    if (soloadconf == 0)
        return;
    static unsigned const MAX_SOLOADCONF = 256;  // XXX FIXME: Why?
    if (soloadconf > MAX_SOLOADCONF)
        info("Load Configuration directory %u > %u", soloadconf, MAX_SOLOADCONF);

    // if there were relocation entries referring to the load config table
    // then we need them for the copy of the table too
    unsigned const take = IDSIZE(PEDIR_RELOC);
    unsigned const skip = IDADDR(PEDIR_RELOC);
    Reloc rel(ibuf.subref("bad reloc %#x", skip, take), take);
    unsigned pos,type;
    while (rel.next(pos, type))
        if (pos >= lcaddr && pos < lcaddr + soloadconf)
        {
            iv->add(pos - lcaddr, type);
            // printf("loadconf reloc detected: %x\n", pos);
        }

    mb_oloadconf.alloc(soloadconf);
    oloadconf = (upx_byte *)mb_oloadconf.getVoidPtr();
    memcpy(oloadconf, loadconf, soloadconf);
}

void PeFile::processLoadConf(Reloc *rel, const Interval *iv,
                                unsigned newaddr) // pass2
{
    // now we have the address of the new load config table
    // so we can create the new relocation entries
    for (unsigned ic = 0; ic < iv->ivnum; ic++)
    {
        rel->add(iv->ivarr[ic].start + newaddr, iv->ivarr[ic].len);
        //printf("loadconf reloc added: %x %d\n",
        //       iv->ivarr[ic].start + newaddr, iv->ivarr[ic].len);
    }
}

/*************************************************************************
// resource handling
**************************************************************************/

struct alignas(1) PeFile::Resource::res_dir_entry {
    LE32  tnl; // Type | Name | Language id - depending on level
    LE32  child;
};

struct alignas(1) PeFile::Resource::res_dir {
    char  _[12]; // flags, timedate, version
    LE16  namedentr;
    LE16  identr;

    unsigned Sizeof() const { return 16 + sizeof(res_dir_entry)*(namedentr + identr); }
    res_dir_entry entries[1];
    // it's usually safe to assume that every res_dir contains
    // at least one res_dir_entry - check() complains otherwise
};

struct alignas(1) PeFile::Resource::res_data {
    LE32  offset;
    LE32  size;
    char  _[8]; // codepage, reserved
};

struct PeFile::Resource::upx_rnode
{
    unsigned        id;
    upx_byte        *name;
    upx_rnode       *parent;
};

struct PeFile::Resource::upx_rbranch : public PeFile::Resource::upx_rnode
{
    unsigned        nc;
    upx_rnode       **children;
    res_dir         data;
};

struct PeFile::Resource::upx_rleaf : public PeFile::Resource::upx_rnode
{
    upx_rleaf       *next;
    unsigned        newoffset;
    res_data        data;
};

PeFile::Resource::Resource(const upx_byte *ibufstart_,
                           const upx_byte *ibufend_) : root(nullptr)
{
    ibufstart = ibufstart_;
    ibufend = ibufend_;
}

PeFile::Resource::Resource(const upx_byte *p,
                           const upx_byte *ibufstart_,
                           const upx_byte *ibufend_)
{
    ibufstart = ibufstart_;
    ibufend = ibufend_;
    init(p);
}

PeFile::Resource::~Resource()
{
    if (root) { destroy(root, 0); root = nullptr; }
}

unsigned PeFile::Resource::dirsize() const
{
    return ALIGN_UP(dsize + ssize, 4u);
}

bool PeFile::Resource::next()
{
    // wow, builtin autorewind... :-)
    return (current = current ? current->next : head) != nullptr;
}

unsigned PeFile::Resource::itype() const
{
    return current->parent->parent->id;
}

const upx_byte *PeFile::Resource::ntype() const
{
    return current->parent->parent->name;
}

unsigned PeFile::Resource::size() const
{
    return ALIGN_UP(current->data.size, 4u);
}

unsigned PeFile::Resource::offs() const
{
    return current->data.offset;
}

unsigned &PeFile::Resource::newoffs()
{
    return current->newoffset;
}

void PeFile::Resource::dump() const
{
    dump(root,0);
}

unsigned PeFile::Resource::iname() const
{
    return current->parent->id;
}

const upx_byte *PeFile::Resource::nname() const
{
    return current->parent->name;
}

/*
    unsigned ilang() const {return current->id;}
    const upx_byte *nlang() const {return current->name;}
*/

void PeFile::Resource::init(const upx_byte *res)
{
    COMPILE_TIME_ASSERT(sizeof(res_dir_entry) == 8)
    COMPILE_TIME_ASSERT(sizeof(res_dir) == 16 + 8)
    COMPILE_TIME_ASSERT(sizeof(res_data) == 16)
    COMPILE_TIME_ASSERT_ALIGNED1(res_dir_entry)
    COMPILE_TIME_ASSERT_ALIGNED1(res_dir)
    COMPILE_TIME_ASSERT_ALIGNED1(res_data)

    start = res;
    root = head = current = nullptr;
    dsize = ssize = 0;
    check((const res_dir*) start,0);
    root = convert(start,nullptr,0);
}

void PeFile::Resource::check(const res_dir *node,unsigned level)
{
    ibufcheck(node, sizeof(*node));
    int ic = node->identr + node->namedentr;
    if (ic == 0)
        return;
    for (const res_dir_entry *rde = node->entries; --ic >= 0; rde++)
    {
        ibufcheck(rde, sizeof(*rde));
        if (((rde->child & 0x80000000) == 0) ^ (level == 2))
            throwCantPack("unsupported resource structure");
        else if (level != 2)
            check((const res_dir*) (start + (rde->child & 0x7fffffff)),level + 1);
    }
}

void PeFile::Resource::ibufcheck(const void *m, unsigned siz)
{
    if (m < ibufstart || m > ibufend - siz)
        throwCantUnpack("corrupted resources");
}

PeFile::Resource::upx_rnode *PeFile::Resource::convert(const void *rnode,
                                                       upx_rnode *parent,
                                                       unsigned level)
{
    if (level == 3)
    {
        const res_data *node = ACC_STATIC_CAST(const res_data *, rnode);
        ibufcheck(node, sizeof(*node));
        upx_rleaf *leaf = new upx_rleaf;
        leaf->id = 0;
        leaf->name = nullptr;
        leaf->parent = parent;
        leaf->next = head;
        leaf->newoffset = 0;
        leaf->data = *node;

        head = leaf; // append node to a linked list for traversal
        dsize += sizeof(res_data);
        return leaf;
    }

    const res_dir *node = ACC_STATIC_CAST(const res_dir *, rnode);
    ibufcheck(node, sizeof(*node));
    int ic = node->identr + node->namedentr;
    if (ic == 0)
        return nullptr;

    upx_rbranch *branch = new upx_rbranch;
    branch->id = 0;
    branch->name = nullptr;
    branch->parent = parent;
    branch->nc = ic;
    branch->children = New(upx_rnode *, ic);
    branch->data = *node;

    for (const res_dir_entry *rde = node->entries + ic - 1; --ic >= 0; rde--)
    {
        upx_rnode *child = convert(start + (rde->child & 0x7fffffff),branch,level + 1);
        xcheck(child);
        branch->children[ic] = child;
        child->id = rde->tnl;
        if (child->id & 0x80000000)
        {
            const upx_byte *p = start + (child->id & 0x7fffffff);
            ibufcheck(p, 2);
            const unsigned len = 2 + 2 * get_le16(p);
            ibufcheck(p, len);
            child->name = New(upx_byte, len);
            memcpy(child->name,p,len); // copy unicode string
            ssize += len; // size of unicode strings
        }
    }
    dsize += node->Sizeof();
    return branch;
}

void PeFile::Resource::build(const upx_rnode *node, unsigned &bpos,
                             unsigned &spos, unsigned level)
{
    if (level == 3)
    {
        if (bpos + sizeof(res_data) > dirsize())
            throwCantUnpack("corrupted resources");

        res_data *l = (res_data*) (newstart + bpos);
        const upx_rleaf *leaf = (const upx_rleaf*) node;
        *l = leaf->data;
        if (leaf->newoffset)
            l->offset = leaf->newoffset;
        bpos += sizeof(*l);
        return;
    }
    if (bpos + sizeof(res_dir) > dirsize())
        throwCantUnpack("corrupted resources");

    res_dir * const b = (res_dir*) (newstart + bpos);
    const upx_rbranch *branch = (const upx_rbranch*) node;
    *b = branch->data;
    bpos += b->Sizeof();
    res_dir_entry *be = b->entries;
    for (unsigned ic = 0; ic < branch->nc; ic++, be++)
    {
        xcheck(branch->children[ic]);
        be->tnl = branch->children[ic]->id;
        be->child = bpos + ((level < 2) ? 0x80000000 : 0);

        const upx_byte *p;
        if ((p = branch->children[ic]->name) != nullptr)
        {
            be->tnl = spos + 0x80000000;
            if (spos + get_le16(p) * 2 + 2 > dirsize())
                throwCantUnpack("corrupted resources");
            memcpy(newstart + spos,p,get_le16(p) * 2 + 2);
            spos += get_le16(p) * 2 + 2;
        }

        build(branch->children[ic],bpos,spos,level + 1);
    }
}

upx_byte *PeFile::Resource::build()
{
    mb_start.dealloc();
    newstart = nullptr;
    if (dirsize()) {
      mb_start.alloc(dirsize());
      newstart = static_cast<upx_byte *>(mb_start.getVoidPtr());
      unsigned bpos = 0,spos = dsize;
      build(root,bpos,spos,0);

      // dirsize() is 4 bytes aligned, so we may need to zero
      // up to 2 bytes to make valgrind happy
      while (spos < dirsize())
          newstart[spos++] = 0;
    }

    return newstart;
}

void PeFile::Resource::destroy(upx_rnode *node,unsigned level)
{
    xcheck(node);
    if (level == 3)
    {
        upx_rleaf *leaf = ACC_STATIC_CAST(upx_rleaf *, node);
        delete [] leaf->name; leaf->name = nullptr;
        delete leaf;
    }
    else
    {
        upx_rbranch *branch = ACC_STATIC_CAST(upx_rbranch *, node);
        delete [] branch->name; branch->name = nullptr;
        for (int ic = branch->nc; --ic >= 0; )
            destroy(branch->children[ic], level + 1);
        delete [] branch->children; branch->children = nullptr;
        delete branch;
    }
}

static void lame_print_unicode(const upx_byte *p)
{
    for (unsigned ic = 0; ic < get_le16(p); ic++)
        printf("%c",(char)p[ic * 2 + 2]);
}

void PeFile::Resource::dump(const upx_rnode *node,unsigned level) const
{
    if (level)
    {
        for (unsigned ic = 1; ic < level; ic++)
            printf("\t\t");
        if (node->name)
            lame_print_unicode(node->name);
        else
            printf("0x%x",node->id);
        printf("\n");
    }
    if (level == 3)
        return;
    const upx_rbranch * const branch = (const upx_rbranch *) node;
    for (unsigned ic = 0; ic < branch->nc; ic++)
        dump(branch->children[ic],level + 1);
}

void PeFile::Resource::clear(upx_byte *node,unsigned level,Interval *iv)
{
    if (level == 3)
        iv->add(node,sizeof (res_data));
    else
    {
        const res_dir * const rd = (res_dir*) node;
        const unsigned n = rd->identr + rd->namedentr;
        const res_dir_entry *rde = rd->entries;
        for (unsigned ic = 0; ic < n; ic++, rde++)
            clear(newstart + (rde->child & 0x7fffffff),level + 1,iv);
        iv->add(rd,rd->Sizeof());
    }
}

bool PeFile::Resource::clear()
{
    newstart = const_cast<upx_byte*> (start);
    Interval iv(newstart);
    clear(newstart,0,&iv);
    iv.flatten();
    if (iv.ivnum == 1)
        iv.clear();
#if TESTING
    if (opt->verbose > 3)
        iv.dump();
#endif
    return iv.ivnum == 1;
}

void PeFile::processResources(Resource *res,unsigned newaddr)
{
    if (IDSIZE(PEDIR_RESOURCE) == 0)
        return;
    while (res->next())
        if (res->newoffs())
            res->newoffs() += newaddr;
    if (res->dirsize()) {
      upx_byte *p = res->build();
      memcpy(oresources,p,res->dirsize());
    }
}

static bool match(unsigned itype, const unsigned char *ntype,
                  unsigned iname, const unsigned char *nname,
                  const char *keep)
{
    // format of string keep: type1[/name1],type2[/name2], ....
    // typex and namex can be string or number
    // hopefully resource names do not have '/' or ',' characters inside

    struct helper
    {
        static bool match(unsigned num, const unsigned char *unistr,
                          const char *mkeep)
        {
            if (!unistr)
                return (unsigned) atoi(mkeep) == num;

            unsigned ic;
            for (ic = 0; ic < get_le16(unistr); ic++)
                if (unistr[2 + ic * 2] != (unsigned char) mkeep[ic])
                    return false;
            return mkeep[ic] == 0 || mkeep[ic] == ',' || mkeep[ic] == '/';
        }
    };

    // FIXME this comparison is not too exact
    for (;;)
    {
        char const *delim1 = strchr(keep, '/');
        char const *delim2 = strchr(keep, ',');
        if (helper::match(itype, ntype, keep))
        {
            if (!delim1)
                return true;
            if (delim2 && delim2 < delim1)
                return true;
            if (helper::match(iname, nname, delim1 + 1))
                return true;
        }

        if (delim2 == nullptr)
            break;
        keep = delim2 + 1;
    }
    return false;
}

void PeFile::processResources(Resource *res)
{
    const unsigned vaddr = IDADDR(PEDIR_RESOURCE);
    if ((soresources = IDSIZE(PEDIR_RESOURCE)) == 0)
        return;

    // setup default options for resource compression
    if (opt->win32_pe.compress_resources < 0)
        opt->win32_pe.compress_resources = !isefi;
    if (!opt->win32_pe.compress_resources)
    {
        opt->win32_pe.compress_icons = false;
        for (int i = 0; i < RT_LAST; i++)
            opt->win32_pe.compress_rt[i] = false;
    }
    if (opt->win32_pe.compress_rt[RT_STRING] < 0)
    {
        // by default, don't compress RT_STRINGs of screensavers (".scr")
        opt->win32_pe.compress_rt[RT_STRING] = true;
        if (fn_has_ext(fi->getName(),"scr"))
            opt->win32_pe.compress_rt[RT_STRING] = false;
    }

    res->init(ibuf.subref("bad res %#x", vaddr, 1));

    for (soresources = res->dirsize(); res->next(); soresources += 4 + res->size())
        ;
    mb_oresources.alloc(soresources);
    mb_oresources.clear();
    oresources = mb_oresources; // => SPAN_S
    SPAN_S_VAR(upx_byte, ores, oresources + res->dirsize());

    char *keep_icons = nullptr; // icon ids in the first icon group
    unsigned iconsin1stdir = 0;
    if (opt->win32_pe.compress_icons == 2)
        while (res->next()) // there is no rewind() in Resource
            if (res->itype() == RT_GROUP_ICON && iconsin1stdir == 0)
            {
                iconsin1stdir = get_le16(ibuf.subref("bad resoff %#x", res->offs() + 4, 2));
                keep_icons = New(char, 1 + iconsin1stdir * 9);
                *keep_icons = 0;
                for (unsigned ic = 0; ic < iconsin1stdir; ic++)
                    upx_safe_snprintf(keep_icons + strlen(keep_icons), 9, "3/%u,",
                                 get_le16(ibuf.subref("bad resoff %#x", res->offs() + 6 + ic * 14 + 12, 2)));
                if (*keep_icons)
                    keep_icons[strlen(keep_icons) - 1] = 0;
            }

    // the icon id which should not be compressed when compress_icons == 1
    unsigned first_icon_id = (unsigned) -1;
    if (opt->win32_pe.compress_icons == 1)
        while (res->next())
            if (res->itype() == RT_GROUP_ICON && first_icon_id == (unsigned) -1)
                first_icon_id = get_le16(ibuf.subref("bad resoff %#x", res->offs() + 6 + 12, 2));

    bool compress_icon = opt->win32_pe.compress_icons > 1;
    bool compress_idir = opt->win32_pe.compress_icons == 3;

    // some statistics
    unsigned usize = 0;
    unsigned csize = 0;
    unsigned unum = 0;
    unsigned cnum = 0;

    while (res->next())
    {
        const unsigned rtype = res->itype();
        bool do_compress = true;
        if (!opt->win32_pe.compress_resources)
            do_compress = false;
        else if (rtype == RT_ICON)          // icon
        {
            if (opt->win32_pe.compress_icons == 0)
                do_compress = false;
            else if (opt->win32_pe.compress_icons == 1)
                if ((first_icon_id == (unsigned) -1
                     || first_icon_id == res->iname()))
                    do_compress = compress_icon;
        }
        else if (rtype == RT_GROUP_ICON)    // icon directory
            do_compress = compress_idir && opt->win32_pe.compress_icons;
        else if (rtype > 0 && rtype < RT_LAST)
            do_compress = opt->win32_pe.compress_rt[rtype] ? true : false;

        if (keep_icons)
            do_compress &= !match(res->itype(), res->ntype(), res->iname(),
                                  res->nname(), keep_icons);
        do_compress &= !match(res->itype(), res->ntype(), res->iname(),
                              res->nname(), "TYPELIB,REGISTRY,16");
        do_compress &= !match(res->itype(), res->ntype(), res->iname(),
                              res->nname(), opt->win32_pe.keep_resource);

        if (do_compress)
        {
            csize += res->size();
            cnum++;
            continue;
        }

        usize += res->size();
        unum++;

        set_le32(ores,res->offs()); // save original offset
        ores += 4;
        unsigned const take = res->size();
        ICHECK(ibuf + res->offs(), take);
        memcpy(ores, ibuf.subref("bad resoff %#x", res->offs(), take), take);
        ibuf.fill(res->offs(), take, FILLVAL);
        res->newoffs() = ptr_diff_bytes(ores,oresources);
        if (rtype == RT_ICON && opt->win32_pe.compress_icons == 1)
            compress_icon = true;
        else if (rtype == RT_GROUP_ICON)
        {
            if (opt->win32_pe.compress_icons == 1)
            {
                icondir_offset = 4 + ptr_diff_bytes(ores,oresources);
                icondir_count = get_le16(oresources + icondir_offset);
                set_le16(oresources + icondir_offset,1);
            }
            compress_idir = true;
        }
        ores += res->size();
    }
    soresources = ptr_diff_bytes(ores,oresources);

    delete[] keep_icons;
    if (!res->clear())
    {
        // The area occupied by the resource directory is not continuous
        // so to still support uncompression, I can't zero this area.
        // This decreases compression ratio, so FIXME somehow.
        infoWarning("can't remove unneeded resource directory");
    }
    info("Resources: compressed %u (%u bytes), not compressed %u (%u bytes)",cnum,csize,unum,usize);
}


unsigned PeFile::virta2objnum(unsigned addr,pe_section_t *sect,unsigned objs)
{
    unsigned ic;
    for (ic = 0; ic < objs; ic++)
    {
        if (sect->vaddr <= addr && sect->vaddr + sect->vsize > addr)
            return ic;
        sect++;
    }
    //throwCantPack("virta2objnum() failed");
    return ic;
}


unsigned PeFile::tryremove (unsigned vaddr,unsigned objs)
{
    unsigned ic = virta2objnum(vaddr,isection,objs);
    if (ic && ic == objs - 1)
    {
        //fprintf(stderr,"removed section: %d size: %lx\n",ic,(long)isection[ic].size);
        info("removed section: %d size: 0x%lx",ic,(long)isection[ic].size);
        objs--;
    }
    return objs;
}


unsigned PeFile::stripDebug(unsigned overlaystart)
{
    if (IDADDR(PEDIR_DEBUG) == 0)
        return overlaystart;

    struct alignas(1) debug_dir_t {
        char  _[16]; // flags, time/date, version, type
        LE32  size;
        char  __[4]; // rva
        LE32  fpos;
    };

    COMPILE_TIME_ASSERT(sizeof(debug_dir_t) == 28)
    COMPILE_TIME_ASSERT_ALIGNED1(debug_dir_t)
    COMPILE_TIME_ASSERT(sizeof(((debug_dir_t*)nullptr)->_)  == 16)
    COMPILE_TIME_ASSERT(sizeof(((debug_dir_t*)nullptr)->__) ==  4)

    unsigned const take = IDSIZE(PEDIR_DEBUG);
    unsigned const skip = IDADDR(PEDIR_DEBUG);
    const debug_dir_t *dd = (const debug_dir_t*)ibuf.subref("bad debug %#x", skip, take);
    for (unsigned ic = 0; ic < IDSIZE(PEDIR_DEBUG) / sizeof(debug_dir_t); ic++, dd++)
        if (overlaystart == dd->fpos)
            overlaystart += dd->size;
    ibuf.fill(IDADDR(PEDIR_DEBUG), IDSIZE(PEDIR_DEBUG), FILLVAL);
    return overlaystart;
}


/*************************************************************************
// pack
**************************************************************************/

void PeFile::readSectionHeaders(unsigned objs, unsigned sizeof_ih)
{
    if (!objs) {
        return;
    }
    mb_isection.alloc(sizeof(pe_section_t) * objs);
    isection = (pe_section_t *)mb_isection.getVoidPtr();
    if (file_size_u < pe_offset + sizeof_ih + sizeof(pe_section_t)*objs) {
        char buf[32]; snprintf(buf, sizeof(buf), "too many sections %d", objs);
        throwCantPack(buf);
    }
    fi->seek(pe_offset+sizeof_ih,SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);
    rvamin = isection[0].vaddr;
    unsigned const rvalast = isection[-1+ objs].vsize + isection[-1+ objs].vaddr;
    for (unsigned j=0; j < objs; ++j) { // expect: first is min, last is max
        unsigned lo = isection[j].vaddr, hi = isection[j].vsize + lo;
        if (hi < lo) { // this checks first and last sections, too!
            char buf[64]; snprintf(buf, sizeof(buf),
                "bad section[%d] wrap-around %#x %#x", j, lo, hi - lo);
            throwCantPack(buf);
        }
        if (lo < rvamin) {
            char buf[64]; snprintf(buf, sizeof(buf),
                "bad section .rva [%d] %#x < [0] %#x", j, lo, rvamin);
            throwCantPack(buf);
        }
        if (rvalast < hi) {
            char buf[80]; snprintf(buf, sizeof(buf),
                "bad section .rva+.vsize  [%d] %#x > [%d] %#x", j, hi,
                (-1+ objs), rvalast);
            throwCantPack(buf);
        }
    }

    infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);
}

void PeFile::checkHeaderValues(unsigned subsystem, unsigned mask,
                               unsigned ih_entry, unsigned ih_filealign)
{
    if ((1u << subsystem) & ~mask)
    {
        char buf[100];
        upx_safe_snprintf(buf, sizeof(buf), "PE: subsystem %u is not supported",
                     subsystem);
        throwCantPack(buf);
    }
    //check CLR Runtime Header directory entry
    if (IDSIZE(PEDIR_COMRT))
        throwCantPack(".NET files are not yet supported");

    if (isection == nullptr)
        throwCantPack("No section was found");

    if (memcmp(isection[0].name,"UPX",3) == 0)
        throwAlreadyPackedByUPX();

    if (!opt->force && IDSIZE(15))
        throwCantPack("file is possibly packed/protected (try --force)");

    if (ih_entry && ih_entry < rvamin)
        throwCantPack("run a virus scanner on this file!");

    const unsigned fam1 = ih_filealign - 1;
    if (!(1+ fam1) || (1+ fam1) & fam1) { // ih_filealign is not a power of 2
        char buf[32]; snprintf(buf, sizeof(buf), "bad file alignment %#x", 1+ fam1);
        throwCantPack(buf);
    }
}

unsigned PeFile::handleStripRelocs(upx_uint64_t ih_imagebase,
                                   upx_uint64_t default_imagebase,
                                   LE16 &dllflags)
{
    if (opt->win32_pe.strip_relocs < 0)
    {
        if (isdll || isefi || dllflags & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE)
            opt->win32_pe.strip_relocs = false;
        else
            opt->win32_pe.strip_relocs = ih_imagebase >= default_imagebase;
    }
    if (opt->win32_pe.strip_relocs)
    {
        if (isdll || isefi)
            throwCantPack("--strip-relocs is not allowed with DLL and EFI images");
        if (dllflags & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE)
        {
            if (opt->force) // Disable ASLR
            {
                // The bit is set, so clear it with XOR
                dllflags ^= IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
                // HIGH_ENTROPY_VA has no effect without DYNAMIC_BASE, so clear
                // it also if set
                dllflags &= ~(unsigned)IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA;
            }
            else
                throwCantPack("--strip-relocs is not allowed with ASLR (use "
                              "with --force to remove)");
        }
        if (!opt->force && ih_imagebase < default_imagebase)
            throwCantPack("--strip-relocs may not support this imagebase (try "
                          "with --force)");
        return RELOCS_STRIPPED;
    }
    else
        info("Base relocations stripping is disabled for this image");
    return 0;
}

static unsigned umax(unsigned a, unsigned b)
{
    return (a >= b) ? a : b;
}

unsigned PeFile::readSections(unsigned objs, unsigned usize,
                              unsigned ih_filealign, unsigned ih_datasize)
{
    const unsigned xtrasize = UPX_MAX(ih_datasize, 65536u) + IDSIZE(PEDIR_IMPORT)
        + IDSIZE(PEDIR_BOUNDIM) + IDSIZE(PEDIR_IAT) + IDSIZE(PEDIR_DELAYIMP)
        + IDSIZE(PEDIR_RELOC);
    ibuf.alloc(usize + xtrasize);

    // BOUND IMPORT support. FIXME: is this ok?
    fi->seek(0,SEEK_SET);
    fi->readx(ibuf,ibufgood= isection[0].rawdataptr);

    //Interval holes(ibuf);

    unsigned ic,jc,overlaystart = 0;
    ibuf.clear(0, usize);
    for (ic = jc = 0; ic < objs; ic++)
    {
        if (isection[ic].rawdataptr && overlaystart < isection[ic].rawdataptr + isection[ic].size)
            overlaystart = ALIGN_UP(isection[ic].rawdataptr + isection[ic].size,ih_filealign);
        if (isection[ic].vsize == 0)
            isection[ic].vsize = isection[ic].size;
        if ((isection[ic].flags & PEFL_BSS) || isection[ic].rawdataptr == 0
            || (isection[ic].flags & PEFL_INFO))
        {
            //holes.add(isection[ic].vaddr,isection[ic].vsize);
            continue;
        }
        if (isection[ic].vaddr + isection[ic].size > usize)
            throwCantPack("section size problem");
        if (!isrtm && ((isection[ic].flags & (PEFL_WRITE|PEFL_SHARED))
            == (PEFL_WRITE|PEFL_SHARED)))
            if (!opt->force)
                throwCantPack("writable shared sections not supported (try --force)");
        if (jc && isection[ic].rawdataptr - jc > ih_filealign && !opt->force)
            throwCantPack("superfluous data between sections (try --force)");
        fi->seek(isection[ic].rawdataptr,SEEK_SET);
        jc = isection[ic].size;
        if (jc > isection[ic].vsize)
            jc = isection[ic].vsize;
        if (isection[ic].vsize == 0) // hack for some tricky programs - may this break other progs?
            jc = isection[ic].vsize = isection[ic].size;
        if (isection[ic].vaddr + jc > ibuf.getSize())
            throwInternalError("buffer too small 1");
        fi->readx(ibuf.subref("bad section %#x", isection[ic].vaddr, jc), jc);
        ibufgood= umax(ibufgood, jc + isection[ic].vaddr);  // FIXME: simplistic
        jc += isection[ic].rawdataptr;
    }
    return overlaystart;
}

void PeFile::callCompressWithFilters(Filter &ft, int filter_strategy, unsigned ih_codebase)
{
    compressWithFilters(&ft, 2048, NULL_cconf, filter_strategy,
                        ih_codebase, rvamin, 0, nullptr, 0);
}

void PeFile::callProcessRelocs(Reloc &rel, unsigned &ic)
{
    // wince wants relocation data at the beginning of a section
    PeFile::processRelocs(&rel);
    ODADDR(PEDIR_RELOC) = soxrelocs ? ic : 0;
    ODSIZE(PEDIR_RELOC) = soxrelocs;
    ic += soxrelocs;
}

void PeFile::callProcessResources(Resource &res, unsigned &ic)
{
    if (soresources)
        processResources(&res,ic);
    ODADDR(PEDIR_RESOURCE) = soresources ? ic : 0;
    ODSIZE(PEDIR_RESOURCE) = soresources;
    ic += soresources;
}

template <typename LEXX, typename ht>
void PeFile::pack0(OutputFile *fo, ht &ih, ht &oh,
                   unsigned subsystem_mask, upx_uint64_t default_imagebase,
                   bool last_section_rsrc_only)
{
    // FIXME: we need to think about better support for --exact
    if (opt->exact)
        throwCantPackExact();

    const unsigned objs = ih.objects;
    readSectionHeaders(objs, sizeof(ih));
    if (!opt->force && handleForceOption())
        throwCantPack("unexpected value in PE header (try --force)");

    if (ih.dllflags & IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY)
    {
        if (opt->force)
            ih.dllflags &= ~(unsigned)IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY;
        else
            throwCantPack("image forces integrity check (use --force to remove)");
    }
    checkHeaderValues(ih.subsystem, subsystem_mask, ih.entry, ih.filealign);

    //remove certificate directory entry
    if (IDSIZE(PEDIR_SEC))
        IDSIZE(PEDIR_SEC) = IDADDR(PEDIR_SEC) = 0;

    if (ih.flags & RELOCS_STRIPPED)
        opt->win32_pe.strip_relocs = true;
    else
        ih.flags |= handleStripRelocs(ih.imagebase, default_imagebase, ih.dllflags);

    if (isefi) {
        // PIC for EFI only to avoid false positive detections of Win32 images
        // without relocations fixed address is smaller
        if (!opt->win32_pe.strip_relocs)
            use_stub_relocs = false;

        // EFI build tools already clear DOS stub
        // and small file alignment benefits from extra space
        unsigned char stub[0x40];
        memset(stub, 0, sizeof(stub));
        set_le16(stub, 'M' + 'Z'*256);
        set_le32(stub + sizeof(stub) - sizeof(LE32), sizeof(stub));
        fo->write(stub, sizeof(stub));
        pe_offset = sizeof(stub);
    } else
        handleStub(fi,fo,pe_offset);
    unsigned overlaystart = readSections(objs, ih.imagesize, ih.filealign, ih.datasize);
    unsigned overlay = file_size - stripDebug(overlaystart);
    if (overlay >= (unsigned) file_size)
        overlay = 0;
    checkOverlay(overlay);

    if (ih.dllflags & IMAGE_DLLCHARACTERISTICS_CONTROL_FLOW_GUARD)
    {
        if (opt->force)
        {
            const unsigned lcsize = IDSIZE(PEDIR_LOADCONF);
            const unsigned lcaddr = IDADDR(PEDIR_LOADCONF);
            const unsigned gfpos = 14 * sizeof(ih.imagebase) +
                                   6 * sizeof(LE32) + 4 * sizeof(LE16);
            if (lcaddr && lcsize >= gfpos + sizeof(LE32))
                // GuardFlags: Set IMAGE_GUARD_SECURITY_COOKIE_UNUSED
                // and clear the rest
                set_le32(ibuf.subref("bad guard flags at %#x", lcaddr + gfpos,
                                     sizeof(LE32)), 0x00000800);
            ih.dllflags ^= IMAGE_DLLCHARACTERISTICS_CONTROL_FLOW_GUARD;
        }
        else
            throwCantPack("CFGuard enabled PE files are not supported (use "
                          "--force to disable)");
    }

    Resource res(ibuf, ibuf + ibuf.getSize());
    Interval tlsiv(ibuf);
    Interval loadconfiv(ibuf);
    Export xport((char*)(unsigned char*)ibuf);

    const unsigned dllstrings = processImports();
    processTls(&tlsiv); // call before processRelocs!!
    processLoadConf(&loadconfiv);
    processResources(&res);
    processExports(&xport);
    processRelocs();

    //OutputFile::dump("x1", ibuf, usize);

    // some checks for broken linkers - disable filter if necessary
    bool allow_filter = true;
    if (/*FIXME ih.codebase == ih.database
        ||*/ ih.codebase + ih.codesize > ih.imagesize
        || (isection[virta2objnum(ih.codebase,isection,objs)].flags & PEFL_CODE) == 0)
        allow_filter = false;

    const unsigned oam1 = ih.objectalign - 1;
    if (!(1+ oam1) || (1+ oam1) & oam1) { // ih.objectalign is not a power of 2
        char buf[32]; snprintf(buf, sizeof(buf), "bad object alignment %#x", 1+ oam1);
        throwCantPack(buf);
    }

    // FIXME: if the last object has a bss then this won't work
    // newvsize = (isection[objs-1].vaddr + isection[objs-1].size + oam1) &~ oam1;
    // temporary solution:
    unsigned newvsize = (isection[objs-1].vaddr + isection[objs-1].vsize + oam1) &~ oam1;

    //fprintf(stderr,"newvsize=%x objs=%d\n",newvsize,objs);
    if (newvsize + soimport + sorelocs > ibuf.getSize())
         throwInternalError("buffer too small 2");
    memcpy(ibuf+newvsize,oimport,soimport);
    memcpy(ibuf+newvsize+soimport,orelocs,sorelocs);

    cimports = newvsize - rvamin;   // rva of preprocessed imports
    crelocs = cimports + soimport;  // rva of preprocessed fixups

    ph.u_len = newvsize + soimport + sorelocs;

    // some extra data for uncompression support
    unsigned s = 0;
    upx_byte * const p1 = ibuf.subref("bad ph.u_len %#x", ph.u_len, sizeof(ih));
    memcpy(p1 + s,&ih,sizeof (ih));
    s += sizeof (ih);
    memcpy(p1 + s,isection,ih.objects * sizeof(*isection));
    s += ih.objects * sizeof(*isection);
    if (soimport)
    {
        set_le32(p1 + s,cimports);
        set_le32(p1 + s + 4,dllstrings);
        s += 8;
    }
    if (sorelocs)
    {
        set_le32(p1 + s,crelocs);
        p1[s + 4] = (unsigned char) (big_relocs & 6);
        s += 5;
    }
    if (soresources)
    {
        set_le16(p1 + s,icondir_count);
        s += 2;
    }
    // end of extra data
    set_le32(p1 + s,ptr_diff_bytes(p1,ibuf) - rvamin);
    s += 4;
    ph.u_len += s;
    obuf.allocForCompression(ph.u_len);

    // prepare packheader
    if (ph.u_len < rvamin) { // readSectionHeaders() should have caught this
        char buf[64]; snprintf(buf, sizeof(buf),
            "bad PE header  ph.u_len=%#x  rvamin=%#x", ph.u_len, rvamin);
        throwInternalError(buf);
    }
    ph.u_len -= rvamin;
    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ih.codesize;
    ft.addvalue = ih.codebase - rvamin;
    // compress
    int filter_strategy = allow_filter ? 0 : -3;

    // disable filters for files with broken headers
    if (ih.codebase + ih.codesize > ph.u_len)
    {
        ft.buf_len = 1;
        filter_strategy = -3;
    }

    callCompressWithFilters(ft, filter_strategy, ih.codebase);
// info: see buildLoader()
    newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
        tlsindex = 0;

    const int oh_filealign = UPX_MIN(ih.filealign, 0x200);
    const unsigned fam1 = oh_filealign - 1;

    int identsize = 0;
    const unsigned codesize = getLoaderSection("IDENTSTR",&identsize);
    assert(identsize > 0);
    unsigned ic;
    getLoaderSection("UPX1HEAD",(int*)&ic);
    identsize += ic;

    const bool has_oxrelocs = !opt->win32_pe.strip_relocs && (use_stub_relocs || sotls || loadconfiv.ivnum);
    const bool has_ncsection = has_oxrelocs || soimpdlls || soexport || soresources;
    const unsigned oobjs = last_section_rsrc_only ? 4 : has_ncsection ? 3 : 2;
    ////pe_section_t osection[oobjs];
    pe_section_t osection[4];
    // section 0 : bss
    //         1 : [ident + header] + packed_data + unpacker + tls + loadconf
    //         2 : not compressed data
    //         3 : resource data -- wince 5 needs a new section for this

    // the last section should start with the resource data, because lots of lame
    // windoze codes assume that resources starts on the beginning of a section

    // note: there should be no data in the last section which needs fixup

    // identsplit - number of ident + (upx header) bytes to put into the PE header
    const unsigned sizeof_osection = sizeof(osection[0]) * oobjs;
    int identsplit = pe_offset + sizeof_osection + sizeof(ht);
    if ((identsplit & fam1) == 0)
        identsplit = 0;
    else if (((identsplit + identsize) ^ identsplit) < oh_filealign)
        identsplit = identsize;
    else
        identsplit = ALIGN_GAP(identsplit, oh_filealign);
    ic = identsize - identsplit;

    const unsigned c_len = ((ph.c_len + ic) & 15) == 0 ? ph.c_len : ph.c_len + 16 - ((ph.c_len + ic) & 15);
    obuf.clear(ph.c_len, c_len - ph.c_len);

    const unsigned aligned_sotls = ALIGN_UP(sotls, (unsigned)sizeof(LEXX));
    const unsigned s1size = ALIGN_UP(ic + c_len + codesize, (unsigned) sizeof(LEXX)) + aligned_sotls + soloadconf;
    const unsigned s1addr = (newvsize - (ic + c_len) + oam1) &~ oam1;

    const unsigned ncsection = (s1addr + s1size + oam1) &~ oam1;
    const unsigned upxsection = s1addr + ic + c_len;

    Reloc rel(1024); // new relocations are put here
    addNewRelocations(rel, upxsection);

    // new PE header
    memcpy(&oh,&ih,sizeof(oh));
    oh.filealign = oh_filealign; // identsplit depends on this
    memset(osection,0,sizeof(osection));

    oh.entry = upxsection;
    oh.objects = oobjs;
    oh.chksum = 0;

    // fill the data directory
    ODADDR(PEDIR_DEBUG) = 0;
    ODSIZE(PEDIR_DEBUG) = 0;
    ODADDR(PEDIR_IAT) = 0;
    ODSIZE(PEDIR_IAT) = 0;
    ODADDR(PEDIR_BOUNDIM) = 0;
    ODSIZE(PEDIR_BOUNDIM) = 0;

    // tls & loadconf are put into section 1
    ic = s1addr + s1size - aligned_sotls - soloadconf;

    if (use_tls_callbacks)
        tls_handler_offset = linker->getSymbolOffset("PETLSC2") + upxsection;

    processTls(&rel,&tlsiv,ic);
    ODADDR(PEDIR_TLS) = aligned_sotls ? ic : 0;
    ODSIZE(PEDIR_TLS) = aligned_sotls ? (sizeof(LEXX) == 4 ? 0x18 : 0x28) : 0;
    ic += aligned_sotls;

    processLoadConf(&rel, &loadconfiv, ic);
    ODADDR(PEDIR_LOADCONF) = soloadconf ? ic : 0;
    ODSIZE(PEDIR_LOADCONF) = soloadconf;
    ic += soloadconf;

    const bool rel_at_sections_start = last_section_rsrc_only;

    ic = ncsection;
    if (!last_section_rsrc_only)
        callProcessResources(res, ic);
    if (rel_at_sections_start)
        callProcessRelocs(rel, ic);

    processImports2(ic, getProcessImportParam(upxsection));
    ODADDR(PEDIR_IMPORT) = soimpdlls ? ic : 0;
    ODSIZE(PEDIR_IMPORT) = soimpdlls;
    ic += soimpdlls;

    processExports(&xport,ic);
    ODADDR(PEDIR_EXPORT) = soexport ? ic : 0;
    ODSIZE(PEDIR_EXPORT) = soexport;
    if (!isdll && opt->win32_pe.compress_exports)
    {
        ODADDR(PEDIR_EXPORT) = IDADDR(PEDIR_EXPORT);
        ODSIZE(PEDIR_EXPORT) = IDSIZE(PEDIR_EXPORT);
    }
    ic += soexport;

    if (!rel_at_sections_start)
        callProcessRelocs(rel, ic);

    // when the resource is put alone into section 3
    const unsigned res_start = (ic + oam1) &~ oam1;
    if (last_section_rsrc_only)
        callProcessResources(res, ic = res_start);

    defineSymbols(ncsection, upxsection, sizeof(oh),
                  identsize - identsplit, s1addr);
    defineFilterSymbols(&ft);
    relocateLoader();
    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    patchPackHeader(loader, lsize);

    const unsigned ncsize = soxrelocs + soimpdlls + soexport
        + (!last_section_rsrc_only ? soresources : 0);
    assert((soxrelocs == 0) == !has_oxrelocs);
    assert((ncsize == 0) == !has_ncsection);

    // this one is tricky: it seems windoze touches 4 bytes after
    // the end of the relocation data - so we have to increase
    // the virtual size of this section
    const unsigned ncsize_virt_increase = soxrelocs && (ncsize & oam1) == 0 ? 8 : 0;

    // fill the sections
    strcpy(osection[0].name,"UPX0");
    strcpy(osection[1].name,"UPX1");
    // after some windoze debugging I found that the name of the sections
    // DOES matter :( .rsrc is used by oleaut32.dll (TYPELIBS)
    // and because of this lame dll, the resource stuff must be the
    // first in the 3rd section - the author of this dll seems to be
    // too idiot to use the data directories... M$ suxx 4 ever!
    // ... even worse: exploder.exe in NiceTry also depends on this to
    // locate version info

    strcpy(osection[2].name, !last_section_rsrc_only && soresources ? ".rsrc" : "UPX2");

    osection[0].vaddr = rvamin;
    osection[1].vaddr = s1addr;
    osection[2].vaddr = ncsection;

    osection[0].size = 0;
    osection[1].size = (s1size + fam1) &~ fam1;
    osection[2].size = (ncsize + fam1) &~ fam1;

    osection[0].vsize = osection[1].vaddr - osection[0].vaddr;
    if (!last_section_rsrc_only)
    {
        osection[1].vsize = (osection[1].size + oam1) &~ oam1;
        osection[2].vsize = (osection[2].size + ncsize_virt_increase + oam1) &~ oam1;
        oh.imagesize = osection[2].vaddr + osection[2].vsize;
        osection[0].rawdataptr = (pe_offset + sizeof(ht) + sizeof_osection + fam1) &~ (size_t)fam1;
        osection[1].rawdataptr = osection[0].rawdataptr;
    }
    else
    {
        osection[1].vsize = osection[1].size;
        osection[2].vsize = osection[2].size;
        osection[0].rawdataptr = 0;
        osection[1].rawdataptr = (pe_offset + sizeof(ht) + sizeof_osection + fam1) &~ (size_t)fam1;
    }
    osection[2].rawdataptr = osection[1].rawdataptr + osection[1].size;

    osection[0].flags = (unsigned) (PEFL_BSS|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[1].flags = (unsigned) (PEFL_DATA|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[2].flags = (unsigned) (PEFL_DATA|PEFL_WRITE|PEFL_READ);

    if (last_section_rsrc_only)
    {
        strcpy(osection[3].name, ".rsrc");
        osection[3].vaddr = res_start;
        osection[3].size = (soresources + fam1) &~ fam1;
        osection[3].vsize = osection[3].size;
        osection[3].rawdataptr = osection[2].rawdataptr + osection[2].size;
        osection[2].flags = (unsigned) (PEFL_DATA|PEFL_READ);
        osection[3].flags = (unsigned) (PEFL_DATA|PEFL_READ);
        oh.imagesize = (osection[3].vaddr + osection[3].vsize + oam1) &~ oam1;
        if (soresources == 0)
        {
            oh.objects = 3;
            memset(&osection[3], 0, sizeof(osection[3]));
        }
    }

    oh.bsssize  = osection[0].vsize;
    oh.datasize = osection[2].vsize + (oobjs > 3 ? osection[3].vsize : 0);
    setOhDataBase(osection);
    oh.codesize = osection[1].vsize;
    oh.codebase = osection[1].vaddr;
    setOhHeaderSize(osection);
    if (rvamin < osection[0].rawdataptr)
        throwCantPack("object alignment too small");

    if (opt->win32_pe.strip_relocs)
        oh.flags |= RELOCS_STRIPPED;

    //for (ic = 0; ic < oh.filealign; ic += 4)
    //    set_le32(ibuf + ic,get_le32("UPX "));
    ibuf.clear(0, oh.filealign);

    info("Image size change: %u -> %u KiB",
         ih.imagesize / 1024, oh.imagesize / 1024);

    infoHeader("[Writing compressed file]");

    // write loader + compressed file
    fo->write(&oh,sizeof(oh));
    fo->write(osection,sizeof(osection[0])*oobjs);
    // some alignment
    if (identsplit == identsize)
    {
        unsigned n = osection[!last_section_rsrc_only ? 0 : 1].rawdataptr - fo->getBytesWritten() - identsize;
        assert(n <= oh.filealign);
        fo->write(ibuf, n);
    }
    fo->write(loader + codesize,identsize);
    infoWriting("loader", fo->getBytesWritten());
    fo->write(obuf,c_len);
    infoWriting("compressed data", c_len);
    fo->write(loader,codesize);
    if (opt->debug.dump_stub_loader)
        OutputFile::dump(opt->debug.dump_stub_loader, loader, codesize);
    if ((ic = fo->getBytesWritten() & (sizeof(LEXX) - 1)) != 0)
        fo->write(ibuf, sizeof(LEXX) - ic);
    fo->write(raw_bytes(otls, aligned_sotls), aligned_sotls);
    fo->write(oloadconf, soloadconf);
    if ((ic = fo->getBytesWritten() & fam1) != 0)
        fo->write(ibuf,oh.filealign - ic);
    if (!last_section_rsrc_only)
        fo->write(raw_bytes(oresources, soresources) ,soresources);
    else
        fo->write(oxrelocs,soxrelocs);
    fo->write(oimpdlls,soimpdlls);
    fo->write(raw_bytes(oexport, soexport), soexport);
    if (!last_section_rsrc_only)
        fo->write(oxrelocs,soxrelocs);

    if ((ic = fo->getBytesWritten() & fam1) != 0)
        fo->write(ibuf,oh.filealign - ic);

    if (last_section_rsrc_only)
    {
        fo->write(raw_bytes(oresources, soresources) ,soresources);
        if ((ic = fo->getBytesWritten() & fam1) != 0)
            fo->write(ibuf,oh.filealign - ic);
    }

#if 0
    printf("%-13s: program hdr  : %8ld bytes\n", getName(), (long) sizeof(oh));
    printf("%-13s: sections     : %8ld bytes\n", getName(), (long) sizeof(osection[0])*oobjs);
    printf("%-13s: ident        : %8ld bytes\n", getName(), (long) identsize);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) codesize);
    printf("%-13s: tls          : %8ld bytes\n", getName(), (long) sotls);
    printf("%-13s: aligned_tls  : %8ld bytes\n", getName(), (long) aligned_sotls);
    printf("%-13s: resources    : %8ld bytes\n", getName(), (long) soresources);
    printf("%-13s: imports      : %8ld bytes\n", getName(), (long) soimpdlls);
    printf("%-13s: exports      : %8ld bytes\n", getName(), (long) soexport);
    printf("%-13s: relocs       : %8ld bytes\n", getName(), (long) soxrelocs);
    printf("%-13s: loadconf     : %8ld bytes\n", getName(), (long) soloadconf);
#endif

    // verify
    verifyOverlappingDecompression();

    // copy the overlay
    copyOverlay(fo, overlay, obuf);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}

/*************************************************************************
// unpack
**************************************************************************/

void PeFile::rebuildRelocs(SPAN_S(upx_byte) & extrainfo, unsigned bits,
                           unsigned flags, upx_uint64_t imagebase)
{
    assert(bits == 32 || bits == 64);
    if (!ODADDR(PEDIR_RELOC) || !ODSIZE(PEDIR_RELOC) || (flags & RELOCS_STRIPPED))
        return;

    if (ODSIZE(PEDIR_RELOC) == 8) // some tricky dlls use this
    {
        omemcpy(obuf + (ODADDR(PEDIR_RELOC) - rvamin), "\x0\x0\x0\x0\x8\x0\x0\x0", 8);
        return;
    }

    SPAN_P_VAR(upx_byte, rdata, obuf);
    rdata += get_le32(extrainfo);
    const upx_byte big = extrainfo[4];
    extrainfo += 5;

    MemBuffer mb_wrkmem;
    unsigned relocn = unoptimizeReloc(rdata,obuf,mb_wrkmem,true,bits);
    unsigned r16 = 0;
    if (big & 6)                // 16 bit relocations
    {
        SPAN_S_VAR(const LE32, q, (const LE32 *) raw_bytes(rdata, 0), obuf);
        while (*q++)
            r16++;
        if ((big & 6) == 6)
            while (*++q)
                r16++;
    }
    Reloc rel(relocn + r16);

    if (big & 6)
    {
        SPAN_S_VAR(LE32, q, (LE32 *) raw_bytes(rdata, 0), obuf);
        while (*q)
            rel.add(*q++ + rvamin,(big & 4) ? 2 : 1);
        if ((big & 6) == 6)
            while (*++q)
                rel.add(*q + rvamin,1);
        // rdata = (upx_byte *) raw_bytes(q, 0); // ???
    }

    SPAN_S_VAR(upx_byte, const wrkmem, mb_wrkmem);
    for (unsigned ic = 0; ic < relocn; ic++)
    {
        OPTR_I(upx_byte, p, obuf + get_le32(wrkmem + 4 * ic));
        if (bits == 32)
            set_le32(p, get_le32(p) + imagebase + rvamin);
        else
            set_le64(p, get_le64(p) + imagebase + rvamin);
        rel.add(rvamin + get_le32(wrkmem + 4 * ic), bits == 32 ? 3 : 10);
    }
    rel.finish (oxrelocs,soxrelocs);

    omemcpy(obuf + (ODADDR(PEDIR_RELOC) - rvamin), oxrelocs, soxrelocs);
    delete [] oxrelocs; oxrelocs = nullptr;
    mb_wrkmem.dealloc();

    ODSIZE(PEDIR_RELOC) = soxrelocs;
}

void PeFile::rebuildExports()
{
    if (ODSIZE(PEDIR_EXPORT) == 0 || ODADDR(PEDIR_EXPORT) == IDADDR(PEDIR_EXPORT))
        return; // nothing to do

    opt->win32_pe.compress_exports = 0;
    Export xport((char*)(unsigned char*) ibuf - isection[2].vaddr);
    processExports(&xport);
    processExports(&xport,ODADDR(PEDIR_EXPORT));
    omemcpy(obuf + (ODADDR(PEDIR_EXPORT) - rvamin), oexport, soexport);
}

void PeFile::rebuildTls()
{
    // this is an easy one : just do nothing ;-)
}

void PeFile::rebuildResources(SPAN_S(upx_byte) & extrainfo, unsigned lastvaddr)
{
    if (ODSIZE(PEDIR_RESOURCE) == 0 || IDSIZE(PEDIR_RESOURCE) == 0)
        return;

    icondir_count = get_le16(extrainfo);
    extrainfo += 2;

    const unsigned vaddr = IDADDR(PEDIR_RESOURCE);

    if (lastvaddr > vaddr || (vaddr - lastvaddr) > ibuf.getSize())
        throwCantUnpack("corrupted PE header");

    // TODO: introduce WildPtr for "virtual pointer" pointing before a buffer
    const upx_byte *r = ibuf.raw_bytes(0) - lastvaddr;
    Resource res(r + vaddr, ibuf, ibuf + ibuf.getSize());
    while (res.next())
        if (res.offs() > vaddr)
        {
            ICHECK(r + res.offs() - 4, 4);
            unsigned origoffs = get_le32(r + res.offs() - 4);
            res.newoffs() = origoffs;
            omemcpy(obuf + (origoffs - rvamin), r + res.offs(), res.size());
            if (icondir_count && res.itype() == RT_GROUP_ICON)
            {
                set_le16(obuf + (origoffs - rvamin + 4), icondir_count);
                icondir_count = 0;
            }
        }
    if (res.dirsize()) {
      upx_byte *p = res.build();
      OCHECK(obuf + (ODADDR(PEDIR_RESOURCE) - rvamin), 16);
      // write back when the original is zeroed
      if (get_le32(obuf + (ODADDR(PEDIR_RESOURCE) - rvamin + 12)) == 0)
        omemcpy(obuf + (ODADDR(PEDIR_RESOURCE) - rvamin), p, res.dirsize());
    }
}

template <typename LEXX, typename ord_mask_t>
void PeFile::rebuildImports(SPAN_S(upx_byte) & extrainfo,
                            ord_mask_t ord_mask, bool set_oft)
{
    if (ODADDR(PEDIR_IMPORT) == 0
        || ODSIZE(PEDIR_IMPORT) <= sizeof(import_desc))
        return;

    OPTR_C(const upx_byte, idata, obuf + get_le32(extrainfo));
    const unsigned inamespos = get_le32(extrainfo + 4);
    extrainfo += 8;

    unsigned sdllnames = 0;

    IPTR_I_D(const upx_byte, import, IDADDR(PEDIR_IMPORT) - isection[2].vaddr);
    OPTR_I(const upx_byte, p, raw_bytes(idata, 4));

    for ( ; get_le32(p) != 0; ++p)
    {
        const upx_byte *dname = raw_bytes(import + get_le32(p), 1);
        const unsigned dlen = strlen(dname);
        ICHECK(dname, dlen + 1);

        sdllnames += dlen + 1;
        for (p += 8; *p;)
            if (*p == 1)
                p += strlen(++p) + 1;
            else if (*p == 0xff)
                p += 3; // ordinal
            else
                p += 5;
    }
    sdllnames = ALIGN_UP(sdllnames, 2u);

    // TODO: introduce WildPtr for "virtual pointer" pointing before a buffer
    upx_byte * const Obuf = obuf.raw_bytes(0) - rvamin;
#if 0
    import_desc * const im0 = (import_desc*) (Obuf + ODADDR(PEDIR_IMPORT));
    import_desc *im = im0;
    upx_byte *dllnames = Obuf + inamespos;
    upx_byte *importednames = dllnames + sdllnames;
    upx_byte * const importednames_start = importednames;
#else
    SPAN_S_VAR(import_desc, const im0, (import_desc *) (Obuf + ODADDR(PEDIR_IMPORT)), obuf);
    SPAN_S_VAR(import_desc, im, im0);
    SPAN_0_VAR(upx_byte, dllnames, inamespos ? Obuf + inamespos : nullptr, obuf);
    SPAN_0_VAR(upx_byte, importednames, inamespos ? dllnames + sdllnames : nullptr);
    SPAN_0_VAR(upx_byte, const importednames_start, importednames);
#endif

    for (p = idata; get_le32(p) != 0; ++p)
    {
        // restore the name of the dll
        const upx_byte *dname = raw_bytes(import + get_le32(p), 1);
        const unsigned dlen = strlen(dname);
        ICHECK(dname, dlen + 1);

        const unsigned iatoffs = get_le32(p + 4) + rvamin;
        if (inamespos)
        {
            // now I rebuild the dll names
            omemcpy(dllnames, dname, dlen + 1);
            im->dllname = ptr_diff_bytes(dllnames,Obuf);
            //;;;printf("\ndll: %s:",dllnames);
            dllnames += dlen + 1;
        }
        else
        {
            omemcpy(Obuf + im->dllname, dname, dlen + 1);
        }
        im->iat = iatoffs;
        if (set_oft)
            im->oft = iatoffs;

        OPTR_I(LEXX, newiat, (LEXX *) (Obuf + iatoffs));

        // restore the imported names+ordinals
        for (p += 8; *p; ++newiat)
            if (*p == 1)
            {
                const unsigned ilen = strlen(++p) + 1;
                if (inamespos)
                {
                    if (ptr_diff_bytes(importednames, importednames_start) & 1)
                        importednames -= 1;
                    omemcpy(importednames + 2, p, ilen);
                    //;;;printf(" %s",importednames+2);
                    *newiat = ptr_diff_bytes(importednames, Obuf);
                    importednames += 2 + ilen;
                }
                else
                {
                    // Beware overlap!
                    omemmove(Obuf + (*newiat + 2), p, ilen);
                }
                p += ilen;
            }
            else if (*p == 0xff)
            {
                *newiat = get_le16(p + 1) + ord_mask;
                //;;;printf(" %x",(unsigned)*newiat);
                p += 3;
            }
            else
            {
                *newiat = * (const LEXX*) raw_bytes(import + get_le32(p + 1), sizeof(LEXX));
                assert(*newiat & ord_mask);
                p += 5;
            }
        *newiat = 0;
        im++;
    }
    //memset(idata,0,p - idata);
}

template <typename ht, typename LEXX, typename ord_mask_t>
void PeFile::unpack0(OutputFile *fo, const ht &ih, ht &oh,
                     ord_mask_t ord_mask, bool set_oft)
{
    //infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);

    handleStub(fi,fo,pe_offset);
    if (ih.filealign == 0)
        throwCantUnpack("unexpected value in the PE header");

    const unsigned iobjs = ih.objects;
    const unsigned overlay = file_size - ALIGN_UP(isection[iobjs - 1].rawdataptr
                                                  + isection[iobjs - 1].size,
                                                  ih.filealign);
    checkOverlay(overlay);

    ibuf.alloc(ph.c_len);
    obuf.allocForDecompression(ph.u_len);
    fi->seek(isection[1].rawdataptr - 64 + ph.buf_offset + ph.getPackHeaderSize(),SEEK_SET);
    fi->readx(ibuf, ibufgood= ph.c_len);

    // decompress
    decompress(ibuf,obuf);
    unsigned skip = get_le32(obuf + ph.u_len - 4);
    unsigned take = sizeof(oh);
    SPAN_S_VAR(upx_byte, extrainfo, obuf);
    extrainfo = obuf.subref("bad extrainfo offset %#x", skip, take);
    //upx_byte * const eistart = raw_bytes(extrainfo, 0);

    memcpy(&oh, extrainfo, take);
    extrainfo += take;
    skip      += take;
    unsigned objs = oh.objects;

    if ((int) objs <= 0 || (iobjs > 2 && isection[2].size == 0))
        throwCantUnpack("unexpected value in the PE header");
    Array(pe_section_t, osection, objs);
    take = sizeof(pe_section_t) * objs;
    extrainfo = obuf.subref("bad extra section size at %#x", skip, take);
    memcpy(osection, extrainfo, take);
    extrainfo += take;
    skip      += take;
    rvamin = osection[0].vaddr;

    if (iobjs > 2)
    {
        // read the noncompressed section
        ibuf.dealloc();
        ibuf.alloc(isection[2].size);
        fi->seek(isection[2].rawdataptr,SEEK_SET);
        fi->readx(ibuf, ibufgood= isection[2].size);
    }

    // unfilter
    if (ph.filter)
    {
        Filter ft(ph.level);
        ft.init(ph.filter,oh.codebase - rvamin);
        ft.cto = (unsigned char) ph.filter_cto;
        OCHECK(obuf + (oh.codebase - rvamin), oh.codesize);
        ft.unfilter(obuf + (oh.codebase - rvamin), oh.codesize);
    }

    // FIXME: ih.flags is checked here because of a bug in UPX 0.92
    if (ih.flags & RELOCS_STRIPPED)
    {
        oh.flags |= RELOCS_STRIPPED;
        ODADDR(PEDIR_RELOC) = 0;
        ODSIZE(PEDIR_RELOC) = 0;
    }

    rebuildImports<LEXX>(extrainfo, ord_mask, set_oft);
    rebuildRelocs(extrainfo, sizeof(ih.imagebase) * 8, oh.flags, oh.imagebase);
    rebuildTls();
    rebuildExports();

    if (iobjs > 3)
    {
        // read the resource section if present
        ibuf.dealloc();
        ibuf.alloc(isection[3].size);
        fi->seek(isection[3].rawdataptr,SEEK_SET);
        fi->readx(ibuf, ibufgood= isection[3].size);
    }

    rebuildResources(extrainfo, isection[ih.objects - 1].vaddr);

    //FIXME: this does bad things if the relocation section got removed
    // during compression ...
    //memset(eistart, 0, ptr_udiff_bytes(extrainfo, eistart) + 4);

    // fill the data directory
    ODADDR(PEDIR_DEBUG) = 0;
    ODSIZE(PEDIR_DEBUG) = 0;
    ODADDR(PEDIR_IAT) = 0;
    ODSIZE(PEDIR_IAT) = 0;
    ODADDR(PEDIR_BOUNDIM) = 0;
    ODSIZE(PEDIR_BOUNDIM) = 0;

    setOhHeaderSize(osection);
    oh.chksum = 0;

    // write decompressed file
    if (fo)
    {
        unsigned ic;
        for (ic = 0; ic < objs && osection[ic].rawdataptr == 0; ic++)
            ;

        ibuf.dealloc();
        ibuf.alloc(osection[ic].rawdataptr);
        ibuf.clear();
        infoHeader("[Writing uncompressed file]");

        // write loader + compressed file
        fo->write(&oh,sizeof(oh));
        fo->write(osection,objs * sizeof(pe_section_t));
        fo->write(ibuf,osection[ic].rawdataptr - fo->getBytesWritten());
        for (ic = 0; ic < objs; ic++)
            if (osection[ic].rawdataptr)
                fo->write(obuf + (osection[ic].vaddr - rvamin), ALIGN_UP(osection[ic].size,oh.filealign));
        copyOverlay(fo, overlay, obuf);
    }
    ibuf.dealloc();
}

int PeFile::canUnpack0(unsigned max_sections, LE16 &ih_objects,
                       LE32 &ih_entry, unsigned ihsize)
{
    if (!canPack())
        return false;

    unsigned objs = ih_objects;
    mb_isection.alloc(sizeof(pe_section_t) * objs);
    isection = (pe_section_t *)mb_isection.getVoidPtr();
    fi->seek(pe_offset + ihsize, SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);
    const unsigned min_sections = isefi ? 2 : 3;
    if (ih_objects < min_sections)
        return -1;
    bool is_packed = (ih_objects >= min_sections && ih_objects <= max_sections &&
                      (IDSIZE(15) || ih_entry > isection[1].vaddr));
    bool found_ph = false;
    if (memcmp(isection[0].name,"UPX",3) == 0)
    {
        // current version
        fi->seek(isection[1].rawdataptr - 64, SEEK_SET);
        found_ph = readPackHeader(1024);
        if (!found_ph)
        {
            // old versions
            fi->seek(isection[2].rawdataptr, SEEK_SET);
            found_ph = readPackHeader(1024);
        }
    }
    if (is_packed && found_ph)
        return true;
    if (!is_packed && !found_ph)
        return -1;
    if (is_packed && ih_entry < isection[2].vaddr)
    {
        unsigned char buf[256];
        bool x = false;

        memset(buf, 0, sizeof(buf));
        try {
            fi->seek(ih_entry - isection[1].vaddr + isection[1].rawdataptr, SEEK_SET);
            fi->read(buf, sizeof(buf));

            // FIXME this is for x86
            static const unsigned char magic[] = "\x8b\x1e\x83\xee\xfc\x11\xdb";
            // mov ebx, [esi];    sub esi, -4;    adc ebx,ebx

            int offset = find(buf, sizeof(buf), magic, 7);
            if (offset >= 0 && find(buf + offset + 1, sizeof(buf) - offset - 1, magic, 7) >= 0)
                x = true;
        } catch (...) {
            //x = true;
        }
        if (x)
            throwCantUnpack("file is modified/hacked/protected; take care!!!");
        else
            throwCantUnpack("file is possibly modified/hacked/protected; take care!");
        return false;   // not reached
    }

    // FIXME: what should we say here ?
    //throwCantUnpack("file is possibly modified/hacked/protected; take care!");
    return false;
}

upx_uint64_t PeFile::ilinkerGetAddress(const char *d, const char *n) const
{
    return ilinker->getAddress(d, n);
}

PeFile::~PeFile()
{
    oimpdlls = nullptr;
    delete [] oxrelocs;
    delete ilinker;
    //delete res;
}


/*************************************************************************
//  PeFile32
**************************************************************************/

PeFile32::PeFile32(InputFile *f) : super(f)
{
    COMPILE_TIME_ASSERT(sizeof(pe_header_t) == 248)
    COMPILE_TIME_ASSERT_ALIGNED1(pe_header_t)

    iddirs = ih.ddirs;
    oddirs = oh.ddirs;
}

PeFile32::~PeFile32()
{}

void PeFile32::readPeHeader()
{
    fi->readx(&ih,sizeof(ih));
    isefi = ((1u << ih.subsystem) & (
                                (1u<<IMAGE_SUBSYSTEM_EFI_APPLICATION)
                              | (1u<<IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER)
                              | (1u<<IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER)
                              | (1u<<IMAGE_SUBSYSTEM_EFI_ROM)
                                  )) != 0;
    isdll = !isefi && (ih.flags & DLL_FLAG) != 0;
    use_dep_hack &= !isefi;
    use_clear_dirty_stack &= !isefi;
}

void PeFile32::pack0(OutputFile *fo, unsigned subsystem_mask,
                     upx_uint64_t default_imagebase,
                     bool last_section_rsrc_only)
{
    super::pack0<LE32>(fo, ih, oh, subsystem_mask,
                       default_imagebase, last_section_rsrc_only);
    infoWarning("End of PeFile32::pack0");
}

void PeFile32::unpack(OutputFile *fo)
{
    bool set_oft = getFormat() == UPX_F_WINCE_ARM_PE;
    unpack0<pe_header_t, LE32>(fo, ih, oh, 1U << 31, set_oft);
}

int PeFile32::canUnpack()
{
    return canUnpack0(getFormat() == UPX_F_WINCE_ARM_PE ? 4 : 3,
                      ih.objects, ih.entry, sizeof(ih));
}

unsigned PeFile32::processImports() // pass 1
{
    return processImports0<LE32>(1u << 31);
}

void PeFile32::processTls(Interval *iv)
{
    processTls1<LE32>(iv, ih.imagebase, ih.imagesize);
}

void PeFile32::processTls(Reloc *r, const Interval *iv, unsigned a)
{
    processTls2<LE32>(r, iv, a, ih.imagebase);
}

/*************************************************************************
//  PeFile64
**************************************************************************/

PeFile64::PeFile64(InputFile *f) : super(f)
{
    COMPILE_TIME_ASSERT(sizeof(pe_header_t) == 264)
    COMPILE_TIME_ASSERT_ALIGNED1(pe_header_t)

    iddirs = ih.ddirs;
    oddirs = oh.ddirs;
}

PeFile64::~PeFile64()
{}

void PeFile64::readPeHeader()
{
    fi->readx(&ih,sizeof(ih));
    isefi = ((1u << ih.subsystem) & (
                                (1u<<IMAGE_SUBSYSTEM_EFI_APPLICATION)
                              | (1u<<IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER)
                              | (1u<<IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER)
                              | (1u<<IMAGE_SUBSYSTEM_EFI_ROM)
                                  )) != 0;
    isdll = !isefi && (ih.flags & DLL_FLAG) != 0;
    use_dep_hack &= !isefi;
    use_clear_dirty_stack &= !isefi;
}

void PeFile64::pack0(OutputFile *fo, unsigned subsystem_mask,
                     upx_uint64_t default_imagebase)
{
    super::pack0<LE64>(fo, ih, oh, subsystem_mask, default_imagebase, false);
}

void PeFile64::unpack(OutputFile *fo)
{
    unpack0<pe_header_t, LE64>(fo, ih, oh, 1ULL << 63, false);
}

int PeFile64::canUnpack()
{
    return canUnpack0(3, ih.objects, ih.entry, sizeof(ih));
}

unsigned PeFile64::processImports() // pass 1
{
    return processImports0<LE64>(1ULL << 63);
}

void PeFile64::processTls(Interval *iv)
{
    processTls1<LE64>(iv, ih.imagebase, ih.imagesize);
}

void PeFile64::processTls(Reloc *r, const Interval *iv, unsigned a)
{
    processTls2<LE64>(r, iv, a, ih.imagebase);
}

/*
 extra info added to help uncompression:

 <ih sizeof(pe_head)>
 <pe_section_t objs*sizeof(pe_section_t)>
 <start of compressed imports 4> - optional           \
 <start of the names from uncompressed imports> - opt /
 <start of compressed relocs 4> - optional   \
 <relocation type indicator 1> - optional    /
 <icondir_count 2> - optional
 <offset of extra info 4>
*/


/* vim:set ts=4 sw=4 et: */

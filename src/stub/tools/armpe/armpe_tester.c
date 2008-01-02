/* armpe_tester.c -- ARM/PE loader/tester for arm linux

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

// arm-9tdmi-linux-gnu-gcc -Wl,--section-start,.interp=0x1000
// arm-wince-pe-gcc -Wl,--image-base,0x400000

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#ifdef i386
# define UPX_MMAP_ADDRESS 0x20000000
#else
# define UPX_MMAP_ADDRESS 0x10000
#endif

#ifdef linux
# include <sys/mman.h>
#else
void *VirtualAlloc(void *address, unsigned size, unsigned type, unsigned protect);
# define MEM_COMMIT 0x1000
# define PAGE_EXECUTE_READWRITE 0x0040
#endif

typedef unsigned short LE16;
typedef unsigned long  LE32;
#define __attribute_packed

struct ddirs_t
{
    LE32    vaddr;
    LE32    size;
}
__attribute_packed;

struct pe_header_t
{
    // 0x0
    char    _[4];
    LE16    cpu;
    LE16    objects;
    char    __[12];
    LE16    opthdrsize;
    LE16    flags;
    // optional header
    char    ___[4];
    LE32    codesize;
    // 0x20
    LE32    datasize;
    LE32    bsssize;
    LE32    entry;
    LE32    codebase;
    // 0x30
    LE32    database;
    // nt specific fields
    LE32    imagebase;
    LE32    objectalign;
    LE32    filealign;
    // 0x40
    char    ____[16];
    // 0x50
    LE32    imagesize;
    LE32    headersize;
    LE32    chksum;
    LE16    subsystem;
    LE16    dllflags;
    // 0x60
    char    _____[20];
    // 0x74
    LE32    ddirsentries;

    struct ddirs_t ddirs[16];
}
__attribute_packed;

struct pe_section_t
{
    char    name[8];
    LE32    vsize;
    LE32    vaddr;
    LE32    size;
    LE32    rawdataptr;
    char    _[12];
    LE32    flags;
}
__attribute_packed;


struct exe_header_t
{
    LE16 mz;
    LE16 m512;
    LE16 p512;
    char _[18];
    LE16 relocoffs;
    char __[34];
    LE32 nexepos;
}
__attribute_packed;

enum {
    PEDIR_EXPORT    = 0,
    PEDIR_IMPORT    = 1,
    PEDIR_RESOURCE  = 2,
    PEDIR_EXCEPTION = 3,
    PEDIR_SEC       = 4,
    PEDIR_RELOC     = 5,
    PEDIR_DEBUG     = 6,
    PEDIR_COPYRIGHT = 7,
    PEDIR_GLOBALPTR = 8,
    PEDIR_TLS       = 9,
    PEDIR_LOADCONF  = 10,
    PEDIR_BOUNDIM   = 11,
    PEDIR_IAT       = 12,
    PEDIR_DELAYIMP  = 13,
    PEDIR_COMRT     = 14
};

#define get_le32(p) (*(unsigned *) (p))
#define set_le32(p, v) (*(unsigned *) (p) = (v))
#define get_le16(p) (*(unsigned short *) (p))

static struct pe_header_t ih;
static struct pe_section_t isections[4];
static FILE *f;
static void *vaddr;
static FILE *out;

static int print(const char *format, ...)
{
    va_list ap;
    int ret;

    va_start(ap, format);
    ret = fprintf(out, format, ap);
    fflush(out);
    va_end(ap);
    return ret;
}

static int load(const char *file)
{
    struct exe_header_t h;
    int ic;
    unsigned pe_offset = 0;

    if ((f = fopen(file, "rb")) == NULL)
        return print("can not open file: %s\n", file);

    for (ic = 0; ic < 20; ic++)
    {
        if (fseek(f, pe_offset, SEEK_SET)
            || fread(&h, sizeof(h), 1, f) != 1)
            return print("read error at %u\n", pe_offset);

        if (h.mz == 'M' + 'Z'*256) // dos exe
        {
            if (h.relocoffs >= 0x40)      // new format exe
                pe_offset += h.nexepos;
            else
                pe_offset += h.p512 * 512 + h.m512 - h.m512 ? 512 : 0;
        }
        else if (get_le32(&h) == 'P' + 'E'*256)
            break;
        else
            return print("bad header at %u\n", pe_offset);
    }
    if (ic == 20)
        return print("pe header not found\n");
    if (fseek(f, pe_offset, SEEK_SET)
        || fread(&ih, sizeof(ih), 1, f) != 1)
        return print("can not load pe header\n");

    if (ih.cpu != 0x1c0 && ih.cpu != 0x1c2)
        return print("unsupported processor type: %x\n", ih.cpu);

    if ((ih.objects != 3 && ih.objects != 4)
        || fread(isections, sizeof(isections), 1, f) != 1)
        return print("error reading section descriptors\n");

    return 0;
}

static int read(void)
{
    unsigned ic;
#ifdef linux
    vaddr = mmap((void *) UPX_MMAP_ADDRESS, ih.imagesize,
                 PROT_WRITE | PROT_READ | PROT_EXEC,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (((int) vaddr) == -1)
        return print("mmap() failed: %d\n", errno);
#else
    if ((vaddr = VirtualAlloc(0, ih.imagesize,
                              MEM_COMMIT, PAGE_EXECUTE_READWRITE)) == 0)
        return print("VirtualAlloc() failed\n");
    print("VirtualAlloc() ok %x\n", vaddr);
#endif

    for (ic = 1; ic <= (unsigned) ih.objects - 1; ic++)
        if (fseek(f, isections[ic].rawdataptr, SEEK_SET)
            || fread(vaddr + isections[ic].vaddr,
                     isections[ic].vsize, 1, f) != 1)
            return print("error reading section %u\n", ic);
    return 0;
}

static void dump(char n)
{
    char buf[100];
#ifdef linux
    snprintf(buf, sizeof(buf), "/tmp/a.dump%c", n);
#else
    snprintf(buf, sizeof(buf), "/a.dump%c", n);
#endif
    FILE *f2 = fopen(buf, "wb");
    fwrite(vaddr + 0x1000, ih.imagesize - 0x1000, 1, f2);
    fclose(f2);
}

static int loadlibraryw(unsigned short *name)
{
    return name[0] + name[1] * 0x100 + name[2] * 0x10000;
}

static int getprocaddressa(unsigned h, const char *proc)
{
    unsigned p = (unsigned) proc;
    if (p < 0x10000)
    {
        print("getprocaddressa called %c%c%c, ordinal %u\n",
               h, h >> 8, h >> 16, p);
        return h + p * 0x10000;
    }
    print("getprocaddressa called %c%c%c, name %s\n",
           h, h >> 8, h >> 16, proc);
    return h + proc[0] * 0x10000 + proc[1] * 0x1000000;
}

static void cachesync(unsigned v)
{
    print("cachesync called %u\n", v);
}

static int import(void)
{
    if (ih.ddirs[PEDIR_IMPORT].vaddr == 0)
        return print("no imports?\n");
    void *imports = vaddr + ih.ddirs[PEDIR_IMPORT].vaddr;
    void *coredll_imports = vaddr + get_le32(imports + 16);
    set_le32(coredll_imports, (unsigned) loadlibraryw);
    set_le32(coredll_imports + 4, (unsigned) getprocaddressa);
    set_le32(coredll_imports + 8, (unsigned) cachesync);
    return 0;
}

static int reloc(void)
{
    if (ih.ddirs[PEDIR_RELOC].vaddr == 0)
        return 0;
    void *relocs = vaddr + ih.ddirs[PEDIR_RELOC].vaddr;
    void *page = vaddr + get_le32(relocs);
    unsigned size = get_le32(relocs + 4);
    if (size != ih.ddirs[PEDIR_RELOC].size)
        return print("only 1 page can be relocated\n");
    unsigned num =  (size - 8) / 2;
    while (num--)
    {
        unsigned pos = get_le16(relocs + 8 + num * 2);
        if (pos == 0)
            continue;
        if ((pos & 0xF000) != 0x3000)
            return print("unknown relocation type: %x\n", pos);

        void *r = page + (pos & 0xFFF);
        set_le32(r, get_le32(r) - ih.imagebase + (unsigned) vaddr);
    }
    return 0;
}

static void dump2(int c)
{
    print("dump2 %c\n", c);
    dump(c);
}

static void call(void)
{
#ifndef i386
    void (*entry)(void (*)(int), unsigned) = vaddr + ih.entry;
    entry(dump2, 1);
    dump('z');
#endif
}

static int main2(int argc, char **argv)
{
    if (argc != 2)
        return print("usage: %s arm_pe_file\n", argv[0]), 1;
    if (load(argv[1]))
        return 2;
    if (read())
        return 3;
    dump('0');
    if (import())
        return 4;
    dump('1');
    if (reloc())
        return 5;
    dump('2');

    call();
    print("ok.\n");
    return 0;
}

int main(int argc, char **argv)
{
    out = stdout;
#ifndef linux
    out = fopen("/wtest.log", "wt");
#endif
    int ret = main2(argc, argv);
    fclose(out);
    return ret;
}

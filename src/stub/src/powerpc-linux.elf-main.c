/* powerpc-linux.elf-main.c -- stub loader for Linux ELF PowerPC32 executable

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


#include "include/linux.h"


/*************************************************************************
// configuration section
**************************************************************************/

// In order to make it much easier to move this code at runtime and execute
// it at an address different from it load address:  there must be no
// static data, and no string constants.

#define MAX_ELF_HDR 512  // Elf32_Ehdr + n*Elf32_Phdr must fit in this


/*************************************************************************
// "file" util
**************************************************************************/

typedef struct {
    size_t size;  // must be first to match size[0] uncompressed size
    char *buf;
} Extent;


static void
xread(Extent *x, char *buf, size_t count)
{
    char *p=x->buf, *q=buf;
    size_t j;
    if (x->size < count) {
        exit(127);
    }
    for (j = count; 0!=j--; ++p, ++q) {
        *q = *p;
    }
    x->buf  += count;
    x->size -= count;
}


/*************************************************************************
// util
**************************************************************************/

#if 1  //{  save space
#define ERR_LAB error: exit(127);
#define err_exit(a) goto error
#else  //}{  save debugging time
#define ERR_LAB
static void
err_exit(int a)
{
    (void)a;  // debugging convenience
    exit(127);
}
#endif  //}

/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

typedef void f_unfilter(
    nrv_byte *,  // also addvalue
    nrv_uint,
    unsigned cto8, // junk in high 24 bits
    unsigned ftid
);
typedef int f_expand(
    const nrv_byte *, nrv_uint,
          nrv_byte *, nrv_uint *, unsigned );

static void
unpackExtent(
    Extent *const xi,  // input
    Extent *const xo,  // output
    f_expand *const f_decompress,
    f_unfilter *f_unf
)
{
    while (xo->size) {
        struct b_info h;
        //   Note: if h.sz_unc == h.sz_cpr then the block was not
        //   compressible and is stored in its uncompressed form.

        // Read and check block sizes.
        xread(xi, (char *)&h, sizeof(h));
        if (h.sz_unc == 0) {                     // uncompressed size 0 -> EOF
            if (h.sz_cpr != UPX_MAGIC_LE32)      // h.sz_cpr must be h->magic
                err_exit(2);
            if (xi->size != 0)                 // all bytes must be written
                err_exit(3);
            break;
        }
        if (h.sz_cpr <= 0) {
            err_exit(4);
ERR_LAB
        }
        if (h.sz_cpr > h.sz_unc
        ||  h.sz_unc > xo->size ) {
            err_exit(5);
        }
        // Now we have:
        //   assert(h.sz_cpr <= h.sz_unc);
        //   assert(h.sz_unc > 0 && h.sz_unc <= blocksize);
        //   assert(h.sz_cpr > 0 && h.sz_cpr <= blocksize);

        if (h.sz_cpr < h.sz_unc) { // Decompress block
            nrv_uint out_len = h.sz_unc;  // EOF for lzma
            int const j = (*f_decompress)(xi->buf, h.sz_cpr,
                xo->buf, &out_len, h.b_method);
            if (j != 0 || out_len != (nrv_uint)h.sz_unc)
                err_exit(7);
            if (h.b_ftid!=0 && f_unf) {  // have filter
                (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);
            }
            xi->buf  += h.sz_cpr;
            xi->size -= h.sz_cpr;
        }
        else { // copy literal block
            xread(xi, xo->buf, h.sz_cpr);
        }
        xo->buf  += h.sz_unc;
        xo->size -= h.sz_unc;
    }
}

static void
upx_bzero(char *p, size_t len)
{
    if (len) do {
        *p++= 0;
    } while (--len);
}
#define bzero upx_bzero


static void
auxv_up(Elf32_auxv_t *av, unsigned type, unsigned const value)
{
    if (av)
    for (;; ++av) {
        if (av->a_type==type || (av->a_type==AT_IGNORE && type!=AT_NULL)) {
            av->a_type = type;
            av->a_un.a_val = value;
            return;
        }
    }
}

// The PF_* and PROT_* bits are {1,2,4}; the conversion table fits in 32 bits.
#define REP8(x) \
    ((x)|((x)<<4)|((x)<<8)|((x)<<12)|((x)<<16)|((x)<<20)|((x)<<24)|((x)<<28))
#define EXP8(y) \
    ((1&(y)) ? 0xf0f0f0f0 : (2&(y)) ? 0xff00ff00 : (4&(y)) ? 0xffff0000 : 0)
#define PF_TO_PROT(pf) \
    ((PROT_READ|PROT_WRITE|PROT_EXEC) & ( \
        ( (REP8(PROT_EXEC ) & EXP8(PF_X)) \
         |(REP8(PROT_READ ) & EXP8(PF_R)) \
         |(REP8(PROT_WRITE) & EXP8(PF_W)) \
        ) >> ((pf & (PF_R|PF_W|PF_X))<<2) ))


// Find convex hull of PT_LOAD (the minimal interval which covers all PT_LOAD),
// and mmap that much, to be sure that a kernel using exec-shield-randomize
// won't place the first piece in a way that leaves no room for the rest.
static unsigned long  // returns relocation constant
xfind_pages(unsigned mflags, Elf32_Phdr const *phdr, int phnum,
    char **const p_brk
)
{
    size_t lo= ~0, hi= 0, szlo= 0;
    char *addr;
    mflags += MAP_PRIVATE | MAP_ANONYMOUS | MAP_DENYWRITE;  // '+' can optimize better than '|'
    for (; --phnum>=0; ++phdr) if (PT_LOAD==phdr->p_type) {
        if (phdr->p_vaddr < lo) {
            lo = phdr->p_vaddr;
            szlo = phdr->p_filesz;
        }
        if (hi < (phdr->p_memsz + phdr->p_vaddr)) {
            hi =  phdr->p_memsz + phdr->p_vaddr;
        }
    }
    szlo += ~PAGE_MASK & lo;  // page fragment on lo edge
    lo   -= ~PAGE_MASK & lo;  // round down to page boundary
    hi    =  PAGE_MASK & (hi - lo - PAGE_MASK -1);  // page length
    szlo  =  PAGE_MASK & (szlo    - PAGE_MASK -1);  // page length
    addr = mmap((void *)lo, hi, PROT_NONE, mflags, -1, 0);
    *p_brk = hi + addr;  // the logical value of brk(0)
    //mprotect(szlo + addr, hi - szlo, PROT_NONE);  // but keep the frames!
    return (unsigned long)addr - lo;
}

static Elf32_Addr  // entry address
do_xmap(
    Elf32_Ehdr const *const ehdr,
    Extent *const xi,
    int const fdi,
    Elf32_auxv_t *const av,
    f_expand *const f_decompress,
    f_unfilter *const f_unf
)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (char const *)ehdr);
    char *v_brk;
    unsigned long const reloc = xfind_pages(
        ((ET_DYN!=ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum, &v_brk);
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (xi && PT_PHDR==phdr->p_type) {
        auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
    } else
    if (PT_LOAD==phdr->p_type) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char  *addr = xo.buf  =                 (char *)phdr->p_vaddr;
        char *haddr =           phdr->p_memsz +                  addr;
        size_t frag  = (int)addr &~ PAGE_MASK;
        mlen += frag;
        addr -= frag;
        addr  += reloc;
        haddr += reloc;

        if (addr != mmap(addr, mlen, prot | (xi ? PROT_WRITE : 0),
                MAP_FIXED | MAP_PRIVATE | (xi ? MAP_ANONYMOUS : 0),
                (xi ? -1 : fdi), phdr->p_offset - frag) ) {
            err_exit(8);
        }
        if (xi) {
            unpackExtent(xi, &xo, f_decompress, f_unf);
        }
        // Linux does not fixup the low end, so neither do we.
        //if (PROT_WRITE & prot) {
        //    bzero(addr, frag);  // fragment at lo end
        //}
        frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
        if (PROT_WRITE & prot) {
            bzero(mlen+addr, frag);  // fragment at hi end
        }
        if (xi) {
            if (0!=mprotect(addr, mlen, prot)) {
                err_exit(10);
ERR_LAB
            }
        }
        addr += mlen + frag;  /* page boundary on hi end */
        if (addr < haddr) { // need pages for .bss
            if (addr != mmap(addr, haddr - addr, prot,
                    MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) ) {
                err_exit(9);
            }
        }
    }
    return ehdr->e_entry + reloc;
}


/*************************************************************************
// upx_main - called by our entry code
**************************************************************************/

void *upx_main(
    struct l_info const *const li,
    size_t const sz_compressed,  // total length
    Elf32_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    size_t const sz_ehdr,
    f_expand *const f_decompress,
    f_unfilter *const f_unf,
    Elf32_auxv_t *const av
)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(1+ ehdr);
    Elf32_Addr entry;

    Extent xi, xo, xi0;
    xi.buf  = CONST_CAST(char *, 1+ (struct p_info const *)(1+ li));  // &b_info
    xi.size = sz_compressed - (sizeof(struct l_info) + sizeof(struct p_info));
    xo.buf  = (char *)ehdr;
    xo.size = ((struct b_info const *)xi.buf)->sz_unc;
    xi0 = xi;

    ACC_UNUSED(sz_ehdr);

    // ehdr = Uncompress Ehdr and Phdrs
    unpackExtent(&xi, &xo, f_decompress, 0);  // never filtered?

    // AT_PHDR.a_un.a_val  is set again by do_xmap if PT_PHDR is present.
    auxv_up(av, AT_PHDR  , (unsigned)(1+(Elf32_Ehdr *)phdr->p_vaddr));
    auxv_up(av, AT_PHNUM , ehdr->e_phnum);
    auxv_up(av, AT_ENTRY , (unsigned)ehdr->e_entry);
    //auxv_up(av, AT_PHENT , ehdr->e_phentsize);  /* this can never change */
    //auxv_up(av, AT_PAGESZ, PAGE_SIZE);  /* ld-linux.so.2 does not need this */

    entry = do_xmap(ehdr, &xi0, 0, av, f_decompress, f_unf);

  { // Map PT_INTERP program interpreter
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        char const *const iname = (char const *)phdr->p_vaddr;
        int const fdi = open(iname, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
        if (MAX_ELF_HDR!=read(fdi, (void *)ehdr, MAX_ELF_HDR)) {
ERR_LAB
            err_exit(19);
        }
        entry = do_xmap(ehdr, 0, fdi, 0, 0, 0);
        close(fdi);
    }
  }

    return (void *)entry;
}


/*
vi:ts=4:et:nowrap
*/


/* i386-linux.elf.shell-main.c -- stub loader for Linux x86 shell script executable

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

struct Extent {
    size_t size;  // must be first to match size[0] uncompressed size
    char *buf;
};


static void
xread(struct Extent *x, char *buf, size_t count)
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

static void *
do_brk(void *addr)
{
    return brk(addr);
}

extern char *mmap(void *addr, size_t len,
    int prot, int flags, int fd, off_t offset);

/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

typedef int f_expand(
    const nrv_byte *, nrv_uint,
          nrv_byte *, nrv_uint *, int method );

static void
unpackExtent(
    struct Extent *const xi,  // input
    struct Extent *const xo,  // output
    f_expand *const f_decompress
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
            int const j = (*f_decompress)((unsigned char *)xi->buf, h.sz_cpr,
                (unsigned char *)xo->buf, &out_len, *(int *)(void *)&h.b_method );
            if (j != 0 || out_len != (nrv_uint)h.sz_unc)
                err_exit(7);
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
bzero(char *p, size_t len)
{
    if (len) do {
        *p++= 0;
    } while (--len);
}

#define REP8(x) \
    ((x)|((x)<<4)|((x)<<8)|((x)<<12)|((x)<<16)|((x)<<20)|((x)<<24)|((x)<<28))
#define EXP8(y) \
    ((1&(y)) ? 0xf0f0f0f0 : (2&(y)) ? 0xff00ff00 : (4&(y)) ? 0xffff0000 : 0)
#define PF_TO_PROT(pf) \
          (7 & (( (REP8(PROT_EXEC ) & EXP8(PF_X)) \
                 |(REP8(PROT_READ ) & EXP8(PF_R)) \
                 |(REP8(PROT_WRITE) & EXP8(PF_W)) ) \
               >> ((pf & (PF_R|PF_W|PF_X))<<2) ))


// Find convex hull of PT_LOAD (the minimal interval which covers all PT_LOAD),
// and mmap that much, to be sure that a kernel using exec-shield-randomize
// won't place the first piece in a way that leaves no room for the rest.
static unsigned long  // returns relocation constant
__attribute__((regparm(3), stdcall))
xfind_pages(unsigned mflags, Elf32_Phdr const *phdr, int phnum)
{
    size_t lo= ~0, hi= 0, szlo= 0;
    char *addr;
    mflags += MAP_PRIVATE | MAP_ANONYMOUS;  // '+' can optimize better than '|'
    for (; --phnum>=0; ++phdr) if (PT_LOAD==phdr->p_type) {
        if (phdr->p_vaddr < lo) {
            lo = phdr->p_vaddr;
            szlo = phdr->p_filesz;
        }
        if (hi < (phdr->p_memsz + phdr->p_vaddr)) {
            hi =  phdr->p_memsz + phdr->p_vaddr;
        }
    }
    if (MAP_FIXED & mflags) { // the "shell", and not the PT_INTERP
        // This is a dirty hack to set the proper value for brk(0) as seen by
        // the "shell" which we will mmap() soon, upon return to do_xmap().
        // It depends on our own brk() starting out at 0x08048000, which is the
        // default base address used by /bin/ld for an ET_EXEC.  We must brk()
        // now.  If we wait until after mmap() of shell pages, then the kernel
        // "vma" containing our original brk() of 0x08048000 will not be contiguous
        // with  hi  [the mmap'ed pages from the shell will be in between],
        // and various linux kernels will not move the brk() in this case;
        // the typical symptom is SIGSEGV early in ld-linux.so.2 (the PT_INTERP).
        do_brk((void *)hi);
    }
    szlo += ~PAGE_MASK & lo;  // page fragment on lo edge
    lo   -= ~PAGE_MASK & lo;  // round down to page boundary
    hi    =  PAGE_MASK & (hi - lo - PAGE_MASK -1);  // page length
    szlo  =  PAGE_MASK & (szlo    - PAGE_MASK -1);  // page length
    addr = mmap((void *)lo, hi, PROT_READ|PROT_WRITE|PROT_EXEC, mflags, -1, 0);

    // Doing this may destroy the brk() that we set so carefully above.
    // The munmap() is "needed" only for discontiguous PT_LOAD,
    // and neither shells nor ld-linux.so.2 have that.
    // munmap(szlo + addr, hi - szlo);

    return (unsigned long)addr - lo;
}

// This do_xmap() has no Extent *xi input because it doesn't decompress anything;
// it only maps the shell and its PT_INTERP.  So, it was specialized by hand
// to reduce compiled instruction size.  gcc 2.91.66 does not notice that
// there is only one call to this static function (from getexec(), which
// would specify 0 for xi), so gcc does not propagate the constant parameter.
// Notice there is no make_hatch(), either.

static Elf32_Addr  // entry address
do_xmap(int const fdi, Elf32_Ehdr const *const ehdr, Elf32_auxv_t *const av)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (char const *)ehdr);
    unsigned long const reloc = xfind_pages(
        ((ET_DYN!=ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum);
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (PT_PHDR==phdr->p_type) {
        av[AT_PHDR -1].a_un.a_val = phdr->p_vaddr;
    }
    else if (PT_LOAD==phdr->p_type) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        struct Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char  *addr = xo.buf  =                 (char *)phdr->p_vaddr;
        char *haddr =           phdr->p_memsz +                  addr;
        size_t frag  = (int)addr &~ PAGE_MASK;
        mlen += frag;
        addr -= frag;
        addr  += reloc;
        haddr += reloc;

        // Decompressor can overrun the destination by 3 bytes.
        if (addr != mmap(addr, mlen, PROT_READ | PROT_WRITE,
                MAP_FIXED | MAP_PRIVATE,
                fdi, phdr->p_offset - frag) ) {
            err_exit(8);
        }
        bzero(addr, frag);  // fragment at lo end
        frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
        bzero(mlen+addr, frag);  // fragment at hi end
        if (0!=mprotect(addr, mlen, prot)) {
            err_exit(10);
ERR_LAB
        }
        addr += mlen + frag;  /* page boundary on hi end */
        if (addr < haddr) { // need pages for .bss
            if (addr != mmap(addr, haddr - addr, prot,
                    MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) ) {
                err_exit(9);
            }
        }
    }
    if (0!=close(fdi)) {
        err_exit(11);
    }
    return ehdr->e_entry + reloc;
}

static Elf32_Addr  // entry address
getexec(char const *const fname, Elf32_Ehdr *const ehdr, Elf32_auxv_t *const av)
{
    int const fdi = open(fname, O_RDONLY, 0);
    if (0 > fdi) {
        err_exit(18);
ERR_LAB
    }
    if (MAX_ELF_HDR!=read(fdi, (void *)ehdr, MAX_ELF_HDR)) {
        err_exit(19);
    }
    return do_xmap(fdi, ehdr, av);
}


/*************************************************************************
// upx_main - called by our entry code
//
// This function is optimized for size.
**************************************************************************/

void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const junk,
    f_expand *const f_decompress,
    Elf32_Ehdr *const ehdr,  // temp char[MAX_ELF_HDR]
    struct Extent xi,
    struct Extent xo
) __asm__("upx_main");

void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const junk,
    f_expand *const f_decompress,
    Elf32_Ehdr *const ehdr,  // temp char[MAX_ELF_HDR]
    struct Extent xi,
    struct Extent xo
)
{
        // 'fn' and 'efn' must not suffer constant-propagation by gcc
        // UPX2 = offset to name_of_shell
        // UPX3 = strlen(name_of_shell)
    char * /*const*/ volatile  fn = UPX2 + xo.buf;  // past "-c" and "#!"
    char * /*const*/ volatile efn = UPX3 + fn;  // &terminator
    Elf32_Addr entry;

    (void)junk;
    unpackExtent(&xi, &xo, f_decompress);

  {     // Map shell program
    char const c = *efn;  *efn = 0;  // terminator
    entry = getexec(fn, ehdr, av);
    *efn = c;  // replace terminator character

    // av[AT_PHDR -1].a_un.a_val  is set again by do_xmap if PT_PHDR is present.
    av[AT_PHDR   -1].a_type = AT_PHDR  ; // av[AT_PHDR-1].a_un.a_val  is set by do_xmap
    av[AT_PHENT  -1].a_type = AT_PHENT ; av[AT_PHENT  -1].a_un.a_val = ehdr->e_phentsize;
    av[AT_PHNUM  -1].a_type = AT_PHNUM ; av[AT_PHNUM  -1].a_un.a_val = ehdr->e_phnum;
    av[AT_PAGESZ -1].a_type = AT_PAGESZ; av[AT_PAGESZ -1].a_un.a_val = PAGE_SIZE;
    av[AT_ENTRY  -1].a_type = AT_ENTRY ; av[AT_ENTRY  -1].a_un.a_val = entry;
  }

  { // Map PT_INTERP program interpreter
    Elf32_Phdr const *phdr = (Elf32_Phdr *)(1+ehdr);
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        entry = getexec((char const *)phdr->p_vaddr, ehdr, 0);
        break;
    }
  }

    return (void *)entry;
}


/*
vi:ts=4:et:nowrap
*/


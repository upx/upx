/* l_lx_sh.c -- stub loader for Linux x86 shell script executable

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2002 Laszlo Molnar
   Copyright (C) 2000-2002 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#include "linux.hh"


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

static char *
do_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    (void)len; (void)prot; (void)flags; (void)fd; (void)offset;
    return mmap((int *)&addr);
}


/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

typedef int f_expand(
    const nrv_byte *, nrv_uint,
          nrv_byte *, nrv_uint * );

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
            nrv_uint out_len;
            int const j = (*f_decompress)(xi->buf, h.sz_cpr, xo->buf, &out_len);
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

// This do_xmap() has no Extent *xi input because it doesn't decompress anything;
// it only maps the shell and its PT_INTERP.  So, it was specialized by hand
// to reduce compiled instruction size.  gdb 2.91.66 does not notice that
// there is only one call to this static function (from getexec(), which
// would specify 0 for xi), so gdb does not propagate the constant parameter.
// Notice there is no make_hatch(), either.

static Elf32_Addr  // entry address
do_xmap(int const fdi, Elf32_Ehdr const *const ehdr, Elf32_auxv_t *const av)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (char const *)ehdr);
    unsigned long base = (ET_DYN==ehdr->e_type) ? 0x40000000 : 0;
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (PT_PHDR==phdr->p_type) {
        av[AT_PHDR -1].a_un.a_val = phdr->p_vaddr;
    }
    else if (PT_LOAD==phdr->p_type) {
        struct Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char  *addr = xo.buf  =                 (char *)phdr->p_vaddr;
        char *haddr =           phdr->p_memsz + (char *)phdr->p_vaddr;
        size_t frag  = (int)addr &~ PAGE_MASK;
        mlen += frag;
        addr -= frag;
        if (ET_DYN==ehdr->e_type) {
            addr  += base;
            haddr += base;
        }
        else { // There is only one brk, the one for the ET_EXEC
            do_brk(haddr+OVERHEAD);  // Also takes care of whole pages of .bss
        }
        // Decompressor can overrun the destination by 3 bytes.
        if (addr != do_mmap(addr, mlen, PROT_READ | PROT_WRITE,
                MAP_FIXED | MAP_PRIVATE,
                fdi, phdr->p_offset - frag) ) {
            err_exit(8);
        }
        if (0==base) {
            base = (unsigned long)addr;
        }
        bzero(addr, frag);  // fragment at lo end
        frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
        bzero(mlen+addr, frag);  // fragment at hi end
        if (phdr->p_memsz != phdr->p_filesz) { // .bss
            if (ET_DYN==ehdr->e_type) { // PT_INTERP whole pages of .bss?
                addr += frag + mlen;
                mlen = haddr - addr;
                if (0 < (int)mlen) { // need more pages, too
                    if (addr != do_mmap(addr, mlen, PROT_READ | PROT_WRITE,
                            MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 ) ) {
                        err_exit(9);
ERR_LAB
                    }
                }
            }
        }
        else { // no .bss
            int prot = 0;
            if (phdr->p_flags & PF_X) { prot |= PROT_EXEC; }
            if (phdr->p_flags & PF_W) { prot |= PROT_WRITE; }
            if (phdr->p_flags & PF_R) { prot |= PROT_READ; }
            if (0!=mprotect(addr, mlen, prot)) {
                err_exit(10);
            }
        }
        if (ET_DYN!=ehdr->e_type) {
            do_brk(haddr);
        }
    }
        if (0!=close(fdi)) {
            err_exit(11);
        }
    if (ET_DYN==ehdr->e_type) {
        return ehdr->e_entry + base;
    }
    else {
        return ehdr->e_entry;
    }
}


Elf32_Addr  // entry address
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


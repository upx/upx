/* l_lx_elf.c -- stub loader for Linux x86 ELF executable

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
   Copyright (C) 2000-2001 John F. Reiser
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

   Markus F.X.J. Oberhumer   Laszlo Molnar           John F. Reiser
   markus@oberhumer.com      ml1050@cdata.tvnet.hu   jreiser@BitWagon.com
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

typedef void f_unfilter(nrv_byte *, nrv_uint);  // 1st param is also addvalue
typedef int f_expand(
    const nrv_byte *, nrv_uint,
          nrv_byte *, nrv_uint * );

static void
unpackExtent(
    struct Extent *const xi,  // input
    struct Extent *const xo,  // output
    f_expand *const f_decompress,
    f_unfilter *f_unf
)
{
    while (xo->size) {
        struct {
            int32_t sz_unc;  // uncompressed
            int32_t sz_cpr;  //   compressed
        } h;
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
        ||  h.sz_unc > (int32_t)xo->size ) {
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
            // Skip Ehdr+Phdrs: separate 1st block, not filtered
            if (f_unf  // have filter
            &&  ((512 < out_len)  // this block is longer than Ehdr+Phdrs
              || (xo->size==(unsigned)h.sz_unc) )  // block is last in Extent
            ) {
                (*f_unf)(xo->buf, out_len);
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

// Create (or find) an escape hatch to use when munmapping ourselves the stub.
// Called by do_xmap to create it, and by assembler code to find it.
void *
make_hatch(Elf32_Phdr const *const phdr)
{
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        unsigned *hatch;
        // The format of the 'if' is
        //  if ( ( (hatch = loc1), test_loc1 )
        //  ||   ( (hatch = loc2), test_loc2 ) ) {
        //      action
        //  }
        // which uses the comma to save bytes when test_locj involves locj
        // and the action is the same when either test succeeds.

        // Try page fragmentation just beyond .text .
        if ( ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  4<=(~PAGE_MASK & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[12..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr)->e_ident[12])),
                (phdr->p_offset==0) ) ) {
            // Omitting 'const' saves repeated literal in gcc.
            unsigned /*const*/ escape = 0xc36180cd;  // "int $0x80; popa; ret"
            // Don't store into read-only page if value is already there.
            if (*hatch != escape) {
                *hatch  = escape;
            }
            return hatch;
        }
    }
    return 0;
}

static void
bzero(char *p, size_t len)
{
    if (len) do {
        *p++= 0;
    } while (--len);
}


static Elf32_Addr  // entry address
do_xmap(int const fdi, Elf32_Ehdr const *const ehdr, struct Extent *const xi,
    Elf32_auxv_t *const a)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (char const *)ehdr);
    unsigned long base = (ET_DYN==ehdr->e_type) ? 0x40000000 : 0;
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (PT_PHDR==phdr->p_type) {
        a->a_un.a_val = phdr->p_vaddr;
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
            // Not needed if compressed a.elf is invoked directly.
            // Needed only if compressed shell script invokes compressed shell.
            do_brk(haddr+OVERHEAD);  // Also takes care of whole pages of .bss
        }
        // Decompressor can overrun the destination by 3 bytes.
        if (addr != do_mmap(addr, mlen + (xi ? 3 : 0), PROT_READ | PROT_WRITE,
                MAP_FIXED | MAP_PRIVATE | (xi ? MAP_ANONYMOUS : 0),
                fdi, phdr->p_offset - frag) ) {
            err_exit(8);
        }
        if (0==base) {
            base = (unsigned long)addr;
        }
        if (xi) {
            unpackExtent(xi, &xo, (f_expand *)fdi,
                ((phdr->p_flags & PF_X) ? (f_unfilter *)(2+ fdi) : 0));
        }
        bzero(addr, frag);  // fragment at lo end
        frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
        bzero(mlen+addr, frag);  // fragment at hi end
        if (xi) {
            make_hatch(phdr);
        }
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
            if (xi) { // cleanup if decompressor overrun crosses page boundary
                mlen += 3;
                addr += mlen;
                mlen &= ~PAGE_MASK;
                if (mlen<=3) { // page fragment was overrun buffer only
                    munmap(addr - mlen, mlen);
                }
            }
        }
        if (ET_DYN!=ehdr->e_type) {
            // Needed only if compressed shell script invokes compressed shell.
            do_brk(haddr);
        }
    }
    if (!xi) {
        if (0!=close(fdi)) {
            err_exit(11);
        }
    }
    if (ET_DYN==ehdr->e_type) {
        return ehdr->e_entry + base;
    }
    else {
        return ehdr->e_entry;
    }
}


/*************************************************************************
// upx_main - called by our entry code
//
// This function is optimized for size.
**************************************************************************/

void *upx_main(
    char *const uncbuf,
    Elf32_Ehdr const *const my_ehdr,
    f_expand *const f_decompress,
    Elf32_auxv_t *const av,
    Elf32_Ehdr *const ehdr
) __asm__("upx_main");

void *upx_main(
    char *const uncbuf,
    Elf32_Ehdr const *const my_ehdr,  // to get compressed size and data
    f_expand *const f_decompress,
    Elf32_auxv_t *const av,
    Elf32_Ehdr *const ehdr  // temp char[MAX_ELF_HDR+OVERHEAD]
)
{
    size_t const lsize = *(unsigned short const *)(0x7c + (char const *)my_ehdr);
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(1+ehdr);
    Elf32_Addr entry;
    struct Extent xo;
    struct Extent xi = { 0, sizeof(struct p_info) + lsize + CONST_CAST(char *, my_ehdr) };

    size_t const sz_elfhdrs = ((size_t *)xi.buf)[0];  // sizeof(Ehdr+Phdrs), uncompressed
    size_t const sz_pckhdrs = ((size_t *)xi.buf)[1];  // sizeof(Ehdr+Phdrs),   compressed

    (void)uncbuf;  // used by l_lx_sh.c
    // Uncompress Ehdr and Phdrs.
    xo.size =                    sz_elfhdrs;   xo.buf = (char *)ehdr;
    xi.size = 2*sizeof(size_t) + sz_pckhdrs;
    unpackExtent(&xi, &xo, f_decompress, 0);

    // Prepare to decompress the Elf headers again, into the first PT_LOAD.
    xi.buf  -= 2*sizeof(size_t) + sz_pckhdrs;
    xi.size = ((Elf32_Phdr const *)(1 + my_ehdr))->p_filesz - lsize;

    // av[0].a_un.a_val  is set again by do_xmap if PT_PHDR is present
    av[0].a_type = AT_PHDR;   av[0].a_un.a_ptr = 1+(Elf32_Ehdr *)phdr->p_vaddr;
    av[1].a_type = AT_PHENT;  av[1].a_un.a_val = ehdr->e_phentsize;
    av[2].a_type = AT_PHNUM;  av[2].a_un.a_val = ehdr->e_phnum;
    av[3].a_type = AT_PAGESZ; av[3].a_un.a_val = PAGE_SIZE;
    av[4].a_type = AT_ENTRY;  av[4].a_un.a_val = ehdr->e_entry;
    av[5].a_type = AT_NULL;
    entry = do_xmap((int)f_decompress, ehdr, &xi, av);

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
        entry = do_xmap(fdi, ehdr, 0, 0);
        break;
    }
  }

    return (void *)entry;
}


/*
vi:ts=4:et:nowrap
*/


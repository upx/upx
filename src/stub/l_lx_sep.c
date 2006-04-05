/* l_lx_sep.c -- separate loader for Linux Elf executable

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
   Copyright (C) 2000-2004 John F. Reiser
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

#define MAX_ELF 512  // Elf32_Ehdr + n*Elf32_Phdr must fit in this


/*************************************************************************
// file util
**************************************************************************/

#undef xread
#undef xwrite

#if 1
//static int xread(int fd, void *buf, int count) __attribute__((__stdcall__));
static int xread(int fd, void *buf, int count)
{
    // note: we can assert(count > 0);
    do {
        int n = read(fd, buf, count);
        if (n == -EINTR)
            continue;
        if (n <= 0)
            break;
        buf += n;               // gcc extension: add to void *
        count -= n;
    } while (count > 0);
    return count;
}
#else
#define xread(fd,buf,count)     ((count) - read(fd,buf,count))
#endif


#if 1
static __inline__ int xwrite(int fd, const void *buf, int count)
{
    // note: we can assert(count > 0);
    do {
        int n = write(fd, buf, count);
        if (n == -EINTR)
            continue;
        if (n <= 0)
            break;
        buf += n;               // gcc extension: add to void *
        count -= n;
    } while (count > 0);
    return count;
}
#else
#define xwrite(fd,buf,count)    ((count) - write(fd,buf,count))
#endif


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

/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

typedef int f_expand(
    const nrv_byte *, nrv_uint,
          nrv_byte *, nrv_uint * );

struct Extent {
    size_t size;  // must be first to match size[0] uncompressed size
    char *buf;
};

static void
unpackExtent(
    struct Extent *const xo,
    int fdi,
    f_expand *const f_decompress
)
{
    while (xo->size) {
        struct {
            int32_t sz_unc;  // uncompressed
            int32_t sz_cpr;  //   compressed
        } h;
        //   Note: if h.sz_unc == h.sz_cpr then the block was not
        //   compressible and is stored in its uncompressed form.
        int j = 0;

        // Read and check block sizes.
        if (xread(fdi, (void *)&h, sizeof(h)) != 0)
            err_exit(1);
        if (h.sz_unc == 0)                       // uncompressed size 0 -> EOF
        {
            if (h.sz_cpr != UPX_MAGIC_LE32)      // h.sz_cpr must be h->magic
                err_exit(2);
            if (xo->size != 0)                 // all bytes must be written
                err_exit(3);
            break;
        }
        if (h.sz_cpr <= 0) {
            err_exit(4);
ERR_LAB
        }
        if (h.sz_cpr > h.sz_unc || h.sz_unc > (int32_t)xo->size) {
            err_exit(5);
        }
        // Now we have:
        //   assert(h.sz_cpr <= h.sz_unc);
        //   assert(h.sz_unc > 0 && h.sz_unc <= blocksize);
        //   assert(h.sz_cpr > 0 && h.sz_cpr <= blocksize);

        j = h.sz_unc - h.sz_cpr;
        if (0 < j) { // Compressed block.
            j += OVERHEAD;
        }
        if (0!=xread(fdi, xo->buf+j, h.sz_cpr)) {
            err_exit(6);
        }

        // Decompress block.
        if (h.sz_cpr < h.sz_unc) {
            // in-place decompression
            nrv_uint out_len;
            j = (*f_decompress)(xo->buf+j, h.sz_cpr, xo->buf, &out_len, h.b_method);
            if (j != 0 || out_len != (nrv_uint)h.sz_unc)
                err_exit(7);
            // j == 0 now
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
do_xmap(int fdi, Elf32_Ehdr const *const ehdr, f_expand *const f_decompress,
        Elf32_auxv_t *const av)
{
    struct Extent x;
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (char const *)ehdr);
    unsigned long base = (ET_DYN==ehdr->e_type) ? 0x40000000 : 0;
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (PT_PHDR==phdr->p_type) {
        av[AT_PHDR -1].a_un.a_val = phdr->p_vaddr;
    }
    else if (PT_LOAD==phdr->p_type) {
        size_t mlen = x.size =                phdr->p_filesz;
        char  *addr = x.buf  =        (char *)phdr->p_vaddr;
        char *haddr = phdr->p_memsz + (char *)phdr->p_vaddr;
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
        if (addr != mmap(addr, mlen + (f_decompress ? 3 : 0), PROT_READ | PROT_WRITE,
                MAP_FIXED | MAP_PRIVATE | (f_decompress ? MAP_ANONYMOUS : 0),
                fdi, phdr->p_offset - frag) ) {
            err_exit(8);
        }
        if (0==base) {
            base = (unsigned long)addr;
        }
        if (f_decompress) {
            unpackExtent(&x, fdi, f_decompress);
        }
        bzero(addr, frag);  // fragment at lo end
        frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
        bzero(mlen+addr, frag);  // fragment at hi end
        if (f_decompress) {
            make_hatch(phdr);
        }
        if (phdr->p_memsz != phdr->p_filesz) { // .bss
            if (ET_DYN==ehdr->e_type) { // PT_INTERP whole pages of .bss?
                addr += frag + mlen;
                mlen = haddr - addr;
                if (0 < (int)mlen) { // need more pages, too
                    if (addr != mmap(addr, mlen, PROT_READ | PROT_WRITE,
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
            if (f_decompress) { // cleanup if decompressor overrun crosses page boundary
                mlen += 3;
                addr += mlen;
                mlen &= ~PAGE_MASK;
                if (mlen<=3) { // page fragment was overrun buffer only
                    munmap(addr - mlen, mlen);
                }
            }
        }
        if (ET_DYN!=ehdr->e_type) {
            do_brk(haddr);
        }
    }
    if (close(fdi) != 0)
        err_exit(11);
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
    char const *argv[],
    f_expand *const f_decompress,
    Elf32_auxv_t *const av,
    Elf32_Ehdr *const ehdr
) __asm__("upx_main");

void *upx_main(
    char const *argv[],
    f_expand *const f_decompress,
    Elf32_auxv_t *const av,
    Elf32_Ehdr *const ehdr  // temp char[MAX_ELF_HDR+OVERHEAD]
)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(1+ehdr);
    int fdi;  // file descriptor
    size_t sz_elfhdrs;  // sizeof(Ehdr and Phdrs), uncompressed
    size_t sz_pckhdrs;  // sizeof(Ehdr and Phdrs), compressed
    Elf32_Addr entry;
    struct Extent xo;
    int j;

    struct p_info header;

    fdi = open(argv[1], O_RDONLY, 0);
#if 0
    // Save some bytes of code - the lseek() below will fail anyway.
    if (fdi < 0)
        err_exit(12);
#endif

#define SCRIPT_MAX 32
    // Seek to start of compressed data.
    if (lseek(fdi, SCRIPT_MAX+sizeof(struct l_info), SEEK_SET) < 0)
        err_exit(13);
    // Read header.
    if (xread(fdi, (void *)&header, sizeof(header)) != 0) {
        err_exit(14);
    }


    //
    // ----- Step 4: decompress blocks -----
    //

    // Get Elf32_Ehdr.  First set xo.size = size[0] = uncompressed size
    if (0!=xread(fdi, (void *)&xo, sizeof(xo))) {
        err_exit(15);
    }
    if (lseek(fdi, -sizeof(xo), SEEK_CUR) < 0) {
        err_exit(16);
ERR_LAB
    }
    sz_elfhdrs = xo.size;
    sz_pckhdrs = (size_t)xo.buf;
    xo.buf = (char *)ehdr;
    unpackExtent(&xo, fdi, f_decompress);

    // Prepare to decompress the Elf headers again, into the first PT_LOAD.
    if (lseek(fdi, -(sizeof(xo) + sz_pckhdrs), SEEK_CUR) < 0) {
        err_exit(17);
    }
    // av[AT_PHDR -1].a_un.a_val  is set again by do_xmap if PT_PHDR is present.
    av[AT_PHDR   -1].a_type = AT_PHDR  ; av[AT_PHDR   -1].a_un.a_ptr = 1+(Elf32_Ehdr *)phdr->p_vaddr;
    av[AT_PHENT  -1].a_type = AT_PHENT ; av[AT_PHENT  -1].a_un.a_val = ehdr->e_phentsize;
    av[AT_PHNUM  -1].a_type = AT_PHNUM ; av[AT_PHNUM  -1].a_un.a_val = ehdr->e_phnum;
    av[AT_PAGESZ -1].a_type = AT_PAGESZ; av[AT_PAGESZ -1].a_un.a_val = PAGE_SIZE;
    av[AT_ENTRY  -1].a_type = AT_ENTRY ; av[AT_ENTRY  -1].a_un.a_val = ehdr->e_entry;
    entry = do_xmap(fdi, ehdr, f_decompress, av);

    // Map PT_INTERP program interpreter
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        char const *const iname = (char const *)phdr->p_vaddr;
        if (0 > (fdi = open(iname, O_RDONLY, 0))) {
            err_exit(18);
        }
        if (0!=xread(fdi, (void *)ehdr, MAX_ELF)) {
            err_exit(19);
        }
        entry = do_xmap(fdi, ehdr, 0, 0);
        break;
    }

    return (void *)entry;
}


/*
vi:ts=4:et:nowrap
*/


/* i386-bsd.elf-main.c -- stub loader for Linux x86 ELF executable

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


#include "include/bsd.h"


/*************************************************************************
// configuration section
**************************************************************************/

// In order to make it much easier to move this code at runtime and execute
// it at an address different from it load address:  there must be no
// static data, and no string constants.

#if 1  /*{*/
#define DPRINTF(a) /* empty: no debug drivel */
#else  /*}{*/
#include "stdarg.h"

static int
unsimal(unsigned x, char *ptr, int n)
{
    if (10<=x) {
        n = unsimal(x/10, ptr, n);
        x %= 10;
    }
    ptr[n] = '0' + x;
    return 1+ n;
}

static int
decimal(int x, char *ptr, int n)
{
    if (x < 0) {
        *ptr++ = '-'; ++n;
        x = -x;
    }
    return unsimal(x, ptr, n);
}

extern char const *STR_hex();

static int
heximal(unsigned x, char *ptr, int n)
{
    if (16<=x) {
        n = heximal(x>>4, ptr, n);
        x &= 0xf;
    }
    ptr[n] = STR_hex()[x];
    return 1+ n;
}


#define DPRINTF(a) dprintf a
extern char const *STR_0x();
extern char const *STR_xread();
extern char const *STR_unpackExtent();
extern char const *STR_make_hatch_arm();
extern char const *STR_auxv_up();
extern char const *STR_xfind_pages();
extern char const *STR_do_xmap();
extern char const *STR_upx_main();

static int
dprintf(char const *fmt, ...)
{
    char c;
    int n= 0;
    char *ptr;
    char buf[20];
    va_list va; va_start(va, fmt);
    ptr= &buf[0];
    while (0!=(c= *fmt++)) if ('%'!=c) goto literal;
    else switch (c= *fmt++) {
    default: {
literal:
        n+= write(2, fmt-1, 1);
    } break;
    case 0: goto done;  /* early */
    case 'u': {
        n+= write(2, buf, unsimal(va_arg(va, unsigned), buf, 0));
    } break;
    case 'd': {
        n+= write(2, buf, decimal(va_arg(va, int), buf, 0));
    } break;
    case 'p':  /* same as 'x'; relies on sizeof(int)==sizeof(void *) */
    case 'x': {
        buf[0] = '0';
        buf[1] = 'x';
        n+= write(2, buf, heximal(va_arg(va, int), buf, 2));
    } break;
    }
done:
    va_end(va);
    return n;
}
#endif  /*}*/

#define MAX_ELF_HDR 512  // Elf32_Ehdr + n*Elf32_Phdr must fit in this


/*************************************************************************
// "file" util
**************************************************************************/

struct Extent {
    size_t size;  // must be first to match size[0] uncompressed size
    char *buf;
};


static void
#if (ACC_CC_GNUC >= 0x030300) && defined(__i386__)  /*{*/
__attribute__((__noinline__, __used__, regparm(3), stdcall))
#endif  /*}*/
xread(struct Extent *x, char *buf, size_t count)
{
    char *p=x->buf, *q=buf;
    size_t j;
    DPRINTF((STR_xread(), x, x->size, x->buf, buf, count));
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
err_exit(int a) __attribute__ ((__noreturn__));
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
    struct Extent *const xi,  // input
    struct Extent *const xo,  // output
    f_expand *const f_decompress,
    f_unfilter *f_unf
)
{
    DPRINTF((STR_unpackExtent(),
        xi, xi->size, xi->buf, xo, xo->size, xo->buf, f_decompress, f_unf));
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
            // Skip Ehdr+Phdrs: separate 1st block, not filtered
            if (h.b_ftid!=0 && f_unf  // have filter
            &&  ((512 < out_len)  // this block is longer than Ehdr+Phdrs
              || (xo->size==(unsigned)h.sz_unc) )  // block is last in Extent
            ) {
                (*f_unf)((unsigned char *)xo->buf, out_len, h.b_cto8, h.b_ftid);
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

#if defined(__i386__)  /*{*/
// Create (or find) an escape hatch to use when munmapping ourselves the stub.
// Called by do_xmap to create it; remembered in AT_NULL.d_val
static void *
make_hatch_x86(Elf32_Phdr const *const phdr, unsigned const reloc)
{
    unsigned *hatch = 0;
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        // The format of the 'if' is
        //  if ( ( (hatch = loc1), test_loc1 )
        //  ||   ( (hatch = loc2), test_loc2 ) ) {
        //      action
        //  }
        // which uses the comma to save bytes when test_locj involves locj
        // and the action is the same when either test succeeds.

        // Try page fragmentation just beyond .text .
        if ( ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  4<=(~PAGE_MASK & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[12..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr + reloc)->e_ident[12])),
                (phdr->p_offset==0) ) ) {
            // Omitting 'const' saves repeated literal in gcc.
            unsigned /*const*/ escape = 0xc3c980cd;  // "int $0x80; leave; ret"
            // Don't store into read-only page if value is already there.
            if (* (volatile unsigned*) hatch != escape) {
                * hatch  = escape;
            }
        }
    }
    return hatch;
}
#elif defined(__arm__)  /*}{*/
static void *
make_hatch_arm(Elf32_Phdr const *const phdr, unsigned const reloc)
{
    unsigned *hatch = 0;
    DPRINTF((STR_make_hatch_arm(),phdr,reloc));
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        // The format of the 'if' is
        //  if ( ( (hatch = loc1), test_loc1 )
        //  ||   ( (hatch = loc2), test_loc2 ) ) {
        //      action
        //  }
        // which uses the comma to save bytes when test_locj involves locj
        // and the action is the same when either test succeeds.

        // Try page fragmentation just beyond .text .
        if ( ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  8<=(~PAGE_MASK & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[8..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr + reloc)->e_ident[8])),
                (phdr->p_offset==0) ) )
        {
            hatch[0]= 0xef90005b;  // syscall __NR_unmap
            hatch[1]= 0xe1a0f00e;  // mov pc,lr
        }
    }
    return hatch;
}
#endif  /*}*/

static void
#if defined(__i386__)  /*{*/
__attribute__((regparm(2), stdcall))
#endif  /*}*/
upx_bzero(char *p, size_t len)
{
    if (len) do {
        *p++= 0;
    } while (--len);
}
#define bzero upx_bzero


static void
#if defined(__i386__)  /*{*/
__attribute__((regparm(3), stdcall))
#endif  /*}*/
auxv_up(Elf32_auxv_t *av, unsigned const type, unsigned const value)
{
    DPRINTF((STR_auxv_up(),av,type,value));
    if (av
#if defined(__i386__)  /*{*/
    && 0==(1&(int)av)  /* PT_INTERP usually inhibits, except for hatch */
#endif  /*}*/
    )
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
#if defined(__i386__)  /*{*/
__attribute__((regparm(3), stdcall))
#endif  /*}*/
xfind_pages(unsigned mflags, Elf32_Phdr const *phdr, int phnum,
    char **const p_brk
)
{
    size_t lo= ~0, hi= 0, szlo= 0;
    char *addr;
    DPRINTF((STR_xfind_pages(), mflags, phdr, phnum, p_brk));
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
    szlo += ~PAGE_MASK & lo;  // page fragment on lo edge
    lo   -= ~PAGE_MASK & lo;  // round down to page boundary
    hi    =  PAGE_MASK & (hi - lo - PAGE_MASK -1);  // page length
    szlo  =  PAGE_MASK & (szlo    - PAGE_MASK -1);  // page length
    addr = mmap((void *)lo, hi, PROT_NONE, mflags, -1, 0);
    *p_brk = hi + addr;  // the logical value of brk(0)
    //mprotect(szlo + addr, hi - szlo, PROT_NONE);  // no access, but keep the frames!
    return (unsigned long)addr - lo;
}

static Elf32_Addr  // entry address
do_xmap(int const fdi, Elf32_Ehdr const *const ehdr, struct Extent *const xi,
    Elf32_auxv_t *const av, unsigned *p_reloc, f_unfilter *const f_unf)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (void const *)ehdr);
    char *v_brk;
    unsigned const reloc = xfind_pages(
        ((ET_EXEC==ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum, &v_brk);
    int j;
    DPRINTF((STR_do_xmap(),
        fdi, ehdr, xi, (xi? xi->size: 0), (xi? xi->buf: 0), av, p_reloc, f_unf));
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (PT_PHDR==phdr->p_type) {
        if (xi) {
            auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
        }
    }
    else if (PT_LOAD==phdr->p_type) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        struct Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char  *addr = xo.buf  =  (char *)(phdr->p_vaddr + reloc);
        char *haddr =           phdr->p_memsz + addr;
        size_t frag  = (int)addr &~ PAGE_MASK;
        mlen += frag;
        addr -= frag;

        if (addr != mmap(addr, mlen
#if defined(__i386__)  /*{*/
            // Decompressor can overrun the destination by 3 bytes.
            + (xi ? 3 : 0)
#endif  /*}*/
                , prot | (xi ? PROT_WRITE : 0),
                MAP_FIXED | MAP_PRIVATE | (xi ? MAP_ANONYMOUS : 0),
                (xi ? -1 : fdi), phdr->p_offset - frag) ) {
            err_exit(8);
        }
        if (xi) {
            unpackExtent(xi, &xo, (f_expand *)fdi,
                ((PROT_EXEC & prot) ? f_unf : 0) );
        }
        // Linux does not fixup the low end, so neither do we.
        //if (PROT_WRITE & prot) {
        //    bzero(addr, frag);  // fragment at lo end
        //}
        frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
        if (PROT_WRITE & prot) { // note: read-only .bss not supported here
            bzero(mlen+addr, frag);  // fragment at hi end
        }
        if (xi) {
#if defined(__i386__)  /*{*/
            void *const hatch = make_hatch_x86(phdr, reloc);
            if (0!=hatch) {
                /* always update AT_NULL, especially for compressed PT_INTERP */
                auxv_up((Elf32_auxv_t *)(~1 & (int)av), AT_NULL, (unsigned)hatch);
            }
#elif defined(__arm__)  /*}{*/
            void *const hatch = make_hatch_arm(phdr, reloc);
            if (0!=hatch) {
                auxv_up((Elf32_auxv_t *)(void *)av, AT_NULL, (unsigned)hatch);
            }
#endif  /*}*/
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
#if defined(__i386__)  /*{*/
        else if (xi) { // cleanup if decompressor overrun crosses page boundary
            mlen = ~PAGE_MASK & (3+ mlen);
            if (mlen<=3) { // page fragment was overrun buffer only
                munmap(addr, mlen);
            }
        }
#endif  /*}*/
    }
    if (!xi) { // 2nd call (PT_INTERP); close()+check is smaller here
        if (0!=close(fdi)) {
            err_exit(11);
        }
    }
    else { // 1st call (main); also have (0!=av) here
        if (ET_DYN!=ehdr->e_type) {
            // Needed only if compressed shell script invokes compressed shell.
            do_brk(v_brk);
        }
    }
    if (0!=p_reloc) {
        *p_reloc = reloc;
    }
    return ehdr->e_entry + reloc;
}


/*************************************************************************
// upx_main - called by our entry code
//
// This function is optimized for size.
**************************************************************************/

void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const sz_compressed,
    f_expand *const f_decompress,
    f_unfilter */*const*/ f_unfilter,
    struct Extent xo,
    struct Extent xi,
    unsigned const volatile dynbase
) __asm__("upx_main");

void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const sz_compressed,
    f_expand *const f_decompress,
    f_unfilter */*const*/ f_unf,
    struct Extent xo,  // {sz_unc, ehdr}    for ELF headers
    struct Extent xi,  // {sz_cpr, &b_info} for ELF headers
    unsigned const volatile dynbase  // value+result: compiler must not change
)
{
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)(void *)xo.buf;  // temp char[MAX_ELF_HDR+OVERHEAD]
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(1+ ehdr);
    Elf32_Addr reloc;
    Elf32_Addr entry;

    // sizeof(Ehdr+Phdrs),   compressed; including b_info header
    size_t const sz_pckhdrs = xi.size;

    DPRINTF((STR_upx_main(),
        av, sz_compressed, f_decompress, f_unf, &xo, xo.size, xo.buf,
        &xi, xi.size, xi.buf, dynbase));
#if defined(__i386__)  /*{*/
    f_unf = (f_unfilter *)(2+ (long)f_decompress);
#endif  /*}*/

    // Uncompress Ehdr and Phdrs.
    unpackExtent(&xi, &xo, f_decompress, 0);

    // Prepare to decompress the Elf headers again, into the first PT_LOAD.
    xi.buf  -= sz_pckhdrs;
    xi.size  = sz_compressed;

    // Some kernels omit AT_PHNUM,AT_PHENT,AT_PHDR because this stub has no PT_INTERP.
    // That is "too much" optimization.  Linux 2.6.x seems to give all AT_*.
    auxv_up(av, AT_PAGESZ, PAGE_SIZE);
    auxv_up(av, AT_PHNUM , ehdr->e_phnum);
    auxv_up(av, AT_PHENT , ehdr->e_phentsize);
    auxv_up(av, AT_PHDR  , dynbase + (unsigned)(1+(Elf32_Ehdr *)phdr->p_vaddr));
    // AT_PHDR.a_un.a_val  is set again by do_xmap if PT_PHDR is present.
    // This is necessary for ET_DYN if|when we override a prelink address.

    entry = do_xmap((int)f_decompress, ehdr, &xi, av, &reloc, f_unf);
    auxv_up(av, AT_ENTRY , entry);  // might not be necessary?

  { // Map PT_INTERP program interpreter
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        int const fdi = open(reloc + (char const *)phdr->p_vaddr, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
        if (MAX_ELF_HDR!=read(fdi, (void *)ehdr, MAX_ELF_HDR)) {
ERR_LAB
            err_exit(19);
        }
        entry = do_xmap(fdi, ehdr, 0, 0, &reloc, 0);
        auxv_up(av, AT_BASE, reloc);
        break;
    }
  }

    return (void *)entry;
}


/*
vi:ts=4:et:nowrap
*/


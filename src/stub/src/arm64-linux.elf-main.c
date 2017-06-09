/* arm64-linux.elf-main.c -- stub loader for Linux 64-bit ELF executable

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2017 Laszlo Molnar
   Copyright (C) 2000-2017 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#include "include/linux.h"

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}

#if !DEBUG //{
#define DPRINTF(fmt, args...) /*empty*/
#else  //}{
// DPRINTF is defined as an expression using "({ ... })"
// so that DPRINTF can be invoked inside an expression,
// and then followed by a comma to ignore the return value.
// The only complication is that percent and backslash
// must be doubled in the format string, because the format
// string is processd twice: once at compile-time by 'asm'
// to produce the assembled value, and once at runtime to use it.
#if defined(__powerpc__)  //{
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("bl 0f; .string \"" fmt "\"; .balign 4; 0: mflr %0" \
/*out*/ : "=r"(r_fmt) \
/* in*/ : \
/*und*/ : "lr"); \
    dprintf(r_fmt, args); \
})
#elif defined(__x86_64) //}{
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("call 0f; .asciz \"" fmt "\"; 0: pop %0" \
/*out*/ : "=r"(r_fmt) ); \
    dprintf(r_fmt, args); \
})
#elif defined(__AARCH64EL__) //}{
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("bl 0f; .string \"" fmt "\"; .balign 4; 0: mov %0,x30" \
/*out*/ : "=r"(r_fmt) \
/* in*/ : \
/*und*/ : "x30"); \
    dprintf(r_fmt, args); \
})
#endif  //}

static int dprintf(char const *fmt, ...); // forward
#endif  /*}*/

/*************************************************************************
// configuration section
**************************************************************************/

// In order to make it much easier to move this code at runtime and execute
// it at an address different from it load address:  there must be no
// static data, and no string constants.

#define MAX_ELF_HDR 1024  // Elf64_Ehdr + n*Elf64_Phdr must fit in this


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
#define ERR_LAB /*empty*/
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
            int const j = (*f_decompress)((unsigned char *)xi->buf, h.sz_cpr,
                (unsigned char *)xo->buf, &out_len, h.b_method );
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

//DEBUG_STRCON(STR_make_hatch, "make_hatch %%p %%x\\n");

static void *
make_hatch_arm64(
    Elf64_Phdr const *const phdr,
    uint64_t const reloc
)
{
    unsigned *hatch = 0;
    //DPRINTF((STR_make_hatch(),phdr,reloc));
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        // The format of the 'if' is
        //  if ( ( (hatch = loc1), test_loc1 )
        //  ||   ( (hatch = loc2), test_loc2 ) ) {
        //      action
        //  }
        // which uses the comma to save bytes when test_locj involves locj
        // and the action is the same when either test succeeds.

        // Try page fragmentation just beyond .text .
        if ( ( (hatch = (void *)(~3ul & (3+ phdr->p_memsz + phdr->p_vaddr + reloc))),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (2*4)<=(~PAGE_MASK & -(uint64_t)hatch) ) ) // space left on page
        ) {
            hatch[0]= 0xd4000001;  // svc #0
            hatch[1]= 0xd65f03c0;  // ret (jmp *lr)
        }
        else {
            hatch = 0;
        }
    }
    return hatch;
}

#if 1  /*{*/
static void
upx_bzero(char *p, size_t len)
{
    if (len) do {
        *p++= 0;
    } while (--len);
}
#define bzero upx_bzero
#else
#define bzero(a,b)  __builtin_memset(a,0,b)
#endif  /*}*/

static void
auxv_up(Elf64_auxv_t *av, unsigned const type, uint64_t const value)
{
    if (!av)
        return;
    DPRINTF("\\nauxv_up %%d  %%p\\n", type, value);
    for (;; ++av) {
        DPRINTF("  %%d  %%p\\n", av->a_type, av->a_un.a_val);
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
xfind_pages(unsigned mflags, Elf64_Phdr const *phdr, int phnum,
    char **const p_brk
)
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
    szlo += ~PAGE_MASK & lo;  // page fragment on lo edge
    lo   -= ~PAGE_MASK & lo;  // round down to page boundary
    hi    =  PAGE_MASK & (hi - lo - PAGE_MASK -1);  // page length
    szlo  =  PAGE_MASK & (szlo    - PAGE_MASK -1);  // page length
    addr = mmap((void *)lo, hi, PROT_NONE, mflags, -1, 0);
    *p_brk = hi + addr;  // the logical value of brk(0)
    //mprotect(szlo + addr, hi - szlo, PROT_NONE);  // no access, but keep the frames!
    return (unsigned long)addr - lo;
}

static Elf64_Addr  // entry address
do_xmap(
    Elf64_Ehdr const *const ehdr,
    Extent *const xi,
    int const fdi,
    Elf64_auxv_t *const av,
    f_expand *const f_decompress,
    f_unfilter *const f_unf,
    Elf64_Addr *p_reloc
)
{
    Elf64_Phdr const *phdr = (Elf64_Phdr const *) (ehdr->e_phoff +
        (char const *)ehdr);
    char *v_brk;
    unsigned long const reloc = xfind_pages(
        ((ET_DYN!=ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum, &v_brk);
    DPRINTF("do_xmap reloc=%%p", reloc);
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (xi && PT_PHDR==phdr->p_type) {
        auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
    } else
    if (PT_LOAD==phdr->p_type) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char  *addr = xo.buf  =         reloc + (char *)phdr->p_vaddr;
        char *haddr =           phdr->p_memsz +                  addr;
        size_t frag  = (long)addr &~ PAGE_MASK;
        mlen += frag;
        addr -= frag;

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
        if (PROT_WRITE & prot) { // note: read-only .bss not supported here
            bzero(mlen+addr, frag);  // fragment at hi end
        }
        if (xi) {
            void *const hatch = make_hatch_arm64(phdr, reloc);
            if (0!=hatch) {
                auxv_up(av, AT_NULL, (uint64_t)hatch);
            }
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
    if (xi) { // 1st call (main); also have (0!=av) here
        if (ET_DYN!=ehdr->e_type) {
            // Needed only if compressed shell script invokes compressed shell.
            // brk(v_brk);  // SIGSEGV when is_big [unmaps ourself!]
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

void *
upx_main(  // returns entry address
/*x0*/    struct b_info const *const bi,  // 1st block header
/*x1*/    size_t const sz_compressed,  // total length
/*x2*/    Elf64_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
/*x3*/    Elf64_auxv_t *const av,
/*x4*/    f_expand *const f_decompress,
/*x5*/    f_unfilter *const f_unf,
/*x6*/    Elf64_Addr reloc
)
{
    Extent xo, xi1, xi2;
    xo.buf  = (char *)ehdr;
    xo.size = bi->sz_unc;
    xi2.buf = CONST_CAST(char *, bi); xi2.size = sz_compressed;
    xi1.buf = CONST_CAST(char *, bi); xi1.size = sz_compressed;

    // ehdr = Uncompress Ehdr and Phdrs
    unpackExtent(&xi2, &xo, f_decompress, 0);  // never filtered?

    // AT_PHDR.a_un.a_val  is set again by do_xmap if PT_PHDR is present.
    auxv_up(av, AT_PHDR , reloc + ehdr->e_phoff);
    auxv_up(av, AT_PHNUM,         ehdr->e_phnum);
    //auxv_up(av, AT_PHENT , ehdr->e_phentsize);  /* this can never change */
    //auxv_up(av, AT_PAGESZ, PAGE_SIZE);  /* ld-linux.so.2 does not need this */

    DPRINTF("upx_main1  .e_entry=%%p  reloc=%%p", ehdr->e_entry, reloc);
    Elf64_Phdr *phdr = (Elf64_Phdr *)(1+ ehdr);
    unsigned const orig_e_type = ehdr->e_type;
    if (ET_DYN==orig_e_type /*&& phdr->p_vaddr==0*/) { // -pie /*FIXME: and not pre-linked*/
        // Unpacked must start at same place as packed, so that brk(0) works.
        ehdr->e_type = ET_EXEC;
        auxv_up(av, AT_ENTRY, ehdr->e_entry += reloc);
        unsigned j;
        for (j=0; j < ehdr->e_phnum; ++phdr, ++j) {
            phdr->p_vaddr += reloc;
            phdr->p_paddr += reloc;
        }
    }

    // De-compress Ehdr again into actual position, then de-compress the rest.
    Elf64_Addr entry = do_xmap(ehdr, &xi1, 0, av, f_decompress, f_unf, &reloc);
    DPRINTF("upx_main2  entry=%%p  reloc=%%p", entry, reloc);
    if (ET_DYN!=orig_e_type) {
        auxv_up(av, AT_ENTRY , entry);
    }

  { // Map PT_INTERP program interpreter
    phdr = (Elf64_Phdr *)(1+ ehdr);
    unsigned j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        char const *const iname = reloc + (char const *)phdr->p_vaddr;
        int const fdi = open(iname, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
        if (MAX_ELF_HDR!=read(fdi, (void *)ehdr, MAX_ELF_HDR)) {
ERR_LAB
            err_exit(19);
        }
        {
            Elf64_Addr i_reloc = 0;
            entry = do_xmap(ehdr, 0, fdi, 0, 0, 0, &i_reloc);
            auxv_up(av, AT_BASE, i_reloc);  // musl
        }
        close(fdi);
    }
  }

    return (void *)entry;
}

#if DEBUG  //{

static int
unsimal(unsigned x, char *ptr, int n)
{
    unsigned m = 10;
    while (10 <= (x / m)) m *= 10;
    while (10 <= x) {
        unsigned d = x / m;
    x -= m * d;
        m /= 10;
        ptr[n++] = '0' + d;
    }
    ptr[n++] = '0' + x;
    return n;
}

static int
decimal(int x, char *ptr, int n)
{
    if (x < 0) {
        x = -x;
        ptr[n++] = '-';
    }
    return unsimal(x, ptr, n);
}

static int
heximal(unsigned long x, char *ptr, int n)
{
    unsigned j = -1+ 2*sizeof(unsigned long);
    unsigned long m = 0xful << (4 * j);
    for (; j; --j, m >>= 4) { // omit leading 0 digits
        if (m & x) break;
    }
    for (; m; --j, m >>= 4) {
        unsigned d = 0xf & (x >> (4 * j));
        ptr[n++] = ((10<=d) ? ('a' - 10) : '0') + d;
    }
    return n;
}

#define va_arg      __builtin_va_arg
#define va_end      __builtin_va_end
#define va_list     __builtin_va_list
#define va_start    __builtin_va_start

static int
dprintf(char const *fmt, ...)
{
    int n= 0;
    char const *literal = 0;  // NULL
    char buf[24];  // ~0ull == 18446744073709551615 ==> 20 chars
    va_list va; va_start(va, fmt);
    for (;;) {
        char c = *fmt++;
        if (!c) { // end of fmt
            if (literal) {
                goto finish;
            }
            break;  // goto done
        }
        if ('%'!=c) {
            if (!literal) {
                literal = fmt;  // 1 beyond start of literal
            }
            continue;
        }
        // '%' == c
        if (literal) {
finish:
            n += write(2, -1+ literal, fmt - literal);
            literal = 0;  // NULL
            if (!c) { // fmt already ended
               break;  // goto done
            }
        }
        switch (c= *fmt++) { // deficiency: does not handle _long_
        default: { // un-implemented conversion
            n+= write(2, -1+ fmt, 1);
        } break;
        case 0: { // fmt ends with "%\0" ==> ignore
            goto done;
        } break;
        case 'u': {
            n+= write(2, buf, unsimal(va_arg(va, unsigned), buf, 0));
        } break;
        case 'd': {
            n+= write(2, buf, decimal(va_arg(va, int), buf, 0));
        } break;
        case 'p': {
            buf[0] = '0';
            buf[1] = 'x';
            n+= write(2, buf, heximal((unsigned long)va_arg(va, void *), buf, 2));
        } break;
        case 'x': {
            buf[0] = '0';
            buf[1] = 'x';
            n+= write(2, buf, heximal(va_arg(va, int), buf, 2));
        } break;
        } // 'switch'
    }
done:
    va_end(va);
    return n;
 }
#endif  //}
/* vim:set ts=4 sw=4 et: */

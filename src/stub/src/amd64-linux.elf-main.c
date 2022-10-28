/* amd64-linux.elf-main.c -- stub loader for Linux 64-bit ELF executable

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
   Copyright (C) 2000-2022 John F. Reiser
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
#elif defined(__aarch64__) //}{
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
    DPRINTF("xread x.size=%%x  x.buf=%%p  buf=%%p  count=%%x\\n",
        x->size, x->buf, buf, count);
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
    DPRINTF("xread done\\n",0);
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
    DPRINTF("err_exit %%d\\n", a);
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
          nrv_byte *, size_t *, unsigned );

static void
unpackExtent(
    Extent *const xi,  // input
    Extent *const xo,  // output
    f_expand *const f_exp,
    f_unfilter *f_unf
)
{
    while (xo->size) {
        DPRINTF("unpackExtent xi=(%%p %%p)  xo=(%%p %%p)  f_exp=%%p  f_unf=%%p\\n",
            xi->size, xi->buf, xo->size, xo->buf, f_exp, f_unf);
        struct b_info h;
        //   Note: if h.sz_unc == h.sz_cpr then the block was not
        //   compressible and is stored in its uncompressed form.

        // Read and check block sizes.
        xread(xi, (char *)&h, sizeof(h));
        DPRINTF("h.sz_unc=%%x  h.sz_cpr=%%x  h.b_method=%%x\\n",
            h.sz_unc, h.sz_cpr, h.b_method);
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
            size_t out_len = h.sz_unc;  // EOF for lzma
            int const j = (*f_exp)((unsigned char *)xi->buf, h.sz_cpr,
                (unsigned char *)xo->buf, &out_len,
#if defined(__x86_64)  //{
                    *(int *)(void *)&h.b_method
#elif defined(__powerpc64__) || defined(__aarch64__) //}{
                    h.b_method
#endif  //}
                );
            if (j != 0 || out_len != (nrv_uint)h.sz_unc) {
                DPRINTF("j=%%x  out_len=%%x  &h=%%p\\n", j, out_len, &h);
                err_exit(7);
            }
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

#if defined(__x86_64__)  //{
static void *
make_hatch_x86_64(
    Elf64_Phdr const *const phdr,
    Elf64_Addr reloc,
    unsigned const frag_mask
)
{
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x  %%x\\n",phdr,reloc,frag_mask, phdr->p_flags);
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        if (
        // Try page fragmentation just beyond .text .
             ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (1*4)<=(frag_mask & -(int)(size_t)hatch) ) ) // space left on page
        // Try Elf64_Ehdr.e_ident[12..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf64_Ehdr *)(phdr->p_vaddr + reloc))->e_ident[12])),
                (phdr->p_offset==0) )
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, PAGE_SIZE, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        )
        {
            hatch[0] = 0xc35a050f;  // syscall; pop %rdx; ret
            if (xprot) {
                mprotect(hatch, 1*sizeof(unsigned), PROT_EXEC|PROT_READ);
            }
        }
        else {
            hatch = 0;
        }
    }
    DPRINTF("hatch=%%p\n", hatch);
    return hatch;
}
#elif defined(__powerpc64__)  //}{
static unsigned
ORRX(unsigned ra, unsigned rs, unsigned rb) // or ra,rs,rb
{
    return (31<<26) | ((037&(rs))<<21) | ((037&(ra))<<16) | ((037&(rb))<<11) | (444<<1) | 0;
}

static void *
make_hatch_ppc64(
    Elf64_Phdr const *const phdr,
    Elf64_Addr reloc,
    unsigned const frag_mask
)
{
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,frag_mask);
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        if (
        // Try page fragmentation just beyond .text .
            ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (4*4)<=(frag_mask & -(int)(size_t)hatch) ) ) // space left on page
        // Try Elf64_Phdr[1].p_paddr (2 instr) and .p_filesz (2 instr)
        ||   ( (hatch = (void *)(&((Elf64_Phdr *)(1+  // Ehdr and Phdr are contiguous
                ((Elf64_Ehdr *)(phdr->p_vaddr + reloc))))[1].p_paddr)),
                (phdr->p_offset==0) )
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, 1<<12, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        )
        {
            hatch[0]= 0x44000002;  // sc
            hatch[1]= ORRX(12,31,31);  // movr r12,r31 ==> or r12,r31,r31
            hatch[2]= 0x38800000;  // li r4,0
            hatch[3]= 0x4e800020;  // blr
            if (xprot) {
                mprotect(hatch, 3*sizeof(unsigned), PROT_EXEC|PROT_READ);
            }
        }
        else {
            hatch = 0;
        }
    }
    return hatch;
}
#elif defined(__aarch64__)  //{
static void *
make_hatch_arm64(
    Elf64_Phdr const *const phdr,
    uint64_t const reloc,
    unsigned const frag_mask
)
{
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,frag_mask);
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        // The format of the 'if' is
        //  if ( ( (hatch = loc1), test_loc1 )
        //  ||   ( (hatch = loc2), test_loc2 ) ) {
        //      action
        //  }
        // which uses the comma to save bytes when test_locj involves locj
        // and the action is the same when either test succeeds.

        if (
        // Try page fragmentation just beyond .text .
             ( (hatch = (void *)(~3ul & (3+ phdr->p_memsz + phdr->p_vaddr + reloc))),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (2*4)<=(frag_mask & -(int)(uint64_t)hatch) ) ) // space left on page
        // Try Elf64_Ehdr.e_ident[8..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf64_Ehdr *)(phdr->p_vaddr + reloc))->e_ident[8])),
                (phdr->p_offset==0) )
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, 1<<12, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        )
        {
            hatch[0] = 0xd4000001;  // svc #0
            hatch[1] = 0xd65f03c0;  // ret (jmp *lr)
            if (xprot) {
                mprotect(hatch, 2*sizeof(unsigned), PROT_EXEC|PROT_READ);
            }
        }
        else {
            hatch = 0;
        }
    }
    DPRINTF("hatch=%%p\n", hatch);
    return hatch;
}
#endif  //}

#if defined(__powerpc64__) || defined(__aarch64__)  //{ bzero
static void
upx_bzero(char *p, size_t len)
{
    DPRINTF("bzero %%x  %%x\\n", p, len);
    if (len) do {
        *p++= 0;
    } while (--len);
}
#define bzero upx_bzero
#else  //}{
#define bzero(a,b)  __builtin_memset(a,0,b)
#endif  //}

static void
auxv_up(Elf64_auxv_t *av, unsigned const type, uint64_t const value)
{
    if (!av || (1& (size_t)av)) { // none, or inhibited for PT_INTERP
        return;
    }
    DPRINTF("\\nauxv_up %%d  %%p\\n", type, value);
    for (;; ++av) {
        DPRINTF("  %%d  %%p\\n", av->a_type, av->a_un.a_val);
        if (av->a_type==type || (av->a_type==AT_IGNORE && type!=AT_NULL)) {
            av->a_type = type;
            av->a_un.a_val = value;
            return;
        }
        if (av->a_type==AT_NULL) {
            // We can't do this as part of the for loop because we overwrite
            // AT_NULL too.
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
static Elf64_Addr // returns relocation constant
xfind_pages(unsigned mflags, Elf64_Phdr const *phdr, int phnum,
    Elf64_Addr *const p_brk
    , Elf64_Addr const elfaddr
#if defined(__powerpc64__) || defined(__aarch64__)
    , size_t const PAGE_MASK
#endif
)
{
    Elf64_Addr lo= ~0, hi= 0, addr= 0;
    mflags += MAP_PRIVATE | MAP_ANONYMOUS;  // '+' can optimize better than '|'
    DPRINTF("xfind_pages  %%x  %%p  %%d  %%p  %%p\\n", mflags, phdr, phnum, elfaddr, p_brk);
    for (; --phnum>=0; ++phdr) if (PT_LOAD==phdr->p_type) {
        DPRINTF(" p_vaddr=%%p  p_memsz=%%p\\n", phdr->p_vaddr, phdr->p_memsz);
        if (phdr->p_vaddr < lo) {
            lo = phdr->p_vaddr;
        }
        if (hi < (phdr->p_memsz + phdr->p_vaddr)) {
            hi =  phdr->p_memsz + phdr->p_vaddr;
        }
    }
    lo -= ~PAGE_MASK & lo;  // round down to page boundary
    hi  =  PAGE_MASK & (hi - lo - PAGE_MASK -1);  // page length
    if (MAP_FIXED & mflags) {
        addr = lo;
    }
    else if (0==lo) { // -pie ET_DYN
        addr = elfaddr;
        if (addr) {
            mflags |= MAP_FIXED;
        }
    }
    DPRINTF("  addr=%%p  lo=%%p  hi=%%p\\n", addr, lo, hi);
    // PROT_WRITE allows testing of 64k pages on 4k Linux
    addr = (Elf64_Addr)mmap((void *)addr, hi, (DEBUG ? PROT_WRITE : PROT_NONE),  // FIXME XXX EVIL
        mflags, -1, 0);
    DPRINTF("  addr=%%p\\n", addr);
    *p_brk = hi + addr;  // the logical value of brk(0)
    return (Elf64_Addr)(addr - lo);
}

static Elf64_Addr  // entry address
do_xmap(
    Elf64_Ehdr const *const ehdr,
    Extent *const xi,
    int const fdi,
    Elf64_auxv_t *const av,
    f_expand *const f_exp,
    f_unfilter *const f_unf,
    Elf64_Addr *p_reloc
#if defined(__powerpc64__) || defined(__aarch64__)
    , size_t const PAGE_MASK
#endif
)
{
    Elf64_Phdr const *phdr = (Elf64_Phdr const *)(void const *)(ehdr->e_phoff +
        (char const *)ehdr);
    Elf64_Addr v_brk;
    Elf64_Addr reloc;
    if (xi) { // compressed main program:
        // C_BASE space reservation, C_TEXT compressed data and stub
        Elf64_Addr ehdr0 = *p_reloc;  // the 'hi' copy!
        Elf64_Phdr const *phdr0 = (Elf64_Phdr const *)(
            ((Elf64_Ehdr *)ehdr0)->e_phoff + ehdr0);
        // Clear the 'lo' space reservation for use by PT_LOADs
        ehdr0 -= phdr0[1].p_vaddr;  // the 'lo' copy
        if (ET_EXEC==ehdr->e_type) {
            ehdr0 = phdr0[0].p_vaddr;
        }
        v_brk = phdr0->p_memsz + ehdr0;
        reloc = (Elf64_Addr)mmap((void *)ehdr0, phdr0->p_memsz, PROT_NONE,
            MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
        if (ET_EXEC==ehdr->e_type) {
            reloc = 0;
        }
    }
    else { // PT_INTERP
        DPRINTF("INTERP\\n", 0);
        reloc = xfind_pages(
            ((ET_DYN!=ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum, &v_brk, *p_reloc
#if defined(__powerpc64__) || defined(__aarch64__)
            , PAGE_MASK
#endif
        );
    }
    DPRINTF("do_xmap reloc=%%p\\n", reloc);
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (xi && PT_PHDR==phdr->p_type) {
        auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
    } else
    if (PT_LOAD==phdr->p_type) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        DPRINTF("LOAD p_offset=%%p  p_vaddr=%%p  p_filesz=%%p  p_memsz=%%p  p_flags=%%x  prot=%%x\\n",
            phdr->p_offset, phdr->p_vaddr, phdr->p_filesz, phdr->p_memsz, phdr->p_flags, prot);
        if (xi && !phdr->p_offset /*&& ET_EXEC==ehdr->e_type*/) { // 1st PT_LOAD
            // ? Compressed PT_INTERP must not overwrite values from compressed a.out?
            auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc + ehdr->e_phoff);
            auxv_up(av, AT_PHNUM, ehdr->e_phnum);
            auxv_up(av, AT_PHENT, ehdr->e_phentsize);  /* ancient kernels might omit! */
            //auxv_up(av, AT_PAGESZ, PAGE_SIZE);  /* ld-linux.so.2 does not need this */
        }
        Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char  *addr = xo.buf = reloc + (char *)phdr->p_vaddr;
        char *hi_addr = phdr->p_memsz  + addr;  // end of local .bss
        char *addr2 = mlen + addr;  // end of local .data
        unsigned lo_frag  = (unsigned)(long)addr &~ PAGE_MASK;
        mlen += lo_frag;
        addr -= lo_frag;
#if defined(__powerpc64__) || defined(__aarch64__)
        // Round up to hardware PAGE_SIZE; allows emulator with smaller.
        // But (later) still need bzero when .p_filesz < .p_memsz .
        mlen += -(mlen + (size_t)addr) &~ PAGE_MASK;
        DPRINTF("  mlen=%%p\\n", mlen);
#endif

        DPRINTF("mmap addr=%%p  mlen=%%p  offset=%%p  lo_frag=%%p  prot=%%x\\n",
            addr, mlen, phdr->p_offset - lo_frag, lo_frag, prot);
        if (addr != mmap(addr, mlen,
                // If compressed, then we need PROT_WRITE to de-compress;
                // but then SELinux 'execmod' requires no PROT_EXEC for now.
                (prot | (xi ? PROT_WRITE : 0)) &~ (xi ? PROT_EXEC : 0),
                MAP_FIXED | MAP_PRIVATE | (xi ? MAP_ANONYMOUS : 0),
                (xi ? -1 : fdi), phdr->p_offset - lo_frag) ) {
            err_exit(8);
        }
        if (xi) {
            unpackExtent(xi, &xo, f_exp, f_unf);
        }
        // Linux does not fixup the low end, so neither do we.
        //if (PROT_WRITE & prot) {
        //    bzero(addr, lo_frag);  // fragment at lo end
        //}
        if (PROT_WRITE & prot) { // note: read-only .bss not supported here
            // Clear to end-of-page (first part of .bss or &_end)
            unsigned hi_frag = -(long)addr2 &~ PAGE_MASK;
            bzero(addr2, hi_frag);
            addr2 += hi_frag;  // will be page aligned
        }
        if (xi) {
#if defined(__x86_64)  //{
            void *const hatch = make_hatch_x86_64(phdr, reloc, ~PAGE_MASK);
#elif defined(__powerpc64__)  //}{
            void *const hatch = make_hatch_ppc64(phdr, reloc, ~PAGE_MASK);
#elif defined(__aarch64__)  //}{
            void *const hatch = make_hatch_arm64(phdr, reloc, ~PAGE_MASK);
#endif  //}
            if (0!=hatch) {
                auxv_up((Elf64_auxv_t *)(~1 & (size_t)av), AT_NULL, (size_t)hatch);
            }
            DPRINTF("mprotect addr=%%p  len=%%p  prot=%%x\\n", addr, mlen, prot);
            if (0!=mprotect(addr, mlen, prot)) {
                err_exit(10);
ERR_LAB
            }
        }
        if (addr2 < hi_addr) { // pages for .bss beyond last page for p_filesz
            DPRINTF("zmap addr2=%%p  len=%%p  prot=%%x\\n", addr2, hi_addr - addr2, prot);
            if (addr2 != mmap(addr2, hi_addr - addr2, prot,
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
    if (p_reloc) {
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
    struct b_info const *const bi,  // 1st block header
    size_t const sz_compressed,  // total length
    Elf64_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    Elf64_auxv_t *const av,
    f_expand *const f_exp,
    f_unfilter *const f_unf
#if defined(__x86_64)  //{
    , Elf64_Addr elfaddr  // In: &Elf64_Ehdr for stub
#elif defined(__powerpc64__)  //}{
    , Elf64_Addr *p_reloc  // In: &Elf64_Ehdr for stub; Out: 'slide' for PT_INTERP
    , size_t const PAGE_MASK
#elif defined(__aarch64__) //}{
    , Elf64_Addr elfaddr
    , size_t const PAGE_MASK
#endif  //}
)
{
    Extent xo, xi1, xi2;
    xo.buf  = (char *)ehdr;
    xo.size = bi->sz_unc;
    xi2.buf = CONST_CAST(char *, bi); xi2.size = bi->sz_cpr + sizeof(*bi);
    xi1.buf = CONST_CAST(char *, bi); xi1.size = sz_compressed;

    // ehdr = Uncompress Ehdr and Phdrs
    unpackExtent(&xi2, &xo, f_exp, 0);  // never filtered?

#if defined(__x86_64) || defined(__aarch64__)  //{
    Elf64_Addr *const p_reloc = &elfaddr;
#endif  //}
    DPRINTF("upx_main1  .e_entry=%%p  p_reloc=%%p  *p_reloc=%%p  PAGE_MASK=%%p\\n",
        ehdr->e_entry, p_reloc, *p_reloc, PAGE_MASK);
    Elf64_Phdr *phdr = (Elf64_Phdr *)(1+ ehdr);

    // De-compress Ehdr again into actual position, then de-compress the rest.
    Elf64_Addr entry = do_xmap(ehdr, &xi1, 0, av, f_exp, f_unf, p_reloc
#if defined(__powerpc64__) || defined(__aarch64__)
       , PAGE_MASK
#endif
    );
    DPRINTF("upx_main2  entry=%%p  *p_reloc=%%p\\n", entry, *p_reloc);
    auxv_up(av, AT_ENTRY , entry);

  { // Map PT_INTERP program interpreter
    phdr = (Elf64_Phdr *)(1+ ehdr);
    unsigned j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        char const *const iname = *p_reloc + (char const *)phdr->p_vaddr;
        int const fdi = open(iname, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
        if (MAX_ELF_HDR!=read(fdi, (void *)ehdr, MAX_ELF_HDR)) {
ERR_LAB
            err_exit(19);
        }
        // We expect PT_INTERP to be ET_DYN at 0.
        // Thus do_xmap will set *p_reloc = slide.
        *p_reloc = 0;  // kernel picks where PT_INTERP goes
        entry = do_xmap(ehdr, 0, fdi, 0, 0, 0, p_reloc
#if defined(__powerpc64__) || defined(__aarch64__)
            , PAGE_MASK
#endif
        );
        auxv_up(av, AT_BASE, *p_reloc);  // musl
        close(fdi);
    }
  }

    return (void *)entry;
}

#if DEBUG  //{

#if defined(__powerpc64__) //{
#define __NR_write 4

typedef unsigned long size_t;

#if 0  //{
static int
write(int fd, char const *ptr, size_t len)
{
    register  int        sys asm("r0") = __NR_write;
    register  int         a0 asm("r3") = fd;
    register void const  *a1 asm("r4") = ptr;
    register size_t const a2 asm("r5") = len;
    __asm__ __volatile__("sc"
    : "=r"(a0)
    : "r"(sys), "r"(a0), "r"(a1), "r"(a2)
    : "r0", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13"
    );
    return a0;
}
#else //}{
ssize_t
write(int fd, void const *ptr, size_t len)
{
    register  int        sys asm("r0") = __NR_write;
    register  int         a0 asm("r3") = fd;
    register void const  *a1 asm("r4") = ptr;
    register size_t       a2 asm("r5") = len;
    __asm__ __volatile__("sc"
    : "+r"(sys), "+r"(a0), "+r"(a1), "+r"(a2)
    :
    : "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13"
    );
    return a0;
}
#endif  //}
#endif  //}

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

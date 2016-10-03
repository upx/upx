/* i386-linux.elf-main.c -- stub loader for Linux x86 ELF executable

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
   Copyright (C) 2000-2016 John F. Reiser
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

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

#include "include/linux.h"
void *mmap(void *, size_t, int, int, int, off_t);
#if defined(__i386__) || defined(__mips__) //{
#  define mmap_privanon(addr,len,prot,flgs) mmap((addr),(len),(prot), \
        MAP_PRIVATE|MAP_ANONYMOUS|(flgs),-1,0)
#else  //}{
  void *mmap_privanon(void *, size_t, int, int);
#endif  //}
ssize_t write(int, void const *, size_t);


/*************************************************************************
// configuration section
**************************************************************************/

// In order to make it much easier to move this code at runtime and execute
// it at an address different from it load address:  there must be no
// static data, and no string constants.

#if !DEBUG  /*{*/
#define DPRINTF(a) /* empty: no debug drivel */
#define DEBUG_STRCON(name, value) /* empty */
#else  /*}{ DEBUG */
#if 0
#include "stdarg.h"
#else
#define va_arg      __builtin_va_arg
#define va_end      __builtin_va_end
#define va_list     __builtin_va_list
#define va_start    __builtin_va_start
#endif

#if defined(__i386__)  /*{*/
#define PIC_STRING(value, var) \
    __asm__ __volatile__ ( \
        "call 0f; .asciz \"" value "\"; \
      0: pop %0;" : "=r"(var) : \
    )
#elif defined(__arm__)  /*}{*/
#define PIC_STRING(value, var) \
    __asm__ __volatile__ ( \
        "mov %0,pc; b 0f; \
        .asciz \"" value "\"; .balign 4; \
      0: " : "=r"(var) \
    )
#elif defined(__mips__)  /*}{*/
#define PIC_STRING(value, var) \
    __asm__ __volatile__ ( \
        ".set noreorder; bal 0f; move %0,$31; .set reorder; \
        .asciz \"" value "\"; .balign 4; \
      0: " \
        : "=r"(var) : : "ra" \
    )
#endif  /*}*/


#define DEBUG_STRCON(name, strcon) \
    static char const *name(void) { \
        register char const *rv; PIC_STRING(strcon, rv); \
        return rv; \
    }


#ifdef __arm__  /*{*/
extern unsigned div10(unsigned);
#else  /*}{*/
static unsigned
div10(unsigned x)
{
    return x / 10u;
}
#endif  /*}*/

static int
unsimal(unsigned x, char *ptr, int n)
{
    if (10<=x) {
        unsigned const q = div10(x);
        x -= 10 * q;
        n = unsimal(q, ptr, n);
    }
    ptr[n] = '0' + x;
    return 1+ n;
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

DEBUG_STRCON(STR_hex, "0123456789abcdef");

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

typedef struct {
    size_t size;  // must be first to match size[0] uncompressed size
    char *buf;
} Extent;


DEBUG_STRCON(STR_xread, "xread %%p(%%x %%p) %%p %%x\\n")

static void
#if (ACC_CC_GNUC >= 0x030300) && defined(__i386__)  /*{*/
__attribute__((__noinline__, __used__, regparm(3), stdcall))
#endif  /*}*/
xread(Extent *x, char *buf, size_t count)
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
#define ERR_LAB /*empty*/
DEBUG_STRCON(STR_exit, "err_exit %%x\\n");

static void __attribute__ ((__noreturn__))
err_exit(int a)
{
    DPRINTF((STR_exit(), a));
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

DEBUG_STRCON(STR_unpackExtent,
        "unpackExtent in=%%p(%%x %%p)  out=%%p(%%x %%p)  %%p %%p\\n");
DEBUG_STRCON(STR_err5, "sz_cpr=%%x  sz_unc=%%x  xo->size=%%x\\n");

static void
unpackExtent(
    Extent *const xi,  // input
    Extent *const xo,  // output
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
            DPRINTF((STR_err5(), h.sz_cpr, h.sz_unc, xo->size));
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

DEBUG_STRCON(STR_make_hatch, "make_hatch %%p %%x %%x\\n");

#if defined(__i386__)  /*{*/
// Create (or find) an escape hatch to use when munmapping ourselves the stub.
// Called by do_xmap to create it; remembered in AT_NULL.d_val
static void *
make_hatch_x86(Elf32_Phdr const *const phdr, unsigned const reloc)
{
    unsigned *hatch = 0;
    DPRINTF((STR_make_hatch(),phdr,reloc,0));
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
            unsigned /*const*/ escape = 0xc36180cd;  // "int $0x80; popa; ret"
            // Don't store into read-only page if value is already there.
            if (* (volatile unsigned*) hatch != escape) {
                * hatch  = escape;
            }
        }
        else {
            hatch = 0;
        }
    }
    return hatch;
}
#elif defined(__arm__)  /*}{*/
static void *
make_hatch_arm(
    Elf32_Phdr const *const phdr,
    unsigned const reloc,
    unsigned const sys_munmap
)
{
    unsigned *hatch = 0;
    DPRINTF((STR_make_hatch(),phdr,reloc,sys_munmap));
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        // The format of the 'if' is
        //  if ( ( (hatch = loc1), test_loc1 )
        //  ||   ( (hatch = loc2), test_loc2 ) ) {
        //      action
        //  }
        // which uses the comma to save bytes when test_locj involves locj
        // and the action is the same when either test succeeds.

        // Try page fragmentation just beyond .text .
        if ( ( (hatch = (void *)(~3u & (3+ phdr->p_memsz + phdr->p_vaddr + reloc))),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (2*4)<=(~PAGE_MASK & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[8..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr + reloc)->e_ident[8])),
                (phdr->p_offset==0) ) )
        {
            hatch[0]= sys_munmap;  // syscall __NR_unmap
            hatch[1]= 0xe1a0f00e;  // mov pc,lr
            __clear_cache(&hatch[0], &hatch[2]);  // ? needed before mprotect()
        }
        else {
            hatch = 0;
        }
    }
    return hatch;
}
#elif defined(__mips__)  /*}{*/
static void *
make_hatch_mips(
    Elf32_Phdr const *const phdr,
    unsigned const reloc,
    unsigned const frag_mask)
{
    unsigned *hatch = 0;
    DPRINTF((STR_make_hatch(),phdr,reloc,frag_mask));
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        // Try page fragmentation just beyond .text .
        if ( ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (3*4)<=(frag_mask & -(int)hatch) ) ) // space left on page
        )
        {
            hatch[0]= 0x0000000c;  // syscall
            hatch[1]= 0x03200008;  // jr $25  # $25 === $t9 === jp
            hatch[2]= 0x00000000;  //   nop
        }
        else {
            hatch = 0;
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


static Elf32_auxv_t *
#if defined(__i386__)  /*{*/
__attribute__((regparm(2), stdcall))
#endif  /*}*/
auxv_find(Elf32_auxv_t *av, unsigned const type)
{
    Elf32_auxv_t *avail = 0;
    if (av
#if defined(__i386__)  /*{*/
    && 0==(1&(int)av)  /* PT_INTERP usually inhibits, except for hatch */
#endif  /*}*/
    ) {
        for (;; ++av) {
            if (av->a_type==type)
                return av;
            if (av->a_type==AT_IGNORE)
                avail = av;
        }
        if (0!=avail && AT_NULL!=type) {
                av = avail;
                av->a_type = type;
                return av;
        }
    }
    return 0;
}

DEBUG_STRCON(STR_auxv_up, "auxv_up  %%p %%x %%x\\n");

static void
#if defined(__i386__)  /*{*/
__attribute__((regparm(3), stdcall))
#endif  /*}*/
auxv_up(Elf32_auxv_t *av, unsigned const type, unsigned const value)
{
    DPRINTF((STR_auxv_up(),av,type,value));
    av = auxv_find(av, type);
    if (av) {
        av->a_un.a_val = value;
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

DEBUG_STRCON(STR_xfind_pages, "xfind_pages  %%x  %%p  %%d  %%p\\n");

// Find convex hull of PT_LOAD (the minimal interval which covers all PT_LOAD),
// and mmap that much, to be sure that a kernel using exec-shield-randomize
// won't place the first piece in a way that leaves no room for the rest.
static unsigned long  // returns relocation constant
#if defined(__i386__)  /*{*/
__attribute__((regparm(3), stdcall))
#endif  /*}*/
xfind_pages(unsigned mflags, Elf32_Phdr const *phdr, int phnum,
    char **const p_brk
#if defined(__mips__)  /*{ any machine with varying PAGE_SIZE */
    , unsigned const page_mask
#else  /*}{*/
#define page_mask PAGE_MASK
#endif  /*}*/
)
{
    size_t lo= ~0, hi= 0, szlo= 0;
    char *addr;
    DPRINTF((STR_xfind_pages(), mflags, phdr, phnum, p_brk));
    for (; --phnum>=0; ++phdr) if (PT_LOAD==phdr->p_type
#if defined(__arm__)  /*{*/
                               &&  phdr->p_memsz
// Android < 4.1 (kernel < 3.0.31) often has PT_INTERP of /system/bin/linker
// with bad PT_LOAD[0].  https://sourceforge.net/p/upx/bugs/221
// Type: EXEC (Executable file)
//
// Program Headers:
// Type   Offset   VirtAddr   PhysAddr FileSiz MemSiz  Flg  Align
// LOAD 0x0000d4 0x00000000 0xb0000000 0x00000 0x00000 R   0x1000
// LOAD 0x001000 0xb0001000 0xb0001000 0x07614 0x07614 R E 0x1000
// LOAD 0x009000 0xb0009000 0xb0009000 0x006f8 0x0ccdc RW  0x1000
#endif  /*}*/
                                                ) {
        if (phdr->p_vaddr < lo) {
            lo = phdr->p_vaddr;
            szlo = phdr->p_filesz;
        }
        if (hi < (phdr->p_memsz + phdr->p_vaddr)) {
            hi =  phdr->p_memsz + phdr->p_vaddr;
        }
    }
    szlo += ~page_mask & lo;  // page fragment on lo edge
    lo   -= ~page_mask & lo;  // round down to page boundary
    hi    =  page_mask & (hi - lo - page_mask -1);  // page length
    szlo  =  page_mask & (szlo    - page_mask -1);  // page length
    if (MAP_FIXED & mflags) {
        addr = (char *)lo;
    }
    else {
        addr = mmap_privanon((void *)lo, hi, PROT_NONE, mflags);
        //munmap(szlo + addr, hi - szlo);
    }
    *p_brk = hi + addr;  // the logical value of brk(0)
    return (unsigned long)addr - lo;
}

DEBUG_STRCON(STR_do_xmap,
    "do_xmap  fdi=%%x  ehdr=%%p  xi=%%p(%%x %%p)  av=%%p  p_reloc=%%p  f_unf=%%p\\n")

static Elf32_Addr  // entry address
do_xmap(int const fdi, Elf32_Ehdr const *const ehdr, Extent *const xi,
    Elf32_auxv_t *const av, unsigned *const p_reloc, f_unfilter *const f_unf)
{
    Elf32_Phdr const *phdr = (Elf32_Phdr const *) (ehdr->e_phoff +
        (void const *)ehdr);
#if defined(__mips__)  /*{ any machine with varying PAGE_SIZE */
    unsigned frag_mask = ~PAGE_MASK;
    {
        Elf32_auxv_t const *const av_pgsz = auxv_find(av, AT_PAGESZ);
        if (av_pgsz) {
            frag_mask = av_pgsz->a_un.a_val -1;
        }
    }
#else  /*}{*/
    unsigned const frag_mask = ~PAGE_MASK;
#endif  /*}*/
    char *v_brk;
#if defined(__arm__)  /*{*/
    unsigned const sys_munmap = *p_reloc;
#endif  /*}*/
    unsigned const reloc = xfind_pages(((ET_EXEC==ehdr->e_type) ? MAP_FIXED : 0),
         phdr, ehdr->e_phnum, &v_brk
#if defined(__mips__)  /*{ any machine with varying PAGE_SIZE */
        , ~frag_mask
#endif  /*}*/
        );
    int j;
    DPRINTF((STR_do_xmap(),
        fdi, ehdr, xi, (xi? xi->size: 0), (xi? xi->buf: 0), av, p_reloc, f_unf));
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (xi && PT_PHDR==phdr->p_type) {
        auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
    }
    else if (PT_LOAD==phdr->p_type
#if defined(__arm__)  /*{*/
         &&  phdr->p_memsz
#endif  /*}*/
                          ) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char * addr = xo.buf  =  (char *)(phdr->p_vaddr + reloc);
        char *const haddr =     phdr->p_memsz + addr;
        size_t frag  = (int)addr & frag_mask;
        mlen += frag;
        addr -= frag;

#if defined(__i386__)  /*{*/
    // Decompressor can overrun the destination by 3 bytes.
#  define LEN_OVER 3
#else  /*}{*/
#  define LEN_OVER 0
#endif  /*}*/

        if (xi) {
            if (addr != mmap_privanon(addr, LEN_OVER + mlen,
                    prot | PROT_WRITE, MAP_FIXED) )
                err_exit(6);
            unpackExtent(xi, &xo, (f_expand *)fdi,
                ((PROT_EXEC & prot) ? f_unf : 0) );
        }
        else {
            if (addr != mmap(addr, mlen, prot, MAP_FIXED | MAP_PRIVATE,
                    fdi, phdr->p_offset - frag) )
                err_exit(8);
        }
        // Linux does not fixup the low end, so neither do we.
        // Indeed, must leave it alone because some PT_GNU_RELRO
        // dangle below PT_LOAD (but still on the low page)!
        //if (PROT_WRITE & prot) {
        //    bzero(addr, frag);  // fragment at lo end
        //}
        frag = (-mlen) & frag_mask;  // distance to next page boundary
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
            void *const hatch = make_hatch_arm(phdr, reloc, sys_munmap);
            if (0!=hatch) {
                auxv_up(av, AT_NULL, (unsigned)hatch);
            }
#elif defined(__mips__)  /*}{*/
            void *const hatch = make_hatch_mips(phdr, reloc, frag_mask);
            if (0!=hatch) {
                auxv_up(av, AT_NULL, (unsigned)hatch);
            }
#endif  /*}*/
            if (0!=mprotect(addr, mlen, prot)) {
                err_exit(10);
ERR_LAB
            }
        }
        addr += mlen + frag;  /* page boundary on hi end */
        if (addr < haddr) { // need pages for .bss
            if (addr != mmap_privanon(addr, haddr - addr, prot, MAP_FIXED)) {
                err_exit(9);
            }
        }
#if defined(__i386__)  /*{*/
        else if (xi) { // cleanup if decompressor overrun crosses page boundary
            mlen = frag_mask & (3+ mlen);
            if (mlen<=3) { // page fragment was overrun buffer only
                munmap(addr, mlen);
            }
        }
#endif  /*}*/
    }
    if (xi && ET_DYN!=ehdr->e_type) {
        // Needed only if compressed shell script invokes compressed shell.
        do_brk(v_brk);
    }
    if (0!=p_reloc) {
        *p_reloc = reloc;
    }
    return ehdr->e_entry + reloc;
}

#if 0 && defined(__arm__)  //{
static uint32_t ascii5(char *p, uint32_t v, unsigned n)
{
    do {
        unsigned char d = (0x1f & v) + 'A';
        if ('Z' < d) d += '0' - (1+ 'Z');
        *--p = d;
        v >>= 5;
    } while (--n > 0);
    return v;
}
#endif  //}

DEBUG_STRCON(STR_upx_main,
    "upx_main av=%%p  szc=%%x  f_dec=%%p  f_unf=%%p  "
    "  xo=%%p(%%x %%p)  xi=%%p(%%x %%p)  dynbase=%%x\\n")

/*************************************************************************
// upx_main - called by our entry code
//
// This function is optimized for size.
**************************************************************************/

#if defined(__mips__)  /*{*/
void *upx_main(  // returns entry address
    struct b_info const *const bi,  // 1st block header
    size_t const sz_compressed,  // total length
    Elf32_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    Elf32_auxv_t *const av,
    f_expand *const f_decompress,
    f_unfilter *const f_unf
) __asm__("upx_main");
void *upx_main(  // returns entry address
    struct b_info const *const bi,  // 1st block header
    size_t const sz_compressed,  // total length
    Elf32_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    Elf32_auxv_t *const av,
    f_expand *const f_decompress,
    f_unfilter *const f_unf
)
#else  /*}{ !__mips__ */
void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const sz_compressed,
    f_expand *const f_decompress,
    f_unfilter */*const*/ f_unfilter,
    Extent xo,
    Extent xi,
    unsigned const volatile dynbase,
    unsigned const sys_munmap
) __asm__("upx_main");
void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const sz_compressed,
    f_expand *const f_decompress,
    f_unfilter */*const*/ f_unf,
    Extent xo,  // {sz_unc, ehdr}    for ELF headers
    Extent xi,  // {sz_cpr, &b_info} for ELF headers
    unsigned const volatile dynbase,  // value+result: compiler must not change
    unsigned const sys_munmap
)
#endif  /*}*/
{
#if !defined(__mips__)  /*{*/
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)(void *)xo.buf;  // temp char[MAX_ELF_HDR+OVERHEAD]
#endif  /*}*/
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(1+ ehdr), *zhdr = phdr;
    Elf32_Addr reloc;
    Elf32_Addr entry;

#if defined(__mips__)  /*{*/
    unsigned dynbase = 0;
    Extent xo, xi, xj;
    xo.buf  = (char *)ehdr;
    xo.size = bi->sz_unc;
    xi.buf = CONST_CAST(char *, bi); xi.size = sz_compressed;
    xj.buf = CONST_CAST(char *, bi); xj.size = sz_compressed;

    DPRINTF((STR_upx_main(),
        av, sz_compressed, f_decompress, f_unf, &xo, xo.size, xo.buf,
        &xi, xi.size, xi.buf, dynbase));

    // ehdr = Uncompress Ehdr and Phdrs
    unpackExtent(&xj, &xo, f_decompress, 0);  // never filtered?
#else  /*}{ !__mips__ */
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
#endif  /*}*/

    // Some kernels omit AT_PHNUM,AT_PHENT,AT_PHDR because this stub has no PT_INTERP.
    // That is "too much" optimization.  Linux 2.6.x seems to give all AT_*.
    //auxv_up(av, AT_PAGESZ, PAGE_SIZE);  /* ld-linux.so.2 does not need this */
    auxv_up(av, AT_PHNUM , ehdr->e_phnum);
    auxv_up(av, AT_PHENT , ehdr->e_phentsize);
    {
        while (PT_LOAD!=zhdr->p_type) ++zhdr;  // skip ARM PT_EXIDX and others
        auxv_up(av, AT_PHDR  , dynbase + (unsigned)(1+(Elf32_Ehdr *)zhdr->p_vaddr));
    }
    // AT_PHDR.a_un.a_val  is set again by do_xmap if PT_PHDR is present.
    // This is necessary for ET_DYN if|when we override a prelink address.

#if defined(__arm__)  /*{*/
    reloc = sys_munmap;  // sneak an input value
#elif !defined(__mips__)  /*}{*/
    (void)sys_munmap;  // UNUSED
#endif  /*}*/
    entry = do_xmap((int)f_decompress, ehdr, &xi, av, &reloc, f_unf);
    auxv_up(av, AT_ENTRY , entry);

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
        entry = do_xmap(fdi, ehdr, 0, av, &reloc, 0);
        auxv_up(av, AT_BASE, reloc);  // uClibc only?
        close(fdi);
        break;
    }
#if 0 && defined(__arm__)  //{ Hack for__clear_cache() not working.
#  define SET4(p, c0, c1, c2, c3) \
        (p)[0] = c0, (p)[1] = c1, (p)[2] = c2, (p)[3] = c3
    if (ehdr->e_phnum <= j) { // no PT_INTERP
        extern unsigned getpid(void);
        extern unsigned unlink(char const *);
        char tmpname_buf[20], *p = tmpname_buf;
        SET4(&p[0], '/', 't', 'm', 'p');
        SET4(&p[4], '/', 'u', 'p', 'x');
        p = &tmpname_buf[sizeof(tmpname_buf)];       *--p = '\0';
        unsigned r = ascii5(p, (uint32_t)getpid(), 4); p -= 4;
        Elf32_auxv_t *const avr = auxv_find(av, AT_RANDOM);
        if (avr) r ^= *(unsigned const *)(~3&(3+avr->a_un.a_val));
        ascii5(p, r, 7); p = &tmpname_buf[0];
        int const fdo = open(p, O_WRONLY | O_CREAT | O_EXCL, 0700);
        unlink(p);
        write(fdo, dynbase + (void const *)zhdr->p_vaddr, zhdr->p_memsz);
        close(fdo);
    }
#endif  //}
  }

    return (void *)entry;
}

/* vim:set ts=4 sw=4 et: */

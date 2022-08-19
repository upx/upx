/* i386-linux.elf-main.c -- stub loader for Linux x86 ELF executable

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

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

#include "include/linux.h"
void *mmap(void *, size_t, int, int, int, off_t);
#if defined(__i386__) || defined(__mips__) || defined(__powerpc__) //{
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
#elif defined(__x86_64) || defined(__i386__) //}{
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("call 0f; .asciz \"" fmt "\"; 0: pop %0" \
/*out*/ : "=r"(r_fmt) ); \
    dprintf(r_fmt, args); \
})
#elif defined(__arm__)  /*}{*/
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("mov %0,pc; b 0f; \
        .asciz \"" fmt "\"; .balign 4; \
      0: " \
/*out*/ : "=r"(r_fmt) ); \
    dprintf(r_fmt, args); \
})
#elif defined(__mips__)  /*}{*/
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm(".set noreorder; bal L%=j; move %0,$31; .set reorder; \
        .asciz \"" fmt "\"; .balign 4; \
      L%=j: " \
/*out*/ : "=r"(r_fmt) \
/* in*/ : \
/*und*/ : "ra"); \
    dprintf(r_fmt, args); \
})
#endif  //}

static int dprintf(char const *fmt, ...); // forward

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
#endif  /*}*/

#define MAX_ELF_HDR 512  // Elf32_Ehdr + n*Elf32_Phdr must fit in this


/*************************************************************************
// "file" util
**************************************************************************/

typedef struct {
    size_t size;  // must be first to match size[0] uncompressed size
    char *buf;
} Extent;

static void
#if (ACC_CC_GNUC >= 0x030300) && defined(__i386__)  /*{*/
__attribute__((__noinline__, __used__, regparm(3), stdcall))
#endif  /*}*/
xread(Extent *x, char *buf, size_t count)
{
    char *p=x->buf, *q=buf;
    size_t j;
    DPRINTF("xread %%p(%%x %%p) %%p %%x\\n", x, x->size, x->buf, buf, count);
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

#if !DEBUG  //{ save space
#define ERR_LAB error: exit(127);
#define err_exit(a) goto error
#else  //}{  save debugging time
#define ERR_LAB /*empty*/

extern void my_bkpt(int, ...);

static void __attribute__ ((__noreturn__))
err_exit(int a)
{
    DPRINTF("err_exit %%x\\n", a);
    (void)a;  // debugging convenience
#if defined(__powerpc__)  //{
    my_bkpt(a);
#endif  //}
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
          nrv_byte *, size_t *, unsigned );


static void
unpackExtent(
    Extent *const xi,  // input
    Extent *const xo,  // output
    f_expand *const f_exp,
    f_unfilter *f_unf
)
{
    DPRINTF("unpackExtent in=%%p(%%x %%p)  out=%%p(%%x %%p)  %%p %%p\\n",
        xi, xi->size, xi->buf, xo, xo->size, xo->buf, f_exp, f_unf);
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
            DPRINTF("sz_cpr=%%x  sz_unc=%%x  xo->size=%%x\\n", h.sz_cpr, h.sz_unc, xo->size);
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
#if defined(__i386__) //{
                *(int *)(void *)&h.b_method
#else
                h.b_method
#endif
                );
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
make_hatch_x86(Elf32_Phdr const *const phdr, ptrdiff_t reloc)
{
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,0);
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
             ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  4<=(~PAGE_MASK & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[12..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr + reloc)->e_ident[12])),
                (phdr->p_offset==0) )
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, PAGE_SIZE, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        ) {
            // Omitting 'const' saves repeated literal in gcc.
            unsigned /*const*/ escape = 0xc36180cd;  // "int $0x80; popa; ret"
            // Don't store into read-only page if value is already there.
            if (* (volatile unsigned*) hatch != escape) {
                * hatch  = escape;
            }
            if (xprot) {
                mprotect(hatch, 1*sizeof(unsigned), PROT_EXEC|PROT_READ);
            }
            DPRINTF(" hatch at %%p\\n", hatch);
        }
        else {
            hatch = 0;
        }
    }
    return hatch;
}
#elif defined(__arm__)  /*}{*/
extern unsigned get_sys_munmap(void);

static void *
make_hatch_arm(
    Elf32_Phdr const *const phdr,
    ptrdiff_t reloc
)
{
    unsigned const sys_munmap = get_sys_munmap();
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,sys_munmap);
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
            ( (hatch = (void *)(~3u & (3+ phdr->p_memsz + phdr->p_vaddr + reloc))),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (2*4)<=(~PAGE_MASK & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[8..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr + reloc)->e_ident[8])),
                (phdr->p_offset==0) )
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, PAGE_SIZE, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        ) {
            hatch[0] = sys_munmap;  // syscall __NR_unmap
            hatch[1] = 0xe1a0f00e;  // mov pc,lr
            __clear_cache(&hatch[0], &hatch[2]);  // ? needed before mprotect()
            if (xprot) {
                mprotect(hatch, 2*sizeof(unsigned), PROT_EXEC|PROT_READ);
            }
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
    ptrdiff_t reloc,
    unsigned const frag_mask)
{
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,frag_mask);
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        if (
        // Try page fragmentation just beyond .text .
            ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (3*4)<=(frag_mask & -(int)hatch) ) ) // space left on page
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, PAGE_SIZE, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        ) {
            hatch[0]= 0x0000000c;  // syscall
#define RS(r) ((037&(r))<<21)
#define JR 010
            hatch[1] = RS(30)|JR;  // jr $30  # s8
            hatch[2] = 0x00000000;  //   nop
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
#elif defined(__powerpc__)  /*}{*/
static void *
make_hatch_ppc32(
    Elf32_Phdr const *const phdr,
    ptrdiff_t reloc,
    unsigned const frag_mask)
{
    unsigned xprot = 0;
    unsigned *hatch = 0;
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,frag_mask);
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        if (
        // Try page fragmentation just beyond .text .
            ( (hatch = (void *)(phdr->p_memsz + phdr->p_vaddr + reloc)),
                ( phdr->p_memsz==phdr->p_filesz  // don't pollute potential .bss
                &&  (2*4)<=(frag_mask & -(int)hatch) ) ) // space left on page
        // Try Elf32_Ehdr.e_ident[8..15] .  warning: 'const' cast away
        ||   ( (hatch = (void *)(&((Elf32_Ehdr *)phdr->p_vaddr + reloc)->e_ident[8])),
                (phdr->p_offset==0) )
        // Allocate and use a new page.
        ||   (  xprot = 1, hatch = mmap(0, PAGE_SIZE, PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        ) {
            hatch[0] = 0x44000002;  // sc
            hatch[1] = 0x4e800020;  // blr
            if (xprot) {
                mprotect(hatch, 2*sizeof(unsigned), PROT_EXEC|PROT_READ);
            }
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


static void
#if defined(__i386__)  /*{*/
__attribute__((regparm(3), stdcall))
#endif  /*}*/
auxv_up(Elf32_auxv_t *av, unsigned const type, unsigned const value)
{
    DPRINTF("auxv_up  %%p %%x %%x\\n",av,type,value);
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


#if defined(__powerpc__)  //{
extern
size_t get_page_mask(void);  // variable page size AT_PAGESZ; see *-fold.S
#elif defined(__mips__)  //}{
    // empty
#else  //}{  // FIXME for __mips__
size_t get_page_mask(void) { return PAGE_MASK; }  // compile-time constant
#endif  //}

// Find convex hull of PT_LOAD (the minimal interval which covers all PT_LOAD),
// and mmap that much, to be sure that a kernel using exec-shield-randomize
// won't place the first piece in a way that leaves no room for the rest.
static ptrdiff_t // returns relocation constant
#if defined(__i386__)  /*{*/
__attribute__((regparm(3), stdcall))
#endif  /*}*/
xfind_pages(unsigned mflags, Elf32_Phdr const *phdr, int phnum,
    Elf32_Addr *const p_brk
#if defined (__mips__)  //{
    , size_t const page_mask
#endif  //}
)
{
#if !defined(__mips__)  //{
    size_t const page_mask = get_page_mask();
#endif  //}
    Elf32_Addr lo= ~0, hi= 0, addr = 0;
    DPRINTF("xfind_pages  %%x  %%p  %%d  %%p\\n", mflags, phdr, phnum, p_brk);
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
        }
        if (hi < (phdr->p_memsz + phdr->p_vaddr)) {
            hi =  phdr->p_memsz + phdr->p_vaddr;
        }
    }
    lo -= ~page_mask & lo;  // round down to page boundary
    hi  =  page_mask & (hi - lo - page_mask -1);  // page length
    DPRINTF("  addr=%%p  lo=%%p  hi=%%p\\n", addr, lo, hi);
    addr = (Elf32_Addr)mmap_privanon((void *)lo, hi, PROT_NONE, mflags);
    DPRINTF("  addr=%%p\\n", addr);
    *p_brk = hi + addr;  // the logical value of brk(0)
    return (ptrdiff_t)addr - lo;
}


static Elf32_Addr  // entry address
do_xmap(int const fdi, Elf32_Ehdr const *const ehdr, Extent *const xi,
    Elf32_auxv_t *const av, unsigned *const p_reloc, f_unfilter *const f_unf
#if defined(__mips__)  //{
    , size_t const page_mask
#endif  //}
)
{
#if defined(__mips__)  //{
    unsigned const frag_mask = ~page_mask;
#else  //}{
    unsigned const frag_mask = ~get_page_mask();
#endif  //}
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(void const *)(ehdr->e_phoff +
        (char const *)ehdr);
    Elf32_Addr v_brk;
    Elf32_Addr reloc;
    if (xi) { // compressed main program:
        // C_BASE space reservation, C_TEXT compressed data and stub
        Elf32_Addr  ehdr0 = *p_reloc;  // the 'hi' copy!
        Elf32_Phdr *phdr0 = (Elf32_Phdr *)(1+ (Elf32_Ehdr *)ehdr0);  // cheats .e_phoff
        // Clear the 'lo' space reservation for use by PT_LOADs
        if (ET_EXEC==((Elf32_Ehdr *)ehdr0)->e_type) {
            ehdr0 = phdr0[0].p_vaddr;
            reloc = 0;
        }
        else {
            ehdr0 -= phdr0[1].p_vaddr;
            reloc = ehdr0;
        }
        v_brk = phdr0->p_memsz + ehdr0;
        DPRINTF("do_xmap munmap [%%p, +%%x)\n", ehdr0, phdr0->p_memsz);
        munmap((void *)ehdr0, phdr0->p_memsz);
    }
    else { // PT_INTERP
        reloc = xfind_pages(
            ((ET_DYN!=ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum, &v_brk
#if defined(__mips__)  //{
         , page_mask
#endif  //}
        );
    }

#if DEBUG &&!defined(__mips__)  //{
    size_t const page_mask = 0;
#endif  //}
    DPRINTF("do_xmap  fdi=%%x  ehdr=%%p  xi=%%p(%%x %%p)\\n"
          "  av=%%p  page_mask=%%p  reloc=%%p  p_reloc=%%p/%%p  f_unf=%%p\\n",
        fdi, ehdr, xi, (xi? xi->size: 0), (xi? xi->buf: 0),
        av, page_mask, reloc, p_reloc, *p_reloc, f_unf);
    int j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j)
    if (xi && PT_PHDR==phdr->p_type) {
        auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
    } else
    if (PT_LOAD==phdr->p_type
#if defined(__arm__)  /*{*/
         &&  phdr->p_memsz
#endif  /*}*/
        ) {
        if (xi && !phdr->p_offset /*&& ET_EXEC==ehdr->e_type*/) { // 1st PT_LOAD
            // ? Compressed PT_INTERP must not overwrite values from compressed a.out?
            auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc + ehdr->e_phoff);
            auxv_up(av, AT_PHNUM, ehdr->e_phnum);
            auxv_up(av, AT_PHENT, ehdr->e_phentsize);  /* ancient kernels might omit! */
            //auxv_up(av, AT_PAGESZ, PAGE_SIZE);  /* ld-linux.so.2 does not need this */
        }
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char * addr = xo.buf  =  (char *)(phdr->p_vaddr + reloc);
        char *const haddr =     phdr->p_memsz + addr;
        size_t frag  = (int)addr & frag_mask;
        mlen += frag;
        addr -= frag;
        DPRINTF("  phdr type=%%x  offset=%%x  vaddr=%%x  paddr=%%x  filesz=%%x  memsz=%%x  flags=%%x  align=%%x\\n",
            phdr->p_type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr,
            phdr->p_filesz, phdr->p_memsz, phdr->p_flags, phdr->p_align);
        DPRINTF("  addr=%%x  mlen=%%x  frag=%%x  prot=%%x\\n", addr, mlen, frag, prot);

#if defined(__i386__)  /*{*/
    // Decompressor can overrun the destination by 3 bytes.
#  define LEN_OVER 3
#else  /*}{*/
#  define LEN_OVER 0
#endif  /*}*/

        if (xi) { // compresed source: mprotect(,,prot) later
            if (addr != mmap_privanon(addr, LEN_OVER + mlen,
                    PROT_WRITE | PROT_READ, MAP_FIXED) )
                err_exit(6);
            unpackExtent(xi, &xo, (f_expand *)fdi,
                ((PROT_EXEC & prot) ? f_unf : 0) );
        }
        else {  // PT_INTERP
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
            void *const hatch = make_hatch_arm(phdr, reloc);
            if (0!=hatch) {
                auxv_up(av, AT_NULL, (unsigned)hatch);
            }
#elif defined(__mips__)  /*}{*/
            void *const hatch = make_hatch_mips(phdr, reloc, frag_mask);
            if (0!=hatch) {
                auxv_up(av, AT_NULL, (unsigned)hatch);
            }
#elif defined(__powerpc__)  /*}{*/
            void *const hatch = make_hatch_ppc32(phdr, reloc, frag_mask);
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
            DPRINTF("addr=%%p  haddr=%%p\\n", addr, haddr);
            if (addr != mmap_privanon(addr, haddr - addr, prot, MAP_FIXED)) {
                for(;;);
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
        do_brk((void *)v_brk);
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
    f_expand *const f_exp,
    f_unfilter *const f_unf,
    Elf32_Addr const elfaddr,
    size_t const page_mask
) __asm__("upx_main");
void *upx_main(  // returns entry address
    struct b_info const *const bi,  // 1st block header
    size_t const sz_compressed,  // total length
    Elf32_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    Elf32_auxv_t *const av,
    f_expand *const f_exp,
    f_unfilter *const f_unf,
    Elf32_Addr const elfaddr,
    size_t const page_mask
)

#elif defined(__powerpc__) //}{
void *upx_main(  // returns entry address
    struct b_info const *const bi,  // 1st block header
    size_t const sz_compressed,  // total length
    Elf32_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    Elf32_auxv_t *const av,
    f_expand *const f_exp,
    f_unfilter *const f_unf,
    Elf32_Addr elfaddr
) __asm__("upx_main");
void *upx_main(  // returns entry address
    struct b_info const *const bi,  // 1st block header
    size_t const sz_compressed,  // total length
    Elf32_Ehdr *const ehdr,  // temp char[sz_ehdr] for decompressing
    Elf32_auxv_t *const av,
    f_expand *const f_exp,
    f_unfilter *const f_unf,
    Elf32_Addr elfaddr
)

#else  /*}{ !__mips__ && !__powerpc__ */
void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const sz_compressed,
    f_expand *const f_exp,
    f_unfilter * /*const*/ f_unfilter,
    Extent xo,
    Extent xi,
    Elf32_Addr const volatile elfaddr
) __asm__("upx_main");
void *upx_main(
    Elf32_auxv_t *const av,
    unsigned const sz_compressed,
    f_expand *const f_exp,
    f_unfilter * /*const*/ f_unf,
    Extent xo,  // {sz_unc, ehdr}    for ELF headers
    Extent xi,  // {sz_cpr, &b_info} for ELF headers
    Elf32_Addr const volatile elfaddr  // value+result: compiler must not change
)
#endif  /*}*/
{
#if defined(__i386__)  //{
    f_unf = (0xeb != *(unsigned char *)f_exp)  // 2-byte jmp around unfilter
        ? 0
        : (f_unfilter *)(2+ (long)f_exp);
#endif  //}

#if !defined(__mips__) && !defined(__powerpc__)  /*{*/
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)(void *)xo.buf;  // temp char[MAX_ELF_HDR+OVERHEAD]
    // sizeof(Ehdr+Phdrs),   compressed; including b_info header
    size_t const sz_first = xi.size;
#endif  /*}*/

#if defined(__powerpc__)  //{
    size_t const sz_first = sizeof(*bi) + bi->sz_cpr;
    Extent xo, xi;
    xo.buf = (char *)ehdr;           xo.size = bi->sz_unc;
    xi.buf = CONST_CAST(char *, bi); xi.size = sz_compressed;
#endif  //}

#if defined(__mips__)  /*{*/
    Extent xo, xi, xj;
    xo.buf  = (char *)ehdr;          xo.size = bi->sz_unc;
    xi.buf = CONST_CAST(char *, bi); xi.size = sz_compressed;
    xj.buf = CONST_CAST(char *, bi); xj.size = sizeof(*bi) + bi->sz_cpr;
#endif  //}

    DPRINTF("upx_main av=%%p  szc=%%x  f_exp=%%p  f_unf=%%p  "
            "  xo=%%p(%%x %%p)  xi=%%p(%%x %%p)  elfaddr=%%x\\n",
        av, sz_compressed, f_exp, f_unf, &xo, xo.size, xo.buf,
        &xi, xi.size, xi.buf, elfaddr);

#if defined(__mips__)  //{
    // ehdr = Uncompress Ehdr and Phdrs
    unpackExtent(&xj, &xo, f_exp, 0);
#else  //}{ !defined(__mips__)
    // Uncompress Ehdr and Phdrs.
    unpackExtent(&xi, &xo, f_exp, 0);
    // Prepare to decompress the Elf headers again, into the first PT_LOAD.
    xi.buf  -= sz_first;
    xi.size  = sz_compressed;
#endif  // !__mips__ }

    Elf32_Addr reloc = elfaddr;  // ET_EXEC problem!
    DPRINTF("upx_main1  .e_entry=%%p  reloc=%%p\\n", ehdr->e_entry, reloc);
    Elf32_Phdr *phdr = (Elf32_Phdr *)(1+ ehdr);

    // De-compress Ehdr again into actual position, then de-compress the rest.
    Elf32_Addr entry = do_xmap((int)f_exp, ehdr, &xi, av, &reloc, f_unf
#if defined(__mips__)  //{
        , page_mask
#endif  //}
        );
    DPRINTF("upx_main2  entry=%%p  reloc=%%p\\n", entry, reloc);
    auxv_up(av, AT_ENTRY , entry);

  { // Map PT_INTERP program interpreter
    int j;
    for (j=0, phdr = (Elf32_Phdr *)(1+ ehdr); j < ehdr->e_phnum; ++phdr, ++j)
    if (PT_INTERP==phdr->p_type) {
        int const fdi = open(reloc + (char const *)phdr->p_vaddr, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
        if (MAX_ELF_HDR!=read(fdi, (void *)ehdr, MAX_ELF_HDR)) {
ERR_LAB
            err_exit(19);
        }
        entry = do_xmap(fdi, ehdr, 0, av, &reloc, 0
#if defined(__mips__)  //{
            , page_mask
#endif  //}
        );
        DPRINTF("upx_main3  entry=%%p  reloc=%%p\\n", entry, reloc);
        auxv_up(av, AT_BASE, reloc);  // uClibc and musl
        close(fdi);
        break;
    }
  }

    return (void *)entry;
}

/* vim:set ts=4 sw=4 et: */

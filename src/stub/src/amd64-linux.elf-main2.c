/* amd64-linux.elf-main.c -- stub loader for Linux 64-bit ELF executable

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
   Copyright (C) John F. Reiser
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

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}

#include "include/linux.h"
// memfd_create, but fall back to /dev/shm
// such as Ubuntu-20.04, Linux kernel 5.15.0-67, #74-20.04.1, 2023-02-22
extern int upxfd_create(char const *tag, unsigned flags);
#define MFD_EXEC 0x0010
#define MS_SYNC 4
#define nullptr 0

extern void *memcpy(void *dst, void const *src, size_t n);
// Pprotect is mprotect but uses page-aligned address (Linux requirement)
extern unsigned Pprotect(void *, size_t, unsigned);
extern int Psync(void*, size_t, unsigned);
//extern void *Pmap(void *, size_t, unsigned, unsigned, int, size_t);
//extern int Punmap(void *, size_t);
extern size_t Pwrite(unsigned, void const *, size_t);
#  define mmap_privanon(addr,len,prot,flgs) mmap((addr),(len),(prot), \
        MAP_PRIVATE|MAP_ANONYMOUS|(flgs),-1,0)

extern void my_bkpt(void *, ...);

#if defined(__powerpc64__) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mflr %0" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
})
#elif defined(__x86_64) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("lea 9f(%%rip),%0; .section STRCON; 9:.asciz \"" string "\"; .previous" \
/*out*/ : "=r"(str) ); \
    str; \
})
#elif defined(__aarch64__) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mov %0,x30" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "x30"); \
    str; \
})
#elif defined(__riscv) //}{
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm("jal %0,0f; .string \"" string "\"; .balign 4; 0:" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : ); \
    str; \
})
#else  //}{
#error;
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
#if defined(__powerpc64__)  //{
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
#elif defined(__riscv) //}{
#define DPRINTF(fmt, args...) ({ \
    dprintf(addr_string(fmt), args); \
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

#define ElfW(sym) Elf64_ ## sym

#include "../../MAX_ELF_HDR.h"

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
    DPRINTF("xread done count=%%x\\n", count);
}


/*************************************************************************
// util
**************************************************************************/

#if !DEBUG  //{  save space
#define ERR_LAB error: exit(127);
#define err_exit(a) goto error
#else  //}{  save debugging time
#define ERR_LAB /*empty*/
static void
err_exit(int a)
{
    (void)a;  // debugging convenience
    DPRINTF("err_exit %%d\\n", a);
    my_bkpt((void *)(long)a);
    exit(127);
}
#endif  //}

/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

extern size_t get_page_mask(void);  // variable page size AT_PAGESZ; see *-fold.S

int f_expand( // .globl in $(ARCH)-expand.S
    nrv_byte const *binfo, nrv_byte *dst, size_t *dstlen);

static void
unpackExtent(
    Extent *const xi,  // input includes struct b_info
    Extent *const xo   // output
)
{
    while (xo->size) {
        DPRINTF("unpackExtent xi=(%%p %%p)  xo=(%%p %%p)\\n",
            xi->size, xi->buf, xo->size, xo->buf);
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
            int const j = f_expand((unsigned char *)xi->buf - sizeof(h),
                (unsigned char *)xo->buf, &out_len);
            if (j != 0 || out_len != (nrv_uint)h.sz_unc) {
                DPRINTF("  j=%%x  out_len=%%x  &h=%%p\\n", j, out_len, &h);
                err_exit(6);
            }
            xi->buf  += h.sz_cpr;
            xi->size -= h.sz_cpr;
        }
        else { // copy literal block
            DPRINTF("  copy %%p  %%p  %%p\\n", xi->buf, xo->buf, h.sz_cpr);
            xi->size += sizeof(h);  // xread(xi, &h, sizeof(h)) was a peek
            xread(xi, xo->buf, h.sz_cpr);
        }
        xo->buf  += h.sz_unc;
        xo->size -= h.sz_unc;
    }
    DPRINTF("unpackExtent done xo->buf=%%p\\n", xo->buf);
}

extern long upx_mmap_and_fd(  // x86_64 Android emulator of i386 is not faithful
     void *ptr
     , unsigned len  // also pre-allocate space in file
     , char *pathname  // 0 ==> call get_upxfn_path, which stores if 1st time
);

// If the escape hatch fits into the last page of .text,
// then put the escape hatch there and return a pointer to it.
// If it does not fit, then set the hatch[] array to the code
// for the eventual new page, and return nullptr.
// Always copying the code into the hatch array may be shorter overall.

#if defined(__x86_64__)  //{
static void *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned const page_mask,
    unsigned hatch[4]  // OUT: temp for prototype
)
{
    DPRINTF("make_hatch %%p %%p %%x\\n", phdr, next_unc, page-mask, hatch);
    hatch[0] = 0xfa1e0ff3;  // endbr64
    hatch[1] = 0x585a050f;  // syscall; pop %arg3{%rdx}; pop %rax
    hatch[2] = 0x90e0ff3e;  // notrack jmp *%rax; [[nop]]

    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        // no need to align on x86_64
        unsigned frag = ~page_mask & -(long)next_unc;  // bytes available
        if (11 <= frag) {  // fits on end of page
            memcpy(next_unc, hatch, 11);
            return next_unc;
        }
        else { // Does not fit
            return nullptr;  // fixup at end of upx_main2()
        }
    }
    DPRINTF("hatch=%%p\\n", hatch);
    return nullptr;
}
#elif defined(__riscv)  //}{
static short *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned const page_mask,
    unsigned hatch[4]  // OUT: temp for prototype
)
{
    DPRINTF("make_hatch %%p %%p %%x\\n", phdr, next_unc, page_mask, hatch);
    short *q = (short *)hatch[0];
    q[0] = 0x0073;  // ecall for munmap(ADRU, LENU)
    q[1] = 0x0000;  // upper 16 bits of ecall
    q[2] = 0x9002 | (15<<7);  // jalr x15

    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += 1+ phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        next_unc  = (char *)(void *)(~1ul & (long)(1+ next_unc));  // .balign 2
        if (3*2 <= (~page_mask & -(long)next_unc)) { // fits on end of page
            short *p = (short *)(void *)next_unc;
            p[0] = q[0];
            p[1] = q[1];
            p[2] = q[2];
            return p;
        }
        else { // does not fit
            return nullptr;  // fixup at end of upx_main2()
        }
    }
    return nullptr;
}
#elif defined(__powerpc64__)  //}{
static unsigned *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned const page_mask,
    unsigned hatch[4]  // OUT: temp for prototype
)
{
    DPRINTF("make_hatch %%p %%p %%x\\n", phdr, next_unc, page_mask, hatch);
    unsigned const *code;
    asm("bl 0f; \
        sc; \
        mr 12,31; \
        li 4,0; \
        blr; \
     0: mflr %0 "
/*out*/ : "=r"(code)
/* in*/ :
/*und*/ : "lr");
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        next_unc = (char *)(~3& (3+ (long)next_unc));  // .balign 4
        if (4*4 <= (~page_mask & -(long)next_unc)) { // fits on end of page
            memcpy(next_unc, code, 4*4);
            return (unsigned *)(void *)next_unc;
        }
        else { // Does not fit
            memcpy(hatch, code, 4*4);
            return nullptr;
        }
    }
    return nullptr;
}
#elif defined(__aarch64__)  //}{
#define NBPI 4
#define NINSTR 3
static unsigned *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned const page_mask,
    unsigned hatch[4]
)
{
    DPRINTF("make_hatch %%p %%p %%x\\n", phdr, next_unc, page_mask, hatch);
    unsigned *code;
    asm ("bl 0f; \
        svc #0; \
        br x30; \
     0: mov %0,x30"
/*out*/ : "=r"(code)
/* in*/ :
/*und*/ : );
    hatch[0] = code[0];
    hatch[1] = code[1];
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        next_unc  = (char *)(void *)(~3ul & (long)(3+ next_unc));  // .balign 4
        if (2*4 <= (~page_mask & -(long)next_unc)) { // fits on end of page
            unsigned *q = (unsigned *)(void *)next_unc;
            q[0] = code[0];
            q[1] = code[1];
            return q;
        }
        else { // Does not fit
            return nullptr;
        }
    }
    DPRINTF("hatch=%%p\\n", hatch);
    return nullptr;
}
#undef NBPI
#undef NINSTR
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

static ElfW(auxv_t) *
auxv_up(ElfW(auxv_t) *av, unsigned const type, uint64_t const value)
{
    ElfW(auxv_t) *rv = nullptr;
    if (!av || (1& (size_t)av)) { // none, or inhibited for PT_INTERP
        return rv;
    }
    DPRINTF("\\nauxv_up %%d  %%p\\n", type, value);
    // Multiple slots can have 'type' which wastes space but is legal.
    // rtld (ld-linux) uses the last one, so we must scan the whole table.
    Elf64_auxv_t *ignore_slot = 0;
    int found = 0;
    for (;; ++av) {
        DPRINTF("  %%d  %%p\\n", av->a_type, av->a_un.a_val);
        if (av->a_type == type) {
            av->a_un.a_val = value;
            rv = av;
            ++found;
        }
        else if (av->a_type == AT_IGNORE) {
            ignore_slot = av;
        }
        if (av->a_type==AT_NULL) { // done scanning
            if (found) {
                return rv;
            }
            if (ignore_slot) {
                ignore_slot->a_type = type;
                ignore_slot->a_un.a_val = value;
                return ignore_slot;
            }
            err_exit(20);
ERR_LAB
            return rv;
        }
    }
}

// Segregate large local array, to avoid code bloat due to large displacements.
// Not 'static' to disaable inlining, to control sizeof stack frame in callers.
/*static*/ void
underlay(unsigned size, char *ptr, unsigned len)  // len <= PAGE_SIZE
{
    DPRINTF("underlay size=%%u  ptr=%%p  len=%%u\\n", size, ptr, len);
    unsigned saved[4096/sizeof(unsigned)];
    memcpy(saved, ptr, len);
    mmap(ptr, size, PROT_WRITE|PROT_READ,
        MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    memcpy(ptr, saved, len);
}

#if 0  //{
// Exchange the bits with values 4 (PF_R, PROT_EXEC) and 1 (PF_X, PROT_READ)
// Use table lookup into a PIC-string that pre-computes the result.
unsigned PF_TO_PROT(unsigned flags)
{
    char const *table = addr_string("\x80\x04\x02\x06\x01\x05\x03\x07");
    return 7& table[flags & (PF_R|PF_W|PF_X)];
}
#else  //}{
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
#endif  //}

// Find convex hull of PT_LOAD (the minimal interval which covers all PT_LOAD),
// and mmap that much, to be sure that a kernel using exec-shield-randomize
// won't place the first piece in a way that leaves no room for the rest.

ElfW(Addr) // returns relocation constant
xfind_pages(unsigned mflags, ElfW(Phdr) const *phdr, int phnum, ElfW(Addr) *const p_brk)
{
    ElfW(Addr) lo= ~0, hi= 0, addr = 0, p_align = 0x1000;
    ElfW(Addr) page_mask = get_page_mask();
    DPRINTF("xfind_pages  %%x  %%p  %%d  %%p  %%p\\n", mflags, phdr, phnum, p_brk, page_mask);
    for (; --phnum>=0; ++phdr) if (PT_LOAD==phdr->p_type && phdr->p_memsz) {
        if (phdr->p_vaddr < lo) {
            lo = phdr->p_vaddr;
        }
        if (hi < (phdr->p_memsz + phdr->p_vaddr)) {
            hi =  phdr->p_memsz + phdr->p_vaddr;
        }
        if (p_align < phdr->p_align) {
            p_align = phdr->p_align;
        }
    } // end scan of PT_LOADs
    size_t page_size = 0u - page_mask;
    lo &= page_mask;  // round down to page boundary
    size_t len1 = page_mask & (hi - lo + page_size -1);  // desired length

    // Linux lacks mmap_aligned(), so allocate a larger space, then trim the ends.
    // Avoid division (p_align / page_size); both are powers of 2
    unsigned q = 1;
    while ((q * page_size) < p_align) q <<= 1;
    --q;  // number of extra pages
    unsigned len2 = len1 + (q * page_size);  // get enough space to align
    addr = (ElfW(Addr))mmap_privanon((void *)lo, len2, PROT_NONE, mflags);
    DPRINTF("  addr=%%p  lo=%%p  hi=%%p align=%%p  q=%%p  len1=%%p  len2=%%p  align=%%p\\n",
        addr, lo, hi, p_align, q, len1, len2, p_align);
    if (q) {
        size_t len3 = (-1 + p_align) & -addr;  // up to p_align boundary
        if (len3) {
            munmap((void *)addr, len3);  // trim the low end
            addr += len3;
            len2 -= len3;
        }
        if (len2 -= len1) {
            munmap((void *)(addr + len1), len2); // trim the high end
        }
    }
    DPRINTF("  addr=%%p\\n", addr);
    *p_brk = len1 + addr;  // the logical value of brk(0)
    DPRINTF("xfind_pages returns %%p\n", addr - lo);
    return (ptrdiff_t)addr - lo;
}

static void *
do_xmap( // mapped addr
    ElfW(Ehdr) const *const ehdr,  // &Ehdr of target output (copy; de-compressed)
    Extent *const xi,
    int const fdi,
    ElfW(auxv_t) *const av,
    ElfW(Addr) const base  // &Ehdr of compressed input (only 2 PT_LOAD, etc.)
)
{
    ElfW(Phdr) const *phdr = (ElfW(Phdr) const *)(void const *)
        (ehdr->e_phoff + (char const *)ehdr);
    ElfW(Addr) reloc = 0;  // default for ET_EXEC
    ElfW(Addr) v_brk = 0;
    unsigned hatch[4], *hatch_p = 0;
    if (xi) { // compressed main program:
        // C_BASE space reservation, C_TEXT compressed data and stub
        ElfW(Phdr) const *const phdr0 = (ElfW(Phdr) *)(1+ (ElfW(Ehdr) *)base);  // cheats .e_phoff
        if (ET_DYN == ehdr->e_type) {
            reloc = base - phdr0[1].p_vaddr;
            v_brk = base + phdr0->p_vaddr + phdr0->p_memsz;
        }
        // paranoia: prevent "hangover" from VMA for C_BASE
        //     munmap((void *)(reloc + phdr0->p_vaddr), phdr0->p_memsz);
        mmap_privanon((void *)(reloc + phdr0->p_vaddr), phdr0->p_memsz,
            PROT_READ|PROT_WRITE, MAP_FIXED);
    }
    else { // PT_INTERP; 'base' not used
        DPRINTF("INTERP\\n", 0);
        reloc = xfind_pages(
            ((ET_DYN!=ehdr->e_type) ? MAP_FIXED : 0), phdr, ehdr->e_phnum, &v_brk);
    }
    DPRINTF("do_xmap  ehdr=%%p  xi=%%p(%%x %%p)  fdi=%%x\\n"
          "  av=%%p  reloc=%%p\\n",
        ehdr, xi, (xi? xi->size: 0), (xi? xi->buf: 0), fdi, av, reloc);

    size_t const page_mask = get_page_mask();
    int j;
  for (j=0; j < ehdr->e_phnum; ++phdr, ++j) {
    if (xi && PT_PHDR==phdr->p_type) {
        auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc);
    } else
    if (PT_LOAD==phdr->p_type && phdr->p_memsz != 0) {
        unsigned const prot = PF_TO_PROT(phdr->p_flags);
        DPRINTF("\\n\\nLOAD@%%p  p_offset=%%p  p_vaddr=%%p  p_filesz=%%p"
            "  p_memsz=%%p  p_flags=%%x  prot=%%x\\n",
            phdr, phdr->p_offset, phdr->p_vaddr, phdr->p_filesz,
            phdr->p_memsz, phdr->p_flags, prot);
        if (xi && !phdr->p_offset /*&& ET_EXEC==ehdr->e_type*/) { // 1st PT_LOAD
            // ? Compressed PT_INTERP must not overwrite values from compressed a.out?
            auxv_up(av, AT_PHDR, phdr->p_vaddr + reloc + ehdr->e_phoff);
            auxv_up(av, AT_PHNUM, ehdr->e_phnum);
            auxv_up(av, AT_PHENT, ehdr->e_phentsize);  /* ancient kernels might omit! */
            //auxv_up(av, AT_PAGESZ, PAGE_SIZE);  /* ld-linux.so.2 does not need this */
        }
        Extent xo;
        size_t mlen = xo.size = phdr->p_filesz;
        char * addr = xo.buf  = reloc + (char *)phdr->p_vaddr;
            // xo.size, xo.buf are not changed except by unpackExtent()
        char *const hi_addr = phdr->p_memsz + addr;  // end of local .bss
        char *addr2 = mlen + addr;  // end of local .data
        size_t frag  = ~page_mask & (ElfW(Addr))addr;
        mlen += frag;
        addr -= frag;

#if defined(__powerpc64__) || defined(__aarch64__)
        // Round up to hardware PAGE_SIZE; allows emulator with smaller.
        // But (later) still need bzero when .p_filesz < .p_memsz .
        mlen += -(mlen + (size_t)addr) &~ page_mask;
        DPRINTF("  mlen=%%p\\n", mlen);
#endif

        DPRINTF("mmap addr=%%p  mlen=%%p  offset=%%p  frag=%%p  prot=%%x\\n",
            addr, mlen, phdr->p_offset - frag, frag, prot);
        int mfd = 0;
        if (xi && phdr->p_flags & PF_X) { // SELinux
            // Cannot set PROT_EXEC except via mmap() into a region (Linux "vma")
            // that has never had PROT_WRITE.  So use a Linux-only "memory file"
            // to hold the contents.
            mfd = upxfd_create(addr_string("upx"), MFD_EXEC);  // the directory entry
            ftruncate(mfd, mlen);  // Allocate the pages in the file.
            if (frag) {
                // Note: *addr does not exist yet, and figuring out a substitute
                // Note: location is a cumbersome heristic.  So do not attempt.
                //
                // write(mfd, addr, frag);  // Save lo fragment of contents on first page.
            }
            if (addr != mmap(addr, mlen, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, mfd, 0)) {
                err_exit(7);
            }
        }
        else {
            unsigned tprot = prot;
            // Notice that first 4 args are same: mmap vs mmap_privanon
            if (xi) {
                tprot |=  PROT_WRITE;  // De-compression needs Write
                tprot &= ~PROT_EXEC;  // Avoid simultaneous Write and eXecute
                if (addr != mmap_privanon(addr, mlen, tprot, MAP_FIXED|MAP_PRIVATE)) {
                    err_exit(11);
                }
            }
            else if (addr != mmap(addr, mlen, tprot, MAP_FIXED|MAP_PRIVATE,
                        fdi, phdr->p_offset - frag)) {
                err_exit(8);
            }
        }
        DPRINTF("addr= %%p\\n", addr);

        if (xi) {
            DPRINTF("before unpack xi=(%%p %%p  xo=(%%p %%p)\\n", xi->size, xi->buf, xo.size, xo.buf);
            unpackExtent(xi, &xo);  // updates xi and xo
            DPRINTF(" after unpack xi=(%%p %%p  xo=(%%p %%p)\\n", xi->size, xi->buf, xo.size, xo.buf);
        }
        if (PROT_WRITE & prot) { // note: read-only .bss not supported here
            // Clear to end-of-page (first part of .bss or &_end)
            unsigned hi_frag = -(long)addr2 &~ page_mask;
            bzero(addr2, hi_frag);
            addr2 += hi_frag;  // will be page aligned
        }

        if (xi && phdr->p_flags & PF_X) {
            if (!hatch_p) // try until hatch fits
                hatch_p = make_hatch(phdr, xo.buf, page_mask, hatch);

            // SELinux: Map the contents of mfd as per *phdr.
            DPRINTF("hatch protect addr=%%p  mlen=%%p\\n", addr, mlen);
            Psync(addr, mlen, MS_SYNC); // be sure file gets de-compressed bytes
            munmap(addr, mlen);  // toss the VMA that has PROT_WRITE
            if (addr != mmap(addr, mlen, prot, MAP_FIXED|MAP_SHARED, mfd, 0)) {
                err_exit(9);
            }
            close(mfd);
        }
        else if ((PROT_WRITE|PROT_READ) != prot
        &&  0!=Pprotect(addr, mlen, prot)) {
            err_exit(10);
ERR_LAB
        }
        if (addr2 < hi_addr) { // pages for .bss beyond last page for p_filesz
            DPRINTF("zmap addr2=%%p  len=%%p\\n", addr2, hi_addr - addr2);
            if (addr2 != mmap_privanon(addr2, hi_addr - addr2, prot, MAP_FIXED)) {
                err_exit(10);
            }
        }
    }
  }
    // install escape hatch
    if (xi && !hatch_p) { // hatch did not fit on end of .text page; need a new page
        unsigned long addr = (unsigned long)upx_mmap_and_fd((void *)0, sizeof(hatch), nullptr);
        DPRINTF("hatch new addr %%p\n", addr);
        unsigned mfd = 0xfff & addr;
        addr -= mfd;  // separate the addr
        --mfd;  // recover the mfd
        write(mfd, hatch, sizeof(hatch));  // the instrs
        hatch_p = (unsigned *)mmap((void *)addr, sizeof(hatch), PROT_READ|PROT_EXEC,
            MAP_PRIVATE|MAP_FIXED, mfd, 0);
        close(mfd);
    }
    if (xi)
        auxv_up((ElfW(auxv_t) *)(~1 & (size_t)av), AT_NULL, (size_t)hatch_p);

    if (xi) {
        brk((void *)v_brk);
    }
    return (char *)reloc;
}


/*************************************************************************
e/ upx_mnullptrain2 - called by our entry code
//
// This function is optimized for size.
**************************************************************************/

char *
upx_main2(  // returns entry address
/*arg1*/    struct b_info const *const bi,  // 1st block header
/*arg2*/    size_t const sz_compressed,  // total length
/*arg3*/    ElfW(Ehdr) *const ehdr,  // temp char[sz_ehdr] for decompressing
/*arg4*/    ElfW(auxv_t) *const av
#if defined(__x86_64) || defined(__aarch64__) || defined(__riscv) // {
/*arg5*/    , ElfW(Addr) elfaddr  // In: &ElfW(Ehdr) for stub
#elif defined(__powerpc64__)  // }{
/*arg5*/    , ElfW(Addr) *p_reloc  // In: &ElfW(Ehdr) for stub; Out: 'slide' for PT_INTERP
#endif  //}
)
{
    DPRINTF("upx_main2  b_info=%%p  sz_compressed=%%p  ehdr=%%p  av=%%p\\n",
        bi, sz_compressed, ehdr, av);
#if defined(__powerpc64__)
    ElfW(Addr) elfaddr = *p_reloc;
    DPRINTF("   p_reloc=%%p\\n", p_reloc);
#endif
    Extent xo, xi1, xi2;
    xo.buf  = (char *)ehdr;
    xo.size = bi->sz_unc;  // can require bi aligned(4)
    xi2.buf = CONST_CAST(char *, bi); xi2.size = bi->sz_cpr + sizeof(*bi);
    xi1.buf = CONST_CAST(char *, bi); xi1.size = sz_compressed;

    // ehdr = Uncompress Ehdr and Phdrs
    unpackExtent(&xi2, &xo);  // never filtered?

#if defined(__x86_64) || defined(__aarch64__) || defined(__riscv)  //{
    ElfW(Addr) *const p_reloc = &auxv_up(av, AT_NULL,
        ((ElfW(Phdr) *)(1+ (ElfW(Ehdr) *)elfaddr))[1].p_paddr)->a_un.a_val;
    *p_reloc = elfaddr;
    DPRINTF("main2  p_reloc=%%p  *p_reloc=%%p\n", p_reloc, *p_reloc);
#endif  //}
    ElfW(Addr) page_mask = get_page_mask(); (void)page_mask;
    DPRINTF("upx_main21  .e_entry=%%p  p_reloc=%%p  *p_reloc=%%p  page_mask=%%p\\n",
        ehdr->e_entry, p_reloc, (p_reloc ? *p_reloc : 0), page_mask);
    ElfW(Phdr) *phdr = (ElfW(Phdr) *)(1+ ehdr);

    // De-compress Ehdr again into actual position, then de-compress the rest.
    void *reloc = do_xmap(ehdr, &xi1, 0, av, elfaddr);
    char *entry = reloc + ((ElfW(Ehdr) *)ehdr)->e_entry;
    DPRINTF("upx_main22  entry=%%p  reloc=%%p\\n", entry, reloc);
    auxv_up(av, AT_ENTRY , (size_t)entry);

  { // Map PT_INTERP program interpreter
    phdr = (ElfW(Phdr) *)(1+ ehdr);
    unsigned j;
    for (j=0; j < ehdr->e_phnum; ++phdr, ++j) if (PT_INTERP==phdr->p_type) {
        char const *const iname = reloc + phdr->p_vaddr;
        int const fdi = open(iname, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
        if (MAX_ELF_HDR_64!=read(fdi, (void *)ehdr, MAX_ELF_HDR_64)) {
ERR_LAB
            err_exit(19);
        }
        // We expect PT_INTERP to be ET_DYN at 0.
        // Thus do_xmap will set *p_reloc = slide.
        reloc = do_xmap(ehdr, 0, fdi, 0, 0);
        entry = reloc + ((ElfW(Ehdr) *)reloc)->e_entry;
        auxv_up(av, AT_BASE, (size_t)reloc);  // musl
        close(fdi);
        break;
    }
  }
    return entry;
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

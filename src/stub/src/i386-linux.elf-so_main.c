/* i386-linux.elf-so_main.c -- stub loader for compressed shared library

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2021 Laszlo Molnar
   Copyright (C) 2000-2023 John F. Reiser
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

extern void my_bkpt(void const *arg1, ...);

#define DEBUG 0

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
#endif  //}

#ifdef __arm__  /*{*/
extern unsigned div10(unsigned);
#endif  /*}*/

#if DEBUG  //{
void dprint8(
    char const *fmt,
    void *a, void *b, void *c, void *d,
    void *e, void *f, void *g, void *h
)
{
    dprintf(fmt, a, b, c, d, e, f, g, h);
}
#endif  //}

// In order to make it much easier to move this code at runtime and execute
// it at an address different from it load address:  there must be no
// static data, and no string constants.

/*************************************************************************
// util
**************************************************************************/

#if 0  //{  save space
#define ERR_LAB error: exit(127);
#define err_exit(a) goto error
#else  //}{  save debugging time
#define ERR_LAB /*empty*/
void my_bkpt(void const *, ...);

static void
err_exit(int a)
{
    (void)a;  // debugging convenience
    DPRINTF("err_exit %%x\\n", a);
    my_bkpt((void const *)a);
    exit(127);
}
#endif  //}

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
        err_exit(8);
    }
    for (j = count; 0!=j--; ++p, ++q) {
        *q = *p;
    }
    x->buf  += count;
    x->size -= count;
    DPRINTF("yread x.size=%%x  x.buf=%%p  buf=%%p  count=%%x\\n",
        x->size, x->buf, buf, count);
}


/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

int f_expand( // .globl in $(ARCH)-linux.elf-so_fold.S
    nrv_byte const *binfo, nrv_byte *dst, size_t *dstlen);

static void
unpackExtent(
    Extent *const xi,  // input includes struct b_info
    Extent *const xo   // output
)
{
    while (xo->size) {
        DPRINTF("unpackExtent xi=(%%p %%p)  xo=(%%p %%p)  f_expand=%%p\\n",
            xi->size, xi->buf, xo->size, xo->buf, f_expand);
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
                err_exit(7);
            }
            xi->buf  += h.sz_cpr;
            xi->size -= h.sz_cpr;
        }
        else { // copy literal block
            DPRINTF("  copy %%p  %%p  %%p\\n", xi->buf, xo->buf, h.sz_cpr);
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
make_hatch_i386(Elf32_Phdr const *const phdr, ptrdiff_t reloc)
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
        // Linux on ARM wants PROT_EXEC or else __clear_cache does not work?
        ||   (  xprot = 1, hatch = mmap(0, PAGE_SIZE, PROT_EXEC|PROT_WRITE|PROT_READ,
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) )
        ) {
            hatch[0] = sys_munmap;  // syscall __NR_unmap
            hatch[1] = 0xe8bd80ff;  // ldmia sp!,{r0,r1,r2,r3,r4,r5,r6,r7,pc}
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

#define bzero(a,b)  __builtin_memset(a,0,b)

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

#define nullptr (void *)0

#if 0  //{
unsigned long
get_PAGE_MASK(void)  // the mask which KEEPS the page address
{
    int fd = open("/proc/self/auxv", O_RDONLY, 0);
    Elf32_auxv_t data[40];
    Elf32_auxv_t *end = &data[read(fd, data, sizeof(data)) / sizeof(data[0])];
    close(fd);
    Elf32_auxv_t *ptr; for (ptr = &data[0]; ptr < end ; ++ptr) {
        if (AT_PAGESZ == ptr->a_type) {
            return ~(-1+ ptr->a_un.a_val);
        }
    }
    return ~0xfff;
}
#endif  //}

extern void *memcpy(void *dst, void const *src, size_t n);
extern void *memset(void *dst, unsigned val, size_t n);

#ifndef __arm__  //{
// Segregate large local array, to avoid code bloat due to large displacements.
static void
underlay(unsigned size, char *ptr, unsigned len, unsigned p_flags)  // len < PAGE_SIZE
{
    (void)p_flags;  // for Linux ARM only
    unsigned saved[-PAGE_MASK/sizeof(unsigned)];
    memcpy(saved, ptr, len);
    mmap(ptr, size, PROT_WRITE|PROT_READ,
        MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    memcpy(ptr, saved, len);
}
#else  //}{
extern void
underlay(unsigned size, char *ptr, unsigned len, unsigned p_flags);
#endif  //}

typedef struct {
    int argc;
    char **argv;
    char **envp;
} So_args;

typedef struct {
    unsigned off_reloc;  // distance back to &Elf32_Ehdr
    unsigned off_user_DT_INIT;
    unsigned off_xct_off;  // where un-compressed bytes end  [unused?]
    unsigned off_info;  //  xct_off: {l_info; p_info; b_info; compressed data)
} So_info;

/*************************************************************************
// upx_so_main - called by our folded entry code
**************************************************************************/

void *
upx_so_main(  // returns &escape_hatch
    So_info *so_info,
    So_args *so_args
)
{
    char *const va_load = (char *)&so_info->off_reloc - so_info->off_reloc;
    So_info so_infc;  // So_info Copy
    memcpy(&so_infc, so_info, sizeof(so_infc));  // before de-compression overwrites
    unsigned const xct_off = so_infc.off_xct_off;
    (void)xct_off;  // use even if not DEBUG

    char *const cpr_ptr = so_infc.off_info + va_load;
    unsigned const cpr_len = (char *)so_info - cpr_ptr;
    DPRINTF("upx_so_main@%%p  va_load=%%p  cpr_ptr=%%p  cpr_len=%%x  xct_off=%%x\\n",
        upx_so_main, va_load, cpr_ptr, cpr_len, xct_off);

    // Copy compressed data before de-compression overwrites it.
    char *const sideaddr = mmap(nullptr, cpr_len, PROT_WRITE|PROT_READ,
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    DPRINTF("sideaddr=%%p\\n", sideaddr);
    memcpy(sideaddr, cpr_ptr, cpr_len);

    // Transition to copied data
    struct b_info *binfo = (struct b_info *)(void *)(sideaddr +
        sizeof(struct l_info) + sizeof(struct p_info));
    DPRINTF("upx_so_main  va_load=%%p  sideaddr=%%p  b_info=%%p\\n",
        va_load, sideaddr, binfo);

    // All the destination page frames exist or have been reserved,
    // but the access permissions may be wrong and the data may be compressed.
    // Also, rtld maps the convex hull of all PT_LOAD but assumes that the
    // file supports those pages, even though the pages might lie beyond EOF.
    // If so, then mprotect() is not enough: SIGBUS will occur.  Thus we
    // must mmap anonymous pages, except for first PT_LOAD with ELF headers.
    // So the general strategy (for each PT_LOAD) is:
    //   Save any contents on low end of destination page (the "prefix" pfx).
    //   mmap(,, PROT_WRITE|PROT_READ, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    //   Restore the prefix on the first destination page.
    //   De-compress from remaining [sideaddr, +sidelen).
    //   mprotect(,, PF_TO_PROT(.p_flags));

    // Get the uncompressed Elf32_Ehdr and Elf32_Phdr
    // The first b_info is aligned, so direct access of fields is OK.
    Extent x1 = {binfo->sz_unc, va_load};  // destination
    mprotect(va_load, binfo->sz_unc, PROT_WRITE|PROT_READ);
    Extent x0 = {binfo->sz_cpr + sizeof(*binfo), (char *)binfo};  // source
    unpackExtent(&x0, &x1);  // de-compress Elf headers; x0.buf is updated

    // Count PT_LOAD; n_LOAD < 3 is special (old binutils PT_LOAD layout)
    unsigned n_phdr;
    Elf32_Phdr const *phdr;
    n_phdr =                  ((Elf32_Ehdr *)(void *)va_load)->e_phnum;
      phdr = (Elf32_Phdr *)(1+ (Elf32_Ehdr *)(void *)va_load);
    unsigned n_LOAD = 0;
    for (; n_phdr > 0; --n_phdr, ++phdr) {
        n_LOAD += (PT_LOAD == phdr->p_type);
    }
    // Old-style binutils with only 2 PT_LOAD: (r-x) and (rw-)
    // has xct_off in middle of first PT_LOAD.
    // New-style binutils has xct-off at beginning of 2nd PT_LOAD.
    Elf32_Addr pfx = (n_LOAD <= 2) ? so_infc.off_xct_off : 0;

    // Process each read-only PT_LOAD.
    // A read+write PT_LOAD might be relocated by rtld before de-compression,
    // so it cannot be compressed.
    struct b_info al_bi;  // for aligned data from binfo
    void *hatch = nullptr;
    unsigned not_first = 0;

    n_phdr =                  ((Elf32_Ehdr *)(void *)va_load)->e_phnum;
      phdr = (Elf32_Phdr *)(1+ (Elf32_Ehdr *)(void *)va_load);
    for (; n_phdr > 0; --n_phdr, ++phdr)
    if ( phdr->p_type == PT_LOAD
    && !(phdr->p_flags & PF_W)
    && (not_first++ || n_LOAD < 3)
    ) {
        DPRINTF("phdr@%%p .p_vaddr=%%p  .p_filesz=%%p  .p_memsz=%%p  n_LOAD=%%p  binfo=%%p\\n",
            phdr, phdr->p_vaddr, phdr->p_filesz, phdr->p_memsz, n_LOAD, x0.buf);

        x0.size = sizeof(struct b_info);
        xread(&x0, (char *)&al_bi, x0.size);  // aligned binfo
        x0.buf -= sizeof(al_bi);  // back up (the xread() was a peek)
        DPRINTF("next1 pfx=%%x binfo@%%p (%%p %%p %%p)\\n", pfx, x0.buf,
            al_bi.sz_unc, al_bi.sz_cpr, *(unsigned *)(void *)&al_bi.b_method);

        // Using .p_memsz implicitly handles .bss via MAP_ANONYMOUS.
        x1.buf =  phdr->p_vaddr + pfx + va_load;
        x1.size = phdr->p_memsz - pfx;
        pfx = (phdr->p_vaddr + pfx) & ~PAGE_MASK;
        x1.buf  -= pfx;
        x1.size += pfx;
        DPRINTF("mmap(%%p %%p) xct_off=%%x pfx=%%x\\n", x1.buf, x1.size, xct_off, pfx);
        underlay(x1.size, x1.buf, pfx, phdr->p_flags);

        x1.buf += pfx;
        x1.size = al_bi.sz_unc;
        x0.size = al_bi.sz_cpr + sizeof(struct b_info);
        unpackExtent(&x0, &x1);  // updates x0 and x1
        pfx = 0;  // consider xct_off at most once

        if (!hatch && phdr->p_flags & PF_X) {
//#define PAGE_MASK ~0xFFFull
#if defined(__arm__)  //{
            hatch = make_hatch_arm(phdr, (Elf32_Addr)va_load);
#elif defined(__powerpc__)  //}{
            hatch = make_hatch_ppc(phdr, (Elf32_Addr)va_load, ~PAGE_MASK);
#elif defined(__i386__)  //}{
            hatch = make_hatch_i386(phdr, (Elf32_Addr)va_load);
#endif  //}
        }
        DPRINTF("mprotect %%p (%%p %%p)\\n", phdr, phdr->p_vaddr + va_load, phdr->p_memsz);
        mprotect(phdr->p_vaddr + va_load, phdr->p_memsz, PF_TO_PROT(phdr->p_flags));
    }

    typedef void (*Dt_init)(int argc, char *argv[], char *envp[]);
    Dt_init dt_init = (Dt_init)(void *)(so_infc.off_user_DT_INIT + va_load);
    munmap(sideaddr, cpr_len);
    DPRINTF("calling user DT_INIT %%p\\n", dt_init);
    dt_init(so_args->argc, so_args->argv, so_args->envp);

    DPRINTF("returning hatch=%%p\\n", hatch);
    return hatch;
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
        ptr[n++] = '-';
    }
    return unsimal(-x, ptr, n);
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
            n+= write(2, buf, unsimal((unsigned)(unsigned long)va_arg(va, void *), buf, 0));
        } break;
        case 'd': {
            n+= write(2, buf, decimal((int)(unsigned long)va_arg(va, void *), buf, 0));
        } break;
        case 'p': {
            buf[0] = '0';
            buf[1] = 'x';
            n+= write(2, buf, heximal((unsigned long)va_arg(va, void *), buf, 2));
        } break;
        case 'x': {
            buf[0] = '0';
            buf[1] = 'x';
            n+= write(2, buf, heximal((unsigned)(unsigned long)va_arg(va, void *), buf, 2));
        } break;
        } // 'switch'
    }
done:
    va_end(va);
    return n;
 }
#endif  //}

/* vim:set ts=4 sw=4 et: */

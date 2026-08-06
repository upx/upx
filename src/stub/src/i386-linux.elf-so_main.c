/* i386-linux.elf-so_main.c -- stub loader for compressed shared library

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


#include "include/linux.h"
#define NBPW 4
#define nullptr (void *)0

extern void my_bkpt(void const *arg1, ...);

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}

// Pprotect is mprotect, but page-aligned on the lo end (Linux requirement)
unsigned Pprotect(void *, size_t, unsigned);
void *mmap(void *, size_t, int, int, int, off_t);
void *Pmap(void *, size_t, int, int, int, off_t);
int Punmap(void *, size_t);
extern int Psync(void const *, size_t, unsigned);
#define MS_SYNC 4
#define EINVAL 22  /* Invalid argument */

#if defined(__i386__) || defined(__mips__) || defined(__powerpc__) //{
#  define mmap_privanon(addr,len,prot,flgs) mmap((addr),(len),(prot), \
        MAP_PRIVATE|MAP_ANONYMOUS|(flgs),-1,0)
#else  //}{
  void *mmap_privanon(void *, size_t, int, int);
#endif  //}
ssize_t write(int, void const *, size_t);
ssize_t Pwrite(int, void const *, size_t);


/*************************************************************************
// configuration section
**************************************************************************/

// In order to make it much easier to move this code at runtime and execute
// it at an address different from it load address:  there must be no
// static data, and no string constants.

#if defined(__i386__) || defined(__x86_64) //{
#define addr_string(string) ({ \
    char const *str; \
    asm("call 0f; .asciz \"" string "\"; 0: pop %0" \
/*out*/ : "=r"(str) ); \
    str; \
})
#elif defined(__arm__) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mov %0,lr" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
})
#elif defined(__mips__) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm(".set noreorder; bal 0f; move %0,ra; .set reorder; .string \"" string "\"; .balign 4;  0:" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "ra"); \
    str; \
})
#elif defined(__powerpc__)  //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mflr %0" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
    dprintf(r_fmt, args); \
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
       error;  // addr_string definition needed
#endif  //}  $(ARCH) addr_string

#if !DEBUG //{
#define DPRINTF(fmt, args...) /*empty*/
#else  //}{
// DPRINTF is defined as an expression using "({ ... })"
// so that DPRINTF can be invoked inside an expression,
// and then followed by a comma to ignore the return value.
// The only complication is that percent and backslash
// must be doubled in the format string, because the format
// string is processed twice: once at compile-time by 'asm'
// to produce the assembled value, and once at runtime to use it.
#define DPRINTF(fmt, args...) ({ dprintf(addr_string(fmt), args); })
static int dprintf(char const *fmt, ...); // forward
#endif  //} DEBUG

#ifdef __arm__  //{
extern unsigned div10(unsigned);
#endif  //}

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
}


/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

extern int f_expand( // .globl in $(ARCH)-linux.elf-so_fold.S
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
                err_exit(7);
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
    DPRINTF("  end unpackExtent\\n", 0);
}


#define ElfW(sym) Elf32_ ## sym

extern char *upx_mmap_and_fd(  // x86_64 Android emulator of i386 is not faithful
     void *ptr  // desired address
     , unsigned len  // also pre-allocate space in file
     , char *pathname  // 0 ==> call get_upxfn_path, which stores if 1st time
);

#if 0  //{
            int mfd = memfd_create(addr_string("upx"), 0);  // the directory entry
            write(mfd, &escape, 4);
            hatch = mmap(0, 4, PROT_READ|PROT_EXEC, MAP_PRIVATE, mfd, 0);
            close(mfd);

            char *fdmap = upx_mmap_and_fd((void *)0, sizeof(code), 0);
            unsigned mfd = -1+ (0xfff& (unsigned)fdmap);
            write(mfd, &code, sizeof(code));
            hatch = mmap((void *)((unsigned long)fdmap & ~0xffful), sizeof(code),
                PROT_READ|PROT_EXEC, MAP_PRIVATE, mfd, 0);
            close(mfd);
#endif  //}

#if defined(__i386__)  //{
// Create (or find) an escape hatch to use when munmapping ourselves the stub.
// Called by do_xmap to create it; remembered in AT_NULL.d_val
static char *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned frag_mask,
    unsigned hatch[4]
)
{
    DPRINTF("make_hatch %%p %%p %%x %%p\\n", phdr, next_unc, frag_mask, hatch);
    hatch[0] = 0xc36180cd;  // int $0x80; popa; ret
    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        frag_mask &= -(long)next_unc;  // bytes left on page
        if (4 <= frag_mask) {
            *(long *)&next_unc = hatch[0];
            return next_unc;
        }
        else { // Does not fit
            return nullptr;
        }
    }
    return nullptr;
}
#elif defined(__arm__)  /*}{*/
extern unsigned get_sys_munmap(void);
extern int upxfd_create(void);  // early 32-bit Android lacks memfd_create
#define SEEK_SET 0

static char *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned frag_mask,
    unsigned hatch[4]
)
{
    DPRINTF("make_hatch %%p %%p %%x %%p\\n", phdr, next_unc, frag_mask, hatch);
    hatch[0] = get_sys_munmap();  // syscall __NR_unmap
    hatch[1] = 0xe8bd80ff;  // ldmia sp!,{r0,r1,r2,r3,r4,r5,r6,r7,pc}

    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        next_unc = (char *)(~3 & (3+ (long)next_unc));  // .balign 4
        frag_mask &= -(long)next_unc;  // bytes left on page
        if (2*4 <= frag_mask) {
            ((unsigned *)(void *)next_unc)[0] = hatch[0];
            ((unsigned *)(void *)next_unc)[1] = hatch[1];
            return next_unc;
        }
        else { // Does not fit
            return nullptr;
        }
    }
    return nullptr;
}
#elif defined(__mips__)  /*}{*/
static char *
make_hatch(
    Elf32_Phdr const *const phdr,
    char *next_unc,
    unsigned const frag_mask,
    unsigned hatch[4]
)
{
    DPRINTF("make_hatch %%p %%x %%x %%[\\n",phdr,next_unc,frag_mask,hatch);
#define RS(r) ((037&(r))<<21)
#define JR 010
    hatch[0]= 0x0000000c;  // syscall
    hatch[1] = RS(31)|JR;  // jr ra
    hatch[2] = 0x00000000;  //   nop

    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        next_unc = ~3 & (3+ (long)next_unc);  // .balign 4
        frag_mask &= -(long)next_unc;  // bytes left on page
        if (3*4 <= frag_mask) {
            ((unsigned *)next_unc([0] = hatch[0];
            ((unsigned *)next_unc([1] = hatch[1];
            ((unsigned *)next_unc([2] = hatch[2];
            return next_unc;
        }
        else {
            return nullptr;
        }
    }
    return nullptr;
}
#elif defined(__powerpc__)  /*}{*/
static char *
make_hatch(
    ElfW(Phdr) const *const phdr,
    char *next_unc,
    unsigned const frag_mask,
    unsigned hatch[4]
{
    DPRINTF("make_hatch %%p %%x %%x\\n",phdr,reloc,frag_mask);
    hatch[0] = 0x44000002;  // sc
    hatch[1] = 0x4e800020;  // blr

    if (phdr->p_type==PT_LOAD && phdr->p_flags & PF_X) {
        next_unc += phdr->p_memsz - phdr->p_filesz;  // Skip over local .bss
        next_unc = ~3 & (3+ (long)next_unc);  // .balign 4
        frag_mask &= -(long)next_unc;  // bytes left on page
        if (2*4 <= frag_mask) {
            ((unsigned *)next_unc)[0] = hatch[0];
            ((unsigned *)next_unc)[1] = hatch[1];
            return next_unc;
        }
        else {
            return nullptr;
        }
    }
    return nullptr;
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

extern unsigned get_page_mask(void);
extern void *memcpy(void *dst, void const *src, size_t n);
extern void *memset(void *dst, unsigned val, size_t n);

#if defined(__powerpc64__) || defined(__powerpc__)  // {
#define SAVED_SIZE (1<<16)  /* 64 KB */
#else  // }{
#define SAVED_SIZE (1<<14)  /* 16 KB: RaspberryPi 5 */
#endif  // }

#ifndef __arm__  // {
// Segregate large local array, to avoid code bloat due to large displacements.
static void
underlay(unsigned size, char *ptr, unsigned page_mask)
{
    unsigned frag = ~page_mask & (unsigned)(long)ptr;
    if (frag) {
        unsigned char saved[SAVED_SIZE];
        ptr -= frag;
        memcpy(saved, ptr, frag);
        mmap(ptr, frag + size, PROT_WRITE|PROT_READ,
            MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        memcpy(ptr, saved, frag);
    }
    else { // already page-aligned
        mmap(ptr, frag + size, PROT_WRITE|PROT_READ,
            MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    }
}
#else  //}{ // use assembler because large local array on __arm__ is horrible
extern void
underlay(unsigned size, char *ptr, unsigned page_mask);
#endif  //}

// Exchange the bits with values 4 (PF_R, PROT_EXEC) and 1 (PF_X, PROT_READ)
// Use table lookup into a PIC-string that pre-computes the result.
unsigned PF_to_PROT(ElfW(Phdr) const *phdr)
{
    return 7& addr_string("@\x04\x02\x06\x01\x05\x03\x07")
        [phdr->p_flags & (PF_R|PF_W|PF_X)];
}

unsigned
fini_SELinux(
    unsigned size,
    char *ptr,
    ElfW(Phdr) const *phdr,
    unsigned mfd,
    ElfW(Addr) base
)
{
    DPRINTF("fini_SELinux  size=%%p  ptr=%%p  phdr=%%p  mfd=%%p  base=%%p\\n",
            size, ptr, phdr, mfd, base);
    if (phdr->p_flags & PF_X) {
        // Map the contents of mfd as per *phdr.

        Psync(ptr, size, MS_SYNC); // be sure file gets de-compressed bytes
            // Android 14 gets -EINVAL; ignore it

        Punmap(ptr, size);
        Pmap(ptr, size, PF_to_PROT(phdr), MAP_FIXED|MAP_PRIVATE, mfd, 0);
        close(mfd);
    }
    else { // easy
        Pprotect( (char *)(phdr->p_vaddr + base), phdr->p_memsz, PF_to_PROT(phdr));
    }
    return 0;
}

unsigned
prep_SELinux(unsigned size, char *ptr, ElfW(Addr) page_mask) // returns mfd
{
    // Cannot set PROT_EXEC except via mmap() into a region (Linux "vma")
    // that has never had PROT_WRITE.  So use a Linux-only "memory file"
    // to hold the contents.
    char saved[SAVED_SIZE];
    char *page = (char *)(page_mask & (ElfW(Addr))ptr);
    unsigned frag = (unsigned)(ptr - page);
    DPRINTF("prep_SELinux size= %%p  ptr= %%p  page_mask= %%p\\n",
        size, ptr, page_mask);
    if (frag) {
        memcpy(saved, page, frag);
    }
    char *val = upx_mmap_and_fd(page, frag + size, nullptr);
    unsigned mfd = 0xfff & (unsigned)(ElfW(Addr))val;
    val -= mfd; --mfd;
    if (val != page) {
        my_bkpt((void const *)0x1262, val, page, ptr, frag);
    }
    if (frag)
        write(mfd, saved, frag);  // Save lo fragment of contents on page.
    return mfd;
}

typedef void (*Dt_init)(int argc, char *argv[], char *envp[]);

typedef struct {
    long argc;
    char **argv;
    char **envp;
} So_args;

typedef struct {
    unsigned off_reloc;  // distance back to &ElfW(Ehdr)
    unsigned off_user_DT_INIT;
    unsigned off_xct_off;  // where un-compressed bytes end
    unsigned off_info;  //  xct_off: {l_info; p_info; b_info; compressed data)
} So_info;

/*************************************************************************
// upx_so_main - called by our folded entry code
**************************************************************************/

void *
upx_so_main(  // returns &escape_hatch
    So_info *so_info,
    So_args *so_args,
    ElfW(Ehdr) *elf_tmp  // scratch for ElfW(Ehdr) and ElfW(Phdrs)
)
{
    ElfW(Addr) const page_mask = get_page_mask();
    char *const va_load = (char *)&so_info->off_reloc - so_info->off_reloc;
    So_info so_infc;  // So_info Copy
    memcpy(&so_infc, so_info, sizeof(so_infc));  // before de-compression overwrites
    unsigned const xct_off = so_infc.off_xct_off;  (void)xct_off;

    char *const cpr_ptr = so_info->off_info + va_load;
    unsigned const cpr_len = (char *)so_info - cpr_ptr;
    Dt_init const dt_init = (Dt_init)(void *)(so_info->off_user_DT_INIT + va_load);
    DPRINTF("upx_so_main  va_load=%%p  so_info= %%p  so_infc=%%p  cpr_ptr=%%p  cpr_len=%%x  xct_off=%%x  dt_init=%%p\\n",
        va_load, so_info, &so_infc, cpr_ptr, cpr_len, xct_off, dt_init);
    // DO NOT USE *so_info AFTER THIS!!  It gets overwritten.

    // Copy compressed data before de-compression overwrites it.
    char *const sideaddr = mmap(nullptr, cpr_len, PROT_WRITE|PROT_READ,
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    DPRINTF("&sideaddr=%%p\\n", &sideaddr);
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
    // If so, then Pprotect() is not enough: SIGBUS will occur.  Thus we
    // must mmap anonymous pages, except for first PT_LOAD with ELF headers.
    // So the general strategy (for each PT_LOAD) is:
    //   Save any contents on low end of destination page (the "prefix" pfx).
    //   mmap(,, PROT_WRITE|PROT_READ, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    //   Restore the prefix on the first destination page.
    //   De-compress from remaining [sideaddr, +sidelen).
    //   Pprotect(,, PF_TO_PROT(.p_flags));

    // Get the uncompressed ElfW(Ehdr) and ElfW(Phdr)
    // The first b_info is aligned, so direct access to fields is OK.
    Extent x1 = {binfo->sz_unc, (char *)elf_tmp};  // destination
    Extent x0 = {binfo->sz_cpr + sizeof(*binfo), (char *)binfo};  // source
    unpackExtent(&x0, &x1);  // de-compress _Ehdr and _Phdrs; x0.buf is updated

    ElfW(Phdr) const *phdr = (ElfW(Phdr) *)(1+ elf_tmp);
    ElfW(Phdr) const *const phdrN = &phdr[elf_tmp->e_phnum];

    // Process each read-only PT_LOAD.
    // A read+write PT_LOAD might be relocated by rtld before de-compression,
    // so it cannot be compressed.
    unsigned hatch[4], *hatch_p = nullptr;
    memset(hatch, 0, sizeof(hatch));  // pacify valgrind
    ElfW(Addr) base = 0;
    int n_load = 0;

    for (; phdr < phdrN; ++phdr)
    if (phdr->p_type == PT_LOAD && !(phdr->p_flags & PF_W)) {
        if  (!base) {
            base = (ElfW(Addr))va_load - phdr->p_vaddr;
            DPRINTF("base= %%p\\n", base);
        }
        unsigned const va_top = phdr->p_filesz + phdr->p_vaddr;
        // Need un-aligned read of b_info to determine compression sizes.
        struct b_info al_bi;  // for aligned data from binfo
        x0.size = sizeof(struct b_info);
        xread(&x0, (char *)&al_bi, x0.size);  // aligned binfo
        x0.buf -= sizeof(al_bi);  // back up (the xread() was a peek)
        x0.size = al_bi.sz_cpr;
        x1.size = al_bi.sz_unc;
        x1.buf = (void *)(va_top + base - al_bi.sz_unc);

        DPRINTF("\\nphdr@%%p  p_offset=%%p  p_vaddr=%%p  p_filesz=%%p  p_memsz=%%p\\n",
            phdr, phdr->p_offset, phdr->p_vaddr, phdr->p_filesz, phdr->p_memsz);
        DPRINTF("x0=%%p  x1=%%p\\n", &x0, &x1);

        if ((phdr->p_filesz + phdr->p_offset) <= xct_off) { // va_top <= xct_off
            if (!n_load) {
                ++n_load;
                continue;  // 1st PT_LOAD is non-compressed loader tables ONLY!
            }
        }

        int mfd = 0;
        if (phdr->p_flags & PF_X) {
            mfd = prep_SELinux(x1.size, x1.buf, page_mask);
        }
        else {
            underlay(x1.size, x1.buf, page_mask);  // also makes PROT_WRITE
        }
        Extent xt = x1;
        unpackExtent(&x0, &x1);  // updates *x0 and *x1
        if (!hatch_p && phdr->p_flags & PF_X) {
            int mfd = memfd_create(addr_string("upx"), 0);  // the directory entry
            write(mfd, hatch, sizeof(hatch));
            hatch_p = mmap(0, 4, PROT_READ|PROT_EXEC, MAP_PRIVATE, mfd, 0);
            close(mfd);
            hatch_p = (void *)make_hatch(phdr, x1.buf, ~page_mask, hatch);
        }
        fini_SELinux(xt.size, xt.buf, phdr, mfd, base);
        ++n_load;
    }

    DPRINTF("Punmap sideaddr=%%p  cpr_len=%%p\\n", sideaddr, cpr_len);
    Punmap(sideaddr, cpr_len);
    DPRINTF("calling user DT_INIT %%p\\n", dt_init);
#ifdef __mips__  //{
    extern Dt_init PIC_call(Dt_init);
    // Note: MIPS does not pass argc,argv,envp to 'constructor' inits
    PIC_call(dt_init)
#else  //}{
             dt_init
#endif  //}
                    (so_args->argc, so_args->argv, so_args->envp);

    DPRINTF("returning hatch=%%p\\n", hatch,_p);
    return hatch_p;
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

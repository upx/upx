/* amd64-darwin.macho-main.c -- loader stub for Mach-o AMD64

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


#define __WORDSIZE 64
#include "include/darwin.h"

#define SIMULATE_ON_LINUX_EABI4 0

#if defined(__arm__)  //{
#define DEBUG 0  /* __arm__ */
#endif  //}

#if defined(__aarch64__)  //{
#define DEBUG 0  /* __aarch64__ */
#endif  //}

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

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
#elif defined(__arm__)  //}{
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("bl 0f; .string \"" fmt "\"; .balign 4; 0: mov %0,lr" \
/*out*/ : "=r"(r_fmt) \
/* in*/ : \
/*und*/ : "lr"); \
    dprintf(r_fmt, args); \
})
#endif  //}

static int dprintf(char const *fmt, ...); // forward
#endif  /*}*/


/*************************************************************************
// "file" util
**************************************************************************/

typedef struct {
    size_t size;  // must be first to match size[0] uncompressed size
    void *buf;
} Extent;

static void
xread(Extent *x, void *buf, size_t count)
{
    unsigned char *p=x->buf, *q=buf;
    size_t j;
    DPRINTF("xread %%p(%%x %%p) %%p %%x\\n", x, x->size, x->buf, buf, count);
    if (x->size < count) {
        DPRINTF("xreadfail %%p(%%x %%p) %%p %%x\\n",
            x, x->size, x->buf, buf, count);
        exit(127);
    }
    for (j = count; 0!=j--; ++p, ++q) {
        *q = *p;
    }
    DPRINTF("   buf: %%x %%x %%x\\n", ((int *)buf)[0], ((int *)buf)[1], ((int *)buf)[2]);
    x->buf  += count;
    x->size -= count;
}

static void
xpeek(Extent *x, void *buf, size_t count)
{
    DPRINTF("xpeek buf=%%p  count=%%x  ", buf, count);
    xread(x, buf, count);
    x->size += count;
    x->buf  -= count;
}

/*************************************************************************
// util
**************************************************************************/

#if 0  //{  save space
#define ERR_LAB error: exit(127);
#define err_exit(a) goto error
#else  //}{  save debugging time
#define ERR_LAB /*empty*/

static void
err_exit(int a)
{
    DPRINTF("err_exit %%x\\n", a);
    (void)a;  // debugging convenience
    exit(a);
}
#endif  //}


/*************************************************************************
// UPX & NRV stuff
**************************************************************************/

struct l_info { // 12-byte trailer for loader (after macho headers)
    unsigned l_checksum;
    unsigned l_magic;  // UPX_MAGIC_LE32
    unsigned short l_lsize;
    unsigned char l_version;
    unsigned char l_format;
};
struct p_info { // 12-byte packed program header
    unsigned p_progid;
    unsigned p_filesize;
    unsigned p_blocksize;
};

struct b_info { // 12-byte header before each compressed block
    unsigned sz_unc;  // uncompressed_size
    unsigned sz_cpr;  //   compressed_size
    unsigned char b_method;  // compression algorithm
    unsigned char b_ftid;  // filter id
    unsigned char b_cto8;  // filter parameter
    unsigned char b_extra;
};

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
        xread(xi, (unsigned char *)&h, sizeof(h));
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
            DPRINTF("sz_cpr=%%x  sz_unc=%%x  xo->size=%%x\\n",
                h.sz_cpr, h.sz_unc, xo->size);
            err_exit(5);
        }
        // Now we have:
        //   assert(h.sz_cpr <= h.sz_unc);
        //   assert(h.sz_unc > 0 && h.sz_unc <= blocksize);
        //   assert(h.sz_cpr > 0 && h.sz_cpr <= blocksize);

        if (h.sz_cpr < h.sz_unc) { // Decompress block
            size_t out_len = h.sz_unc;  // EOF for lzma
            int const j = (*f_exp)(xi->buf, h.sz_cpr,
                xo->buf, &out_len, h.b_method);
            if (j != 0 || out_len != (nrv_uint)h.sz_unc)
                err_exit(7);
            if (h.b_ftid!=0 && f_unf) {  // have filter
                (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);
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

static void
upx_bzero(unsigned char *p, size_t len)
{
    if (len) do {
        *p++= 0;
    } while (--len);
}
#define bzero upx_bzero


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

typedef struct {
    unsigned magic;
    unsigned nfat_arch;
} Fat_header;
typedef struct {
    unsigned cputype;
    unsigned cpusubtype;
    unsigned offset;
    unsigned size;
    unsigned align;  /* shift count (log base 2) */
} Fat_arch;
    enum e8 {
        FAT_MAGIC = 0xcafebabe,
        FAT_CIGAM = 0xbebafeca
    };
    enum e9 {
        CPU_TYPE_I386      =          7,
        CPU_TYPE_AMD64     = 0x01000007,
        CPU_TYPE_ARM       =         12,
        CPU_TYPE_POWERPC   = 0x00000012,
        CPU_TYPE_POWERPC64 = 0x01000012
    };

typedef struct {
    unsigned magic;
    unsigned cputype;
    unsigned cpysubtype;
    unsigned filetype;
    unsigned ncmds;
    unsigned sizeofcmds;
    unsigned flags;
#if defined(__x86_64__) || defined(__aarch64__)  //{
    unsigned reserved;  // for 64-bit alignment
#endif  //}
} Mach_header;  // also Mach_header64 which has 4 more bytes
        enum e0 {
            MH_MAGIC   =   0xfeedface,
            MH_MAGIC64 = 1+0xfeedface
        };
        enum e2 {
            MH_EXECUTE = 2,
            MH_DYLINKER= 7     /* /usr/bin/dyld */
        };
        enum e3 {
            MH_NOUNDEFS = 1
            , MH_PIE      = 0x200000   // ASLR
        };

typedef struct {
    unsigned cmd;
    unsigned cmdsize;
} Mach_load_command;
        enum e4 {
//            LC_SEGMENT       = 0x1,
//            LC_SEGMENT_64    = 0x19,
            LC_THREAD        = 0x4,
            LC_UNIXTHREAD    = 0x5,
            LC_LOAD_DYLINKER = 0xe
        };

typedef struct {
    unsigned cmd;
    unsigned cmdsize;
    char segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    unsigned maxprot;
    unsigned initprot;
    unsigned nsects;
    unsigned flags;
} Mach_segment_command;
        enum e5 {
            VM_PROT_READ = 1,
            VM_PROT_WRITE = 2,
            VM_PROT_EXECUTE = 4
        };

typedef struct {
    char sectname[16];
    char segname[16];
    uint64_t addr;   /* memory address */
    uint64_t size;   /* size in bytes */
    unsigned offset; /* file offset */
    unsigned align;  /* power of 2 */
    unsigned reloff; /* file offset of relocation entries */
    unsigned nreloc; /* number of relocation entries */
    unsigned flags;  /* section type and attributes */
    unsigned reserved1;  /* for offset or index */
    unsigned reserved2;  /* for count or sizeof */
} Mach_section_command;

typedef struct {
    uint32_t cmd;  // LC_MAIN;  MH_EXECUTE only
    uint32_t cmdsize;  // 24
    uint64_t entryoff;  // file offset of main() [expected in __TEXT]
    uint64_t stacksize;  // non-default initial stack size
} Mach_main_command;

#if defined(__aarch64__)  // {
typedef struct {
    uint64_t x0,  x1,  x2,  x3;
    uint64_t x4,  x5,  x6,  x7;
    uint64_t x8,  x9,  x10, x11;
    uint64_t x12, x13, x14, x15;
    uint64_t x16, x17, x18, x19;
    uint64_t x20, x21, x22, x23;
    uint64_t x24, x25, x26, x27;
    uint64_t x28, fp,  lr,  sp;
    uint64_t pc;
    uint32_t cpsr;
} Mach_thread_state;  // Mach_ARM64_thread_state
        enum e6 {
            THREAD_STATE = 4  // ARM64_THREAD_STATE
        };
        enum e7 {
            THREAD_STATE_COUNT = sizeof(Mach_thread_state)/4
        };
        enum e10 {
            LC_SEGMENT       = 0x19  // LC_SEGMENT_64
        };

#elif defined(__arm__)  //}{
typedef struct {
    uint32_t r[13];  // r0-r12
    uint32_t sp;  // r13
    uint32_t lr;  // r14
    uint32_t pc;  // r15
    uint32_t cpsr;
} Mach_thread_state;  // Mach_ARM_thead_state;
        enum e6 {
            THREAD_STATE = 1  // ARM_THREAD_STATE
        };
        enum e7 {
            THREAD_STATE_COUNT = sizeof(Mach_thread_state)/4
        };
        enum e10 {
            LC_SEGMENT       = 0x1
        };

#elif defined(__x86_64__)  //}{
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rdi, rsi, rbp, rsp;
    uint64_t  r8,  r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cs, fs, gs;
} Mach_thread_state;  // Mach_AMD64_thread_state;
        enum e6 {
            THREAD_STATE = 4  // AMD_THREAD_STATE
        };
        enum e7 {
            THREAD_STATE_COUNT = sizeof(Mach_thread_state)/4
        };
        enum e10 {
            LC_SEGMENT       = 0x19  // LC_SEGMENT_64
        };

#endif  //}

typedef struct {
    unsigned cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
    unsigned cmdsize;        /* total size of this command */
    unsigned flavor;
    unsigned count;          /* sizeof(following_thread_state)/4 */
    Mach_thread_state state;
} Mach_thread_command;

typedef union {
    unsigned offset;  /* from start of load command to string */
} Mach_lc_str;

#define MAP_FIXED     0x10
#define MAP_PRIVATE   0x02

#if SIMULATE_ON_LINUX_EABI4  //{
#define MAP_ANON  0x20  /* SIMULATE_ON_LINUX_EABI4 */
#else  //}{
#define MAP_ANON    0x1000  /* native darwin usual case */
#endif  //}
#define MAP_ANON_FD    -1

#define PROT_NONE      0
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4

extern void *mmap(void *, size_t, unsigned, unsigned, int, off_t_upx_stub);
ssize_t pread(int, void *, size_t, off_t_upx_stub);
extern void bswap(void *, unsigned);

typedef size_t Addr;  // this source file is used by 32-bit and 64-bit machines

// Find convex hull of PT_LOAD (the minimal interval which covers all PT_LOAD),
// and mmap that much, to be sure that a kernel using exec-shield-randomize
// won't place the first piece in a way that leaves no room for the rest.
static Addr // returns relocation constant
xfind_pages(
    Mach_header const *const mhdr,
    Mach_segment_command const *sc,
    int const ncmds,
    Addr addr
)
{
    Addr lo= ~(Addr)0, hi= 0;
    int j;
    unsigned mflags = ((mhdr->filetype == MH_DYLINKER || mhdr->flags & MH_PIE) ? 0 : MAP_FIXED);
    mflags += MAP_PRIVATE | MAP_ANON;  // '+' can optimize better than '|'
    DPRINTF("xfind_pages  mhdr=%%p  sc=%%p  ncmds=%%d  addr=%%p  mflags=%%x\\n",
        mhdr, sc, ncmds, addr, mflags);
    for (j=0; j < ncmds; ++j,
        (sc = (Mach_segment_command const *)((sc->cmdsize>>2) + (unsigned const *)sc))
    ) if (LC_SEGMENT==sc->cmd) {
        DPRINTF("  #%%d  cmd=%%x  cmdsize=%%x  vmaddr=%%p  vmsize==%%p  filesize=%%p  lo=%%p  mflags=%%x\\n",
            j, sc->cmd, sc->cmdsize, sc->vmaddr, sc->vmsize, sc->filesize, lo, mflags);
        if (sc->vmsize  // theoretically occupies address space
        &&  !(sc->vmaddr==0 && (MAP_FIXED & mflags))  // but ignore PAGEZERO when MAP_FIXED
        ) {
            if (mhdr->filetype == MH_DYLINKER  // /usr/lib/dyld
            &&  0==(1+ lo)  // 1st LC_SEGMENT
            &&  sc->vmaddr != 0  // non-floating address
            ) {
                // "pre-linked" dyld on MacOS 10.11.x El Capitan
                mflags |= MAP_FIXED;
            }
            if (lo > sc->vmaddr) {
                lo = sc->vmaddr;
            }
            if (hi < (sc->vmsize + sc->vmaddr)) {
                hi =  sc->vmsize + sc->vmaddr;
            }
        }
    }
    lo -= ~PAGE_MASK & lo;  // round down to page boundary
    hi  =  PAGE_MASK & (hi - lo - PAGE_MASK -1);  // page length
    DPRINTF("  addr=%%p  lo=%%p  len=%%p  mflags=%%x\\n", addr, lo, hi, mflags);
    if (MAP_FIXED & mflags) {
        addr = lo;
        int rv = munmap((void *)addr, hi);
        if (rv) {
            DPRINTF("munmap addr=%%p len=%%p, rv=%%x\\n", addr, hi, rv);
        }
    }
    addr = (Addr)mmap((void *)addr, hi, PROT_NONE, mflags, MAP_ANON_FD, 0);
    DPRINTF("  addr=%%p\\n", addr);
    if (~PAGE_MASK & addr) {
        err_exit(6);
    }
    return (Addr)(addr - lo);
}

unsigned * // &hatch if main; &Mach_thread_state if dyld
do_xmap(
    Mach_header *const mhdr,
    off_t_upx_stub const fat_offset,
    Extent *const xi,
    int const fdi,
    Mach_header **mhdrpp,
    f_expand *const f_exp,
    f_unfilter *const f_unf
)
{
    DPRINTF("do_xmap  fdi=%%x  mhdr=%%p  *mhdrpp=%%p  xi=%%p(%%x %%p) f_unf=%%p\\n",
        fdi, mhdr, (mhdrpp ? *mhdrpp : 0), xi, (xi? xi->size: 0), (xi? xi->buf: 0), f_unf);

    unsigned *rv = 0;
    Extent xi_orig = {0, 0};
    Mach_segment_command *sc = (Mach_segment_command *)(1+ mhdr);
    Addr const reloc = xfind_pages(mhdr, sc, mhdr->ncmds, 0);
    DPRINTF("do_xmap reloc=%%p\\n", reloc);
    unsigned j;
    if (xi) { // remember "Beginning of tape"
        xi_orig = *xi;
    }
    for ( j=0; j < mhdr->ncmds; ++j,
        (sc = (Mach_segment_command *)((sc->cmdsize>>2) + (unsigned *)sc))
    ) {
        DPRINTF("  #%%d  cmd=%%x  cmdsize=%%x  vmsize=%%x\\n",
                j, sc->cmd, sc->cmdsize, sc->vmsize);
        if (LC_SEGMENT==sc->cmd) {
            struct b_info h;
            if (xi && sc->filesize) { // Find the correct compressed block.
                xpeek(xi, (unsigned char *)&h, sizeof(h));
                DPRINTF("  h.b_extra=%%d  j=%%d\n", h.b_extra, j);
                if (h.b_extra != j) { // not the next one
                    *xi = xi_orig;  // rewind
                    for (;;) {
                        xpeek(xi, (unsigned char *)&h, sizeof(h));
                        if (h.b_extra == j) {
                            break;
                        }
                        xi->size -= sizeof(h) + h.sz_cpr;
                        xi->buf  += sizeof(h) + h.sz_cpr;
                    }
                }
            }
            if (!sc->vmsize) { // not mapped, such as __DWARF info for 'go'
                if (xi) {
                    DPRINTF("    0==.vmsize; skipping %%x\\n", h.sz_cpr);
                    xi->size -= sizeof(h) + h.sz_cpr;
                    xi->buf  += sizeof(h) + h.sz_cpr;
                }
                continue;  // redundant for clarity
            }
            else { // finally, some meat!
                Extent xo;
                size_t mlen = xo.size = sc->filesize;
                              xo.buf  = (void *)(reloc + sc->vmaddr);
                Addr  addr = (Addr)xo.buf;
                Addr haddr = sc->vmsize + addr;
                size_t frag = addr &~ PAGE_MASK;
                addr -= frag;
                mlen += frag;

                DPRINTF("    mlen=%%p  frag=%%p  addr=%%p\\n", mlen, frag, addr);
                if (0!=mlen) {
                    size_t const mlen3 = mlen
        #if defined(__x86_64__)  //{
                        // Decompressor can overrun the destination by 3 bytes.  [x86 only]
                        + (xi ? 3 : 0)
        #endif  //}
                        ;
                    unsigned const prot = VM_PROT_READ | VM_PROT_WRITE;
                    // MAP_FIXED: xfind_pages() reserved them, so use them!
                    unsigned const flags = MAP_FIXED | MAP_PRIVATE |
                                    ((xi || 0==sc->filesize) ? MAP_ANON    : 0);
                    int const fdm = ((xi || 0==sc->filesize) ? MAP_ANON_FD : fdi);
                    off_t_upx_stub const offset = sc->fileoff + fat_offset;

                    DPRINTF("mmap  addr=%%p  len=%%p  prot=%%x  flags=%%x  fd=%%d  off=%%p  reloc=%%p\\n",
                        addr, mlen3, prot, flags, fdm, offset, reloc);
                    {
                        Addr maddr = (Addr)mmap((void *)addr, mlen3, prot, flags, fdm, offset);
                        DPRINTF("maddr=%%p\\n", maddr);
                        if (maddr != addr) {
                            err_exit(8);
                        }
                        addr = maddr;
                    }
                    if (!*mhdrpp) { // MH_DYLINKER
                        *mhdrpp = (Mach_header*)addr;
                    }
                }
                if (xi && 0!=sc->filesize) {
                    if (0==sc->fileoff /*&& 0!=mhdrpp*/) {
                        *mhdrpp = (Mach_header *)(void *)addr;
                    }
                    unpackExtent(xi, &xo, f_exp, f_unf);
                }
                DPRINTF("xi=%%p  mlen=%%p  fileoff=%%p  nsects=%%d\\n",
                    xi, mlen, sc->fileoff, sc->nsects);
                if (xi && mlen && !sc->fileoff && sc->nsects) {
                    // main target __TEXT segment at beginning of file with sections (__text)
                    // Use upto 2 words of header padding for the escape hatch.
                    // fold.S could do this easier, except PROT_WRITE is missing then.
                    union {
                        unsigned char  *p0;
                        unsigned short *p1;
                        unsigned int   *p2;
                        unsigned long  *p3;
                    } u;
                    u.p0 = (unsigned char *)addr;
                    Mach_segment_command *segp = (Mach_segment_command *)((((char *)sc - (char *)mhdr)>>2) + u.p2);
                    Mach_section_command *const secp = (Mach_section_command *)(1+ segp);
        #if defined(__aarch64__)  //{
                    unsigned *hatch= -2+ (secp->offset>>2) + u.p2;
                    hatch[0] = 0xd4000001;  // svc #0  // syscall
                    hatch[1] = 0xd65f03c0;  // ret
        #elif defined(__arm__)  //}{
                    unsigned *hatch= -2+ (secp->offset>>2) + u.p2;
                    hatch[0] = 0xef000000;  // svc 0x0  // syscall
                    hatch[1] = 0xe12fff1e;  // bx lr
        #elif defined(__x86_64__)  //}{
                    unsigned *hatch= -1+ (secp->offset>>2) + u.p2;
                    hatch[0] = 0xc3050f90;  // nop; syscall; ret
        #endif  //}
                    DPRINTF("hatch=%%p  secp=%%p  segp=%%p  mhdr=%%p\\n", hatch, secp, segp, addr);
                    rv = hatch;
                }
                /*bzero(addr, frag);*/  // fragment at lo end
                frag = (-mlen) &~ PAGE_MASK;  // distance to next page boundary
                bzero((void *)(mlen+addr), frag);  // fragment at hi end
                if (0!=mlen && 0!=mprotect((void *)addr, mlen, sc->initprot)) {
                    err_exit(10);
        ERR_LAB
                }
                addr += mlen + frag;  /* page boundary on hi end */
                if (
        #if SIMULATE_ON_LINUX_EABI4  /*{*/
                    0!=addr &&
        #endif  /*}*/
                                addr < haddr) { // need pages for .bss
                    if (0!=addr && addr != (Addr)mmap((void *)addr, haddr - addr, sc->initprot,
                            MAP_FIXED | MAP_PRIVATE | MAP_ANON, MAP_ANON_FD, 0 ) ) {
                        err_exit(9);
                    }
                }
                else if (xi) { // cleanup if decompressor overrun crosses page boundary
                    mlen = ~PAGE_MASK & (3+ mlen);
                    if (mlen<=3) { // page fragment was overrun buffer only
                        DPRINTF("munmap  %%x  %%x\\n", addr, mlen);
                        munmap((char *)addr, mlen);
                    }
                }
            }
        }
        else if (!xi  // dyld
        && (LC_UNIXTHREAD==sc->cmd || LC_THREAD==sc->cmd)) {
            Mach_thread_command *const thrc = (Mach_thread_command *)sc;
            DPRINTF("thread_command= %%p\\n", sc);
            if (1
            // FIXME  THREAD_STATE      ==thrc->flavor
            //    &&  THREAD_STATE_COUNT==thrc->count
            ) {
                DPRINTF("thread_state= %%p  flavor=%%d  count=%%x  reloc=%%p\\n",
                    &thrc->state, thrc->flavor, thrc->count, reloc);
    #if defined(__aarch64__)  //{
                thrc->state.pc += reloc;
    #elif defined(__arm__)  //}{
                thrc->state.pc += reloc;
    #elif defined(__x86_64__)  //}{
                thrc->state.rip += reloc;
    #endif  //}
                rv = (unsigned *)&thrc->state;
            }
        }
    }
    DPRINTF("do_xmap= %%p\\n", rv);
    return rv;
}


/*************************************************************************
// upx_main - called by our unfolded entry code
//
**************************************************************************/
Mach_thread_state const *
upx_main(
    struct l_info const *const li,
    size_t volatile sz_compressed,  // total length
    Mach_header *const mhdr,  // temp char[sz_mhdr] for decompressing
    size_t const sz_mhdr,
    f_expand *const f_exp,
    f_unfilter *const f_unf,
    Mach_header **const mhdrpp  // Out: *mhdrpp= &real Mach_header
)
{
    Mach_thread_state *ts = 0;
    unsigned *hatch;
    off_t_upx_stub fat_offset = 0;
    Extent xi, xo, xi0;
    xi.buf  = CONST_CAST(unsigned char *, 1+ (struct p_info const *)(1+ li));  // &b_info
    xi.size = sz_compressed - (sizeof(struct l_info) + sizeof(struct p_info));
    xo.buf  = (unsigned char *)mhdr;
    xo.size = ((struct b_info const *)(void const *)xi.buf)->sz_unc;
    xi0 = xi;

    DPRINTF("upx_main szc=%%x  f_exp=%%p  f_unf=%%p  "
        "  xo=%%p(%%x %%p)  xi=%%p(%%x %%p)  mhdrpp=%%p  mhdrp=%%p\\n",
        sz_compressed, f_exp, f_unf, &xo, xo.size, xo.buf,
        &xi, xi.size, xi.buf, mhdrpp, *mhdrpp);

    // Uncompress Macho headers
    unpackExtent(&xi, &xo, f_exp, 0);  // never filtered?

    // Overwrite the OS-chosen address space at *mhdrpp.
    hatch = do_xmap(mhdr, fat_offset, &xi0, MAP_ANON_FD, mhdrpp, f_exp, f_unf);

  { // Map dyld dynamic loader
    Mach_load_command const *lc = (Mach_load_command const *)(1+ mhdr);
    unsigned j;

    for (j=0; j < mhdr->ncmds; ++j,
        (lc = (Mach_load_command const *)(lc->cmdsize + (void const *)lc))
    ) if (LC_LOAD_DYLINKER==lc->cmd) {
        char const *const dyld_name = ((Mach_lc_str const *)(1+ lc))->offset +
            (char const *)lc;
        DPRINTF("dyld= %%s\\n", dyld_name);
        int const fdi = open(dyld_name, O_RDONLY, 0);
        if (0 > fdi) {
            err_exit(18);
        }
fat:
        if ((ssize_t)sz_mhdr!=pread(fdi, (void *)mhdr, sz_mhdr, fat_offset)) {
ERR_LAB
            err_exit(19);
        }
        switch (mhdr->magic) {
        case MH_MAGIC: break;
        case MH_MAGIC64: break;
        case FAT_CIGAM:
        case FAT_MAGIC: {
            // stupid Apple: waste code and a page fault on EVERY execve
            Fat_header *const fh = (Fat_header *)mhdr;
            Fat_arch *fa = (Fat_arch *)(1+ fh);
            bswap(fh, sizeof(*fh) + (fh->nfat_arch>>24)*sizeof(*fa));
            for (j= 0; j < fh->nfat_arch; ++j, ++fa) {
                if (CPU_TYPE_AMD64==fa->cputype) {
                    fat_offset= fa->offset;
                    goto fat;
                }
            }
        } break;
        } // switch
        Mach_header *dyhdr = 0;
        ts = (Mach_thread_state *)do_xmap(mhdr, fat_offset, 0, fdi, &dyhdr, 0, 0);
            DPRINTF("ts= %%p  hatch=%%p\\n", ts, hatch);
#if defined(__aarch64__)  // {
            ts->x0 = (uint64_t)hatch;
#elif defined(__arm__)  //}{
            ts->r[0] = (uint32_t)hatch;
#elif defined(__x86_64__)  //}{
            ts->rax = (uint64_t)hatch;
#endif  //}
        close(fdi);
        break;
    }
  }

    return ts;
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
            n+= write(2, buf, heximal(va_arg(va, unsigned int), buf, 2));
        } break;
        case 's': {
            char *s0= (char *)va_arg(va, unsigned char *), *s= s0;
            if (s) while (*s) ++s;
            n+= write(2, s0, s - s0);
        } break;
        } // 'switch'
    }
done:
    va_end(va);
    return n;
}

#endif  //}

/* vim:set ts=4 sw=4 et: */

/*  mipsel-linux.shlib-init.S -- Linux Elf shared library init & decompressor
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2022 Laszlo Molnar
*  Copyright (C) 2000-2022 John F. Reiser
*  All Rights Reserved.
*
*  UPX and the UCL library are free software; you can redistribute them
*  and/or modify them under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; see the file COPYING.
*  If not, write to the Free Software Foundation, Inc.,
*  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Markus F.X.J. Oberhumer              Laszlo Molnar
*  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

NBPW= 4
#ifndef BIG_ENDIAN //{
#define BIG_ENDIAN 0
#endif  //}

#include "arch/mips/r3000/macros.ash"
#include "arch/mips/r3000/bits.ash"

        .set mips1
        .set noreorder
        .set noat
        .altmacro

sz_Elf32_Ehdr = 13*NBPW
sz_Elf32_Phdr =  8*NBPW

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE=       2
MAP_FIXED=     0x010
MAP_ANONYMOUS= 0x800  // ugh!!!

PAGE_SHIFT= 12
PAGE_MASK=  (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

a_type = 0*NBPW
a_val  = 1*NBPW
AT_NULL=   0
AT_PAGESZ= 6

__NR_Linux = 4000
__NR_exit     =   1+ __NR_Linux
__NR_read     =   3+ __NR_Linux
__NR_write    =   4+ __NR_Linux
__NR_open     =   5+ __NR_Linux
__NR_close    =   6+ __NR_Linux
__NR_mmap64   =  90+ __NR_Linux
__NR_munmap   =  91+ __NR_Linux
__NR_mprotect = 125+ __NR_Linux
__NR_cacheflush = 147+ __NR_Linux

/* asm/cachectl.h */
ICACHE= 1<<0
DCACHE= 1<<1

.macro do_sys n
        li v0,\n; syscall
.endm

#define r_esys a3

#define arg1 a0
#define arg2 a1
#define arg3 a2
#define arg4 a3
#define arg5 a4
#define arg6 a5

#define edi a0
#define esi a1
#define edx a2
#define ta3 a3
#define eax $8 /* a4 t0 */
#define ecx $9 /* a5 t1 */

#define UNFILTER 0  /* no unfilter for MIPS */

N_SLOTS= 0
sp_frame = 26 * NBPW
.macro slot symbol, n
  .ifnb n
    N_SLOTS = \n + N_SLOTS
  .else
    N_SLOTS =  1 + N_SLOTS
  .endif
  \symbol= sp_frame - N_SLOTS*NBPW
.endm

  section ELFMAINX
y_reloc= . - 4*NBPW  //  .long offset(.)  // detect relocation
u_dtini= . - 3*NBPW  //  .long offset(user DT_INIT)
e_hatch= . - 2*NBPW  //  .long offset(escape_hatch)
pb_info= . - 1*NBPW  //  .long offset({p_info; b_info; compressed data})

  slot f_my_ra
  slot f_fp
  slot f_uinit
  slot f_envp
  slot f_argv
  slot f_argc
  slot f_main
  slot f_fd

_start: .globl _start  // IN: jp= &_start
// If glibc: arg1= argc; arg2= argv; arg3= envp
// But musl: no args!
////    break  // for debugging
        addiu sp,sp,-sp_frame
        sw   ra,f_my_ra(sp)
        sw arg3,f_envp(sp)
        sw arg2,f_argv(sp)
        sw arg1,f_argc(sp)
        sw   fp,f_fp(sp)

        li v0,%lo(_start)
        subu jp,jp,v0
        addiu jp,jp,%lo(main)  // jp= &main
        sw   jp,f_main(sp)

// Determine PAGE_MASK
        bal 0f
          move a0,ra
        .asciz "/proc/self/auxv"
bad_open:
bad_read:
        li v1,1<<PAGE_SHIFT  // default
        b 9f
          negu fp,v1  // PAGE_MASK

BUFLEN= 40*2*NBPW
0:
        move a1,zero  // O_RDONLY
        do_sys __NR_open
        bnez a3,bad_open
          sw v0,f_fd(sp)  // fd

        addiu sp,sp,-BUFLEN
        li a2,BUFLEN  // buflen
        move a1,sp  // buffer
        move a0,v0  // fd
        do_sys __NR_read
        bltz a3,bad_read
          addu a2,sp,v0  // end-of-buffer

        move a3,sp  // buffer
0:  // Find AT_PAGESZ
        beq a3,a2,5f  // not found
          li v1,1<<PAGE_SHIFT  // default value
        lw v0,a_type(arg3)
        beqz v0,5f  // AT_NULL
          addiu v0,v0,-AT_PAGESZ
        bnez v0,0b
          addiu arg3,arg3,2*NBPW
        lw v1,-2*NBPW + a_val(arg3)  // non-default value
5:  // v1= PAGE_SIZE
        addiu sp,sp,BUFLEN
        negu fp,v1  // PAGE_MASK
        lw a0,f_fd(sp)
        do_sys __NR_close
9:
        lw jp,f_main(sp)
        jalr jp  // ra= &f_decompress
          nop
e_start:

/* Returns 0 on success; non-zero on failure. */
f_exp:  // alternate name
decompressor:  // (uchar const *lxsrc, size_t lxsrclen, uchar *lxdst, u32 &lxdstlen, uint method)
#define lxsrc    a0
#define lxsrclen a1
#define lxdst    a2
#define lxdstlen a3

#undef src  /* bits.ash */
#define src     lxsrc
#define lsrc    lxsrclen
#undef dst  /* bits.ash */
#define dst     lxdst
#define ldst    lxdstlen
#define meth     a4

        UCL_init    32,1,0
        decomp_done = eof
#include "arch/mips/r3000/nrv2e_d.ash"
  section NRV2E
        build nrv2e, full

#include "arch/mips/r3000/nrv2d_d.ash"
  section NRV2D
        build nrv2d, full

#include "arch/mips/r3000/nrv2b_d.ash"
  section NRV2B
        build nrv2b, full

section     LZMA_ELF00 # (a0=lxsrc, a1=lxsrclen, a2=lxdst, a3= &lxdstlen)

/* LzmaDecode(a0=CLzmaDecoderState *,
        a1=src, a2=srclen, a3=*psrcdone,
        dst, dstlen, *pdstdone);
struct CLzmaDecoderState {
        uchar lit_context_bits;
        uchar lit_pos_bits;
        uchar pos_bits;
        uchar unused;
        struct CProb[LZMA_BASE_SIZE + (LZMA_LIT_SIZE<<n)];
};
*/

LZMA_BASE_NUM = 1846
LZMA_LIT_NUM  =  768

lxlzma_szframe  = 12*NBPW
lxlzma_sv_pc    = 11*NBPW
lxlzma_sv_sp    = 10*NBPW
lxlzma_dst      =  9*NBPW
lxlzma_dstdone  =  8*NBPW
lxlzma_srcdone  =  7*NBPW
lxlzma_retval   = lxlzma_srcdone

#define a4 t0
#define a5 t1
#define a6 t2

        lbu t9,0(lxsrc)  # ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        li v1,-2*LZMA_LIT_NUM
        lbu t8,1(lxsrc)  # (lit_pos_bits<<4) | lit_context_bits
        andi v0,t9,7  # pos_bits
        srl t9,t9,3  # (lit_context_bits + lit_pos_bits)
        sllv v1,v1,t9
        addiu v1,v1,-4 - 2*LZMA_BASE_NUM - lxlzma_szframe
        addu sp,sp,v1  # alloca
                sw v1,lxlzma_sv_sp(sp)  # dynamic frame size
        addiu a6,sp,lxlzma_dstdone
                sw ra, lxlzma_sv_pc(sp)
        lw    a5,0(lxdstlen)
                sw lxdst,lxlzma_dst(sp)
        move  a4,lxdst
        addiu a3,sp,lxlzma_srcdone
        addiu a2,lxsrclen,-2  # 2 header bytes
        addiu a1,lxsrc,2  # 2 header bytes
        addiu a0,sp,lxlzma_szframe  # &CLzamDecoderState
        sb     v0,2(a0)   # pos_bits
        andi v1,t8,0xf
        sb   v1, 0(a0)  # lit_context_bits
        srl  t8,t8,4
        bal lzma_decode
          sb   t8,1(a0)   # lit_pos_bits

/* It seems that for our uses the icache does not need to be invalidated,
   because no lines from the destination have ever been fetched.  However,
   if the dcache is write-back, then some of the results might not be in
   memory yet, and the icache could fetch stale data; so memory must be
   updated from dcache.
   The *next* call of the decompressor will tend to sweep much of the dcache
   anyway, because the probability history array (typically ushort[7990] or
   ushort[14134]) gets initialized.
*/
        sw v0,lxlzma_retval(sp)  # return value from decompression

        lw a0,lxlzma_dst(sp)
        lw a1,lxlzma_dstdone(sp)
        li a2,ICACHE|DCACHE
        do_sys __NR_cacheflush

        lw v0,lxlzma_retval(sp)  # return value from decompression

        lw v1,lxlzma_sv_sp(sp)
        lw ra,lxlzma_sv_pc(sp)
/* Workaround suspected glibc bug: elf/rtld.c assumes uninit local is zero.
   2007-11-24 openembedded.org mipsel-linux 2.6.12.6/glibc 2.3.2
*/
        subu v1,sp,v1  # previous sp (un_alloca)
0:
        addiu sp,4
        bne sp,v1,0b
          sw $0,-4(sp)

        jr ra
          nop


lzma_decode:
  section LZMA_DEC20
#if 1  /*{*/
#include "arch/mips/r3000/lzma_d.S"
#else  /*}{*/
#include "arch/mips/r3000/lzma_d-mips3k.S"       /* gpp_inc:ignore=1: */
#endif  /*}*/


  section LZMA_DEC30
        break  // FIXME

  section NRV_HEAD
        addiu sp,-4
        sw ra,0(sp)
        add lxsrclen,lxsrclen,lxsrc  //  src_EOF
        sw lxdst,(lxdstlen)  // original lxdst in &lxdstlen

  section NRV_TAIL
eof:
        lw v1,(lxdstlen)  // original lxdst
        subu t8,lxsrc,lxsrclen  // new_src - src_EOF;  // return 0: good; else: bad
        lw ra,0(sp)
        sw t8,0(sp)

  section CFLUSH
        move a0,v1  // original lxdst
        subu a1,lxdst,v1  // actual length generated
          sw a1,(lxdstlen)
        li a2,ICACHE|DCACHE
        do_sys __NR_cacheflush

        lw v0,0(sp)
        jr ra
          addiu sp,4

  section ELFMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section ELFMAINZ
.macro lodslu  // eax= *esi++;  # 4 bytes unaligned, native endian
  .if BIG_ENDIAN
        lwl eax,0(esi)  // hi bytes
        lwr eax,3(esi)  // lo bytes
  .else  // LITTLE_ENDIAN
        lwr eax,0(esi)  // lo bytes
        lwl eax,3(esi)  // hi bytes
  .endif
        addiu esi,esi,NBPW
.endm

main:  // IN: ra= &e_start; jp= &main; fp= PAGE_MASK
//  1. allocate temporary pages
//  2. copy to temporary pages:
//       fragment of page below dst; compressed src;
//       decompress+unfilter; supervise
//  3. mmap destination pages for decompressed data
//  4. create escape hatch
//  5. jump to temporary pages
//  6. uncompress
//  7. unfilter
//  8. mprotect decompressed pages
//  9  setup args for unmap of temp pages
// 10. jump to escape hatch
// 11. unmap temporary pages
// 12. goto user DT_INIT

          subu jp,jp,ra  // length(f_exp)
        la ecx,y_reloc - e_start(ra)
          srl jp,jp,2  // n_words(f_exp)
        lw eax,y_reloc - e_start(ra); subu ecx,ecx,eax  // &Elf32_Ehdr of this
        lw eax,u_dtini - e_start(ra); addu eax,ecx,eax;   sw eax,f_uinit(sp)  // reloc DT_INIT  for step 12
  slot f_hatch
        lw eax,e_hatch - e_start(ra); addu eax,ecx,eax;   sw eax,f_hatch(sp)  // reloc &hatch   for step 10
        lw eax,pb_info - e_start(ra); addu edi,ecx,eax  // &l_info; also destination for decompress
        addiu esi,edi,sz_l_info + sz_p_info  // &b_info

  slot p_unmap, 2

        lw eax,sz_cpr(esi); addiu esi,esi,3*NBPW
        addu esi,esi,eax  // skip unpack helper block

        lodslu  // eax=dstlen
        and v0,edi,fp  // dst page
        subu ecx,edi,v0  // ecx= w_fragment below dst
  slot p_mprot,2
        addu eax,eax,ecx; sw eax,1*NBPW + p_mprot(sp)  // length to protect  step 8
        subu edi,edi,ecx; sw edi,0*NBPW + p_mprot(sp)  // base to protect
        subu eax,eax,ecx  // dstlen
        addu edi,edi,ecx  // dst
  slot f_wfrag
        srl ecx,ecx,2
        sh  ecx,f_wfrag(sp)  // w_fragment
f_wexpf= 2+ f_wfrag
          sh  jp,f_wexpf(sp)
  slot o_dstlen
        sw eax,o_dstlen(sp)

#if UNFILTER  //{
  slot f_unflt
  slot p_unflt,5
        sw edi,0*NBPW + p_unflt(sp)  // dst    param for unfilter  step 7
        sw eax,1*NBPW + p_unflt(sp)  // dstlen  also for unfilter  step 7
        lb v1,b_method-4+1(esi)
        sw v1,2*NBPW + p_unflt(sp)  // ftid
        lb v1,b_method-4+2(esi)
        sw v1,3*NBPW + p_unflt(sp)  // cto8
#endif  //} UNFILTER
        lodslu; move ecx,eax  // ecx= srclen
#if UNFILTER  //{
        lodslu; sw eax,4*NBPW + p_unflt(sp) // method,filter,cto,junk
#else  //}{
        addiu esi,esi,NBPW  // esi= &compressed
#endif  //} UNFILTER

  slot f_expf
         sw ra,f_expf(sp)

        la v1,o_dstlen(sp)  // &dstlen
  slot p_uncpr,4
        sw esi,0*NBPW + p_uncpr(sp)  // src; arglist ready for decompress  step 6
        sw ecx,1*NBPW + p_uncpr(sp)  // srclen
        sw edi,2*NBPW + p_uncpr(sp)  // dst
        sw v1,3*NBPW + p_uncpr(sp)  // &dstlen

        andi v1,esi,3  // length of prefix alignment
        addi ecx,ecx,3  // allow  suffix alignment
        add ecx,ecx,v1  // prefix increases byte length
        srl ecx,ecx,2
        lh v1,f_wfrag(sp); add edx,v1,ecx  // w_srclen + w_frag
        lh v1,f_wexpf(sp); add edx,v1,edx  // + n_words(f_exp)

#if UNFILTER  //{
        bal wlen_subr  // edx += n_words (f_unf)
          lw ta3,f_unflt(sp)
#endif  //} UNFILTER

  slot f_super
        bal L220
          sw ra,f_super(sp)
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        li arg4,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        lw arg2,1*NBPW+p_mprot(sp)  // dstlen
        bal mmapARW
          lw arg1,0*NBPW+p_mprot(sp)  // dst

        // Restore fragment of page below dst
        lh ecx,f_wfrag(sp)
        move edi,v0  // page_mask & dst
        bal movsl
          lw esi,p_unmap(sp)

        lw jp,f_expf(sp)
        lw arg4,3*NBPW + p_uncpr(sp)
        lw arg3,2*NBPW + p_uncpr(sp)
        lw arg2,1*NBPW + p_uncpr(sp)
        jalr jp  // decompress
          lw arg1,0*NBPW + p_uncpr(sp)

        bal L620
          lw eax,f_hatch(sp)
//hatch:
//  IN: v0= __NR_munmap; arg1= addr; arg2= len
//  IN: t0= argc; t1= argv; arg3= envp; jp= user DT_INIT; ra= ret.addr
        syscall
        move arg2,t1  // argv
        jr jp  // goto user DT_INIT
          move arg1,t0  // argc

L620:  // Implant escape hatch at end of .text
        lw v0,0*NBPW(ra); sw v0,0*NBPW(eax)
        lw v0,1*NBPW(ra); sw v0,1*NBPW(eax)
        lw v0,2*NBPW(ra); sw v0,2*NBPW(eax)
        lw v0,3*NBPW(ra); sw v0,3*NBPW(eax)

#if UNFILTER  //{
//p_unflt
        lw arg4,3*NBPW + p_unflt(sp)
        lw arg3,2*NBPW + p_unflt(sp)
        beqz arg4,0f  // 0==ftid ==> no filter
          lw jp,4*NBPW + p_unflt(sp)
        lw arg2,1*NBPW + p_unflt(sp)
        jalr jp  // unfilter
          lw arg1,0*NBPW + p_unflt(sp)
0:
#endif  //} UNFILTER

//p_mprot
        lw arg1,0*NBPW + p_mprot(sp)  // dst
        lw arg2,1*NBPW + p_mprot(sp)  // len
        li arg3,0
        addu  arg2,arg2,arg1  // last(dst)
        addiu arg2,arg2,2*NBPW  // len(hatch)
        do_sys __NR_cacheflush

        lw arg1,0*NBPW + p_mprot(sp)
        lw arg2,1*NBPW + p_mprot(sp)
        li arg3,PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

//p_unmap
        lw t8,f_hatch(sp)
        lw arg2,1*NBPW + p_unmap(sp)
        lw arg1,0*NBPW + p_unmap(sp)
          lw ra, f_my_ra(sp)
          lw fp,    f_fp(sp)
          lw jp, f_uinit(sp)
          lw arg3,f_envp(sp)
          lw t1,  f_argv(sp)
          lw t0,  f_argc(sp)
          addiu sp,sp,sp_frame
        jr t8  // goto hatch
          li v0,__NR_munmap

// Called by 'supervise', so must be before L220
movsl_subr:
        lw ecx,-2*NBPW(esi)  // 'bal <over>' instruction word
        sll ecx,ecx,16
        srl ecx,ecx,16  // word displ
        b movsl
          addiu ecx,ecx,-1  // displ includes delay slot

9:
        lw v0,0*NBPW(esi); addiu esi,esi,1*NBPW
        sw v0,0*NBPW(edi); addiu edi,edi,1*NBPW
        addiu ecx,ecx,-1
movsl:  // edi= 4-byte aligned dst; esi= 4-byte aligned src; ecx= *WORD* count
        andi v1,ecx,3; bnez v1,4+ 9b  //; nop  # same instr at 9b: and 9f:
9:
        lw v0,0*NBPW(esi); lw v1,1*NBPW(esi); lw t8,2*NBPW(esi); lw t9,3*NBPW(esi)
        sw v0,0*NBPW(edi); sw v1,1*NBPW(edi); sw t8,2*NBPW(edi); sw t9,3*NBPW(edi)
        addiu ecx,ecx,-4
          addiu esi,esi,4*NBPW
        bnez ecx,9b
          addiu edi,edi,4*NBPW
        jr ra; nop

mmapARW:
        li v0,-1; addiu sp,sp,-6*NBPW
        sw v0,  4*NBPW(sp)  // arg4: fd; cater to *BSD for MAP_ANON
        sw zero,5*NBPW(sp)  // arg6: off_t>>12
        li arg3,PROT_READ|PROT_WRITE
        do_sys __NR_mmap64; beqz a3,0f; nop; break; 0:
        jr ra; addiu sp,sp,6*NBPW

L220:
        move ta3,ra
        bal wlen_subr  // wlen_supervise
          nop

        // Allocate pages to hold temporary copy.
        sll arg2,edx,2  // convert to bytes
        li arg4,MAP_PRIVATE|MAP_ANONYMOUS
        sw arg2,1*NBPW + p_unmap(sp)  // length to unmap
        bal mmapARW
          li arg1,0  // any addr
        sw v0,0*NBPW + p_unmap(sp)  // address to unmap

        lw esi,0*NBPW + p_mprot(sp)
        move edi,v0  // edi= dst
        bal movsl  // copy the fragment
          lh ecx,f_wfrag(sp)  // w_fragment

        lw esi,0*NBPW + p_uncpr(sp)  // src
        lw ecx,1*NBPW + p_uncpr(sp)  // len
        andi v1,esi,3  // length of prefix alignment
        subu esi,esi,v1  // down to word aligned
        addu ecx,ecx,v1  // prefix increases byte length
        addu v1,edi,v1  // skip prefix at destination
        sw v1,0*NBPW + p_uncpr(sp)  // dst
        addi ecx,ecx,3  // round up to full words
        bal movsl  // copy all aligned words that contain compressed data
          srl ecx,ecx,2

        move edx,edi  // lo(dst) of copied code

        lh ecx,f_wexpf(sp)  // n_words
        lw esi,f_expf(sp)
        bal movsl  // copy decompressor
          sw edi,f_expf(sp)

#if UNFILTER  //{
        lw esi,f_unflt(sp)
        bal movsl_subr  // copy unfilter
          sw edi,f_unflt(sp)
#endif  //} UNFILTER

        lw esi,f_super(sp)
        bal movsl_subr  // copy supervisor
          sw edi,f_super(sp)

        move arg2,edi  //  &last of copied code
        move arg1,edx  // &first of copied code
        li arg3,0
        do_sys __NR_cacheflush

        lw jp,f_super(sp)
        jr jp
          nop

wlen_subr:  // edx+= nwords of inline subr at *ta3
        lw  ta3,-2*NBPW(ta3)  // 'bal <over>' instruction word
        sll ta3,ta3,16
        srl ta3,ta3,16
        addiu ta3,ta3,-1  // displ includes delay slot
        jr ra
          addu edx,edx,ta3

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */

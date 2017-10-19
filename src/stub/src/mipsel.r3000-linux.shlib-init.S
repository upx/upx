/*  mipsel-linux.shlib-init.S -- Linux Elf shared library init & decompressor
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2017 Laszlo Molnar
*  Copyright (C) 2000-2017 John F. Reiser
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

#ifndef BIG_ENDIAN //{
#define BIG_ENDIAN 0
#endif  //}

#include "arch/mips/r3000/macros.ash"
#include "arch/mips/r3000/bits.ash"

        .set mips1
        .set noreorder
        .set noat
        .altmacro

#deifne PUSH1(a)           addiu sp,sp,-1*4; sw a,0(sp)
#define PUSH4(a,b,c,d)     addiu sp,sp,-4*4; sw a,0(sp); sw b,1*4(sp); sw c,2*4(sp); sw d,3*4(sp)
#define PUSH6(a,b,c,d,e,f) addiu sp,sp,-6*4; sw a,0(sp); sw b,1*4(sp); sw c,2*4(sp); sw d,3*4(sp); sw e,4*4(sp); sw f,5*4(sp)
#define POP1(a)           lw a,0*4(sp);                                                                       addiu sp,sp,1*4
#define POP2(a,b)         lw a,0*4(sp); lw b,1*4(sp);                                                         addiu sp,sp,2*4
#define POP3(a,b,c)       lw a,0*4(sp); lw b,1*4(sp); lw c,2*4(sp);                                           addiu sp,sp,3*4
#define POP5(a,b,c,d,e)   lw a,0*4(sp); lw b,1*4(sp); lw c,2*4(sp); lw d,3*4(sp); lw e,4*4(sp);               addiu sp,sp,5*4
#define POP6(a,b,c,d,e,f) lw a,0*4(sp); lw b,1*4(sp); lw c,2*4(sp); lw d,3*4(sp); lw e,4*4(sp); lw f,5*4(sp); addiu sp,sp,6*4

sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 12
PAGE_MASK=  (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

__NR_Linux = 4000
__NR_exit     =   1+ __NR_Linux
__NR_write    =   4+ __NR_Linux
__NR_mmap64   =  90+ __NR_Linux
__NR_munmap   =  91+ __NR_Linux
__NR_mprotect = 125+ __NR_Linux
__NR_cacheflush = 147 + __NR_Linux

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
#define eax $8 /* a4 */
#define ecx $9 /* a5 */

#define lr ra

  section ELFMAINX
y_reloc= . - 4*4  //  .long offset(.)  // detect relocation
u_dtini= . - 3*4  //  .long offset(user DT_INIT)
e_hatch= . - 2*4  //  .long offset(escape_hatch)
pb_info= . - 1*4  //  .long offset({p_info; b_info; compressed data})

_start: .globl _start
    break  // for debugging
        PUSH6(arg1,arg2,arg3,fp,lr,ecx)
o_uinit= 5*4  // slot
        bltzal $0,9f  # ra= &9f; no branch (condition is false!)
          li v0,%lo(9f)
9:
        subu v0,ra,v0
        addiu v0,v0,%lo(main)
        jalr v0  // ra= &f_decompress
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

lxlzma_szframe  = 12*4
lxlzma_sv_pc    = 11*4
lxlzma_sv_sp    = 10*4
lxlzma_dst      =  9*4
lxlzma_dstdone  =  8*4
lxlzma_srcdone  =  7*4
lxlzma_retval   = lxlzma_srcdone

#define a4 t0
#define a5 t1
#define a6 t2

        lbu t9,0(lxsrc)  # ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        li ta3,-2*LZMA_LIT_NUM
        lbu t8,1(lxsrc)  # (lit_pos_bits<<4) | lit_context_bits
        andi v0,t9,7  # pos_bits
        srl t9,t9,3  # (lit_context_bits + lit_pos_bits)
        sllv ta3,ta3,t9
        addiu ta3,ta3,-4 - 2*LZMA_BASE_NUM - lxlzma_szframe
        addu sp,sp,ta3  # alloca
                sw ta3,lxlzma_sv_sp(sp)  # dynamic frame size
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
        andi ta3,t8,0xf
        sb   ta3, 0(a0)  # lit_context_bits
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
        li v0,__NR_cacheflush; syscall

        lw v0,lxlzma_retval(sp)  # return value from decompression

        lw ta3,lxlzma_sv_sp(sp)
        lw ra,lxlzma_sv_pc(sp)
/* Workaround suspected glibc bug: elf/rtld.c assumes uninit local is zero.
   2007-11-24 openembedded.org mipsel-linux 2.6.12.6/glibc 2.3.2
*/
        subu ta3,sp,ta3  # previous sp (un_alloca)
0:
        addiu sp,4
        bne sp,ta3,0b
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
        li v0,__NR_cacheflush; syscall

        lw v0,0(sp)
        jr ra
          addiu sp,4

  section ELFMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section ELFMAINZ
.macro push reg
        addiu sp,sp,-4
        sw \reg,0(sp)
.endm
.macro pop reg
        lw \reg,0(sp)
        addiu sp,sp,4
.endm

#define lodslu bal get4u; nop

main:
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

        addiu ecx,lr,y_reloc - e_start
        lw eax,y_reloc - e_start(lr); subu ecx,ecx,eax; //sw ecx,o_reloc(fp)
        lw eax,u_dtini - e_start(lr); addu eax,ecx,eax;   sw eax,o_uinit(fp)  // reloc DT_INIT  for step 12
        lw eax,e_hatch - e_start(lr); addu eax,ecx,eax; push eax               // reloc &hatch   for step 10
o_hatch= -1*4
        lw eax,pb_info - e_start(lr); addu edi,ecx,eax  // &l_info; also destination for decompress
        addiu esi,edi,sz_l_info + sz_p_info  // &b_info

        addiu sp,sp,-2*4  // param space: munmap temp pages  step 9
p_unmap= -3*4

        lw eax,sz_cpr(esi); addiu esi,esi,3*4
        addu esi,esi,eax  // skip unpack helper block

        lodslu  // eax=dstlen
        sll ecx,edi,(32-PAGE_SHIFT)
        srl ecx,ecx,(32-PAGE_SHIFT)  // ecx= w_fragment
        addu eax,eax,ecx; push eax  // params: mprotect restored pages  step 8
        subu edi,edi,ecx; push edi
p_mprot= -5*4
        subu eax,eax,ecx  // dstlen
        addu edi,edi,ecx  // dst
        push ecx  // w_fragment
o_wfrag= -6*4

        addiu sp,sp,-4  // FIXME
o_unflt= -7*4

        lb ta3,b_method-4+1(esi); push ta3  // ftid
        lb ta3,b_method-4+2(esi); push ta3  // cto8
        push eax  // dstlen  also for unfilter  step 7
        push edi  // dst    param for unfilter  step 7
p_unflt= -11*4

        lodslu; move ecx,eax  // ecx= srclen
        lodslu; push eax  // method,filter,cto,junk
        push lr  // &decompress
o_uncpr= -13*4
        addiu ta3,fp,p_unflt+1*4  // &dstlen
        PUSH4(esi,ecx,edi,ta3)  // src, srclen, dst, &dstlen;  arglist ready for decompress  step 6
p_uncpr= -17*4

        andi ta3,esi,3  // length of prefix alignment
        addi ecx,ecx,3  // allow  suffix alignment
        add ecx,ecx,ta3  // prefix increases byte length
        srl ecx,ecx,2
        lw ta3,o_wfrag(fp); add edx,ta3,ecx  // w_srclen + w_frag
        bal wlen_subr
          lw ta3,o_uncpr(fp)
        bal wlen_subr
          lw ta3,o_unflt(fp)

        bal L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        li arg5,-1  // cater to *BSD for fd of MAP_ANON
        li arg4,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        li arg3,PROT_READ|PROT_WRITE
        lw arg2,p_mprot+4(fp)  // dstlen
        lw arg1,p_mprot  (fp)  // dst
        move arg6,arg1  // required result
        do_sys __NR_mmap64; subu ta3,v0,arg6; beqz ta3,0f; break; 0:

        // Restore fragment of page below dst
        lw ecx,o_wfrag(fp)
        //move edi,r0  // NOP: edi==r0
        bal movsl
          lw esi,p_unmap(fp)

        POP5(arg1,arg2,arg3,arg4,eax)
        jalr eax  // decompress
          nop
        addiu sp,sp,4  // toss arg5

        bal L620
//hatch:
        do_sys __NR_munmap
        POP5(arg1,arg3,arg3,fp,lr)
        jalr ra
          nop

L620:  // Implant escape hatch at end of .text
        lw eax,o_hatch(fp)
        POP2(arg1,arg2)
        sw arg1,0(eax)
        sw arg2,4(eax)

//p_unflt
        POP6(arg1,arg2,arg3,arg4,eax,ecx)  // ecx=w_fragment [toss]
        beqz arg4,0f  // 0==ftid ==> no filter
          nop
        jalr eax  // unfilter
          nop
0:
//p_mprot
        lw arg1,0*4(sp)  // lo(dst)
        lw arg2,1*4(sp)  // len
        li arg3,0
        addu  arg2,arg2,arg1  // hi(dst)
        addiu arg2,arg2,2*4  // len(hatch)
        do_sys __NR_cacheflush

        POP2(arg1,arg2)
        li arg3,PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

//p_unmap
        POP3(arg1,arg2,arg3)
        jr arg3  // goto hatch

movsl_subr:
        lw ecx,-4(esi)  // 'bal <over>' instruction word
        sll ecx,ecx,16
        srl ecx,ecx,16  // displ
        addiu ecx,ecx,-1  // displ includes delay slot
// FALL THROUGH to the part of 'movsl' that trims to a multiple of 4 words.
9:
        lw v0,0*4(esi); addiu esi,esi,1*4
        sw v0,0*4(edi); addiu edi,edi,1*4
        addiu ecx,ecx,-1
movsl:  // edi= 4-byte aligned dst; esi= 4-byte aligned src; ecx= word count
        andi ta3,ecx,3; bnez ta3,4+ 9b  //; nop  # same instr at 9b: and 7f:
7:
        lw v0,0*4(esi); lw v1,1*4(esi); lw t0,2*4(esi); lw t1,3*4(esi)
        sw v0,0*4(edi); sw v1,1*4(edi); sw t0,2*4(edi); sw t1,3*4(edi)
        addiu ecx,ecx,-4
          addiu esi,esi,4*4
        bnez ecx,7b
          addiu edi,edi,4*4
9:
        jr ra; nop

L220:
        addiu sp,sp,-1*4; sw ra,0(sp)  // PUSH1(ra)
o_super= -18*4
        move ta3,lr
        bal wlen_subr  // wlen_supervise
          nop
        sll arg2,edx,2  // convert to bytes

        // Allocate pages to hold temporary copy.
        li arg5,-1  // cater to *BSD for fd of MAP_ANON
        li arg4,MAP_PRIVATE|MAP_ANONYMOUS
        li arg3,PROT_READ|PROT_WRITE|PROT_EXEC
        sw arg2,p_unmap+1*4(fp)  // length to unmap
        li arg1,0  // any addr
        do_sys __NR_mmap64; beqz r_esys,0f; break; 0:
        sw v0,p_unmap+0*4(fp)  // address to unmap

        lw esi,p_mprot(fp)
        //move edi,r0  // edi= dst  NOP: edi==r0
        bal movsl  // copy the fragment
          lw ecx,o_wfrag(fp)  // w_fragment

        lw esi,p_uncpr+0*4(fp)  // src
        lw ecx,p_uncpr+1*4(fp)  // len
        andi ta3,esi,3  // length of prefix alignment
        subu esi,esi,ta3  // down to word aligned
        addu ecx,ecx,ta3  // prefix increases byte length
        addu ta3,ta3,edi // skip prefix at destination
        sw ta3,p_uncpr+0*4(fp)  // dst
        addi ecx,ecx,3  // round up to full words
        bal movsl  // copy all aligned words that contain compressed data
          srl ecx,ecx,2

        move edx,edi  // lo(dst) of copied code

        lw esi,o_uncpr(fp)
        bal movsl_subr  // copy decompressor
          sw edi,o_uncpr(fp)

        lw esi,o_unflt(fp)
        bal movsl_subr  // copy unfilter
          sw edi,o_unflt(fp)

        POP2(esi,edi)  // &supervise, &copied
        bal movsl_subr  // copy supervisor
          nop

        move arg2,edi  // hi(dst) of copied code
        move arg1,edx  // lo(dst) of copied code
        li arg3,0
        do_sys __NR_cacheflush

        POP1(ra)
        jr ra
          nop

get4u:
        lb eax,0(esi)
        lb ta3,1(esi); sll ta3,ta3,1*8; or eax,eax,ta3
        lb ta3,2(esi); sll ta3,ta3,2*8; or eax,eax,ta3
        lb ta3,3(esi); sll ta3,ta3,3*8; or eax,eax,ta3
        jr ra
          addiu esi,esi,4

wlen_subr:  // edx+= nwords of inline subr at *ta3
        lw  ta3,-4(ta3)  // 'bal <over>' instruction word
        sll ta3,ta3,16
        srl ta3,ta3,16
        addiu ta3,ta3,-1  // displ includes delay slot
        jr ra
          addu edx,edx,ta3

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */

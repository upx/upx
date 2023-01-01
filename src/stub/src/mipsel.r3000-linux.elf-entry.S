/*  mipsel-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
*  Copyright (C) 2000-2023 John F. Reiser
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
#include "arch/mips/r3000/macros.ash"
#include "arch/mips/r3000/bits.ash"

sp_frame= 0x20
  // These are local temporaries.
F_ADRM=  2*NBPW
F_LENM=  3*NBPW

  // These are passed on stack to unfolded code.
F_PMASK= 4*NBPW
F_fd=    5*NBPW
F_ADRU=  6*NBPW
F_LENU=  7*NBPW
  // r_PMASK still is used here

// C-language uses 8 args in registers
a5=        9
a4=        8

//ra             31
#define r_fexp   30  /* s8 */
//sp             29  /* hardware */
#define r_PMASK  28  /* gp */
//k1             27  /* trashed by syscall */
//k0             26  /* trashed by syscall */
//t9, jp         25  /* trashed by syscall ? */
//t8             24  /* trashed by syscall ? */
#define r_fd     23  /* s7 */
#define r_auxv   22  /* s6 */
#define r_elfa   21  /* s5 */
#define r_FOLD   20  /* s4 */
#define r_szuf   19  /* s3 */
#define r_relo   18  /* s2 */
#define r_LENX   17  /* s1 */
#define r_ADRX   16  /* s0 */
#define r_LENU   r_LENX

        .set mips1
        .set noreorder
        .set noat
        .altmacro

sz_Ehdr= 52
sz_Phdr= 32

sz_l_info= 12
  l_lsize= 8

sz_p_info= 12

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
  b_ftid=   9
  b_cto8=  10
  b_unused=11

AT_NULL= 0  // <elf.h>
AT_PAGESZ= 6
a_type= 0
a_val=  NBPW
sz_auxv= 2*NBPW

O_RDONLY= 0

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE=        2
MAP_FIXED=       0x10
MAP_ANONYMOUS=  0x800

PAGE_SHIFT= 12  // default only
PAGE_SIZE = -(~0<<PAGE_SHIFT)

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

/* These from /usr/include/asm/unistd.h */
__NR_Linux = 4000
__NR_exit     =   1+ __NR_Linux
__NR_mmap     =  90+ __NR_Linux
__NR_mprotect = 125+ __NR_Linux
__NR_open     =   5+ __NR_Linux
__NR_write    =   4+ __NR_Linux
__NR_cacheflush = 147+ __NR_Linux

/* asm/cachectl.h */
ICACHE= 1<<0
DCACHE= 1<<1

//BAL=0x04110000

  section ELFMAINX
sz_pack2 = . - 4

_start: .globl _start
////    break  # debug only
        bal main
          addiu $r_fexp,ra, f_exp - 0f
0:

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
        li tmp,-2*LZMA_LIT_NUM
        lbu t8,1(lxsrc)  # (lit_pos_bits<<4) | lit_context_bits
        andi v0,t9,7  # pos_bits
        srl t9,t9,3  # (lit_context_bits + lit_pos_bits)
        sllv tmp,tmp,t9
        addiu tmp,tmp,-4 - 2*LZMA_BASE_NUM - lxlzma_szframe
        addu sp,sp,tmp  # alloca
                sw tmp,lxlzma_sv_sp(sp)  # dynamic frame size
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
        andi tmp,t8,0xf
        sb   tmp, 0(a0)  # lit_context_bits
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

        lw tmp,lxlzma_sv_sp(sp)
        lw ra,lxlzma_sv_pc(sp)
/* Workaround suspected glibc bug: elf/rtld.c assumes uninit local is zero.
   2007-11-24 openembedded.org mipsel-linux 2.6.12.6/glibc 2.3.2
*/
        subu tmp,sp,tmp  # previous sp (un_alloca)
0:
        addiu sp,4
        bne sp,tmp,0b
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
msg_SELinux:
        addiu a2,zero,L71 - L70  // length
        bal L72
          move a1,ra
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        // IDENTSTR goes here

  section ELFMAINZ
L72:
        li a0,2  // fd stderr
        li v0,__NR_write; syscall
die:
        li a0,127
        li v0,__NR_exit; syscall

unfold:  // IN: $r_fexp,$r_auxv,$r_PMASK,$r_FOLD
        addiu sp,sp,-sp_frame
        sw $r_PMASK,F_PMASK(sp)

// Reserve enough space to decompress the folded code of the stub
        lw $r_LENX,sz_pack2 - f_exp($r_fexp)  // length before stub
        la $r_elfa,sz_pack2 - f_exp($r_fexp)
          li a4,-1
        lw $r_szuf,sz_unc($r_FOLD)  // sz_unc of fold
          li a3,MAP_PRIVATE|MAP_ANONYMOUS
        sub $r_elfa,$r_elfa,$r_LENX  // $r_elfa= &Elf32_Ehdr of this stub
          li a0,0  // kenrel chooses addr
        move a1,$r_szuf
        bal mmapRW0
          sw a1,F_LENU(sp)
        sw v0,  F_ADRU(sp)

        lw $r_ADRX,-4($r_FOLD)  // O_BINFO
// Decompress folded code
        lb meth,b_method($r_FOLD)  // a4
        sw $r_szuf,0(sp)  // lzma uses for EOF
        move ldst,sp  // a3  &slot on stack
        move dst,v0  // a2  dst for unfolding
        lw lsrc,sz_cpr($r_FOLD)  // a1
        jalr $r_fexp  // decompress it
          addiu src,$r_FOLD,sz_b_info  // a0  folded code

// Generate code to compute PAGE_MASK.
        lw a0,F_ADRU(sp)
          srl v1,$r_PMASK,9
        lw v0,0(a0)  // "li v0,0"
        andi v1,v1,0xffff  // 0xffff is NOT sign-extended
        or v0,v0,v1  // replace immediate constant
        sw v0,0(a0)

// PROT_EXEC
        li a2,PROT_EXEC|PROT_READ
        lw a1,F_LENU(sp)  // length
      //lw a0,F_ADRU(sp)  // base
        li v0,__NR_mprotect; syscall

// Use the unfolded code
        lw ra,F_ADRU(sp)
          //nop  r_LENX
        addi ra,ra,4*4  // jmp over get_page_mask()
        jr ra
          addu $r_ADRX,$r_elfa,$r_ADRX  // compressed data

mmapRW0:
        li a5,0  // offset
mmapRW:
        li a2,PROT_WRITE|PROT_READ
mmap:
        addiu sp,sp,-sp_frame
a4_sys=  4*NBPW  // temporary for syscall
a5_sys=  5*NBPW  // temporary for syscall
        sw a4,a4_sys(sp)
        sw a5,a5_sys(sp)
        li v0,__NR_mmap; syscall
        bnez a3,mmap_bad
          nop
        jr ra
          addiu sp,sp, sp_frame
mmap_bad:
        break  // mmap_bad

zfind:  // result in $r_auxv
        lw v1,(a0); addiu a0,a0,NBPW
        bnez v1,zfind
          move $r_auxv,a0
        jr ra
          li t0,AT_PAGESZ  // prepare early

main:
        bal zfind
          move a0,sp
        bal zfind
          move a0,$r_auxv

// set $r_PMASK by finding actual page size in Elf32_auxv_t
1:
        lw  v1,a_type(a0)
          addiu a0,a0,sz_auxv
        beq v1,t0,2f  // AT_PAGESZ
          lw v0,a_val - sz_auxv(a0)
        bnez v1,1b  // AT_NULL
          li v0,PAGE_SIZE
2:
        sub $r_PMASK,zero,v0
        bal unfold
          addiu $r_FOLD,ra,LrFLD - 0f  // &b_info for folded loader
0:
        .long O_BINFO
LrFLD:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */

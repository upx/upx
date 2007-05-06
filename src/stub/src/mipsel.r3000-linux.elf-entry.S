/*  mipsel-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2007 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2007 Laszlo Molnar
*  Copyright (C) 2000-2007 John F. Reiser
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
*  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/mips/mipsel.r3000/macros.ash"
#include "arch/mips/mipsel.r3000/bits.ash"

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

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE=        2
MAP_FIXED=       0x10
MAP_ANONYMOUS=  0x800

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

//BAL=0x04110000

  section ELFMAINX
sz_pack2 = . - 4
adrm:   .long ADRM
lenm:   .long LENM
jmpu:   .long JMPU
adru:   .long ADRU
adrc:   .long ADRC
lenu:   .long LENU
adrx:   .long ADRX
lenx:   .long LENX
_start: .globl _start
        break
        bal main
          addiu s7,ra, decompressor - 0f  // s7= &decompress
0:

/* Returns 0 on success; non-zero on failure. */
decompressor:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)

#include "arch/mips/mipsel.r3000/nrv2e_d.ash"
  section NRV2E
        nrv2e eof

#include "arch/mips/mipsel.r3000/nrv2d_d.ash"
  section NRV2D
        nrv2d eof

#include "arch/mips/mipsel.r3000/nrv2b_d.ash"
  section NRV2B
        nrv2b eof

section     LZMA_ELF00 # (a0=lxsrc, a1=lxsrclen, a2=lxdst, a3= &lxdstlen)
#define lxsrc    a0
#define lxsrclen a1
#define lxdst    a2
#define lxdstlen a3

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
        /* 9 */
lxlzma_dstdone  =  8*4
lxlzma_srcdone  =  7*4

lxlzma_pdstdone =  6*4
lxlzma_dstlen   =  5*4
lxlzma_dst      =  4*4

        lbu t0,0(lxsrc)  # ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        li tmp,-2*LZMA_LIT_NUM
        lbu t1,1(lxsrc)  # (lit_pos_bits<<4) | lit_context_bits
        andi v0,t0,7  # pos_bits
        srl t0,t0,3  # (lit_context_bits + lit_pos_bits)
        sllv tmp,tmp,t0
        addiu tmp,tmp,-4 - 2*LZMA_BASE_NUM - lxlzma_szframe
        addu sp,sp,tmp  # alloca
        sw tmp,   lxlzma_sv_sp(sp)  # dynamic frame size
        addiu tmp,sp,lxlzma_dstdone
        sw lxdstlen,lxlzma_dstlen(sp)
        sw lxdst,   lxlzma_dst(sp)
        addiu a3,sp,lxlzma_srcdone
        sw ra,lxlzma_sv_pc(sp)
        addiu a2,lxsrclen,-2  # 2 header bytes
        sw tmp,     lxlzma_pdstdone(sp)
        addiu a1,lxsrc,2  # 2 header bytes
        addiu a0,sp,lxlzma_szframe  # &CLzamDecoderState
        sb   v0,2(a0)   # pos_bits
        andi tmp,t1,0xf
        sb   tmp,0(a0)  # lit_context_bits
        srl  t1,t1,4
        bal lzma_decode
          sb   t1,1(a0)   # lit_pos_bits
        lw ra,lxlzma_sv_pc(sp)
        lw tmp,lxlzma_sv_sp(sp)
        jr ra
          subu sp,sp,tmp  # un_alloca

lzma_decode:
#include "arch/mips/mipsel.r3000/lzma_d.S"

  section LZMA_DEC30
        break  // FIXME

  section NRV_TAIL
        // empty

.macro pop reg
        lw \reg,0(sp)
          addiu sp,sp,4
.endm

  section ELFMAINY
eof:
        pop t0; sub v0,v0,t0  // src -= eof;  // return 0: good; else: bad
        pop t0; sub s0,s0,t0  // dst -= original_dst
        pop t0; sw s0,(t0)  // actual length used t0 dst
        pop t0; jr t0; nop

/* These from /usr/include/asm/unistd.h */
__NR_Linux = 4000
__NR_write = 4+ __NR_Linux
__NR_exit  = 1+ __NR_Linux
__NR_mmap  = 90+ __NR_Linux

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
        li v0,__NR_write
        syscall
die:
        li a0,127
        li v0,__NR_exit
        syscall

/* Decompress the rest of this loader, and jump to it.
   Map a page to hold the decompressed bytes.  Logically this could
   be done by setting .p_memsz for our first PT_LOAD.  But as of 2005-11-09,
   linux 2.6.14 only does ".bss expansion" on the PT_LOAD that describes the
   highest address.  [I regard this as a bug, and it makes the kernel's
   fs/binfmt_elf.c complicated, buggy, and insecure.]  For us, that is the 2nd
   PT_LOAD, which is the only way that linux allows to set the brk() for the
   uncompressed program.  [This is a significant kernel misfeature.]
*/
unfold:  // s7= &decompress; s6= &b_info(fold); s5= sz_pack2
        // compute s5= &decompressor - &b_info(user)
        addiu s5,s5,(decompressor - sz_pack2) - (sz_Ehdr + 2*sz_Phdr + sz_l_info + sz_p_info)

/* Get some pages.  If small, then get 1 page located just after the end
   of the first PT_LOAD of the compressed program.  This will still be below
   all of the uncompressed program.  If large (>=3MB uncompressed), then enough
   to duplicate the entire compressed PT_LOAD, plus 1 page, located just after
   the brk() of the _un_compressed program.  The address and length are pre-
   calculated by PackLinuxElf64amd::defineSymbols().
*/
#define a4 0x10
#define a5 0x14
#define sp_frame 0x20

        addiu sp,sp,-sp_frame
        lw a0,adrm - decompressor(s7)
        li a2, PROT_READ | PROT_WRITE | PROT_EXEC
        lw a1,lenm - decompressor(s7)
        li a3, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
        sw zero,a4(sp)  //; sw zero,a5_mmap(sp)  // MAP_ANON ==> ignore offset
        li v0,__NR_mmap; syscall; bnez a3,err_syscall
        move a0,v0  // &new page(s)

        lw a1,adrc - decompressor(s7)
        lw s4,jmpu - decompressor(s7)  // for unmap in fold
        lw s3,adru - decompressor(s7)  // for unmap in fold
        lw s2,lenu - decompressor(s7)  // for unmap in fold
        lw s1,adrx - decompressor(s7)  // for upx_main

        beq a0,a1,L80  // no copy
          subu t0,a0,a1  // (ADRM - ADRC) == relocation amount
        addu s7,s7,t0  // update &decompress
        move a2,s5  // length
move_up:  // memcpy(dst=a0, src=a1, len=round_up(a2, 8))
        lw t0, 0(a1); addiu a0,a0, 8
        lw t1, 4(a1); addiu a2,a2,-8
        sw t0,-8(a0); addiu a1,a1, 8
        bgtz a2,move_up
          sw t1,-4(a0)
L80:
// Decompress the folded part of this stub, then execute it.
        lw t0,sz_unc(s6); move a3,sp; sw t0,0(sp)  // &dst_len
        move a2,v0  // &dst
        lw a1,sz_cpr(s6)  // src_len
        addiu a0,s6,sz_b_info
        jr s7  // decompress(src, srclen, dst, &dstlen /*, method*/)
          move ra,a2  // return to decompressed code

err_syscall:
        b err_syscall
          nop
main:
        lw s5,sz_pack2 - decompressor(s7)  // length before stub
        bal unfold
          move s6,ra
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
 */


/*  mipsel-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2024 Laszlo Molnar
*  Copyright (C) 2000-2024 John F. Reiser
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

sp_frame= 8*NBPW  # first 4 are ??
  # These are passed on stack to unfolded code.
F_PMASK= 4*NBPW
F_SZPK2= 5*NBPW; F_EOF= F_SZPK2
F_ADRU=  6*NBPW
F_LENU=  7*NBPW
  # r_PMASK still is used here

# C-language uses 8 args in registers
#define a5       9
#define a4       8

#ra             31
#define r_fexp   30  /* s8 */
#sp             29  /* hardware */
#define r_PMASK  28  /* gp */
#k1             27  /* trashed by syscall */
#k0             26  /* trashed by syscall */
#t9, jp         25  /* trashed by syscall ? */
#t8             24  /* trashed by syscall ? */
#define r_mfd    23  /* s7 */
#define r_auxv   22  /* s6 */
#define r_elfa   21  /* s5 */
#define r_FOLD   20  /* s4 */
#define r_szuf   19  /* s3 */
//#define r_relo   18  /* s2 */
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
#define b_method 8
  b_ftid=   9
  b_cto8=  10
  b_unused=11

AT_NULL= 0  # <elf.h>
AT_PAGESZ= 6
a_type= 0
a_val=  NBPW
sz_auxv= 2*NBPW

O_RDONLY= 0

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_SHARED=         1
MAP_PRIVATE=        2
MAP_FIXED=       0x10
MAP_ANONYMOUS=  0x800

PAGE_SHIFT= 12  # default only
PAGE_SIZE = -(~0<<PAGE_SHIFT)

M_NRV2B_LE32=2  # ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

/* These from /usr/include/asm/unistd.h */
__NR_Linux = 4000
__NR_close    =   6+ __NR_Linux
__NR_exit     =   1+ __NR_Linux
__NR_memfd_create= 354 + __NR_Linux
__NR_mmap     =  90+ __NR_Linux
__NR_mprotect = 125+ __NR_Linux
__NR_open     =   5+ __NR_Linux
__NR_write    =   4+ __NR_Linux
__NR_cacheflush = 147+ __NR_Linux

/* asm/cachectl.h */
ICACHE= 1<<0
DCACHE= 1<<1

#BAL=0x04110000

  section ELFMAINX; .set noreorder
sz_pack2 = . - 4

_start: .globl _start
#//     break  # debug only
        bal main
          addiu $r_fexp,ra, f_exp - 0f
0:

/* Returns 0 on success; non-zero on failure. */
f_exp:  # alternate name
decompressor:  # (uchar const *lxsrc, size_t lxsrclen, uchar *lxdst, u32 &lxdstlen, uint method)
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

        NRV_pre
        UCL_init    32,1,0
        decomp_done = NRV_eof
#include "arch/mips/r3000/nrv2b_d.ash"
        build nrv2b, full
        NRV_post

  section ELFMAINZ; .set noreorder
L72:
        li a0,2  # fd stderr
        li v0,__NR_write; syscall
die:
        li a0,127
        li v0,__NR_exit; syscall

unfold:  # IN: $r_fexp,$r_auxv,$r_PMASK,$r_FOLD
        addiu sp,sp,-sp_frame
        sw $r_PMASK,F_PMASK(sp)
        lw $r_LENX,sz_pack2 - f_exp($r_fexp)  # length before stub
        la $r_elfa,sz_pack2 - f_exp($r_fexp)
        sub $r_elfa,$r_elfa,$r_LENX  # $r_elfa= &Elf32_Ehdr of this stub

        bal 0f; move a0,ra; .asciz "upx"; .balign 4
0:
        move a1,zero
        li v0,__NR_memfd_create; syscall; move $r_mfd,v0

# alloca() for de-compressed stub
        lw $r_szuf,sz_unc($r_FOLD)  # sz_unc of fold
        move fp,sp
        sw $r_szuf,F_LENU(sp)
        li at,-2*NBPW
        subu sp,$r_szuf; and sp,at

        lw $r_ADRX,-NBPW($r_FOLD)  # O_BINFO
# Decompress folded code
        lb $meth,b_method($r_FOLD)  # a4
        sw $r_szuf,F_ADRU(fp)  # lzma uses for EOF
        la ldst,   F_ADRU(fp)  # a3  &slot on stack
        move dst,sp            # a2  dst for unfolding
        lw lsrc,sz_cpr($r_FOLD)  # a1
        bal f_exp  # decompress it
          addiu src,$r_FOLD,sz_b_info  # a0  folded code
        sw $r_PMASK,0*NBPW(sp)  # forward the PAGE_MASK

# Write de-compressed stub
        move a2,$r_szuf  # .sz_unc
        move a1,sp
        move a0,$r_mfd
        li v0,__NR_write; syscall
        move sp,fp

# Map de-compressed stub
        addiu sp,-6*NBPW  # space for all 6 args to mmap()
        sw zero,  5*NBPW(sp)  # last 2 args are in memory
        sw $r_mfd,4*NBPW(sp)
        li   a3,MAP_SHARED  # first 4 args are in registers, but kernel might store onto stack
        li   a2,PROT_WRITE|PROT_READ|PROT_EXEC  # FIXME: PROT_WRITE is DEBUG only
        lw   a1,sz_unc($r_FOLD)
        move a0,zero
        li v0,__NR_mmap; syscall; addiu sp,6*NBPW
        sw v0,F_ADRU(sp)
        addiu ra,v0,2*NBPW

        move a0,$r_mfd; li v0,__NR_close; syscall

        jr ra
          addu $r_ADRX,$r_elfa,$r_ADRX  # compressed data

zfind:  # result in $r_auxv
        lw v1,(a0); addiu a0,a0,NBPW
        bnez v1,zfind
          move $r_auxv,a0
        jr ra
          li t0,AT_PAGESZ  # prepare early

main:
        bal zfind
          addiu a0,sp,NBPW  # avoid feint of 0==argc
        bal zfind
          move a0,$r_auxv

# set $r_PMASK by finding actual page size in Elf32_auxv_t
1:
        lw  v1,a_type(a0)
          addiu a0,a0,sz_auxv
        beq v1,t0,2f  # AT_PAGESZ
          lw v0,a_val - sz_auxv(a0)
        bnez v1,1b  # AT_NULL
          li v0,PAGE_SIZE
2:
        neg $r_PMASK,v0
        bal unfold
          addiu $r_FOLD,ra,LrFLD - 0f  # &b_info for folded loader
0:
        .long O_BINFO
LrFLD:
        # { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */

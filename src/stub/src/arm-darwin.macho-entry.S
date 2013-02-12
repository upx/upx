/*  arm-darwin.macho-entry.S -- iPhone program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2013 Laszlo Molnar
*  Copyright (C) 2000-2013 John F. Reiser
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
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

//#define SIMULATE_ON_DEBIAN_EABI4 1
#undef  SIMULATE_ON_DEBIAN_EABI4

#ifdef SIMULATE_ON_DEBIAN_EABI4  /*{*/
  #define LINUX_ARM_CACHEFLUSH 1  /* SIMULATE_ON_DEBIAN_EABI4 */
  #define ARMEL_EABI4 1           /* SIMULATE_ON_DEBIAN_EABI4 */
#else  /*}{ usual case */
  #define DARWIN_ARM_CACHEFLUSH 1
  #define ARMEL_DARWIN 1
#endif  /*}*/

#include "arch/arm/v5a/macros.S"

#define bkpt .long 0xe1200070

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
PAGE_SIZE = -(~0<<PAGE_SHIFT)

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_mmap  =   197 + __NR_SYSCALL_BASE

// DEBUG ONLY:
__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

        //.long sz_pack2  // placed there by ::pack3()
  section MACHMAINX
_start: .globl _start
        sub sp,sp,#4  // space for mhdrp
        stmdb sp!,{r0-r12,r14,r15}  // paranoia [omit sp===r13]
        bl main
L20:
f_decompress:

  section NRV_HEAD
        // empty

  section NRV2E
#include "arch/arm/v4a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v4a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v4a/nrv2b_d8.S"

  section NRV_TAIL
        // empty

#include "arch/arm/v4a/lzma_d.S"

  section MACHMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section MACHMAINZ
unfold:  // lr= &L100
        ldr ldst,[lr,#sz_unc]
        ldr lsrc,[lr,#sz_cpr]
        add dst,lsrc,lr; add dst,dst,#sz_b_info
        add src,ldst,lr; add src,src,#GAP+NO_LAP  // defend against prefetch and overlap

        mov r4,lsrc  // loop count
movup:  // descending copy folded_loader to higher address
        ldrb ip,[dst,#-1]!
        subs r4,r4,#1
        strb ip,[src,#-1]!
        bne movup

        ldr r4,  [lr,#b_method]
        add lr,lr,#GAP
        PUSH {ldst}; mov ldst,sp  // LZMA needs for EOF
        mov dst,lr  // unfolded result
          sub r11,r5,#4+ L20 - _start  // &sz_pack2  param to unfolded loader
        PUSH {r4}  // 5th param to decompressor
          mov r10,lr  // DEBUG ONLY
        blx r5  // bx r5
          bx r10  // DEBUG ONLY

main:
        mov r5,lr  // &f_decompress
        bl unfold
L100:
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

GAP= 128  // > farthest prefetch;               must match ../../p_mach.cpp
NO_LAP= 64  // avoid overlap for folded loader; must match ../../p_mach.cpp

/*
vi:ts=8:et:nowrap
*/



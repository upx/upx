/* arm64-darwin.macho-entry.S -- iPhone program entry point & decompressor (Elf binary)
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

//#define SIMULATE_ON_DEBIAN_EABI4 1
#undef  SIMULATE_ON_DEBIAN_EABI4

#ifdef SIMULATE_ON_DEBIAN_EABI4  /*{*/
  #define LINUX_ARM_CACHEFLUSH 1  /* SIMULATE_ON_DEBIAN_EABI4 */
  #define ARMEL_EABI4 1           /* SIMULATE_ON_DEBIAN_EABI4 */
#else  /*}{ usual case */
  #define DARWIN_ARM_CACHEFLUSH 1
  #define ARMEL_DARWIN 1
#endif  /*}*/

#define lr x30

#include "arch/arm64/v8/macros.S"

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
        stp x0,x1,[sp,#-30*8]!  // paranoia; omit r30(lr), r31(sp)
        stp x2,x3,[sp,#2*8]
        stp x4,x5,[sp,#4*8]
        stp x6,x7,[sp,#6*8]
        stp x8,x9,[sp,#8*8]
        stp x10,x11,[sp,#10*8]
        stp x12,x13,[sp,#12*8]
        stp x14,x15,[sp,#14*8]
        stp x16,x17,[sp,#16*8]
        stp x18,x19,[sp,#18*8]
        stp x20,x21,[sp,#20*8]
        stp x22,x23,[sp,#22*8]
        stp x24,x25,[sp,#24*8]
        stp x26,x27,[sp,#26*8]
        stp x28,x29,[sp,#28*8]
        bl main
L20:
f_decompress:

  section NRV_HEAD
        // empty

  section NRV2E
#include "arch/arm64/v8/nrv2e_d8.S"

  section NRV2D
#include "arch/arm64/v8/nrv2d_d8.S"

  section NRV2B
#include "arch/arm64/v8/nrv2b_d8.S"

  section NRV_TAIL
        // empty

#include "arch/arm64/v8/lzma_d.S"

  section MACHMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section MACHMAINZ
#define lsrc w1
#define ldst w3
#define ldstx x3
unfold:  // lr= &L100
        ldr ldst,[lr,#sz_unc]
        ldr lsrc,[lr,#sz_cpr]
        add dst,lr,lsrc,uxtw; add dst,dst,#sz_b_info
        add src,lr,ldst,uxtw; add src,src,#GAP+NO_LAP  // defend against prefetch and overlap

        mov w4,lsrc  // loop count
#define t0 w9
movup:  // descending copy folded_loader to higher address
        ldr t0,[dst,#-4]!; sub w4,w4,#4
        str t0,[src,#-4]!; cbnz w4,movup

        ldr w4,  [lr,#b_method]  // 5th param to decompressor
        add lr,lr,#GAP
        PUSH1(ldst); mov ldstx,sp  // LZMA needs for EOF
        mov dst,lr  // unfolded result
          sub x11,x5,#4+ L20 - _start  // &sz_pack2  param to unfolded loader
          mov x10,lr  // DEBUG ONLY
        blr x5  // br r5
          br x10  // DEBUG ONLY

main:
        mov x5,lr  // &f_decompress
        bl unfold
L100:
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

GAP= 128  // > farthest prefetch;               must match ../../p_mach.cpp
NO_LAP= 64  // avoid overlap for folded loader; must match ../../p_mach.cpp

#undef ldstx
#undef ldst

/* vim:set ts=8 sw=8 et: */

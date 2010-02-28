/*  arm-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2010 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2010 Laszlo Molnar
*  Copyright (C) 2000-2010 John F. Reiser
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

#define ARM_OLDABI 1
#include "arch/arm/v4a/macros.S"

#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */
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
PAGE_SIZE = -(~0<<PAGE_SHIFT)

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_mmap64 = 0xc0 + __NR_SYSCALL_BASE

__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

        //.long sz_pack2  // placed there by ::pack3()
  section ELFMAINX
start_params:
        .long ADRM  // dst  for map
        .long LENF  // end_decompress - (start_params -4)
        .long CPR0  //           cpr0 - (start_params -4)
_start: .globl _start
////    nop; bkpt
/* Get some pages: enough
   to duplicate the entire compressed PT_LOAD, plus 1 page, located just after
   the brk() of the _un_compressed program.  The address is pre-calculated
   calculated by PackLinuxElf32arm::addLinkerSymbols().
*/
        adr r12,start_params -4  // &sz_pack2
        ldmia r12,{r1,r2, r10,r11}  // r1= sz_pack2; r2= ADRM; r10= LENF; r11= CPR0;
        add r10,r10,r12  // end_decompress
        add r11,r11,r12  // cpr0
        mov r0,r2  // ADRM
        sub r9,r12,r1  // &our_Elf32_Ehdr
        add r1,r1,# PAGE_SIZE
  section LUNMP000
        mov r3,#0<<12  // 0-page crumb
  section LUNMP001
        mov r3,#1<<12  // 1-page crumb
  section ELFMAINXu
        stmdb sp!,{r0,r1,r2,r3}  // ADRU,LENU,space for sz_unc,crumb
D_sz_unc=2*4  // stack displacement to sz_unc
        mov r2,#PROT_READ | PROT_WRITE | PROT_EXEC
        mov r3,#MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
        mvn r4,#0  // -1; cater to *BSD for fd when MAP_ANON
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_mmap64
        swi 0
#else  /*}{*/
        swi __NR_mmap64
#endif  /*}*/
        cmn r0,#4096
        bcs msg_SELinux
copy:
        ldmia r9!,{r1,r2,r3,r4,r5,r6,r7,r8}; cmp r9,r10  // upto end-decompress
        stmia r0!,{r1,r2,r3,r4,r5,r6,r7,r8}; blo copy

        sub r4,r0,r9  // relocation amount
        adr r5,f_decompress
        mov lr,r0  // dst for unfolded code
        add r5,r5,r4  // relocated f_decompress

        mov r0,r5
        add r1,r9,r4  // relocated end_decompress
        mov r2,#0
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,   #__ARM_NR_cacheflush& 0xff
        orr r7,r7,#__ARM_NR_cacheflush&~0xff
        swi 0
#else  /*}{*/
        swi __ARM_NR_cacheflush
#endif  /*}*/

        ldr r9,[r12]  // sz_pack2
        sub r9,r9,#sz_Elf32_Ehdr + 2*sz_Elf32_Phdr + sz_l_info + sz_p_info
        sub r10,r12,r9  // &b_info
        add r10,r10,r4  // relocated &b_info

        ldr r0,[r11,# b_method ]  // 5th param (whole word: endian issues!)
        stmdb sp!,{r0, lr}  // 5th param, dst for unfolded
D_stm1=2*4
        ldr  r3,[r11,# sz_unc]
        add  r0, r11,# sz_b_info
        ldr  r1,[r11,# sz_cpr ]
        mov  r2,lr  // dst
        str  r3,[sp,#D_stm1 + D_sz_unc]  // sz_unc; lzma needs for EOF
        add  r3, sp,#D_stm1 + D_sz_unc   // &sz_unc
        mov lr,pc; mov pc,r5  // decompress folded code [opcode 'bx' not in v4a]
        ldmia sp!,{r1, pc}  // discard 5th param; goto unfolded

f_decompress:
#define LINUX_ARM_CACHEFLUSH 1

  section NRV_HEAD
        // empty
  section NRV_TAIL
        // empty

  section NRV2E
#include "arch/arm/v4a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v4a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v4a/nrv2b_d8.S"

#include "arch/arm/v4a/lzma_d.S"

  section ELFMAINY
end_decompress: .globl end_decompress

msg_SELinux:
        mov r2,#L71 - L70  // length
        adr r1,L70  // message text
        mov r0,#2  // fd stderr
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_write
        swi 0
#else  /*}{*/
        swi __NR_write
#endif  /*}*/
die:
        mov r0,#127
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_exit
        swi 0
#else  /*}{*/
        swi __NR_exit
#endif  /*}*/
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        /* IDENTSTR goes here */

  section ELFMAINZ
cpr0: .globl cpr0
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/*
vi:ts=8:et:nowrap
*/


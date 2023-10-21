/*
;  i386-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2023 Laszlo Molnar
;  Copyright (C) 2000-2023 John F. Reiser
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

#include "arch/i386/macros.S"
        .att_syntax  // opcode src,dst
#define arg1 ebx
#define arg2 ecx
#define arg3 edx

NBPW= 4

O_RDWR= 2
O_CREAT= 0100
O_DIRECTORY= 0200000   // 0x10000
O_TMPFILE= 020000000  // 0x400000

__NR_exit=  1
__NR_write= 4
__NR_open=  5
__NR_close= 6
__NR_oldmmap=      90  // %ebx -> args[6]
__NR_memfd_create= 356
__NR_mprotect= 125

ENOSYS= 38	/* Invalid system call number */

M_NRV2B_LE32= 2

/*************************************************************************
// program entry point
// see glibc/sysdeps/i386/elf/start.S
**************************************************************************/
/*
;; How to debug this code:  Uncomment the 'int3' breakpoint instruction above.
;; Build the stubs and upx.  Compress a testcase, such as a copy of /bin/date.
;; Invoke gdb, and give a 'run' command.  Define a single-step macro such as
;;      define g
;;      stepi
;;      x/i $pc
;;      end
;; and a step-over macro such as
;;      define h
;;      x/2i $pc
;;      tbreak *$_
;;      continue
;;      x/i $pc
;;      end
;; Step through the code; remember that <Enter> repeats the previous command.
;;
*/

section ELFMAINX
sz_pack2 = -4+ _start
_start: .globl _start
////    nop; int3  // DEBUG
        call here; jmps L20  // esi= L05
L05:  Dsz05 = sz_pack2 - L05
        .asciz "/dev/shm"
L07:  D0705 = L07 - L05
        .asciz "/proc/self/exe"
L20:
// Create anonymous temporary file on mfd
        push $'u'|('p'<<8)|('x'<<16)|(0<<24)
        mov %esp,%arg1  // "upx"
        sub %arg2,%arg2  // 0
        movl $__NR_memfd_create,%eax; int $0x80  // our own error handling
        pop %ecx  // discard "upx"
        push %eax  // mfd in %entry

        test %eax,%eax; jns mfd_OK  // success
        cmp $-ENOSYS,%eax; jne mfd_OK  // not Android error
// memfd_create() gets ENOSYS on early Android.  There are 32-bit x86 Android
// such as Zenfone 2 (discontinued 2018?), x86 Chromebooks (2019 and later),
// FydeOS, Windows subsystem for Android.  But the main use is for developing,
// to make Android emulator running on x86_64 (Linux or Windows) run faster
// by emulating x86 instead of ARM.
//
// So fall back to an unnamed temporary file in /dev/shm.
// (Won't work if /dev/shm is mounted 'noexec'; too bad.)
        mov $0700,%arg3  // S_IRWXU  permissions
        mov $O_RDWR | O_DIRECTORY | O_TMPFILE,%arg2
        mov %esi,%arg1  // L05
        mov $__NR_open,%al; call sys_check1
        push %eax  // P_10 mfd in %entry
mfd_OK:
        sub %arg2,%arg2  // 0; O_RDONLY
        lea D0705(%esi),%arg1 // "/proc/self/exe"
        mov $__NR_open,%al; call sys_check1
        pop %ebx; push %ebx  // %ebx= mfd
        push %eax  // P_11  xfd  FIXME: why now?

PAGE_SHIFT= 12
PAGE_SIZE= ( 1 << PAGE_SHIFT)
PAGE_MASK= (~0 << PAGE_SHIFT)

MAP_FIXED=     0x10
MAP_SHARED=    0x01
MAP_PRIVATE=   0x02
MAP_ANONYMOUS= 0x20

PROT_READ=      1
PROT_WRITE=     2
PROT_EXEC=      4

szElf32_Ehdr= 0x34
p_memsz=  5*4

sz_unc= 0
sz_cpr= 4

EINVAL= 22

// Decompress the rest of this loader, and jump to it
// FIXME: this ought to be simpler!
unfold:
        lea Dsz05(%esi),%edi  // &sz_pack2
        mov (%edi),%edx  // sz_pack2
        sub %edx,%edi
        push %edi  // P_40   &Elf32_Ehdr,xfd,mfd; argc...
        call fold_data; pop %esi  // %esi/ O_BINFO,b_info
        lodsl; sub %eax,%edx; push %edx // P_41  LENX
        add %edi,%eax; push %eax  // P_42  ADRX,LENX,&Elf32_Ehdr,xfd,mfd; argc...

// alloca() for unfolded stub onto stack (at most 64K: l_info.l_lsize)
        mov (%esi),%edx  // .sz_unc
        lea 1*NBPW(%edx),%ecx  // room for P_55
        mov %esp,%eax
        sub %ecx,%eax  // -(.sz_unc + room for P_55)
        and $-2*NBPW,%eax  // aligned
        mov %eax,%edi  // dst for unfold
        push %edx  // P_55  .sz_unc
        xchg %eax,%esp  // installed

        push %eax  // P_52 old_sp
        push %esi  // P_59 &b_info
        lodsl; push %eax  // P_53  sz_unc
        mov %esp,%edx  // &sz_unc
        lodsl; xchg %eax,%ecx  // sz_cpr
        lodsl; cmpw $M_NRV2B_LE32|(0<<8),%ax; je 0f; hlt; 0:  // check method and filter bytes
        add %esi,%ecx; push %ecx  // P_57  &input_eof

// This is nrv2b_d32, inlined and optimized for small space (about 160 bytes).
// The task is to de-compress the folded pieces for shared library init:
// the de-compressor(s) of the PT_LOAD pieces, and the C-code supervisor
// which adjusts the placement and mapping of the address space.
// The output length is a couple KB for NRV, a few KB for Lzma, 64KB for Zstd.
// This is motivated by the possibility of using multiple de-compressors
// depending on the characteristics of each PT_LOAD, and by the increased size
// and compressability of C-coded de-compressors for Lzma and Zstd
// in contrast to the simple and small assembly-coded NRV.

//%esp:
//  P_57  &input_eof
//  P_53  sz_unc
//  P_59  &b_info
//  P_52  old_sp
//    unfolded_code
//  P_55  sz_unc
//  P_42  ADRX
//  P_41  LENX
//  P_40  &Elf32_Ehdr
//  P_11  xfd
//  P_10  %entry{mfd}
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define bits %ebx
#define len  %ecx  /* XXX: 2GB */
#define disp %ebp

#define GETBIT call *%edx
#define jnextb0 GETBIT; jnc
#define jnextb1 GETBIT; jc

/* rotate next bit into bottom bit of reg */
#define getnextb(reg) GETBIT; adcl reg,reg

        call L300
getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret
refill:
        movl (%esi),bits; sub $-4,%esi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        rep; ret
L300:
        pop %edx  // for GETBIT

        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        orl $-1,disp  // initial displacement
        cld  // paranoia
        .byte 0xa8  // "testb $... ,%al" ==> "jmp top_n2b"
lit_n2b:
        movsb  // *dst++ = *src++;
top_n2b:
        jnextb1 lit_n2b
        lea 1(len),off  # [len= 0] off= 1
offmore_n2b:
        getnextb(off)
        jnextb0 offmore_n2b

        subl $ 3,off; jc len_n2b  # use previous offset
        shll $ 8,off; lodsb  # off is %eax, so 'lodsb' is "off |= *src++;"
        xorl $~0,off; jz eof_n2b
        movl off,disp  # XXX: 2GB; FIXME for 4GB
len_n2b:
        lea 1(len),off  # [len= 0] off= 1
        getnextb(len); getnextb(len)  # two bits; cc set on result
        jnz gotlen_n2b  # raw 1,2,3 ==> 2,3,4
        movl off,len  # len= 1, the msb
        addl $3-1,off  # raw 2.. ==> 5..
lenmore_n2b:
        getnextb(len)
        jnextb0 lenmore_n2b
gotlen_n2b:
        cmpl $-0xd00,disp  # XXX: 2GB
        adcl off,len  # len += off + (disp < -0xd00)

        push %esi  // MATCH_06
          lea (%edi,disp),%esi
          rep; movsb
        pop %esi  // MATCH_06

        jmp top_n2b

eof_n2b:
        pop %ecx  // P_57  &input_eof
        cmp %ecx,%esi; je 0f; hlt; 0:  // test for ending in correct place
        //FIXME: check dst, too

// Write de-compressed 'fold' to file
        pop %arg3  // P_13  LENU
        pop %esi  // P_59  &b_info
        pop %ebp  // P_52  old_sp
        mov 5*NBPW(%ebp),%arg1  // mfd
        mov %esp,%arg2  // buffer
        mov $__NR_write,%al; call sys_check1  // %eax= LENU

// de-alloca()
        mov %ebp,%esp  // LENU,ADRX,LENX,elfaddr,xfd,mfd

// Map unfolded code the SELinux way
        push $0  // arg6  offset
        push %ebx  // arg5  mfd
        push $MAP_SHARED  // arg3  mode
        push $PROT_READ|PROT_EXEC
        push %eax  // LENU
        push $0  // arg1  kernel chooses addr
        call oldmmap  // preserves %ebx %arg1  %eax= ADRU

        pop %ecx  // P_55  LENU
        pop %edx  // P_42  ADRX
        pop %esi  // P_41  LENX
        pop %edi  // P_40  elfaddr

        push %ecx  // P_60  LENU
        push %eax  // P_61  ADRU
        push %esi  // P_62  LENX
        push %edx  // P_63  ADRX

        //mov %ebx,%arg1  // mfd
        mov $__NR_close,%al; call sys_check1  // FIXME

// Jump to unfolded code
        pop %eax  // P_63  ADRX
        pop %ecx  // P_62  LENX
// eax:ADRX; ebx:free; ecx:LENX
// ebp:free; esi:free; edi:elfaddr(dynbase?)
        pop %edx; push %edx  // P_61  ADRU
        jmp *%edx  // esp/ ADRU,LENU,xfd,%entry{mfd},argc,argv,0,...

oldmmap:  // %arg1 -> 6 arguments; remove arguments on return
        push %ebx  // preserve %ebx value
        lea 2*NBPW(%esp),%arg1
        mov $__NR_oldmmap,%al; call sys_check1
        pop %ebx  // restore %ebx value
        ret $6*NBPW

sys_check1:
        movzbl %al,%eax
sys_check:
        push %eax  // save __NR_ for debug
        int $0x80
        pop %edx  // recover __NR_ for debug
        cmp $PAGE_MASK,%eax; jae 0f; ret; 0:
        hlt
here:
        mov (%esp),%esi  // return address
        lodsw  // esi += 2;  skip 2-byte 'jmp'
        ret

  section ELFMAINZ
        .balign 4  // for Android emulator
        nop  // 1 byte
fold_data:
        pop %eax; call *%eax  // 1+2 bytes
        .long O_BINFO
            // compressed fold_elf86 follows
eof:

/* vim:set ts=8 sw=8 et: */

/*  i386-darwin.macho-fold.S -- linkage to C code to process Mach-o binary
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

        .att_syntax  // opcode src,dst
NBPW= 4

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

_start: .globl _start  # ignored, but silence "cannot find entry symbol _start" from ld

//In:
// %ebp= f_exp
// %esp/ MHDR, LENX, ADRX, FUNF, argc, argv, 0, envp, 0, apple, 0, strings

fold_begin:
//  int3
        pop %eax  // MHDR
        pop %edx  // LENX
        pop %ecx  // ADRX  &{l_info; p_info; b_info}

        mov sz_unc + sz_p_info + sz_l_info(%ecx),%ebx  # sz_mach_headers
        movl $2048,%esi
        cmp %esi,%ebx  # allow for /usr/lib/dyld: 1564==sizeofcmds (MacOS 10.6.8)
        ja 0f
        mov %esi,%ebx
0:
        pop %esi  // FUNF

        push $0  # default value for mhdrp
        mov %esp,%edi  # &mhdrp
        sub %ebx,%esp  # alloca
        pusha  # (mhdrpp, f_unfilter, f_decompress, mhdr, sz_mhdr, sz_total, src, MHDR)
        call upx_main  # Out: %eax= entry
        movl 5*NBPW(%esp),%edi  # sz_total
        movl 7*NBPW(%esp),%esi  # MHDR
        push %eax; push %edi; push %esi; call munmap
        pop %eax; pop %eax; pop %eax  # %eax= entry

        mov 4*NBPW(%esp),%ecx  # sz_mhdr
        lea 8*NBPW(%esp,%ecx),%esp  # un_pusha, un_alloca; keep mhdrp
        jmp *%eax  # esp: mhdrp, argc, argv...

bswap: .globl bswap
        mov 1*NBPW(%esp),%eax  # ptr
        mov 2*NBPW(%esp),%ecx  # len
0:
        mov (%eax),%edx
        .byte 0x0f,0xc8+2  // bswap edx
        mov %edx,(%eax)
        sub $NBPW,%ecx
        lea  NBPW(%eax),%eax
        ja 0b
        ret

SYS_exit  =1
SYS_fork  =2
SYS_read  =3
SYS_write =4
SYS_open  =5
SYS_close =6

SYS_pread =153
SYS_mmap    =197
SYS_munmap  = 73
SYS_mprotect= 74

spin: .globl spin
        rep; nop; jmp spin
        ret

sysgo:
        pop %edx  # return address for sysenter
        .byte 0x0f, 0x34  # sysenter

// lazy jmps enable compression of this code
write: .globl write
        mov $SYS_write,%al; 0:jmp 0f
exit: .globl exit
        mov $SYS_exit,%al;  0:jmp 0f
mprotect: .globl mprotect
        mov $SYS_mprotect,%al; 0:jmp 0f
munmap: .globl munmap
        mov $SYS_munmap,%al; 0:jmp 0f
pread: .globl pread
        mov $SYS_pread,%al; 0:jmp 0f
close: .globl close
        mov $SYS_close,%al; 0:jmp 0f
open: .globl open
        mov $SYS_open,%al;  0:jmp 0f
mmap: .globl mmap
        mov $SYS_mmap,%al;  0:jmp 0f
read: .globl read
        mov $SYS_read,%al;  0:

        movzbl %al,%eax  # SYS_nnnnn
        mov %esp,%ecx  # &{user_ret_addr, arg1, arg2, ...}
        call sysgo
        jnc 0f
        //mov errno,%eax
        or $~0,%eax
0:
        ret

.balign 2,144

/* vim:set ts=8 sw=8 et: */

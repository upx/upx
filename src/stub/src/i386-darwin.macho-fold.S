/*  i386-darwin.macho-fold.S -- linkage to C code to process Mach-o binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2008 Laszlo Molnar
*  Copyright (C) 2000-2008 John F. Reiser
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

#include "arch/i386/macros.S"

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

_start: .globl _start  # ignored, but silence "cannot find entry symbol _start" from ld

i386_ts_eax = 0*4
i386_ts_ebx = 1*4
i386_ts_ecx = 2*4
i386_ts_edx = 3*4
i386_ts_edi = 4*4
i386_ts_esi = 5*4
i386_ts_ebp = 6*4
i386_ts_esp = 7*4
i386_ts_ss  = 8*4
i386_ts_eflags = 9*4
i386_ts_eip = 10*4
i386_ts_cs  = 11*4
i386_ts_ds  = 12*4
i386_ts_es  = 13*4
i386_ts_fs  = 14*4
i386_ts_gs  = 15*4

fold_begin:  // In: ebx= &total_length
////        int3
        push 0  # default value for mhdrp
        mov edi,esp  # &mhdrp
        lea esi,[ 2+ ebp]  # &f_unfilter
        mov edx,[ebx]  # sz_total
        mov ecx,ebx
        sub ecx,edx  # src= &{l_info; p_info; b_info}
        mov ebx,[sz_unc + sz_p_info + sz_l_info + ecx]  # sz_mach_headers
        mov eax,2048  # allow for /usr/lib/dyld
        cmp eax,ebx
        jbe 0f
        mov ebx,eax
0:
        sub esp,ebx  # alloca
        pusha  # (mhdrpp, f_unfilter, f_decompress, mhdr, sz_mhdr, sz_total, src, junk)
        call upx_main  # Out: eax= &Mach_i386_thread_state of dyld
        mov ecx,[4*4 + esp]  # sz_mhdr
        lea esp,[8*4 + 1*ecx + esp]  # un_pusha, un_alloca; keep mhdrp
        jmp [i386_ts_eip + eax]  # esp: mhdrp, argc, argv...

bswap: .globl bswap
        mov eax,[4+ esp]  # ptr
        mov ecx,[8+ esp]  # len
0:
        mov edx,[eax]
        .byte 0x0f,0xc8+2  // bswap edx
        mov [eax],edx
        sub ecx,4
        lea eax,[4+ eax]
        jae 0b
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

sysgo:
        pop edx  # return address for sysenter
        .byte 0x0f, 0x34  # sysenter

// lazy jmps enable compression of this code
exit: .globl exit
        mov al,SYS_exit;  jmps 2+ 0f; 0:
mprotect: .globl mprotect
        mov al,SYS_mprotect; jmps 2+ 0f; 0:
munmap: .globl munmap
        mov al,SYS_munmap; jmps 2+ 0f; 0:
pread: .globl pread
        mov al,SYS_pread; jmps 2+ 0f; 0:
close: .globl close
        mov al,SYS_close; jmps 2+ 0f; 0:
open: .globl open
        mov al,SYS_open;  jmps 2+ 0f; 0:
mmap: .globl mmap
        mov al,SYS_mmap;  jmps 2+ 0f; 0:
read: .globl read
        mov al,SYS_read

        movzbl eax,al  # SYS_nnnnn
        mov ecx,esp  # &{user_ret_addr, arg1, arg2, ...}
        call sysgo
        jncs 0f
        //mov errno,eax
        or eax,~0
0:
        ret

/*
vi:ts=8:et:nowrap
*/


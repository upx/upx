/*  i386-darwin.macho-fold.S -- linkage to C code to process Mach-o binary
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
fold_begin:
        int3
        call upx_main  # Out: eax= &Mach_i386_thread_state of dyld
        jmp [i386_ts_eip + eax]

SYS_exit  =1
SYS_fork  =2
SYS_read  =3
SYS_write =4
SYS_open  =5
SYS_close =6

SYS_pread =153
SYS_mmap    =197
SYS_mprotect= 74

/* SYS_mmap takes a 64-bit off_t, but gcc-3.4.1-20040827 passes long long
   in wrong registers.  So change C interface to use size_t (32-bits) instead
   of off_t (64 bits), and convert here.
*/
#define pushbli  .byte 0x6a,

pread: .globl pread
        int3
        pushbli SYS_pread
mmap: .globl mmap
        int3
        pushbli SYS_mmap

exit: .globl exit
        int3
        pushbli SYS_exit
read: .globl read
        int3
        pushbli SYS_read
open: .globl open
        int3
        pushbli SYS_open
close: .globl close
        pushbli SYS_close
mprotect: .globl mprotect
        pushbli SYS_mprotect

/*
vi:ts=8:et:nowrap
*/


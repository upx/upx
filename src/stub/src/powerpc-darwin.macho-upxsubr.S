/*
;  powerpc-darwin.macho-upxsubr.S -- system calls
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
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/
        .machine ppc

SYS_exit  =1
SYS_fork  =2
SYS_read  =3
SYS_write =4
SYS_open  =5
SYS_close =6

SYS_pread =153
SYS_mmap    =197
SYS_munmap=   73
SYS_mprotect= 74

_start: .globl _start
        //trap  // debug
        lwz r3,0(r1)  // argc
        la  r4,4(r1)  // argv
        la  r1,-(1+6)*4(r1)  // 6:frame convention, 1: mhdrpp
        bl _main
        mtctr r3  // entry to dyloader
        la  r1, (0+6)*4(r1)  // undo frame; retain mhdrp
        li r0,0
        mtlr r0  // paranoia
        bctrl
        trap  // should never happen

/* SYS_mmap takes a 64-bit off_t, but gcc-3.4.1-20040827 passes long long
   in wrong registers.  So change C interface to use size_t (32-bits) instead
   of off_t (64 bits), and convert here.
*/
_pread: .globl _pread
        mr r7,r6
        li r6,0
        li r0,153
        b sysgo
_mmap: .globl _mmap
        mr r9,r8
        li r8,0
        li r0,197
sysgo:
        sc
          li r3,-1
        blr

_exit: .globl _exit
        li r0,SYS_exit
        b sysgo
_read: .globl _read
        li r0,SYS_read
        b sysgo
_open: .globl _open
        li r0,SYS_open
        b sysgo
_close: .globl _close
        li r0,SYS_close
        b sysgo
_mprotect: .globl _mprotect
        li r0,SYS_mprotect
        b sysgo
_munmap: .globl _munmap
        li r0, SYS_munmap
        b sysgo

dyld_stub_binding_helper: .globl dyld_stub_binding_helper
        b dyld_stub_binding_helper

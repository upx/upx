/*
;  i386-bsd.syscall.S -- *BSD system calls
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006 John F. Reiser
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

#include "arch/i386/macros.S"

#define SYS_mmap    197
#define SYS_syscall 198

mmap: .globl mmap
        push ebp
        mov ebp,esp
        xor eax,eax  // 0
        push eax  // convert to 64-bit
        push dword ptr [7*4+ebp]  // offset
        push eax  // pad
        push dword ptr [6*4+ebp]  // fd
        push dword ptr [5*4+ebp]  // flags
        push dword ptr [4*4+ebp]  // prot
        push dword ptr [3*4+ebp]  // len
        push dword ptr [2*4+ebp]  // addr
        push eax  // current thread
        mov al,SYS_mmap
        push eax
        push eax  // fake ret.addr
        mov al,SYS_syscall
        int 0x80
        leave
        ret

brk: .globl brk
        ret

#define SYS_exit   1
#define SYS_read   3
#define SYS_write  4
#define SYS_open   5
#define SYS_close  6
#define SYS_munmap   73
#define SYS_mprotect 74

#define SYS_access  33
#define SYS_getpid  20
#define SYS_fcntl   92
#define SYS_execve  59
#define SYS_fork     2
#define SYS_ftruncate  130
#define SYS_gettimeofday  116
#define SYS_nanosleep  240
#define SYS_unlink  10
#define SYS_waitpid 7  /* FIXME: wait4 */

exit: .globl exit
        mov al,SYS_exit
nf_sysgo:
        movzx eax,al
        int 0x80
        ret

#define SYS(symbol) symbol: .globl symbol; mov al,SYS_##symbol; jmp nf_sysgo

SYS(read)
SYS(write)
SYS(open)
SYS(close)
SYS(munmap)
SYS(mprotect)

SYS(access)
SYS(getpid)
SYS(fcntl)
SYS(execve)
SYS(fork)
SYS(ftruncate)
SYS(gettimeofday)
SYS(nanosleep)
SYS(unlink)
SYS(waitpid)

// vi:ts=8:et:nowrap


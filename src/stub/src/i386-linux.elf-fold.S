/*
;  i386-linux.elf-fold.S -- linkage to C code to process Elf binary
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2000-2017 John F. Reiser
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


PAGE_SIZE= ( 1<<12)
PATH_MAX= 4096

ET_DYN= 3
#define szElf32_Ehdr 0x34
#define szElf32_Phdr 8*4
e_type=   16
e_phnum=  44
#define e_entry  (16 + 2*2 + 4)
#define p_memsz  5*4
#define szb_info 12
#define szl_info 12
#define szp_info 12
#define a_type 0
#define a_val  4
#define sz_auxv 8

__NR_readlink= 85
__NR_munmap=   91
__NR_open= 5
__NR_close= 6
__NR_mmap= 0xc0

MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

// control just falls through, after this part and compiled C code
// are uncompressed.

// enter: %ebx= &Elf32_Ehdr of this program
// %ebp= f_expand
// %edi= total length of compressed data
// %esp: O_BINFO,argc,argv,0,envp,0,auxv,0,strings

fold_begin:
        pop eax  // O_BINFO
        pop edx  // argc

        mov ecx,edi  // total length of compressed data
        mov esi,esp  // argv
        sub esp,PATH_MAX
        mov edi,esp
        push edx  // argc
        push eax  // O_BINFO
        push ecx  // total length of compressed data
        push 0  // space for fd
        push ebx  // &Elf32_Ehdr
        push ebp  // f_exp
        mov ebp,esp  // frame: f_exp,&Elf32_Ehdr,0,tot_len,O_BINFO,argc
0:
        lodsd; test %eax,%eax; stosd; jne 0b  // argv
        push edi  // &new_env[0]; f_exp,&Elf32_Ehdr,0,tot_len,O_BINFO,argc
        stosd  // space for new_env[0]
0:
        lodsd; test %eax,%eax; stosd; jne 0b  // env
        push edi  // &old_auxv,&new_env[0]; f_exp,&Elf32_Ehdr,0,tot_len,O_BINFO,argc
0:
        lodsd; test %eax,%eax; stosd; movsd; jne 0b  // auxv

        inc eax  // 1, AT_IGNORE
        sub edi,2*4  // {AT_NULL}
        mov ecx,5*2; rep stosd  // 5 extra slots
        dec eax; stosd; stosd  // {AT_IGNORE}
        sub [-2*4 + ebp],edi  // -len_aux
        push edi  // &new_aux[N],-len_aux,&new_env[0]; f_exp,&Elf32_Ehdr,0,tot_len,O_BINFO,argc
        push esi  // &strings,&new_aux[N],-len_aux,&new_env[0]; f_exp,&Elf32_Ehdr,0,tot_len,O_BINFO,argc

        mov edx,-1+ PATH_MAX  // buflen
        mov ecx,edi  // buffer
        call 1f
0:
        .asciz "/proc/self/exe"
1:
        pop ebx  // path
        mov eax,__NR_readlink; int 0x80; test eax,eax; jns 0f
        mov ecx,ebx  // point at name
        mov eax,-1+ (1b - 0b)  // len
0:
        lea esi,[-1+ ecx + eax]  // src last byte
        xchg ecx,eax  // ecx= byte count

        std
        pop edi; dec edi  // abuts old strings; &new_aux[N],-len_aux,&new_env[0]; f_exp,&Elf32_Ehdr,0,tot_len,O_BINFO,argc
        mov al,0; stosb  // terminate
        rep movsb  // slide up
        mov eax, 0+ ('='<<24)|(' '<<16)|(' '<<8)|(' '<<0)  # env var name
        sub edi,3; mov [edi],eax
        mov eax,[-1*4 + ebp]; mov [eax],edi  // new_env[0]
        and edi,~3  // word align

        sub ecx,ecx  // O_RDONLY
        // mov ebx,ebx  // name
        mov eax,__NR_open; int 0x80
        mov [2*4 + ebp],eax  // fd for later mmap

        pop esi  // &new_aux[N]; -len_aux,&new_env[0]; f_exp,&Elf32_Ehdr,fd,tot_len,O_BINFO,argc
        pop edx  // -len_aux; &new_env[0]; f_exp,&Elf32_Ehdr,fd,tot_len,O_BINFO,argc
          add edx,edi  // edx= &final_aux[0]

        scasd  // edi -= 4
                mov ecx,esi
        lodsd  // esi -= 4
                sub ecx,esp
                shr ecx,2
        rep movsd
        lea esp,[4+ edi]
          mov edi,edx  // &final_aux[0]
        cld

        pop ecx  // toss &new_env[0]
        pop ebp  // f_exp
        pop ebx  // &Elf32_Ehdr
        pop eax  // fd
        pop esi  // tot_len
        pop ecx  // O_BINFO
// stack is back to original state: argc,argv,0,env,0,aux,0,strings,0

        mov edx, [p_memsz + szElf32_Ehdr + ebx]  // phdr[0].p_memsz
        lea edx, [       PAGE_SIZE + edx + ebx]  // 1 page for unfold
        sub edx,ebx; push edx  // length
        push ebx  // start of unmap region (&Elf32_Ehdr of this stub)
        push eax  // fd

#define OVERHEAD 2048
#define MAX_ELF_HDR 512
        sub esp, MAX_ELF_HDR + OVERHEAD  // alloca

        sub eax, eax  // 0
        cmp word ptr [e_type + ebx], ET_DYN
        jne L53
        xchg eax, edx  // dynbase for ET_DYN; assumes mmap(0, ...) is placed after us!
L53:
        push eax  // dynbase

        lea eax, [ecx + ebx]  // 1st &b_info
        mov ebx, [   eax]  // length of uncompressed ELF headers
        mov ecx, [4+ eax]  // length of   compressed ELF headers
        add ecx, szb_info
        lea edx, [3*4 + esp]  // &tmp
        pusha  // (AT_table, sz_cpr, f_expand, &tmp_ehdr, {sz_unc, &tmp}, {sz_cpr, &b1st_info} )
        inc edi  // swap with above 'pusha' to inhibit auxv_up for PT_INTERP
.extern upx_main
        call upx_main  // returns entry address
        dec edi
        add esp, (8 +1)*4 + MAX_ELF_HDR + OVERHEAD  // 8 params, dynbase, un-alloca
        pop ecx  // fd
        pop ebx  // base to unmap
        pop edx  // length

        push eax  // entry address
        push edx  // length
        push ebx  // base to unmap
        push edi  // auxv
        push ecx  // fd, auxv, unmap, length, entry

        sub ebp,ebp  // 0 block in file
        mov edi,ecx  // fd
        mov esi,MAP_PRIVATE
        mov edx,PROT_READ
        mov ecx,PAGE_SIZE
        sub ebx,ebx  // 0 ==> Linux chooses page frame
        mov eax,__NR_mmap; int 0x80

        pop ebx  // fd;  auxv, base, length, entry
        mov eax,__NR_close; int 0x80

        pop edi  // auxv table
        sub eax,eax  // 0, also AT_NULL
        .byte 0x3c  // "cmpb al, byte ..." like "jmp 1+L60" but 1 byte shorter
L60:
        scasd  // a_un.a_val etc.
        scasd  // a_type
        jne L60  // not AT_NULL
// edi now points at [AT_NULL]a_un.a_ptr which contains result of make_hatch()
        pop ebx  // base to unmap (&Elf32_Ehdr of this stub)
        pop ecx  // length

        push eax
        push eax
        push eax
        push eax
        push eax
        push eax
        push eax
        push eax  // 32 bytes of zeroes now on stack, ready for 'popa'

        mov al, __NR_munmap  // eax was 0 from L60
        jmp [edi]  // unmap ourselves via escape hatch, then goto entry

mmap: .globl mmap  // what happened to the ebx->args_on_stack method?
        push ebp
        push ebx
        push esi
        push edi
        mov ebx,[ 5*4 + esp]
        mov ecx,[ 6*4 + esp]
        mov edx,[ 7*4 + esp]
        mov esi,[ 8*4 + esp]
        mov edi,[ 9*4 + esp]
        mov ebp,[10*4 + esp]
        shr ebp,12
        mov eax,__NR_mmap
        int 0x80

        pop edi
        pop esi
        pop ebx
        pop ebp
        ret

.balign 4,0

/* vim:set ts=8 sw=8 et: */

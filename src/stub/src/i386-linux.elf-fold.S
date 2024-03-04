/*
;  i386-linux.elf-fold.S -- linkage to C code to process Elf binary
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2000-2024 John F. Reiser
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
NBPW= 4

# WARNING: This file uses Intel syntax:  opcode dst,src
# Memory operands use square brackets:  [displ + %reg]
# Literal operands lack square brackets.

PAGE_SIZE= ( 1<<12)
PAGE_MASK= -PAGE_SIZE
PATH_MAX= 4096

ET_DYN= 3
ET_EXEC= 2
sz_Elf32_Ehdr= 0x34
  e_type=   16
  e_entry=  (16 + 2*2 + 4)
  e_phnum=  44
sz_Elf32_Phdr= 8*4
  p_memsz=  5*4

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
__NR_newmmap= 0xc0  // new mmap: args in ebx,ecx,edx,esi,edi,ebp
__NR_oldmmap=   90  // old mmap: %ebx -> args[6]
__NR_mprotect= 0x7d
__NR_brk= 45

MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

// control just falls through, after this part and compiled C code
// are uncompressed.

// enter at fold :
// esp/ ADRX,LENX,elfaddr,ADRU,LENU,%fd,%entry,argc,argv,0,envp,0,auxv,0,strings
//      (ADRU,LENU) = params for final munmap()
//      (ADRX,LENX) = extent of compressed program
F_ENTR= 6*NBPW; F_PMASK= F_ENTR

PAGE_MASK=  . - 3*NBPW
upxfn_path= . - 2*NBPW  // displacement to filename string
mflg_data=  . - 1*NBPW  // QNZ vs Linux

fold:
  // int3  // DEBUG
        jmp L10

get_page_mask: .globl get_page_mask
        call 0f; 0: pop %ecx
        mov %eax,[PAGE_MASK - 0b + %ecx]
        ret

get_upxfn_path: .globl get_upxfn_path
        call 0f; 0: pop %ecx
        mov %eax,[upxfn_path - 0b + %ecx]
        test %eax,%eax; je 1f; add %eax,%ecx
1:
        ret

L10:
        mov esi,esp; sub esp,PATH_MAX
        mov edi,esp; push 8; pop ecx; rep movsd  // copy ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc

        push ebp  // f_exp  FIXME: this is no longer used
        mov ebp,esp  // frame: f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
0:
        lodsd; test %eax,%eax; stosd; jne 0b  // argv
        push edi  // &new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
        stosd  // space for new_env[0]
0:
        lodsd; test %eax,%eax; stosd; jne 0b  // env
        push edi  // &old_auxv,&new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
0:
        lodsd; test %eax,%eax; stosd; movsd; jne 0b  // auxv

        inc eax  // 1, AT_IGNORE
        sub edi,2*NBPW  // back to {AT_NULL}
        mov ecx,5*2; rep stosd  // 5 extra slots of AT_IGNORE
        dec eax; stosd; stosd  // 0, AT_NULL
        sub [-2*NBPW + ebp],edi  // -len_aux
        push edi  // P_07  &new_aux[N],-len_aux,&new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
        push esi  // &strings,&new_aux[N],-len_aux,&new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc

        call 1f
0:
        .asciz "/proc/self/exe"
1:
        // Use inline 'int 0x80' because failure is not fatal
        pop ebx  // path
        sub ecx,ecx  // O_RDONLY
        push __NR_open; pop eax; int 0x80; mov [esp+ 10*NBPW],eax  // fd for mmap

        mov edx,-5*2*NBPW -1+ PATH_MAX  // buflen
        mov ecx,edi  // buffer
        // mov ebx,ebx  // name
        push __NR_readlink; pop eax; int 0x80
        test eax,eax; jns 0f
        mov ecx,ebx  // point at name
        mov eax,-1+ (1b - 0b)  // len
0:
        lea esi,[-1+ ecx + eax]  // src last byte
        xchg ecx,eax  // ecx= byte count

   std
        pop edi; dec edi  // abuts old strings; &new_aux[N],-len_aux,&new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
        mov al,0; stosb  // terminate
        rep movsb  // slide up
        mov eax, 0+ ('='<<24)|(' '<<16)|(' '<<8)|(' '<<0)  # env var name
        sub edi,3; mov [edi],eax
        mov eax,[-1*NBPW + ebp]; mov [eax],edi  // new_env[0]
        and edi,~3  // word align

        pop esi  // P_07  &new_aux[N]; -len_aux,&new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
// Final sp must be 0 mod 8.  There are now 10 words below argc.
        mov ecx,esi  // last
        sub ecx,esp  // length of moved block
        mov eax,ecx
        xor eax,edi  // check parity of purported destination
        and eax,4
        sub edi,eax  // align &new_aux[last]

        pop edx  // -len_aux; &new_env[0]; f_exp,ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc
          add edx,edi  // edx= &final_aux[0]

        scasd  // edi -= 4
        lodsd  // esi -= 4
        shr ecx,2; dec ecx; rep movsd  // dec: compensate for P_07
   cld
// Clear the vacated stack, for buggy programs that assume it is 0
        lea ecx,[1*NBPW+ edi]  // correct for 'std'
        xor eax,eax  // 0
        sub ecx,esp
        mov edi,esp
        shr ecx,2; rep stosd
        mov esp,edi
          mov edi,edx  // &final_aux[0]

        pop eax  // toss &new_env[0]
        pop ebp  // f_exp
// stack is back to original state: ADRX,LENX,elfaddr,ADRU,LENU,fd,entry,argc

        pop eax  // ADRX: &b_info
        pop esi  // LENX: total_size
        pop ebp  // elfaddr
#define OVERHEAD 2048
#include "MAX_ELF_HDR.S"
        sub esp, MAX_ELF_HDR_32 + OVERHEAD  // alloca
        push esp  // &tmp
        push edi  // &final_auxv[0]
        push ebp  // elfaddr
        push esi  // LENX total size
        push eax  // ADRX &b_info
        call upx_main  // returns entry address
        mov esi,eax  // esi= entry;  edi= auxv; ebp= elfaddr

// Discard C_TEXT (includes [ADRC,+LENC) )
        mov ecx,[ebp+ p_memsz+sz_Elf32_Phdr+sz_Elf32_Ehdr]  // Phdr[C_TEXT= 1].p_memsz
        mov ebx,ebp  // hi &Elf32_Ehdr
        cmpw [e_type + ebp],ET_EXEC; jne 1f
          mov al,__NR_brk; call sys_check_al  // static __end_bss
1:
        mov al,__NR_munmap; call sys_check_al  // discard C_TEXT compressed data

// Buggy programs may depend on uninit stack being 0, so clear what we used.
        mov edx,edi  // save auxv
        mov edi,esp
        mov ecx,(5*NBPW + MAX_ELF_HDR_32 + OVERHEAD) >>2  // 5 params, un-alloca
        xor eax,eax  // 0
        rep stosd  // clear frame on exit
        mov esp,edi  // end of frame

        pop eax  // ADRU
        pop ecx  // LENU
        pop edi  // fd
        pop ebx  // %entry
        push esi  // entry
        push ecx  // LENU
        push eax  // ADRU
        push edx  // auxv
        push edi  // fd, auxv, ADRU, LENU, entry, argc

// Map one page of /proc/self/exe so that symlink does not disappear
        pop ebx  // fd in case no_pse_map
        test edi,edi; js no_pse_map
        push ebx  // restore fd
        sub ebp,ebp  // 0 block in file
        // edi has fd
        push MAP_PRIVATE; pop esi
        push PROT_READ; pop edx
        mov ecx,PAGE_SIZE
        sub ebx,ebx  // 0 ==> Linux chooses page frame
        mov al,__NR_newmmap; call sys_check_al

        pop ebx  // fd;  auxv, LENU, ADRU, entry, argc
        mov al,__NR_close; call sys_check_al
no_pse_map:
        pop edi  // auxv table
        sub eax,eax  // 0, also AT_NULL
        .byte 0x3c  // "cmpb al, byte ..." like "jmp 1+L60" but 1 byte shorter
L60:
        scasd  // a_un.a_val etc.
        scasd  // a_type
        jne L60  // not AT_NULL
// edi now points at [AT_NULL]a_un.a_ptr which contains result of make_hatch()

        pop ebx  // ADRU  parameters for final munmap
        pop ecx  // LENU

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

        section SYSCALLS
// Sometimes linux enforces page-aligned address
Pprotect: .globl Pprotect
    xchg ebx,[1*NBPW + esp]  // save reg, %ebx= address
    mov ecx,ebx  // copy address
    and ebx, ~0<<12  // page align
    sub ecx,%ebx  // extra length
    add ecx,[2*NBPW + esp]  // length
    mov edx,[3*NBPW + esp]  // bits
    mov al,__NR_mprotect; call sys_check_al
    cmp eax, -0x1000; jna 0f; hlt; 0:
    mov ebx,[1*NBPW + esp]  // restore reg
    ret

    // FIXME: page-shift the file offset (last parameter) ??
// C-callable, so do NOT remove arguments as part of return
mmap: .globl mmap  // oldmmap: ebx -> 6 arguments
        push ebx  // save register
        lea ebx,[2*NBPW + esp]
        mov al,__NR_oldmmap; call sys_check_al
        pop ebx  // restore register
        ret

sys_check_al:
        movzbl eax,al
sys_check:
        push eax  // save __NR_ for debug
        int 0x80
        pop edx  // recover __NR_ for debug
        cmp %eax,-1<<12; jae 0f; ret; 0:
        hlt

my_memset: .globl my_memset  // (dst, val, n)
        push %ebp; mov %esp,%ebp
        push %edi
        mov %ecx,[(2+ 2)*NBPW + %ebp]
        mov %eax,[(2+ 1)*NBPW + %ebp]
        mov %edi,[(2+ 0)*NBPW + %ebp]
        rep stosb
        pop %edi
        pop %ebp
        ret

mempcpy: .globl mempcpy  // (dst, src, n)
        push %ebp; mov %esp,%ebp
        push %edi; push %esi
        mov %ecx,[(2+ 2)*NBPW + %ebp]
        mov %eax,[(2+ 1)*NBPW + %ebp]
        mov %edi,[(2+ 0)*NBPW + %ebp]
        rep movsb
        mov %edi,%eax
        pop %esi; pop %edi; pop %ebp
        ret

my_bkpt: .globl my_bkpt
        int3  // my_bkpt
        ret

.balign 4,0

/* vim:set ts=8 sw=8 et: */

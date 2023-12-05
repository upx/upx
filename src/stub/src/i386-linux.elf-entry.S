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

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

PROT_READ=      1
PROT_WRITE=     2
PROT_EXEC=      4

MAP_FIXED=     0x10
MAP_SHARED=    0x01
MAP_PRIVATE=   0x02
MAP_ANONYMOUS= 0x20

szElf32_Ehdr= 0x34
p_memsz=  5*4

__NR_exit=  1
__NR_write= 4
__NR_open=  5
__NR_close= 6
__NR_oldmmap=      90  // %ebx -> args[6]
__NR_memfd_create= 356
__NR_mprotect= 125

ENOSYS= 38	/* Invalid system call number */
NAME_MAX=  255  // # chars in file name; linux/include/uapi/linux/limits.h

PAGE_SHIFT= 12
PAGE_SIZE= ( 1 << PAGE_SHIFT)
PAGE_MASK= (~0 << PAGE_SHIFT)

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
////    nop; int3  // DEBUG  i386 entry.S
        call L70  // MATCH_08  push $&getbit
L70ret:

#define bits %ebx
getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret  // rep: stop instruction pipeline (spend 1 byte for speed)
refill:
        mov (%esi),bits; sub $-NBPW,%esi  // next 32 bits; set Carry
        adc bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        ret  // infrequent (1/32)

#define foldi %esi
#define r_getb %edx
#define r_unc %ebx
#define u_len %edi

L20:
        pop foldi  // MATCH_09  &fold_info

#define old_sp %ebp
F_FRAME= 7*NBPW
F_ENTR= 6*NBPW; F_PMASK= F_ENTR
F_MFD=  5*NBPW
F_LENU= 4*NBPW
F_ADRU= 3*NBPW
F_ELFA= 2*NBPW
F_LENX= 1*NBPW
F_ADRX= 0*NBPW

D_FOLD=  3*NBPW  // .data space at start of unfold
D_QFLG=  2*NBPW
D_FNAME= 1*NBPW
D_PMASK= 0*NBPW

#if 0  //{ DEBUG
        mov $0xa5a5a5a5,%eax
        push %eax; push %eax; push %eax; push %eax
        push %eax; push %eax; push %eax
#else  //}{ non-DEBUG
        sub $F_FRAME,%esp
#endif  //}
AT_PAGESZ= 6  // /usr/include/elf.h
        lea NBPW + F_FRAME(%esp),%edi  // past argc
        call zfind  // skip argv
        call zfind  // skip env
0: // find AT_PAGESZ in auxv
        mov     (%edi),%eax
        mov NBPW(%edi),%ecx; add $2*NBPW,%edi
        cmp $AT_PAGESZ,%eax; je 5f
        mov $1<<12,%ecx  // default PAGE_SIZE
        test %eax,%eax; jne 0b
5:
        neg %ecx  // PAGE_MASK

        mov %esp,old_sp
        mov %ecx,F_PMASK(old_sp)
        lea sz_pack2 - L70ret(r_getb),%edi
        mov (%edi),%ecx
        sub %ecx,%edi;   mov %edi,F_ELFA(old_sp)
        lodsl  // O_BINFO; advance to &b_info
        add %eax,%edi; mov %edi,F_ADRX(old_sp)
        sub %eax,%ecx; mov %ecx,F_LENX(old_sp)
//        cmpw $M_NRV2B_LE32|(0<<8),b_method(foldi); je 0f; hlt; 0:  // check method and filter bytes

        mov /*sz_unc*/(foldi),r_unc
            // align + {page_mask, fd_name} + "/data/data/$APP_NAME/cache/upxAAA"
        lea (1+ NAME_MAX + 8 + D_FOLD + (1+ 11 + 13))(r_unc),u_len
        mov u_len,F_LENU(old_sp)
        sub u_len,%esp  // alloca
        and $-2*NBPW,%esp  // align stack

        lea (8 + D_FOLD)(r_unc),%eax; add %esp,%eax
        and $-2*NBPW,%eax; push %eax  // MATCH_31 where to put pathname
        mov %eax,%ecx
        sub %esp,%ecx;            mov %ecx,D_FNAME(%esp)  // forward to unfolded code
        mov F_PMASK(old_sp),%ecx; mov %ecx,D_PMASK(%esp)  // forward to unfolded code
        mov $MAP_PRIVATE|MAP_ANONYMOUS,%ecx; mov %ecx,D_QFLG(%esp)  // forward to unfolded code

        xor %ecx,%ecx  // zero
        push %edx  // MATCH_33  save &getbit
        push %eax  // arg3  &pathname
        mov %ecx,(%eax)  // empty string
        push u_len  // arg2  length required
        push %ecx  // arg1  0==> any page
        call upx_mmap_and_fd; add $3*NBPW,%esp  // (ptr, len, pathname)
        pop %edx  // MATCH_33  restore &getbit
        mov %eax,%ecx
        shr $12,%eax; shl $12,%eax; mov %eax,F_ADRU(old_sp)
        sub %eax,%ecx; sub $1,%ecx; mov %ecx,F_MFD (old_sp)
#undef r_unc
#undef u_len

// This is nrv2b_d32, inlined and optimized for small space (about 160 bytes).
// The task is to de-compress the folded pieces for shared library init:
// the de-compressor(s) of the PT_LOAD pieces, and the C-code supervisor
// which adjusts the placement and mapping of the address space.
// The output length is a couple KB for NRV, a few KB for Lzma, 64KB for Zstd.
// This is motivated by the possibility of using multiple de-compressors
// depending on the characteristics of each PT_LOAD, and by the increased size
// and compressability of C-coded de-compressors for Lzma and Zstd
// in contrast to the simple and small assembly-coded NRV.

/* Working registers for local NRV2B */
#define off  %eax  /* XXX: 2GB */
//#define bits %ebx
#define len  %ecx  /* XXX: 2GB */
#define disp %ebp

#define GETBIT call *r_getb  /* %edx */
#define jnextb0 GETBIT; jnc
#define jnextb1 GETBIT; jc

/* rotate next bit (now in Carry) into bottom bit of reg */
#define getnextb(reg) GETBIT; adcl reg,reg

/* Arguments to decompress() */
#define src  %esi
#define lsrc %ecx
#define dst  %edi
//#define ldst %edx  /* Out: actually a reference: &len_dst */

        lea D_FOLD(%esp),dst  // &unfolded code
        push old_sp  // MATCH_16  save register
        mov    sz_cpr(foldi),lsrc
        lea sz_b_info(foldi),src
#undef foldi

decompress:  // inlined: (uchar const *src, uint len, uchar *dst /*, u32 &ldst, uint method */)
        add src,lsrc; push lsrc  // MATCH_05  &input_eof
        //subq src,lsrc  // restore the value of lsrc; dead for inlined nrv2b

//%esp:
//  MATCH_05  &input_eof
//  MATCH_16  old_sp
//  space for de-compressed code

//old_sp:
//  MATCH_10  len unfolded_code
//  MATCH_14  &so_info
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        or $~0,disp  // -1: initial dispacement
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

        sub $ 3,off; jc len_n2b  # use previous offset
        shl $ 8,off; lodsb  # off is %eax, so 'lodsb' is "off |= *src++;"
        xor $~0,off; jz eof_n2b
        mov off,disp  # XXX: 2GB
len_n2b:
        lea 1(len),off  # [len= 0] off= 1
        getnextb(len); getnextb(len)  # two bits; cc set on result
        jnz gotlen_n2b  # raw 1,2,3 ==> 2,3,4
        mov off,len  # len= 1, the msb
        add $3-1,off  # raw 2.. ==> 5..
lenmore_n2b:
        getnextb(len)
        jnextb0 lenmore_n2b
gotlen_n2b:
        cmp $-0xd00,disp  # XXX: 2GB
        adc off,len  # len += off + (disp < -0xd00)
        push %esi  // MATCH_06
          lea (%edi,disp),%esi
          rep; movsb
        pop %esi  // MATCH_06
        jmp top_n2b

eof_n2b:
        pop %ecx  // MATCH_05  &input_eof
        cmp %ecx,%esi; je 0f; hlt; 0:  // test for ending in correct place
        pop old_sp  // MATCH_16

#define mfd %edi
        mov F_MFD(old_sp),mfd
        mov F_LENU(old_sp),%edx
        mov %esp,%ecx
        mov %edi,%ebx
        mov $__NR_write,%al; call sys_check_al

        mov old_sp,%esp  // de-alloca

        push $0  // arg6
        push mfd  // arg5
        push $MAP_FIXED|MAP_PRIVATE  // arg4
        push $PROT_READ|PROT_EXEC  // arg3
        push F_LENU(%ebp)  // arg2
        push F_ADRU(%ebp)  // arg1
        call oldmmap

        mov mfd,%ebx
        mov $__NR_close,%al; call sys_check_al

        mov F_ADRU(%ebp),%eax
        add $D_FOLD,%eax  // PAGE_MASK, upxfd_path, mflg_data
        jmp *%eax
// %esp:
//  MATCH_13  ptr unfolded_code; for escape hatch
//  MATCH_12  len unfolded code; for escape hatch
//  MATCH_14  &so_info
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp


sys_check_al:
        movzbl %al,%eax
sys_check:  // syscall with error checking; if halt then %eax= -errno, %ecx= __NR_
        push %eax // save __NR_ for debug
        int $0x80
        pop %ecx  // recover __NR_ for debug
        cmp $~0<<12,%eax; jb 0f; hlt; 0:  // -errno
        ret

mmap: .globl mmap  // for C-callable, such as from upx_mmap_and_fd()
oldmmap:  // oldmmap: 6 arguments on stack; remove arguments on return
        push %ebx  // save register
        lea 2*NBPW(%esp),%ebx
        mov $__NR_oldmmap,%al; call sys_check_al
        pop %ebx  // restore register
        ret $6*NBPW

get_upxfn_path: .globl get_upxfn_path
        sub %eax,%eax  // should not be called!
        ret

my_memset: .globl my_memset  // (dst, val, n)
        push %ebp; mov %esp,%ebp
        push %edi
        mov (2+ 2)*NBPW(%ebp),%ecx
        mov (2+ 1)*NBPW(%ebp),%eax
        mov (2+ 0)*NBPW(%ebp),%edi
        rep stosb
        pop %edi
        pop %ebp
        ret

mempcpy: .globl mempcpy  // (dst, src, n)
        push %ebp; mov %esp,%ebp
        push %edi; push %esi
        mov (2+ 2)*NBPW(%ebp),%ecx
        mov (2+ 1)*NBPW(%ebp),%esi
        mov (2+ 0)*NBPW(%ebp),%edi
        rep movsb
        mov %edi,%eax
        pop %esi; pop %edi; pop %ebp
        ret

zfind:
        mov (%edi),%eax; add $NBPW,%edi
        test %eax,%eax; jne zfind
        ret

// IDENTSTR goes here

  section ELFMAINZ
L70:
        pop r_getb  // MATCH_08  &getbit  (also L70ret)
        call L20  // MATCH_09  push $&fold_info
fold_info:  // nice if 4-byte aligned
        .long O_BINFO
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */

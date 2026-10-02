#include "../../MAX_ELF_HDR.h"

#define section .section
NBPW= 4
MAP_FIXED=     0x10

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}

// %esp:
//  MATCH_14  &so_info
//            PMASK
//            ADRU
//            LENU
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

  section SO_HEAD
ZERO:
PAGE_MASK: .int 0xfffff000  // default
qflg_data: .int 0  // QNX vs Linux: MAP_PRIVATE | MAP_ANONYMOUS
upxfn_path:.int 0  // displacement from ZERO

fold_begin: .globl fold_begin
        jmp L05

get_page_mask: .globl get_page_mask
    call 0f; 0: pop %eax
    movl PAGE_MASK-0b(%eax),%eax
    ret

get_upxfn_path: .globl get_upxfn_path  // char * (*)(void)
    call 0f; 0: pop %ecx; lea ZERO-0b(%ecx),%ecx
    mov upxfn_path-ZERO(%ecx),%eax  // offset(upxfn_path)
    test %eax,%eax; jz 1f
    add  %ecx,%eax; 1:  // &path
    ret

L05:  // %esp/ &so_info,PMASK,F_ADRU,F_LENU,8regs,ret_addr,argc
    pop %ecx  // &so_info
    lea (3+8+1)*NBPW(%esp),%eax  // &{argc,argv,envp}
    sub $MAX_ELF_HDR_32,%esp; push %esp  // &elf_tmp
    push %eax  // &{argc,argv,envp}
    push %ecx  // &so_info
    call upx_so_main  // (&so_info, &{argc, argv, envp}, &elf_tmp); returns &escape_hatch
    add $MAX_ELF_HDR_32 + (3+1)*NBPW,%esp  // remove args and PMASK
    mov %eax,%ebp  // save &escape_hatch

#define sys1 ebx
#define sys2 ecx
#define sys3 edx
    pop %sys1  // MATCH_13  ADRU
    pop %sys2  // MATCH_12  LENU
    push $__NR_munmap; pop %eax
    jmp *%ebp  // goto &escape_hatch
        //  syscall
        //  popa
        //  ret

L10:
  section ptr_NEXT
    pop %eax; call *%eax
f_exp:  // start of code for actual de-compressor
// "lea f_exp(%eip)," addressing on x86_64 subsumes the need for code,
// but keep the empty section to unify buildLinuxLoader()

// De-compressor sections inserted here:
// section NRV_HEAD
// section NRV2B
// section NRV2D
// section NRV2E
// section NRV_TAIL
// section LZMA_*
// section ZSTD  future

  section SO_TAIL
  .type eof,@function
  .globl eof
eof:  // end of a compressed extent
        pop %ecx  // &input_eof
        mov %esi,%eax; sub %ecx,%eax  // src -= eof;  // return 0: good; else: bad
        pop %edx;      sub %edx,%edi  // dst -= original dst
        pop %ecx;            movl %edi,(%ecx)  // actual length used at dst  XXX: 4GB
        pop %ebx; pop %ebp
        ret

//
// Subroutines and syscalls needed by upx_so_main
//
my_bkpt: .globl my_bkpt
        int3  // my_bkpt
        ret

memset: .globl memset  // void *memset(void *s, int c, size_t n);
    push %edi  // result = dst
    mov %esi,%eax  // c
    mov %edx,%ecx  // n
    rep; stosb
    pop %eax  // result
    ret

memcpy: .globl memcpy  // void (memcpy(void *dst, void const *src, size_t len)
                   pop %ecx  // ret_addr
    mov %edi,%eax; pop %edi  // dst
    mov %esi,%edx; pop %esi  // src
    xchg (%esp),%ecx  // len
    push %edi  // save eventual return value in slot for original src
    shr %ecx; jnc 0f; movsb; 0:
    shr %ecx; jnc 0f; movsw; 0:
    jz 0f;       rep; movsl; 0:
    mov %eax,%edi  // restore saved register
    mov %edx,%esi  // restore saved register
    pop %eax   // retval (original dst, saved in slot for original src)
    pop %edx  // %edx= ret_addr (saved in slot for original len)
    sub $3*NBPW,%esp // space for dst,src,len
    push %edx  // ret_addr
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

/* 32-bit mode only! */
__NR_read=  3
__NR_write= 4
__NR_open=  5
__NR_close= 6

__NR_memfd_create= 0x164  // 356
__NR_mmap=     90
__NR_mprotect=125
__NR_msync=    0x90  // 144
__NR_munmap=   91
__NR_mremap=  163

__NR_brk=      45

__NR_exit=      1
__NR_readlink= 85


Pmap: .globl Pmap
mmap: .globl mmap  // oldmmap: %ebx -> 6 word parameters
    push %ebx  // save C-lang register
    lea 2*NBPW(%esp),%ebx
    mov (%ebx),%eax  // arg1
    and $0xfff,%eax  // lo fragment  PAGE_SIZE
    sub %eax,    (%ebx)  // page align lo end
    add %eax,NBPW(%ebx)
    movb $ __NR_mmap,%al; call sys_check_al
        mov 0*NBPW(%ebx),%ecx  // requested addr
        test %ecx,%ecx; je 0f  // kernel chose
        testb $MAP_FIXED,3*NBPW(%ebx); je 0f
        cmp %ecx,%eax; je 0f  // addr was preserved
        hlt
0:
    pop %ebx  // restore
    ret

#if DEBUG  //{
  push %sys4  // %e10
  push %e9
  push %e8
  push %ecx; mov %ecx,%arg6
  push %edx; mov %edx,%arg5
  push %esi; mov %esi,%arg4
  push %edi; mov %edi,%arg3
  push %eax; mov %eax,%arg2
  call 0f; .asciz "syscall %p(%p %p  %p %p  %p %p  %p)\n";
0: pop %arg1
  call dprint8
  pop %eax
  pop %edi
  pop %esi
  pop %edx
  pop %ecx
  pop %e8
  pop %e9
  pop %e10
#endif  //}

// Sometimes linux enforces page-aligned address
Pprotect: .globl Pprotect  // from C
    xchg %ebx,1*NBPW(%esp)  // save reg, %ebx= address
    mov %ebx,%ecx  // copy address
    and $~0<<12,%ebx  // page align  PAGE_MASK
    sub %ebx,%ecx  // extra length
    add 2*NBPW(%esp),%ecx  // length
    mov 3*NBPW(%esp),%edx  // bits
    movb $__NR_mprotect,%al; call sys_check_al
    mov 1*NBPW(%esp),%ebx  // restore reg
    ret

Psync: .globl Psync
    xchg %ebx,1*NBPW(%esp)  // save reg, %ebx= address
    mov %ebx,%ecx  // copy address
    and $~0<<12,%ebx  // page align  PAGE_MASK
    sub %ebx,%ecx  // extra length
    add 2*NBPW(%esp),%ecx  // length
    mov 3*NBPW(%esp),%edx  // bits
    movb $__NR_msync,%al; call sys_check_al
    mov 1*NBPW(%esp),%ebx  // restore reg
    ret

Punmap: .globl Punmap  // from C
    push %ebp; mov %esp,%ebp
    push %ebx
    mov (0+2)*NBPW(%ebp),%ebx  // addr
    mov %ebx,%eax; and $-1+ (1<<12),%eax  // PAGE_MASK
    sub %eax,%ebx
    mov (1+2)*NBPW(%ebp),%ecx  // len
    add %eax,%ecx
    movb $__NR_munmap,%al; call sys_check_al
    pop %ebx; pop %ebp
    ret

memfd_create: .globl memfd_create
0:
        mov $__NR_memfd_create,%eax; int $0x80
        test %eax,%eax; js 1f; ret  // success
1:
        test %sys2,%sys2; jz no_memfd  // memfd_create failed twice
        xor  %sys2,%sys2; jmp 0b  // try again without MFD_EXEC
no_memfd:  // so try /dev/shm
O_RDWR= 2
O_DIRECTORY= 0200000  // 0x010000
O_TMPFILE= 020000000  // 0x400000
        call 0f; .int 0700, O_RDWR|O_DIRECTORY|O_TMPFILE; .asciz "/dev/shm"
0:
        pop %esi
        lodsl;                 xchg %eax,%sys3
        lodsl; mov %esi,%sys1; xchg %eax,%sys2
        push $__NR_open; jmp sys_check

mprotect: .globl mprotect  // also Pprotect
        mov %ebx,%eax; and $-1+ (1<<12),%eax  // PAGE_MASK
        sub %eax,%ebx
        add %eax,%ecx
        movb $ __NR_mprotect,%al; 5: jmp 5f
exit: .globl exit
        movb $ __NR_exit,%al; 5: jmp 5f
close: .globl close
        movb $__NR_close,%al; 5: jmp 5f
munmap: .globl munmap
        movb $ __NR_munmap,%al; 5: jmp 5f
Pwrite: .globl Pwrite
        int3
write: .globl write
        movb $__NR_write,%al; 5:
sys_check_al:
        movzbl %al,%eax
sys_check:
        push %eax  // save __NR_ for debug
        int $0x80
        pop %edx  // recover __NR_ for debug
        cmp $-1<<12,%eax; jae 0f; ret; 0:
        hlt

// section SO_MAIN inserted here

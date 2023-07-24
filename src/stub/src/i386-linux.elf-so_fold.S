#include "MAX_ELF_HDR.S"

#define section .section
NBPW= 4

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}

// %esp:
//   MATCH_13  ptr unfolded_code; for escape hatch
//   MATCH_12  len unfolded code; for escape hatch
//   MATCH_14  &so_info:
//     .long offset(.)  // detect relocation
//     .long offset(user DT_INIT)
//     .long offset(escape_hatch)
//     .long offset({l_info; p_info; b_info; compressed data})
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

  section SO_HEAD
fold:
    mov %esp,%eax
    mov 2*NBPW(%eax),%ecx  // &so_info
    sub $MAX_ELF_HDR_32,%esp; push %esp  // &elf_tmp
    add $(3+8+1)*NBPW,%eax; push %eax  // &{argc,argv,envp}
    push %ecx  // &so_info
    call upx_so_main  // (&so_info, &{argc, argv, envp}, &elf_tmp); returns &escape_hatch
    add $MAX_ELF_HDR_32 + 3*NBPW,%esp  // remove args
    mov %eax,%ebp  // save &escape_hatch

    pop %ebx  // MATCH_13  ptr unfolded code
    pop %ecx  // MATCH_12  len unfolded code
    pop %edx  // MATCH_14  discard &so_info
    push $__NR_munmap; pop %eax
    jmp *%ebp  // goto &escape_hatch

    ret  // MATCH_30  ==>escape_hatch:
        //  syscall
        //  pop %ebx
        //  pop %ebp
        //  ???

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
        int3
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

/* 32-bit mode only! */
__NR_read=  3
__NR_write= 4
__NR_open=  5
__NR_close= 6

__NR_memfd_create= 0x164  // 356
__NR_mmap=     90
__NR_mprotect=125
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
    and $0xfff,%eax  // lo fragment
    sub %eax,    (%ebx)  // page align lo end
    add %eax,NBPW(%ebx)
    push $ __NR_mmap; pop %eax
    int $0x80; cmp $-0x1000,%eax; jna 0f; hlt; 0:
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
    and $~0<<12,%ebx  // page align
    sub %ebx,%ecx  // extra length
    add 2*NBPW(%esp),%ecx  // length
    mov 3*NBPW(%esp),%edx  // bits
    push $__NR_mprotect; pop %eax; int $0x80
    cmp $-0x1000,%eax; jna 0f; hlt; 0:
    mov 1*NBPW(%esp),%ebx  // restore reg
    ret

Punmap: .globl Punmap  // from C
    push %ebp; mov %esp,%ebp
    push %ebx
    mov (0+2)*NBPW(%ebp),%ebx  // addr
    mov %ebx,%eax; and $-1+ (1<<12),%eax
    sub %eax,%ebx
    mov (1+2)*NBPW(%ebp),%ecx  // len
    add %eax,%ecx
    push $__NR_munmap; pop %eax; int $0x80
    cmp $-0x1000,%eax; jna 0f; hlt; 0:
    pop %ebx; pop %ebp
    ret

memfd_create: .globl memfd_create
        push $__NR_memfd_create; 5: jmp 5f
mprotect: .globl mprotect
        mov %ebx,%eax; and $-1+ (1<<12),%eax
        sub %eax,%ebx
        add %eax,%ecx
        push $ __NR_mprotect; 5: jmp 5f
exit: .globl exit
        push $ __NR_exit; 5: jmp 5f
close: .globl close
        push $__NR_close; 5: jmp 5f
munmap: .globl munmap
        push $ __NR_munmap; 5: jmp 5f
Pwrite: .globl Pwrite
write: .globl write
        push $__NR_write; 5:
        pop %eax
        int $0x80
        ret


// section SO_MAIN inserted here

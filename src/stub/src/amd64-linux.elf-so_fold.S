#include "arch/amd64/regs.h"
#include "arch/amd64/macros.S"
#include "MAX_ELF_HDR.S"
NBPW= 8

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}

// %rsp:
//   MATCH_11  ptr unfolded_code; for escape hatch
//   MATCH_10  len unfolded code; for escape hatch
//   MATCH_14  &so_info:
//     .word offset(.)  // detect relocation
//     .word offset(user DT_INIT)
//     .word offset(escape_hatch)
//     .word offset({l_info; p_info; b_info; compressed data})
// %rbp:
//   MATCH_02  saved %rbp
//   MATCH_03  saved %rbx
//   MATCH_00  argc
//   MATCH_01  argv
//   MATCH_07  envp

  section SO_HEAD
fold:
    pop %rbx  // MATCH_11  ptr unfolded code
    pop %rbp  // MATCH_10  len unfolded code
    pop %arg1  // MATCH_14  &so_info
    lea 2*NBPW(%rsp),%arg2  // &{argc, argv, envp}
    sub $MAX_ELF_HDR_64,%rsp; mov %rsp,%arg3  // space for Elf64_Ehdr and Elf64_Phdrs
    call upx_so_main  // (&so_info, &{argc, argv, envp}, elf_tmp); returns &escape_hatch
    add $MAX_ELF_HDR_64,%rsp

    push %rbp; pop %arg2  // len unfolded code
    push %rbx; pop %arg1  // ptr unfolded code
    pop %rbp  // MATCH_02  restore
    pop %rbx  // MATCH_03  restore
    push %rax  // MATCH_30  &escape_hatch
    push $__NR_munmap; pop %rax
    ret  // MATCH_30  ==>escape_hatch:
        //  syscall
        //  pop %arg1  // MATCH_00  argc
        //  pop %arg2  // MATCH_01  argv
        //  pop %arg3  // MATCH_07  envp
        //  ret  // ==> user_DT_INIT

  section ptr_NEXT
//    pop %rax; call *%rax
// "lea f_exp(%rip)," addressing on x86_64 subsumes the need for code,
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
        pop %rcx  // &input_eof
        movq %rsi,%rax; subq %rcx,%rax  // src -= eof;  // return 0: good; else: bad
        pop %rdx;       subq %rdx,%rdi  // dst -= original dst
        pop %rcx;            movl %edi,(%rcx)  // actual length used at dst  XXX: 4GB
        pop %rbx; pop %rbp
        ret

//
// Subroutines and syscalls needed by upx_so_main
//
my_bkpt: .globl my_bkpt
        int3  // my_bkpt
        ret

memset: .globl memset  // void *memset(void *s, int c, size_t n);
    push %rdi  // result = dst
    mov %esi,%eax  // c
    mov %edx,%ecx  // n
    rep; stosb
    pop %rax  // result
    ret

memcpy: .globl memcpy  // void *memcpy(void *dst, void const *src, size_t n)
    push %rdi  // result = dst
    mov %edx,%eax; and $7,%eax  // n mod 8
    mov %rdx,%rcx; shr $3,%rcx; rep movsq  // n/8 whole 8-byte
    xchg %eax,%ecx; rep movsb  // (n mod 8) bytes
    pop %rax  // result
    ret

/* 64-bit mode only! */
__NR_read=  0
__NR_write= 1
//__NR_open=  2
__NR_openat= 257
__NR_close= 3

__NR_mmap=      9
__NR_mprotect= 10
__NR_munmap=   11
__NR_mremap=  216
__NR_memfd_create= 0x13f  // 319
__NR_ftruncate= 0x4d  // 77

__NR_brk=      12

__NR_exit= 60
__NR_readlink= 89


O_BINFO: .globl O_BINFO  # ignored dummy needed by elf-entry.o

Pmap: .globl Pmap
        mov %arg1,%rax; and $0xfff,%eax
        sub %rax,%arg1  # page align lo end
        add %rax,%arg2
mmap: .globl mmap
        push $ __NR_mmap
sys_4args:
        movq %arg4,%sys4
sysgo:  # NOTE: kernel demands 4th arg in %sys4, NOT %arg4
        pop %rax  # syscall __NR_
#if DEBUG  //{
  push %sys4  // %r10
  push %r9
  push %r8
  push %rcx; mov %rcx,%arg6
  push %rdx; mov %rdx,%arg5
  push %rsi; mov %rsi,%arg4
  push %rdi; mov %rdi,%arg3
  push %rax; mov %rax,%arg2
  call 0f; .asciz "syscall %p(%p %p  %p %p  %p %p  %p)\n";
0: pop %arg1
  call dprint8
  pop %rax
  pop %rdi
  pop %rsi
  pop %rdx
  pop %rcx
  pop %r8
  pop %r9
  pop %r10
#endif  //}
        syscall; cmp $-4096,%rax; jb 0f; int3; 0:
        ret

Punmap: .globl Punmap  // page-align the lo end
        mov %arg1,%rax; and $0xfff,%eax
        sub %rax,%arg1
        add %rax,%arg2
munmap: .globl munmap
        push $__NR_munmap; 5: jmp sysgo
exit: .globl exit
        push $ __NR_exit; 5: jmp 5f
ftruncate: .globl ftruncate
        push $__NR_ftruncate; 5: jmp 5f
memfd_create: .globl memfd_create
        push $__NR_memfd_create; 5: jmp 5f
close: .globl close
        push $ __NR_close; 5: jmp 5f
openat: .globl openat
        push $ __NR_openat; 5: jmp 5f
Pwrite: .globl Pwrite
write: .globl write
        push $__NR_write; 5: jmp 5f
read: .globl read
        push $ __NR_read; 5: jmp 5f

// Sometimes Linux enforces page-aligned address for mprotect
Pprotect: .globl Pprotect
        mov %rdi,%rax; and $-1+ (1<<12),%rax
        sub %rax,%rdi
        add %rax,%rsi
mprotect: .globl mprotect
        push $ __NR_mprotect; 5: jmp sysgo

// section SO_MAIN inserted here

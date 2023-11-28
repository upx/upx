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

get_upxfn_path: .globl get_upxfn_path  // char * (*)(void)
    call 0f; 0: pop %edx
    add $fold -2*NBPW -0b,%edx  // link-time 0
    mov 1*NBPW(%edx),%eax  // distance forward to &name
    test %eax,%eax; je 1f  // nullptr
    add %edx,%eax  // &name
1:
    ret

    ret  // MATCH_30  ==>escape_hatch:
        //  syscall
        //  pop %ebx
        //  pop %ebp
        //  ???

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
__NR_exit=  1
__NR_read=  3
__NR_write= 4
__NR_open=  5
__NR_close= 6

__NR_brk=      45
__NR_readlink= 85
__NR_oldmmap=  90
__NR_munmap=   91
__NR_mprotect=125
__NR_mremap=  163

__NR_memfd_create= 0x164  // 356

// These system calls are called from C (so_main.c, upxfd_android.c).
// Input parameters are on the stack.
// Registers %ebx,%esp,%ebp,%esi,%edi must be preserved.

// Sometimes linux enforces page-aligned address
Pprotect: .globl Pprotect  // from C
    nop
mprotect: .globl mprotect
    push %ebx
    push $__NR_mprotect; 4: jmp 4f

Punmap: .globl Punmap
    push %ebx
    push $__NR_munmap; 4: jmp 4f

Pmap: .globl Pmap
    nop
mmap: .globl mmap  // oldmmap: %ebx -> 6 word parameters
    push %ebx
    push $ __NR_oldmmap
4:
    lea 3*NBPW(%esp),%ebx  // &arg1
    mov (%ebx),%eax  // ptr
    and $~(~0<<12),%eax  // offset in page
    sub %eax,    (%ebx)  // ptr
    add %eax,NBPW(%ebx)  // len
    pop %eax  // __NR_
    cmp $__NR_oldmmap,%eax; je 6f  // %ebx -> 6 args
    mov (%ebx),%ebx  // arg1
6:
    jmp 6f

// System calls below take 3 or fewer arguments,
// and %ecx,%edx need not be preserved.
// So always pass 3 arguments.

memfd_create: .globl memfd_create
        push $__NR_memfd_create; 5: jmp 5f
exit: .globl exit
        push $ __NR_exit; 5: jmp 5f
close: .globl close
        push $__NR_close; 5: jmp 5f
munmap: .globl munmap
        push $ __NR_munmap; 5: jmp 5f
Pwrite: .globl Pwrite
write: .globl write
        push $__NR_write
5:
        pop %eax  // __NR_
        push %ebx  // save regsiter
        mov 2*NBPW(%esp),%ebx
6:
        mov 3*NBPW(%esp),%ecx
        mov 4*NBPW(%esp),%edx
sys_check:  // syscall with error checking; if halt then %eax= -errno, %ecx= __NR_
        push %eax // save __NR_ for debug
        int $0x80
        pop %ecx  // recover __NR_ for debug
        cmp $~0<<12,%eax; jb 0f; hlt; 0:  // -errno
        pop %ebx  // restore
        ret

// section SO_MAIN inserted here

#define GLOBAL(sym) sym: .globl sym; _##sym: .globl _##sym

GLOBAL(bswap)
0:
        mov (%rdi),%eax
        .byte 0x0f,0xc8  // bswap eax
        mov %eax,(%rdi)
        sub $4,%rsi
        lea 4(%rdi),%rdi
        ja 0b
        ret

GLOBAL(bzero)
GLOBAL(__bzero)
	xor %eax,%eax  // the value
	mov %esi,%ecx  // the count
	rep; stosb  // *rdi++ = %al
	ret

SYS_exit  =1
SYS_read  =3
SYS_write =4
SYS_open  =5
SYS_close =6

SYS_pread    =0x99
SYS_mmap     =0xc5
SYS_munmap   =0x49
SYS_mprotect =0x4a

// lazy jmps enable compression of this code
GLOBAL(write)
        mov $SYS_write,%al;  .word 0x02eb
GLOBAL(exit)
        mov $SYS_exit,%al;  .word 0x02eb
GLOBAL(mprotect)
        mov $SYS_mprotect,%al; .word 0x02eb
GLOBAL(munmap)
        mov $SYS_munmap,%al; .word 0x02eb
GLOBAL(pread)
        mov $SYS_pread,%al; .word 0x02eb
GLOBAL(close)
        mov $SYS_close,%al; .word 0x02eb
GLOBAL(open)
        mov $SYS_open,%al;  .word 0x02eb
GLOBAL(mmap)
        mov $SYS_mmap,%al;  .word 0x02eb
GLOBAL(read)
        mov $SYS_read,%al

        movzbl %al,%eax; or $0x02000000,%eax
        mov %rcx,%r10
        syscall  // .byte 0x0f,0x05
        jnc 0f
        or $~0,%rax  //mov errno,eax
0:
        ret

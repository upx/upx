#include "ppc_regs.h"

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD= 2048
LINKAREA= 4*4  # SysV C linkage area: (sp, lr); but 16-byte align
/* In:
   r31= &decompress; also 8+ (char *)&(offset to {l_info; p_info; b_info})
*/
fold_begin:
	call L90
#include "ppc_bxx.S"

/* The SysV convention for argument registers after execve is nice:
	a0= argc
	a1= argv
	a2= envp
	a3= auxvp
	a4= fini
	sp= ~0xf & (-2*4 + (void *)&argc)  # 0(sp): old_sp, pc
   Instead, Linux gives only
	sp= &{argc,argv...,0,env...,0,auxv...,strings}  # 16-byte aligned?
   We must figure out the rest, particularly auxvp.
*/
zfind:
	lwz t0,0(a6); addi a6,a6,4
	cmpi cr7,t0,0; bne+ cr7,zfind
	ret
L90:
	mflr a5  # &ppcbxx: f_unfilter
	lwz a6,0(sp)  # sp at execve
	call zfind  # a6= &env
	call zfind  # a6= &Elf32_auxv
        lwz a1,-8(r31)  # total size = offset to {l_info; p_info; b_info}
	  rlwinm r30,a5,0,0,31-12  # r30= &this_page
	la a2,-OVERHEAD(sp)  # &Elf32_Ehdr temporary space
	mr a4,r31  # &decompress: f_expand
	subf a0,a1,r31  # &l_info
	addi sp,sp,-(LINKAREA+OVERHEAD)
	  rlwinm r29,a0,0,0,31-12  # r29= &our_Elf32_Ehdr
        lwz a3,sz_unc+sz_p_info+sz_l_info(a0)  # sz_elf_headers
	call upx_main  # Out: a0= entry
/* entry= upx_main(l_info *a0, total_size a1, Elf32_Ehdr *a2, sz_ehdr a3,
      f_decomp a4, f_unf a5, Elf32_auxv_t *a6)
*/
	mr r31,a0  # save &entry

	mr a0,r29  # &our_Elf32_Ehdr
	subf a1,r29,r30  # size
	call munmap  # unmap compressed program; /proc/self/exe disappears

	mtlr r31  # entry address
	lmw r2,4+LINKAREA+OVERHEAD(sp)  # restore registers r2 thru r31
	lwz r1,  LINKAREA+OVERHEAD(sp)  # restore r1; deallocate space
	ret  # enter /lib/ld.so.1

SYS_exit=  1
SYS_fork=  2
SYS_read=  3
SYS_write= 4
SYS_open=  5
SYS_close= 6

SYS_brk=       45
SYS_mmap=      90
SYS_munmap=    91
SYS_mprotect= 125

mmap: .globl mmap
	li 0,SYS_mmap
sysgo:
	sc
	bns+ no_fail  # 'bns': branch if No Summary[Overflow]
	li a0,-1  # failure; IGNORE errno
no_fail:
	ret

exit: .globl exit
	li 0,SYS_exit; b sysgo
read: .globl read
	li 0,SYS_read; b sysgo
open: .globl open
	li 0,SYS_open; b sysgo
close: .globl close
	li 0,SYS_close; b sysgo
mprotect: .globl mprotect
	li 0,SYS_mprotect; b sysgo
munmap: .globl munmap
	li 0,SYS_munmap; b sysgo
brk: .globl brk
	li 0,SYS_brk; b sysgo


#ifndef __ppc64macros__
#define __ppc64macros__ 1

.macro          section name
                .section \name
.endm

.macro PUSH4 p1,p2,p3,p4
    stdu \p1,-4*NBPW(sp)
    std  \p2, 1*NBPW(sp)
    std  \p3, 2*NBPW(sp)
    std  \p4, 3*NBPW(sp)
.endm
.macro POP4 p1,p2,p3,p4
    ld  \p1, 0*NBPW(sp)
    ld  \p2, 1*NBPW(sp)
    ld  \p3, 2*NBPW(sp)
    ld  \p4, 3*NBPW(sp)
    addi sp,sp,4*NBPW
.endm
.macro PUSH3 p1,p2,p3
    stdu \p1,-3*NBPW(sp)
    std  \p2, 1*NBPW(sp)
    std  \p3, 2*NBPW(sp)
.endm
.macro POP3 p1,p2,p3
    ld  \p1, 0*NBPW(sp)
    ld  \p2, 1*NBPW(sp)
    ld  \p3, 2*NBPW(sp)
    addi sp,sp,3*NBPW
.endm
.macro POP2 p1,p2
    ld  \p1, 0*NBPW(sp)
    ld  \p2, 1*NBPW(sp)
    addi sp,sp,2*NBPW
.endm
.macro POP1 p1
    ld  \p1, 0*NBPW(sp)
    addi sp,sp,1*NBPW
.endm

#endif

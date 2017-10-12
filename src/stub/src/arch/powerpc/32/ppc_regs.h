#ifndef __PPC_REGS__  /*{*/
#define __PPC_REGS__ 1

#define r0 0
#define r1 1
#define r2 2
#define r3 3
#define r4 4
#define r5 5
#define r6 6
#define r7 7
#define r8 8
#define r9 9
#define r10 10
#define r11 11
#define r12 12
#define r13 13
#define r14 14
#define r15 15
#define r16 16
#define r17 17
#define r18 18
#define r19 19
#define r20 20
#define r21 21
#define r22 22
#define r23 23
#define r24 24
#define r25 25
#define r26 26
#define r27 27
#define r28 28
#define r29 29
#define r30 30
#define r31 31

// http://refspecs.linux-foundation.org/elf/elfspec_ppc.pdf
//
/* Stack pointer */
#define sp 1
SZ_FRAME= 6*4  // (sp,cr,lr, xx,yy,zz) save area per calling convention

/* Subroutine arguments; not saved by callee */
#define a0 3
#define a1 4
#define a2 5
#define a3 6
#define a4 7
#define a5 8
#define a6 9
#define a7 10

/* Scratch (temporary) registers; not saved by callee */
//#define t0  2
#define t1 11
#define t2 12
#define t3 13
/* t4 is unofficial */
#define t4 14

/* branch and link */
#define call bl

/* branch to link register */
#define ret  blr

/* move register */
#define movr mr

#endif  /*} __PPC_REGS__ */


/*
vi:ts=4:et:nowrap
*/


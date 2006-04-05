#ifndef __PPC_REGS__  /*{*/
#define __PPC_REGS__

#define r0 0
#define r1 1
#define r2 2

#define r29 29
#define r30 30
#define r31 31

/* Stack pointer */
#define sp 1

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
#define t0  2
#define t1 11
#define t2 12
#define t3 13

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


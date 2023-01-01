// In LZMA SDK 4.63 file lzma.txt (2008-12-30):
//      LZMA SDK is written and placed in the public domain by Igor Pavlov.
// The creative expression of this hand compilation into assembly language,
// including (but not limited to) code organization and register assignment,
// remains copyright by John F. Reiser and licensed under GNU Lesser General
// Public License (GNU LGPL).

// Hand compiled Copyright (c) 2006-2023 John F. Reiser  (2007-06-18)
// from modified LzmaDecode.c.
// LZMA SDK 4.40 Copyright (c) 1999-2023 Igor Pavlov (2006-05-01)
//
// This file is licensed under either of these two licenses:
//   1) GNU Lesser General Public License (GNU LGPL)
//   2) Common Public License (CPL)
// See files LGPL.txt and CPL.html for the text of the licenses.

#include "macros.S"

kLzmaStreamWasFinishedId= (-1)

kNumTopBits= 24
kTopValue= 1<<kNumTopBits

kNumBitModelTotalBits= 11
kBitModelTotal= (1 << kNumBitModelTotalBits)
kNumMoveBits= 5

kNumPosBitsMax= 4
kNumPosStatesMax= (1 << kNumPosBitsMax)

kLenNumLowBits= 3
kLenNumLowSymbols= (1 << kLenNumLowBits)
kLenNumMidBits= 3
kLenNumMidSymbols= (1 << kLenNumMidBits)
kLenNumHighBits= 8
kLenNumHighSymbols= (1 << kLenNumHighBits)

LenChoice= 0
LenChoice2= (LenChoice + 1)
LenLow= (LenChoice2 + 1)
LenMid= (LenLow + (kNumPosStatesMax << kLenNumLowBits))
LenHigh= (LenMid + (kNumPosStatesMax << kLenNumMidBits))
kNumLenProbs= (LenHigh + kLenNumHighSymbols)

kNumStates= 12
kNumLitStates= 7

kStartPosModelIndex= 4
kEndPosModelIndex= 14
kNumFullDistances= (1 << (kEndPosModelIndex >> 1))

kNumPosSlotBits= 6
kNumLenToPosStates= 4

kNumAlignBits= 4
kAlignTableSize= (1 << kNumAlignBits)

kMatchMinLen= 2

IsMatch= 0
IsRep= (IsMatch + (kNumStates << kNumPosBitsMax))
IsRepG0= (IsRep + kNumStates)
IsRepG1= (IsRepG0 + kNumStates)
IsRepG2= (IsRepG1 + kNumStates)
IsRep0Long= (IsRepG2 + kNumStates)
PosSlot= (IsRep0Long + (kNumStates << kNumPosBitsMax))
SpecPos= (PosSlot + (kNumLenToPosStates << kNumPosSlotBits))
Align= (SpecPos + kNumFullDistances - kEndPosModelIndex)

   LenCoder= (Align + kAlignTableSize)
RepLenCoder= (LenCoder + kNumLenProbs)
Literal=  (RepLenCoder + kNumLenProbs)

// Already #define in lzma_d.S
#ifndef LZMA_LIT_SIZE  //{
#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE  768
#endif  //}

#define call bl  /* subroutine call */

#define symbol  mi
#define mo      mi

#define mi      w0
#define p_in    x1
#define t1      w2
#define t1x     x2
#define t0      w3
#define t0x     x3

#define bit     w4
#define bitx    x4
#define i       w5
#define ix      x5
#define state   w6
#define Range   w7

#define inPtr   x8
#define outPtr  x9
#define prob    x10
#define p       x11

#define Code    w12
#define inLim   x13
#define outLim  x14

#define rep0    w15

#define t2x     x17

#define nowPos          bit  /* temporary only */
#define nowPosx         bitx /* temporary only */
#define distance        bit
#define numDirectBits   bit
#define hibit           bit
#define matchB          i
#define probLen         ix
#define probLit         ix
#define posSlot         posState
#define r_posSlot       t0

#define r_len           i

#define vs x0
vs_ps= 2
vs_lp= 1
vs_lc= 0

#if !defined(PARAMETER_STYLE)  /*{*/
  #define  PARAMETER_STYLE 1
//  Possible choices:
//      1  /*   0 bytes;  1-byte encoding of pb,lp,lc  [default] */
//      2  /* -24 bytes;  2-byte encoding requires no division */
//      3  /* -32 bytes;  separate bytes lc,lp,pb,xx at -4+ probs */
#endif  /*}*/

/* LzmaDecode(x0=vs, x1=inStream, w2=inSize, x3= &inSizeProcessed,
        x4= outStream, w5= outSize, x6= &outSizeProcessed)
*/
LzmaDecode: .globl LzmaDecode
                // save &inSizeProcesesed and caller registers
        PUSH5(x3,x4,x5,x6, lr)
        mov inPtr,x1
        add inLim,x1,w2, uxtw
        mov outPtr,x4
        add outLim,x4,w5, uxtw

#if 1==PARAMETER_STYLE  /*{ [0]: pb*45 + lp*9 + lc */
        mov p,vs
        ldrb w6,[inPtr],#1

        mov w0,#45;  udiv w4,w6,w0  // w4 = quo(w6, 45) = pb
        mul w0,w0,w4; sub w6,w6,w0  // w6 = rem(w6, 45)

        mov w0,#9;   udiv w5,w6,w0  // w5 = quo(w6, 9) = lp
        mul w0,w0,w5; sub w6,w6,w0  // w6 = rem(w6, 9) = lc
#endif  /*}*/

#if 2==PARAMETER_STYLE  /*{ [0]: ((lc + lp)<<3) | pb;  [1]: (lp<<4) | lc */
        mov p,vs
        ldrb w4,[inPtr],#1; and w4,w4,#7  // pb
        ldrb w6,[inPtr],#1; mov w5,w6,lsr #4  // lp
                            and w6,w6,#0xf  // lc

#endif  /*}*/

#if 3==PARAMETER_STYLE  /*{  lc,lp,pb,xx  in separate bytes before probs[] */
        add p,vs,#4
        ldrb w6,[vs, #vs_lc]
        ldrb w5,[vs, #vs_lp]
        ldrb w4,[vs, #vs_ps]
#endif  /*}*/
#undef vs

        add w14,w5,w6  // lp + lc
        mov Range,#~0
        lsl w5,Range,w5; mvn w5,w5  // ~(~0<<lp) == litPosMask
        lsl w4,Range,w4; mvn w4,w4  // ~(~0<<ps) == posStateMask
        mov w3,#1
        mov w2,#1
        mov w1,#1
        mov w0,#1
// FIXME  stmdb sp!,{r0,r1,r2,r3, r4,r5,r6,r7, r8,r9,r10,r11, r12,r14}
//#define rep0    [sp, #0*4]
#define rep1    [sp, #1*4]
#define rep2    [sp, #2*4]
#define rep3    [sp, #3*4]
#define posStateMask    [sp, #4*4]
#define litPosMask      [sp, #5*4]
#define lc      [sp, #6*4]
#define prevB   [sp, #7*4]
#define inBuf   [sp, #8*4]
#define outBuf  [sp, #9*4]
//#define outLim  [sp, #10*4]
#define posState [sp, #11*4]
//#define inLim   [sp, #12*4]
#define m_len   [sp, #13*4]

#define  inSizeProcessed [sp, #(14     )*4]
#define outSizeProcessed [sp, #(14+10+2)*4]

        mov state,#0
        str state,prevB


        mov w2,#LZMA_LIT_SIZE
        mov w0,#LZMA_BASE_SIZE
        lsl w2,w2,w14  // LZMA_LIT_SIZE << (lp + lc)
        add w2,w2,w0
        mov x0,#(kBitModelTotal>>1)  // 0.5 starting probability
        mov x1,p
        orr x0,x0,x0,lsl #16
        orr x0,x0,x0,lsl #32
L10:
        str x0,[x1],#4*2 // 4 at a time
        subs w2,w2,#4; bgt L10

        add x0,inPtr,#5  // sentinel
L14:
        call rcInit2; cmp x0,inPtr; bne L14

L200:  // main loop
        ldr t0x,outBuf
        ldr t1,posStateMask
        sub nowPosx,outPtr,t0x
        and mi,nowPos,t1
        str mi,posState
        add mi,mi,state, lsl #kNumPosBitsMax
        add p_in,p,#IsMatch<<1
        call rcGetBit_mi0; bne L270

        ldr t0,litPosMask
        ldr t1,lc
        and t0,t0,nowPos
        ldrb mi,prevB
        lsl t0,t0,t1  // (nowPos & litPosMask)<<lc
        neg t1,t1; add t1,t1,#8  // rsb t1,t1,#8
        lsr t1,mi,t1  // prevB >> (8- lc)
        add t0,t0,t1
        add prob,   p,#Literal<<1
        add t0,t0,t0,lsl #1  // *3
        uxtw t0x,t0
        mov symbol,#1
        add prob,prob,t0x,lsl #1+ 8  // *768 *2

        cmp state,#kNumLitStates; blo L240
L205:
        sxtw t0x,rep0
        neg  t0x,t0x
        ldrb matchB,[outPtr,t0x]
L210:  // symbol === mi === mo
        lsl matchB,matchB,#1
        add p_in,prob,#0x100<<1
        and bit,matchB,#0x100
        uxtw t0x,bit
        add p_in,p_in,t0x,lsl #1
        call rcGetBit_mi
        and t0,symbol,#1
        cmp t0,bit,lsr #8; bne L243  // break
        cmp symbol,#0x100; blo L210
        b L245
L240:  // symbol === mi === mo
        mov p_in,prob
        call rcGetBit_mi
L243:
        cmp symbol,#0x100; blo L240
L245:
                       mov  t1,#3; mov t0,#6
        cmp state,#10; csel t0,t1,   t0,lo
        cmp state,# 4; csel t0,state,t0,lo
        sub state,state,t0
        b L298  // assumes symbol===w0
L270:
        add p_in,p,#IsRep<<1
        call rcGetBit_state0; bne L290
        ldr t0,rep2
        ldr t1,rep1
        str t0,rep3
        str t1,rep2
        str rep0,rep1
        mov t0,#0
        cmp state,#kNumLitStates
        mov  state,#3
        csel state,t0,state,lo
        add prob,   p,#LenCoder<<1
        b L350
L290:
        add p_in,p,#IsRepG0<<1
        call rcGetBit_state0; bne L300
L293:
        ldr t0,posState
        add p_in,p,#IsRep0Long<<1
        add mi,t0,state,lsl #kNumPosBitsMax
        call rcGetBit_mi0; bne L340
L295:
        mov t0,#9
        cmp state,#kNumLitStates
        mov  state,#11
        csel state,t0,state,lo
L297:
        ldr t0x,outBuf
        sub nowPosx,outPtr,t0x
        cmp nowPos,rep0; blo lzmaDataError
        sxtw t0x,rep0
        neg t0x,t0x
        ldrb w0,[outPtr,t0x]
L298:
        strb w0,[outPtr],#1
        b L519
L300:
        add p_in,p,#IsRepG1<<1
        call rcGetBit_state0; ldr distance,rep1; beq L330
L310:
        add p_in,p,#IsRepG2<<1
        call rcGetBit_state0; ldr distance,rep2; beq L325
L320:
        ldr t0,rep2
        ldr distance,rep3
        str t0,rep3
L325:
        ldr t0,rep1
        str t0,rep2
L330:
        str rep0,rep1
        mov rep0,distance
L340:
        mov t0,#8
        cmp state,#kNumLitStates
        mov  state,#11
        csel state,t0,state,lo
        add prob,   p,#RepLenCoder<<1
L350:
        add p_in,prob,#LenChoice<<1
        call rcGetBit_0; bne L360
        ldr t0,posState
        add probLen,prob,#LenLow<<1
        uxtw t0x,t0
        mov t1,#0
        add probLen,probLen,t0x,lsl #1+ kLenNumLowBits
        mov hibit,#1<<kLenNumLowBits
        b L390
L360:
        add p_in,prob,#LenChoice2<<1
        call rcGetBit_0; bne L370
        ldr t0,posState
        add probLen,prob,#LenMid<<1
        uxtw t0x,t0
        mov t1,#kLenNumLowSymbols
        add probLen,probLen,t0x,lsl #1+ kLenNumMidBits
        mov hibit,#1<<kLenNumMidBits
        b L390
L370:
        add probLen,prob,#LenHigh<<1
        mov t1,#kLenNumLowSymbols + kLenNumMidSymbols
        mov hibit,#1<<kLenNumHighBits
L390:
        str t1,m_len
        mov mi,#1
L395:  // RangeDecoderBitTreeDecode
        mov p_in,probLen
        call rcGetBit_mi; subs t0,mo,hibit; blo L395
        ldr r_len,m_len
        add r_len,r_len,t0
        str r_len,m_len
        cmp state,#4; bhs L500
/*L400:*/
        add state,state,#kNumLitStates
        mov     t0,#kNumLenToPosStates -1
        cmp  r_len,#kNumLenToPosStates
        csel r_len,t0,r_len,hs
        uxtw t0x,r_len
        add probLit,p,t0x,lsl #1+ kNumPosSlotBits
        add probLit,probLit,#PosSlot<<1
        mov mi,#1
        mov hibit,#1<<kNumPosSlotBits
L403:  // RangeDecoderBitTreeDecode
        mov p_in,probLit
        call rcGetBit_mi; subs r_posSlot,mo,hibit; blo L403
        str r_posSlot,posSlot

        cmp r_posSlot,#kStartPosModelIndex; blo L460
L405:
        lsr numDirectBits,r_posSlot,#1
        sub numDirectBits,numDirectBits,#1
        and rep0,r_posSlot,#1
        orr rep0,rep0,#2
        cmp r_posSlot,#kEndPosModelIndex; bhs L410
L407:
        lsl rep0,rep0,numDirectBits
        add prob,p,   #(SpecPos -1)<<1
        sub t0,rep0,r_posSlot  // r_posSlot dies
        uxtw t0x,t0
        add prob,prob,t0x,lsl #1
        b L438
L410:
        sub numDirectBits,numDirectBits,#kNumAlignBits
L420:
        call rcNormalize
        lsr Range,Range,#1
        subs t0,Code,Range
        csel Code,t0,Code,hs  // if (Code>=Range) Code-=Range;
        adc rep0,rep0,rep0  // rep0 = (rep0<<1) + (Code>=Range)
L430:
        subs numDirectBits,numDirectBits,#1; bne L420
        add prob,p,   #Align<<1
        lsl rep0,rep0,#kNumAlignBits
        mov numDirectBits,#kNumAlignBits
L438:
        mov i,#1
        mov mi,#1
L440:
        mov p_in,prob; call rcGetBit_mi
        tst mo,#1; beq L445
        orr rep0,rep0,i
L445:
        lsl i,i,#1
        subs numDirectBits,numDirectBits,#1; bne L440
        b L465
L450:
L460:
        ldr rep0,posSlot
L465:
        adds rep0,rep0,#1
#if 0  /*{ only for the stream version */
        bne L470
        mov t0,#kLzmaStreamWasFinishedId
        str t0,m_len
        b L530
L470:
#endif  /*}*/
        ldr r_len,m_len
L500:
        ldr t0x,outBuf
        add r_len,r_len,#kMatchMinLen
        sub t0x,outPtr,t0x  // nowPos
        cmp rep0,t0; bhi lzmaDataError
        sxtw t0x,rep0
        neg  t0x,t0x
L510:  // const t0x= -rep0;
        ldrb w0,[outPtr,t0x]
        strb w0,[outPtr],#1
        cmp outPtr,outLim; bhs L530
        subs r_len,r_len,#1; bne L510
        // FIXME:  prfm PLDL1KEEP,outPtr,#32  // fetch next cache line
L519:
        strb w0,prevB  // implicit &0xFF
L520:  // bottom of while loop
        cmp outPtr,outLim; blo L200
L530:
        call rcNormalize
        mov w0,#0  // success
lzmaExit:
        ldr t1x,inBuf
        sub t0x,inPtr,t1x
        ldr t1x,inSizeProcessed
        str t0,[t1x]

        ldr t1x,outBuf
        sub t0x,outPtr,t1x
        ldr t1x,outSizeProcessed
        str t0,[t1x]

        add sp,sp,#(14+1)*4
// FIXME        ldmia sp!,{r4,r5,r6,r7, r8,r9,r10,r11, pc}

lzmaDataError:
        mov w0,#1  // failure
        b lzmaExit


rcNormalize:
        lsr t0,Range,#kNumTopBits
        cbnz t0,retNorm
rcLoad:
        cmp inPtr,inLim
        lsl Range,Range,#8
        bhs lzmaDataError
rcInit2:
        ldrb t0,[inPtr],#1
        orr Code,t0,Code, lsl #8
retNorm:
        ret

rcGetBit_state0:  // rcGetBit(0, state + p_in)
        mov mi,state
rcGetBit_mi0:  // rcGetBit(0, mi + p_in)
        add p_in,p_in,mi, uxtw #1
rcGetBit_0:  // rcGetBit(0, p_in)
        mov mi,#0
rcGetBit_mi:  // rcGetBit(mi, mi + p_in)
        add p_in,p_in,mi, uxtw #1
rcGetBit:  // Out: CC set on mo
        lsr t0,Range,#kNumTopBits
        mov t2x,lr  // save lr if need rcLoad
        adr lr,rcGetBitCont; cbz t0,rcLoad  // conditional subroutine call
rcGetBitCont:
#define starp t0
#define bound t1
#define y0tmp t1
        ldrh starp,[p_in]
        lsr y0tmp,Range,#kNumBitModelTotalBits
        mul bound,starp,y0tmp
        cmp Code,bound; bhs rcGB1
rcGB0: // Code < bound
        mov Range,bound  // 'bound' dies
        mov y0tmp,#kBitModelTotal
        sub y0tmp,y0tmp,starp
        adcs mo,mi,mi  // mo = (mi<<1) | (Code >= bound);  set CC
        add starp,starp,y0tmp, lsr #kNumMoveBits
        strh starp,[p_in]
        ret t2x
rcGB1: // Code >= bound
        sub Code,Code,bound
        sub Range,Range,bound
        sub starp,starp,starp, lsr #kNumMoveBits
        adcs mo,mi,mi  // mo = (mi<<1) | (Code >= bound);  set CC
        strh starp,[p_in]
        ret t2x
#undef y0tmp
#undef bound
#undef starp
#undef t1x
#undef t1
#undef t0x
#undef t0

// vi:ts=8:et

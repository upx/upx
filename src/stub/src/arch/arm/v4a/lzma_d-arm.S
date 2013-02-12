// In LZMA SDK 4.63 file lzma.txt (2008-12-30):
//      LZMA SDK is written and placed in the public domain by Igor Pavlov.
// The creative expression of this hand compilation into assembly language,
// including (but not limited to) code organization and register assignment,
// remains copyright by John F. Reiser and licensed under GNU Lesser General
// Public License (GNU LGPL).

// Hand compiled Copyright (c) 2006-2013 John F. Reiser  (2007-06-18)
// from modified LzmaDecode.c.
// LZMA SDK 4.40 Copyright (c) 1999-2013 Igor Pavlov (2006-05-01)
//
// This file is licensed under either of these two licenses:
//   1) GNU Lesser General Public License (GNU LGPL)
//   2) Common Public License (CPL)
// See files LGPL.txt and CPL.html for the text of the licenses.

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

#define hi8(x) ((x) - lo8(x))
#define lo8(x) (0xff&(x))
//#define lo8(x) ((0xff & ((x) >> lg4(x))) << lg4(x))

//already #define    LZMA_BASE_SIZE= Literal  /* 1846 */
//already #define    LZMA_LIT_SIZE=  768

#ifndef HAS_ARM_PLD  /*{*/
#define HAS_ARM_PLD 0
#endif  /*}*/

#define call bl  /* subroutine call */

#define symbol  mi
#define mo      mi

#define mi      r0
#define p_in    r1
#define t1      r2
#define t0      r3

#define bit     r4
#define i       r5
#define state   r6
#define Range   r7

#define inPtr   r8
#define outPtr  r9
#define prob    r10
#define p       r11

#define Code    r12

#define nowPos          bit  /* temporary only */
#define distance        bit
#define numDirectBits   bit
#define hibit           bit
#define matchB          i
#define probLen         i
#define probLit         i
#define posSlot         posState
#define r_posSlot       t0

#define r_len           i
#define r_rep0          p_in

#define vs r0
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

/* LzmaDecode(r0=vs, r1=inStream, r2=inSize, r3= &inSizeProcessed,
        [sp,#0]= outStream, [sp,#4]= outSize, [sp,#8]= &outSizeProcessed)
*/
LzmaDecode: .globl LzmaDecode
                // save &inSizeProcesesed and caller registers
        stmdb sp!,{r3, r4,r5,r6,r7, r8,r9,r10,r11, lr}
        add r12,r2,r1  // inLim
        ldr outPtr,[sp, #10*4 + 0*4]  // outStream
        ldr r7,[sp, #10*4 + 1*4]  // outSize
        mov inPtr,r1
        add r10,r7,outPtr  // outLim

#if 1==PARAMETER_STYLE  /*{ [0]: pb*45 + lp*9 + lc */
        mov p,vs
        ldrb r6,[inPtr],#1

        mov r4,#0  // r4= pb;
1:
        cmp r6,#45; subhs r6,r6,#45; addhs r4,r4,#1; bhs 1b

        mov r5,#0  // r5= lp;
1:
        cmp r6,#9; subhs r6,r6, #9; addhs r5,r5,#1; bhs 1b  // r6= lc;
#endif  /*}*/

#if 2==PARAMETER_STYLE  /*{ [0]: ((lc + lp)<<3) | pb;  [1]: (lp<<4) | lc */
        mov p,vs
        ldrb r4,[inPtr],#1; and r4,r4,#7  // pb
        ldrb r6,[inPtr],#1; mov r5,r6,lsr #4  // lp
                            and r6,r6,#0xf  // lc

#endif  /*}*/

#if 3==PARAMETER_STYLE  /*{  lc,lp,pb,xx  in separate bytes before probs[] */
        add p,vs,#4
        ldrb r6,[vs, #vs_lc]
        ldrb r5,[vs, #vs_lp]
        ldrb r4,[vs, #vs_ps]
#endif  /*}*/
#undef vs

        add r14,r5,r6  // lp + lc
        mvn Range,#0  // ~0
        mvn r5,Range,lsl r5  // ~(~0<<lp) == litPosMask
        mvn r4,Range,lsl r4  // ~(~0<<ps) == posStateMask
        mov r3,#1
        mov r2,#1
        mov r1,#1
        mov r0,#1
        stmdb sp!,{r0,r1,r2,r3, r4,r5,r6,r7, r8,r9,r10,r11, r12,r14}
#define rep0    [sp, #0*4]
#define rep1    [sp, #1*4]
#define rep2    [sp, #2*4]
#define rep3    [sp, #3*4]
#define posStateMask    [sp, #4*4]
#define litPosMask      [sp, #5*4]
#define lc      [sp, #6*4]
#define prevB   [sp, #7*4]
#define inBuf   [sp, #8*4]
#define outBuf  [sp, #9*4]
#define outLim  [sp, #10*4]
#define posState [sp, #11*4]
#define inLim   [sp, #12*4]
#define m_len   [sp, #13*4]

#define  inSizeProcessed [sp, #(14     )*4]
#define outSizeProcessed [sp, #(14+10+2)*4]

        mov state,#0
        str state,prevB


        mov r2,#LZMA_LIT_SIZE
        mov r2,r2, lsl r14  // LZMA_LIT_SIZE << (lp + lc)
        add r2,r2,#lo8(LZMA_BASE_SIZE)
        add r2,r2,#hi8(LZMA_BASE_SIZE)  // nProbs
        mov r1,p
        mov r0,   #(kBitModelTotal>>1)
        orr r0,r0,#(kBitModelTotal>>1)<<16
L10:
        str r0,[r1],#4
        subs r2,r2,#2; bgt L10

        add r0,inPtr,#5  // sentinel
L14:
        call rcInit2; cmp r0,inPtr; bne L14

L200:  // main loop
        ldr t0,outBuf
        ldr t1,posStateMask
        sub nowPos,outPtr,t0
        and mi,nowPos,t1
        str mi,posState
        add mi,mi,state, lsl #kNumPosBitsMax
        add p_in,p,#IsMatch<<1
        call rcGetBit_mi0; bne L270

        ldr t0,litPosMask
        ldr t1,lc
        and t0,t0,nowPos
        ldrb mi,prevB
        mov t0,t0,lsl t1  // (nowPos & litPosMask)<<lc
        rsb t1,t1,#8
        add t0,t0,mi,lsr t1  // + (prevB >> (8- lc))
        add prob,   p,#lo8(Literal<<1)
        add t0,t0,t0,lsl #1  // *3
        add prob,prob,#hi8(Literal<<1)
        mov symbol,#1
        add prob,prob,t0,lsl #1+ 8  // *768 *2

        cmp state,#kNumLitStates; blo L240
L205:
        ldr r_rep0,rep0
        ldrb matchB,[outPtr, -r_rep0]
L210:  // symbol === mi === mo
        mov matchB,matchB,lsl #1
        add p_in,prob,#0x100<<1
        and bit,matchB,#0x100
        add p_in,p_in,bit,lsl #1
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
                       mov   t0,#6
        cmp state,#10; movlo t0,#3
        cmp state,# 4; movlo t0,state
        sub state,state,t0
        b L298  // assumes symbol===r0
L270:
        add p_in,p,#IsRep<<1
        call rcGetBit_state0; bne L290
        ldr t0,rep2
        ldr t1,rep1
        ldr r_rep0,rep0
        str t0,rep3
        str t1,rep2
        str r_rep0,rep1
        cmp state,#kNumLitStates
        mov   state,#3
        movlo state,#0
        add prob,   p,#lo8(LenCoder<<1)
        add prob,prob,#hi8(LenCoder<<1)
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
        cmp state,#kNumLitStates
        mov   state,#11
        movlo state,#9
L297:
        ldr t0,outBuf
        ldr r_rep0,rep0
        sub nowPos,outPtr,t0
        cmp nowPos,r_rep0; blo lzmaDataError
        ldrb r0,[outPtr, -r_rep0]
L298:
        strb r0,[outPtr],#1
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
        ldr r_rep0,rep0
        str distance,rep0
        str r_rep0,rep1
L340:
        cmp state,#kNumLitStates
        mov   state,#11
        movlo state,#8
        add prob,   p,#lo8(RepLenCoder<<1)
        add prob,prob,#hi8(RepLenCoder<<1)
L350:
        add p_in,prob,#LenChoice<<1
        call rcGetBit_0; bne L360
        ldr t0,posState
        add probLen,prob,#LenLow<<1
        mov t1,#0
        add probLen,probLen,t0,lsl #1+ kLenNumLowBits
        mov hibit,#1<<kLenNumLowBits
        b L390
L360:
        add p_in,prob,#LenChoice2<<1
        call rcGetBit_0; bne L370
        ldr t0,posState
        add probLen,prob,#LenMid<<1
        mov t1,#kLenNumLowSymbols
        add probLen,probLen,t0,lsl #1+ kLenNumMidBits
        mov hibit,#1<<kLenNumMidBits
        b L390
L370:
        add probLen,prob,#LenHigh<<1
        mov t1,#kLenNumLowSymbols + kLenNumMidSymbols
        mov hibit,$1<<kLenNumHighBits
L390:
        str t1,m_len
        mov mi,#1
L395:  // RangeDecoderBitTreeDecode
        mov p_in,probLen
        call rcGetBit_mi; subs t0,mo,hibit; blo L395
        ldr r_len,m_len
        add r_len,r_len,t0
        str r_len,m_len
        ldr r_rep0,rep0  // rep0 in register from L400 to L500(L510)
        cmp state,#4; bhs L500
/*L400:*/
        add state,state,#kNumLitStates
        cmp   r_len,#kNumLenToPosStates
        movhs r_len,#kNumLenToPosStates -1
        add probLit,p,r_len,lsl #1+ kNumPosSlotBits
        add probLit,probLit,#PosSlot<<1
        mov mi,#1
        mov hibit,#1<<kNumPosSlotBits
L403:  // RangeDecoderBitTreeDecode
        mov p_in,probLit
        call rcGetBit_mi; subs r_posSlot,mo,hibit; blo L403
        str r_posSlot,posSlot

        cmp r_posSlot,#kStartPosModelIndex; blo L460
L405:
        mov numDirectBits,r_posSlot,lsr #1
        sub numDirectBits,numDirectBits,#1
        and r_rep0,r_posSlot,#1
        orr r_rep0,r_rep0,#2
        cmp r_posSlot,#kEndPosModelIndex; bhs L410
L407:
        mov r_rep0,r_rep0,lsl numDirectBits
        add prob,p,   #lo8((SpecPos -1)<<1)
        sub t0,r_rep0,r_posSlot  // r_posSlot dies
        add prob,prob,#hi8((SpecPos -1)<<1)
        add prob,prob,t0,lsl #1
        b L438
L410:
        sub numDirectBits,numDirectBits,#kNumAlignBits
L420:
        call rcNormalize
        mov Range,Range,lsr #1
        cmp        Code,Range
        subhs Code,Code,Range  // if (Code>=Range) Code-=Range;
        adc r_rep0,r_rep0,r_rep0  // r_rep0 = (r_rep0<<1) + (Code>=Range)
L430:
        subs numDirectBits,numDirectBits,#1; bne L420
        add prob,p,   #lo8(Align<<1)
        add prob,prob,#hi8(Align<<1)
        mov r_rep0,r_rep0,lsl #kNumAlignBits
        mov numDirectBits,#kNumAlignBits
L438:
        str r_rep0,rep0
        mov i,#1
        mov mi,#1
L440:
        mov p_in,prob; call rcGetBit_mi
        tst mo,#1; beq L445
        ldr r_rep0,rep0
        orr r_rep0,r_rep0,i
        str r_rep0,rep0
L445:
        mov i,i,lsl #1
        subs numDirectBits,numDirectBits,#1; bne L440
        ldr r_rep0,rep0
        b L465
L450:
L460:
        ldr r_rep0,posSlot
L465:
        adds r_rep0,r_rep0,#1
#if 0  /*{ only for the stream version */
        bne L470
        mov t0,#kLzmaStreamWasFinishedId
        str t0,m_len
        b L530
L470:
#endif  /*}*/
        str r_rep0,rep0
        ldr r_len,m_len
L500:
        ldr t0,outBuf
        add r_len,r_len,#kMatchMinLen
        sub t0,outPtr,t0  // nowPos
        cmp r_rep0,t0; bhi lzmaDataError
        ldr t1,outLim
L510:
        ldrb r0,[outPtr, -r_rep0]
        strb r0,[outPtr],#1
        cmp outPtr,t1; bhs L530
        subs r_len,r_len,#1; bne L510
#if HAS_ARM_PLD  /*{*/
        pld [outPtr,#32]  // fetch next cache line
#endif  /*}*/
L519:
        strb r0,prevB  // implicit &0xFF
L520:  // bottom of while loop
        ldr t1,outLim
        cmp outPtr,t1; blo L200
L530:
        call rcNormalize
        mov r0,#0  // success
lzmaExit:
        ldr t1,inBuf
        sub t0,inPtr,t1
        ldr t1,inSizeProcessed
        str t0,[t1]

        ldr t1,outBuf
        sub t0,outPtr,t1
        ldr t1,outSizeProcessed
        str t0,[t1]

        add sp,sp,#(14+1)*4
        ldmia sp!,{r4,r5,r6,r7, r8,r9,r10,r11, pc}

lzmaDataError:
        mov r0,#1  // failure
        b lzmaExit


rcNormalize:
        cmp Range,#kTopValue
        movhs pc,lr
rcLoad:
        ldr t0,inLim
        mov Range,Range, lsl #8
        cmp t0,inPtr
        beq lzmaDataError
rcInit2:
        ldrb t0,[inPtr],#1
        orr Code,t0,Code, lsl #8
        mov pc,lr

rcGetBit_state0:  // rcGetBit(0, state + p_in)
        mov mi,state
rcGetBit_mi0:  // rcGetBit(0, mi + p_in)
        add p_in,p_in,mi, lsl #1
rcGetBit_0:  // rcGetBit(0, p_in)
        mov mi,#0
rcGetBit_mi:  // rcGetBit(mi, mi + p_in)
        add p_in,p_in,mi, lsl #1
rcGetBit:  // Out: CC set on mo
        cmp Range,#kTopValue
        mov t1,lr
        bllo rcLoad  // conditional subroutine call
#define starp t0
#define bound lr
#define y0tmp lr
        ldrh starp,[p_in]
        mov y0tmp,Range, lsr #kNumBitModelTotalBits
        mul bound,starp,y0tmp
        cmp Code,bound
        movlo Range,      bound
        subhs Range,Range,bound
        rsblo y0tmp,starp,#kBitModelTotal
        subhs Code, Code, bound
        addlo starp,starp,y0tmp, lsr #kNumMoveBits
        subhs starp,starp,starp, lsr #kNumMoveBits
        adcs mo,mi,mi  // mo = (mi<<1) | (Code >= bound);  set CC
        strh starp,[p_in]
#undef y0tmp
#undef bound
#undef starp
        mov pc,t1

// vi:ts=8:et

//      .section        .text.LzmaDecode,"ax",%progbits
//      .align  2
//      .global LzmaDecode
//      .type   LzmaDecode, %function
//LzmaDecode:
        stp     x29, x30, [sp, -112]!
        mov     w8, 1024
        add     x29, sp, 0
        stp     x19, x20, [sp, 16]
        stp     x21, x22, [sp, 32]
        stp     x23, x24, [sp, 48]
        stp     x25, x26, [sp, 64]
        ldp     w19, w15, [x0]
        ldr     w14, [x0, 8]
        ldr     x11, [x0, 16]
        str     wzr, [x3]
        str     wzr, [x6]
        ldr     w7, [x0, 4]
        mov     w0, 768
        add     w7, w19, w7
        lsl     w7, w0, w7
        mov     w0, 1
        adds    w7, w7, 1846
        lsl     w14, w0, w14
        lsl     w0, w0, w15
        sub     w14, w14, #1
        sub     w15, w0, #1
        mov     x0, 0
        beq     .L22
.L79:
        strh    w8, [x11, x0, lsl 1]
        add     x0, x0, 1
        cmp     w7, w0
        bhi     .L79
.L22:
        add     x2, x1, x2, uxtw
        mov     w27, 0
        str     x2, [x29, 96]
        mov     w28, -1
        add     x0, x1, 1
        add     x10, x1, 6
        add     x2, x2, 1
.L20:
        cmp     x0, x2
        mov     x8, x0
        beq     .L41
        str     x0, [x29, 104]
        add     x0, x0, 1
        cmp     x0, x10
        ldrb    w7, [x0, -2]
        orr     w27, w7, w27, lsl 8
        bne     .L20
        stp     x1, x6, [x29, 80]
        mov     x9, x4
        mov     x25, x3
        cbz     w5, .L63
        mov     w23, 1
        mov     w26, 8
        mov     w22, w23
        mov     w21, w23
        mov     w7, w23
        mov     w10, 0
        mov     w13, 0
        mov     w6, 0
        sub     w20, w26, w19
.L60:
        and     w12, w14, w6
        lsl     w8, w10, 4
        add     x18, x29, 96
        mov     w0, 0
        sxtw    x1, w12
        add     x8, x1, x8, sxtw
        lsl     x8, x8, 1
        add     x1, x11, x8
        bl      rcGetBit.2736
        cbnz    w0, .L26
        and     w0, w15, w6
        asr     w8, w13, w20
        lsl     w0, w0, w19
        cmp     w10, 6
        add     w8, w0, w8
        lsl     w0, w8, 2
        sub     w8, w0, w8
        ubfiz   x8, x8, 8, 24
        add     x8, x8, 1846
        add     x8, x11, x8, lsl 1
        ble     .L64
        sub     w0, w6, w7
        mov     w2, 1
        ldrb    w12, [x9, x0]
        b       .L32
.L96:
        cbnz    w13, .L29
.L30:
        cmp     w2, 255
        bgt     .L31
.L32:
        lsl     w12, w12, 1
        mov     w0, w2
        and     w13, w12, 256
        add     x18, x29, 96
        sxtw    x1, w13
        add     x1, x1, 256
        add     x1, x1, x2, sxtw
        add     x1, x8, x1, lsl 1
        bl      rcGetBit.2736
        mov     w2, w0
        tbz     x2, 0, .L96
        cbnz    w13, .L30
.L29:
        cmp     w2, 255
        bgt     .L31
.L33:
        add     x1, x8, x2, sxtw 1
        mov     w0, w2
        add     x18, x29, 96
        bl      rcGetBit.2736
        cmp     w0, 255
        mov     w2, w0
        ble     .L33
        uxtb    w13, w0
        cmp     w10, 3
        strb    w13, [x9, w6, uxtw]
        add     w6, w6, 1
        bgt     .L62
        mov     w10, 0
.L34:
        cmp     w5, w6
        bhi     .L60
.L58:
        mov     w1, 16777215
        cmp     w28, w1
        bhi     .L97
        ldp     x2, x1, [x29, 96]
        cmp     x1, x2
        beq     .L41
        add     x8, x1, 1
        str     x8, [x29, 104]
        lsl     w28, w28, 8
        ldrb    w0, [x1]
        orr     w27, w0, w27, lsl 8
.L25:
        ldr     x0, [x29, 80]
        ldp     x19, x20, [sp, 16]
        sub     x8, x8, x0
        ldr     x0, [x29, 88]
        str     w8, [x25]
        ldp     x21, x22, [sp, 32]
        str     w6, [x0]
        mov     w0, 0
        ldp     x23, x24, [sp, 48]
        ldp     x25, x26, [sp, 64]
        ldp     x29, x30, [sp], 112
        ret
.L64:
        mov     w2, 1
        b       .L33
.L31:
        uxtb    w13, w2
        strb    w13, [x9, w6, uxtw]
        add     w6, w6, 1
.L62:
        cmp     w10, 9
        bgt     .L35
        sub     w10, w10, #3
        b       .L34
.L26:
        sxtw    x13, w10
        add     x18, x29, 96
        add     x13, x13, 192
        mov     w0, 0
        lsl     x13, x13, 1
        add     x1, x11, x13
        bl      rcGetBit.2736
        cbnz    w0, .L36
        mov     w0, 3
        cmp     w10, 6
        mov     w23, w22
        csel    w10, w0, wzr, gt
        mov     w22, w21
        add     x24, x11, 1636
        mov     w21, w7
.L38:
        add     x18, x29, 96
        mov     x1, x24
        mov     w0, 0
        bl      rcGetBit.2736
        cbnz    w0, .L44
        lsl     w12, w12, 3
        mov     w8, -8
        mov     w13, 3
        sxtw    x12, w12
        add     x12, x12, 2
        add     x12, x24, x12, lsl 1
.L45:
        mov     w0, 1
.L47:
        add     x18, x29, 96
        add     x1, x12, x0, uxtw 1
        bl      rcGetBit.2736
        subs    w13, w13, #1
        bne     .L47
        cmp     w10, 3
        add     w8, w0, w8
        bgt     .L48
        mov     w7, 3
        mov     w12, 6
        cmp     w8, w7
        mov     w0, 1
        csel    w7, w8, w7, le
        lsl     w7, w7, 6
        sxtw    x7, w7
        add     x7, x7, 432
        lsl     x7, x7, 1
.L49:
        add     x1, x7, x0, uxtw 1
        add     x18, x29, 96
        add     x1, x11, x1
        bl      rcGetBit.2736
        subs    w12, w12, #1
        bne     .L49
        sub     w7, w0, #64
        add     w10, w10, 7
        cmp     w7, 3
        mov     w12, w7
        ble     .L50
        cmp     w7, 13
        and     w0, w7, 1
        asr     w1, w7, 1
        orr     w7, w0, 2
        bgt     .L51
        sub     w13, w1, #1
        mov     x0, 688
        lsl     w7, w7, w13
        sub     x12, x0, x12, sxtw
        add     x12, x12, x7, uxtw
        add     x12, x11, x12, lsl 1
        sub     x12, x12, #2
.L52:
        mov     w0, 1
        mov     w24, w0
.L57:
        add     x1, x12, x0, sxtw 1
        add     x18, x29, 96
        bl      rcGetBit.2736
        and     w3, w0, 1
        orr     w1, w7, w24
        cmp     w3, wzr
        csel    w7, w1, w7, ne
        subs    w13, w13, #1
        lsl     w24, w24, 1
        bne     .L57
.L50:
        adds    w7, w7, 1
        beq     .L58
.L48:
        cmp     w7, w6
        add     w0, w8, 2
        bhi     .L41
        mov     w1, w6
.L59:
        sub     w2, w6, w7
        sub     w0, w0, #1
        add     w6, w6, 1
        cmp     w0, wzr
        ccmp    w5, w6, 0, ne
        ldrb    w13, [x9, x2]
        strb    w13, [x9, w1, uxtw]
        mov     w1, w6
        bhi     .L59
        b       .L34
.L35:
        sub     w10, w10, #6
        b       .L34
.L36:
        add     x1, x13, 24
        add     x18, x29, 96
        add     x1, x11, x1
        mov     w0, 0
        bl      rcGetBit.2736
        cbnz    w0, .L39
        add     x1, x8, 480
        add     x18, x29, 96
        add     x1, x11, x1
        bl      rcGetBit.2736
        cbnz    w0, .L67
        cbz     w6, .L41
        sub     w0, w6, w7
        cmp     w10, 6
        mov     w10, 11
        ldrb    w13, [x9, x0]
        mov     w0, 9
        strb    w13, [x9, w6, uxtw]
        csel    w10, w10, w0, gt
        add     w6, w6, 1
        b       .L34
.L44:
        add     x18, x29, 96
        add     x1, x24, 2
        mov     w0, 0
        bl      rcGetBit.2736
        mov     w8, w0
        cbnz    w0, .L46
        lsl     w0, w12, 3
        mov     w13, 3
        sxtw    x0, w0
        add     x0, x0, 130
        add     x12, x24, x0, lsl 1
        b       .L45
.L39:
        add     x1, x13, 48
        add     x18, x29, 96
        add     x1, x11, x1
        mov     w0, 0
        bl      rcGetBit.2736
        cbnz    w0, .L98
.L40:
        cmp     w10, 6
        mov     w0, w7
        mov     w10, 11
        mov     w7, w21
        csel    w10, w10, w26, gt
        mov     w21, w0
        add     x24, x11, 2664
        b       .L38
.L46:
        add     x12, x24, 516
        mov     w8, -240
        mov     w13, 8
        b       .L45
.L41:
        ldp     x19, x20, [sp, 16]
        ldp     x21, x22, [sp, 32]
        ldp     x23, x24, [sp, 48]
        ldp     x25, x26, [sp, 64]
        mov     w0, 1
        ldp     x29, x30, [sp], 112
        ret
.L51:
        ldr     x12, [x29, 96]
        sub     w1, w1, #5
        mov     w3, 16777215
.L55:
        cmp     w28, w3
        lsl     w7, w7, 1
        bhi     .L53
        ldr     x2, [x29, 104]
        cmp     x2, x12
        add     x4, x2, 1
        beq     .L41
        str     x4, [x29, 104]
        lsl     w28, w28, 8
        ldrb    w0, [x2]
        orr     w27, w0, w27, lsl 8
.L53:
        lsr     w0, w28, 1
        cmp     w0, w27
        mov     w28, w0
        bhi     .L54
        sub     w27, w27, w0
        orr     w7, w7, 1
.L54:
        subs    w1, w1, #1
        bne     .L55
        lsl     w7, w7, 4
        add     x12, x11, 1604
        mov     w13, 4
        b       .L52
.L67:
        mov     w0, w21
        mov     w21, w7
        mov     w7, w0
        b       .L40
.L98:
        add     x1, x13, 72
        add     x18, x29, 96
        add     x1, x11, x1
        mov     w0, 0
        bl      rcGetBit.2736
        cbnz    w0, .L69
        mov     w0, w21
        mov     w21, w22
        mov     w22, w0
        b       .L40
.L69:
        mov     w0, w21
        mov     w21, w23
        mov     w23, w22
        mov     w22, w0
        b       .L40
.L97:
        ldr     x8, [x29, 104]
        b       .L25
.L63:
        mov     w6, 0
        b       .L25
//      .size   LzmaDecode, .-LzmaDecode

//      .section        .text.rcGetBit.2736,"ax",%progbits
//      .align  2
//      .type   rcGetBit.2736, %function
rcGetBit.2736:
        mov     w3, 16777215
        cmp     w28, w3
        bhi     .L2
        ldp     x4, x3, [x18]
        cmp     x3, x4
        beq     .L5
        add     x4, x3, 1
        str     x4, [x18, 8]
        lsl     w28, w28, 8
        ldrb    w2, [x3]
        orr     w27, w2, w27, lsl 8
.L2:
        ldrh    w3, [x1]
        lsr     w2, w28, 11
        mul     w2, w2, w3
        cmp     w2, w27
        bhi     .L7
        lsl     w0, w0, 1
        sub     w3, w3, w3, lsr 5
        sub     w27, w27, w2
        sub     w28, w28, w2
        strh    w3, [x1]
        add     w0, w0, 1
        ret
.L7:
        mov     w4, 2048
        mov     w28, w2
        sub     w2, w4, w3
        lsl     w0, w0, 1
        add     w3, w3, w2, asr 5
        strh    w3, [x1]
        ret
.L5:
        mov     w0, 1
        ret
//      .size   rcGetBit.2736, .-rcGetBit.2736

//      .ident  "GCC: (GNU) 5.2.1 20151104 (Red Hat Cross 5.2.1-4)"
//      .section        .note.GNU-stack,"",%progbits

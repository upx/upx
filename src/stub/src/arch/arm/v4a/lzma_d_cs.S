	.file	"lzma_d_c.c"
	.section	.text.LzmaDecodeProperties,"ax",%progbits
	.align	2
	.global	LzmaDecodeProperties
	.type	LzmaDecodeProperties, %function
LzmaDecodeProperties:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	cmp	r2, #4
	@ lr needed for prologue
	ble	.L18
	ldrb	r1, [r1, #0]	@ zero_extendqisi2
	cmp	r1, #224
	bls	.L3
.L18:
	mov	r0, #1
	mov	pc, lr
.L3:
	mov	r3, #0
	cmp	r1, #44
	str	r3, [r0, #8]
	bls	.L13
	mov	r2, r3
.L7:
	sub	r3, r1, #45
	and	r1, r3, #255
	cmp	r1, #44
	add	r2, r2, #1
	bhi	.L7
	str	r2, [r0, #8]
.L13:
	mov	r3, #0
	cmp	r1, #8
	str	r3, [r0, #4]
	bls	.L15
	mov	r2, r3
.L11:
	sub	r3, r1, #9
	and	r1, r3, #255
	cmp	r1, #8
	add	r2, r2, #1
	bhi	.L11
	str	r2, [r0, #4]
.L15:
	str	r1, [r0, #0]
	mov	r0, #0
	mov	pc, lr
	.size	LzmaDecodeProperties, .-LzmaDecodeProperties
	.section	.text.LzmaDecode,"ax",%progbits
	.align	2
	.global	LzmaDecode
	.type	LzmaDecode, %function
LzmaDecode:
	@ args = 12, pretend = 0, frame = 52
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, lr}
	sub	sp, sp, #52
	str	r3, [sp, #44]
	mov	r3, #0
	str	r3, [sp, #36]
	ldrb	ip, [r0, #0]	@ zero_extendqisi2
	ldrb	r4, [r0, #1]	@ zero_extendqisi2
	ldrb	lr, [r0, #2]	@ zero_extendqisi2
	str	ip, [sp, #24]
	ldr	ip, [sp, #44]
	str	r3, [ip, #0]
	ldr	ip, [sp, #96]
	str	r3, [ip, #0]
	ldr	r3, [sp, #24]
	ldrb	ip, [r0, #1]	@ zero_extendqisi2
	add	ip, r3, ip
	mov	r3, #768
	mov	r3, r3, asl ip
	add	ip, r3, #1840
	mov	r3, #1
	mov	lr, r3, asl lr
	sub	lr, lr, #1
	str	r3, [sp, #16]
	mov	r3, r3, asl r4
	str	lr, [sp, #32]
	mov	r4, #0
	mov	lr, #1
	sub	r3, r3, #1
	add	r0, r0, #4
	add	ip, ip, #6
	str	r3, [sp, #28]
	str	r0, [sp, #40]
	str	r4, [sp, #20]
	str	lr, [sp, #12]
	str	lr, [sp, #8]
	str	lr, [sp, #4]
	str	r1, [sp, #48]
	cmp	r4, ip
	mov	r1, r4
.L157:
	bcs	.L132
	mov	r3, r1, asl #1
	ldr	r0, [sp, #40]
	add	r1, r1, #1
	mov	lr, #1024	@ movhi
	cmp	r1, ip
	strh	lr, [r3, r0]	@ movhi 
	b	.L157
.L132:
	ldr	r0, [sp, #48]
	mov	r8, #0
	add	r2, r0, r2
	str	r2, [sp, #0]
	mov	r7, r0
	mvn	lr, #0
	mov	r2, r8
.L28:
	ldr	r1, [sp, #0]
	cmp	r7, r1
	add	r2, r2, #1
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	cmp	r2, #4
	orr	r8, r3, r8, asl #8
	ble	.L28
	ldr	r2, [sp, #36]
	ldr	r3, [sp, #92]
	cmp	r2, r3
.L158:
	bcs	.L30
	add	r0, sp, #32
	ldmia	r0, {r0, ip}	@ phole ldm
	ldr	r1, [sp, #20]
	and	r9, ip, r0
	mvn	sl, #-16777216
	add	r3, r9, r1, asl #4
	cmp	lr, sl
	mov	r5, r3, asl #1
	bhi	.L31
	ldr	r2, [sp, #0]
	cmp	r7, r2
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L31:
	ldr	r3, [sp, #40]
	ldrh	r1, [r5, r3]
	mov	r2, r1, asl #16
	mov	r3, lr, lsr #11
	mov	r0, r2, lsr #16
	mul	ip, r0, r3
	cmp	r8, ip
	bcs	.L33
	ldr	lr, [sp, #24]
	rsb	r3, lr, #8
	mov	r3, r4, asr r3
	ldr	lr, [sp, #28]
	ldr	r4, [sp, #36]
	and	r2, r4, lr
	ldr	r4, [sp, #24]
	ldr	lr, [sp, #40]
	add	r3, r3, r2, asl r4
	add	r3, r3, r3, asl #1
	rsb	r2, r0, #2048
	ldr	r0, [sp, #20]
	add	r3, lr, r3, asl #9
	add	r2, r1, r2, asr #5
	add	r4, r3, #3680
	cmp	r0, #6
	strh	r2, [r5, lr]	@ movhi 
	add	r4, r4, #12
	mov	lr, ip
	mov	r9, #1
	ble	.L50
	ldr	r1, [sp, #36]
	ldr	r2, [sp, #16]
	ldr	ip, [sp, #88]
	rsb	r3, r2, r1
	ldrb	fp, [ip, r3]	@ zero_extendqisi2
.L35:
	mov	fp, fp, asl #1
	and	r6, fp, #256
	mov	sl, r9, asl #1
	add	r3, r4, r6, asl #1
	add	r3, r3, sl
	cmp	lr, #16777216
	add	r5, r3, #512
	bcs	.L38
	ldr	r0, [sp, #0]
	cmp	r7, r0
	mov	lr, lr, asl #8
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	orr	r8, r3, r8, asl #8
.L38:
	ldrh	r2, [r5, #0]
	mov	r1, r2, asl #16
	mov	r3, r1, lsr #16
	mov	r0, lr, lsr #11
	mul	ip, r3, r0
	rsb	r3, r3, #2048
	cmp	r8, ip
	add	r3, r2, r3, asr #5
	mov	r9, sl
	sub	r2, r2, r1, lsr #21
	bcs	.L40
	cmp	r6, #0
	strh	r3, [r5, #0]	@ movhi 
	mov	lr, ip
	beq	.L37
	b	.L34
.L40:
	cmp	r6, #0
	strh	r2, [r5, #0]	@ movhi 
	rsb	r8, ip, r8
	add	r9, sl, #1
	rsb	lr, ip, lr
	beq	.L34
.L37:
	cmp	r9, #255
	ble	.L35
	b	.L137
.L34:
	cmp	r9, #255
	bgt	.L137
.L50:
	mov	r5, r9, asl #1
	cmp	lr, #16777216
	mov	r9, r5
	bcs	.L46
	ldr	r1, [sp, #0]
	cmp	r7, r1
	mov	lr, lr, asl #8
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	orr	r8, r3, r8, asl #8
.L46:
	ldrh	r2, [r4, r5]
	mov	r0, r2, asl #16
	mov	r1, lr, lsr #11
	mov	r3, r0, lsr #16
	mul	ip, r3, r1
	rsb	r3, r3, #2048
	cmp	r8, ip
	add	r3, r2, r3, asr #5
	rsb	lr, ip, lr
	sub	r2, r2, r0, lsr #21
	movcc	lr, ip
	rsbcs	r8, ip, r8
	addcs	r9, r5, #1
	strcch	r3, [r4, r5]	@ movhi 
	strcsh	r2, [r4, r5]	@ movhi 
	b	.L34
.L137:
	ldr	r2, [sp, #20]
	ldr	r3, [sp, #36]
	ldr	ip, [sp, #88]
	cmp	r2, #3
	and	r4, r9, #255
	strb	r4, [r3, ip]
	movle	r0, #0
	add	r3, r3, #1
	str	r3, [sp, #36]
	strle	r0, [sp, #20]
	ble	.L29
	ldr	r1, [sp, #20]
	cmp	r1, #9
	ldrgt	r2, [sp, #20]
	suble	r1, r1, #3
	subgt	r2, r2, #6
	strle	r1, [sp, #20]
	strgt	r2, [sp, #20]
	b	.L29
.L33:
	ldr	r4, [sp, #40]
	ldr	r0, [sp, #20]
	rsb	lr, ip, lr
	sub	r3, r1, r2, lsr #21
	add	r6, r4, r0, asl #1
	cmp	lr, sl
	strh	r3, [r5, r4]	@ movhi 
	rsb	r8, ip, r8
	add	r4, r6, #384
	bhi	.L56
	ldr	r1, [sp, #0]
	cmp	r7, r1
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L56:
	ldrh	r2, [r4, #0]
	mov	r0, r2, asl #16
	mov	r3, lr, lsr #11
	mov	r1, r0, lsr #16
	mul	ip, r1, r3
	cmp	r8, ip
	bcs	.L58
	rsb	r3, r1, #2048
	add	r3, r2, r3, asr #5
	strh	r3, [r4, #0]	@ movhi 
	ldr	r2, [sp, #40]
	ldr	r3, [sp, #20]
	ldr	lr, [sp, #8]
	cmp	r3, #6
	ldr	r0, [sp, #12]
	add	r4, r2, #1632
	ldr	r1, [sp, #16]
	movgt	r3, #3
	movle	r3, #0
	str	lr, [sp, #4]
	add	r4, r4, #4
	mov	lr, ip
	str	r3, [sp, #20]
	str	r0, [sp, #8]
	str	r1, [sp, #12]
	b	.L61
.L58:
	rsb	lr, ip, lr
	sub	r3, r2, r0, lsr #21
	cmp	lr, sl
	strh	r3, [r4, #0]	@ movhi 
	rsb	r8, ip, r8
	add	r4, r6, #408
	bhi	.L62
	ldr	r2, [sp, #0]
	cmp	r7, r2
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L62:
	ldrh	r1, [r4, #0]
	mov	r0, r1, asl #16
	mov	r3, lr, lsr #11
	mov	r2, r0, lsr #16
	mul	ip, r2, r3
	cmp	r8, ip
	bcs	.L64
	mov	lr, ip
	ldr	r0, [sp, #20]
	ldr	ip, [sp, #40]
	rsb	r3, r2, #2048
	add	r2, ip, r0, asl #5
	add	r3, r1, r3, asr #5
	add	r2, r2, r9, asl #1
	cmp	lr, sl
	strh	r3, [r4, #0]	@ movhi 
	add	r4, r2, #480
	bhi	.L65
	ldr	r1, [sp, #0]
	cmp	r7, r1
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L65:
	ldrh	r2, [r4, #0]
	mov	r0, r2, asl #16
	mov	r3, lr, lsr #11
	mov	r1, r0, lsr #16
	mul	ip, r1, r3
	cmp	r8, ip
	subcs	r3, r2, r0, lsr #21
	rsbcs	r8, ip, r8
	rsbcs	lr, ip, lr
	strcsh	r3, [r4, #0]	@ movhi 
	bcs	.L72
	rsb	r3, r1, #2048
	add	r3, r2, r3, asr #5
	ldr	r2, [sp, #36]
	cmp	r2, #0
	strh	r3, [r4, #0]	@ movhi 
	mov	lr, ip
	beq	.L155
	ldr	r4, [sp, #16]
	ldr	r0, [sp, #20]
	ldr	ip, [sp, #88]
	rsb	r3, r4, r2
	ldrb	r4, [ip, r3]	@ zero_extendqisi2
	cmp	r0, #6
	movgt	r0, #11
	movle	r0, #9
	str	r0, [sp, #20]
	strb	r4, [r2, ip]
	add	r2, r2, #1
	str	r2, [sp, #36]
	b	.L29
.L64:
	rsb	lr, ip, lr
	sub	r3, r1, r0, lsr #21
	cmp	lr, sl
	strh	r3, [r4, #0]	@ movhi 
	rsb	r8, ip, r8
	add	r4, r6, #432
	bhi	.L73
	ldr	r1, [sp, #0]
	cmp	r7, r1
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L73:
	ldrh	r2, [r4, #0]
	mov	r0, r2, asl #16
	mov	r3, lr, lsr #11
	mov	r1, r0, lsr #16
	mul	ip, r1, r3
	cmp	r8, ip
	bcs	.L75
	rsb	r3, r1, #2048
	add	r3, r2, r3, asr #5
	ldr	r2, [sp, #12]
	mov	lr, ip
	strh	r3, [r4, #0]	@ movhi 
	b	.L76
.L75:
	rsb	lr, ip, lr
	sub	r3, r2, r0, lsr #21
	cmp	lr, sl
	strh	r3, [r4, #0]	@ movhi 
	rsb	r8, ip, r8
	add	r4, r6, #456
	bhi	.L77
	ldr	r2, [sp, #0]
	cmp	r7, r2
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L77:
	ldrh	r2, [r4, #0]
	mov	r0, r2, asl #16
	mov	r3, lr, lsr #11
	mov	r1, r0, lsr #16
	mul	ip, r1, r3
	cmp	r8, ip
	bcs	.L79
	rsb	r3, r1, #2048
	add	r3, r2, r3, asr #5
	ldr	r2, [sp, #8]
	mov	lr, ip
	strh	r3, [r4, #0]	@ movhi 
	b	.L80
.L79:
	sub	r3, r2, r0, lsr #21
	strh	r3, [r4, #0]	@ movhi 
	ldmib	sp, {r2, r3}	@ phole ldm
	str	r3, [sp, #4]
	rsb	r8, ip, r8
	rsb	lr, ip, lr
.L80:
	ldr	r4, [sp, #12]
	str	r4, [sp, #8]
.L76:
	ldr	ip, [sp, #16]
	str	r2, [sp, #16]
	str	ip, [sp, #12]
.L72:
	ldr	r1, [sp, #20]
	ldr	r0, [sp, #40]
	cmp	r1, #6
	movgt	r1, #11
	movle	r1, #8
	add	r4, r0, #2656
	str	r1, [sp, #20]
	add	r4, r4, #8
.L61:
	mvn	r5, #-16777216
	cmp	lr, r5
	bhi	.L83
	ldr	r2, [sp, #0]
	cmp	r7, r2
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L83:
	ldrh	r1, [r4, #0]
	mov	r2, r1, asl #16
	mov	r3, lr, lsr #11
	mov	r0, r2, lsr #16
	mul	ip, r0, r3
	cmp	r8, ip
	bcs	.L85
	add	r2, r4, r9, asl #4
	rsb	r3, r0, #2048
	add	r3, r1, r3, asr #5
	mov	lr, ip
	add	r5, r2, #4
	mov	sl, #0
	mov	fp, #3
	strh	r3, [r4, #0]	@ movhi 
	b	.L86
.L85:
	rsb	lr, ip, lr
	sub	r3, r1, r2, lsr #21
	cmp	lr, r5
	rsb	r8, ip, r8
	strh	r3, [r4, #0]	@ movhi 
	bhi	.L87
	ldr	r3, [sp, #0]
	cmp	r7, r3
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L87:
	ldrh	r1, [r4, #2]
	mov	r2, r1, asl #16
	mov	r3, lr, lsr #11
	mov	r0, r2, lsr #16
	mul	ip, r0, r3
	cmp	r8, ip
	bcs	.L89
	add	r2, r4, r9, asl #4
	rsb	r3, r0, #2048
	add	r3, r1, r3, asr #5
	mov	lr, ip
	add	r5, r2, #260
	mov	sl, #8
	mov	fp, #3
	strh	r3, [r4, #2]	@ movhi 
	b	.L86
.L89:
	sub	r3, r1, r2, lsr #21
	strh	r3, [r4, #2]	@ movhi 
	rsb	r8, ip, r8
	rsb	lr, ip, lr
	add	r5, r4, #516
	mov	sl, #16
	mov	fp, #8
.L86:
	mov	r6, fp
	mov	r9, #1
.L91:
	mov	r4, r9, asl #1
	cmp	lr, #16777216
	mov	r9, r4
	bcs	.L94
	ldr	ip, [sp, #0]
	cmp	r7, ip
	mov	lr, lr, asl #8
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	orr	r8, r3, r8, asl #8
.L94:
	ldrh	r2, [r5, r4]
	mov	r0, r2, asl #16
	mov	r1, lr, lsr #11
	mov	r3, r0, lsr #16
	mul	ip, r3, r1
	rsb	r3, r3, #2048
	cmp	r8, ip
	add	r3, r2, r3, asr #5
	sub	r2, r2, r0, lsr #21
	rsb	lr, ip, lr
	strcch	r3, [r5, r4]	@ movhi 
	movcc	lr, ip
	rsbcs	r8, ip, r8
	addcs	r9, r4, #1
	strcsh	r2, [r5, r4]	@ movhi 
	subs	r6, r6, #1
	bne	.L91
	ldr	r0, [sp, #20]
	mov	r2, #1
	sub	r9, r9, r2, asl fp
	cmp	r0, #3
	add	r9, r9, sl
	bgt	.L98
	ldr	r1, [sp, #40]
	cmp	r9, #3
	movlt	r3, r9
	movge	r3, #3
	add	r0, r0, #7
	add	r3, r1, r3, asl #7
	str	r0, [sp, #20]
	add	r4, r3, #864
	mov	r6, r2
	mov	sl, #6
.L99:
	mov	r5, r6, asl #1
	cmp	lr, #16777216
	mov	r6, r5
	bcs	.L102
	ldr	r2, [sp, #0]
	cmp	r7, r2
	mov	lr, lr, asl #8
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	orr	r8, r3, r8, asl #8
.L102:
	ldrh	r2, [r4, r5]
	mov	r0, r2, asl #16
	mov	r1, lr, lsr #11
	mov	r3, r0, lsr #16
	mul	ip, r3, r1
	rsb	r3, r3, #2048
	cmp	r8, ip
	add	r3, r2, r3, asr #5
	sub	r2, r2, r0, lsr #21
	rsb	lr, ip, lr
	strcch	r3, [r4, r5]	@ movhi 
	movcc	lr, ip
	rsbcs	r8, ip, r8
	addcs	r6, r5, #1
	strcsh	r2, [r4, r5]	@ movhi 
	subs	sl, sl, #1
	bne	.L99
	sub	r6, r6, #64
	cmp	r6, #3
	strle	r6, [sp, #16]
	ble	.L122
	and	r3, r6, #1
	mov	r2, r6, asr #1
	cmp	r6, #13
	orr	r3, r3, #2
	sub	r5, r2, #1
	str	r3, [sp, #16]
	subgt	r5, r2, #5
	bgt	.L109
	ldr	r4, [sp, #40]
	mov	r3, r3, asl r5
	str	r3, [sp, #16]
	add	r3, r4, r3, asl #1
	sub	r3, r3, r6, asl #1
	add	r4, r3, #1360
	add	r4, r4, #14
	b	.L108
.L109:
	cmp	lr, #16777216
	bcs	.L112
	ldr	ip, [sp, #0]
	cmp	r7, ip
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	mov	lr, lr, asl #8
	orr	r8, r3, r8, asl #8
.L112:
	ldr	r0, [sp, #16]
	mov	lr, lr, lsr #1
	cmp	r8, lr
	mov	r0, r0, asl #1
	str	r0, [sp, #16]
	orrcs	r0, r0, #1
	rsbcs	r8, lr, r8
	strcs	r0, [sp, #16]
	subs	r5, r5, #1
	bne	.L109
	ldr	r2, [sp, #16]
	ldr	r1, [sp, #40]
	mov	r2, r2, asl #4
	add	r4, r1, #1600
	str	r2, [sp, #16]
	add	r4, r4, #4
	mov	r5, #4
.L108:
	mov	sl, #1
	mov	fp, sl
.L115:
	mov	r6, fp, asl #1
	cmp	lr, #16777216
	mov	fp, r6
	bcs	.L118
	ldr	r3, [sp, #0]
	cmp	r7, r3
	mov	lr, lr, asl #8
	beq	.L155
	ldrb	r3, [r7], #1	@ zero_extendqisi2
	orr	r8, r3, r8, asl #8
.L118:
	ldrh	r2, [r4, r6]
	mov	r0, r2, asl #16
	mov	r1, lr, lsr #11
	mov	r3, r0, lsr #16
	mul	ip, r3, r1
	cmp	r8, ip
	rsb	lr, ip, lr
	rsbcs	r8, ip, r8
	ldrcs	ip, [sp, #16]
	rsb	r3, r3, #2048
	orrcs	ip, ip, sl
	add	r3, r2, r3, asr #5
	sub	r2, r2, r0, lsr #21
	movcc	lr, ip
	strcch	r3, [r4, r6]	@ movhi 
	addcs	fp, r6, #1
	strcsh	r2, [r4, r6]	@ movhi 
	strcs	ip, [sp, #16]
	subs	r5, r5, #1
	mov	sl, sl, asl #1
	bne	.L115
.L122:
	ldr	r0, [sp, #16]
	adds	r0, r0, #1
	str	r0, [sp, #16]
	beq	.L30
.L98:
	ldr	r1, [sp, #16]
	ldr	r2, [sp, #36]
	cmp	r1, r2
	add	r9, r9, #2
	bhi	.L155
.L125:
	ldr	r4, [sp, #36]
	ldr	ip, [sp, #16]
	ldr	r0, [sp, #88]
	rsb	r3, ip, r4
	ldrb	r4, [r0, r3]	@ zero_extendqisi2
	ldr	r1, [sp, #36]
	strb	r4, [r1, r0]
	subs	r9, r9, #1
	ldr	r2, [sp, #92]
	moveq	r3, #0
	movne	r3, #1
	add	r1, r1, #1
	cmp	r1, r2
	movcs	r3, #0
	andcc	r3, r3, #1
	cmp	r3, #0
	str	r1, [sp, #36]
	bne	.L125
.L29:
	ldr	r3, [sp, #36]
	ldr	ip, [sp, #92]
	cmp	r3, ip
	b	.L158
.L30:
	cmp	lr, #16777216
	bcs	.L129
	ldr	lr, [sp, #0]
	cmp	r7, lr
	addne	r7, r7, #1
	bne	.L129
.L155:
	mov	r0, #1
	b	.L19
.L129:
	ldr	r0, [sp, #48]
	ldr	r1, [sp, #44]
	rsb	r3, r0, r7
	str	r3, [r1, #0]
	ldr	r2, [sp, #96]
	ldr	r3, [sp, #36]
	str	r3, [r2, #0]
	mov	r0, #0
.L19:
	add	sp, sp, #52
	ldmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, pc}
	.size	LzmaDecode, .-LzmaDecode
	.ident	"GCC: (GNU) 3.4.5"

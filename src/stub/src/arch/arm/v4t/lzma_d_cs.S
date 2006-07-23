	.code	16
	.file	"lzma_d_c.c"
	.section	.text.LzmaDecodeProperties,"ax",%progbits
	.align	2
	.global	LzmaDecodeProperties
	.code 16
	.thumb_func
	.type	LzmaDecodeProperties, %function
LzmaDecodeProperties:
	push	{lr}
	cmp	r2, #4
	ble	.L18
	ldrb	r1, [r1]
	cmp	r1, #224
	bls	.L3
.L18:
	mov	r0, #1
	b	.L1
.L3:
	mov	r3, #0
	str	r3, [r0, #8]
	cmp	r1, #44
	bls	.L13
	mov	r2, #0
.L7:
	mov	r3, r1
	sub	r3, r3, #45
	lsl	r3, r3, #24
	lsr	r1, r3, #24
	add	r2, r2, #1
	cmp	r1, #44
	bhi	.L7
	str	r2, [r0, #8]
.L13:
	mov	r3, #0
	str	r3, [r0, #4]
	cmp	r1, #8
	bls	.L15
	mov	r2, #0
.L11:
	mov	r3, r1
	sub	r3, r3, #9
	lsl	r3, r3, #24
	lsr	r1, r3, #24
	add	r2, r2, #1
	cmp	r1, #8
	bhi	.L11
	str	r2, [r0, #4]
.L15:
	str	r1, [r0]
	mov	r0, #0
.L1:
	@ sp needed for prologue
	pop	{pc}
	.size	LzmaDecodeProperties, .-LzmaDecodeProperties
	.section	.text.LzmaDecode,"ax",%progbits
	.align	2
	.global	LzmaDecode
	.code 16
	.thumb_func
	.type	LzmaDecode, %function
LzmaDecode:
	push	{r4, r5, r6, r7, lr}
	sub	sp, sp, #128
	str	r1, [sp, #124]
	mov	r5, r2
	add	r1, r0, #4
	mov	r2, #0
	str	r3, [sp, #120]
	str	r1, [sp, #116]
	str	r2, [sp, #112]
	str	r2, [sp, #108]
	mov	r2, #1
	ldrb	r3, [r0, #2]
	mov	r4, r2
	lsl	r4, r4, r3
	mov	r3, r4
	sub	r3, r3, #1
	str	r3, [sp, #104]
	ldrb	r3, [r0, #1]
	lsl	r2, r2, r3
	sub	r2, r2, #1
	str	r2, [sp, #100]
	ldrb	r7, [r0]
	ldr	r4, [sp, #120]
	str	r7, [sp, #96]
	ldr	r7, [sp, #156]
	mov	r2, #1
	mov	r3, #0
	mov	r1, #0
	str	r3, [r4]
	str	r1, [sp, #92]
	str	r3, [r7]
	str	r2, [sp, #88]
	str	r2, [sp, #84]
	str	r2, [sp, #80]
	str	r2, [sp, #76]
	ldr	r1, [sp, #96]
	ldrb	r2, [r0, #1]
	mov	r3, #192
	add	r2, r1, r2
	lsl	r3, r3, #2
	lsl	r3, r3, r2
	ldr	r2, .L157
	add	r4, r3, r2
	mov	r0, #0
	cmp	r4, #0
	bls	.L133
	ldr	r2, [sp, #116]
.L23:
	mov	r3, #128
	lsl	r3, r3, #3
	add	r0, r0, #1
	strh	r3, [r2]
	add	r2, r2, #2
	cmp	r0, r4
	bcc	.L23
.L133:
	ldr	r0, [sp, #124]
	add	r5, r0, r5
	str	r5, [sp, #68]
	mov	r5, #1
	mov	r6, #0
	neg	r5, r5
	mov	r2, #0
.L28:
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LCB157
	b	.L156	@long jump
.LCB157:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	add	r2, r2, #1
	orr	r6, r6, r3
	add	r0, r0, #1
	cmp	r2, #4
	ble	.L28
	ldr	r4, [sp, #152]
	cmp	r4, #0
	bhi	.LCB172
	b	.L30	@long jump
.LCB172:
.L129:
	ldr	r1, [sp, #104]
	ldr	r7, [sp, #112]
	ldr	r2, [sp, #92]
	and	r7, r7, r1
	lsl	r3, r2, #4
	add	r3, r3, r7
	str	r7, [sp, #64]
	ldr	r1, .L157+4
	ldr	r7, [sp, #116]
	lsl	r3, r3, #1
	add	r4, r3, r7
	cmp	r5, r1
	bhi	.L31
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LCB191
	b	.L156	@long jump
.LCB191:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L31:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r7, r1, #16
	lsr	r3, r5, #11
	mov	ip, r7
	mov	r7, ip
	mul	r7, r7, r3
	mov	r3, r7
	cmp	r6, r3
	bcc	.LCB209
	b	.L33	@long jump
.LCB209:
	mov	r5, r3
	mov	r3, #128
	mov	r1, ip
	lsl	r3, r3, #4
	sub	r3, r3, r1
	asr	r3, r3, #5
	add	r3, r3, r2
	strh	r3, [r4]
	ldr	r2, [sp, #112]
	ldr	r3, [sp, #100]
	ldr	r4, [sp, #96]
	and	r2, r2, r3
	ldr	r1, [sp, #108]
	mov	r3, #8
	sub	r3, r3, r4
	asr	r1, r1, r3
	lsl	r2, r2, r4
	add	r2, r2, r1
	lsl	r3, r2, #1
	add	r3, r3, r2
	ldr	r2, [sp, #116]
	lsl	r3, r3, #9
	add	r3, r2, r3
	ldr	r1, .L157+8
	ldr	r2, [sp, #92]
	mov	r7, #1
	add	r4, r3, r1
	cmp	r2, #6
	ble	.L50
	ldr	r1, [sp, #112]
	ldr	r2, [sp, #88]
	sub	r3, r1, r2
	ldr	r1, [sp, #148]
	ldrb	r3, [r1, r3]
	str	r3, [sp, #60]
.L35:
	ldr	r2, [sp, #60]
	mov	r3, #128
	lsl	r2, r2, #1
	lsl	r1, r3, #1
	str	r2, [sp, #60]
	and	r2, r2, r1
	lsl	r3, r2, #1
	lsl	r7, r7, #1
	mov	ip, r2
	add	r3, r4, r3
	mov	r2, #128
	add	r3, r3, r7
	lsl	r2, r2, #2
	add	r2, r3, r2
	ldr	r3, .L157+4
	str	r7, [sp, #52]
	str	r2, [sp, #56]
	cmp	r5, r3
	bhi	.L38
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LCB271
	b	.L156	@long jump
.LCB271:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L38:
	ldr	r7, [sp, #56]
	ldrh	r2, [r7]
	lsl	r7, r2, #16
	lsr	r3, r5, #11
	lsr	r1, r7, #16
	mul	r3, r3, r1
	cmp	r6, r3
	bcs	.L40
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r1
	asr	r3, r3, #5
	ldr	r1, [sp, #56]
	add	r3, r3, r2
	mov	r2, ip
	strh	r3, [r1]
	ldr	r7, [sp, #52]
	cmp	r2, #0
	beq	.L37
	b	.L34
.L40:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r7, #21
	ldr	r7, [sp, #56]
	sub	r3, r2, r3
	strh	r3, [r7]
	ldr	r7, [sp, #52]
	mov	r1, ip
	add	r7, r7, #1
	cmp	r1, #0
	beq	.L34
.L37:
	cmp	r7, #255
	ble	.L35
	b	.L138
.L34:
	cmp	r7, #255
	bgt	.L138
.L50:
	lsl	r7, r7, #1
	ldr	r3, .L157+4
	str	r7, [sp, #48]
	add	r7, r7, r4
	mov	ip, r7
	cmp	r5, r3
	bhi	.L46
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LCB338
	b	.L156	@long jump
.LCB338:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L46:
	mov	r7, ip
	ldrh	r2, [r7]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L48
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	ldr	r7, [sp, #48]
	add	r3, r3, r2
	mov	r1, ip
	strh	r3, [r1]
	b	.L34
.L48:
	ldr	r7, [sp, #48]
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	add	r7, r7, #1
	mov	r2, ip
	strh	r3, [r2]
	b	.L34
.L138:
	ldr	r4, [sp, #112]
	lsl	r3, r7, #24
	ldr	r7, [sp, #148]
	lsr	r3, r3, #24
	str	r3, [sp, #108]
	strb	r3, [r4, r7]
	ldr	r7, [sp, #92]
	add	r4, r4, #1
	str	r4, [sp, #112]
	cmp	r7, #3
	bgt	.L51
	mov	r1, #0
	str	r1, [sp, #92]
	b	.L29
.L51:
	ldr	r2, [sp, #92]
	cmp	r2, #9
	bgt	.L53
	sub	r2, r2, #3
	str	r2, [sp, #92]
	b	.L29
.L53:
	ldr	r3, [sp, #92]
	sub	r3, r3, #6
	str	r3, [sp, #92]
	b	.L29
.L33:
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r4, [sp, #92]
	sub	r5, r5, r7
	sub	r6, r6, r7
	ldr	r7, [sp, #116]
	lsl	r3, r4, #1
	add	r7, r7, r3
	mov	r4, #192
	ldr	r1, .L157+4
	mov	ip, r7
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r1
	bhi	.L56
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LCB436
	b	.L156	@long jump
.LCB436:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L56:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L58
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	strh	r3, [r4]
	ldr	r7, [sp, #88]
	ldr	r3, [sp, #80]
	ldr	r4, [sp, #84]
	ldr	r1, [sp, #92]
	str	r3, [sp, #76]
	str	r4, [sp, #80]
	str	r7, [sp, #84]
	cmp	r1, #6
	bgt	.L59
	mov	r3, #0
	b	.L60
.L59:
	mov	r3, #3
.L60:
	str	r3, [sp, #92]
	ldr	r2, [sp, #116]
	ldr	r3, .L157+12
	add	r4, r2, r3
	b	.L61
.L58:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r7, .L157+4
	mov	r4, #204
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r7
	bhi	.L62
	ldr	r1, [sp, #68]
	cmp	r0, r1
	bne	.LCB498
	b	.L156	@long jump
.LCB498:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L62:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L64
	mov	r5, r3
	mov	r3, #128
	lsl	r1, r3, #4
	sub	r3, r1, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	ldr	r2, [sp, #92]
	strh	r3, [r4]
	ldr	r7, [sp, #64]
	ldr	r4, [sp, #116]
	lsl	r3, r2, #5
	add	r3, r4, r3
	lsl	r2, r7, #1
	add	r3, r3, r2
	mov	ip, r1
	ldr	r2, .L157+4
	mov	r1, #240
	lsl	r1, r1, #1
	add	r4, r3, r1
	cmp	r5, r2
	bhi	.L65
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LCB538
	b	.L156	@long jump
.LCB538:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L65:
	ldrh	r2, [r4]
	lsl	r7, r2, #16
	lsr	r3, r5, #11
	lsr	r1, r7, #16
	mul	r3, r3, r1
	cmp	r6, r3
	bcs	.L67
	mov	r7, ip
	mov	r5, r3
	sub	r3, r7, r1
	asr	r3, r3, #5
	ldr	r1, [sp, #112]
	add	r3, r3, r2
	strh	r3, [r4]
	cmp	r1, #0
	bne	.LCB563
	b	.L156	@long jump
.LCB563:
	ldr	r2, [sp, #92]
	cmp	r2, #6
	bgt	.L69
	mov	r3, #9
	b	.L70
.L69:
	mov	r3, #11
.L70:
	ldr	r4, [sp, #112]
	ldr	r7, [sp, #88]
	str	r3, [sp, #92]
	ldr	r1, [sp, #148]
	sub	r3, r4, r7
	ldrb	r3, [r1, r3]
	str	r3, [sp, #108]
	strb	r3, [r4, r1]
	add	r4, r4, #1
	str	r4, [sp, #112]
	b	.L29
.L67:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r7, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	b	.L72
.L64:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	mov	r4, #216
	ldr	r3, .L157+4
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r3
	bhi	.L73
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LCB614
	b	.L156	@long jump
.LCB614:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L73:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L75
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	ldr	r1, [sp, #84]
	asr	r3, r3, #5
	add	r3, r3, r2
	mov	ip, r1
	strh	r3, [r4]
	b	.L76
.L158:
	.align	2
.L157:
	.word	1846
	.word	16777215
	.word	3692
	.word	1636
.L75:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r2, .L159
	mov	r4, #228
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r2
	bhi	.L77
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LCB666
	b	.L156	@long jump
.LCB666:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L77:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L79
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	strh	r3, [r4]
	ldr	r4, [sp, #80]
	mov	ip, r4
	b	.L80
.L79:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	ldr	r1, [sp, #80]
	ldr	r7, [sp, #76]
	sub	r3, r2, r3
	strh	r3, [r4]
	str	r1, [sp, #76]
	mov	ip, r7
.L80:
	ldr	r2, [sp, #84]
	str	r2, [sp, #80]
.L76:
	ldr	r3, [sp, #88]
	mov	r4, ip
	str	r3, [sp, #84]
	str	r4, [sp, #88]
.L72:
	ldr	r7, [sp, #92]
	cmp	r7, #6
	bgt	.L81
	mov	r3, #8
	b	.L82
.L81:
	mov	r3, #11
.L82:
	ldr	r1, [sp, #116]
	ldr	r2, .L159+4
	str	r3, [sp, #92]
	add	r4, r1, r2
.L61:
	ldr	r3, .L159
	mov	ip, r3
	cmp	r5, ip
	bhi	.L83
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LCB740
	b	.L156	@long jump
.LCB740:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L83:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L85
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	ldr	r1, [sp, #64]
	add	r3, r3, r2
	strh	r3, [r4]
	lsl	r3, r1, #4
	add	r3, r4, r3
	add	r3, r3, #4
	str	r3, [sp, #36]
	mov	r2, #0
	mov	r3, #3
	str	r2, [sp, #40]
	str	r3, [sp, #44]
	b	.L86
.L85:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	cmp	r5, ip
	bhi	.L87
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LCB786
	b	.L156	@long jump
.LCB786:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L87:
	ldrh	r2, [r4, #2]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L89
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	ldr	r1, [sp, #64]
	add	r3, r3, r2
	strh	r3, [r4, #2]
	mov	r2, #130
	lsl	r3, r1, #4
	add	r3, r4, r3
	lsl	r2, r2, #1
	add	r2, r3, r2
	mov	r4, #3
	mov	r3, #8
	str	r2, [sp, #36]
	str	r3, [sp, #40]
	str	r4, [sp, #44]
	b	.L86
.L89:
	mov	r7, #129
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsl	r7, r7, #2
	lsr	r3, r1, #21
	sub	r3, r2, r3
	add	r7, r4, r7
	mov	r1, #16
	mov	r2, #8
	strh	r3, [r4, #2]
	str	r7, [sp, #36]
	str	r1, [sp, #40]
	str	r2, [sp, #44]
.L86:
	ldr	r3, [sp, #44]
	mov	r4, #1
	str	r3, [sp, #32]
	str	r4, [sp, #72]
.L91:
	ldr	r7, [sp, #72]
	ldr	r1, [sp, #36]
	lsl	r7, r7, #1
	ldr	r3, .L159
	mov	ip, r7
	add	r1, r1, ip
	cmp	r5, r3
	bhi	.L94
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LCB857
	b	.L156	@long jump
.LCB857:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L94:
	ldrh	r4, [r1]
	lsl	r2, r4, #16
	lsr	r3, r5, #11
	lsr	r7, r2, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L96
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r4
	strh	r3, [r1]
	mov	r3, ip
	str	r3, [sp, #72]
	b	.L93
.L96:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r2, #21
	sub	r3, r4, r3
	mov	r4, ip
	add	r4, r4, #1
	strh	r3, [r1]
	str	r4, [sp, #72]
.L93:
	ldr	r7, [sp, #32]
	sub	r7, r7, #1
	str	r7, [sp, #32]
	cmp	r7, #0
	bne	.L91
	ldr	r1, [sp, #44]
	ldr	r2, [sp, #72]
	mov	r3, #1
	lsl	r3, r3, r1
	sub	r2, r2, r3
	ldr	r3, [sp, #40]
	ldr	r4, [sp, #92]
	add	r2, r2, r3
	str	r2, [sp, #72]
	cmp	r4, #3
	ble	.LCB914
	b	.L98	@long jump
.LCB914:
	add	r4, r4, #7
	str	r4, [sp, #92]
	mov	r3, r2
	cmp	r2, #3
	ble	.L99
	mov	r3, #3
.L99:
	ldr	r7, [sp, #116]
	lsl	r3, r3, #7
	mov	r1, #216
	mov	r2, #6
	add	r3, r7, r3
	lsl	r1, r1, #2
	str	r2, [sp, #28]
	add	r4, r3, r1
	mov	r2, #1
.L100:
	lsl	r2, r2, #1
	ldr	r3, .L159
	str	r2, [sp, #24]
	add	r2, r2, r4
	mov	ip, r2
	cmp	r5, r3
	bhi	.L103
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LCB944
	b	.L156	@long jump
.LCB944:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L103:
	mov	r7, ip
	ldrh	r2, [r7]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.L105
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	ldr	r2, [sp, #24]
	mov	r1, ip
	strh	r3, [r1]
	b	.L102
.L105:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	mov	r2, ip
	strh	r3, [r2]
	ldr	r2, [sp, #24]
	add	r2, r2, #1
.L102:
	ldr	r3, [sp, #28]
	sub	r3, r3, #1
	str	r3, [sp, #28]
	cmp	r3, #0
	bne	.L100
	sub	r2, r2, #64
	cmp	r2, #3
	bgt	.LCB994
	b	.L107	@long jump
.LCB994:
	mov	r1, #1
	and	r1, r1, r2
	mov	r3, #2
	asr	r4, r2, #1
	orr	r1, r1, r3
	sub	r7, r4, #1
	str	r1, [sp, #88]
	cmp	r2, #13
	bgt	.L108
	lsl	r1, r1, r7
	ldr	r4, [sp, #116]
	lsl	r3, r1, #1
	add	r3, r4, r3
	str	r1, [sp, #88]
	lsl	r2, r2, #1
	ldr	r1, .L159+8
	sub	r3, r3, r2
	add	r4, r3, r1
	b	.L109
.L108:
	sub	r7, r4, #5
.L110:
	ldr	r3, .L159
	cmp	r5, r3
	bhi	.L113
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LCB1026
	b	.L156	@long jump
.LCB1026:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L113:
	ldr	r3, [sp, #88]
	lsr	r5, r5, #1
	lsl	r3, r3, #1
	str	r3, [sp, #88]
	cmp	r6, r5
	bcc	.L112
	ldr	r4, [sp, #88]
	mov	r3, #1
	orr	r4, r4, r3
	str	r4, [sp, #88]
	sub	r6, r6, r5
.L112:
	sub	r7, r7, #1
	cmp	r7, #0
	bne	.L110
	ldr	r2, [sp, #88]
	ldr	r7, [sp, #116]
	ldr	r1, .L159+12
	lsl	r2, r2, #4
	str	r2, [sp, #88]
	add	r4, r7, r1
	mov	r7, #4
.L109:
	mov	r3, #1
	str	r3, [sp, #20]
.L116:
	lsl	r3, r3, #1
	add	r1, r4, r3
	str	r3, [sp, #12]
	ldr	r3, .L159
	str	r1, [sp, #16]
	cmp	r5, r3
	bhi	.L119
	ldr	r2, [sp, #68]
	cmp	r0, r2
	beq	.L156
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.L119:
	ldr	r1, [sp, #16]
	ldrh	r2, [r1]
	mov	r1, sp
	strh	r2, [r1]
	lsr	r3, r5, #11
	str	r3, [sp, #8]
	ldrh	r3, [r1]
	lsl	r3, r3, #16
	mov	ip, r3
	ldr	r1, [sp, #8]
	lsr	r3, r3, #16
	mov	r2, r3
	str	r3, [sp, #4]
	mov	r3, r1
	mul	r3, r3, r2
	cmp	r6, r3
	bcs	.L121
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	mov	r1, sp
	sub	r3, r3, r2
	ldrh	r1, [r1]
	asr	r3, r3, #5
	ldr	r2, [sp, #16]
	add	r3, r3, r1
	strh	r3, [r2]
	ldr	r3, [sp, #12]
	b	.L122
.L121:
	mov	r2, sp
	mov	r1, ip
	ldrh	r2, [r2]
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	ldr	r1, [sp, #16]
	sub	r3, r2, r3
	strh	r3, [r1]
	ldr	r2, [sp, #88]
	ldr	r1, [sp, #20]
	ldr	r3, [sp, #12]
	orr	r2, r2, r1
	str	r2, [sp, #88]
	add	r3, r3, #1
.L122:
	ldr	r2, [sp, #20]
	sub	r7, r7, #1
	lsl	r2, r2, #1
	str	r2, [sp, #20]
	cmp	r7, #0
	bne	.L116
	b	.L123
.L160:
	.align	2
.L159:
	.word	16777215
	.word	2664
	.word	1374
	.word	1604
.L107:
	str	r2, [sp, #88]
.L123:
	ldr	r3, [sp, #88]
	add	r3, r3, #1
	str	r3, [sp, #88]
	cmp	r3, #0
	beq	.L30
.L98:
	ldr	r4, [sp, #72]
	ldr	r7, [sp, #88]
	ldr	r1, [sp, #112]
	add	r4, r4, #2
	str	r4, [sp, #72]
	cmp	r7, r1
	bhi	.L156
	ldr	r2, [sp, #148]
	sub	r3, r2, r7
	add	r2, r1, r3
.L126:
	ldrb	r3, [r2]
	lsl	r3, r3, #24
	lsr	r3, r3, #24
	str	r3, [sp, #108]
	add	r4, sp, #108
	ldr	r3, [sp, #72]
	ldrb	r1, [r4]
	ldr	r7, [sp, #148]
	ldr	r4, [sp, #112]
	sub	r3, r3, #1
	str	r3, [sp, #72]
	strb	r1, [r4, r7]
	add	r4, r4, #1
	add	r2, r2, #1
	str	r4, [sp, #112]
	cmp	r3, #0
	beq	.L29
	ldr	r3, [sp, #152]
	cmp	r4, r3
	bcc	.L126
	b	.L30
.L29:
	ldr	r4, [sp, #112]
	ldr	r7, [sp, #152]
	cmp	r4, r7
	bcs	.LCB1211
	b	.L129	@long jump
.LCB1211:
.L30:
	ldr	r3, .L161
	cmp	r5, r3
	bhi	.L130
	ldr	r1, [sp, #68]
	cmp	r0, r1
	bne	.L131
.L156:
	mov	r0, #1
	b	.L19
.L131:
	add	r0, r0, #1
.L130:
	ldr	r2, [sp, #124]
	ldr	r4, [sp, #120]
	ldr	r7, [sp, #112]
	ldr	r1, [sp, #156]
	sub	r3, r0, r2
	str	r3, [r4]
	str	r7, [r1]
	mov	r0, #0
.L19:
	add	sp, sp, #128
	@ sp needed for prologue
	pop	{r4, r5, r6, r7, pc}
.L162:
	.align	2
.L161:
	.word	16777215
	.size	LzmaDecode, .-LzmaDecode
	.ident	"GCC: (GNU) 3.4.5"

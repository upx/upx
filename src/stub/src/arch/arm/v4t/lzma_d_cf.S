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
	ldr	r2, .Lf157
	add	r4, r3, r2
	mov	r0, #0
	cmp	r4, #0
	bls	.Lf133
	ldr	r2, [sp, #116]
.Lf23:
	mov	r3, #128
	lsl	r3, r3, #3
	add	r0, r0, #1
	strh	r3, [r2]
	add	r2, r2, #2
	cmp	r0, r4
	bcc	.Lf23
.Lf133:
	ldr	r0, [sp, #124]
	add	r5, r0, r5
	str	r5, [sp, #68]
	mov	r5, #1
	mov	r6, #0
	neg	r5, r5
	mov	r2, #0
.Lf28:
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LfCB157
	b	.Lf156	@long jump
.LfCB157:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	add	r2, r2, #1
	orr	r6, r6, r3
	add	r0, r0, #1
	cmp	r2, #4
	ble	.Lf28
	ldr	r4, [sp, #152]
	cmp	r4, #0
	bhi	.LfCB172
	b	.Lf30	@long jump
.LfCB172:
.Lf129:
	ldr	r1, [sp, #104]
	ldr	r7, [sp, #112]
	ldr	r2, [sp, #92]
	and	r7, r7, r1
	lsl	r3, r2, #4
	add	r3, r3, r7
	str	r7, [sp, #64]
	ldr	r1, .Lf157+4
	ldr	r7, [sp, #116]
	lsl	r3, r3, #1
	add	r4, r3, r7
	cmp	r5, r1
	bhi	.Lf31
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LfCB191
	b	.Lf156	@long jump
.LfCB191:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf31:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r7, r1, #16
	lsr	r3, r5, #11
	mov	ip, r7
	mov	r7, ip
	mul	r7, r7, r3
	mov	r3, r7
	cmp	r6, r3
	bcc	.LfCB209
	b	.Lf33	@long jump
.LfCB209:
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
	ldr	r1, .Lf157+8
	ldr	r2, [sp, #92]
	mov	r7, #1
	add	r4, r3, r1
	cmp	r2, #6
	ble	.Lf50
	ldr	r1, [sp, #112]
	ldr	r2, [sp, #88]
	sub	r3, r1, r2
	ldr	r1, [sp, #148]
	ldrb	r3, [r1, r3]
	str	r3, [sp, #60]
.Lf35:
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
	ldr	r3, .Lf157+4
	str	r7, [sp, #52]
	str	r2, [sp, #56]
	cmp	r5, r3
	bhi	.Lf38
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LfCB271
	b	.Lf156	@long jump
.LfCB271:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf38:
	ldr	r7, [sp, #56]
	ldrh	r2, [r7]
	lsl	r7, r2, #16
	lsr	r3, r5, #11
	lsr	r1, r7, #16
	mul	r3, r3, r1
	cmp	r6, r3
	bcs	.Lf40
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
	beq	.Lf37
	b	.Lf34
.Lf40:
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
	beq	.Lf34
.Lf37:
	cmp	r7, #255
	ble	.Lf35
	b	.Lf138
.Lf34:
	cmp	r7, #255
	bgt	.Lf138
.Lf50:
	lsl	r7, r7, #1
	ldr	r3, .Lf157+4
	str	r7, [sp, #48]
	add	r7, r7, r4
	mov	ip, r7
	cmp	r5, r3
	bhi	.Lf46
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LfCB338
	b	.Lf156	@long jump
.LfCB338:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf46:
	mov	r7, ip
	ldrh	r2, [r7]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf48
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	ldr	r7, [sp, #48]
	add	r3, r3, r2
	mov	r1, ip
	strh	r3, [r1]
	b	.Lf34
.Lf48:
	ldr	r7, [sp, #48]
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	add	r7, r7, #1
	mov	r2, ip
	strh	r3, [r2]
	b	.Lf34
.Lf138:
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
	bgt	.Lf51
	mov	r1, #0
	str	r1, [sp, #92]
	b	.Lf29
.Lf51:
	ldr	r2, [sp, #92]
	cmp	r2, #9
	bgt	.Lf53
	sub	r2, r2, #3
	str	r2, [sp, #92]
	b	.Lf29
.Lf53:
	ldr	r3, [sp, #92]
	sub	r3, r3, #6
	str	r3, [sp, #92]
	b	.Lf29
.Lf33:
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
	ldr	r1, .Lf157+4
	mov	ip, r7
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r1
	bhi	.Lf56
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LfCB436
	b	.Lf156	@long jump
.LfCB436:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf56:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf58
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
	bgt	.Lf59
	mov	r3, #0
	b	.Lf60
.Lf59:
	mov	r3, #3
.Lf60:
	str	r3, [sp, #92]
	ldr	r2, [sp, #116]
	ldr	r3, .Lf157+12
	add	r4, r2, r3
	b	.Lf61
.Lf58:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r7, .Lf157+4
	mov	r4, #204
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r7
	bhi	.Lf62
	ldr	r1, [sp, #68]
	cmp	r0, r1
	bne	.LfCB498
	b	.Lf156	@long jump
.LfCB498:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf62:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf64
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
	ldr	r2, .Lf157+4
	mov	r1, #240
	lsl	r1, r1, #1
	add	r4, r3, r1
	cmp	r5, r2
	bhi	.Lf65
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LfCB538
	b	.Lf156	@long jump
.LfCB538:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf65:
	ldrh	r2, [r4]
	lsl	r7, r2, #16
	lsr	r3, r5, #11
	lsr	r1, r7, #16
	mul	r3, r3, r1
	cmp	r6, r3
	bcs	.Lf67
	mov	r7, ip
	mov	r5, r3
	sub	r3, r7, r1
	asr	r3, r3, #5
	ldr	r1, [sp, #112]
	add	r3, r3, r2
	strh	r3, [r4]
	cmp	r1, #0
	bne	.LfCB563
	b	.Lf156	@long jump
.LfCB563:
	ldr	r2, [sp, #92]
	cmp	r2, #6
	bgt	.Lf69
	mov	r3, #9
	b	.Lf70
.Lf69:
	mov	r3, #11
.Lf70:
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
	b	.Lf29
.Lf67:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r7, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	b	.Lf72
.Lf64:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	mov	r4, #216
	ldr	r3, .Lf157+4
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r3
	bhi	.Lf73
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LfCB614
	b	.Lf156	@long jump
.LfCB614:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf73:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf75
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	ldr	r1, [sp, #84]
	asr	r3, r3, #5
	add	r3, r3, r2
	mov	ip, r1
	strh	r3, [r4]
	b	.Lf76
.Lf158:
	.align	2
.Lf157:
	.word	1846
	.word	16777215
	.word	3692
	.word	1636
.Lf75:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r2, .Lf159
	mov	r4, #228
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r2
	bhi	.Lf77
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LfCB666
	b	.Lf156	@long jump
.LfCB666:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf77:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf79
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	strh	r3, [r4]
	ldr	r4, [sp, #80]
	mov	ip, r4
	b	.Lf80
.Lf79:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	ldr	r1, [sp, #80]
	ldr	r7, [sp, #76]
	sub	r3, r2, r3
	strh	r3, [r4]
	str	r1, [sp, #76]
	mov	ip, r7
.Lf80:
	ldr	r2, [sp, #84]
	str	r2, [sp, #80]
.Lf76:
	ldr	r3, [sp, #88]
	mov	r4, ip
	str	r3, [sp, #84]
	str	r4, [sp, #88]
.Lf72:
	ldr	r7, [sp, #92]
	cmp	r7, #6
	bgt	.Lf81
	mov	r3, #8
	b	.Lf82
.Lf81:
	mov	r3, #11
.Lf82:
	ldr	r1, [sp, #116]
	ldr	r2, .Lf159+4
	str	r3, [sp, #92]
	add	r4, r1, r2
.Lf61:
	ldr	r3, .Lf159
	mov	ip, r3
	cmp	r5, ip
	bhi	.Lf83
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LfCB740
	b	.Lf156	@long jump
.LfCB740:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf83:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf85
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
	b	.Lf86
.Lf85:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	cmp	r5, ip
	bhi	.Lf87
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LfCB786
	b	.Lf156	@long jump
.LfCB786:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf87:
	ldrh	r2, [r4, #2]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf89
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
	b	.Lf86
.Lf89:
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
.Lf86:
	ldr	r3, [sp, #44]
	mov	r4, #1
	str	r3, [sp, #32]
	str	r4, [sp, #72]
.Lf91:
	ldr	r7, [sp, #72]
	ldr	r1, [sp, #36]
	lsl	r7, r7, #1
	ldr	r3, .Lf159
	mov	ip, r7
	add	r1, r1, ip
	cmp	r5, r3
	bhi	.Lf94
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LfCB857
	b	.Lf156	@long jump
.LfCB857:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf94:
	ldrh	r4, [r1]
	lsl	r2, r4, #16
	lsr	r3, r5, #11
	lsr	r7, r2, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf96
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r4
	strh	r3, [r1]
	mov	r3, ip
	str	r3, [sp, #72]
	b	.Lf93
.Lf96:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r2, #21
	sub	r3, r4, r3
	mov	r4, ip
	add	r4, r4, #1
	strh	r3, [r1]
	str	r4, [sp, #72]
.Lf93:
	ldr	r7, [sp, #32]
	sub	r7, r7, #1
	str	r7, [sp, #32]
	cmp	r7, #0
	bne	.Lf91
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
	ble	.LfCB914
	b	.Lf98	@long jump
.LfCB914:
	add	r4, r4, #7
	str	r4, [sp, #92]
	mov	r3, r2
	cmp	r2, #3
	ble	.Lf99
	mov	r3, #3
.Lf99:
	ldr	r7, [sp, #116]
	lsl	r3, r3, #7
	mov	r1, #216
	mov	r2, #6
	add	r3, r7, r3
	lsl	r1, r1, #2
	str	r2, [sp, #28]
	add	r4, r3, r1
	mov	r2, #1
.Lf100:
	lsl	r2, r2, #1
	ldr	r3, .Lf159
	str	r2, [sp, #24]
	add	r2, r2, r4
	mov	ip, r2
	cmp	r5, r3
	bhi	.Lf103
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LfCB944
	b	.Lf156	@long jump
.LfCB944:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf103:
	mov	r7, ip
	ldrh	r2, [r7]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Lf105
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	ldr	r2, [sp, #24]
	mov	r1, ip
	strh	r3, [r1]
	b	.Lf102
.Lf105:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	mov	r2, ip
	strh	r3, [r2]
	ldr	r2, [sp, #24]
	add	r2, r2, #1
.Lf102:
	ldr	r3, [sp, #28]
	sub	r3, r3, #1
	str	r3, [sp, #28]
	cmp	r3, #0
	bne	.Lf100
	sub	r2, r2, #64
	cmp	r2, #3
	bgt	.LfCB994
	b	.Lf107	@long jump
.LfCB994:
	mov	r1, #1
	and	r1, r1, r2
	mov	r3, #2
	asr	r4, r2, #1
	orr	r1, r1, r3
	sub	r7, r4, #1
	str	r1, [sp, #88]
	cmp	r2, #13
	bgt	.Lf108
	lsl	r1, r1, r7
	ldr	r4, [sp, #116]
	lsl	r3, r1, #1
	add	r3, r4, r3
	str	r1, [sp, #88]
	lsl	r2, r2, #1
	ldr	r1, .Lf159+8
	sub	r3, r3, r2
	add	r4, r3, r1
	b	.Lf109
.Lf108:
	sub	r7, r4, #5
.Lf110:
	ldr	r3, .Lf159
	cmp	r5, r3
	bhi	.Lf113
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LfCB1026
	b	.Lf156	@long jump
.LfCB1026:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf113:
	ldr	r3, [sp, #88]
	lsr	r5, r5, #1
	lsl	r3, r3, #1
	str	r3, [sp, #88]
	cmp	r6, r5
	bcc	.Lf112
	ldr	r4, [sp, #88]
	mov	r3, #1
	orr	r4, r4, r3
	str	r4, [sp, #88]
	sub	r6, r6, r5
.Lf112:
	sub	r7, r7, #1
	cmp	r7, #0
	bne	.Lf110
	ldr	r2, [sp, #88]
	ldr	r7, [sp, #116]
	ldr	r1, .Lf159+12
	lsl	r2, r2, #4
	str	r2, [sp, #88]
	add	r4, r7, r1
	mov	r7, #4
.Lf109:
	mov	r3, #1
	str	r3, [sp, #20]
.Lf116:
	lsl	r3, r3, #1
	add	r1, r4, r3
	str	r3, [sp, #12]
	ldr	r3, .Lf159
	str	r1, [sp, #16]
	cmp	r5, r3
	bhi	.Lf119
	ldr	r2, [sp, #68]
	cmp	r0, r2
	beq	.Lf156
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Lf119:
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
	bcs	.Lf121
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
	b	.Lf122
.Lf121:
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
.Lf122:
	ldr	r2, [sp, #20]
	sub	r7, r7, #1
	lsl	r2, r2, #1
	str	r2, [sp, #20]
	cmp	r7, #0
	bne	.Lf116
	b	.Lf123
.Lf160:
	.align	2
.Lf159:
	.word	16777215
	.word	2664
	.word	1374
	.word	1604
.Lf107:
	str	r2, [sp, #88]
.Lf123:
	ldr	r3, [sp, #88]
	add	r3, r3, #1
	str	r3, [sp, #88]
	cmp	r3, #0
	beq	.Lf30
.Lf98:
	ldr	r4, [sp, #72]
	ldr	r7, [sp, #88]
	ldr	r1, [sp, #112]
	add	r4, r4, #2
	str	r4, [sp, #72]
	cmp	r7, r1
	bhi	.Lf156
	ldr	r2, [sp, #148]
	sub	r3, r2, r7
	add	r2, r1, r3
.Lf126:
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
	beq	.Lf29
	ldr	r3, [sp, #152]
	cmp	r4, r3
	bcc	.Lf126
	b	.Lf30
.Lf29:
	ldr	r4, [sp, #112]
	ldr	r7, [sp, #152]
	cmp	r4, r7
	bcs	.LfCB1211
	b	.Lf129	@long jump
.LfCB1211:
.Lf30:
	ldr	r3, .Lf161
	cmp	r5, r3
	bhi	.Lf130
	ldr	r1, [sp, #68]
	cmp	r0, r1
	bne	.Lf131
.Lf156:
	mov	r0, #1
	b	.Lf19
.Lf131:
	add	r0, r0, #1
.Lf130:
	ldr	r2, [sp, #124]
	ldr	r4, [sp, #120]
	ldr	r7, [sp, #112]
	ldr	r1, [sp, #156]
	sub	r3, r0, r2
	str	r3, [r4]
	str	r7, [r1]
	mov	r0, #0
.Lf19:
	add	sp, sp, #128
	@ sp needed for prologue
	pop	{r4, r5, r6, r7, pc}
.Lf162:
	.align	2
.Lf161:
	.word	16777215

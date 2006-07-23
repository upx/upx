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
	ldr	r2, .Ls157
	add	r4, r3, r2
	mov	r0, #0
	cmp	r4, #0
	bls	.Ls133
	ldr	r2, [sp, #116]
.Ls23:
	mov	r3, #128
	lsl	r3, r3, #3
	add	r0, r0, #1
	strh	r3, [r2]
	add	r2, r2, #2
	cmp	r0, r4
	bcc	.Ls23
.Ls133:
	ldr	r0, [sp, #124]
	add	r5, r0, r5
	str	r5, [sp, #68]
	mov	r5, #1
	mov	r6, #0
	neg	r5, r5
	mov	r2, #0
.Ls28:
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LsCB157
	b	.Ls156	@long jump
.LsCB157:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	add	r2, r2, #1
	orr	r6, r6, r3
	add	r0, r0, #1
	cmp	r2, #4
	ble	.Ls28
	ldr	r4, [sp, #152]
	cmp	r4, #0
	bhi	.LsCB172
	b	.Ls30	@long jump
.LsCB172:
.Ls129:
	ldr	r1, [sp, #104]
	ldr	r7, [sp, #112]
	ldr	r2, [sp, #92]
	and	r7, r7, r1
	lsl	r3, r2, #4
	add	r3, r3, r7
	str	r7, [sp, #64]
	ldr	r1, .Ls157+4
	ldr	r7, [sp, #116]
	lsl	r3, r3, #1
	add	r4, r3, r7
	cmp	r5, r1
	bhi	.Ls31
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LsCB191
	b	.Ls156	@long jump
.LsCB191:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls31:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r7, r1, #16
	lsr	r3, r5, #11
	mov	ip, r7
	mov	r7, ip
	mul	r7, r7, r3
	mov	r3, r7
	cmp	r6, r3
	bcc	.LsCB209
	b	.Ls33	@long jump
.LsCB209:
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
	ldr	r1, .Ls157+8
	ldr	r2, [sp, #92]
	mov	r7, #1
	add	r4, r3, r1
	cmp	r2, #6
	ble	.Ls50
	ldr	r1, [sp, #112]
	ldr	r2, [sp, #88]
	sub	r3, r1, r2
	ldr	r1, [sp, #148]
	ldrb	r3, [r1, r3]
	str	r3, [sp, #60]
.Ls35:
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
	ldr	r3, .Ls157+4
	str	r7, [sp, #52]
	str	r2, [sp, #56]
	cmp	r5, r3
	bhi	.Ls38
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LsCB271
	b	.Ls156	@long jump
.LsCB271:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls38:
	ldr	r7, [sp, #56]
	ldrh	r2, [r7]
	lsl	r7, r2, #16
	lsr	r3, r5, #11
	lsr	r1, r7, #16
	mul	r3, r3, r1
	cmp	r6, r3
	bcs	.Ls40
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
	beq	.Ls37
	b	.Ls34
.Ls40:
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
	beq	.Ls34
.Ls37:
	cmp	r7, #255
	ble	.Ls35
	b	.Ls138
.Ls34:
	cmp	r7, #255
	bgt	.Ls138
.Ls50:
	lsl	r7, r7, #1
	ldr	r3, .Ls157+4
	str	r7, [sp, #48]
	add	r7, r7, r4
	mov	ip, r7
	cmp	r5, r3
	bhi	.Ls46
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LsCB338
	b	.Ls156	@long jump
.LsCB338:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls46:
	mov	r7, ip
	ldrh	r2, [r7]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls48
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	ldr	r7, [sp, #48]
	add	r3, r3, r2
	mov	r1, ip
	strh	r3, [r1]
	b	.Ls34
.Ls48:
	ldr	r7, [sp, #48]
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	add	r7, r7, #1
	mov	r2, ip
	strh	r3, [r2]
	b	.Ls34
.Ls138:
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
	bgt	.Ls51
	mov	r1, #0
	str	r1, [sp, #92]
	b	.Ls29
.Ls51:
	ldr	r2, [sp, #92]
	cmp	r2, #9
	bgt	.Ls53
	sub	r2, r2, #3
	str	r2, [sp, #92]
	b	.Ls29
.Ls53:
	ldr	r3, [sp, #92]
	sub	r3, r3, #6
	str	r3, [sp, #92]
	b	.Ls29
.Ls33:
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
	ldr	r1, .Ls157+4
	mov	ip, r7
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r1
	bhi	.Ls56
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LsCB436
	b	.Ls156	@long jump
.LsCB436:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls56:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls58
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
	bgt	.Ls59
	mov	r3, #0
	b	.Ls60
.Ls59:
	mov	r3, #3
.Ls60:
	str	r3, [sp, #92]
	ldr	r2, [sp, #116]
	ldr	r3, .Ls157+12
	add	r4, r2, r3
	b	.Ls61
.Ls58:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r7, .Ls157+4
	mov	r4, #204
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r7
	bhi	.Ls62
	ldr	r1, [sp, #68]
	cmp	r0, r1
	bne	.LsCB498
	b	.Ls156	@long jump
.LsCB498:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls62:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls64
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
	ldr	r2, .Ls157+4
	mov	r1, #240
	lsl	r1, r1, #1
	add	r4, r3, r1
	cmp	r5, r2
	bhi	.Ls65
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LsCB538
	b	.Ls156	@long jump
.LsCB538:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls65:
	ldrh	r2, [r4]
	lsl	r7, r2, #16
	lsr	r3, r5, #11
	lsr	r1, r7, #16
	mul	r3, r3, r1
	cmp	r6, r3
	bcs	.Ls67
	mov	r7, ip
	mov	r5, r3
	sub	r3, r7, r1
	asr	r3, r3, #5
	ldr	r1, [sp, #112]
	add	r3, r3, r2
	strh	r3, [r4]
	cmp	r1, #0
	bne	.LsCB563
	b	.Ls156	@long jump
.LsCB563:
	ldr	r2, [sp, #92]
	cmp	r2, #6
	bgt	.Ls69
	mov	r3, #9
	b	.Ls70
.Ls69:
	mov	r3, #11
.Ls70:
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
	b	.Ls29
.Ls67:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r7, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	b	.Ls72
.Ls64:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	mov	r4, #216
	ldr	r3, .Ls157+4
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r3
	bhi	.Ls73
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LsCB614
	b	.Ls156	@long jump
.LsCB614:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls73:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls75
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	ldr	r1, [sp, #84]
	asr	r3, r3, #5
	add	r3, r3, r2
	mov	ip, r1
	strh	r3, [r4]
	b	.Ls76
.Ls158:
	.align	2
.Ls157:
	.word	1846
	.word	16777215
	.word	3692
	.word	1636
.Ls75:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r2, .Ls159
	mov	r4, #228
	lsl	r4, r4, #1
	add	r4, r4, ip
	cmp	r5, r2
	bhi	.Ls77
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LsCB666
	b	.Ls156	@long jump
.LsCB666:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls77:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls79
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	strh	r3, [r4]
	ldr	r4, [sp, #80]
	mov	ip, r4
	b	.Ls80
.Ls79:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	ldr	r1, [sp, #80]
	ldr	r7, [sp, #76]
	sub	r3, r2, r3
	strh	r3, [r4]
	str	r1, [sp, #76]
	mov	ip, r7
.Ls80:
	ldr	r2, [sp, #84]
	str	r2, [sp, #80]
.Ls76:
	ldr	r3, [sp, #88]
	mov	r4, ip
	str	r3, [sp, #84]
	str	r4, [sp, #88]
.Ls72:
	ldr	r7, [sp, #92]
	cmp	r7, #6
	bgt	.Ls81
	mov	r3, #8
	b	.Ls82
.Ls81:
	mov	r3, #11
.Ls82:
	ldr	r1, [sp, #116]
	ldr	r2, .Ls159+4
	str	r3, [sp, #92]
	add	r4, r1, r2
.Ls61:
	ldr	r3, .Ls159
	mov	ip, r3
	cmp	r5, ip
	bhi	.Ls83
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LsCB740
	b	.Ls156	@long jump
.LsCB740:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls83:
	ldrh	r2, [r4]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls85
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
	b	.Ls86
.Ls85:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	strh	r3, [r4]
	cmp	r5, ip
	bhi	.Ls87
	ldr	r7, [sp, #68]
	cmp	r0, r7
	bne	.LsCB786
	b	.Ls156	@long jump
.LsCB786:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls87:
	ldrh	r2, [r4, #2]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls89
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
	b	.Ls86
.Ls89:
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
.Ls86:
	ldr	r3, [sp, #44]
	mov	r4, #1
	str	r3, [sp, #32]
	str	r4, [sp, #72]
.Ls91:
	ldr	r7, [sp, #72]
	ldr	r1, [sp, #36]
	lsl	r7, r7, #1
	ldr	r3, .Ls159
	mov	ip, r7
	add	r1, r1, ip
	cmp	r5, r3
	bhi	.Ls94
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LsCB857
	b	.Ls156	@long jump
.LsCB857:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls94:
	ldrh	r4, [r1]
	lsl	r2, r4, #16
	lsr	r3, r5, #11
	lsr	r7, r2, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls96
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r4
	strh	r3, [r1]
	mov	r3, ip
	str	r3, [sp, #72]
	b	.Ls93
.Ls96:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r2, #21
	sub	r3, r4, r3
	mov	r4, ip
	add	r4, r4, #1
	strh	r3, [r1]
	str	r4, [sp, #72]
.Ls93:
	ldr	r7, [sp, #32]
	sub	r7, r7, #1
	str	r7, [sp, #32]
	cmp	r7, #0
	bne	.Ls91
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
	ble	.LsCB914
	b	.Ls98	@long jump
.LsCB914:
	add	r4, r4, #7
	str	r4, [sp, #92]
	mov	r3, r2
	cmp	r2, #3
	ble	.Ls99
	mov	r3, #3
.Ls99:
	ldr	r7, [sp, #116]
	lsl	r3, r3, #7
	mov	r1, #216
	mov	r2, #6
	add	r3, r7, r3
	lsl	r1, r1, #2
	str	r2, [sp, #28]
	add	r4, r3, r1
	mov	r2, #1
.Ls100:
	lsl	r2, r2, #1
	ldr	r3, .Ls159
	str	r2, [sp, #24]
	add	r2, r2, r4
	mov	ip, r2
	cmp	r5, r3
	bhi	.Ls103
	ldr	r3, [sp, #68]
	cmp	r0, r3
	bne	.LsCB944
	b	.Ls156	@long jump
.LsCB944:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls103:
	mov	r7, ip
	ldrh	r2, [r7]
	lsl	r1, r2, #16
	lsr	r3, r5, #11
	lsr	r7, r1, #16
	mul	r3, r3, r7
	cmp	r6, r3
	bcs	.Ls105
	mov	r5, r3
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r7
	asr	r3, r3, #5
	add	r3, r3, r2
	ldr	r2, [sp, #24]
	mov	r1, ip
	strh	r3, [r1]
	b	.Ls102
.Ls105:
	sub	r5, r5, r3
	sub	r6, r6, r3
	lsr	r3, r1, #21
	sub	r3, r2, r3
	mov	r2, ip
	strh	r3, [r2]
	ldr	r2, [sp, #24]
	add	r2, r2, #1
.Ls102:
	ldr	r3, [sp, #28]
	sub	r3, r3, #1
	str	r3, [sp, #28]
	cmp	r3, #0
	bne	.Ls100
	sub	r2, r2, #64
	cmp	r2, #3
	bgt	.LsCB994
	b	.Ls107	@long jump
.LsCB994:
	mov	r1, #1
	and	r1, r1, r2
	mov	r3, #2
	asr	r4, r2, #1
	orr	r1, r1, r3
	sub	r7, r4, #1
	str	r1, [sp, #88]
	cmp	r2, #13
	bgt	.Ls108
	lsl	r1, r1, r7
	ldr	r4, [sp, #116]
	lsl	r3, r1, #1
	add	r3, r4, r3
	str	r1, [sp, #88]
	lsl	r2, r2, #1
	ldr	r1, .Ls159+8
	sub	r3, r3, r2
	add	r4, r3, r1
	b	.Ls109
.Ls108:
	sub	r7, r4, #5
.Ls110:
	ldr	r3, .Ls159
	cmp	r5, r3
	bhi	.Ls113
	ldr	r2, [sp, #68]
	cmp	r0, r2
	bne	.LsCB1026
	b	.Ls156	@long jump
.LsCB1026:
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls113:
	ldr	r3, [sp, #88]
	lsr	r5, r5, #1
	lsl	r3, r3, #1
	str	r3, [sp, #88]
	cmp	r6, r5
	bcc	.Ls112
	ldr	r4, [sp, #88]
	mov	r3, #1
	orr	r4, r4, r3
	str	r4, [sp, #88]
	sub	r6, r6, r5
.Ls112:
	sub	r7, r7, #1
	cmp	r7, #0
	bne	.Ls110
	ldr	r2, [sp, #88]
	ldr	r7, [sp, #116]
	ldr	r1, .Ls159+12
	lsl	r2, r2, #4
	str	r2, [sp, #88]
	add	r4, r7, r1
	mov	r7, #4
.Ls109:
	mov	r3, #1
	str	r3, [sp, #20]
.Ls116:
	lsl	r3, r3, #1
	add	r1, r4, r3
	str	r3, [sp, #12]
	ldr	r3, .Ls159
	str	r1, [sp, #16]
	cmp	r5, r3
	bhi	.Ls119
	ldr	r2, [sp, #68]
	cmp	r0, r2
	beq	.Ls156
	ldrb	r3, [r0]
	lsl	r6, r6, #8
	lsl	r5, r5, #8
	orr	r6, r6, r3
	add	r0, r0, #1
.Ls119:
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
	bcs	.Ls121
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
	b	.Ls122
.Ls121:
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
.Ls122:
	ldr	r2, [sp, #20]
	sub	r7, r7, #1
	lsl	r2, r2, #1
	str	r2, [sp, #20]
	cmp	r7, #0
	bne	.Ls116
	b	.Ls123
.Ls160:
	.align	2
.Ls159:
	.word	16777215
	.word	2664
	.word	1374
	.word	1604
.Ls107:
	str	r2, [sp, #88]
.Ls123:
	ldr	r3, [sp, #88]
	add	r3, r3, #1
	str	r3, [sp, #88]
	cmp	r3, #0
	beq	.Ls30
.Ls98:
	ldr	r4, [sp, #72]
	ldr	r7, [sp, #88]
	ldr	r1, [sp, #112]
	add	r4, r4, #2
	str	r4, [sp, #72]
	cmp	r7, r1
	bhi	.Ls156
	ldr	r2, [sp, #148]
	sub	r3, r2, r7
	add	r2, r1, r3
.Ls126:
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
	beq	.Ls29
	ldr	r3, [sp, #152]
	cmp	r4, r3
	bcc	.Ls126
	b	.Ls30
.Ls29:
	ldr	r4, [sp, #112]
	ldr	r7, [sp, #152]
	cmp	r4, r7
	bcs	.LsCB1211
	b	.Ls129	@long jump
.LsCB1211:
.Ls30:
	ldr	r3, .Ls161
	cmp	r5, r3
	bhi	.Ls130
	ldr	r1, [sp, #68]
	cmp	r0, r1
	bne	.Ls131
.Ls156:
	mov	r0, #1
	b	.Ls19
.Ls131:
	add	r0, r0, #1
.Ls130:
	ldr	r2, [sp, #124]
	ldr	r4, [sp, #120]
	ldr	r7, [sp, #112]
	ldr	r1, [sp, #156]
	sub	r3, r0, r2
	str	r3, [r4]
	str	r7, [r1]
	mov	r0, #0
.Ls19:
	add	sp, sp, #128
	@ sp needed for prologue
	pop	{r4, r5, r6, r7, pc}
.Ls162:
	.align	2
.Ls161:
	.word	16777215

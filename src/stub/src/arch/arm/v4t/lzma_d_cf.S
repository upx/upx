.thumb
	push	{r4, r5, r6, r7, lr}
	sub	sp, sp, #124
	str	r1, [sp, #16]
	add	r1, r0, #4
	str	r1, [sp, #24]
	str	r3, [sp, #12]
	mov	r5, r2
	ldrb	r3, [r0, #2]
	mov	r2, #1
	mov	r1, r2
	lsl	r1, r1, r3
	mov	r3, r1
	sub	r3, r3, #1
	str	r3, [sp, #32]
	ldrb	r3, [r0, #1]
	lsl	r2, r2, r3
	sub	r2, r2, #1
	str	r2, [sp, #36]
	ldrb	r2, [r0]
	ldr	r1, [sp, #12]
	str	r2, [sp, #40]
	ldr	r2, [sp, #152]
	mov	r3, #0
	str	r3, [r1]
	str	r3, [r2]
	ldr	r3, [sp, #40]
	ldrb	r2, [r0, #1]
	add	r2, r3, r2
	mov	r3, #192
	ldr	r0, .Lf171
	lsl	r3, r3, #2
	lsl	r3, r3, r2
	ldr	r2, [sp, #24]
	add	r4, r3, r0
	mov	r0, #0
	b	.Lf14
.Lf15:
	mov	r3, #128
	lsl	r3, r3, #3
	strh	r3, [r2]
	add	r0, r0, #1
	add	r2, r2, #2
.Lf14:
	cmp	r0, r4
	bne	.Lf15
	ldr	r1, [sp, #16]
	add	r5, r1, r5
	str	r5, [sp, #64]
	mov	r7, #0
	mov	r5, r1
	mov	r0, #0
.Lf17:
	ldr	r2, [sp, #64]
	ldr	r1, [sp, #16]
	sub	r3, r2, r1
	cmp	r0, r3
	bne	.LfCB158
	b	.Lf18	@long jump
.LfCB158:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	add	r0, r0, #1
	orr	r7, r7, r3
	add	r5, r5, #1
	cmp	r0, #5
	bne	.Lf17
	mov	r2, #0
	mov	r1, #1
	mov	r3, #1
	mov	ip, r2
	neg	r1, r1
	str	r2, [sp, #28]
	str	r2, [sp, #44]
	str	r3, [sp, #120]
	str	r3, [sp, #48]
	str	r3, [sp, #52]
	str	r3, [sp, #56]
	b	.Lf21
.Lf22:
	ldr	r0, .Lf171+4
	cmp	r1, r0
	bhi	.Lf23
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LfCB190
	b	.Lf18	@long jump
.LfCB190:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf23:
	ldr	r0, [sp, #32]
	ldr	r3, [sp, #28]
	ldr	r2, [sp, #44]
	and	r3, r3, r0
	lsl	r2, r2, #4
	str	r3, [sp, #68]
	ldr	r0, [sp, #24]
	add	r3, r3, r2
	lsl	r3, r3, #1
	add	r4, r0, r3
	str	r2, [sp, #20]
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r6, r2
	mul	r6, r3
	cmp	r7, r6
	bcc	.LfCB214
	b	.Lf26	@long jump
.LfCB214:
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	ldr	r1, [sp, #36]
	ldr	r2, [sp, #28]
	strh	r3, [r4]
	ldr	r3, [sp, #40]
	and	r2, r2, r1
	ldr	r0, [sp, #40]
	lsl	r2, r2, r3
	mov	r3, #8
	sub	r3, r3, r0
	mov	r1, ip
	asr	r1, r1, r3
	add	r2, r2, r1
	lsl	r3, r2, #1
	add	r3, r3, r2
	ldr	r2, [sp, #24]
	lsl	r3, r3, #9
	ldr	r0, .Lf171+8
	add	r3, r2, r3
	ldr	r1, [sp, #44]
	add	r0, r3, r0
	str	r0, [sp, #112]
	cmp	r1, #6
	bgt	.Lf28
	mov	r2, #1
	mov	r1, r6
	str	r2, [sp, #72]
	b	.Lf166
.Lf28:
	ldr	r1, [sp, #120]
	ldr	r0, [sp, #28]
	ldr	r2, [sp, #144]
	sub	r3, r0, r1
	ldrb	r3, [r2, r3]
	str	r3, [sp, #76]
	mov	r3, #1
	str	r3, [sp, #72]
	mov	r1, r6
.Lf31:
	ldr	r3, .Lf171+4
	cmp	r1, r3
	bhi	.Lf32
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LfCB267
	b	.Lf18	@long jump
.LfCB267:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf32:
	ldr	r2, [sp, #76]
	mov	r3, #128
	lsl	r2, r2, #1
	lsl	r3, r3, #1
	mov	r6, r2
	ldr	r0, [sp, #112]
	str	r2, [sp, #76]
	and	r6, r6, r3
	ldr	r2, [sp, #72]
	lsl	r3, r6, #1
	lsl	r2, r2, #1
	add	r3, r0, r3
	mov	r0, #128
	add	r3, r3, r2
	lsl	r0, r0, #2
	add	r4, r3, r0
	str	r2, [sp, #4]
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf35
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	ldr	r1, [sp, #4]
	add	r3, r2, r3
	strh	r3, [r4]
	str	r1, [sp, #72]
	cmp	r6, #0
	bne	.Lf167
	mov	r1, r0
	b	.Lf39
.Lf35:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r3, [sp, #72]
	ldr	r2, [sp, #72]
	add	r3, r3, #1
	add	r2, r2, r3
	sub	r1, r1, r0
	sub	r7, r7, r0
	str	r2, [sp, #72]
	cmp	r6, #0
	beq	.Lf166
.Lf39:
	ldr	r3, [sp, #72]
	cmp	r3, #255
	bgt	.Lf46
	b	.Lf31
.Lf40:
	ldr	r3, .Lf171+4
	cmp	r1, r3
	bhi	.Lf41
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LfCB340
	b	.Lf18	@long jump
.LfCB340:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf41:
	ldr	r2, [sp, #72]
	ldr	r3, [sp, #112]
	lsl	r6, r2, #1
	add	r4, r3, r6
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf44
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r4]
	str	r6, [sp, #72]
.Lf167:
	mov	r1, r0
	b	.Lf166
.Lf44:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r3, [sp, #72]
	sub	r1, r1, r0
	sub	r7, r7, r0
	ldr	r0, [sp, #72]
	add	r3, r3, #1
	add	r0, r0, r3
	str	r0, [sp, #72]
.Lf166:
	ldr	r2, [sp, #72]
	cmp	r2, #255
	ble	.Lf40
.Lf46:
	ldr	r0, [sp, #72]
	lsl	r3, r0, #24
	lsr	r3, r3, #24
	add	r0, r3, #0
	ldr	r2, [sp, #144]
	mov	ip, r3
	ldr	r3, [sp, #28]
	strb	r0, [r2, r3]
	mov	r2, r3
	ldr	r3, [sp, #44]
	add	r2, r2, #1
	cmp	r3, #3
	bgt	.Lf47
	mov	r0, #0
	b	.Lf170
.Lf47:
	ldr	r3, [sp, #44]
	cmp	r3, #9
	bgt	.Lf50
	sub	r3, r3, #3
	str	r3, [sp, #44]
	b	.Lf49
.Lf50:
	ldr	r0, [sp, #44]
	sub	r0, r0, #6
.Lf170:
	str	r0, [sp, #44]
	b	.Lf49
.Lf26:
	sub	r0, r1, r6
	lsr	r3, r2, #5
	ldr	r1, .Lf171+4
	sub	r3, r2, r3
	sub	r7, r7, r6
	strh	r3, [r4]
	cmp	r0, r1
	bhi	.Lf52
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LfCB435
	b	.Lf18	@long jump
.LfCB435:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r0, r0, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf52:
	ldr	r1, [sp, #44]
	ldr	r2, [sp, #24]
	lsl	r3, r1, #1
	add	r2, r2, r3
	mov	r6, #192
	mov	ip, r2
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r2, [r6]
	lsr	r3, r0, #11
	mov	r1, r2
	mul	r1, r3
	cmp	r7, r1
	bcs	.Lf55
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r6]
	ldr	r3, [sp, #44]
	cmp	r3, #6
	bgt	.Lf57
	mov	r0, #0
	str	r0, [sp, #44]
	b	.Lf59
.Lf57:
	mov	r2, #3
	str	r2, [sp, #44]
.Lf59:
	ldr	r3, [sp, #24]
	ldr	r0, .Lf171+12
	add	r6, r3, r0
	ldr	r0, [sp, #120]
	ldr	r2, [sp, #52]
	ldr	r3, [sp, #48]
	str	r0, [sp, #48]
	mov	r0, r1
	str	r2, [sp, #56]
	str	r3, [sp, #52]
	b	.Lf60
.Lf55:
	sub	r4, r0, r1
	sub	r7, r7, r1
	lsr	r3, r2, #5
	ldr	r1, .Lf171+4
	sub	r3, r2, r3
	strh	r3, [r6]
	cmp	r4, r1
	bhi	.Lf61
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LfCB500
	b	.Lf18	@long jump
.LfCB500:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r4, r4, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf61:
	mov	r6, #204
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r0, [r6]
	lsr	r3, r4, #11
	mov	r2, r0
	mul	r2, r3
	cmp	r7, r2
	bcs	.Lf64
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r0
	asr	r3, r3, #5
	add	r3, r0, r3
	strh	r3, [r6]
	ldr	r3, .Lf171+4
	cmp	r2, r3
	bls	.Lf66
	mov	r0, r2
	b	.Lf68
.Lf66:
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LfCB533
	b	.Lf18	@long jump
.LfCB533:
	ldrb	r3, [r5]
	lsl	r0, r2, #8
	lsl	r2, r7, #8
	mov	r7, r2
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf68:
	ldr	r1, [sp, #20]
	ldr	r2, [sp, #24]
	lsl	r3, r1, #1
	ldr	r1, [sp, #68]
	add	r3, r2, r3
	lsl	r2, r1, #1
	add	r3, r3, r2
	mov	r2, #240
	lsl	r2, r2, #1
	add	r4, r3, r2
	ldrh	r2, [r4]
	lsr	r3, r0, #11
	mov	r6, r2
	mul	r6, r3
	cmp	r7, r6
	bcs	.Lf70
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r4]
	ldr	r3, [sp, #28]
	cmp	r3, #0
	bne	.LfCB565
	b	.Lf18	@long jump
.LfCB565:
	ldr	r0, [sp, #44]
	cmp	r0, #6
	bgt	.Lf73
	mov	r1, #9
	str	r1, [sp, #44]
	b	.Lf75
.Lf73:
	mov	r2, #11
	str	r2, [sp, #44]
.Lf75:
	ldr	r0, [sp, #28]
	ldr	r1, [sp, #120]
	ldr	r2, [sp, #144]
	sub	r3, r0, r1
	ldrb	r3, [r2, r3]
	strb	r3, [r2, r0]
	mov	r2, r0
	mov	ip, r3
	add	r2, r2, #1
	mov	r1, r6
	b	.Lf49
.Lf70:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	sub	r0, r0, r6
	sub	r7, r7, r6
	strh	r3, [r4]
	b	.Lf76
.Lf172:
	.align	2
.Lf171:
	.word	1846
	.word	16777215
	.word	3692
	.word	1636
.Lf64:
	lsr	r3, r0, #5
	sub	r3, r0, r3
	ldr	r0, .Lf173
	sub	r4, r4, r2
	sub	r7, r7, r2
	strh	r3, [r6]
	cmp	r4, r0
	bhi	.Lf77
	ldr	r1, [sp, #64]
	cmp	r5, r1
	bne	.LfCB621
	b	.Lf18	@long jump
.LfCB621:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r4, r4, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf77:
	mov	r6, #216
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r2, [r6]
	lsr	r3, r4, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf80
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r6]
	ldr	r2, [sp, #48]
	ldr	r3, [sp, #120]
	str	r3, [sp, #48]
	b	.Lf168
.Lf80:
	sub	r1, r4, r0
	sub	r7, r7, r0
	lsr	r3, r2, #5
	ldr	r0, .Lf173
	sub	r3, r2, r3
	strh	r3, [r6]
	cmp	r1, r0
	bhi	.Lf82
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LfCB661
	b	.Lf18	@long jump
.LfCB661:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf82:
	mov	r6, #228
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r2, [r6]
	lsr	r3, r1, #11
	mov	r4, r2
	mul	r4, r3
	cmp	r7, r4
	bcs	.Lf85
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	ldr	r0, [sp, #120]
	ldr	r2, [sp, #52]
	strh	r3, [r6]
	ldr	r3, [sp, #48]
	str	r0, [sp, #48]
	mov	r0, r4
	str	r3, [sp, #52]
	str	r2, [sp, #120]
	b	.Lf76
.Lf85:
	lsr	r3, r2, #5
	sub	r0, r1, r4
	ldr	r1, [sp, #52]
	sub	r3, r2, r3
	strh	r3, [r6]
	ldr	r2, [sp, #56]
	ldr	r3, [sp, #48]
	str	r1, [sp, #56]
	ldr	r1, [sp, #120]
	str	r3, [sp, #52]
	str	r1, [sp, #48]
	sub	r7, r7, r4
.Lf168:
	str	r2, [sp, #120]
.Lf76:
	ldr	r2, [sp, #44]
	cmp	r2, #6
	bgt	.Lf87
	mov	r3, #8
	str	r3, [sp, #44]
	b	.Lf89
.Lf87:
	mov	r1, #11
	str	r1, [sp, #44]
.Lf89:
	ldr	r2, [sp, #24]
	ldr	r3, .Lf173+4
	add	r6, r2, r3
.Lf60:
	ldr	r1, .Lf173
	mov	ip, r1
	cmp	r0, ip
	bhi	.Lf90
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LfCB736
	b	.Lf18	@long jump
.LfCB736:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r0, r0, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf90:
	ldrh	r2, [r6]
	lsr	r3, r0, #11
	mov	r1, r2
	mul	r1, r3
	cmp	r7, r1
	bcs	.Lf93
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	ldr	r0, [sp, #68]
	add	r3, r2, r3
	strh	r3, [r6]
	lsl	r3, r0, #4
	add	r3, r6, r3
	add	r3, r3, #4
	str	r3, [sp, #88]
	mov	r2, #3
	mov	r3, #0
	str	r2, [sp, #80]
	str	r3, [sp, #84]
	b	.Lf95
.Lf93:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	sub	r4, r0, r1
	sub	r7, r7, r1
	strh	r3, [r6]
	cmp	r4, ip
	bhi	.Lf96
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LfCB778
	b	.Lf18	@long jump
.LfCB778:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r4, r4, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf96:
	ldrh	r2, [r6, #2]
	lsr	r3, r4, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf99
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	ldr	r1, [sp, #68]
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r6, #2]
	mov	r2, #130
	lsl	r3, r1, #4
	add	r3, r6, r3
	lsl	r2, r2, #1
	add	r2, r3, r2
	mov	r1, r0
	mov	r3, #3
	mov	r0, #8
	str	r2, [sp, #88]
	str	r3, [sp, #80]
	b	.Lf169
.Lf99:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	mov	r2, #129
	lsl	r2, r2, #2
	strh	r3, [r6, #2]
	add	r2, r6, r2
	mov	r3, #8
	str	r2, [sp, #88]
	str	r3, [sp, #80]
	sub	r1, r4, r0
	sub	r7, r7, r0
	mov	r0, #16
.Lf169:
	str	r0, [sp, #84]
.Lf95:
	ldr	r2, [sp, #80]
	mov	r3, #1
	str	r3, [sp, #60]
	mov	ip, r2
.Lf101:
	ldr	r3, .Lf173
	cmp	r1, r3
	bhi	.Lf102
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LfCB842
	b	.Lf18	@long jump
.LfCB842:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf102:
	ldr	r2, [sp, #60]
	ldr	r3, [sp, #88]
	lsl	r6, r2, #1
	add	r4, r3, r6
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf105
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	mov	r1, r0
	strh	r3, [r4]
	str	r6, [sp, #60]
	b	.Lf107
.Lf105:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r3, [sp, #60]
	sub	r1, r1, r0
	sub	r7, r7, r0
	ldr	r0, [sp, #60]
	add	r3, r3, #1
	add	r0, r0, r3
	str	r0, [sp, #60]
.Lf107:
	mov	r2, #1
	neg	r2, r2
	add	ip, ip, r2
	mov	r3, ip
	cmp	r3, #0
	bne	.Lf101
	ldr	r0, [sp, #80]
	ldr	r2, [sp, #60]
	mov	r3, #1
	lsl	r3, r3, r0
	ldr	r0, [sp, #84]
	sub	r3, r2, r3
	ldr	r2, [sp, #44]
	add	r3, r3, r0
	str	r3, [sp, #116]
	cmp	r2, #3
	ble	.LfCB901
	b	.Lf109	@long jump
.LfCB901:
	cmp	r3, #3
	ble	.Lf111
	mov	r3, #3
.Lf111:
	ldr	r0, [sp, #24]
	lsl	r3, r3, #7
	mov	r2, #216
	add	r3, r0, r3
	lsl	r2, r2, #2
	add	r2, r3, r2
	mov	r3, #6
	str	r2, [sp, #108]
	str	r3, [sp, #92]
	mov	r6, #1
.Lf112:
	ldr	r3, .Lf173
	cmp	r1, r3
	bhi	.Lf113
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LfCB925
	b	.Lf18	@long jump
.LfCB925:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf113:
	lsl	r2, r6, #1
	ldr	r4, [sp, #108]
	mov	ip, r2
	add	r4, r4, ip
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf116
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	mov	r6, ip
	mov	r1, r0
	strh	r3, [r4]
	b	.Lf118
.Lf116:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	strh	r3, [r4]
	add	r3, r6, #1
	sub	r1, r1, r0
	sub	r7, r7, r0
	add	r6, r6, r3
.Lf118:
	ldr	r3, [sp, #92]
	sub	r3, r3, #1
	str	r3, [sp, #92]
	cmp	r3, #0
	bne	.Lf112
	mov	r0, r6
	sub	r0, r0, #64
	cmp	r0, #3
	ble	.Lf120
	asr	r6, r0, #1
	sub	r2, r6, #1
	mov	r3, #1
	mov	ip, r2
	mov	r2, r0
	and	r2, r2, r3
	mov	r4, r2
	mov	r3, #2
	orr	r4, r4, r3
	cmp	r0, #13
	bgt	.Lf122
	mov	r3, ip
	mov	r6, r4
	ldr	r2, [sp, #24]
	lsl	r6, r6, r3
	lsl	r3, r6, #1
	add	r3, r2, r3
	lsl	r2, r0, #1
	ldr	r0, .Lf173+8
	sub	r3, r3, r2
	add	r0, r3, r0
	str	r0, [sp, #104]
	b	.Lf124
.Lf122:
	sub	r6, r6, #5
	mov	r0, #0
.Lf125:
	ldr	r3, .Lf173
	cmp	r1, r3
	bhi	.Lf126
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LfCB1010
	b	.Lf18	@long jump
.LfCB1010:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf126:
	lsr	r1, r1, #1
	lsl	r4, r4, #1
	cmp	r7, r1
	bcc	.Lf129
	mov	r3, #1
	sub	r7, r7, r1
	orr	r4, r4, r3
.Lf129:
	add	r0, r0, #1
	cmp	r0, r6
	bne	.Lf125
	ldr	r0, .Lf173+12
	ldr	r3, [sp, #24]
	add	r0, r3, r0
	mov	r2, #4
	str	r0, [sp, #104]
	lsl	r6, r4, #4
	mov	ip, r2
.Lf124:
	mov	r3, #1
	str	r3, [sp, #96]
	str	r3, [sp, #100]
.Lf132:
	ldr	r3, .Lf173
	cmp	r1, r3
	bhi	.Lf133
	ldr	r0, [sp, #64]
	cmp	r5, r0
	beq	.Lf18
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Lf133:
	ldr	r2, [sp, #100]
	ldr	r3, [sp, #104]
	lsl	r2, r2, #1
	add	r4, r3, r2
	str	r2, [sp, #8]
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Lf136
	mov	r3, #128
	lsl	r3, r3, #4
	ldr	r1, [sp, #8]
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	str	r1, [sp, #100]
	mov	r1, r0
	strh	r3, [r4]
	b	.Lf138
.Lf136:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	strh	r3, [r4]
	ldr	r3, [sp, #100]
	ldr	r2, [sp, #100]
	add	r3, r3, #1
	add	r2, r2, r3
	ldr	r3, [sp, #96]
	str	r2, [sp, #100]
	sub	r1, r1, r0
	sub	r7, r7, r0
	orr	r6, r6, r3
.Lf138:
	mov	r0, #1
	neg	r0, r0
	add	ip, ip, r0
	mov	r2, ip
	cmp	r2, #0
	beq	.Lf139
	ldr	r3, [sp, #96]
	lsl	r3, r3, #1
	str	r3, [sp, #96]
	b	.Lf132
.Lf174:
	.align	2
.Lf173:
	.word	16777215
	.word	2664
	.word	1374
	.word	1604
.Lf120:
	mov	r6, r0
.Lf139:
	add	r6, r6, #1
	str	r6, [sp, #120]
	cmp	r6, #0
	beq	.Lf141
	ldr	r0, [sp, #44]
	add	r0, r0, #7
	str	r0, [sp, #44]
.Lf109:
	ldr	r2, [sp, #120]
	ldr	r3, [sp, #28]
	cmp	r2, r3
	bhi	.Lf18
	sub	r3, r3, r2
	ldr	r0, [sp, #144]
	ldr	r6, [sp, #116]
	ldr	r2, [sp, #28]
	add	r4, r0, r3
	add	r6, r6, #2
	add	r0, r0, r2
.Lf144:
	ldrb	r3, [r4]
	strb	r3, [r0]
	mov	ip, r3
	ldr	r3, [sp, #28]
	add	r2, r2, #1
	add	r3, r3, r6
	str	r3, [sp]
	cmp	r2, r3
	beq	.Lf49
	ldr	r3, [sp, #148]
	add	r4, r4, #1
	add	r0, r0, #1
	cmp	r2, r3
	bne	.Lf144
.Lf49:
	str	r2, [sp, #28]
.Lf21:
	ldr	r0, [sp, #28]
	ldr	r2, [sp, #148]
	cmp	r0, r2
	bcs	.LfCB1173
	b	.Lf22	@long jump
.LfCB1173:
.Lf141:
	ldr	r3, .Lf175
	cmp	r1, r3
	bhi	.Lf146
	ldr	r3, [sp, #64]
	cmp	r5, r3
	beq	.Lf18
	add	r5, r5, #1
.Lf146:
	ldr	r0, [sp, #16]
	ldr	r1, [sp, #12]
	sub	r3, r5, r0
	str	r3, [r1]
	ldr	r2, [sp, #28]
	ldr	r3, [sp, #152]
	mov	r0, #0
	str	r2, [r3]
	b	.Lf149
.Lf18:
	mov	r0, #1
.Lf149:
	add	sp, sp, #124
	@ sp needed for prologue
	pop	{r4, r5, r6, r7, pc}
.Lf176:
	.align	2
.Lf175:
	.word	16777215

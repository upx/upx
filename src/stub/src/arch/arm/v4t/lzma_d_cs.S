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
	ldr	r0, .Ls171
	lsl	r3, r3, #2
	lsl	r3, r3, r2
	ldr	r2, [sp, #24]
	add	r4, r3, r0
	mov	r0, #0
	b	.Ls14
.Ls15:
	mov	r3, #128
	lsl	r3, r3, #3
	strh	r3, [r2]
	add	r0, r0, #1
	add	r2, r2, #2
.Ls14:
	cmp	r0, r4
	bne	.Ls15
	ldr	r1, [sp, #16]
	add	r5, r1, r5
	str	r5, [sp, #64]
	mov	r7, #0
	mov	r5, r1
	mov	r0, #0
.Ls17:
	ldr	r2, [sp, #64]
	ldr	r1, [sp, #16]
	sub	r3, r2, r1
	cmp	r0, r3
	bne	.LsCB158
	b	.Ls18	@long jump
.LsCB158:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	add	r0, r0, #1
	orr	r7, r7, r3
	add	r5, r5, #1
	cmp	r0, #5
	bne	.Ls17
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
	b	.Ls21
.Ls22:
	ldr	r0, .Ls171+4
	cmp	r1, r0
	bhi	.Ls23
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LsCB190
	b	.Ls18	@long jump
.LsCB190:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls23:
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
	bcc	.LsCB214
	b	.Ls26	@long jump
.LsCB214:
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
	ldr	r0, .Ls171+8
	add	r3, r2, r3
	ldr	r1, [sp, #44]
	add	r0, r3, r0
	str	r0, [sp, #112]
	cmp	r1, #6
	bgt	.Ls28
	mov	r2, #1
	mov	r1, r6
	str	r2, [sp, #72]
	b	.Ls166
.Ls28:
	ldr	r1, [sp, #120]
	ldr	r0, [sp, #28]
	ldr	r2, [sp, #144]
	sub	r3, r0, r1
	ldrb	r3, [r2, r3]
	str	r3, [sp, #76]
	mov	r3, #1
	str	r3, [sp, #72]
	mov	r1, r6
.Ls31:
	ldr	r3, .Ls171+4
	cmp	r1, r3
	bhi	.Ls32
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LsCB267
	b	.Ls18	@long jump
.LsCB267:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls32:
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
	bcs	.Ls35
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	ldr	r1, [sp, #4]
	add	r3, r2, r3
	strh	r3, [r4]
	str	r1, [sp, #72]
	cmp	r6, #0
	bne	.Ls167
	mov	r1, r0
	b	.Ls39
.Ls35:
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
	beq	.Ls166
.Ls39:
	ldr	r3, [sp, #72]
	cmp	r3, #255
	bgt	.Ls46
	b	.Ls31
.Ls40:
	ldr	r3, .Ls171+4
	cmp	r1, r3
	bhi	.Ls41
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LsCB340
	b	.Ls18	@long jump
.LsCB340:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls41:
	ldr	r2, [sp, #72]
	ldr	r3, [sp, #112]
	lsl	r6, r2, #1
	add	r4, r3, r6
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Ls44
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r4]
	str	r6, [sp, #72]
.Ls167:
	mov	r1, r0
	b	.Ls166
.Ls44:
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
.Ls166:
	ldr	r2, [sp, #72]
	cmp	r2, #255
	ble	.Ls40
.Ls46:
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
	bgt	.Ls47
	mov	r0, #0
	b	.Ls170
.Ls47:
	ldr	r3, [sp, #44]
	cmp	r3, #9
	bgt	.Ls50
	sub	r3, r3, #3
	str	r3, [sp, #44]
	b	.Ls49
.Ls50:
	ldr	r0, [sp, #44]
	sub	r0, r0, #6
.Ls170:
	str	r0, [sp, #44]
	b	.Ls49
.Ls26:
	sub	r0, r1, r6
	lsr	r3, r2, #5
	ldr	r1, .Ls171+4
	sub	r3, r2, r3
	sub	r7, r7, r6
	strh	r3, [r4]
	cmp	r0, r1
	bhi	.Ls52
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LsCB435
	b	.Ls18	@long jump
.LsCB435:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r0, r0, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls52:
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
	bcs	.Ls55
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r6]
	ldr	r3, [sp, #44]
	cmp	r3, #6
	bgt	.Ls57
	mov	r0, #0
	str	r0, [sp, #44]
	b	.Ls59
.Ls57:
	mov	r2, #3
	str	r2, [sp, #44]
.Ls59:
	ldr	r3, [sp, #24]
	ldr	r0, .Ls171+12
	add	r6, r3, r0
	ldr	r0, [sp, #120]
	ldr	r2, [sp, #52]
	ldr	r3, [sp, #48]
	str	r0, [sp, #48]
	mov	r0, r1
	str	r2, [sp, #56]
	str	r3, [sp, #52]
	b	.Ls60
.Ls55:
	sub	r4, r0, r1
	sub	r7, r7, r1
	lsr	r3, r2, #5
	ldr	r1, .Ls171+4
	sub	r3, r2, r3
	strh	r3, [r6]
	cmp	r4, r1
	bhi	.Ls61
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LsCB500
	b	.Ls18	@long jump
.LsCB500:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r4, r4, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls61:
	mov	r6, #204
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r0, [r6]
	lsr	r3, r4, #11
	mov	r2, r0
	mul	r2, r3
	cmp	r7, r2
	bcs	.Ls64
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r0
	asr	r3, r3, #5
	add	r3, r0, r3
	strh	r3, [r6]
	ldr	r3, .Ls171+4
	cmp	r2, r3
	bls	.Ls66
	mov	r0, r2
	b	.Ls68
.Ls66:
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LsCB533
	b	.Ls18	@long jump
.LsCB533:
	ldrb	r3, [r5]
	lsl	r0, r2, #8
	lsl	r2, r7, #8
	mov	r7, r2
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls68:
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
	bcs	.Ls70
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r4]
	ldr	r3, [sp, #28]
	cmp	r3, #0
	bne	.LsCB565
	b	.Ls18	@long jump
.LsCB565:
	ldr	r0, [sp, #44]
	cmp	r0, #6
	bgt	.Ls73
	mov	r1, #9
	str	r1, [sp, #44]
	b	.Ls75
.Ls73:
	mov	r2, #11
	str	r2, [sp, #44]
.Ls75:
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
	b	.Ls49
.Ls70:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	sub	r0, r0, r6
	sub	r7, r7, r6
	strh	r3, [r4]
	b	.Ls76
.Ls172:
	.align	2
.Ls171:
	.word	1846
	.word	16777215
	.word	3692
	.word	1636
.Ls64:
	lsr	r3, r0, #5
	sub	r3, r0, r3
	ldr	r0, .Ls173
	sub	r4, r4, r2
	sub	r7, r7, r2
	strh	r3, [r6]
	cmp	r4, r0
	bhi	.Ls77
	ldr	r1, [sp, #64]
	cmp	r5, r1
	bne	.LsCB621
	b	.Ls18	@long jump
.LsCB621:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r4, r4, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls77:
	mov	r6, #216
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r2, [r6]
	lsr	r3, r4, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Ls80
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	strh	r3, [r6]
	ldr	r2, [sp, #48]
	ldr	r3, [sp, #120]
	str	r3, [sp, #48]
	b	.Ls168
.Ls80:
	sub	r1, r4, r0
	sub	r7, r7, r0
	lsr	r3, r2, #5
	ldr	r0, .Ls173
	sub	r3, r2, r3
	strh	r3, [r6]
	cmp	r1, r0
	bhi	.Ls82
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LsCB661
	b	.Ls18	@long jump
.LsCB661:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls82:
	mov	r6, #228
	lsl	r6, r6, #1
	add	r6, r6, ip
	ldrh	r2, [r6]
	lsr	r3, r1, #11
	mov	r4, r2
	mul	r4, r3
	cmp	r7, r4
	bcs	.Ls85
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
	b	.Ls76
.Ls85:
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
.Ls168:
	str	r2, [sp, #120]
.Ls76:
	ldr	r2, [sp, #44]
	cmp	r2, #6
	bgt	.Ls87
	mov	r3, #8
	str	r3, [sp, #44]
	b	.Ls89
.Ls87:
	mov	r1, #11
	str	r1, [sp, #44]
.Ls89:
	ldr	r2, [sp, #24]
	ldr	r3, .Ls173+4
	add	r6, r2, r3
.Ls60:
	ldr	r1, .Ls173
	mov	ip, r1
	cmp	r0, ip
	bhi	.Ls90
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LsCB736
	b	.Ls18	@long jump
.LsCB736:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r0, r0, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls90:
	ldrh	r2, [r6]
	lsr	r3, r0, #11
	mov	r1, r2
	mul	r1, r3
	cmp	r7, r1
	bcs	.Ls93
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
	b	.Ls95
.Ls93:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	sub	r4, r0, r1
	sub	r7, r7, r1
	strh	r3, [r6]
	cmp	r4, ip
	bhi	.Ls96
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LsCB778
	b	.Ls18	@long jump
.LsCB778:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r4, r4, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls96:
	ldrh	r2, [r6, #2]
	lsr	r3, r4, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Ls99
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
	b	.Ls169
.Ls99:
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
.Ls169:
	str	r0, [sp, #84]
.Ls95:
	ldr	r2, [sp, #80]
	mov	r3, #1
	str	r3, [sp, #60]
	mov	ip, r2
.Ls101:
	ldr	r3, .Ls173
	cmp	r1, r3
	bhi	.Ls102
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LsCB842
	b	.Ls18	@long jump
.LsCB842:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls102:
	ldr	r2, [sp, #60]
	ldr	r3, [sp, #88]
	lsl	r6, r2, #1
	add	r4, r3, r6
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Ls105
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	mov	r1, r0
	strh	r3, [r4]
	str	r6, [sp, #60]
	b	.Ls107
.Ls105:
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
.Ls107:
	mov	r2, #1
	neg	r2, r2
	add	ip, ip, r2
	mov	r3, ip
	cmp	r3, #0
	bne	.Ls101
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
	ble	.LsCB901
	b	.Ls109	@long jump
.LsCB901:
	cmp	r3, #3
	ble	.Ls111
	mov	r3, #3
.Ls111:
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
.Ls112:
	ldr	r3, .Ls173
	cmp	r1, r3
	bhi	.Ls113
	ldr	r0, [sp, #64]
	cmp	r5, r0
	bne	.LsCB925
	b	.Ls18	@long jump
.LsCB925:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls113:
	lsl	r2, r6, #1
	ldr	r4, [sp, #108]
	mov	ip, r2
	add	r4, r4, ip
	ldrh	r2, [r4]
	lsr	r3, r1, #11
	mov	r0, r2
	mul	r0, r3
	cmp	r7, r0
	bcs	.Ls116
	mov	r3, #128
	lsl	r3, r3, #4
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	mov	r6, ip
	mov	r1, r0
	strh	r3, [r4]
	b	.Ls118
.Ls116:
	lsr	r3, r2, #5
	sub	r3, r2, r3
	strh	r3, [r4]
	add	r3, r6, #1
	sub	r1, r1, r0
	sub	r7, r7, r0
	add	r6, r6, r3
.Ls118:
	ldr	r3, [sp, #92]
	sub	r3, r3, #1
	str	r3, [sp, #92]
	cmp	r3, #0
	bne	.Ls112
	mov	r0, r6
	sub	r0, r0, #64
	cmp	r0, #3
	ble	.Ls120
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
	bgt	.Ls122
	mov	r3, ip
	mov	r6, r4
	ldr	r2, [sp, #24]
	lsl	r6, r6, r3
	lsl	r3, r6, #1
	add	r3, r2, r3
	lsl	r2, r0, #1
	ldr	r0, .Ls173+8
	sub	r3, r3, r2
	add	r0, r3, r0
	str	r0, [sp, #104]
	b	.Ls124
.Ls122:
	sub	r6, r6, #5
	mov	r0, #0
.Ls125:
	ldr	r3, .Ls173
	cmp	r1, r3
	bhi	.Ls126
	ldr	r2, [sp, #64]
	cmp	r5, r2
	bne	.LsCB1010
	b	.Ls18	@long jump
.LsCB1010:
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls126:
	lsr	r1, r1, #1
	lsl	r4, r4, #1
	cmp	r7, r1
	bcc	.Ls129
	mov	r3, #1
	sub	r7, r7, r1
	orr	r4, r4, r3
.Ls129:
	add	r0, r0, #1
	cmp	r0, r6
	bne	.Ls125
	ldr	r0, .Ls173+12
	ldr	r3, [sp, #24]
	add	r0, r3, r0
	mov	r2, #4
	str	r0, [sp, #104]
	lsl	r6, r4, #4
	mov	ip, r2
.Ls124:
	mov	r3, #1
	str	r3, [sp, #96]
	str	r3, [sp, #100]
.Ls132:
	ldr	r3, .Ls173
	cmp	r1, r3
	bhi	.Ls133
	ldr	r0, [sp, #64]
	cmp	r5, r0
	beq	.Ls18
	ldrb	r3, [r5]
	lsl	r2, r7, #8
	mov	r7, r2
	lsl	r1, r1, #8
	orr	r7, r7, r3
	add	r5, r5, #1
.Ls133:
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
	bcs	.Ls136
	mov	r3, #128
	lsl	r3, r3, #4
	ldr	r1, [sp, #8]
	sub	r3, r3, r2
	asr	r3, r3, #5
	add	r3, r2, r3
	str	r1, [sp, #100]
	mov	r1, r0
	strh	r3, [r4]
	b	.Ls138
.Ls136:
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
.Ls138:
	mov	r0, #1
	neg	r0, r0
	add	ip, ip, r0
	mov	r2, ip
	cmp	r2, #0
	beq	.Ls139
	ldr	r3, [sp, #96]
	lsl	r3, r3, #1
	str	r3, [sp, #96]
	b	.Ls132
.Ls174:
	.align	2
.Ls173:
	.word	16777215
	.word	2664
	.word	1374
	.word	1604
.Ls120:
	mov	r6, r0
.Ls139:
	add	r6, r6, #1
	str	r6, [sp, #120]
	cmp	r6, #0
	beq	.Ls141
	ldr	r0, [sp, #44]
	add	r0, r0, #7
	str	r0, [sp, #44]
.Ls109:
	ldr	r2, [sp, #120]
	ldr	r3, [sp, #28]
	cmp	r2, r3
	bhi	.Ls18
	sub	r3, r3, r2
	ldr	r0, [sp, #144]
	ldr	r6, [sp, #116]
	ldr	r2, [sp, #28]
	add	r4, r0, r3
	add	r6, r6, #2
	add	r0, r0, r2
.Ls144:
	ldrb	r3, [r4]
	strb	r3, [r0]
	mov	ip, r3
	ldr	r3, [sp, #28]
	add	r2, r2, #1
	add	r3, r3, r6
	str	r3, [sp]
	cmp	r2, r3
	beq	.Ls49
	ldr	r3, [sp, #148]
	add	r4, r4, #1
	add	r0, r0, #1
	cmp	r2, r3
	bne	.Ls144
.Ls49:
	str	r2, [sp, #28]
.Ls21:
	ldr	r0, [sp, #28]
	ldr	r2, [sp, #148]
	cmp	r0, r2
	bcs	.LsCB1173
	b	.Ls22	@long jump
.LsCB1173:
.Ls141:
	ldr	r3, .Ls175
	cmp	r1, r3
	bhi	.Ls146
	ldr	r3, [sp, #64]
	cmp	r5, r3
	beq	.Ls18
	add	r5, r5, #1
.Ls146:
	ldr	r0, [sp, #16]
	ldr	r1, [sp, #12]
	sub	r3, r5, r0
	str	r3, [r1]
	ldr	r2, [sp, #28]
	ldr	r3, [sp, #152]
	mov	r0, #0
	str	r2, [r3]
	b	.Ls149
.Ls18:
	mov	r0, #1
.Ls149:
	add	sp, sp, #124
	@ sp needed for prologue
	pop	{r4, r5, r6, r7, pc}
.Ls176:
	.align	2
.Ls175:
	.word	16777215

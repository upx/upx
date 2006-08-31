.arm
	@ args = 12, pretend = 0, frame = 48
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, lr}
	sub	sp, sp, #48
	str	r3, [sp, #0]
	ldrb	r3, [r0, #0]	@ zero_extendqisi2
	ldrb	r5, [r0, #2]	@ zero_extendqisi2
	ldrb	lr, [r0, #1]	@ zero_extendqisi2
	ldr	ip, [sp, #0]
	str	r3, [sp, #20]
	ldr	r3, [sp, #92]
	mov	r4, #0
	str	r4, [ip, #0]
	str	r4, [r3, #0]
	ldr	ip, [sp, #20]
	ldrb	r3, [r0, #1]	@ zero_extendqisi2
	add	r3, ip, r3
	mov	ip, #768
	mov	ip, ip, asl r3
	mov	r3, #1
	mov	lr, r3, asl lr
	mov	r3, r3, asl r5
	add	ip, ip, #1840
	sub	r3, r3, #1
	sub	lr, lr, #1
	add	r0, r0, #4
	add	ip, ip, #6
	str	r1, [sp, #4]
	str	r3, [sp, #12]
	str	lr, [sp, #16]
	str	r0, [sp, #8]
	b	.Ls14
.Ls15:
	ldr	r0, [sp, #8]
	mov	r1, #1024	@ movhi
	strh	r1, [r3, r0]	@ movhi
.Ls14:
	cmp	r4, ip
	mov	r3, r4, asl #1
	add	r4, r4, #1
	bne	.Ls15
	ldr	r3, [sp, #4]
	mov	r6, #0
	add	r9, r3, r2
	mov	lr, r3
	mov	r2, r6
.Ls17:
	ldr	r5, [sp, #4]
	rsb	r3, r5, r9
	cmp	r2, r3
	add	lr, lr, #1
	beq	.Ls18
	ldrb	r3, [r2, r5]	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r2, #5
	orr	r6, r3, r6, asl #8
	bne	.Ls17
	mov	fp, #0
	mov	ip, #1
	mov	r7, fp
	mvn	r0, #0
	str	ip, [sp, #44]
	str	fp, [sp, #24]
	str	ip, [sp, #28]
	str	ip, [sp, #32]
	str	ip, [sp, #36]
	b	.Ls172
.Ls22:
	mvn	r8, #-16777216
	cmp	r0, r8
	bhi	.Ls23
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r0, r0, asl #8
	orr	r6, r3, r6, asl #8
.Ls23:
	ldr	r1, [sp, #12]
	ldr	r2, [sp, #24]
	and	sl, fp, r1
	mov	r1, r2, asl #4
	add	r3, sl, r1
	mov	r5, r3, asl #1
	ldr	r3, [sp, #8]
	ldrh	ip, [r3, r5]
	mov	r3, r0, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls26
	ldr	r0, [sp, #20]
	rsb	r3, r0, #8
	mov	r3, r7, asr r3
	ldr	r1, [sp, #16]
	and	r2, fp, r1
	add	r3, r3, r2, asl r0
	ldr	r0, [sp, #8]
	mov	r2, #1536
	mla	r2, r3, r2, r0
	ldr	r1, [sp, #24]
	rsb	r3, ip, #2048
	cmp	r1, #6
	add	r3, ip, r3, asr #5
	add	r8, r2, #3680
	strh	r3, [r0, r5]	@ movhi
	add	r8, r8, #12
	movle	r0, r4
	movle	r1, #1
	ble	.Ls171
	ldr	r2, [sp, #44]
	ldr	r5, [sp, #84]
	rsb	r3, r2, fp
	ldrb	r7, [r5, r3]	@ zero_extendqisi2
	mov	r0, r4
	mov	r1, #1
.Ls31:
	mov	r7, r7, asl #1
	and	r5, r7, #256
	mov	sl, r1, asl #1
	add	r3, r8, r5, asl #1
	add	r3, r3, sl
	cmp	r0, #16777216
	add	r4, r3, #512
	add	ip, r1, #1
	bcs	.Ls32
	cmp	lr, r9
	mov	r0, r0, asl #8
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
.Ls32:
	ldrh	r3, [r4, #0]
	mov	r2, r0, lsr #11
	add	r1, r1, ip
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	add	r2, r3, r2, asr #5
	sub	r3, r3, r3, lsr #5
	bcs	.Ls35
	cmp	r5, #0
	mov	r0, ip
	strh	r2, [r4, #0]	@ movhi
	mov	r1, sl
	beq	.Ls39
	mov	r0, ip
	b	.Ls171
.Ls35:
	cmp	r5, #0
	strh	r3, [r4, #0]	@ movhi
	rsb	r6, ip, r6
	rsb	r0, ip, r0
	beq	.Ls171
.Ls39:
	cmp	r1, #255
	bgt	.Ls46
	b	.Ls31
.Ls40:
	cmp	r0, #16777216
	add	r1, r1, r3
	bcs	.Ls41
	cmp	lr, r9
	mov	r0, r0, asl #8
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
.Ls41:
	ldrh	r3, [r8, r4]
	mov	r2, r0, lsr #11
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	sub	r5, r3, r3, lsr #5
	add	r3, r3, r2, asr #5
	strcch	r3, [r8, r4]	@ movhi
	strcsh	r5, [r8, r4]	@ movhi
	rsb	r0, ip, r0
	movcc	r1, r4
	movcc	r0, ip
	rsbcs	r6, ip, r6
.Ls171:
	cmp	r1, #255
	mov	r4, r1, asl #1
	add	r3, r1, #1
	ble	.Ls40
.Ls46:
	ldr	ip, [sp, #24]
	and	r7, r1, #255
	cmp	ip, #3
	ldr	r1, [sp, #84]
	movle	r2, #0
	strb	r7, [r1, fp]
	add	fp, fp, #1
	strle	r2, [sp, #24]
	ble	.Ls172
	ldr	r3, [sp, #24]
	cmp	r3, #9
	ldrgt	r5, [sp, #24]
	suble	r3, r3, #3
	subgt	r5, r5, #6
	strle	r3, [sp, #24]
	strgt	r5, [sp, #24]
	b	.Ls172
.Ls26:
	sub	r3, ip, ip, lsr #5
	rsb	r2, r4, r0
	ldr	ip, [sp, #8]
	cmp	r2, r8
	strh	r3, [ip, r5]	@ movhi
	rsb	r6, r4, r6
	bhi	.Ls51
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
.Ls51:
	ldr	r0, [sp, #8]
	ldr	r3, [sp, #24]
	add	r7, r0, r3, asl #1
	add	r0, r7, #384
	ldrh	ip, [r0, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls54
	ldr	r2, [sp, #24]
	ldr	r5, [sp, #8]
	cmp	r2, #6
	rsb	r3, ip, #2048
	add	r1, r5, #1632
	movgt	r2, #3
	ldr	r5, [sp, #32]
	movle	r2, #0
	add	r3, ip, r3, asr #5
	str	r2, [sp, #24]
	ldr	ip, [sp, #28]
	ldr	r2, [sp, #44]
	str	r5, [sp, #36]
	add	r1, r1, #4
	mov	r5, r4
	str	ip, [sp, #32]
	str	r2, [sp, #28]
	strh	r3, [r0, #0]	@ movhi
	b	.Ls59
.Ls54:
	rsb	r2, r4, r2
	sub	r3, ip, ip, lsr #5
	cmp	r2, r8
	strh	r3, [r0, #0]	@ movhi
	rsb	r6, r4, r6
	bhi	.Ls60
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
.Ls60:
	add	r5, r7, #408
	ldrh	ip, [r5, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls63
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	cmp	r4, r8
	strh	r3, [r5, #0]	@ movhi
	bhi	.Ls67
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r4, r4, asl #8
	orr	r6, r3, r6, asl #8
.Ls67:
	ldr	r5, [sp, #8]
	add	r3, r5, r1, asl #1
	add	r3, r3, sl, asl #1
	add	r1, r3, #480
	ldrh	ip, [r1, #0]
	mov	r3, r4, lsr #11
	mul	r2, ip, r3
	cmp	r6, r2
	rsbcs	r6, r2, r6
	subcs	r3, ip, ip, lsr #5
	rsbcs	r5, r2, r4
	bcs	.Ls169
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	cmp	fp, #0
	strh	r3, [r1, #0]	@ movhi
	beq	.Ls18
	ldr	ip, [sp, #44]
	ldr	r1, [sp, #24]
	ldr	r0, [sp, #84]
	rsb	r3, ip, fp
	ldrb	r7, [r0, r3]	@ zero_extendqisi2
	cmp	r1, #6
	movgt	r1, #11
	movle	r1, #9
	str	r1, [sp, #24]
	strb	r7, [r0, fp]
	mov	r0, r2
	add	fp, fp, #1
	b	.Ls172
.Ls63:
	rsb	r2, r4, r2
	sub	r3, ip, ip, lsr #5
	cmp	r2, r8
	strh	r3, [r5, #0]	@ movhi
	rsb	r6, r4, r6
	bhi	.Ls76
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
.Ls76:
	add	r1, r7, #432
	ldrh	ip, [r1, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls79
	ldr	r5, [sp, #44]
	rsb	r3, ip, #2048
	ldr	r2, [sp, #28]
	add	r3, ip, r3, asr #5
	str	r5, [sp, #28]
	mov	r5, r4
.Ls170:
	str	r2, [sp, #44]
.Ls169:
	strh	r3, [r1, #0]	@ movhi
	b	.Ls75
.Ls79:
	rsb	r2, r4, r2
	sub	r3, ip, ip, lsr #5
	cmp	r2, r8
	strh	r3, [r1, #0]	@ movhi
	rsb	r6, r4, r6
	bhi	.Ls81
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
.Ls81:
	add	r1, r7, #456
	ldrh	ip, [r1, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls84
	rsb	r3, ip, #2048
	ldr	r2, [sp, #32]
	add	r3, ip, r3, asr #5
	ldr	r0, [sp, #44]
	ldr	ip, [sp, #28]
	mov	r5, r4
	str	ip, [sp, #32]
	str	r0, [sp, #28]
	b	.Ls170
.Ls84:
	sub	r3, ip, ip, lsr #5
	strh	r3, [r1, #0]	@ movhi
	ldr	ip, [sp, #44]
	add	r1, sp, #32
	ldmia	r1, {r1, r3}	@ phole ldm
	rsb	r5, r4, r2
	ldr	r2, [sp, #28]
	str	r1, [sp, #36]
	str	r2, [sp, #32]
	str	ip, [sp, #28]
	str	r3, [sp, #44]
	rsb	r6, r4, r6
.Ls75:
	ldr	r2, [sp, #24]
	ldr	r0, [sp, #8]
	cmp	r2, #6
	movgt	r2, #11
	movle	r2, #8
	add	r1, r0, #2656
	str	r2, [sp, #24]
	add	r1, r1, #8
.Ls59:
	mvn	r7, #-16777216
	cmp	r5, r7
	bhi	.Ls89
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r5, r5, asl #8
	orr	r6, r3, r6, asl #8
.Ls89:
	ldrh	ip, [r1, #0]
	mov	r3, r5, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls92
	add	r2, r1, sl, asl #4
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	add	r5, r2, #4
	mov	r0, r4
	mov	sl, #3
	mov	r8, #0
	strh	r3, [r1, #0]	@ movhi
	b	.Ls94
.Ls92:
	rsb	r2, r4, r5
	sub	r3, ip, ip, lsr #5
	cmp	r2, r7
	rsb	r6, r4, r6
	strh	r3, [r1, #0]	@ movhi
	bhi	.Ls95
	cmp	lr, r9
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
.Ls95:
	ldrh	ip, [r1, #2]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	.Ls98
	add	r2, r1, sl, asl #4
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	add	r5, r2, #260
	mov	r0, r4
	mov	sl, #3
	mov	r8, #8
	strh	r3, [r1, #2]	@ movhi
	b	.Ls94
.Ls98:
	sub	r3, ip, ip, lsr #5
	strh	r3, [r1, #2]	@ movhi
	rsb	r6, r4, r6
	rsb	r0, r4, r2
	add	r5, r1, #516
	mov	sl, #8
	mov	r8, #16
.Ls94:
	mov	r7, sl
	mov	r1, #1
.Ls100:
	add	r3, r1, #1
	cmp	r0, #16777216
	mov	r4, r1, asl #1
	add	r1, r1, r3
	bcs	.Ls101
	cmp	lr, r9
	mov	r0, r0, asl #8
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
.Ls101:
	ldrh	r3, [r5, r4]
	mov	r2, r0, lsr #11
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	add	r2, r3, r2, asr #5
	sub	r3, r3, r3, lsr #5
	rsb	r0, ip, r0
	movcc	r1, r4
	movcc	r0, ip
	strcch	r2, [r5, r4]	@ movhi
	rsbcs	r6, ip, r6
	strcsh	r3, [r5, r4]	@ movhi
	subs	r7, r7, #1
	bne	.Ls100
	mov	r2, #1
	sub	r3, r1, r2, asl sl
	ldr	r5, [sp, #24]
	add	r3, r3, r8
	cmp	r5, #3
	str	r3, [sp, #40]
	bgt	.Ls108
	ldr	ip, [sp, #8]
	cmp	r3, #3
	movge	r3, #3
	add	r3, ip, r3, asl #7
	add	r5, r3, #864
	mov	r7, r2
	mov	r8, #6
.Ls110:
	add	r3, r7, #1
	cmp	r0, #16777216
	mov	r4, r7, asl #1
	add	r7, r7, r3
	bcs	.Ls111
	cmp	lr, r9
	mov	r0, r0, asl #8
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
.Ls111:
	ldrh	r3, [r5, r4]
	mov	r2, r0, lsr #11
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	add	r2, r3, r2, asr #5
	sub	r3, r3, r3, lsr #5
	rsb	r0, ip, r0
	movcc	r7, r4
	movcc	r0, ip
	strcch	r2, [r5, r4]	@ movhi
	rsbcs	r6, ip, r6
	strcsh	r3, [r5, r4]	@ movhi
	subs	r8, r8, #1
	bne	.Ls110
	sub	r2, r7, #64
	cmp	r2, #3
	movle	r5, r2
	ble	.Ls137
	mov	r4, r2, asr #1
	cmp	r2, #13
	and	r3, r2, #1
	sub	r7, r4, #1
	orr	ip, r3, #2
	subgt	r4, r4, #5
	movgt	r2, r8
	bgt	.Ls123
	mov	r5, ip, asl r7
	ldr	r1, [sp, #8]
	add	r3, r1, r5, asl #1
	sub	r3, r3, r2, asl #1
	add	r4, r3, #1360
	add	r4, r4, #14
	b	.Ls122
.Ls123:
	cmp	r0, #16777216
	add	r2, r2, #1
	bcs	.Ls124
	cmp	lr, r9
	mov	r0, r0, asl #8
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
.Ls124:
	mov	r0, r0, lsr #1
	cmp	r6, r0
	mov	ip, ip, asl #1
	rsbcs	r6, r0, r6
	orrcs	ip, ip, #1
	cmp	r2, r4
	bne	.Ls123
	ldr	r2, [sp, #8]
	add	r4, r2, #1600
	add	r4, r4, #4
	mov	r5, ip, asl #4
	mov	r7, #4
.Ls122:
	mov	sl, #1
	mov	r8, sl
.Ls130:
	cmp	r0, #16777216
	mov	r1, r8, asl #1
	add	ip, r8, #1
	bcs	.Ls131
	cmp	lr, r9
	mov	r0, r0, asl #8
	beq	.Ls18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
.Ls131:
	ldrh	r3, [r4, r1]
	mov	r2, r0, lsr #11
	add	r8, r8, ip
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	add	r2, r3, r2, asr #5
	sub	r3, r3, r3, lsr #5
	orrcs	r5, r5, sl
	movcc	r8, r1
	movcc	r0, ip
	strcch	r2, [r4, r1]	@ movhi
	strcsh	r3, [r4, r1]	@ movhi
	rsbcs	r6, ip, r6
	rsbcs	r0, ip, r0
	subs	r7, r7, #1
	mov	sl, sl, asl #1
	bne	.Ls130
.Ls137:
	adds	r5, r5, #1
	str	r5, [sp, #44]
	beq	.Ls139
	ldr	r3, [sp, #24]
	add	r3, r3, #7
	str	r3, [sp, #24]
.Ls108:
	ldr	r5, [sp, #44]
	cmp	r5, fp
	bhi	.Ls18
	ldr	ip, [sp, #40]
	ldr	r1, [sp, #84]
	rsb	r3, r5, fp
	add	r2, ip, #2
	add	r4, r1, r3
	add	ip, r1, fp
.Ls142:
	subs	r2, r2, #1
	ldr	r5, [sp, #88]
	moveq	r3, #0
	movne	r3, #1
	add	fp, fp, #1
	cmp	fp, r5
	movcs	r3, #0
	andcc	r3, r3, #1
	ldrb	r7, [r4], #1	@ zero_extendqisi2
	cmp	r3, #0
	strb	r7, [ip], #1
	bne	.Ls142
.Ls172:
	ldr	ip, [sp, #88]
	cmp	fp, ip
	bcc	.Ls22
.Ls139:
	cmp	r0, #16777216
	bcs	.Ls143
	cmp	lr, r9
	beq	.Ls18
	add	lr, lr, #1
.Ls143:
	ldr	r0, [sp, #4]
	ldr	r1, [sp, #0]
	ldr	r2, [sp, #92]
	rsb	r3, r0, lr
	mov	r0, #0
	str	r3, [r1, #0]
	str	fp, [r2, #0]
	b	.Ls146
.Ls18:
	mov	r0, #1
.Ls146:
	add	sp, sp, #48
	ldmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, pc}

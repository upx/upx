	@ args = 12, pretend = 0, frame = 52
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, fp, lr}
	sub	sp, sp, #52
	str	r3, [sp, #0]
	ldrb	r3, [r0, #0]	@ zero_extendqisi2
	ldrb	r5, [r0, #2]	@ zero_extendqisi2
	ldrb	lr, [r0, #1]	@ zero_extendqisi2
	ldr	ip, [sp, #0]
	str	r3, [sp, #24]
	ldr	r3, [sp, #92]
	mov	r4, #0
	str	r4, [ip, #0]
	str	r4, [r3, #0]
	ldr	ip, [sp, #24]
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
	str	r3, [sp, #16]
	str	lr, [sp, #20]
	str	r0, [sp, #8]
	b	f.L14
f.L15:
	ldr	r0, [sp, #8]
	mov	r1, #1024	@ movhi
	strh	r1, [r3, r0]	@ movhi
f.L14:
	cmp	r4, ip
	mov	r3, r4, asl #1
	add	r4, r4, #1
	bne	f.L15
	ldr	r3, [sp, #4]
	mov	r6, #0
	add	fp, r3, r2
	mov	lr, r3
	mov	r2, r6
f.L17:
	ldr	r5, [sp, #4]
	rsb	r3, r5, fp
	cmp	r2, r3
	add	lr, lr, #1
	beq	f.L18
	ldrb	r3, [r2, r5]	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r2, #5
	orr	r6, r3, r6, asl #8
	bne	f.L17
	mov	r0, #0
	mov	ip, #1
	str	r0, [sp, #12]
	mov	r7, r0
	str	r0, [sp, #28]
	mvn	r0, #0
	str	ip, [sp, #48]
	str	ip, [sp, #32]
	str	ip, [sp, #36]
	str	ip, [sp, #40]
	b	f.L172
f.L22:
	mvn	r8, #-16777216
	cmp	r0, r8
	bhi	f.L23
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r0, r0, asl #8
	orr	r6, r3, r6, asl #8
f.L23:
	add	r1, sp, #12
	ldmia	r1, {r1, r2}	@ phole ldm
	ldr	r3, [sp, #28]
	and	r9, r1, r2
	mov	r1, r3, asl #4
	add	r3, r9, r1
	ldr	r2, [sp, #8]
	mov	r5, r3, asl #1
	ldrh	ip, [r2, r5]
	mov	r3, r0, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L26
	ldr	r0, [sp, #24]
	rsb	r3, r0, #8
	mov	r3, r7, asr r3
	ldr	r1, [sp, #12]
	ldr	r0, [sp, #20]
	and	r2, r1, r0
	ldr	r1, [sp, #24]
	add	r3, r3, r2, asl r1
	ldr	r0, [sp, #8]
	mov	r2, #1536
	mla	r2, r3, r2, r0
	ldr	r1, [sp, #28]
	rsb	r3, ip, #2048
	cmp	r1, #6
	add	r3, ip, r3, asr #5
	add	r8, r2, #3680
	strh	r3, [r0, r5]	@ movhi
	add	r8, r8, #12
	movle	r0, r4
	movle	r1, #1
	ble	f.L171
	ldr	r2, [sp, #12]
	ldr	r5, [sp, #48]
	ldr	ip, [sp, #84]
	rsb	r3, r5, r2
	ldrb	r7, [ip, r3]	@ zero_extendqisi2
	mov	r0, r4
	mov	r1, #1
f.L31:
	mov	r7, r7, asl #1
	and	r5, r7, #256
	mov	r9, r1, asl #1
	add	r3, r8, r5, asl #1
	add	r3, r3, r9
	cmp	r0, #16777216
	add	r4, r3, #512
	add	ip, r1, #1
	bcs	f.L32
	cmp	lr, fp
	mov	r0, r0, asl #8
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
f.L32:
	ldrh	r3, [r4, #0]
	mov	r2, r0, lsr #11
	add	r1, r1, ip
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	add	r2, r3, r2, asr #5
	sub	r3, r3, r3, lsr #5
	bcs	f.L35
	cmp	r5, #0
	mov	r0, ip
	strh	r2, [r4, #0]	@ movhi
	mov	r1, r9
	beq	f.L39
	mov	r0, ip
	b	f.L171
f.L35:
	cmp	r5, #0
	strh	r3, [r4, #0]	@ movhi
	rsb	r6, ip, r6
	rsb	r0, ip, r0
	beq	f.L171
f.L39:
	cmp	r1, #255
	bgt	f.L46
	b	f.L31
f.L40:
	cmp	r0, #16777216
	add	r1, r1, r3
	bcs	f.L41
	cmp	lr, fp
	mov	r0, r0, asl #8
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
f.L41:
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
f.L171:
	cmp	r1, #255
	mov	r4, r1, asl #1
	add	r3, r1, #1
	ble	f.L40
f.L46:
	and	r7, r1, #255
	ldr	r1, [sp, #28]
	ldr	r3, [sp, #12]
	ldr	r2, [sp, #84]
	cmp	r1, #3
	strb	r7, [r2, r3]
	movle	r5, #0
	add	r3, r3, #1
	str	r3, [sp, #12]
	strle	r5, [sp, #28]
	ble	f.L172
	ldr	ip, [sp, #28]
	cmp	ip, #9
	ldrgt	r1, [sp, #28]
	suble	ip, ip, #3
	subgt	r1, r1, #6
	strle	ip, [sp, #28]
	strgt	r1, [sp, #28]
	b	f.L172
f.L26:
	sub	r3, ip, ip, lsr #5
	rsb	r2, r4, r0
	ldr	ip, [sp, #8]
	cmp	r2, r8
	strh	r3, [ip, r5]	@ movhi
	rsb	r6, r4, r6
	bhi	f.L51
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
f.L51:
	ldr	r0, [sp, #8]
	ldr	r3, [sp, #28]
	add	r7, r0, r3, asl #1
	add	r0, r7, #384
	ldrh	ip, [r0, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L54
	ldr	r2, [sp, #28]
	ldr	r5, [sp, #8]
	cmp	r2, #6
	rsb	r3, ip, #2048
	add	r1, r5, #1632
	movgt	r2, #3
	ldr	r5, [sp, #36]
	movle	r2, #0
	add	r3, ip, r3, asr #5
	str	r2, [sp, #28]
	ldr	ip, [sp, #32]
	ldr	r2, [sp, #48]
	str	r5, [sp, #40]
	add	r1, r1, #4
	mov	r5, r4
	str	ip, [sp, #36]
	str	r2, [sp, #32]
	strh	r3, [r0, #0]	@ movhi
	b	f.L59
f.L54:
	rsb	r2, r4, r2
	sub	r3, ip, ip, lsr #5
	cmp	r2, r8
	strh	r3, [r0, #0]	@ movhi
	rsb	r6, r4, r6
	bhi	f.L60
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
f.L60:
	add	r5, r7, #408
	ldrh	ip, [r5, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L63
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	cmp	r4, r8
	strh	r3, [r5, #0]	@ movhi
	bhi	f.L67
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r4, r4, asl #8
	orr	r6, r3, r6, asl #8
f.L67:
	ldr	r5, [sp, #8]
	add	r3, r5, r1, asl #1
	add	r3, r3, r9, asl #1
	add	r1, r3, #480
	ldrh	ip, [r1, #0]
	mov	r3, r4, lsr #11
	mul	r2, ip, r3
	cmp	r6, r2
	rsbcs	r6, r2, r6
	subcs	r3, ip, ip, lsr #5
	rsbcs	r5, r2, r4
	bcs	f.L169
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	ldr	ip, [sp, #12]
	cmp	ip, #0
	strh	r3, [r1, #0]	@ movhi
	beq	f.L18
	ldr	r0, [sp, #48]
	ldr	r1, [sp, #84]
	rsb	r3, r0, ip
	ldrb	r7, [r1, r3]	@ zero_extendqisi2
	ldr	r3, [sp, #28]
	cmp	r3, #6
	movgt	r3, #11
	movle	r3, #9
	str	r3, [sp, #28]
	mov	r0, r2
	strb	r7, [r1, ip]
	add	ip, ip, #1
	str	ip, [sp, #12]
	b	f.L172
f.L63:
	rsb	r2, r4, r2
	sub	r3, ip, ip, lsr #5
	cmp	r2, r8
	strh	r3, [r5, #0]	@ movhi
	rsb	r6, r4, r6
	bhi	f.L76
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
f.L76:
	add	r1, r7, #432
	ldrh	ip, [r1, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L79
	ldr	r5, [sp, #48]
	rsb	r3, ip, #2048
	ldr	r2, [sp, #32]
	add	r3, ip, r3, asr #5
	str	r5, [sp, #32]
	mov	r5, r4
f.L170:
	str	r2, [sp, #48]
f.L169:
	strh	r3, [r1, #0]	@ movhi
	b	f.L75
f.L79:
	rsb	r2, r4, r2
	sub	r3, ip, ip, lsr #5
	cmp	r2, r8
	strh	r3, [r1, #0]	@ movhi
	rsb	r6, r4, r6
	bhi	f.L81
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
f.L81:
	add	r1, r7, #456
	ldrh	ip, [r1, #0]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L84
	rsb	r3, ip, #2048
	ldr	r2, [sp, #36]
	add	r3, ip, r3, asr #5
	ldr	r0, [sp, #48]
	ldr	ip, [sp, #32]
	mov	r5, r4
	str	ip, [sp, #36]
	str	r0, [sp, #32]
	b	f.L170
f.L84:
	sub	r3, ip, ip, lsr #5
	strh	r3, [r1, #0]	@ movhi
	ldr	ip, [sp, #48]
	add	r1, sp, #36
	ldmia	r1, {r1, r3}	@ phole ldm
	rsb	r5, r4, r2
	ldr	r2, [sp, #32]
	str	r1, [sp, #40]
	str	r2, [sp, #36]
	str	ip, [sp, #32]
	str	r3, [sp, #48]
	rsb	r6, r4, r6
f.L75:
	ldr	r2, [sp, #28]
	ldr	r0, [sp, #8]
	cmp	r2, #6
	movgt	r2, #11
	movle	r2, #8
	add	r1, r0, #2656
	str	r2, [sp, #28]
	add	r1, r1, #8
f.L59:
	mvn	r7, #-16777216
	cmp	r5, r7
	bhi	f.L89
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r5, r5, asl #8
	orr	r6, r3, r6, asl #8
f.L89:
	ldrh	ip, [r1, #0]
	mov	r3, r5, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L92
	add	r2, r1, r9, asl #4
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	add	r5, r2, #4
	mov	r0, r4
	mov	r9, #3
	mov	r8, #0
	strh	r3, [r1, #0]	@ movhi
	b	f.L94
f.L92:
	rsb	r2, r4, r5
	sub	r3, ip, ip, lsr #5
	cmp	r2, r7
	rsb	r6, r4, r6
	strh	r3, [r1, #0]	@ movhi
	bhi	f.L95
	cmp	lr, fp
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	mov	r2, r2, asl #8
	orr	r6, r3, r6, asl #8
f.L95:
	ldrh	ip, [r1, #2]
	mov	r3, r2, lsr #11
	mul	r4, ip, r3
	cmp	r6, r4
	bcs	f.L98
	add	r2, r1, r9, asl #4
	rsb	r3, ip, #2048
	add	r3, ip, r3, asr #5
	add	r5, r2, #260
	mov	r0, r4
	mov	r9, #3
	mov	r8, #8
	strh	r3, [r1, #2]	@ movhi
	b	f.L94
f.L98:
	sub	r3, ip, ip, lsr #5
	strh	r3, [r1, #2]	@ movhi
	rsb	r6, r4, r6
	rsb	r0, r4, r2
	add	r5, r1, #516
	mov	r9, #8
	mov	r8, #16
f.L94:
	mov	r7, r9
	mov	r1, #1
f.L100:
	add	r3, r1, #1
	cmp	r0, #16777216
	mov	r4, r1, asl #1
	add	r1, r1, r3
	bcs	f.L101
	cmp	lr, fp
	mov	r0, r0, asl #8
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
f.L101:
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
	bne	f.L100
	mov	r2, #1
	sub	r3, r1, r2, asl r9
	ldr	r5, [sp, #28]
	add	r3, r3, r8
	cmp	r5, #3
	str	r3, [sp, #44]
	bgt	f.L108
	ldr	ip, [sp, #8]
	cmp	r3, #3
	movge	r3, #3
	add	r3, ip, r3, asl #7
	add	r5, r3, #864
	mov	r7, r2
	mov	r8, #6
f.L110:
	add	r3, r7, #1
	cmp	r0, #16777216
	mov	r4, r7, asl #1
	add	r7, r7, r3
	bcs	f.L111
	cmp	lr, fp
	mov	r0, r0, asl #8
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
f.L111:
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
	bne	f.L110
	sub	r2, r7, #64
	cmp	r2, #3
	movle	r5, r2
	ble	f.L137
	mov	r4, r2, asr #1
	cmp	r2, #13
	and	r3, r2, #1
	sub	r7, r4, #1
	orr	ip, r3, #2
	subgt	r4, r4, #5
	movgt	r2, r8
	bgt	f.L123
	mov	r5, ip, asl r7
	ldr	r1, [sp, #8]
	add	r3, r1, r5, asl #1
	sub	r3, r3, r2, asl #1
	add	r4, r3, #1360
	add	r4, r4, #14
	b	f.L122
f.L123:
	cmp	r0, #16777216
	add	r2, r2, #1
	bcs	f.L124
	cmp	lr, fp
	mov	r0, r0, asl #8
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
f.L124:
	mov	r0, r0, lsr #1
	cmp	r6, r0
	mov	ip, ip, asl #1
	rsbcs	r6, r0, r6
	orrcs	ip, ip, #1
	cmp	r2, r4
	bne	f.L123
	ldr	r2, [sp, #8]
	add	r4, r2, #1600
	add	r4, r4, #4
	mov	r5, ip, asl #4
	mov	r7, #4
f.L122:
	mov	r9, #1
	mov	r8, r9
f.L130:
	cmp	r0, #16777216
	mov	r1, r8, asl #1
	add	ip, r8, #1
	bcs	f.L131
	cmp	lr, fp
	mov	r0, r0, asl #8
	beq	f.L18
	ldrb	r3, [lr], #1	@ zero_extendqisi2
	orr	r6, r3, r6, asl #8
f.L131:
	ldrh	r3, [r4, r1]
	mov	r2, r0, lsr #11
	add	r8, r8, ip
	mul	ip, r3, r2
	rsb	r2, r3, #2048
	cmp	r6, ip
	add	r2, r3, r2, asr #5
	sub	r3, r3, r3, lsr #5
	orrcs	r5, r5, r9
	movcc	r8, r1
	movcc	r0, ip
	strcch	r2, [r4, r1]	@ movhi
	strcsh	r3, [r4, r1]	@ movhi
	rsbcs	r6, ip, r6
	rsbcs	r0, ip, r0
	subs	r7, r7, #1
	mov	r9, r9, asl #1
	bne	f.L130
f.L137:
	adds	r5, r5, #1
	str	r5, [sp, #48]
	beq	f.L139
	ldr	r3, [sp, #28]
	add	r3, r3, #7
	str	r3, [sp, #28]
f.L108:
	ldr	r5, [sp, #48]
	ldr	ip, [sp, #12]
	cmp	r5, ip
	bhi	f.L18
	rsb	r3, r5, ip
	ldr	r1, [sp, #44]
	ldr	r5, [sp, #84]
	add	r2, r1, #2
	add	r4, r5, r3
	add	ip, r5, ip
f.L142:
	ldr	r1, [sp, #12]
	add	r1, r1, #1
	subs	r2, r2, #1
	str	r1, [sp, #12]
	ldr	r5, [sp, #88]
	moveq	r3, #0
	movne	r3, #1
	cmp	r1, r5
	movcs	r3, #0
	andcc	r3, r3, #1
	ldrb	r7, [r4], #1	@ zero_extendqisi2
	cmp	r3, #0
	strb	r7, [ip], #1
	bne	f.L142
f.L172:
	ldr	ip, [sp, #12]
	ldr	r1, [sp, #88]
	cmp	ip, r1
	bcc	f.L22
f.L139:
	cmp	r0, #16777216
	bcs	f.L143
	cmp	lr, fp
	beq	f.L18
	add	lr, lr, #1
f.L143:
	ldr	r2, [sp, #4]
	ldr	r5, [sp, #0]
	ldr	r1, [sp, #12]
	ldr	ip, [sp, #92]
	rsb	r3, r2, lr
	mov	r0, #0
	str	r3, [r5, #0]
	str	r1, [ip, #0]
	b	f.L146
f.L18:
	mov	r0, #1
f.L146:
	add	sp, sp, #52
	ldmfd	sp!, {r4, r5, r6, r7, r8, r9, fp, pc}

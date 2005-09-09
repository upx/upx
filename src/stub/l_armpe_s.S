        .text
        .align  0
        .global _mainCRTStartup

_mainCRTStartup:
        stmfd   sp!, {r0 - r3, lr}
	mov	r0, #L2 - L0 - 8
L0:
        add     r0, pc, r0
	bl	_upx_main
        ldmfd   sp!, {r0 - r3, lr}
	ldr	pc, L21
        .ascii  "XxxX"
L2:
        .ascii  "SRC0DST0BIMPONAMGETPLOAD"
L21:
	.ascii  "ENTRSRCLDSTL"

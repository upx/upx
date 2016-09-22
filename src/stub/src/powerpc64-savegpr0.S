_savegpr0_16: .globl _savegpr0_16
        std r16,-8*(32-16)(r1)
_savegpr0_17: .globl _savegpr0_17
        std r17,-8*(32-17)(r1)
_savegpr0_18: .globl _savegpr0_18
        std r18,-8*(32-18)(r1)
_savegpr0_19: .globl _savegpr0_19
        std r19,-8*(32-19)(r1)
_savegpr0_20: .globl _savegpr0_20
        std r20,-8*(32-20)(r1)
_savegpr0_21: .globl _savegpr0_21
        std r21,-8*(32-21)(r1)
_savegpr0_22: .globl _savegpr0_22
        std r22,-8*(32-22)(r1)
_savegpr0_23: .globl _savegpr0_23
        std r23,-8*(32-23)(r1)
_savegpr0_24: .globl _savegpr0_24
        std r24,-8*(32-24)(r1)
_savegpr0_25: .globl _savegpr0_25
        std r25,-8*(32-25)(r1)
_savegpr0_26: .globl _savegpr0_26
        std r26,-8*(32-26)(r1)
_savegpr0_27: .globl _savegpr0_27
        std r27,-8*(32-27)(r1)
_savegpr0_28: .globl _savegpr0_28
        std r28,-8*(32-28)(r1)
_savegpr0_29: .globl _savegpr0_29
        std r29,-8*(32-29)(r1)
        std r30,-8*(32-30)(r1)
        std r31,-8*(32-31)(r1)
        std r0, 2*8(r1)  # save LR; SP/ SP, CR, LR
        blr  # enter the fn after the prolog

_restgpr0_16: .globl _restgpr0_16
        ldd r16,-8*(32-16)(r1)
_restgpr0_17: .globl _restgpr0_17
        ldd r17,-8*(32-17)(r1)
_restgpr0_18: .globl _restgpr0_18
        ldd r18,-8*(32-18)(r1)
_restgpr0_19: .globl _restgpr0_19
        ldd r19,-8*(32-19)(r1)
_restgpr0_20: .globl _restgpr0_20
        ldd r20,-8*(32-20)(r1)
_restgpr0_21: .globl _restgpr0_21
        ldd r21,-8*(32-21)(r1)
_restgpr0_22: .globl _restgpr0_22
        ldd r22,-8*(32-22)(r1)
_restgpr0_23: .globl _restgpr0_23
        ldd r23,-8*(32-23)(r1)
_restgpr0_24: .globl _restgpr0_24
        ldd r24,-8*(32-24)(r1)
_restgpr0_25: .globl _restgpr0_25
        ldd r25,-8*(32-25)(r1)
_restgpr0_26: .globl _restgpr0_26
        ldd r26,-8*(32-26)(r1)
_restgpr0_27: .globl _restgpr0_27
        ldd r27,-8*(32-27)(r1)
_restgpr0_28: .globl _restgpr0_28
        ldd r28,-8*(32-28)(r1)
_restgpr0_29: .globl _restgpr0_29
        ldd r0, 2*8(r1)  # LR; SP/ SP, CR, LR
        ldd r29,-8*(32-29)(r1)
        mtlr r0  # retaddr
        ldd r30,-8*(32-30)(r1)
        ldd r31,-8*(32-31)(r1)
         blr  # return to caller

/* vim:set ts=8 sw=8 et: */

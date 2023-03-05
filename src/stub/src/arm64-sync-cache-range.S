
CTR_IDC_SHIFT= 28
CTR_DIC_SHIFT= 29

sync_cache_range:  // (void *lo, void *const hi)
    mrs x3,ctr_el0
    tbnz w3,#CTR_IDC_SHIFT,dc_not_dirty
    ubfx x2,x3,#16,#4  // -2+ log2(dline_size)
    mov x4,#-4; lsl x4,x4,x2  // sz_dline mask
    and x2,x4,x0  // round down to dc line
    cmp x2,x1; b.hs dc_done
dc_loop:
    dc cvau,x2  // sync dline
    sub x2,x2,x4  // next dline
    cmp x2,x1; b.lo dc_loop
dc_done:
dc_not_dirty:
    dsb ish  // why here if dc not dirty?

    tbnz w3,#CTR_DIC_SHIFT,ic_not_dirty
    and x3,x3,#0xf  // -2+ log2(iline_size)
    mov x4,#-4; lsl x4,x4,x3  // sz_iline mask
    and x0,x4,x0  // round down to ic line
    cmp x0,x1; b.hs ic_done
ic_loop:
    ic ivau,x0  // sync iline
    sub x0,x0,x4  // next iline
    cmp x0,x1; b.lo ic_loop
ic_done:
    dsb ish
ic_not_dirty:
    isb


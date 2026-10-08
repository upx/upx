
// /*************************************************************************
//   inline f_unfilter (dst, len, cto8, fid)  FALL_THROUGH return
// **************************************************************************/

f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req x0
        len  .req w1
        cto  .req w2  // unused
        fid  .req w3

        t1   .req w4
        t2   .req w5

        and t2,fid,#0xfe  // accept 0x52 and 0x53
        cmp t2,#0x52
        bne unf_done

        lsr len,len,#2  // word count
        cbz ptr,unf_done
        cbz len,unf_done

unf_top:
        sub len,len,#1
        lsl t2,len,#2
        ldr t1,[ptr,t2,uxtw]  // the 32-bit instruction

// ubfx dst,src,#pos,#wid  // Unsigned BitField Extract

        ubfx t2,t1,#26,#5; cmp t2,#5; bne not_w26  // B, BL
is_w26:
#ifdef FILTER_ADDVALUE
        add t2,len,cto
        sub t2,t1,t2
#else
        sub t2,t1,len  // pos is #0 ==> ubfx for the field is not needed
#endif
        bfi t1,t2,#0,#26  // re-combine
        b unf_str
not_w26:
        tbz fid,#0,unf_tst  // 0x52 leaves conditional branches unchanged
        ubfx t2,t1,#24,#8; cmp t2,#0x54; beq is_w19  // B.cond
        ubfx t2,t1,#25,#6; cmp t2,#0x1a; beq is_w19  // CB{z,nz}
                           cmp t2,#0x1b; bne unf_tst // TB{z,nz}
is_w14:
        ubfx t2,t1,#5,#14  // the field
        sub t2,t2,len
        bfi t1,t2,#5,#14  // re-combine
        b unf_str
is_w19:
        ubfx t2,t1,#5,#19  // the field
        sub t2,t2,len
        bfi t1,t2,#5,#19  // re-combine
unf_str:
        lsl t2,len,#2
        str t1,[ptr,t2,uxtw]
unf_tst:
        cmp len,#0
        bne unf_top
unf_done:

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

        .unreq t1
        .unreq t2

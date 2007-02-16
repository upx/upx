        push    bp
        mov     bp,sp
        sub     sp,0x92
        push    si
        push    di
        mov     di,word ptr [bp+4]
        mov     ax,di
        add     ax,0x4
        mov     word ptr [bp-112],ax
        mov     word ptr [bp-108],0x0
        mov     word ptr [bp-106],0x0
        mov     byte ptr [bp-104],0x0
        mov     ax,0x1
        mov     cl,byte ptr [di+2]
        xor     ch,ch
        shl     ax,cl
        dec     ax
        cwd
        mov     word ptr [bp-98],dx
        mov     word ptr [bp-100],ax
        mov     ax,0x1
        mov     cl,byte ptr [di+1]
        xor     ch,ch
        shl     ax,cl
        dec     ax
        cwd
        mov     word ptr [bp-94],dx
        mov     word ptr [bp-96],ax
        mov     al,byte ptr [di]
        xor     ah,ah
        mov     word ptr [bp-92],ax
        mov     word ptr [bp-90],0x0
        mov     word ptr [bp-88],0x1
        mov     word ptr [bp-86],0x0
        mov     word ptr [bp-84],0x1
        mov     word ptr [bp-82],0x0
        mov     word ptr [bp-80],0x1
        mov     word ptr [bp-78],0x0
        mov     word ptr [bp-76],0x1
        mov     word ptr [bp-74],0x0
        mov     bx,word ptr [bp+14]
        mov     word ptr [bx],0x0
        mov     word ptr [bx+2],0x0
        mov     bx,word ptr [bp+24]
        mov     word ptr [bx],0x0
        mov     word ptr [bx+2],0x0
        mov     word ptr [bp-52],0x0
        mov     word ptr [bp-50],0x0
        mov     bx,0x300
        xor     dx,dx
        mov     cl,byte ptr [di+1]
        xor     ch,ch
        add     cx,ax
        jcxz    .Ls001
.Ls000:
        shl     bx,1
        rcl     dx,1
        loop    .Ls000
.Ls001:
        add     bx,0x736
        adc     dx,cx
        mov     ax,dx
        or      ax,bx
        mov     word ptr [bp-46],dx
        mov     word ptr [bp-48],bx
        je      .Ls004
        mov     si,word ptr [bp-52]
        shl     si,1
        add     si,word ptr [bp-112]
.Ls002:
        mov     word ptr [si],0x400
        add     si,0x2
        add     word ptr [bp-52],0x1
        adc     word ptr [bp-50],0x0
        mov     dx,word ptr [bp-50]
        mov     ax,word ptr [bp-52]
        cmp     dx,word ptr [bp-46]
        jne     .Ls003
        cmp     ax,word ptr [bp-48]
.Ls003:
        jb      .Ls002
.Ls004:
        mov     dx,word ptr [bp+8]
        mov     ax,word ptr [bp+6]
        mov     word ptr [bp-66],dx
        mov     word ptr [bp-68],ax
        mov     cx,dx
        mov     bx,ax
        mov     dx,word ptr [bp+12]
        mov     ax,word ptr [bp+10]
        add     ax,bx
        mov     bx,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,bx
        mov     word ptr [bp-62],dx
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-56],0x0
        mov     word ptr [bp-54],0x0
        mov     word ptr [bp-60],0xffff
        mov     word ptr [bp-58],0xffff
        xor     di,di
.Ls005:
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls006
        cmp     ax,word ptr [bp-64]
.Ls006:
        jne     .Ls007
        jmp     .Ls162
.Ls007:
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
        inc     di
        cmp     di,0x5
        jb      .Ls005
        mov     ax,word ptr [bp+22]
        or      ax,word ptr [bp+20]
        jne     .Ls008
        jmp     .Ls160
.Ls008:
        mov     di,word ptr [bp-108]
        and     di,word ptr [bp-100]
        mov     word ptr [bp-40],di
        shl     di,1
        mov     ax,word ptr [bp-90]
        mov     cx,0x5
        shl     ax,cl
        add     ax,word ptr [bp-112]
        add     di,ax
        cmp     word ptr [bp-58],0x100
        jne     .Ls009
        cmp     word ptr [bp-60],0x0
.Ls009:
        jae     .Ls013
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls010
        cmp     ax,word ptr [bp-64]
.Ls010:
        jne     .Ls011
        jmp     .Ls162
.Ls011:
        mov     cl,0x8
.Ls012:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls012
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls013:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls014
        cmp     ax,word ptr [bp-56]
.Ls014:
        ja      .Ls015
        jmp     .Ls043
.Ls015:
        mov     word ptr [bp-38],0x1
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        sub     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [di],ax
        mov     ax,word ptr [bp-108]
        and     ax,word ptr [bp-96]
        mov     cx,word ptr [bp-92]
        shl     ax,cl
        mov     dl,byte ptr [bp-104]
        xor     dh,dh
        mov     cx,0x8
        sub     cx,word ptr [bp-92]
        sar     dx,cl
        add     ax,dx
        mov     cx,0x300
        mul     cx
        mov     di,ax
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0xe6c
        cmp     word ptr [bp-90],0x7
        jge     .Ls016
        jmp     .Ls026
.Ls016:
        mov     dx,word ptr [bp-106]
        mov     bx,word ptr [bp-108]
        mov     cx,word ptr [bp+18]
        mov     ax,word ptr [bp+16]
        add     bx,ax
        mov     ax,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,ax
        sub     bx,word ptr [bp-88]
        sbb     dx,word ptr [bp-86]
        movw    es,dx
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        mov     word ptr [bp-36],ax
.Ls017:
        mov     dx,word ptr [bp-36]
        shl     dx,1
        mov     word ptr [bp-36],dx
        mov     si,word ptr [bp-38]
        shl     si,1
        and     dx,0x100
        mov     word ptr [bp-34],dx
        shl     dx,1
        add     dx,di
        add     si,dx
        add     si,0x200
        cmp     word ptr [bp-58],0x100
        jne     .Ls018
        cmp     word ptr [bp-60],0x0
.Ls018:
        jae     .Ls022
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls019
        cmp     ax,word ptr [bp-64]
.Ls019:
        jne     .Ls020
        jmp     .Ls162
.Ls020:
        mov     word ptr [bp-32],si
        mov     cx,0x8
.Ls021:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls021
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
        mov     si,word ptr [bp-32]
.Ls022:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-116],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls023
        cmp     ax,word ptr [bp-56]
.Ls023:
        jbe     .Ls024
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     dx,0x800
        sub     dx,word ptr [si]
        mov     cx,0x5
        shr     dx,cl
        add     word ptr [si],dx
        shl     word ptr [bp-38],1
        cmp     word ptr [bp-34],0x0
        jne     .Ls026
        jmp     .Ls025
.Ls024:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     dx,word ptr [bp-116]
        mov     cx,0x5
        shr     dx,cl
        sub     word ptr [si],dx
        mov     cx,word ptr [bp-38]
        shl     cx,1
        inc     cx
        mov     word ptr [bp-38],cx
        cmp     word ptr [bp-34],0x0
        je      .Ls026
.Ls025:
        cmp     word ptr [bp-38],0x100
        jge     .Ls026
        jmp     .Ls017
.Ls026:
        cmp     word ptr [bp-38],0x100
        jl      .Ls027
        jmp     .Ls037
.Ls027:
        mov     si,word ptr [bp-38]
        shl     si,1
        add     si,di
        cmp     word ptr [bp-58],0x100
        jne     .Ls028
        cmp     word ptr [bp-60],0x0
.Ls028:
        jae     .Ls032
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls029
        cmp     ax,word ptr [bp-64]
.Ls029:
        jne     .Ls030
        jmp     .Ls162
.Ls030:
        mov     word ptr [bp-30],si
        mov     cx,0x8
.Ls031:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls031
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
        mov     si,word ptr [bp-30]
.Ls032:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-114],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls033
        cmp     ax,word ptr [bp-56]
.Ls033:
        jbe     .Ls036
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     dx,0x800
        sub     dx,word ptr [si]
        mov     cx,0x5
        shr     dx,cl
        add     word ptr [si],dx
        shl     word ptr [bp-38],1
.Ls034:
        cmp     word ptr [bp-38],0x100
        jge     .Ls035
        jmp     .Ls027
.Ls035:
        jmp     .Ls037
.Ls036:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     dx,word ptr [bp-114]
        mov     cx,0x5
        shr     dx,cl
        sub     word ptr [si],dx
        mov     cx,word ptr [bp-38]
        shl     cx,1
        inc     cx
        mov     word ptr [bp-38],cx
        jmp     .Ls034
.Ls037:
        mov     al,byte ptr [bp-38]
        mov     byte ptr [bp-104],al
        mov     bx,word ptr [bp-108]
        mov     dx,word ptr [bp-106]
        add     word ptr [bp-108],0x1
        adc     word ptr [bp-106],0x0
        mov     cx,word ptr [bp+18]
        mov     si,word ptr [bp+16]
        add     bx,si
        mov     si,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,si
        movw    es,dx
        mov     byte ptr es:[bx],al
        cmp     word ptr [bp-90],0x4
        jge     .Ls041
        mov     word ptr [bp-90],0x0
.Ls038:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Ls039
        cmp     ax,word ptr [bp+20]
.Ls039:
        jae     .Ls040
        jmp     .Ls008
.Ls040:
        jmp     .Ls160
.Ls041:
        cmp     word ptr [bp-90],0xa
        jge     .Ls042
        add     word ptr [bp-90],0xfffffffd
        jmp     .Ls038
.Ls042:
        add     word ptr [bp-90],0xfffffffa
        jmp     .Ls038
.Ls043:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        mov     di,word ptr [bp-90]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x180
        cmp     word ptr [bp-58],0x100
        jne     .Ls044
        cmp     word ptr [bp-60],0x0
.Ls044:
        jae     .Ls048
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls045
        cmp     ax,word ptr [bp-64]
.Ls045:
        jne     .Ls046
        jmp     .Ls162
.Ls046:
        mov     cl,0x8
.Ls047:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls047
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls048:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls049
        cmp     ax,word ptr [bp-56]
.Ls049:
        jbe     .Ls051
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        sub     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [di],ax
        mov     dx,word ptr [bp-78]
        mov     ax,word ptr [bp-80]
        mov     word ptr [bp-74],dx
        mov     word ptr [bp-76],ax
        mov     dx,word ptr [bp-82]
        mov     ax,word ptr [bp-84]
        mov     word ptr [bp-78],dx
        mov     word ptr [bp-80],ax
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        mov     word ptr [bp-82],dx
        mov     word ptr [bp-84],ax
        cmp     word ptr [bp-90],0x7
        mov     ax,0x0
        jl      .Ls050
        mov     al,0x3
.Ls050:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0x664
        jmp     .Ls088
.Ls051:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        mov     di,word ptr [bp-90]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x198
        cmp     word ptr [bp-58],0x100
        jne     .Ls052
        cmp     word ptr [bp-60],0x0
.Ls052:
        jae     .Ls056
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls053
        cmp     ax,word ptr [bp-64]
.Ls053:
        jne     .Ls054
        jmp     .Ls162
.Ls054:
        mov     cl,0x8
.Ls055:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls055
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls056:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls057
        cmp     ax,word ptr [bp-56]
.Ls057:
        ja      .Ls058
        jmp     .Ls069
.Ls058:
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     bx,0x800
        sub     bx,word ptr [di]
        mov     cx,0x5
        shr     bx,cl
        add     word ptr [di],bx
        mov     di,word ptr [bp-40]
        shl     di,1
        mov     bx,word ptr [bp-90]
        shl     bx,cl
        add     bx,word ptr [bp-112]
        add     di,bx
        add     di,0x1e0
        cmp     dx,0x100
        jne     .Ls059
        test    ax,ax
.Ls059:
        jae     .Ls063
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls060
        cmp     ax,word ptr [bp-64]
.Ls060:
        jne     .Ls061
        jmp     .Ls162
.Ls061:
        mov     cl,0x8
.Ls062:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls062
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls063:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls064
        cmp     ax,word ptr [bp-56]
.Ls064:
        ja      .Ls065
        jmp     .Ls068
.Ls065:
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        sub     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [di],ax
        mov     ax,word ptr [bp-106]
        or      ax,word ptr [bp-108]
        jne     .Ls066
        jmp     .Ls162
.Ls066:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x9
        jl      .Ls067
        mov     al,0xb
.Ls067:
        mov     word ptr [bp-90],ax
        mov     dx,word ptr [bp-106]
        mov     bx,word ptr [bp-108]
        mov     cx,word ptr [bp+18]
        mov     ax,word ptr [bp+16]
        add     bx,ax
        mov     ax,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,ax
        sub     bx,word ptr [bp-88]
        sbb     dx,word ptr [bp-86]
        movw    es,dx
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-104],al
        mov     bx,word ptr [bp-108]
        mov     dx,word ptr [bp-106]
        add     word ptr [bp-108],0x1
        adc     word ptr [bp-106],0x0
        mov     cx,word ptr [bp+18]
        mov     si,word ptr [bp+16]
        add     bx,si
        mov     si,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,si
        movw    es,dx
        mov     byte ptr es:[bx],al
        jmp     .Ls038
.Ls068:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        jmp     .Ls086
.Ls069:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        mov     di,word ptr [bp-90]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x1b0
        cmp     word ptr [bp-58],0x100
        jne     .Ls070
        cmp     word ptr [bp-60],0x0
.Ls070:
        jae     .Ls074
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls071
        cmp     ax,word ptr [bp-64]
.Ls071:
        jne     .Ls072
        jmp     .Ls162
.Ls072:
        mov     cl,0x8
.Ls073:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls073
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls074:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls075
        cmp     ax,word ptr [bp-56]
.Ls075:
        jbe     .Ls076
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        sub     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [di],ax
        mov     dx,word ptr [bp-82]
        mov     ax,word ptr [bp-84]
        mov     word ptr [bp-26],dx
        mov     word ptr [bp-28],ax
        jmp     .Ls085
.Ls076:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        mov     di,word ptr [bp-90]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x1c8
        cmp     word ptr [bp-58],0x100
        jne     .Ls077
        cmp     word ptr [bp-60],0x0
.Ls077:
        jae     .Ls081
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls078
        cmp     ax,word ptr [bp-64]
.Ls078:
        jne     .Ls079
        jmp     .Ls162
.Ls079:
        mov     cl,0x8
.Ls080:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls080
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls081:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls082
        cmp     ax,word ptr [bp-56]
.Ls082:
        jbe     .Ls083
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        sub     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [di],ax
        mov     dx,word ptr [bp-78]
        mov     ax,word ptr [bp-80]
        mov     word ptr [bp-26],dx
        mov     word ptr [bp-28],ax
        jmp     .Ls084
.Ls083:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        mov     dx,word ptr [bp-74]
        mov     ax,word ptr [bp-76]
        mov     word ptr [bp-26],dx
        mov     word ptr [bp-28],ax
        mov     dx,word ptr [bp-78]
        mov     ax,word ptr [bp-80]
        mov     word ptr [bp-74],dx
        mov     word ptr [bp-76],ax
.Ls084:
        mov     dx,word ptr [bp-82]
        mov     ax,word ptr [bp-84]
        mov     word ptr [bp-78],dx
        mov     word ptr [bp-80],ax
.Ls085:
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        mov     word ptr [bp-82],dx
        mov     word ptr [bp-84],ax
        mov     dx,word ptr [bp-26]
        mov     ax,word ptr [bp-28]
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
.Ls086:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x8
        jl      .Ls087
        mov     al,0xb
.Ls087:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0xa68
.Ls088:
        cmp     word ptr [bp-58],0x100
        jne     .Ls089
        cmp     word ptr [bp-60],0x0
.Ls089:
        jae     .Ls093
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls090
        cmp     ax,word ptr [bp-64]
.Ls090:
        jne     .Ls091
        jmp     .Ls162
.Ls091:
        mov     cl,0x8
.Ls092:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls092
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls093:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls094
        cmp     ax,word ptr [bp-56]
.Ls094:
        jbe     .Ls095
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        sub     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [di],ax
        mov     ax,word ptr [bp-40]
        shl     ax,1
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     ax,di
        add     ax,0x4
        mov     word ptr [bp-20],ax
        mov     word ptr [bp-22],0x0
        mov     word ptr [bp-24],0x3
        jmp     .Ls103
.Ls095:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        mov     ax,di
        add     ax,0x2
        mov     word ptr [bp-20],ax
        cmp     word ptr [bp-58],0x100
        jne     .Ls096
        cmp     word ptr [bp-60],0x0
.Ls096:
        jae     .Ls100
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls097
        cmp     ax,word ptr [bp-64]
.Ls097:
        jne     .Ls098
        jmp     .Ls162
.Ls098:
        mov     cl,0x8
.Ls099:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls099
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls100:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [bp-20]
        mov     bx,word ptr [bx]
        mov     word ptr [bp-124],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls101
        cmp     ax,word ptr [bp-56]
.Ls101:
        jbe     .Ls102
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     ax,0x800
        mov     bx,word ptr [bp-20]
        sub     ax,word ptr [bx]
        mov     cx,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-40]
        shl     ax,1
        shl     ax,1
        shl     ax,1
        shl     ax,1
        add     ax,di
        add     ax,0x104
        mov     word ptr [bp-20],ax
        mov     word ptr [bp-22],0x8
        mov     word ptr [bp-24],0x3
        jmp     .Ls103
.Ls102:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [bp-124]
        mov     cx,0x5
        shr     ax,cl
        mov     bx,word ptr [bp-20]
        sub     word ptr [bx],ax
        mov     ax,di
        add     ax,0x204
        mov     word ptr [bp-20],ax
        mov     word ptr [bp-22],0x10
        mov     word ptr [bp-24],0x8
.Ls103:
        mov     ax,word ptr [bp-24]
        mov     word ptr [bp-18],ax
        mov     di,0x1
.Ls104:
        mov     si,di
        shl     si,1
        add     si,word ptr [bp-20]
        cmp     word ptr [bp-58],0x100
        jne     .Ls105
        cmp     word ptr [bp-60],0x0
.Ls105:
        jae     .Ls109
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls106
        cmp     ax,word ptr [bp-64]
.Ls106:
        jne     .Ls107
        jmp     .Ls162
.Ls107:
        mov     word ptr [bp-16],si
        mov     cx,0x8
.Ls108:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls108
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
        mov     si,word ptr [bp-16]
.Ls109:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-122],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls110
        cmp     ax,word ptr [bp-56]
.Ls110:
        jbe     .Ls111
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     dx,0x800
        sub     dx,word ptr [si]
        mov     cx,0x5
        shr     dx,cl
        add     word ptr [si],dx
        shl     di,1
        jmp     .Ls112
.Ls111:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     dx,word ptr [bp-122]
        mov     cx,0x5
        shr     dx,cl
        sub     word ptr [si],dx
        shl     di,1
        inc     di
.Ls112:
        dec     word ptr [bp-18]
        je      .Ls113
        jmp     .Ls104
.Ls113:
        mov     word ptr [bp-72],di
        mov     ax,0x1
        mov     cx,word ptr [bp-24]
        shl     ax,cl
        sub     word ptr [bp-72],ax
        mov     ax,word ptr [bp-22]
        add     word ptr [bp-72],ax
        cmp     word ptr [bp-90],0x4
        jl      .Ls114
        jmp     .Ls154
.Ls114:
        add     word ptr [bp-90],0x7
        cmp     word ptr [bp-72],0x4
        jge     .Ls115
        mov     di,word ptr [bp-72]
        jmp     .Ls116
.Ls115:
        mov     di,0x3
.Ls116:
        mov     cx,0x7
        shl     di,cl
        add     di,word ptr [bp-112]
        add     di,0x360
        mov     word ptr [bp-12],0x6
        mov     si,0x1
.Ls117:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-10],cx
        cmp     word ptr [bp-58],0x100
        jne     .Ls118
        cmp     word ptr [bp-60],0x0
.Ls118:
        jae     .Ls122
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls119
        cmp     ax,word ptr [bp-64]
.Ls119:
        jne     .Ls120
        jmp     .Ls162
.Ls120:
        mov     word ptr [bp-14],si
        mov     cx,0x8
.Ls121:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls121
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
        mov     si,word ptr [bp-14]
.Ls122:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [bp-10]
        mov     bx,word ptr [bx]
        mov     word ptr [bp-120],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls123
        cmp     ax,word ptr [bp-56]
.Ls123:
        jbe     .Ls124
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     dx,0x800
        mov     bx,word ptr [bp-10]
        sub     dx,word ptr [bx]
        mov     cx,0x5
        shr     dx,cl
        add     word ptr [bx],dx
        shl     si,1
        jmp     .Ls125
.Ls124:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     dx,word ptr [bp-120]
        mov     cx,0x5
        shr     dx,cl
        mov     bx,word ptr [bp-10]
        sub     word ptr [bx],dx
        shl     si,1
        inc     si
.Ls125:
        dec     word ptr [bp-12]
        je      .Ls126
        jmp     .Ls117
.Ls126:
        mov     di,word ptr [bp-72]
        add     si,0xffffffc0
        cmp     si,0x4
        jge     .Ls127
        jmp     .Ls152
.Ls127:
        mov     cx,si
        sar     cx,1
        dec     cx
        mov     word ptr [bp-8],cx
        mov     ax,si
        and     ax,0x1
        or      al,0x2
        xor     cx,cx
        mov     word ptr [bp-86],cx
        mov     word ptr [bp-88],ax
        cmp     si,0xe
        jge     .Ls130
        mov     word ptr [bp-72],di
        mov     cx,word ptr [bp-8]
        jcxz    .Ls129
.Ls128:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Ls128
.Ls129:
        mov     di,word ptr [bp-88]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x55e
        mov     cx,si
        shl     cx,1
        sub     di,cx
        jmp     .Ls141
.Ls130:
        add     word ptr [bp-8],0xfffffffc
.Ls131:
        cmp     word ptr [bp-58],0x100
        jne     .Ls132
        cmp     word ptr [bp-60],0x0
.Ls132:
        jae     .Ls136
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls133
        cmp     ax,word ptr [bp-64]
.Ls133:
        jne     .Ls134
        jmp     .Ls162
.Ls134:
        mov     cx,0x8
.Ls135:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls135
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
.Ls136:
        shr     word ptr [bp-58],1
        rcr     word ptr [bp-60],1
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        cmp     word ptr [bp-54],dx
        jne     .Ls137
        cmp     word ptr [bp-56],ax
.Ls137:
        jb      .Ls138
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        or      byte ptr [bp-88],0x1
.Ls138:
        dec     word ptr [bp-8]
        je      .Ls139
        jmp     .Ls131
.Ls139:
        mov     word ptr [bp-72],di
        mov     di,word ptr [bp-112]
        add     di,0x644
        mov     cx,0x4
.Ls140:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Ls140
        mov     word ptr [bp-8],0x4
.Ls141:
        mov     word ptr [bp-6],0x1
        mov     si,0x1
.Ls142:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-2],cx
        cmp     word ptr [bp-58],0x100
        jne     .Ls143
        cmp     word ptr [bp-60],0x0
.Ls143:
        jae     .Ls147
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls144
        cmp     ax,word ptr [bp-64]
.Ls144:
        jne     .Ls145
        jmp     .Ls162
.Ls145:
        mov     word ptr [bp-4],si
        mov     cx,0x8
.Ls146:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Ls146
        mov     dx,word ptr [bp-54]
        mov     ax,word ptr [bp-56]
        M_shld_8
        push    ax
        push    dx
        les     bx,dword ptr [bp-68]
        xor     si,si
        add     word ptr [bp-68],0x1
        adc     si,0x0
        mov     cx,__AHSHIFT
        shl     si,cl
        add     word ptr [bp-66],si
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        test    ax,ax
        cwd
        mov     cx,dx
        mov     bx,ax
        pop     dx
        pop     ax
        or      ax,bx
        or      dx,cx
        mov     word ptr [bp-54],dx
        mov     word ptr [bp-56],ax
        mov     si,word ptr [bp-4]
.Ls147:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [bp-2]
        mov     bx,word ptr [bx]
        mov     word ptr [bp-118],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls148
        cmp     ax,word ptr [bp-56]
.Ls148:
        jbe     .Ls149
        mov     dx,word ptr [bp-42]
        mov     ax,word ptr [bp-44]
        mov     word ptr [bp-58],dx
        mov     word ptr [bp-60],ax
        mov     dx,0x800
        mov     bx,word ptr [bp-2]
        sub     dx,word ptr [bx]
        mov     cx,0x5
        shr     dx,cl
        add     word ptr [bx],dx
        shl     si,1
        jmp     .Ls150
.Ls149:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     dx,word ptr [bp-118]
        mov     cx,0x5
        shr     dx,cl
        mov     bx,word ptr [bp-2]
        sub     word ptr [bx],dx
        shl     si,1
        inc     si
        mov     ax,word ptr [bp-6]
        test    ax,ax
        cwd
        or      word ptr [bp-88],ax
        or      word ptr [bp-86],dx
.Ls150:
        shl     word ptr [bp-6],1
        dec     word ptr [bp-8]
        je      .Ls151
        jmp     .Ls142
.Ls151:
        jmp     .Ls153
.Ls152:
        mov     ax,si
        test    ax,ax
        cwd
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
        mov     word ptr [bp-72],di
.Ls153:
        add     word ptr [bp-88],0x1
        adc     word ptr [bp-86],0x0
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        or      dx,ax
        jne     .Ls154
        jmp     .Ls160
.Ls154:
        add     word ptr [bp-72],0x2
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        cmp     dx,word ptr [bp-106]
        jne     .Ls155
        cmp     ax,word ptr [bp-108]
.Ls155:
        jbe     .Ls156
        jmp     .Ls162
.Ls156:
        mov     di,word ptr [bp-72]
        mov     cx,word ptr [bp+18]
        mov     bx,word ptr [bp+16]
        mov     dx,word ptr [bp-106]
        mov     si,word ptr [bp-108]
        add     si,bx
        mov     bx,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,bx
        mov     cx,dx
        mov     bx,si
        mov     dx,word ptr [bp-86]
        mov     si,ax
        neg     dx
        neg     si
        sbb     dx,0x0
        add     si,bx
        mov     bx,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,bx
        mov     word ptr [bp-0x8c],dx
        mov     word ptr [bp-0x8e],si
.Ls157:
        les     bx,dword ptr [bp-0x8e]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-104],al
        dec     di
        mov     bx,0x1
        xor     dx,dx
        add     word ptr [bp-0x8e],bx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     word ptr [bp-0x8c],dx
        mov     bx,word ptr [bp-108]
        mov     dx,word ptr [bp-106]
        add     word ptr [bp-108],0x1
        adc     word ptr [bp-106],0x0
        mov     cx,word ptr [bp+18]
        mov     si,word ptr [bp+16]
        add     bx,si
        mov     si,cx
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     dx,si
        movw    es,dx
        mov     byte ptr es:[bx],al
        test    di,di
        jne     .Ls158
        jmp     .Ls038
.Ls158:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Ls159
        cmp     ax,word ptr [bp+20]
.Ls159:
        jb      .Ls157
        jmp     .Ls038
.Ls160:
        cmp     word ptr [bp-58],0x100
        jne     .Ls161
        cmp     word ptr [bp-60],0x0
.Ls161:
        jae     .Ls164
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls163
        cmp     ax,word ptr [bp-64]
        jne     .Ls163
.Ls162:
        mov     ax,0x1
        pop     di
        pop     si
        mov     sp,bp
        pop     bp
        ret
.Ls163:
        mov     ax,0x1
        cwd
        add     word ptr [bp-68],ax
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     word ptr [bp-66],dx
.Ls164:
        mov     bx,word ptr [bp+14]
        mov     word ptr [bx+2],dx
        mov     word ptr [bx],ax
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        mov     bx,word ptr [bp+24]
        mov     word ptr [bx+2],dx
        mov     word ptr [bx],ax
        xor     ax,ax
        pop     di
        pop     si
        mov     sp,bp
        pop     bp
        ret

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
        jmp     .Ls148
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
        jmp     .Ls146
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
        jae     .Ls012
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls010
        cmp     ax,word ptr [bp-64]
.Ls010:
        jne     .Ls011
        jmp     .Ls148
.Ls011:
        M_shld_8_bp -60 -58
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
.Ls012:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls013
        cmp     ax,word ptr [bp-56]
.Ls013:
        ja      .Ls014
        jmp     .Ls040
.Ls014:
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
        jge     .Ls015
        jmp     .Ls024
.Ls015:
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
.Ls016:
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
        jne     .Ls017
        cmp     word ptr [bp-60],0x0
.Ls017:
        jae     .Ls020
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls018
        cmp     ax,word ptr [bp-64]
.Ls018:
        jne     .Ls019
        jmp     .Ls148
.Ls019:
        mov     word ptr [bp-32],si
        M_shld_8_bp -60 -58
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
.Ls020:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-116],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls021
        cmp     ax,word ptr [bp-56]
.Ls021:
        jbe     .Ls022
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
        jne     .Ls024
        jmp     .Ls023
.Ls022:
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
        je      .Ls024
.Ls023:
        cmp     word ptr [bp-38],0x100
        jge     .Ls024
        jmp     .Ls016
.Ls024:
        cmp     word ptr [bp-38],0x100
        jl      .Ls025
        jmp     .Ls034
.Ls025:
        mov     si,word ptr [bp-38]
        shl     si,1
        add     si,di
        cmp     word ptr [bp-58],0x100
        jne     .Ls026
        cmp     word ptr [bp-60],0x0
.Ls026:
        jae     .Ls029
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls027
        cmp     ax,word ptr [bp-64]
.Ls027:
        jne     .Ls028
        jmp     .Ls148
.Ls028:
        mov     word ptr [bp-30],si
        M_shld_8_bp -60 -58
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
.Ls029:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-114],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls030
        cmp     ax,word ptr [bp-56]
.Ls030:
        jbe     .Ls033
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
.Ls031:
        cmp     word ptr [bp-38],0x100
        jge     .Ls032
        jmp     .Ls025
.Ls032:
        jmp     .Ls034
.Ls033:
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
        jmp     .Ls031
.Ls034:
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
        jge     .Ls038
        mov     word ptr [bp-90],0x0
.Ls035:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Ls036
        cmp     ax,word ptr [bp+20]
.Ls036:
        jae     .Ls037
        jmp     .Ls008
.Ls037:
        jmp     .Ls146
.Ls038:
        cmp     word ptr [bp-90],0xa
        jge     .Ls039
        add     word ptr [bp-90],0xfffffffd
        jmp     .Ls035
.Ls039:
        add     word ptr [bp-90],0xfffffffa
        jmp     .Ls035
.Ls040:
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
        jne     .Ls041
        cmp     word ptr [bp-60],0x0
.Ls041:
        jae     .Ls044
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls042
        cmp     ax,word ptr [bp-64]
.Ls042:
        jne     .Ls043
        jmp     .Ls148
.Ls043:
        M_shld_8_bp -60 -58
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
.Ls044:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls045
        cmp     ax,word ptr [bp-56]
.Ls045:
        jbe     .Ls047
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
        jl      .Ls046
        mov     al,0x3
.Ls046:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0x664
        jmp     .Ls080
.Ls047:
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
        jne     .Ls048
        cmp     word ptr [bp-60],0x0
.Ls048:
        jae     .Ls051
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls049
        cmp     ax,word ptr [bp-64]
.Ls049:
        jne     .Ls050
        jmp     .Ls148
.Ls050:
        M_shld_8_bp -60 -58
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
.Ls051:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls052
        cmp     ax,word ptr [bp-56]
.Ls052:
        ja      .Ls053
        jmp     .Ls063
.Ls053:
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
        jne     .Ls054
        test    ax,ax
.Ls054:
        jae     .Ls057
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls055
        cmp     ax,word ptr [bp-64]
.Ls055:
        jne     .Ls056
        jmp     .Ls148
.Ls056:
        M_shld_8_bp -60 -58
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
.Ls057:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls058
        cmp     ax,word ptr [bp-56]
.Ls058:
        ja      .Ls059
        jmp     .Ls062
.Ls059:
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
        jne     .Ls060
        jmp     .Ls148
.Ls060:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x9
        jl      .Ls061
        mov     al,0xb
.Ls061:
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
        jmp     .Ls035
.Ls062:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        jmp     .Ls078
.Ls063:
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
        jne     .Ls064
        cmp     word ptr [bp-60],0x0
.Ls064:
        jae     .Ls067
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls065
        cmp     ax,word ptr [bp-64]
.Ls065:
        jne     .Ls066
        jmp     .Ls148
.Ls066:
        M_shld_8_bp -60 -58
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
.Ls067:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls068
        cmp     ax,word ptr [bp-56]
.Ls068:
        jbe     .Ls069
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
        jmp     .Ls077
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
        add     di,0x1c8
        cmp     word ptr [bp-58],0x100
        jne     .Ls070
        cmp     word ptr [bp-60],0x0
.Ls070:
        jae     .Ls073
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls071
        cmp     ax,word ptr [bp-64]
.Ls071:
        jne     .Ls072
        jmp     .Ls148
.Ls072:
        M_shld_8_bp -60 -58
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
.Ls073:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls074
        cmp     ax,word ptr [bp-56]
.Ls074:
        jbe     .Ls075
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
        jmp     .Ls076
.Ls075:
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
.Ls076:
        mov     dx,word ptr [bp-82]
        mov     ax,word ptr [bp-84]
        mov     word ptr [bp-78],dx
        mov     word ptr [bp-80],ax
.Ls077:
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        mov     word ptr [bp-82],dx
        mov     word ptr [bp-84],ax
        mov     dx,word ptr [bp-26]
        mov     ax,word ptr [bp-28]
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
.Ls078:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x8
        jl      .Ls079
        mov     al,0xb
.Ls079:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0xa68
.Ls080:
        cmp     word ptr [bp-58],0x100
        jne     .Ls081
        cmp     word ptr [bp-60],0x0
.Ls081:
        jae     .Ls084
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls082
        cmp     ax,word ptr [bp-64]
.Ls082:
        jne     .Ls083
        jmp     .Ls148
.Ls083:
        M_shld_8_bp -60 -58
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
.Ls084:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls085
        cmp     ax,word ptr [bp-56]
.Ls085:
        jbe     .Ls086
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
        jmp     .Ls093
.Ls086:
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
        jne     .Ls087
        cmp     word ptr [bp-60],0x0
.Ls087:
        jae     .Ls090
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls088
        cmp     ax,word ptr [bp-64]
.Ls088:
        jne     .Ls089
        jmp     .Ls148
.Ls089:
        M_shld_8_bp -60 -58
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
.Ls090:
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
        jne     .Ls091
        cmp     ax,word ptr [bp-56]
.Ls091:
        jbe     .Ls092
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
        jmp     .Ls093
.Ls092:
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
.Ls093:
        mov     ax,word ptr [bp-24]
        mov     word ptr [bp-18],ax
        mov     di,0x1
.Ls094:
        mov     si,di
        shl     si,1
        add     si,word ptr [bp-20]
        cmp     word ptr [bp-58],0x100
        jne     .Ls095
        cmp     word ptr [bp-60],0x0
.Ls095:
        jae     .Ls098
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls096
        cmp     ax,word ptr [bp-64]
.Ls096:
        jne     .Ls097
        jmp     .Ls148
.Ls097:
        mov     word ptr [bp-16],si
        M_shld_8_bp -60 -58
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
.Ls098:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-122],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Ls099
        cmp     ax,word ptr [bp-56]
.Ls099:
        jbe     .Ls100
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
        jmp     .Ls101
.Ls100:
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
.Ls101:
        dec     word ptr [bp-18]
        je      .Ls102
        jmp     .Ls094
.Ls102:
        mov     word ptr [bp-72],di
        mov     ax,0x1
        mov     cx,word ptr [bp-24]
        shl     ax,cl
        sub     word ptr [bp-72],ax
        mov     ax,word ptr [bp-22]
        add     word ptr [bp-72],ax
        cmp     word ptr [bp-90],0x4
        jl      .Ls103
        jmp     .Ls140
.Ls103:
        add     word ptr [bp-90],0x7
        cmp     word ptr [bp-72],0x4
        jge     .Ls104
        mov     di,word ptr [bp-72]
        jmp     .Ls105
.Ls104:
        mov     di,0x3
.Ls105:
        mov     cx,0x7
        shl     di,cl
        add     di,word ptr [bp-112]
        add     di,0x360
        mov     word ptr [bp-12],0x6
        mov     si,0x1
.Ls106:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-10],cx
        cmp     word ptr [bp-58],0x100
        jne     .Ls107
        cmp     word ptr [bp-60],0x0
.Ls107:
        jae     .Ls110
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls108
        cmp     ax,word ptr [bp-64]
.Ls108:
        jne     .Ls109
        jmp     .Ls148
.Ls109:
        mov     word ptr [bp-14],si
        M_shld_8_bp -60 -58
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
.Ls110:
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
        jne     .Ls111
        cmp     ax,word ptr [bp-56]
.Ls111:
        jbe     .Ls112
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
        jmp     .Ls113
.Ls112:
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
.Ls113:
        dec     word ptr [bp-12]
        je      .Ls114
        jmp     .Ls106
.Ls114:
        mov     di,word ptr [bp-72]
        add     si,0xffffffc0
        cmp     si,0x4
        jge     .Ls115
        jmp     .Ls138
.Ls115:
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
        jge     .Ls118
        mov     word ptr [bp-72],di
        mov     cx,word ptr [bp-8]
        jcxz    .Ls117
.Ls116:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Ls116
.Ls117:
        mov     di,word ptr [bp-88]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x55e
        mov     cx,si
        shl     cx,1
        sub     di,cx
        jmp     .Ls128
.Ls118:
        add     word ptr [bp-8],0xfffffffc
.Ls119:
        cmp     word ptr [bp-58],0x100
        jne     .Ls120
        cmp     word ptr [bp-60],0x0
.Ls120:
        jae     .Ls123
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls121
        cmp     ax,word ptr [bp-64]
.Ls121:
        jne     .Ls122
        jmp     .Ls148
.Ls122:
        M_shld_8_bp -60 -58
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
.Ls123:
        shr     word ptr [bp-58],1
        rcr     word ptr [bp-60],1
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        cmp     word ptr [bp-54],dx
        jne     .Ls124
        cmp     word ptr [bp-56],ax
.Ls124:
        jb      .Ls125
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        or      byte ptr [bp-88],0x1
.Ls125:
        dec     word ptr [bp-8]
        je      .Ls126
        jmp     .Ls119
.Ls126:
        mov     word ptr [bp-72],di
        mov     di,word ptr [bp-112]
        add     di,0x644
        mov     cx,0x4
.Ls127:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Ls127
        mov     word ptr [bp-8],0x4
.Ls128:
        mov     word ptr [bp-6],0x1
        mov     si,0x1
.Ls129:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-2],cx
        cmp     word ptr [bp-58],0x100
        jne     .Ls130
        cmp     word ptr [bp-60],0x0
.Ls130:
        jae     .Ls133
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls131
        cmp     ax,word ptr [bp-64]
.Ls131:
        jne     .Ls132
        jmp     .Ls148
.Ls132:
        mov     word ptr [bp-4],si
        M_shld_8_bp -60 -58
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
.Ls133:
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
        jne     .Ls134
        cmp     ax,word ptr [bp-56]
.Ls134:
        jbe     .Ls135
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
        jmp     .Ls136
.Ls135:
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
.Ls136:
        shl     word ptr [bp-6],1
        dec     word ptr [bp-8]
        je      .Ls137
        jmp     .Ls129
.Ls137:
        jmp     .Ls139
.Ls138:
        mov     ax,si
        test    ax,ax
        cwd
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
        mov     word ptr [bp-72],di
.Ls139:
        add     word ptr [bp-88],0x1
        adc     word ptr [bp-86],0x0
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        or      dx,ax
        jne     .Ls140
        jmp     .Ls146
.Ls140:
        add     word ptr [bp-72],0x2
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        cmp     dx,word ptr [bp-106]
        jne     .Ls141
        cmp     ax,word ptr [bp-108]
.Ls141:
        jbe     .Ls142
        jmp     .Ls148
.Ls142:
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
.Ls143:
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
        jne     .Ls144
        jmp     .Ls035
.Ls144:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Ls145
        cmp     ax,word ptr [bp+20]
.Ls145:
        jb      .Ls143
        jmp     .Ls035
.Ls146:
        cmp     word ptr [bp-58],0x100
        jne     .Ls147
        cmp     word ptr [bp-60],0x0
.Ls147:
        jae     .Ls150
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Ls149
        cmp     ax,word ptr [bp-64]
        jne     .Ls149
.Ls148:
        mov     ax,0x1
        pop     di
        pop     si
        mov     sp,bp
        pop     bp
        ret
.Ls149:
        mov     ax,0x1
        cwd
        add     word ptr [bp-68],ax
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     word ptr [bp-66],dx
.Ls150:
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

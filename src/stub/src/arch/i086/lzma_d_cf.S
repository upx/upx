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
        jcxz    .Lf001
.Lf000:
        shl     bx,1
        rcl     dx,1
        loop    .Lf000
.Lf001:
        add     bx,0x736
        adc     dx,cx
        mov     ax,dx
        or      ax,bx
        mov     word ptr [bp-46],dx
        mov     word ptr [bp-48],bx
        je      .Lf004
        mov     si,word ptr [bp-52]
        shl     si,1
        add     si,word ptr [bp-112]
.Lf002:
        mov     word ptr [si],0x400
        add     si,0x2
        add     word ptr [bp-52],0x1
        adc     word ptr [bp-50],0x0
        mov     dx,word ptr [bp-50]
        mov     ax,word ptr [bp-52]
        cmp     dx,word ptr [bp-46]
        jne     .Lf003
        cmp     ax,word ptr [bp-48]
.Lf003:
        jb      .Lf002
.Lf004:
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
.Lf005:
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf006
        cmp     ax,word ptr [bp-64]
.Lf006:
        jne     .Lf007
        jmp     .Lf148
.Lf007:
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
        jb      .Lf005
        mov     ax,word ptr [bp+22]
        or      ax,word ptr [bp+20]
        jne     .Lf008
        jmp     .Lf146
.Lf008:
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
        jne     .Lf009
        cmp     word ptr [bp-60],0x0
.Lf009:
        jae     .Lf012
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf010
        cmp     ax,word ptr [bp-64]
.Lf010:
        jne     .Lf011
        jmp     .Lf148
.Lf011:
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
.Lf012:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf013
        cmp     ax,word ptr [bp-56]
.Lf013:
        ja      .Lf014
        jmp     .Lf040
.Lf014:
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
        jge     .Lf015
        jmp     .Lf024
.Lf015:
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
.Lf016:
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
        jne     .Lf017
        cmp     word ptr [bp-60],0x0
.Lf017:
        jae     .Lf020
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf018
        cmp     ax,word ptr [bp-64]
.Lf018:
        jne     .Lf019
        jmp     .Lf148
.Lf019:
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
.Lf020:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-116],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf021
        cmp     ax,word ptr [bp-56]
.Lf021:
        jbe     .Lf022
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
        jne     .Lf024
        jmp     .Lf023
.Lf022:
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
        je      .Lf024
.Lf023:
        cmp     word ptr [bp-38],0x100
        jge     .Lf024
        jmp     .Lf016
.Lf024:
        cmp     word ptr [bp-38],0x100
        jl      .Lf025
        jmp     .Lf034
.Lf025:
        mov     si,word ptr [bp-38]
        shl     si,1
        add     si,di
        cmp     word ptr [bp-58],0x100
        jne     .Lf026
        cmp     word ptr [bp-60],0x0
.Lf026:
        jae     .Lf029
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf027
        cmp     ax,word ptr [bp-64]
.Lf027:
        jne     .Lf028
        jmp     .Lf148
.Lf028:
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
.Lf029:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-114],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf030
        cmp     ax,word ptr [bp-56]
.Lf030:
        jbe     .Lf033
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
.Lf031:
        cmp     word ptr [bp-38],0x100
        jge     .Lf032
        jmp     .Lf025
.Lf032:
        jmp     .Lf034
.Lf033:
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
        jmp     .Lf031
.Lf034:
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
        jge     .Lf038
        mov     word ptr [bp-90],0x0
.Lf035:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Lf036
        cmp     ax,word ptr [bp+20]
.Lf036:
        jae     .Lf037
        jmp     .Lf008
.Lf037:
        jmp     .Lf146
.Lf038:
        cmp     word ptr [bp-90],0xa
        jge     .Lf039
        add     word ptr [bp-90],0xfffffffd
        jmp     .Lf035
.Lf039:
        add     word ptr [bp-90],0xfffffffa
        jmp     .Lf035
.Lf040:
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
        jne     .Lf041
        cmp     word ptr [bp-60],0x0
.Lf041:
        jae     .Lf044
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf042
        cmp     ax,word ptr [bp-64]
.Lf042:
        jne     .Lf043
        jmp     .Lf148
.Lf043:
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
.Lf044:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf045
        cmp     ax,word ptr [bp-56]
.Lf045:
        jbe     .Lf047
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
        jl      .Lf046
        mov     al,0x3
.Lf046:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0x664
        jmp     .Lf080
.Lf047:
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
        jne     .Lf048
        cmp     word ptr [bp-60],0x0
.Lf048:
        jae     .Lf051
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf049
        cmp     ax,word ptr [bp-64]
.Lf049:
        jne     .Lf050
        jmp     .Lf148
.Lf050:
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
.Lf051:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf052
        cmp     ax,word ptr [bp-56]
.Lf052:
        ja      .Lf053
        jmp     .Lf063
.Lf053:
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
        jne     .Lf054
        test    ax,ax
.Lf054:
        jae     .Lf057
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf055
        cmp     ax,word ptr [bp-64]
.Lf055:
        jne     .Lf056
        jmp     .Lf148
.Lf056:
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
.Lf057:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf058
        cmp     ax,word ptr [bp-56]
.Lf058:
        ja      .Lf059
        jmp     .Lf062
.Lf059:
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
        jne     .Lf060
        jmp     .Lf148
.Lf060:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x9
        jl      .Lf061
        mov     al,0xb
.Lf061:
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
        jmp     .Lf035
.Lf062:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        jmp     .Lf078
.Lf063:
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
        jne     .Lf064
        cmp     word ptr [bp-60],0x0
.Lf064:
        jae     .Lf067
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf065
        cmp     ax,word ptr [bp-64]
.Lf065:
        jne     .Lf066
        jmp     .Lf148
.Lf066:
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
.Lf067:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf068
        cmp     ax,word ptr [bp-56]
.Lf068:
        jbe     .Lf069
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
        jmp     .Lf077
.Lf069:
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
        jne     .Lf070
        cmp     word ptr [bp-60],0x0
.Lf070:
        jae     .Lf073
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf071
        cmp     ax,word ptr [bp-64]
.Lf071:
        jne     .Lf072
        jmp     .Lf148
.Lf072:
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
.Lf073:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf074
        cmp     ax,word ptr [bp-56]
.Lf074:
        jbe     .Lf075
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
        jmp     .Lf076
.Lf075:
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
.Lf076:
        mov     dx,word ptr [bp-82]
        mov     ax,word ptr [bp-84]
        mov     word ptr [bp-78],dx
        mov     word ptr [bp-80],ax
.Lf077:
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        mov     word ptr [bp-82],dx
        mov     word ptr [bp-84],ax
        mov     dx,word ptr [bp-26]
        mov     ax,word ptr [bp-28]
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
.Lf078:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x8
        jl      .Lf079
        mov     al,0xb
.Lf079:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0xa68
.Lf080:
        cmp     word ptr [bp-58],0x100
        jne     .Lf081
        cmp     word ptr [bp-60],0x0
.Lf081:
        jae     .Lf084
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf082
        cmp     ax,word ptr [bp-64]
.Lf082:
        jne     .Lf083
        jmp     .Lf148
.Lf083:
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
.Lf084:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf085
        cmp     ax,word ptr [bp-56]
.Lf085:
        jbe     .Lf086
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
        jmp     .Lf093
.Lf086:
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
        jne     .Lf087
        cmp     word ptr [bp-60],0x0
.Lf087:
        jae     .Lf090
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf088
        cmp     ax,word ptr [bp-64]
.Lf088:
        jne     .Lf089
        jmp     .Lf148
.Lf089:
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
.Lf090:
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
        jne     .Lf091
        cmp     ax,word ptr [bp-56]
.Lf091:
        jbe     .Lf092
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
        jmp     .Lf093
.Lf092:
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
.Lf093:
        mov     ax,word ptr [bp-24]
        mov     word ptr [bp-18],ax
        mov     di,0x1
.Lf094:
        mov     si,di
        shl     si,1
        add     si,word ptr [bp-20]
        cmp     word ptr [bp-58],0x100
        jne     .Lf095
        cmp     word ptr [bp-60],0x0
.Lf095:
        jae     .Lf098
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf096
        cmp     ax,word ptr [bp-64]
.Lf096:
        jne     .Lf097
        jmp     .Lf148
.Lf097:
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
.Lf098:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-122],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf099
        cmp     ax,word ptr [bp-56]
.Lf099:
        jbe     .Lf100
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
        jmp     .Lf101
.Lf100:
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
.Lf101:
        dec     word ptr [bp-18]
        je      .Lf102
        jmp     .Lf094
.Lf102:
        mov     word ptr [bp-72],di
        mov     ax,0x1
        mov     cx,word ptr [bp-24]
        shl     ax,cl
        sub     word ptr [bp-72],ax
        mov     ax,word ptr [bp-22]
        add     word ptr [bp-72],ax
        cmp     word ptr [bp-90],0x4
        jl      .Lf103
        jmp     .Lf140
.Lf103:
        add     word ptr [bp-90],0x7
        cmp     word ptr [bp-72],0x4
        jge     .Lf104
        mov     di,word ptr [bp-72]
        jmp     .Lf105
.Lf104:
        mov     di,0x3
.Lf105:
        mov     cx,0x7
        shl     di,cl
        add     di,word ptr [bp-112]
        add     di,0x360
        mov     word ptr [bp-12],0x6
        mov     si,0x1
.Lf106:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-10],cx
        cmp     word ptr [bp-58],0x100
        jne     .Lf107
        cmp     word ptr [bp-60],0x0
.Lf107:
        jae     .Lf110
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf108
        cmp     ax,word ptr [bp-64]
.Lf108:
        jne     .Lf109
        jmp     .Lf148
.Lf109:
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
.Lf110:
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
        jne     .Lf111
        cmp     ax,word ptr [bp-56]
.Lf111:
        jbe     .Lf112
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
        jmp     .Lf113
.Lf112:
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
.Lf113:
        dec     word ptr [bp-12]
        je      .Lf114
        jmp     .Lf106
.Lf114:
        mov     di,word ptr [bp-72]
        add     si,0xffffffc0
        cmp     si,0x4
        jge     .Lf115
        jmp     .Lf138
.Lf115:
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
        jge     .Lf118
        mov     word ptr [bp-72],di
        mov     cx,word ptr [bp-8]
        jcxz    .Lf117
.Lf116:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Lf116
.Lf117:
        mov     di,word ptr [bp-88]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x55e
        mov     cx,si
        shl     cx,1
        sub     di,cx
        jmp     .Lf128
.Lf118:
        add     word ptr [bp-8],0xfffffffc
.Lf119:
        cmp     word ptr [bp-58],0x100
        jne     .Lf120
        cmp     word ptr [bp-60],0x0
.Lf120:
        jae     .Lf123
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf121
        cmp     ax,word ptr [bp-64]
.Lf121:
        jne     .Lf122
        jmp     .Lf148
.Lf122:
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
.Lf123:
        shr     word ptr [bp-58],1
        rcr     word ptr [bp-60],1
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        cmp     word ptr [bp-54],dx
        jne     .Lf124
        cmp     word ptr [bp-56],ax
.Lf124:
        jb      .Lf125
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        or      byte ptr [bp-88],0x1
.Lf125:
        dec     word ptr [bp-8]
        je      .Lf126
        jmp     .Lf119
.Lf126:
        mov     word ptr [bp-72],di
        mov     di,word ptr [bp-112]
        add     di,0x644
        mov     cx,0x4
.Lf127:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Lf127
        mov     word ptr [bp-8],0x4
.Lf128:
        mov     word ptr [bp-6],0x1
        mov     si,0x1
.Lf129:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-2],cx
        cmp     word ptr [bp-58],0x100
        jne     .Lf130
        cmp     word ptr [bp-60],0x0
.Lf130:
        jae     .Lf133
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf131
        cmp     ax,word ptr [bp-64]
.Lf131:
        jne     .Lf132
        jmp     .Lf148
.Lf132:
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
.Lf133:
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
        jne     .Lf134
        cmp     ax,word ptr [bp-56]
.Lf134:
        jbe     .Lf135
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
        jmp     .Lf136
.Lf135:
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
.Lf136:
        shl     word ptr [bp-6],1
        dec     word ptr [bp-8]
        je      .Lf137
        jmp     .Lf129
.Lf137:
        jmp     .Lf139
.Lf138:
        mov     ax,si
        test    ax,ax
        cwd
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
        mov     word ptr [bp-72],di
.Lf139:
        add     word ptr [bp-88],0x1
        adc     word ptr [bp-86],0x0
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        or      dx,ax
        jne     .Lf140
        jmp     .Lf146
.Lf140:
        add     word ptr [bp-72],0x2
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        cmp     dx,word ptr [bp-106]
        jne     .Lf141
        cmp     ax,word ptr [bp-108]
.Lf141:
        jbe     .Lf142
        jmp     .Lf148
.Lf142:
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
.Lf143:
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
        jne     .Lf144
        jmp     .Lf035
.Lf144:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Lf145
        cmp     ax,word ptr [bp+20]
.Lf145:
        jb      .Lf143
        jmp     .Lf035
.Lf146:
        cmp     word ptr [bp-58],0x100
        jne     .Lf147
        cmp     word ptr [bp-60],0x0
.Lf147:
        jae     .Lf150
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf149
        cmp     ax,word ptr [bp-64]
        jne     .Lf149
.Lf148:
        mov     ax,0x1
        pop     di
        pop     si
        mov     sp,bp
        pop     bp
        ret
.Lf149:
        mov     ax,0x1
        cwd
        add     word ptr [bp-68],ax
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     word ptr [bp-66],dx
.Lf150:
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

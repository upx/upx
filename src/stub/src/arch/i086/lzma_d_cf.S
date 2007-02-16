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
        jmp     .Lf162
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
        jmp     .Lf160
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
        jae     .Lf013
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf010
        cmp     ax,word ptr [bp-64]
.Lf010:
        jne     .Lf011
        jmp     .Lf162
.Lf011:
        mov     cl,0x8
.Lf012:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf012
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
.Lf013:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf014
        cmp     ax,word ptr [bp-56]
.Lf014:
        ja      .Lf015
        jmp     .Lf043
.Lf015:
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
        jge     .Lf016
        jmp     .Lf026
.Lf016:
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
.Lf017:
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
        jne     .Lf018
        cmp     word ptr [bp-60],0x0
.Lf018:
        jae     .Lf022
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf019
        cmp     ax,word ptr [bp-64]
.Lf019:
        jne     .Lf020
        jmp     .Lf162
.Lf020:
        mov     word ptr [bp-32],si
        mov     cx,0x8
.Lf021:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf021
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
.Lf022:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-116],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf023
        cmp     ax,word ptr [bp-56]
.Lf023:
        jbe     .Lf024
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
        jne     .Lf026
        jmp     .Lf025
.Lf024:
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
        je      .Lf026
.Lf025:
        cmp     word ptr [bp-38],0x100
        jge     .Lf026
        jmp     .Lf017
.Lf026:
        cmp     word ptr [bp-38],0x100
        jl      .Lf027
        jmp     .Lf037
.Lf027:
        mov     si,word ptr [bp-38]
        shl     si,1
        add     si,di
        cmp     word ptr [bp-58],0x100
        jne     .Lf028
        cmp     word ptr [bp-60],0x0
.Lf028:
        jae     .Lf032
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf029
        cmp     ax,word ptr [bp-64]
.Lf029:
        jne     .Lf030
        jmp     .Lf162
.Lf030:
        mov     word ptr [bp-30],si
        mov     cx,0x8
.Lf031:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf031
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
.Lf032:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-114],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf033
        cmp     ax,word ptr [bp-56]
.Lf033:
        jbe     .Lf036
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
.Lf034:
        cmp     word ptr [bp-38],0x100
        jge     .Lf035
        jmp     .Lf027
.Lf035:
        jmp     .Lf037
.Lf036:
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
        jmp     .Lf034
.Lf037:
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
        jge     .Lf041
        mov     word ptr [bp-90],0x0
.Lf038:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Lf039
        cmp     ax,word ptr [bp+20]
.Lf039:
        jae     .Lf040
        jmp     .Lf008
.Lf040:
        jmp     .Lf160
.Lf041:
        cmp     word ptr [bp-90],0xa
        jge     .Lf042
        add     word ptr [bp-90],0xfffffffd
        jmp     .Lf038
.Lf042:
        add     word ptr [bp-90],0xfffffffa
        jmp     .Lf038
.Lf043:
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
        jne     .Lf044
        cmp     word ptr [bp-60],0x0
.Lf044:
        jae     .Lf048
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf045
        cmp     ax,word ptr [bp-64]
.Lf045:
        jne     .Lf046
        jmp     .Lf162
.Lf046:
        mov     cl,0x8
.Lf047:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf047
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
.Lf048:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf049
        cmp     ax,word ptr [bp-56]
.Lf049:
        jbe     .Lf051
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
        jl      .Lf050
        mov     al,0x3
.Lf050:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0x664
        jmp     .Lf088
.Lf051:
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
        jne     .Lf052
        cmp     word ptr [bp-60],0x0
.Lf052:
        jae     .Lf056
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf053
        cmp     ax,word ptr [bp-64]
.Lf053:
        jne     .Lf054
        jmp     .Lf162
.Lf054:
        mov     cl,0x8
.Lf055:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf055
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
.Lf056:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf057
        cmp     ax,word ptr [bp-56]
.Lf057:
        ja      .Lf058
        jmp     .Lf069
.Lf058:
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
        jne     .Lf059
        test    ax,ax
.Lf059:
        jae     .Lf063
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf060
        cmp     ax,word ptr [bp-64]
.Lf060:
        jne     .Lf061
        jmp     .Lf162
.Lf061:
        mov     cl,0x8
.Lf062:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf062
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
.Lf063:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf064
        cmp     ax,word ptr [bp-56]
.Lf064:
        ja      .Lf065
        jmp     .Lf068
.Lf065:
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
        jne     .Lf066
        jmp     .Lf162
.Lf066:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x9
        jl      .Lf067
        mov     al,0xb
.Lf067:
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
        jmp     .Lf038
.Lf068:
        sub     word ptr [bp-60],ax
        sbb     word ptr [bp-58],dx
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        mov     ax,word ptr [di]
        mov     cx,0x5
        shr     ax,cl
        sub     word ptr [di],ax
        jmp     .Lf086
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
        add     di,0x1b0
        cmp     word ptr [bp-58],0x100
        jne     .Lf070
        cmp     word ptr [bp-60],0x0
.Lf070:
        jae     .Lf074
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf071
        cmp     ax,word ptr [bp-64]
.Lf071:
        jne     .Lf072
        jmp     .Lf162
.Lf072:
        mov     cl,0x8
.Lf073:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf073
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
.Lf074:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf075
        cmp     ax,word ptr [bp-56]
.Lf075:
        jbe     .Lf076
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
        jmp     .Lf085
.Lf076:
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
        jne     .Lf077
        cmp     word ptr [bp-60],0x0
.Lf077:
        jae     .Lf081
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf078
        cmp     ax,word ptr [bp-64]
.Lf078:
        jne     .Lf079
        jmp     .Lf162
.Lf079:
        mov     cl,0x8
.Lf080:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf080
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
.Lf081:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf082
        cmp     ax,word ptr [bp-56]
.Lf082:
        jbe     .Lf083
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
        jmp     .Lf084
.Lf083:
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
.Lf084:
        mov     dx,word ptr [bp-82]
        mov     ax,word ptr [bp-84]
        mov     word ptr [bp-78],dx
        mov     word ptr [bp-80],ax
.Lf085:
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        mov     word ptr [bp-82],dx
        mov     word ptr [bp-84],ax
        mov     dx,word ptr [bp-26]
        mov     ax,word ptr [bp-28]
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
.Lf086:
        cmp     word ptr [bp-90],0x7
        mov     ax,0x8
        jl      .Lf087
        mov     al,0xb
.Lf087:
        mov     word ptr [bp-90],ax
        mov     di,word ptr [bp-112]
        add     di,0xa68
.Lf088:
        cmp     word ptr [bp-58],0x100
        jne     .Lf089
        cmp     word ptr [bp-60],0x0
.Lf089:
        jae     .Lf093
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf090
        cmp     ax,word ptr [bp-64]
.Lf090:
        jne     .Lf091
        jmp     .Lf162
.Lf091:
        mov     cl,0x8
.Lf092:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf092
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
.Lf093:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [di]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf094
        cmp     ax,word ptr [bp-56]
.Lf094:
        jbe     .Lf095
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
        jmp     .Lf103
.Lf095:
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
        jne     .Lf096
        cmp     word ptr [bp-60],0x0
.Lf096:
        jae     .Lf100
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf097
        cmp     ax,word ptr [bp-64]
.Lf097:
        jne     .Lf098
        jmp     .Lf162
.Lf098:
        mov     cl,0x8
.Lf099:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf099
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
.Lf100:
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
        jne     .Lf101
        cmp     ax,word ptr [bp-56]
.Lf101:
        jbe     .Lf102
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
        jmp     .Lf103
.Lf102:
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
.Lf103:
        mov     ax,word ptr [bp-24]
        mov     word ptr [bp-18],ax
        mov     di,0x1
.Lf104:
        mov     si,di
        shl     si,1
        add     si,word ptr [bp-20]
        cmp     word ptr [bp-58],0x100
        jne     .Lf105
        cmp     word ptr [bp-60],0x0
.Lf105:
        jae     .Lf109
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf106
        cmp     ax,word ptr [bp-64]
.Lf106:
        jne     .Lf107
        jmp     .Lf162
.Lf107:
        mov     word ptr [bp-16],si
        mov     cx,0x8
.Lf108:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf108
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
.Lf109:
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        M_shrd_11
        mov     bx,word ptr [si]
        mov     word ptr [bp-122],bx
        M_LMUL_dxax_00bx
        mov     word ptr [bp-42],dx
        mov     word ptr [bp-44],ax
        cmp     dx,word ptr [bp-54]
        jne     .Lf110
        cmp     ax,word ptr [bp-56]
.Lf110:
        jbe     .Lf111
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
        jmp     .Lf112
.Lf111:
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
.Lf112:
        dec     word ptr [bp-18]
        je      .Lf113
        jmp     .Lf104
.Lf113:
        mov     word ptr [bp-72],di
        mov     ax,0x1
        mov     cx,word ptr [bp-24]
        shl     ax,cl
        sub     word ptr [bp-72],ax
        mov     ax,word ptr [bp-22]
        add     word ptr [bp-72],ax
        cmp     word ptr [bp-90],0x4
        jl      .Lf114
        jmp     .Lf154
.Lf114:
        add     word ptr [bp-90],0x7
        cmp     word ptr [bp-72],0x4
        jge     .Lf115
        mov     di,word ptr [bp-72]
        jmp     .Lf116
.Lf115:
        mov     di,0x3
.Lf116:
        mov     cx,0x7
        shl     di,cl
        add     di,word ptr [bp-112]
        add     di,0x360
        mov     word ptr [bp-12],0x6
        mov     si,0x1
.Lf117:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-10],cx
        cmp     word ptr [bp-58],0x100
        jne     .Lf118
        cmp     word ptr [bp-60],0x0
.Lf118:
        jae     .Lf122
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf119
        cmp     ax,word ptr [bp-64]
.Lf119:
        jne     .Lf120
        jmp     .Lf162
.Lf120:
        mov     word ptr [bp-14],si
        mov     cx,0x8
.Lf121:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf121
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
.Lf122:
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
        jne     .Lf123
        cmp     ax,word ptr [bp-56]
.Lf123:
        jbe     .Lf124
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
        jmp     .Lf125
.Lf124:
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
.Lf125:
        dec     word ptr [bp-12]
        je      .Lf126
        jmp     .Lf117
.Lf126:
        mov     di,word ptr [bp-72]
        add     si,0xffffffc0
        cmp     si,0x4
        jge     .Lf127
        jmp     .Lf152
.Lf127:
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
        jge     .Lf130
        mov     word ptr [bp-72],di
        mov     cx,word ptr [bp-8]
        jcxz    .Lf129
.Lf128:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Lf128
.Lf129:
        mov     di,word ptr [bp-88]
        shl     di,1
        add     di,word ptr [bp-112]
        add     di,0x55e
        mov     cx,si
        shl     cx,1
        sub     di,cx
        jmp     .Lf141
.Lf130:
        add     word ptr [bp-8],0xfffffffc
.Lf131:
        cmp     word ptr [bp-58],0x100
        jne     .Lf132
        cmp     word ptr [bp-60],0x0
.Lf132:
        jae     .Lf136
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf133
        cmp     ax,word ptr [bp-64]
.Lf133:
        jne     .Lf134
        jmp     .Lf162
.Lf134:
        mov     cx,0x8
.Lf135:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf135
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
.Lf136:
        shr     word ptr [bp-58],1
        rcr     word ptr [bp-60],1
        mov     dx,word ptr [bp-58]
        mov     ax,word ptr [bp-60]
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        cmp     word ptr [bp-54],dx
        jne     .Lf137
        cmp     word ptr [bp-56],ax
.Lf137:
        jb      .Lf138
        sub     word ptr [bp-56],ax
        sbb     word ptr [bp-54],dx
        or      byte ptr [bp-88],0x1
.Lf138:
        dec     word ptr [bp-8]
        je      .Lf139
        jmp     .Lf131
.Lf139:
        mov     word ptr [bp-72],di
        mov     di,word ptr [bp-112]
        add     di,0x644
        mov     cx,0x4
.Lf140:
        shl     word ptr [bp-88],1
        rcl     word ptr [bp-86],1
        loop    .Lf140
        mov     word ptr [bp-8],0x4
.Lf141:
        mov     word ptr [bp-6],0x1
        mov     si,0x1
.Lf142:
        mov     cx,si
        shl     cx,1
        add     cx,di
        mov     word ptr [bp-2],cx
        cmp     word ptr [bp-58],0x100
        jne     .Lf143
        cmp     word ptr [bp-60],0x0
.Lf143:
        jae     .Lf147
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf144
        cmp     ax,word ptr [bp-64]
.Lf144:
        jne     .Lf145
        jmp     .Lf162
.Lf145:
        mov     word ptr [bp-4],si
        mov     cx,0x8
.Lf146:
        shl     word ptr [bp-60],1
        rcl     word ptr [bp-58],1
        loop    .Lf146
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
.Lf147:
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
        jne     .Lf148
        cmp     ax,word ptr [bp-56]
.Lf148:
        jbe     .Lf149
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
        jmp     .Lf150
.Lf149:
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
.Lf150:
        shl     word ptr [bp-6],1
        dec     word ptr [bp-8]
        je      .Lf151
        jmp     .Lf142
.Lf151:
        jmp     .Lf153
.Lf152:
        mov     ax,si
        test    ax,ax
        cwd
        mov     word ptr [bp-86],dx
        mov     word ptr [bp-88],ax
        mov     word ptr [bp-72],di
.Lf153:
        add     word ptr [bp-88],0x1
        adc     word ptr [bp-86],0x0
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        or      dx,ax
        jne     .Lf154
        jmp     .Lf160
.Lf154:
        add     word ptr [bp-72],0x2
        mov     dx,word ptr [bp-86]
        mov     ax,word ptr [bp-88]
        cmp     dx,word ptr [bp-106]
        jne     .Lf155
        cmp     ax,word ptr [bp-108]
.Lf155:
        jbe     .Lf156
        jmp     .Lf162
.Lf156:
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
.Lf157:
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
        jne     .Lf158
        jmp     .Lf038
.Lf158:
        mov     dx,word ptr [bp-106]
        mov     ax,word ptr [bp-108]
        cmp     dx,word ptr [bp+22]
        jne     .Lf159
        cmp     ax,word ptr [bp+20]
.Lf159:
        jb      .Lf157
        jmp     .Lf038
.Lf160:
        cmp     word ptr [bp-58],0x100
        jne     .Lf161
        cmp     word ptr [bp-60],0x0
.Lf161:
        jae     .Lf164
        mov     dx,word ptr [bp-66]
        mov     ax,word ptr [bp-68]
        cmp     dx,word ptr [bp-62]
        jne     .Lf163
        cmp     ax,word ptr [bp-64]
        jne     .Lf163
.Lf162:
        mov     ax,0x1
        pop     di
        pop     si
        mov     sp,bp
        pop     bp
        ret
.Lf163:
        mov     ax,0x1
        cwd
        add     word ptr [bp-68],ax
        adc     dx,0x0
        mov     cx,__AHSHIFT
        shl     dx,cl
        add     word ptr [bp-66],dx
.Lf164:
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

        push    si
        push    di
        push    bp
        mov     bp,sp
        sub     sp,0x66
        mov     si,word ptr [bp+8]
        lea     bx,[si+4]
        mov     word ptr [bp-34],bx
        xor     ax,ax
        mov     word ptr [bp-20],ax
        mov     word ptr [bp-18],ax
        mov     byte ptr [bp-2],0x0
        mov     cl,byte ptr [si+2]
        mov     ax,0x1
        shl     ax,cl
        dec     ax
        mov     word ptr [bp-88],ax
        mov     cl,byte ptr [si+1]
        mov     ax,0x1
        shl     ax,cl
        dec     ax
        cwd
        mov     word ptr [bp-86],ax
        mov     word ptr [bp-84],dx
        mov     al,byte ptr [si]
        xor     ah,ah
        mov     word ptr [bp-78],ax
        mov     word ptr [bp-16],0x0
        mov     word ptr [bp-102],0x1
        mov     word ptr [bp-100],0x0
        mov     word ptr [bp-52],0x1
        mov     word ptr [bp-50],0x0
        mov     word ptr [bp-56],0x1
        mov     word ptr [bp-54],0x0
        mov     word ptr [bp-68],0x1
        mov     word ptr [bp-74],0x0
        mov     bx,word ptr [bp+18]
        mov     word ptr [bx],0x0
        mov     word ptr [bx+2],0x0
        mov     bx,word ptr [bp+28]
        mov     word ptr [bx],0x0
        mov     word ptr [bx+2],0x0
        mov     dl,byte ptr [si+1]
        xor     dh,dh
        mov     cx,ax
        add     cx,dx
        mov     ax,0x300
        xor     dl,dl
        jcxz    .Lf001
.Lf000:
        shl     ax,1
        rcl     dx,1
        loop    .Lf000
.Lf001:
        mov     si,ax
        add     si,0x736
        mov     cx,dx
        adc     cx,0x0
        xor     dx,dx
        xor     ax,ax
.Lf002:
        cmp     ax,cx
        jb      .Lf003
        jne     .Lf004
        cmp     dx,si
        jae     .Lf004
.Lf003:
        mov     bx,dx
        shl     bx,1
        add     bx,word ptr [bp-34]
        mov     word ptr [bx],0x400
        add     dx,0x1
        adc     ax,0x0
        jmp     .Lf002
.Lf004:
        mov     bx,word ptr [bp+10]
        mov     word ptr [bp-12],bx
        mov     ax,word ptr [bp+12]
        mov     word ptr [bp-10],ax
        mov     ax,bx
        mov     dx,word ptr [bp+12]
        mov     bx,word ptr [bp+14]
        mov     cx,word ptr [bp+16]
        M_PIA
        mov     word ptr [bp-30],ax
        mov     word ptr [bp-28],dx
        xor     ax,ax
        mov     word ptr [bp-6],ax
        mov     word ptr [bp-4],ax
        mov     si,0xffff
        mov     di,si
        mov     word ptr [bp-72],ax
        jmp     .Lf006
.Lf005:
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
        inc     word ptr [bp-72]
        cmp     word ptr [bp-72],0x5
        jge     .Lf008
.Lf006:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf005
.Lf007:
        mov     ax,0x1
        jmp     .Lf139
.Lf008:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Lf011
        je      .Lf010
.Lf009:
        jmp     .Lf136
.Lf010:
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jae     .Lf009
.Lf011:
        mov     ax,word ptr [bp-20]
        and     ax,word ptr [bp-88]
        mov     word ptr [bp-58],ax
        mov     cl,0x5
        mov     dx,word ptr [bp-16]
        shl     dx,cl
        add     dx,word ptr [bp-34]
        shl     ax,1
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,0x100
        jae     .Lf013
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        je      .Lf007
        mov     cx,0x8
.Lf012:
        shl     si,1
        rcl     di,1
        loop    .Lf012
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf013:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf016
        je      .Lf015
.Lf014:
        jmp     .Lf037
.Lf015:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf014
.Lf016:
        mov     word ptr [bp-26],0x1
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     cx,0x8
        sub     cx,word ptr [bp-78]
        mov     al,byte ptr [bp-2]
        xor     ah,ah
        sar     ax,cl
        cwd
        mov     word ptr [bp-90],ax
        mov     bx,dx
        mov     ax,word ptr [bp-20]
        and     ax,word ptr [bp-86]
        mov     dx,word ptr [bp-18]
        and     dx,word ptr [bp-84]
        mov     cx,word ptr [bp-78]
        jcxz    .Lf018
.Lf017:
        shl     ax,1
        rcl     dx,1
        loop    .Lf017
.Lf018:
        add     ax,word ptr [bp-90]
        adc     dx,bx
        mov     bx,0x300
        M_LMUL_dxax_00bx
        shl     ax,1
        rcl     dx,1
        mov     dx,word ptr [bp-34]
        add     dx,0xe6c
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     word ptr [bp-16],0x7
        jge     .Lf019
        jmp     .Lf027
.Lf019:
        mov     bx,word ptr [bp-20]
        sub     bx,word ptr [bp-102]
        mov     cx,word ptr [bp-18]
        sbb     cx,word ptr [bp-100]
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        movw    es,dx
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        mov     word ptr [bp-76],ax
.Lf020:
        shl     word ptr [bp-76],1
        mov     ax,word ptr [bp-76]
        xor     al,al
        and     ah,0x1
        mov     word ptr [bp-66],ax
        shl     ax,1
        mov     dx,word ptr [bp-14]
        add     dh,0x2
        add     dx,ax
        mov     ax,word ptr [bp-26]
        shl     ax,1
        add     dx,ax
        mov     word ptr [bp-46],dx
        cmp     di,0x100
        jae     .Lf023
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf021
        jmp     .Lf007
.Lf021:
        mov     cx,0x8
.Lf022:
        shl     si,1
        rcl     di,1
        loop    .Lf022
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf023:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-46]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf024
        jne     .Lf025
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf025
.Lf024:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-46]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        cmp     word ptr [bp-66],0x0
        jne     .Lf027
        jmp     .Lf026
.Lf025:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-46]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-26]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-26],ax
        cmp     word ptr [bp-66],0x0
        je      .Lf027
.Lf026:
        cmp     word ptr [bp-26],0x100
        jge     .Lf027
        jmp     .Lf020
.Lf027:
        mov     ax,word ptr [bp-26]
        cmp     ax,0x100
        jl      .Lf028
        jmp     .Lf034
.Lf028:
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-40],bx
        cmp     di,0x100
        jae     .Lf031
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf029
        jmp     .Lf007
.Lf029:
        mov     cx,0x8
.Lf030:
        shl     si,1
        rcl     di,1
        loop    .Lf030
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf031:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-40]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf032
        jne     .Lf033
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf033
.Lf032:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-40]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        jmp     .Lf027
.Lf033:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-40]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-26]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-26],ax
        jmp     .Lf027
.Lf034:
        mov     al,byte ptr [bp-26]
        mov     byte ptr [bp-2],al
        mov     bx,word ptr [bp-20]
        mov     cx,word ptr [bp-18]
        add     word ptr [bp-20],0x1
        adc     word ptr [bp-18],0x0
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        movw    es,dx
        mov     al,byte ptr [bp-26]
        mov     byte ptr es:[bx],al
        mov     ax,word ptr [bp-16]
        cmp     ax,0x4
        jge     .Lf035
        xor     ax,ax
        mov     word ptr [bp-16],ax
        jmp     .Lf008
.Lf035:
        cmp     ax,0xa
        jge     .Lf036
        sub     word ptr [bp-16],0x3
        jmp     .Lf008
.Lf036:
        sub     word ptr [bp-16],0x6
        jmp     .Lf008
.Lf037:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-16]
        shl     ax,1
        mov     dx,word ptr [bp-34]
        add     dx,0x180
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,0x100
        jae     .Lf040
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf038
        jmp     .Lf007
.Lf038:
        mov     cx,0x8
.Lf039:
        shl     si,1
        rcl     di,1
        loop    .Lf039
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf040:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf041
        jne     .Lf044
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf044
.Lf041:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-56]
        mov     word ptr [bp-68],ax
        mov     ax,word ptr [bp-54]
        mov     word ptr [bp-74],ax
        mov     ax,word ptr [bp-52]
        mov     word ptr [bp-56],ax
        mov     ax,word ptr [bp-50]
        mov     word ptr [bp-54],ax
        mov     ax,word ptr [bp-102]
        mov     word ptr [bp-52],ax
        mov     ax,word ptr [bp-100]
        mov     word ptr [bp-50],ax
        cmp     word ptr [bp-16],0x7
        jge     .Lf042
        xor     ax,ax
        jmp     .Lf043
.Lf042:
        mov     ax,0x3
.Lf043:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0x664
        jmp     .Lf076
.Lf044:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     dx,word ptr [bp-16]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,0x198
        add     ax,dx
        mov     word ptr [bp-14],ax
        cmp     di,0x100
        jae     .Lf047
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf045
        jmp     .Lf007
.Lf045:
        mov     cx,0x8
.Lf046:
        shl     si,1
        rcl     di,1
        loop    .Lf046
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf047:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        M_PIA1
        mov     word ptr [bp-82],ax
        mov     word ptr [bp-80],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,word ptr [bp-94]
        jb      .Lf050
        je      .Lf049
.Lf048:
        jmp     .Lf060
.Lf049:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf048
.Lf050:
        mov     si,word ptr [bp-8]
        mov     di,word ptr [bp-94]
        mov     ax,0x800
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-16]
        shl     ax,cl
        mov     dx,word ptr [bp-34]
        add     dx,0x1e0
        add     dx,ax
        mov     ax,word ptr [bp-58]
        shl     ax,1
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,0x100
        jae     .Lf053
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf051
        jmp     .Lf007
.Lf051:
        mov     ax,si
        mov     cx,0x8
.Lf052:
        shl     ax,1
        rcl     di,1
        loop    .Lf052
        mov     si,ax
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     bx,word ptr [bp-82]
        mov     word ptr [bp-12],bx
        mov     ax,word ptr [bp-80]
        mov     word ptr [bp-10],ax
.Lf053:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf054
        jne     .Lf056
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf059
.Lf054:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-18]
        or      ax,word ptr [bp-20]
        jne     .Lf055
        jmp     .Lf007
.Lf055:
        cmp     word ptr [bp-16],0x7
        jge     .Lf057
        mov     ax,0x9
        jmp     .Lf058
.Lf056:
        jmp     .Lf059
.Lf057:
        mov     ax,0xb
.Lf058:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-20]
        sub     bx,word ptr [bp-102]
        mov     cx,word ptr [bp-18]
        sbb     cx,word ptr [bp-100]
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        movw    es,dx
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-2],al
        mov     bx,word ptr [bp-20]
        mov     cx,word ptr [bp-18]
        add     word ptr [bp-20],0x1
        adc     word ptr [bp-18],0x0
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        movw    es,dx
        mov     al,byte ptr [bp-2]
        mov     byte ptr es:[bx],al
        jmp     .Lf008
.Lf059:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        jmp     .Lf073
.Lf060:
        sub     si,word ptr [bp-8]
        sbb     di,word ptr [bp-94]
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        mov     ax,word ptr [bp-94]
        sbb     word ptr [bp-4],ax
        mov     cl,0x5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-16]
        shl     ax,1
        mov     dx,word ptr [bp-34]
        add     dx,0x1b0
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,0x100
        jae     .Lf063
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf061
        jmp     .Lf007
.Lf061:
        mov     cx,0x8
.Lf062:
        shl     si,1
        rcl     di,1
        loop    .Lf062
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     bx,word ptr [bp-82]
        mov     word ptr [bp-12],bx
        mov     ax,word ptr [bp-80]
        mov     word ptr [bp-10],ax
.Lf063:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf064
        jne     .Lf065
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf065
.Lf064:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-52]
        mov     cx,word ptr [bp-50]
        jmp     .Lf072
.Lf065:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     dx,word ptr [bp-16]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,0x1c8
        add     ax,dx
        mov     word ptr [bp-14],ax
        cmp     di,0x100
        jae     .Lf068
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf066
        jmp     .Lf007
.Lf066:
        mov     cx,0x8
.Lf067:
        shl     si,1
        rcl     di,1
        loop    .Lf067
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf068:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf069
        jne     .Lf070
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf070
.Lf069:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-56]
        mov     cx,word ptr [bp-54]
        jmp     .Lf071
.Lf070:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-68]
        mov     cx,word ptr [bp-74]
        mov     dx,word ptr [bp-56]
        mov     word ptr [bp-68],dx
        mov     dx,word ptr [bp-54]
        mov     word ptr [bp-74],dx
.Lf071:
        mov     dx,word ptr [bp-52]
        mov     word ptr [bp-56],dx
        mov     dx,word ptr [bp-50]
        mov     word ptr [bp-54],dx
.Lf072:
        mov     dx,word ptr [bp-102]
        mov     word ptr [bp-52],dx
        mov     dx,word ptr [bp-100]
        mov     word ptr [bp-50],dx
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],cx
.Lf073:
        cmp     word ptr [bp-16],0x7
        jge     .Lf074
        mov     ax,0x8
        jmp     .Lf075
.Lf074:
        mov     ax,0xb
.Lf075:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0xa68
.Lf076:
        mov     word ptr [bp-14],bx
        mov     bx,word ptr [bp-14]
        mov     word ptr [bp-22],bx
        cmp     di,0x100
        jae     .Lf079
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf077
        jmp     .Lf007
.Lf077:
        mov     cx,0x8
.Lf078:
        shl     si,1
        rcl     di,1
        loop    .Lf078
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf079:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf080
        jne     .Lf082
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf082
.Lf080:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-22]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     cl,0x4
        mov     ax,word ptr [bp-58]
        shl     ax,cl
        mov     dx,word ptr [bp-14]
        add     dx,0x4
        add     dx,ax
        mov     word ptr [bp-22],dx
        xor     ax,ax
        mov     word ptr [bp-70],ax
.Lf081:
        mov     word ptr [bp-60],0x3
        jmp     .Lf088
.Lf082:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-22]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     bx,word ptr [bp-14]
        inc     bx
        inc     bx
        mov     word ptr [bp-22],bx
        cmp     di,0x100
        jae     .Lf085
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf083
        jmp     .Lf007
.Lf083:
        mov     cx,0x8
.Lf084:
        shl     si,1
        rcl     di,1
        loop    .Lf084
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf085:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf086
        jne     .Lf087
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf087
.Lf086:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-22]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     cl,0x4
        mov     ax,word ptr [bp-58]
        shl     ax,cl
        mov     dx,word ptr [bp-14]
        add     dx,0x104
        add     dx,ax
        mov     word ptr [bp-22],dx
        mov     word ptr [bp-70],0x8
        jmp     .Lf081
.Lf087:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-22]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     bx,word ptr [bp-14]
        add     bx,0x204
        mov     word ptr [bp-22],bx
        mov     word ptr [bp-70],0x10
        mov     word ptr [bp-60],0x8
.Lf088:
        mov     ax,word ptr [bp-60]
        mov     word ptr [bp-62],ax
        mov     word ptr [bp-24],0x1
.Lf089:
        mov     ax,word ptr [bp-24]
        shl     ax,1
        mov     bx,word ptr [bp-22]
        add     bx,ax
        mov     word ptr [bp-38],bx
        cmp     di,0x100
        jae     .Lf092
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf090
        jmp     .Lf007
.Lf090:
        mov     cx,0x8
.Lf091:
        shl     si,1
        rcl     di,1
        loop    .Lf091
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf092:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-38]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf093
        jne     .Lf094
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf094
.Lf093:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-38]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-24],1
        jmp     .Lf095
.Lf094:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-38]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-24]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-24],ax
.Lf095:
        dec     word ptr [bp-62]
        je      .Lf096
        jmp     .Lf089
.Lf096:
        mov     cl,byte ptr [bp-60]
        mov     ax,0x1
        shl     ax,cl
        sub     word ptr [bp-24],ax
        mov     ax,word ptr [bp-70]
        add     word ptr [bp-24],ax
        cmp     word ptr [bp-16],0x4
        jl      .Lf097
        jmp     .Lf130
.Lf097:
        add     word ptr [bp-16],0x7
        mov     ax,word ptr [bp-24]
        cmp     ax,0x4
        jl      .Lf098
        mov     ax,0x3
.Lf098:
        mov     cl,0x7
        mov     dx,ax
        shl     dx,cl
        mov     ax,word ptr [bp-34]
        add     ax,0x360
        add     ax,dx
        mov     word ptr [bp-14],ax
        mov     word ptr [bp-92],0x6
        mov     word ptr [bp-32],0x1
.Lf099:
        mov     ax,word ptr [bp-32]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-42],bx
        cmp     di,0x100
        jae     .Lf102
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf100
        jmp     .Lf007
.Lf100:
        mov     cx,0x8
.Lf101:
        shl     si,1
        rcl     di,1
        loop    .Lf101
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf102:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-42]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf103
        jne     .Lf104
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf104
.Lf103:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-42]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-32],1
        jmp     .Lf105
.Lf104:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-42]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-32]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-32],ax
.Lf105:
        dec     word ptr [bp-92]
        je      .Lf106
        jmp     .Lf099
.Lf106:
        sub     word ptr [bp-32],0x40
        mov     ax,word ptr [bp-32]
        cmp     ax,0x4
        jl      .Lf109
        sar     ax,1
        dec     ax
        mov     word ptr [bp-36],ax
        mov     ax,word ptr [bp-32]
        and     ax,0x1
        or      al,0x2
        mov     word ptr [bp-102],ax
        xor     ax,ax
        mov     word ptr [bp-100],ax
        cmp     word ptr [bp-32],0xe
        jge     .Lf110
        mov     cx,word ptr [bp-36]
        jcxz    .Lf108
.Lf107:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Lf107
.Lf108:
        mov     dx,word ptr [bp-102]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,0x560
        add     dx,ax
        mov     ax,word ptr [bp-32]
        shl     ax,1
        sub     dx,ax
        dec     dx
        dec     dx
        mov     word ptr [bp-14],dx
        jmp     .Lf119
.Lf109:
        jmp     .Lf128
.Lf110:
        sub     word ptr [bp-36],0x4
.Lf111:
        cmp     di,0x100
        jae     .Lf114
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf112
        jmp     .Lf007
.Lf112:
        mov     cx,0x8
.Lf113:
        shl     si,1
        rcl     di,1
        loop    .Lf113
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf114:
        shr     di,1
        rcr     si,1
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        mov     ax,word ptr [bp-4]
        cmp     di,ax
        jb      .Lf115
        jne     .Lf116
        cmp     si,word ptr [bp-6]
        ja      .Lf116
.Lf115:
        sub     word ptr [bp-6],si
        sbb     word ptr [bp-4],di
        or      byte ptr [bp-102],0x1
.Lf116:
        dec     word ptr [bp-36]
        je      .Lf117
        jmp     .Lf111
.Lf117:
        mov     bx,word ptr [bp-34]
        add     bx,0x644
        mov     word ptr [bp-14],bx
        mov     cx,0x4
.Lf118:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Lf118
        mov     word ptr [bp-36],0x4
.Lf119:
        mov     ax,0x1
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-48],ax
.Lf120:
        mov     ax,word ptr [bp-48]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-44],bx
        cmp     di,0x100
        jae     .Lf123
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf121
        jmp     .Lf007
.Lf121:
        mov     cx,0x8
.Lf122:
        shl     si,1
        rcl     di,1
        loop    .Lf122
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0x0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        movw    dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf123:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-44]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf124
        jne     .Lf125
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf125
.Lf124:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-44]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-48],1
        jmp     .Lf126
.Lf125:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,0x5
        mov     bx,word ptr [bp-44]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-48]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-48],ax
        mov     ax,word ptr [bp-64]
        cwd
        or      word ptr [bp-102],ax
        or      word ptr [bp-100],dx
.Lf126:
        shl     word ptr [bp-64],1
        dec     word ptr [bp-36]
        je      .Lf127
        jmp     .Lf120
.Lf127:
        jmp     .Lf129
.Lf128:
        cwd
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],dx
.Lf129:
        add     word ptr [bp-102],0x1
        adc     word ptr [bp-100],0x0
        mov     ax,word ptr [bp-100]
        or      ax,word ptr [bp-102]
        jne     .Lf130
        jmp     .Lf136
.Lf130:
        add     word ptr [bp-24],0x2
        mov     ax,word ptr [bp-100]
        cmp     ax,word ptr [bp-18]
        jbe     .Lf132
.Lf131:
        jmp     .Lf007
.Lf132:
        jne     .Lf133
        mov     ax,word ptr [bp-102]
        cmp     ax,word ptr [bp-20]
        ja      .Lf131
.Lf133:
        mov     bx,word ptr [bp-20]
        sub     bx,word ptr [bp-102]
        mov     cx,word ptr [bp-18]
        sbb     cx,word ptr [bp-100]
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        movw    es,dx
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-2],al
        mov     bx,word ptr [bp-20]
        mov     cx,word ptr [bp-18]
        dec     word ptr [bp-24]
        add     word ptr [bp-20],0x1
        adc     word ptr [bp-18],0x0
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        movw    es,dx
        mov     al,byte ptr [bp-2]
        mov     byte ptr es:[bx],al
        cmp     word ptr [bp-24],0x0
        jne     .Lf135
.Lf134:
        jmp     .Lf008
.Lf135:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Lf133
        jne     .Lf134
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jb      .Lf133
        jmp     .Lf134
.Lf136:
        cmp     di,0x100
        jae     .Lf138
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf137
        jmp     .Lf007
.Lf137:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf138:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp+10]
        mov     cx,word ptr [bp+12]
        M_PTS
        mov     bx,word ptr [bp+18]
        mov     word ptr [bx],ax
        mov     word ptr [bx+2],dx
        mov     ax,word ptr [bp-20]
        mov     bx,word ptr [bp+28]
        mov     word ptr [bx],ax
        mov     ax,word ptr [bp-18]
        mov     word ptr [bx+2],ax
        xor     ax,ax
.Lf139:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
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
        M_PTC
        jne     .Lf005
.Lf007:
        mov     ax,0x1
        jmp     .Lf152
.Lf008:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Lf011
        je      .Lf010
.Lf009:
        jmp     .Lf149
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf013:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf014:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf014
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf017
        je      .Lf016
.Lf015:
        jmp     .Lf040
.Lf016:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf015
.Lf017:
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
        jcxz    .Lf019
.Lf018:
        shl     ax,1
        rcl     dx,1
        loop    .Lf018
.Lf019:
        add     ax,word ptr [bp-90]
        adc     dx,bx
        mov     bx,0x300
        xor     cx,cx
        M_U4M
        shl     ax,1
        rcl     dx,1
        mov     dx,word ptr [bp-34]
        add     dx,0xe6c
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     word ptr [bp-16],0x7
        jge     .Lf020
        jmp     .Lf029
.Lf020:
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
.Lf021:
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
        jae     .Lf024
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf022
        jmp     .Lf007
.Lf022:
        mov     cx,0x8
.Lf023:
        shl     si,1
        rcl     di,1
        loop    .Lf023
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf024:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf025:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf025
        mov     bx,word ptr [bp-46]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf026
        jne     .Lf027
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf027
.Lf026:
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
        jne     .Lf029
        jmp     .Lf028
.Lf027:
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
        je      .Lf029
.Lf028:
        cmp     word ptr [bp-26],0x100
        jge     .Lf029
        jmp     .Lf021
.Lf029:
        mov     ax,word ptr [bp-26]
        cmp     ax,0x100
        jl      .Lf030
        jmp     .Lf037
.Lf030:
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-40],bx
        cmp     di,0x100
        jae     .Lf033
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf031
        jmp     .Lf007
.Lf031:
        mov     cx,0x8
.Lf032:
        shl     si,1
        rcl     di,1
        loop    .Lf032
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf033:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf034:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf034
        mov     bx,word ptr [bp-40]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf035
        jne     .Lf036
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf036
.Lf035:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-40]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        jmp     .Lf029
.Lf036:
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
        jmp     .Lf029
.Lf037:
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
        jge     .Lf038
        xor     ax,ax
        mov     word ptr [bp-16],ax
        jmp     .Lf008
.Lf038:
        cmp     ax,0xa
        jge     .Lf039
        sub     word ptr [bp-16],0x3
        jmp     .Lf008
.Lf039:
        sub     word ptr [bp-16],0x6
        jmp     .Lf008
.Lf040:
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
        jae     .Lf043
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf041
        jmp     .Lf007
.Lf041:
        mov     cx,0x8
.Lf042:
        shl     si,1
        rcl     di,1
        loop    .Lf042
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf043:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf044:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf044
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf045
        jne     .Lf048
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf048
.Lf045:
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
        jge     .Lf046
        xor     ax,ax
        jmp     .Lf047
.Lf046:
        mov     ax,0x3
.Lf047:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0x664
        jmp     .Lf084
.Lf048:
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
        jae     .Lf051
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf049
        jmp     .Lf007
.Lf049:
        mov     cx,0x8
.Lf050:
        shl     si,1
        rcl     di,1
        loop    .Lf050
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf051:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf052:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf052
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-82],ax
        mov     word ptr [bp-80],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,word ptr [bp-94]
        jb      .Lf055
        je      .Lf054
.Lf053:
        jmp     .Lf066
.Lf054:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf053
.Lf055:
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
        jae     .Lf058
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf056
        jmp     .Lf007
.Lf056:
        mov     ax,si
        mov     cx,0x8
.Lf057:
        shl     ax,1
        rcl     di,1
        loop    .Lf057
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
.Lf058:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf059:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf059
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf060
        jne     .Lf062
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf065
.Lf060:
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
        jne     .Lf061
        jmp     .Lf007
.Lf061:
        cmp     word ptr [bp-16],0x7
        jge     .Lf063
        mov     ax,0x9
        jmp     .Lf064
.Lf062:
        jmp     .Lf065
.Lf063:
        mov     ax,0xb
.Lf064:
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
        jmp     .Lf081
.Lf066:
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
        jae     .Lf069
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf067
        jmp     .Lf007
.Lf067:
        mov     cx,0x8
.Lf068:
        shl     si,1
        rcl     di,1
        loop    .Lf068
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
.Lf069:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf070:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf070
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf071
        jne     .Lf072
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf072
.Lf071:
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
        jmp     .Lf080
.Lf072:
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
        jae     .Lf075
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf073
        jmp     .Lf007
.Lf073:
        mov     cx,0x8
.Lf074:
        shl     si,1
        rcl     di,1
        loop    .Lf074
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf075:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf076:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf076
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf077
        jne     .Lf078
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf078
.Lf077:
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
        jmp     .Lf079
.Lf078:
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
.Lf079:
        mov     dx,word ptr [bp-52]
        mov     word ptr [bp-56],dx
        mov     dx,word ptr [bp-50]
        mov     word ptr [bp-54],dx
.Lf080:
        mov     dx,word ptr [bp-102]
        mov     word ptr [bp-52],dx
        mov     dx,word ptr [bp-100]
        mov     word ptr [bp-50],dx
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],cx
.Lf081:
        cmp     word ptr [bp-16],0x7
        jge     .Lf082
        mov     ax,0x8
        jmp     .Lf083
.Lf082:
        mov     ax,0xb
.Lf083:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0xa68
.Lf084:
        mov     word ptr [bp-14],bx
        mov     bx,word ptr [bp-14]
        mov     word ptr [bp-22],bx
        cmp     di,0x100
        jae     .Lf087
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf085
        jmp     .Lf007
.Lf085:
        mov     cx,0x8
.Lf086:
        shl     si,1
        rcl     di,1
        loop    .Lf086
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf087:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf088:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf088
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf089
        jne     .Lf091
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf091
.Lf089:
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
.Lf090:
        mov     word ptr [bp-60],0x3
        jmp     .Lf098
.Lf091:
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
        jae     .Lf094
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf092
        jmp     .Lf007
.Lf092:
        mov     cx,0x8
.Lf093:
        shl     si,1
        rcl     di,1
        loop    .Lf093
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf094:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf095:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf095
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf096
        jne     .Lf097
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf097
.Lf096:
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
        jmp     .Lf090
.Lf097:
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
.Lf098:
        mov     ax,word ptr [bp-60]
        mov     word ptr [bp-62],ax
        mov     word ptr [bp-24],0x1
.Lf099:
        mov     ax,word ptr [bp-24]
        shl     ax,1
        mov     bx,word ptr [bp-22]
        add     bx,ax
        mov     word ptr [bp-38],bx
        cmp     di,0x100
        jae     .Lf102
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf100
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf102:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf103:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf103
        mov     bx,word ptr [bp-38]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf104
        jne     .Lf105
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf105
.Lf104:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-38]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-24],1
        jmp     .Lf106
.Lf105:
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
.Lf106:
        dec     word ptr [bp-62]
        je      .Lf107
        jmp     .Lf099
.Lf107:
        mov     cl,byte ptr [bp-60]
        mov     ax,0x1
        shl     ax,cl
        sub     word ptr [bp-24],ax
        mov     ax,word ptr [bp-70]
        add     word ptr [bp-24],ax
        cmp     word ptr [bp-16],0x4
        jl      .Lf108
        jmp     .Lf143
.Lf108:
        add     word ptr [bp-16],0x7
        mov     ax,word ptr [bp-24]
        cmp     ax,0x4
        jl      .Lf109
        mov     ax,0x3
.Lf109:
        mov     cl,0x7
        mov     dx,ax
        shl     dx,cl
        mov     ax,word ptr [bp-34]
        add     ax,0x360
        add     ax,dx
        mov     word ptr [bp-14],ax
        mov     word ptr [bp-92],0x6
        mov     word ptr [bp-32],0x1
.Lf110:
        mov     ax,word ptr [bp-32]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-42],bx
        cmp     di,0x100
        jae     .Lf113
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf111
        jmp     .Lf007
.Lf111:
        mov     cx,0x8
.Lf112:
        shl     si,1
        rcl     di,1
        loop    .Lf112
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf113:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf114:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf114
        mov     bx,word ptr [bp-42]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf115
        jne     .Lf116
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf116
.Lf115:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-42]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-32],1
        jmp     .Lf117
.Lf116:
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
.Lf117:
        dec     word ptr [bp-92]
        je      .Lf118
        jmp     .Lf110
.Lf118:
        sub     word ptr [bp-32],0x40
        mov     ax,word ptr [bp-32]
        cmp     ax,0x4
        jl      .Lf121
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
        jge     .Lf122
        mov     cx,word ptr [bp-36]
        jcxz    .Lf120
.Lf119:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Lf119
.Lf120:
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
        jmp     .Lf131
.Lf121:
        jmp     .Lf141
.Lf122:
        sub     word ptr [bp-36],0x4
.Lf123:
        cmp     di,0x100
        jae     .Lf126
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf124
        jmp     .Lf007
.Lf124:
        mov     cx,0x8
.Lf125:
        shl     si,1
        rcl     di,1
        loop    .Lf125
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf126:
        shr     di,1
        rcr     si,1
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        mov     ax,word ptr [bp-4]
        cmp     di,ax
        jb      .Lf127
        jne     .Lf128
        cmp     si,word ptr [bp-6]
        ja      .Lf128
.Lf127:
        sub     word ptr [bp-6],si
        sbb     word ptr [bp-4],di
        or      byte ptr [bp-102],0x1
.Lf128:
        dec     word ptr [bp-36]
        je      .Lf129
        jmp     .Lf123
.Lf129:
        mov     bx,word ptr [bp-34]
        add     bx,0x644
        mov     word ptr [bp-14],bx
        mov     cx,0x4
.Lf130:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Lf130
        mov     word ptr [bp-36],0x4
.Lf131:
        mov     ax,0x1
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-48],ax
.Lf132:
        mov     ax,word ptr [bp-48]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-44],bx
        cmp     di,0x100
        jae     .Lf135
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf133
        jmp     .Lf007
.Lf133:
        mov     cx,0x8
.Lf134:
        shl     si,1
        rcl     di,1
        loop    .Lf134
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
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf135:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Lf136:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Lf136
        mov     bx,word ptr [bp-44]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        xor     cx,cx
        M_U4M
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf137
        jne     .Lf138
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf138
.Lf137:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-44]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-48],1
        jmp     .Lf139
.Lf138:
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
.Lf139:
        shl     word ptr [bp-64],1
        dec     word ptr [bp-36]
        je      .Lf140
        jmp     .Lf132
.Lf140:
        jmp     .Lf142
.Lf141:
        cwd
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],dx
.Lf142:
        add     word ptr [bp-102],0x1
        adc     word ptr [bp-100],0x0
        mov     ax,word ptr [bp-100]
        or      ax,word ptr [bp-102]
        jne     .Lf143
        jmp     .Lf149
.Lf143:
        add     word ptr [bp-24],0x2
        mov     ax,word ptr [bp-100]
        cmp     ax,word ptr [bp-18]
        jbe     .Lf145
.Lf144:
        jmp     .Lf007
.Lf145:
        jne     .Lf146
        mov     ax,word ptr [bp-102]
        cmp     ax,word ptr [bp-20]
        ja      .Lf144
.Lf146:
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
        jne     .Lf148
.Lf147:
        jmp     .Lf008
.Lf148:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Lf146
        jne     .Lf147
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jb      .Lf146
        jmp     .Lf147
.Lf149:
        cmp     di,0x100
        jae     .Lf151
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Lf150
        jmp     .Lf007
.Lf150:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,0x1
        xor     cx,cx
        M_PIA
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf151:
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
.Lf152:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

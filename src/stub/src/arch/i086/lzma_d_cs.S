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
        jcxz    .Ls001
.Ls000:
        shl     ax,1
        rcl     dx,1
        loop    .Ls000
.Ls001:
        mov     si,ax
        add     si,0x736
        mov     cx,dx
        adc     cx,0x0
        xor     dx,dx
        xor     ax,ax
.Ls002:
        cmp     ax,cx
        jb      .Ls003
        jne     .Ls004
        cmp     dx,si
        jae     .Ls004
.Ls003:
        mov     bx,dx
        shl     bx,1
        add     bx,word ptr [bp-34]
        mov     word ptr [bx],0x400
        add     dx,0x1
        adc     ax,0x0
        jmp     .Ls002
.Ls004:
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
        jmp     .Ls006
.Ls005:
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
        jge     .Ls008
.Ls006:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls005
.Ls007:
        mov     ax,0x1
        jmp     .Ls152
.Ls008:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Ls011
        je      .Ls010
.Ls009:
        jmp     .Ls149
.Ls010:
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jae     .Ls009
.Ls011:
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
        jae     .Ls013
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        je      .Ls007
        mov     cx,0x8
.Ls012:
        shl     si,1
        rcl     di,1
        loop    .Ls012
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
.Ls013:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls014:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls014
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls017
        je      .Ls016
.Ls015:
        jmp     .Ls040
.Ls016:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls015
.Ls017:
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
        jcxz    .Ls019
.Ls018:
        shl     ax,1
        rcl     dx,1
        loop    .Ls018
.Ls019:
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
        jge     .Ls020
        jmp     .Ls029
.Ls020:
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
.Ls021:
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
        jae     .Ls024
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls022
        jmp     .Ls007
.Ls022:
        mov     cx,0x8
.Ls023:
        shl     si,1
        rcl     di,1
        loop    .Ls023
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
.Ls024:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls025:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls025
        mov     bx,word ptr [bp-46]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls026
        jne     .Ls027
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls027
.Ls026:
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
        jne     .Ls029
        jmp     .Ls028
.Ls027:
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
        je      .Ls029
.Ls028:
        cmp     word ptr [bp-26],0x100
        jge     .Ls029
        jmp     .Ls021
.Ls029:
        mov     ax,word ptr [bp-26]
        cmp     ax,0x100
        jl      .Ls030
        jmp     .Ls037
.Ls030:
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-40],bx
        cmp     di,0x100
        jae     .Ls033
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls031
        jmp     .Ls007
.Ls031:
        mov     cx,0x8
.Ls032:
        shl     si,1
        rcl     di,1
        loop    .Ls032
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
.Ls033:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls034:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls034
        mov     bx,word ptr [bp-40]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls035
        jne     .Ls036
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls036
.Ls035:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-40]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        jmp     .Ls029
.Ls036:
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
        jmp     .Ls029
.Ls037:
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
        jge     .Ls038
        xor     ax,ax
        mov     word ptr [bp-16],ax
        jmp     .Ls008
.Ls038:
        cmp     ax,0xa
        jge     .Ls039
        sub     word ptr [bp-16],0x3
        jmp     .Ls008
.Ls039:
        sub     word ptr [bp-16],0x6
        jmp     .Ls008
.Ls040:
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
        jae     .Ls043
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls041
        jmp     .Ls007
.Ls041:
        mov     cx,0x8
.Ls042:
        shl     si,1
        rcl     di,1
        loop    .Ls042
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
.Ls043:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls044:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls044
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls045
        jne     .Ls048
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls048
.Ls045:
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
        jge     .Ls046
        xor     ax,ax
        jmp     .Ls047
.Ls046:
        mov     ax,0x3
.Ls047:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0x664
        jmp     .Ls084
.Ls048:
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
        jae     .Ls051
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls049
        jmp     .Ls007
.Ls049:
        mov     cx,0x8
.Ls050:
        shl     si,1
        rcl     di,1
        loop    .Ls050
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
.Ls051:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls052:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls052
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
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
        jb      .Ls055
        je      .Ls054
.Ls053:
        jmp     .Ls066
.Ls054:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls053
.Ls055:
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
        jae     .Ls058
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls056
        jmp     .Ls007
.Ls056:
        mov     ax,si
        mov     cx,0x8
.Ls057:
        shl     ax,1
        rcl     di,1
        loop    .Ls057
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
.Ls058:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls059:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls059
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls060
        jne     .Ls062
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls065
.Ls060:
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
        jne     .Ls061
        jmp     .Ls007
.Ls061:
        cmp     word ptr [bp-16],0x7
        jge     .Ls063
        mov     ax,0x9
        jmp     .Ls064
.Ls062:
        jmp     .Ls065
.Ls063:
        mov     ax,0xb
.Ls064:
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
        jmp     .Ls008
.Ls065:
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
        jmp     .Ls081
.Ls066:
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
        jae     .Ls069
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls067
        jmp     .Ls007
.Ls067:
        mov     cx,0x8
.Ls068:
        shl     si,1
        rcl     di,1
        loop    .Ls068
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
.Ls069:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls070:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls070
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls071
        jne     .Ls072
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls072
.Ls071:
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
        jmp     .Ls080
.Ls072:
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
        jae     .Ls075
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls073
        jmp     .Ls007
.Ls073:
        mov     cx,0x8
.Ls074:
        shl     si,1
        rcl     di,1
        loop    .Ls074
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
.Ls075:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls076:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls076
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls077
        jne     .Ls078
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls078
.Ls077:
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
        jmp     .Ls079
.Ls078:
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
.Ls079:
        mov     dx,word ptr [bp-52]
        mov     word ptr [bp-56],dx
        mov     dx,word ptr [bp-50]
        mov     word ptr [bp-54],dx
.Ls080:
        mov     dx,word ptr [bp-102]
        mov     word ptr [bp-52],dx
        mov     dx,word ptr [bp-100]
        mov     word ptr [bp-50],dx
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],cx
.Ls081:
        cmp     word ptr [bp-16],0x7
        jge     .Ls082
        mov     ax,0x8
        jmp     .Ls083
.Ls082:
        mov     ax,0xb
.Ls083:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0xa68
.Ls084:
        mov     word ptr [bp-14],bx
        mov     bx,word ptr [bp-14]
        mov     word ptr [bp-22],bx
        cmp     di,0x100
        jae     .Ls087
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls085
        jmp     .Ls007
.Ls085:
        mov     cx,0x8
.Ls086:
        shl     si,1
        rcl     di,1
        loop    .Ls086
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
.Ls087:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls088:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls088
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls089
        jne     .Ls091
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls091
.Ls089:
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
.Ls090:
        mov     word ptr [bp-60],0x3
        jmp     .Ls098
.Ls091:
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
        jae     .Ls094
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls092
        jmp     .Ls007
.Ls092:
        mov     cx,0x8
.Ls093:
        shl     si,1
        rcl     di,1
        loop    .Ls093
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
.Ls094:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls095:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls095
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls096
        jne     .Ls097
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls097
.Ls096:
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
        jmp     .Ls090
.Ls097:
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
.Ls098:
        mov     ax,word ptr [bp-60]
        mov     word ptr [bp-62],ax
        mov     word ptr [bp-24],0x1
.Ls099:
        mov     ax,word ptr [bp-24]
        shl     ax,1
        mov     bx,word ptr [bp-22]
        add     bx,ax
        mov     word ptr [bp-38],bx
        cmp     di,0x100
        jae     .Ls102
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls100
        jmp     .Ls007
.Ls100:
        mov     cx,0x8
.Ls101:
        shl     si,1
        rcl     di,1
        loop    .Ls101
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
.Ls102:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls103:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls103
        mov     bx,word ptr [bp-38]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls104
        jne     .Ls105
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls105
.Ls104:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-38]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-24],1
        jmp     .Ls106
.Ls105:
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
.Ls106:
        dec     word ptr [bp-62]
        je      .Ls107
        jmp     .Ls099
.Ls107:
        mov     cl,byte ptr [bp-60]
        mov     ax,0x1
        shl     ax,cl
        sub     word ptr [bp-24],ax
        mov     ax,word ptr [bp-70]
        add     word ptr [bp-24],ax
        cmp     word ptr [bp-16],0x4
        jl      .Ls108
        jmp     .Ls143
.Ls108:
        add     word ptr [bp-16],0x7
        mov     ax,word ptr [bp-24]
        cmp     ax,0x4
        jl      .Ls109
        mov     ax,0x3
.Ls109:
        mov     cl,0x7
        mov     dx,ax
        shl     dx,cl
        mov     ax,word ptr [bp-34]
        add     ax,0x360
        add     ax,dx
        mov     word ptr [bp-14],ax
        mov     word ptr [bp-92],0x6
        mov     word ptr [bp-32],0x1
.Ls110:
        mov     ax,word ptr [bp-32]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-42],bx
        cmp     di,0x100
        jae     .Ls113
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls111
        jmp     .Ls007
.Ls111:
        mov     cx,0x8
.Ls112:
        shl     si,1
        rcl     di,1
        loop    .Ls112
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
.Ls113:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls114:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls114
        mov     bx,word ptr [bp-42]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls115
        jne     .Ls116
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls116
.Ls115:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-42]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-32],1
        jmp     .Ls117
.Ls116:
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
.Ls117:
        dec     word ptr [bp-92]
        je      .Ls118
        jmp     .Ls110
.Ls118:
        sub     word ptr [bp-32],0x40
        mov     ax,word ptr [bp-32]
        cmp     ax,0x4
        jl      .Ls121
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
        jge     .Ls122
        mov     cx,word ptr [bp-36]
        jcxz    .Ls120
.Ls119:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Ls119
.Ls120:
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
        jmp     .Ls131
.Ls121:
        jmp     .Ls141
.Ls122:
        sub     word ptr [bp-36],0x4
.Ls123:
        cmp     di,0x100
        jae     .Ls126
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls124
        jmp     .Ls007
.Ls124:
        mov     cx,0x8
.Ls125:
        shl     si,1
        rcl     di,1
        loop    .Ls125
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
.Ls126:
        shr     di,1
        rcr     si,1
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        mov     ax,word ptr [bp-4]
        cmp     di,ax
        jb      .Ls127
        jne     .Ls128
        cmp     si,word ptr [bp-6]
        ja      .Ls128
.Ls127:
        sub     word ptr [bp-6],si
        sbb     word ptr [bp-4],di
        or      byte ptr [bp-102],0x1
.Ls128:
        dec     word ptr [bp-36]
        je      .Ls129
        jmp     .Ls123
.Ls129:
        mov     bx,word ptr [bp-34]
        add     bx,0x644
        mov     word ptr [bp-14],bx
        mov     cx,0x4
.Ls130:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Ls130
        mov     word ptr [bp-36],0x4
.Ls131:
        mov     ax,0x1
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-48],ax
.Ls132:
        mov     ax,word ptr [bp-48]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-44],bx
        cmp     di,0x100
        jae     .Ls135
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls133
        jmp     .Ls007
.Ls133:
        mov     cx,0x8
.Ls134:
        shl     si,1
        rcl     di,1
        loop    .Ls134
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
.Ls135:
        mov     word ptr [bp-98],si
        mov     word ptr [bp-96],di
        mov     cx,0xb
.Ls136:
        shr     word ptr [bp-96],1
        rcr     word ptr [bp-98],1
        loop    .Ls136
        mov     bx,word ptr [bp-44]
        mov     bx,word ptr [bx]
        mov     ax,word ptr [bp-98]
        mov     dx,word ptr [bp-96]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls137
        jne     .Ls138
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls138
.Ls137:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-44]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-48],1
        jmp     .Ls139
.Ls138:
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
.Ls139:
        shl     word ptr [bp-64],1
        dec     word ptr [bp-36]
        je      .Ls140
        jmp     .Ls132
.Ls140:
        jmp     .Ls142
.Ls141:
        cwd
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],dx
.Ls142:
        add     word ptr [bp-102],0x1
        adc     word ptr [bp-100],0x0
        mov     ax,word ptr [bp-100]
        or      ax,word ptr [bp-102]
        jne     .Ls143
        jmp     .Ls149
.Ls143:
        add     word ptr [bp-24],0x2
        mov     ax,word ptr [bp-100]
        cmp     ax,word ptr [bp-18]
        jbe     .Ls145
.Ls144:
        jmp     .Ls007
.Ls145:
        jne     .Ls146
        mov     ax,word ptr [bp-102]
        cmp     ax,word ptr [bp-20]
        ja      .Ls144
.Ls146:
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
        jne     .Ls148
.Ls147:
        jmp     .Ls008
.Ls148:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Ls146
        jne     .Ls147
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jb      .Ls146
        jmp     .Ls147
.Ls149:
        cmp     di,0x100
        jae     .Ls151
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        jne     .Ls150
        jmp     .Ls007
.Ls150:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Ls151:
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
.Ls152:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

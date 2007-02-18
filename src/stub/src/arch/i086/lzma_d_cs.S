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
        M_PTC_JNE .Ls005
.Ls007:
        mov     ax,0x1
        jmp     .Ls139
.Ls008:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Ls011
        je      .Ls010
.Ls009:
        jmp     .Ls136
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
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls016
        je      .Ls015
.Ls014:
        jmp     .Ls037
.Ls015:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls014
.Ls016:
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
        jcxz    .Ls018
.Ls017:
        shl     ax,1
        rcl     dx,1
        loop    .Ls017
.Ls018:
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
        jge     .Ls019
        jmp     .Ls027
.Ls019:
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
.Ls020:
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
        jae     .Ls023
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls021
        jmp     .Ls007
.Ls021:
        mov     cx,0x8
.Ls022:
        shl     si,1
        rcl     di,1
        loop    .Ls022
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
.Ls023:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-46]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls024
        jne     .Ls025
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls025
.Ls024:
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
        jne     .Ls027
        jmp     .Ls026
.Ls025:
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
        je      .Ls027
.Ls026:
        cmp     word ptr [bp-26],0x100
        jge     .Ls027
        jmp     .Ls020
.Ls027:
        mov     ax,word ptr [bp-26]
        cmp     ax,0x100
        jl      .Ls028
        jmp     .Ls034
.Ls028:
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-40],bx
        cmp     di,0x100
        jae     .Ls031
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls029
        jmp     .Ls007
.Ls029:
        mov     cx,0x8
.Ls030:
        shl     si,1
        rcl     di,1
        loop    .Ls030
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
.Ls031:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-40]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls032
        jne     .Ls033
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls033
.Ls032:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-40]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        jmp     .Ls027
.Ls033:
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
        jmp     .Ls027
.Ls034:
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
        jge     .Ls035
        xor     ax,ax
        mov     word ptr [bp-16],ax
        jmp     .Ls008
.Ls035:
        cmp     ax,0xa
        jge     .Ls036
        sub     word ptr [bp-16],0x3
        jmp     .Ls008
.Ls036:
        sub     word ptr [bp-16],0x6
        jmp     .Ls008
.Ls037:
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
        jae     .Ls040
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls038
        jmp     .Ls007
.Ls038:
        mov     cx,0x8
.Ls039:
        shl     si,1
        rcl     di,1
        loop    .Ls039
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
.Ls040:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls041
        jne     .Ls044
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls044
.Ls041:
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
        jge     .Ls042
        xor     ax,ax
        jmp     .Ls043
.Ls042:
        mov     ax,0x3
.Ls043:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0x664
        jmp     .Ls076
.Ls044:
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
        jae     .Ls047
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls045
        jmp     .Ls007
.Ls045:
        mov     cx,0x8
.Ls046:
        shl     si,1
        rcl     di,1
        loop    .Ls046
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
.Ls047:
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
        jb      .Ls050
        je      .Ls049
.Ls048:
        jmp     .Ls060
.Ls049:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls048
.Ls050:
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
        jae     .Ls053
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls051
        jmp     .Ls007
.Ls051:
        mov     ax,si
        mov     cx,0x8
.Ls052:
        shl     ax,1
        rcl     di,1
        loop    .Ls052
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
.Ls053:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls054
        jne     .Ls056
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls059
.Ls054:
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
        jne     .Ls055
        jmp     .Ls007
.Ls055:
        cmp     word ptr [bp-16],0x7
        jge     .Ls057
        mov     ax,0x9
        jmp     .Ls058
.Ls056:
        jmp     .Ls059
.Ls057:
        mov     ax,0xb
.Ls058:
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
.Ls059:
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
        jmp     .Ls073
.Ls060:
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
        jae     .Ls063
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls061
        jmp     .Ls007
.Ls061:
        mov     cx,0x8
.Ls062:
        shl     si,1
        rcl     di,1
        loop    .Ls062
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
.Ls063:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls064
        jne     .Ls065
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls065
.Ls064:
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
        jmp     .Ls072
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
        mov     dx,word ptr [bp-16]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,0x1c8
        add     ax,dx
        mov     word ptr [bp-14],ax
        cmp     di,0x100
        jae     .Ls068
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls066
        jmp     .Ls007
.Ls066:
        mov     cx,0x8
.Ls067:
        shl     si,1
        rcl     di,1
        loop    .Ls067
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
.Ls068:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls069
        jne     .Ls070
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls070
.Ls069:
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
        jmp     .Ls071
.Ls070:
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
.Ls071:
        mov     dx,word ptr [bp-52]
        mov     word ptr [bp-56],dx
        mov     dx,word ptr [bp-50]
        mov     word ptr [bp-54],dx
.Ls072:
        mov     dx,word ptr [bp-102]
        mov     word ptr [bp-52],dx
        mov     dx,word ptr [bp-100]
        mov     word ptr [bp-50],dx
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],cx
.Ls073:
        cmp     word ptr [bp-16],0x7
        jge     .Ls074
        mov     ax,0x8
        jmp     .Ls075
.Ls074:
        mov     ax,0xb
.Ls075:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,0xa68
.Ls076:
        mov     word ptr [bp-14],bx
        mov     bx,word ptr [bp-14]
        mov     word ptr [bp-22],bx
        cmp     di,0x100
        jae     .Ls079
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls077
        jmp     .Ls007
.Ls077:
        mov     cx,0x8
.Ls078:
        shl     si,1
        rcl     di,1
        loop    .Ls078
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
.Ls079:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls080
        jne     .Ls082
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls082
.Ls080:
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
.Ls081:
        mov     word ptr [bp-60],0x3
        jmp     .Ls088
.Ls082:
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
        jae     .Ls085
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls083
        jmp     .Ls007
.Ls083:
        mov     cx,0x8
.Ls084:
        shl     si,1
        rcl     di,1
        loop    .Ls084
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
.Ls085:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls086
        jne     .Ls087
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls087
.Ls086:
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
        jmp     .Ls081
.Ls087:
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
.Ls088:
        mov     ax,word ptr [bp-60]
        mov     word ptr [bp-62],ax
        mov     word ptr [bp-24],0x1
.Ls089:
        mov     ax,word ptr [bp-24]
        shl     ax,1
        mov     bx,word ptr [bp-22]
        add     bx,ax
        mov     word ptr [bp-38],bx
        cmp     di,0x100
        jae     .Ls092
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls090
        jmp     .Ls007
.Ls090:
        mov     cx,0x8
.Ls091:
        shl     si,1
        rcl     di,1
        loop    .Ls091
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
.Ls092:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-38]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls093
        jne     .Ls094
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls094
.Ls093:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-38]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-24],1
        jmp     .Ls095
.Ls094:
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
.Ls095:
        dec     word ptr [bp-62]
        je      .Ls096
        jmp     .Ls089
.Ls096:
        mov     cl,byte ptr [bp-60]
        mov     ax,0x1
        shl     ax,cl
        sub     word ptr [bp-24],ax
        mov     ax,word ptr [bp-70]
        add     word ptr [bp-24],ax
        cmp     word ptr [bp-16],0x4
        jl      .Ls097
        jmp     .Ls130
.Ls097:
        add     word ptr [bp-16],0x7
        mov     ax,word ptr [bp-24]
        cmp     ax,0x4
        jl      .Ls098
        mov     ax,0x3
.Ls098:
        mov     cl,0x7
        mov     dx,ax
        shl     dx,cl
        mov     ax,word ptr [bp-34]
        add     ax,0x360
        add     ax,dx
        mov     word ptr [bp-14],ax
        mov     word ptr [bp-92],0x6
        mov     word ptr [bp-32],0x1
.Ls099:
        mov     ax,word ptr [bp-32]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-42],bx
        cmp     di,0x100
        jae     .Ls102
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls100
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
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-42]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls103
        jne     .Ls104
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls104
.Ls103:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-42]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-32],1
        jmp     .Ls105
.Ls104:
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
.Ls105:
        dec     word ptr [bp-92]
        je      .Ls106
        jmp     .Ls099
.Ls106:
        sub     word ptr [bp-32],0x40
        mov     ax,word ptr [bp-32]
        cmp     ax,0x4
        jl      .Ls109
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
        jge     .Ls110
        mov     cx,word ptr [bp-36]
        jcxz    .Ls108
.Ls107:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Ls107
.Ls108:
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
        jmp     .Ls119
.Ls109:
        jmp     .Ls128
.Ls110:
        sub     word ptr [bp-36],0x4
.Ls111:
        cmp     di,0x100
        jae     .Ls114
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls112
        jmp     .Ls007
.Ls112:
        mov     cx,0x8
.Ls113:
        shl     si,1
        rcl     di,1
        loop    .Ls113
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
.Ls114:
        shr     di,1
        rcr     si,1
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        mov     ax,word ptr [bp-4]
        cmp     di,ax
        jb      .Ls115
        jne     .Ls116
        cmp     si,word ptr [bp-6]
        ja      .Ls116
.Ls115:
        sub     word ptr [bp-6],si
        sbb     word ptr [bp-4],di
        or      byte ptr [bp-102],0x1
.Ls116:
        dec     word ptr [bp-36]
        je      .Ls117
        jmp     .Ls111
.Ls117:
        mov     bx,word ptr [bp-34]
        add     bx,0x644
        mov     word ptr [bp-14],bx
        mov     cx,0x4
.Ls118:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Ls118
        mov     word ptr [bp-36],0x4
.Ls119:
        mov     ax,0x1
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-48],ax
.Ls120:
        mov     ax,word ptr [bp-48]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-44],bx
        cmp     di,0x100
        jae     .Ls123
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls121
        jmp     .Ls007
.Ls121:
        mov     cx,0x8
.Ls122:
        shl     si,1
        rcl     di,1
        loop    .Ls122
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
.Ls123:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-44]
        mov     bx,word ptr [bx]
        M_LMUL_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls124
        jne     .Ls125
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls125
.Ls124:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,0x800
        mov     bx,word ptr [bp-44]
        sub     ax,word ptr [bx]
        mov     cl,0x5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-48],1
        jmp     .Ls126
.Ls125:
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
.Ls126:
        shl     word ptr [bp-64],1
        dec     word ptr [bp-36]
        je      .Ls127
        jmp     .Ls120
.Ls127:
        jmp     .Ls129
.Ls128:
        cwd
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],dx
.Ls129:
        add     word ptr [bp-102],0x1
        adc     word ptr [bp-100],0x0
        mov     ax,word ptr [bp-100]
        or      ax,word ptr [bp-102]
        jne     .Ls130
        jmp     .Ls136
.Ls130:
        add     word ptr [bp-24],0x2
        mov     ax,word ptr [bp-100]
        cmp     ax,word ptr [bp-18]
        jbe     .Ls132
.Ls131:
        jmp     .Ls007
.Ls132:
        jne     .Ls133
        mov     ax,word ptr [bp-102]
        cmp     ax,word ptr [bp-20]
        ja      .Ls131
.Ls133:
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
        jne     .Ls135
.Ls134:
        jmp     .Ls008
.Ls135:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Ls133
        jne     .Ls134
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jb      .Ls133
        jmp     .Ls134
.Ls136:
        cmp     di,0x100
        jae     .Ls138
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls137
        jmp     .Ls007
.Ls137:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Ls138:
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
.Ls139:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

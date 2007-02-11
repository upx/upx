        push    si
        push    di
        push    bp
        mov     bp,sp
        sub     sp,0x6e
        mov     si,ax
        movw    ds,dx
        mov     WORD PTR [bp-82],bx
        mov     WORD PTR [bp-76],cx
        mov     WORD PTR [bp-36],dx
        add     ax,0x4
        mov     WORD PTR [bp-38],ax
        xor     ax,ax
        mov     WORD PTR [bp-24],ax
        mov     WORD PTR [bp-20],ax
        mov     BYTE PTR [bp-2],0x0
        mov     cl,BYTE PTR [si+2]
        mov     ax,0x1
        shl     ax,cl
        dec     ax
        mov     WORD PTR [bp-98],ax
        mov     cl,BYTE PTR [si+1]
        mov     ax,0x1
        shl     ax,cl
        dec     ax
        cwd
        mov     WORD PTR [bp-96],ax
        mov     WORD PTR [bp-94],dx
        mov     al,BYTE PTR [si]
        xor     ah,ah
        mov     WORD PTR [bp-88],ax
        mov     WORD PTR [bp-18],0x0
        mov     WORD PTR [bp-110],0x1
        mov     WORD PTR [bp-108],0x0
        mov     WORD PTR [bp-60],0x1
        mov     WORD PTR [bp-58],0x0
        mov     WORD PTR [bp-56],0x1
        mov     WORD PTR [bp-54],0x0
        mov     WORD PTR [bp-84],0x1
        mov     WORD PTR [bp-68],0x0
        les     bx,DWORD PTR [bp+12]
        mov     WORD PTR es:[bx],0x0
        mov     WORD PTR es:[bx+2],0x0
        les     bx,DWORD PTR [bp+24]
        mov     WORD PTR es:[bx],0x0
        mov     WORD PTR es:[bx+2],0x0
        mov     dl,BYTE PTR [si+1]
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
        xor     ax,ax
        xor     dx,dx
        movw    ds,WORD PTR [bp-36]
.Ls002:
        cmp     dx,cx
        jb      .Ls003
        jne     .Ls004
        cmp     ax,si
        jae     .Ls004
.Ls003:
        mov     bx,ax
        shl     bx,1
        add     bx,WORD PTR [bp-38]
        mov     WORD PTR [bx],0x400
        add     ax,0x1
        adc     dx,0x0
        jmp     .Ls002
.Ls004:
        mov     bx,WORD PTR [bp-82]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-76]
        mov     ax,bx
        movw    dx,ds
        mov     bx,WORD PTR [bp+8]
        mov     cx,WORD PTR [bp+10]
        call    __PIA
        mov     WORD PTR [bp-30],ax
        mov     WORD PTR [bp-28],dx
        xor     ax,ax
        mov     WORD PTR [bp-8],ax
        mov     WORD PTR [bp-4],ax
        mov     si,0xffff
        mov     di,si
        mov     WORD PTR [bp-74],ax
        jmp     .Ls007
.Ls005:
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls006:
        shl     ax,1
        rcl     dx,1
        loop    .Ls006
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
        inc     WORD PTR [bp-74]
        cmp     WORD PTR [bp-74],0x5
        jge     .Ls009
.Ls007:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls005
.Ls008:
        mov     ax,0x1
        jmp     .Ls168
.Ls009:
        mov     ax,WORD PTR [bp-20]
        cmp     ax,WORD PTR [bp+22]
        jb      .Ls012
        je      .Ls011
.Ls010:
        jmp     .Ls165
.Ls011:
        mov     ax,WORD PTR [bp-24]
        cmp     ax,WORD PTR [bp+20]
        jae     .Ls010
.Ls012:
        mov     ax,WORD PTR [bp-24]
        and     ax,WORD PTR [bp-98]
        mov     WORD PTR [bp-64],ax
        mov     cl,0x5
        mov     ax,WORD PTR [bp-18]
        shl     ax,cl
        add     ax,WORD PTR [bp-38]
        mov     dx,WORD PTR [bp-64]
        shl     dx,1
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Ls015
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        je      .Ls008
        mov     cx,0x8
.Ls013:
        shl     si,1
        rcl     di,1
        loop    .Ls013
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls014:
        shl     ax,1
        rcl     dx,1
        loop    .Ls014
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls015:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls016:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls016
        les     bx,DWORD PTR [bp-16]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls019
        je      .Ls018
.Ls017:
        jmp     .Ls045
.Ls018:
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls017
.Ls019:
        mov     WORD PTR [bp-102],0x1
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     cx,0x8
        sub     cx,WORD PTR [bp-88]
        mov     al,BYTE PTR [bp-2]
        xor     ah,ah
        sar     ax,cl
        cwd
        mov     WORD PTR [bp-100],ax
        mov     bx,dx
        mov     ax,WORD PTR [bp-24]
        and     ax,WORD PTR [bp-96]
        mov     dx,WORD PTR [bp-20]
        and     dx,WORD PTR [bp-94]
        mov     cx,WORD PTR [bp-88]
        jcxz    .Ls021
.Ls020:
        shl     ax,1
        rcl     dx,1
        loop    .Ls020
.Ls021:
        add     ax,WORD PTR [bp-100]
        adc     dx,bx
        mov     bx,0x300
        xor     cx,cx
        call    __U4M
        shl     ax,1
        rcl     dx,1
        mov     dx,WORD PTR [bp-38]
        add     dx,0xe6c
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        cmp     WORD PTR [bp-18],0x7
        jge     .Ls022
        jmp     .Ls032
.Ls022:
        mov     bx,WORD PTR [bp-24]
        sub     bx,WORD PTR [bp-110]
        mov     cx,WORD PTR [bp-20]
        sbb     cx,WORD PTR [bp-108]
        mov     ax,WORD PTR [bp+16]
        mov     dx,WORD PTR [bp+18]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR es:[bx]
        xor     ah,ah
        mov     WORD PTR [bp-86],ax
        movw    es,WORD PTR [bp-14]
.Ls023:
        shl     WORD PTR [bp-86],1
        mov     ax,WORD PTR [bp-86]
        xor     al,al
        and     ah,0x1
        mov     WORD PTR [bp-78],ax
        mov     dx,ax
        shl     dx,1
        mov     ax,WORD PTR [bp-16]
        add     ah,0x2
        add     ax,dx
        mov     dx,WORD PTR [bp-102]
        shl     dx,1
        add     ax,dx
        mov     WORD PTR [bp-48],ax
        cmp     di,0x100
        jae     .Ls027
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls024
        jmp     .Ls008
.Ls024:
        mov     cx,0x8
.Ls025:
        shl     si,1
        rcl     di,1
        loop    .Ls025
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls026:
        shl     ax,1
        rcl     dx,1
        loop    .Ls026
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls027:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls028:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls028
        mov     bx,WORD PTR [bp-48]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls029
        jne     .Ls030
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls030
.Ls029:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-48]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-102],1
        cmp     WORD PTR [bp-78],0x0
        jne     .Ls032
        jmp     .Ls031
.Ls030:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-48]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-102]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-102],ax
        cmp     WORD PTR [bp-78],0x0
        je      .Ls032
.Ls031:
        cmp     WORD PTR [bp-102],0x100
        jge     .Ls032
        jmp     .Ls023
.Ls032:
        movw    es,WORD PTR [bp-14]
.Ls033:
        mov     ax,WORD PTR [bp-102]
        cmp     ax,0x100
        jl      .Ls034
        jmp     .Ls042
.Ls034:
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-50],bx
        cmp     di,0x100
        jae     .Ls038
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls035
        jmp     .Ls008
.Ls035:
        mov     cx,0x8
.Ls036:
        shl     si,1
        rcl     di,1
        loop    .Ls036
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls037:
        shl     ax,1
        rcl     dx,1
        loop    .Ls037
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls038:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls039:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls039
        mov     bx,WORD PTR [bp-50]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls040
        jne     .Ls041
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls041
.Ls040:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-50]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-102],1
        jmp     .Ls033
.Ls041:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-50]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-102]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-102],ax
        jmp     .Ls033
.Ls042:
        mov     al,BYTE PTR [bp-102]
        mov     BYTE PTR [bp-2],al
        mov     bx,WORD PTR [bp-24]
        mov     cx,WORD PTR [bp-20]
        add     WORD PTR [bp-24],0x1
        adc     WORD PTR [bp-20],0x0
        mov     ax,WORD PTR [bp+16]
        mov     dx,WORD PTR [bp+18]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR [bp-102]
        mov     BYTE PTR es:[bx],al
        mov     ax,WORD PTR [bp-18]
        cmp     ax,0x4
        jge     .Ls043
        xor     ax,ax
        mov     WORD PTR [bp-18],ax
        jmp     .Ls009
.Ls043:
        cmp     ax,0xa
        jge     .Ls044
        sub     WORD PTR [bp-18],0x3
        jmp     .Ls009
.Ls044:
        sub     WORD PTR [bp-18],0x6
        jmp     .Ls009
.Ls045:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-16]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-18]
        shl     ax,1
        mov     dx,WORD PTR [bp-38]
        add     dx,0x180
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        cmp     di,0x100
        jae     .Ls049
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls046
        jmp     .Ls008
.Ls046:
        mov     cx,0x8
.Ls047:
        shl     si,1
        rcl     di,1
        loop    .Ls047
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls048:
        shl     ax,1
        rcl     dx,1
        loop    .Ls048
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls049:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls050:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls050
        les     bx,DWORD PTR [bp-16]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls051
        jne     .Ls054
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls054
.Ls051:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-56]
        mov     WORD PTR [bp-84],ax
        mov     ax,WORD PTR [bp-54]
        mov     WORD PTR [bp-68],ax
        mov     ax,WORD PTR [bp-60]
        mov     WORD PTR [bp-56],ax
        mov     ax,WORD PTR [bp-58]
        mov     WORD PTR [bp-54],ax
        mov     ax,WORD PTR [bp-110]
        mov     WORD PTR [bp-60],ax
        mov     ax,WORD PTR [bp-108]
        mov     WORD PTR [bp-58],ax
        cmp     WORD PTR [bp-18],0x7
        jge     .Ls052
        xor     ax,ax
        jmp     .Ls053
.Ls052:
        mov     ax,0x3
.Ls053:
        mov     WORD PTR [bp-18],ax
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-38]
        add     bx,0x664
        jmp     .Ls094
.Ls054:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-16]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     dx,WORD PTR [bp-18]
        shl     dx,1
        mov     ax,WORD PTR [bp-38]
        add     ax,0x198
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Ls058
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls055
        jmp     .Ls008
.Ls055:
        mov     cx,0x8
.Ls056:
        shl     si,1
        rcl     di,1
        loop    .Ls056
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls057:
        shl     ax,1
        rcl     dx,1
        loop    .Ls057
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls058:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls059:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls059
        les     bx,DWORD PTR [bp-16]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-90],ax
        mov     WORD PTR [bp-92],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,WORD PTR [bp-6]
        jb      .Ls062
        je      .Ls061
.Ls060:
        jmp     .Ls074
.Ls061:
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls060
.Ls062:
        mov     si,WORD PTR [bp-10]
        mov     di,WORD PTR [bp-6]
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-18]
        shl     ax,cl
        mov     dx,WORD PTR [bp-38]
        add     dx,0x1e0
        add     ax,dx
        mov     dx,WORD PTR [bp-64]
        shl     dx,1
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Ls066
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls063
        jmp     .Ls008
.Ls063:
        mov     ax,si
        mov     cx,0x8
.Ls064:
        shl     ax,1
        rcl     di,1
        loop    .Ls064
        mov     si,ax
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls065:
        shl     ax,1
        rcl     dx,1
        loop    .Ls065
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     bx,WORD PTR [bp-90]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-92]
.Ls066:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls067:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls067
        les     bx,DWORD PTR [bp-16]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls068
        jne     .Ls070
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls073
.Ls068:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-20]
        or      ax,WORD PTR [bp-24]
        jne     .Ls069
        jmp     .Ls008
.Ls069:
        cmp     WORD PTR [bp-18],0x7
        jge     .Ls071
        mov     ax,0x9
        jmp     .Ls072
.Ls070:
        jmp     .Ls073
.Ls071:
        mov     ax,0xb
.Ls072:
        mov     WORD PTR [bp-18],ax
        mov     bx,WORD PTR [bp-24]
        sub     bx,WORD PTR [bp-110]
        mov     cx,WORD PTR [bp-20]
        sbb     cx,WORD PTR [bp-108]
        mov     ax,WORD PTR [bp+16]
        mov     dx,WORD PTR [bp+18]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR es:[bx]
        mov     BYTE PTR [bp-2],al
        mov     bx,WORD PTR [bp-24]
        mov     cx,WORD PTR [bp-20]
        add     WORD PTR [bp-24],0x1
        adc     WORD PTR [bp-20],0x0
        mov     ax,WORD PTR [bp+16]
        mov     dx,WORD PTR [bp+18]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR [bp-2]
        mov     BYTE PTR es:[bx],al
        jmp     .Ls009
.Ls073:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-16]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        jmp     .Ls091
.Ls074:
        sub     si,WORD PTR [bp-10]
        sbb     di,WORD PTR [bp-6]
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        mov     ax,WORD PTR [bp-6]
        sbb     WORD PTR [bp-4],ax
        mov     cl,0x5
        mov     bx,WORD PTR [bp-16]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     dx,WORD PTR [bp-18]
        shl     dx,1
        mov     ax,WORD PTR [bp-38]
        add     ax,0x1b0
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Ls078
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls075
        jmp     .Ls008
.Ls075:
        mov     cx,0x8
.Ls076:
        shl     si,1
        rcl     di,1
        loop    .Ls076
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls077:
        shl     ax,1
        rcl     dx,1
        loop    .Ls077
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     bx,WORD PTR [bp-90]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-92]
.Ls078:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls079:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls079
        les     bx,DWORD PTR [bp-16]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls080
        jne     .Ls081
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls081
.Ls080:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-60]
        mov     cx,WORD PTR [bp-58]
        jmp     .Ls090
.Ls081:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-16]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-18]
        shl     ax,1
        mov     dx,WORD PTR [bp-38]
        add     dx,0x1c8
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        cmp     di,0x100
        jae     .Ls085
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls082
        jmp     .Ls008
.Ls082:
        mov     cx,0x8
.Ls083:
        shl     si,1
        rcl     di,1
        loop    .Ls083
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls084:
        shl     ax,1
        rcl     dx,1
        loop    .Ls084
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls085:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls086:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls086
        les     bx,DWORD PTR [bp-16]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls087
        jne     .Ls088
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls088
.Ls087:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-56]
        mov     cx,WORD PTR [bp-54]
        jmp     .Ls089
.Ls088:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-16]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-84]
        mov     cx,WORD PTR [bp-68]
        mov     dx,WORD PTR [bp-56]
        mov     WORD PTR [bp-84],dx
        mov     dx,WORD PTR [bp-54]
        mov     WORD PTR [bp-68],dx
.Ls089:
        mov     dx,WORD PTR [bp-60]
        mov     WORD PTR [bp-56],dx
        mov     dx,WORD PTR [bp-58]
        mov     WORD PTR [bp-54],dx
.Ls090:
        mov     dx,WORD PTR [bp-110]
        mov     WORD PTR [bp-60],dx
        mov     dx,WORD PTR [bp-108]
        mov     WORD PTR [bp-58],dx
        mov     WORD PTR [bp-110],ax
        mov     WORD PTR [bp-108],cx
.Ls091:
        cmp     WORD PTR [bp-18],0x7
        jge     .Ls092
        mov     ax,0x8
        jmp     .Ls093
.Ls092:
        mov     ax,0xb
.Ls093:
        mov     WORD PTR [bp-18],ax
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-38]
        add     bx,0xa68
.Ls094:
        mov     WORD PTR [bp-16],bx
        mov     bx,WORD PTR [bp-16]
        mov     WORD PTR [bp-22],bx
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-32],ax
        cmp     di,0x100
        jae     .Ls098
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls095
        jmp     .Ls008
.Ls095:
        mov     cx,0x8
.Ls096:
        shl     si,1
        rcl     di,1
        loop    .Ls096
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls097:
        shl     ax,1
        rcl     dx,1
        loop    .Ls097
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls098:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls099:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls099
        movw    es,WORD PTR [bp-32]
        mov     bx,WORD PTR [bp-22]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls100
        jne     .Ls102
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls102
.Ls100:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-22]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     cl,0x4
        mov     ax,WORD PTR [bp-64]
        shl     ax,cl
        mov     dx,WORD PTR [bp-16]
        add     dx,0x4
        mov     bx,WORD PTR [bp-14]
        mov     WORD PTR [bp-32],bx
        add     dx,ax
        mov     WORD PTR [bp-22],dx
        xor     ax,ax
        mov     WORD PTR [bp-80],ax
.Ls101:
        mov     WORD PTR [bp-62],0x3
        jmp     .Ls110
.Ls102:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-22]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-32],ax
        mov     bx,WORD PTR [bp-16]
        inc     bx
        inc     bx
        mov     WORD PTR [bp-22],bx
        cmp     di,0x100
        jae     .Ls106
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls103
        jmp     .Ls008
.Ls103:
        mov     cx,0x8
.Ls104:
        shl     si,1
        rcl     di,1
        loop    .Ls104
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls105:
        shl     ax,1
        rcl     dx,1
        loop    .Ls105
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls106:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls107:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls107
        movw    es,WORD PTR [bp-32]
        mov     bx,WORD PTR [bp-22]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls108
        jne     .Ls109
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls109
.Ls108:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-22]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     cl,0x4
        mov     dx,WORD PTR [bp-64]
        shl     dx,cl
        mov     ax,WORD PTR [bp-16]
        add     ax,0x104
        mov     bx,WORD PTR [bp-14]
        mov     WORD PTR [bp-32],bx
        add     ax,dx
        mov     WORD PTR [bp-22],ax
        mov     WORD PTR [bp-80],0x8
        jmp     .Ls101
.Ls109:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-22]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-32],ax
        mov     bx,WORD PTR [bp-16]
        add     bx,0x204
        mov     WORD PTR [bp-22],bx
        mov     WORD PTR [bp-80],0x10
        mov     WORD PTR [bp-62],0x8
.Ls110:
        mov     ax,WORD PTR [bp-62]
        mov     WORD PTR [bp-70],ax
        mov     WORD PTR [bp-26],0x1
        movw    es,WORD PTR [bp-32]
.Ls111:
        mov     ax,WORD PTR [bp-26]
        shl     ax,1
        mov     bx,WORD PTR [bp-22]
        add     bx,ax
        mov     WORD PTR [bp-42],bx
        cmp     di,0x100
        jae     .Ls115
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls112
        jmp     .Ls008
.Ls112:
        mov     cx,0x8
.Ls113:
        shl     si,1
        rcl     di,1
        loop    .Ls113
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls114:
        shl     ax,1
        rcl     dx,1
        loop    .Ls114
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls115:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls116:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls116
        mov     bx,WORD PTR [bp-42]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls117
        jne     .Ls118
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls118
.Ls117:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-42]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-26],1
        jmp     .Ls119
.Ls118:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-42]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-26]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-26],ax
.Ls119:
        dec     WORD PTR [bp-70]
        je      .Ls120
        jmp     .Ls111
.Ls120:
        mov     cl,BYTE PTR [bp-62]
        mov     ax,0x1
        shl     ax,cl
        sub     WORD PTR [bp-26],ax
        mov     ax,WORD PTR [bp-80]
        add     WORD PTR [bp-26],ax
        cmp     WORD PTR [bp-18],0x4
        jl      .Ls121
        jmp     .Ls159
.Ls121:
        add     WORD PTR [bp-18],0x7
        mov     ax,WORD PTR [bp-26]
        cmp     ax,0x4
        jl      .Ls122
        mov     ax,0x3
.Ls122:
        mov     cl,0x7
        shl     ax,cl
        mov     dx,WORD PTR [bp-38]
        add     dx,0x360
        mov     bx,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        mov     WORD PTR [bp-72],0x6
        mov     WORD PTR [bp-34],0x1
        movw    es,WORD PTR [bp-14]
.Ls123:
        mov     ax,WORD PTR [bp-34]
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-46],bx
        cmp     di,0x100
        jae     .Ls127
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls124
        jmp     .Ls008
.Ls124:
        mov     cx,0x8
.Ls125:
        shl     si,1
        rcl     di,1
        loop    .Ls125
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls126:
        shl     ax,1
        rcl     dx,1
        loop    .Ls126
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls127:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls128:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls128
        mov     bx,WORD PTR [bp-46]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls129
        jne     .Ls130
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls130
.Ls129:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-46]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-34],1
        jmp     .Ls131
.Ls130:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-46]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-34]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-34],ax
.Ls131:
        dec     WORD PTR [bp-72]
        je      .Ls132
        jmp     .Ls123
.Ls132:
        sub     WORD PTR [bp-34],0x40
        mov     ax,WORD PTR [bp-34]
        cmp     ax,0x4
        jl      .Ls135
        sar     ax,1
        dec     ax
        mov     WORD PTR [bp-40],ax
        mov     ax,WORD PTR [bp-34]
        and     ax,0x1
        or      al,0x2
        mov     WORD PTR [bp-110],ax
        xor     ax,ax
        mov     WORD PTR [bp-108],ax
        cmp     WORD PTR [bp-34],0xe
        jge     .Ls136
        mov     cx,WORD PTR [bp-40]
        jcxz    .Ls134
.Ls133:
        shl     WORD PTR [bp-110],1
        rcl     WORD PTR [bp-108],1
        loop    .Ls133
.Ls134:
        mov     dx,WORD PTR [bp-110]
        shl     dx,1
        mov     ax,WORD PTR [bp-38]
        add     ax,0x560
        add     dx,ax
        mov     ax,WORD PTR [bp-34]
        shl     ax,1
        sub     dx,ax
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        dec     dx
        dec     dx
        mov     WORD PTR [bp-16],dx
        jmp     .Ls146
.Ls135:
        jmp     .Ls157
.Ls136:
        sub     WORD PTR [bp-40],0x4
.Ls137:
        cmp     di,0x100
        jae     .Ls141
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls138
        jmp     .Ls008
.Ls138:
        mov     cx,0x8
.Ls139:
        shl     si,1
        rcl     di,1
        loop    .Ls139
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls140:
        shl     ax,1
        rcl     dx,1
        loop    .Ls140
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls141:
        shr     di,1
        rcr     si,1
        shl     WORD PTR [bp-110],1
        rcl     WORD PTR [bp-108],1
        mov     ax,WORD PTR [bp-4]
        cmp     di,ax
        jb      .Ls142
        jne     .Ls143
        cmp     si,WORD PTR [bp-8]
        ja      .Ls143
.Ls142:
        sub     WORD PTR [bp-8],si
        sbb     WORD PTR [bp-4],di
        or      BYTE PTR [bp-110],0x1
.Ls143:
        dec     WORD PTR [bp-40]
        je      .Ls144
        jmp     .Ls137
.Ls144:
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-38]
        add     bx,0x644
        mov     WORD PTR [bp-16],bx
        mov     cx,0x4
.Ls145:
        shl     WORD PTR [bp-110],1
        rcl     WORD PTR [bp-108],1
        loop    .Ls145
        mov     WORD PTR [bp-40],0x4
.Ls146:
        mov     ax,0x1
        mov     WORD PTR [bp-66],ax
        mov     WORD PTR [bp-52],ax
        movw    es,WORD PTR [bp-14]
.Ls147:
        mov     ax,WORD PTR [bp-52]
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-44],bx
        cmp     di,0x100
        jae     .Ls151
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls148
        jmp     .Ls008
.Ls148:
        mov     cx,0x8
.Ls149:
        shl     si,1
        rcl     di,1
        loop    .Ls149
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Ls150:
        shl     ax,1
        rcl     dx,1
        loop    .Ls150
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls151:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Ls152:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Ls152
        mov     bx,WORD PTR [bp-44]
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-106]
        mov     dx,WORD PTR [bp-104]
        xor     cx,cx
        call    __U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        mov     ax,WORD PTR [bp-4]
        cmp     ax,dx
        jb      .Ls153
        jne     .Ls154
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Ls154
.Ls153:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-44]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-52],1
        jmp     .Ls155
.Ls154:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-44]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-52]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-52],ax
        mov     ax,WORD PTR [bp-66]
        cwd
        or      WORD PTR [bp-110],ax
        or      WORD PTR [bp-108],dx
.Ls155:
        shl     WORD PTR [bp-66],1
        dec     WORD PTR [bp-40]
        je      .Ls156
        jmp     .Ls147
.Ls156:
        jmp     .Ls158
.Ls157:
        cwd
        mov     WORD PTR [bp-110],ax
        mov     WORD PTR [bp-108],dx
.Ls158:
        add     WORD PTR [bp-110],0x1
        adc     WORD PTR [bp-108],0x0
        mov     ax,WORD PTR [bp-108]
        or      ax,WORD PTR [bp-110]
        jne     .Ls159
        jmp     .Ls165
.Ls159:
        add     WORD PTR [bp-26],0x2
        mov     ax,WORD PTR [bp-108]
        cmp     ax,WORD PTR [bp-20]
        jbe     .Ls161
.Ls160:
        jmp     .Ls008
.Ls161:
        jne     .Ls162
        mov     ax,WORD PTR [bp-110]
        cmp     ax,WORD PTR [bp-24]
        ja      .Ls160
.Ls162:
        mov     bx,WORD PTR [bp-24]
        sub     bx,WORD PTR [bp-110]
        mov     cx,WORD PTR [bp-20]
        sbb     cx,WORD PTR [bp-108]
        mov     ax,WORD PTR [bp+16]
        mov     dx,WORD PTR [bp+18]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR es:[bx]
        mov     BYTE PTR [bp-2],al
        mov     bx,WORD PTR [bp-24]
        mov     cx,WORD PTR [bp-20]
        dec     WORD PTR [bp-26]
        add     WORD PTR [bp-24],0x1
        adc     WORD PTR [bp-20],0x0
        mov     ax,WORD PTR [bp+16]
        mov     dx,WORD PTR [bp+18]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR [bp-2]
        mov     BYTE PTR es:[bx],al
        cmp     WORD PTR [bp-26],0x0
        jne     .Ls164
.Ls163:
        jmp     .Ls009
.Ls164:
        mov     ax,WORD PTR [bp-20]
        cmp     ax,WORD PTR [bp+22]
        jb      .Ls162
        jne     .Ls163
        mov     ax,WORD PTR [bp-24]
        cmp     ax,WORD PTR [bp+20]
        jb      .Ls162
        jmp     .Ls163
.Ls165:
        cmp     di,0x100
        jae     .Ls167
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Ls166
        jmp     .Ls008
.Ls166:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Ls167:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-82]
        mov     cx,WORD PTR [bp-76]
        call    __PTS
        lds     bx,DWORD PTR [bp+12]
        mov     WORD PTR [bx],ax
        mov     WORD PTR [bx+2],dx
        mov     ax,WORD PTR [bp-24]
        lds     bx,DWORD PTR [bp+24]
        mov     WORD PTR [bx],ax
        mov     ax,WORD PTR [bp-20]
        mov     WORD PTR [bx+2],ax
        xor     ax,ax
.Ls168:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret     0x14

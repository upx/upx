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
        xor     ax,ax
        xor     dx,dx
        movw    ds,WORD PTR [bp-36]
.Lf002:
        cmp     dx,cx
        jb      .Lf003
        jne     .Lf004
        cmp     ax,si
        jae     .Lf004
.Lf003:
        mov     bx,ax
        shl     bx,1
        add     bx,WORD PTR [bp-38]
        mov     WORD PTR [bx],0x400
        add     ax,0x1
        adc     dx,0x0
        jmp     .Lf002
.Lf004:
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
        jmp     .Lf007
.Lf005:
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf006:
        shl     ax,1
        rcl     dx,1
        loop    .Lf006
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
        jge     .Lf009
.Lf007:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf005
.Lf008:
        mov     ax,0x1
        jmp     .Lf168
.Lf009:
        mov     ax,WORD PTR [bp-20]
        cmp     ax,WORD PTR [bp+22]
        jb      .Lf012
        je      .Lf011
.Lf010:
        jmp     .Lf165
.Lf011:
        mov     ax,WORD PTR [bp-24]
        cmp     ax,WORD PTR [bp+20]
        jae     .Lf010
.Lf012:
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
        jae     .Lf015
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        je      .Lf008
        mov     cx,0x8
.Lf013:
        shl     si,1
        rcl     di,1
        loop    .Lf013
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf014:
        shl     ax,1
        rcl     dx,1
        loop    .Lf014
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
.Lf015:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf016:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf016
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
        jb      .Lf019
        je      .Lf018
.Lf017:
        jmp     .Lf045
.Lf018:
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf017
.Lf019:
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
        jcxz    .Lf021
.Lf020:
        shl     ax,1
        rcl     dx,1
        loop    .Lf020
.Lf021:
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
        jge     .Lf022
        jmp     .Lf032
.Lf022:
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
.Lf023:
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
        jae     .Lf027
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf024
        jmp     .Lf008
.Lf024:
        mov     cx,0x8
.Lf025:
        shl     si,1
        rcl     di,1
        loop    .Lf025
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf026:
        shl     ax,1
        rcl     dx,1
        loop    .Lf026
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
.Lf027:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf028:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf028
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
        jb      .Lf029
        jne     .Lf030
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf030
.Lf029:
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
        jne     .Lf032
        jmp     .Lf031
.Lf030:
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
        je      .Lf032
.Lf031:
        cmp     WORD PTR [bp-102],0x100
        jge     .Lf032
        jmp     .Lf023
.Lf032:
        movw    es,WORD PTR [bp-14]
.Lf033:
        mov     ax,WORD PTR [bp-102]
        cmp     ax,0x100
        jl      .Lf034
        jmp     .Lf042
.Lf034:
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-50],bx
        cmp     di,0x100
        jae     .Lf038
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf035
        jmp     .Lf008
.Lf035:
        mov     cx,0x8
.Lf036:
        shl     si,1
        rcl     di,1
        loop    .Lf036
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf037:
        shl     ax,1
        rcl     dx,1
        loop    .Lf037
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
.Lf038:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf039:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf039
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
        jb      .Lf040
        jne     .Lf041
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf041
.Lf040:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-50]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-102],1
        jmp     .Lf033
.Lf041:
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
        jmp     .Lf033
.Lf042:
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
        jge     .Lf043
        xor     ax,ax
        mov     WORD PTR [bp-18],ax
        jmp     .Lf009
.Lf043:
        cmp     ax,0xa
        jge     .Lf044
        sub     WORD PTR [bp-18],0x3
        jmp     .Lf009
.Lf044:
        sub     WORD PTR [bp-18],0x6
        jmp     .Lf009
.Lf045:
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
        jae     .Lf049
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf046
        jmp     .Lf008
.Lf046:
        mov     cx,0x8
.Lf047:
        shl     si,1
        rcl     di,1
        loop    .Lf047
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf048:
        shl     ax,1
        rcl     dx,1
        loop    .Lf048
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
.Lf049:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf050:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf050
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
        jb      .Lf051
        jne     .Lf054
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf054
.Lf051:
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
        jge     .Lf052
        xor     ax,ax
        jmp     .Lf053
.Lf052:
        mov     ax,0x3
.Lf053:
        mov     WORD PTR [bp-18],ax
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-38]
        add     bx,0x664
        jmp     .Lf094
.Lf054:
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
        jae     .Lf058
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf055
        jmp     .Lf008
.Lf055:
        mov     cx,0x8
.Lf056:
        shl     si,1
        rcl     di,1
        loop    .Lf056
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf057:
        shl     ax,1
        rcl     dx,1
        loop    .Lf057
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
.Lf058:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf059:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf059
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
        jb      .Lf062
        je      .Lf061
.Lf060:
        jmp     .Lf074
.Lf061:
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf060
.Lf062:
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
        jae     .Lf066
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf063
        jmp     .Lf008
.Lf063:
        mov     ax,si
        mov     cx,0x8
.Lf064:
        shl     ax,1
        rcl     di,1
        loop    .Lf064
        mov     si,ax
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf065:
        shl     ax,1
        rcl     dx,1
        loop    .Lf065
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     bx,WORD PTR [bp-90]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-92]
.Lf066:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf067:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf067
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
        jb      .Lf068
        jne     .Lf070
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf073
.Lf068:
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
        jne     .Lf069
        jmp     .Lf008
.Lf069:
        cmp     WORD PTR [bp-18],0x7
        jge     .Lf071
        mov     ax,0x9
        jmp     .Lf072
.Lf070:
        jmp     .Lf073
.Lf071:
        mov     ax,0xb
.Lf072:
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
        jmp     .Lf009
.Lf073:
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
        jmp     .Lf091
.Lf074:
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
        jae     .Lf078
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf075
        jmp     .Lf008
.Lf075:
        mov     cx,0x8
.Lf076:
        shl     si,1
        rcl     di,1
        loop    .Lf076
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf077:
        shl     ax,1
        rcl     dx,1
        loop    .Lf077
        mov     bx,WORD PTR [bp-100]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     bx,WORD PTR [bp-90]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-92]
.Lf078:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf079:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf079
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
        jb      .Lf080
        jne     .Lf081
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf081
.Lf080:
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
        jmp     .Lf090
.Lf081:
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
        jae     .Lf085
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf082
        jmp     .Lf008
.Lf082:
        mov     cx,0x8
.Lf083:
        shl     si,1
        rcl     di,1
        loop    .Lf083
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf084:
        shl     ax,1
        rcl     dx,1
        loop    .Lf084
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
.Lf085:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf086:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf086
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
        jb      .Lf087
        jne     .Lf088
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf088
.Lf087:
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
        jmp     .Lf089
.Lf088:
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
.Lf089:
        mov     dx,WORD PTR [bp-60]
        mov     WORD PTR [bp-56],dx
        mov     dx,WORD PTR [bp-58]
        mov     WORD PTR [bp-54],dx
.Lf090:
        mov     dx,WORD PTR [bp-110]
        mov     WORD PTR [bp-60],dx
        mov     dx,WORD PTR [bp-108]
        mov     WORD PTR [bp-58],dx
        mov     WORD PTR [bp-110],ax
        mov     WORD PTR [bp-108],cx
.Lf091:
        cmp     WORD PTR [bp-18],0x7
        jge     .Lf092
        mov     ax,0x8
        jmp     .Lf093
.Lf092:
        mov     ax,0xb
.Lf093:
        mov     WORD PTR [bp-18],ax
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-38]
        add     bx,0xa68
.Lf094:
        mov     WORD PTR [bp-16],bx
        mov     bx,WORD PTR [bp-16]
        mov     WORD PTR [bp-22],bx
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-32],ax
        cmp     di,0x100
        jae     .Lf098
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf095
        jmp     .Lf008
.Lf095:
        mov     cx,0x8
.Lf096:
        shl     si,1
        rcl     di,1
        loop    .Lf096
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf097:
        shl     ax,1
        rcl     dx,1
        loop    .Lf097
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
.Lf098:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf099:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf099
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
        jb      .Lf100
        jne     .Lf102
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf102
.Lf100:
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
.Lf101:
        mov     WORD PTR [bp-62],0x3
        jmp     .Lf110
.Lf102:
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
        jae     .Lf106
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf103
        jmp     .Lf008
.Lf103:
        mov     cx,0x8
.Lf104:
        shl     si,1
        rcl     di,1
        loop    .Lf104
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf105:
        shl     ax,1
        rcl     dx,1
        loop    .Lf105
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
.Lf106:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf107:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf107
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
        jb      .Lf108
        jne     .Lf109
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf109
.Lf108:
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
        jmp     .Lf101
.Lf109:
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
.Lf110:
        mov     ax,WORD PTR [bp-62]
        mov     WORD PTR [bp-70],ax
        mov     WORD PTR [bp-26],0x1
        movw    es,WORD PTR [bp-32]
.Lf111:
        mov     ax,WORD PTR [bp-26]
        shl     ax,1
        mov     bx,WORD PTR [bp-22]
        add     bx,ax
        mov     WORD PTR [bp-42],bx
        cmp     di,0x100
        jae     .Lf115
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf112
        jmp     .Lf008
.Lf112:
        mov     cx,0x8
.Lf113:
        shl     si,1
        rcl     di,1
        loop    .Lf113
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf114:
        shl     ax,1
        rcl     dx,1
        loop    .Lf114
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
.Lf115:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf116:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf116
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
        jb      .Lf117
        jne     .Lf118
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf118
.Lf117:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-42]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-26],1
        jmp     .Lf119
.Lf118:
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
.Lf119:
        dec     WORD PTR [bp-70]
        je      .Lf120
        jmp     .Lf111
.Lf120:
        mov     cl,BYTE PTR [bp-62]
        mov     ax,0x1
        shl     ax,cl
        sub     WORD PTR [bp-26],ax
        mov     ax,WORD PTR [bp-80]
        add     WORD PTR [bp-26],ax
        cmp     WORD PTR [bp-18],0x4
        jl      .Lf121
        jmp     .Lf159
.Lf121:
        add     WORD PTR [bp-18],0x7
        mov     ax,WORD PTR [bp-26]
        cmp     ax,0x4
        jl      .Lf122
        mov     ax,0x3
.Lf122:
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
.Lf123:
        mov     ax,WORD PTR [bp-34]
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-46],bx
        cmp     di,0x100
        jae     .Lf127
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf124
        jmp     .Lf008
.Lf124:
        mov     cx,0x8
.Lf125:
        shl     si,1
        rcl     di,1
        loop    .Lf125
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf126:
        shl     ax,1
        rcl     dx,1
        loop    .Lf126
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
.Lf127:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf128:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf128
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
        jb      .Lf129
        jne     .Lf130
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf130
.Lf129:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-46]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-34],1
        jmp     .Lf131
.Lf130:
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
.Lf131:
        dec     WORD PTR [bp-72]
        je      .Lf132
        jmp     .Lf123
.Lf132:
        sub     WORD PTR [bp-34],0x40
        mov     ax,WORD PTR [bp-34]
        cmp     ax,0x4
        jl      .Lf135
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
        jge     .Lf136
        mov     cx,WORD PTR [bp-40]
        jcxz    .Lf134
.Lf133:
        shl     WORD PTR [bp-110],1
        rcl     WORD PTR [bp-108],1
        loop    .Lf133
.Lf134:
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
        jmp     .Lf146
.Lf135:
        jmp     .Lf157
.Lf136:
        sub     WORD PTR [bp-40],0x4
.Lf137:
        cmp     di,0x100
        jae     .Lf141
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf138
        jmp     .Lf008
.Lf138:
        mov     cx,0x8
.Lf139:
        shl     si,1
        rcl     di,1
        loop    .Lf139
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf140:
        shl     ax,1
        rcl     dx,1
        loop    .Lf140
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
.Lf141:
        shr     di,1
        rcr     si,1
        shl     WORD PTR [bp-110],1
        rcl     WORD PTR [bp-108],1
        mov     ax,WORD PTR [bp-4]
        cmp     di,ax
        jb      .Lf142
        jne     .Lf143
        cmp     si,WORD PTR [bp-8]
        ja      .Lf143
.Lf142:
        sub     WORD PTR [bp-8],si
        sbb     WORD PTR [bp-4],di
        or      BYTE PTR [bp-110],0x1
.Lf143:
        dec     WORD PTR [bp-40]
        je      .Lf144
        jmp     .Lf137
.Lf144:
        mov     ax,WORD PTR [bp-36]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-38]
        add     bx,0x644
        mov     WORD PTR [bp-16],bx
        mov     cx,0x4
.Lf145:
        shl     WORD PTR [bp-110],1
        rcl     WORD PTR [bp-108],1
        loop    .Lf145
        mov     WORD PTR [bp-40],0x4
.Lf146:
        mov     ax,0x1
        mov     WORD PTR [bp-66],ax
        mov     WORD PTR [bp-52],ax
        movw    es,WORD PTR [bp-14]
.Lf147:
        mov     ax,WORD PTR [bp-52]
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-44],bx
        cmp     di,0x100
        jae     .Lf151
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf148
        jmp     .Lf008
.Lf148:
        mov     cx,0x8
.Lf149:
        shl     si,1
        rcl     di,1
        loop    .Lf149
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-100],al
        mov     BYTE PTR [bp-99],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf150:
        shl     ax,1
        rcl     dx,1
        loop    .Lf150
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
.Lf151:
        mov     WORD PTR [bp-106],si
        mov     WORD PTR [bp-104],di
        mov     cx,0xb
.Lf152:
        shr     WORD PTR [bp-104],1
        rcr     WORD PTR [bp-106],1
        loop    .Lf152
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
        jb      .Lf153
        jne     .Lf154
        mov     ax,WORD PTR [bp-8]
        cmp     ax,WORD PTR [bp-10]
        jae     .Lf154
.Lf153:
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-44]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-52],1
        jmp     .Lf155
.Lf154:
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
.Lf155:
        shl     WORD PTR [bp-66],1
        dec     WORD PTR [bp-40]
        je      .Lf156
        jmp     .Lf147
.Lf156:
        jmp     .Lf158
.Lf157:
        cwd
        mov     WORD PTR [bp-110],ax
        mov     WORD PTR [bp-108],dx
.Lf158:
        add     WORD PTR [bp-110],0x1
        adc     WORD PTR [bp-108],0x0
        mov     ax,WORD PTR [bp-108]
        or      ax,WORD PTR [bp-110]
        jne     .Lf159
        jmp     .Lf165
.Lf159:
        add     WORD PTR [bp-26],0x2
        mov     ax,WORD PTR [bp-108]
        cmp     ax,WORD PTR [bp-20]
        jbe     .Lf161
.Lf160:
        jmp     .Lf008
.Lf161:
        jne     .Lf162
        mov     ax,WORD PTR [bp-110]
        cmp     ax,WORD PTR [bp-24]
        ja      .Lf160
.Lf162:
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
        jne     .Lf164
.Lf163:
        jmp     .Lf009
.Lf164:
        mov     ax,WORD PTR [bp-20]
        cmp     ax,WORD PTR [bp+22]
        jb      .Lf162
        jne     .Lf163
        mov     ax,WORD PTR [bp-24]
        cmp     ax,WORD PTR [bp+20]
        jb      .Lf162
        jmp     .Lf163
.Lf165:
        cmp     di,0x100
        jae     .Lf167
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-30]
        mov     cx,WORD PTR [bp-28]
        call    __PTC
        jne     .Lf166
        jmp     .Lf008
.Lf166:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        call    __PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
.Lf167:
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
.Lf168:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret     0x14

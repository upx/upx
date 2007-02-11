        push    si
        push    di
        push    bp
        mov     bp,sp
        sub     sp,0x6a
        lds     si,DWORD PTR [bp+8]
        movw    WORD PTR [bp-34],ds
        lea     ax,[si+4]
        mov     WORD PTR [bp-36],ax
        xor     ax,ax
        mov     WORD PTR [bp-20],ax
        mov     WORD PTR [bp-96],ax
        mov     BYTE PTR [bp-2],0x0
        mov     cl,BYTE PTR [si+2]
        mov     ax,0x1
        shl     ax,cl
        dec     ax
        mov     WORD PTR [bp-88],ax
        mov     cl,BYTE PTR [si+1]
        mov     ax,0x1
        shl     ax,cl
        dec     ax
        cwd
        mov     WORD PTR [bp-92],ax
        mov     WORD PTR [bp-90],dx
        mov     al,BYTE PTR [si]
        xor     ah,ah
        mov     WORD PTR [bp-82],ax
        mov     WORD PTR [bp-18],0x0
        mov     WORD PTR [bp-106],0x1
        mov     WORD PTR [bp-104],0x0
        mov     WORD PTR [bp-58],0x1
        mov     WORD PTR [bp-52],0x0
        mov     WORD PTR [bp-56],0x1
        mov     WORD PTR [bp-54],0x0
        mov     WORD PTR [bp-76],0x1
        mov     WORD PTR [bp-64],0x0
        les     bx,DWORD PTR [bp+20]
        mov     WORD PTR es:[bx],0x0
        mov     WORD PTR es:[bx+2],0x0
        les     bx,DWORD PTR [bp+32]
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
        xor     dx,dx
        xor     ax,ax
        movw    ds,WORD PTR [bp-34]
.Lf002:
        cmp     ax,cx
        jb      .Lf003
        jne     .Lf004
        cmp     dx,si
        jae     .Lf004
.Lf003:
        mov     bx,dx
        shl     bx,1
        add     bx,WORD PTR [bp-36]
        mov     WORD PTR [bx],0x400
        add     dx,0x1
        adc     ax,0x0
        jmp     .Lf002
.Lf004:
        mov     bx,WORD PTR [bp+12]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp+14]
        mov     ax,bx
        movw    dx,ds
        mov     bx,WORD PTR [bp+16]
        mov     cx,WORD PTR [bp+18]
        call    __PIA
        mov     WORD PTR [bp-26],ax
        mov     WORD PTR [bp-24],dx
        xor     ax,ax
        mov     WORD PTR [bp-8],ax
        mov     WORD PTR [bp-4],ax
        mov     si,0xffff
        mov     di,si
        mov     WORD PTR [bp-66],ax
        jmp     .Lf007
.Lf005:
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf006:
        shl     ax,1
        rcl     dx,1
        loop    .Lf006
        call    __PIA_V04
        inc     WORD PTR [bp-66]
        cmp     WORD PTR [bp-66],0x5
        jge     .Lf009
.Lf007:
        call    __PTC_V01
        jne     .Lf005
.Lf008:
        mov     ax,0x1
        jmp     .Lf168
.Lf009:
        mov     ax,WORD PTR [bp-96]
        cmp     ax,WORD PTR [bp+30]
        jb      .Lf012
        je      .Lf011
.Lf010:
        jmp     .Lf165
.Lf011:
        mov     ax,WORD PTR [bp-20]
        cmp     ax,WORD PTR [bp+28]
        jae     .Lf010
.Lf012:
        mov     ax,WORD PTR [bp-20]
        and     ax,WORD PTR [bp-88]
        mov     WORD PTR [bp-60],ax
        mov     cl,0x5
        mov     ax,WORD PTR [bp-18]
        shl     ax,cl
        add     ax,WORD PTR [bp-36]
        mov     dx,WORD PTR [bp-60]
        shl     dx,1
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Lf015
        call    __PTC_V01
        je      .Lf008
        mov     cx,0x8
.Lf013:
        shl     si,1
        rcl     di,1
        loop    .Lf013
        mov     bx,WORD PTR [bp-12]
        mov     al,BYTE PTR [bx]
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf014:
        shl     ax,1
        rcl     dx,1
        loop    .Lf014
        call    __PIA_V04
.Lf015:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf016:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf016
        les     bx,DWORD PTR [bp-16]
        call    __U4M_V02
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
        mov     WORD PTR [bp-28],0x1
        mov     si,WORD PTR [bp-10]
        mov     di,dx
        mov     ax,0x800
        mov     bx,WORD PTR [bp-16]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     cx,0x8
        sub     cx,WORD PTR [bp-82]
        mov     al,BYTE PTR [bp-2]
        xor     ah,ah
        sar     ax,cl
        cwd
        mov     WORD PTR [bp-94],ax
        mov     bx,dx
        mov     ax,WORD PTR [bp-20]
        and     ax,WORD PTR [bp-92]
        mov     dx,WORD PTR [bp-96]
        and     dx,WORD PTR [bp-90]
        mov     cx,WORD PTR [bp-82]
        jcxz    .Lf021
.Lf020:
        shl     ax,1
        rcl     dx,1
        loop    .Lf020
.Lf021:
        add     ax,WORD PTR [bp-94]
        adc     dx,bx
        mov     bx,0x300
        call    __U4M_V01
        shl     ax,1
        rcl     dx,1
        mov     dx,WORD PTR [bp-36]
        add     dx,0xe6c
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        cmp     WORD PTR [bp-18],0x7
        jge     .Lf022
        jmp     .Lf032
.Lf022:
        mov     bx,WORD PTR [bp-20]
        sub     bx,WORD PTR [bp-106]
        mov     cx,WORD PTR [bp-96]
        sbb     cx,WORD PTR [bp-104]
        mov     ax,WORD PTR [bp+24]
        mov     dx,WORD PTR [bp+26]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR es:[bx]
        xor     ah,ah
        mov     WORD PTR [bp-72],ax
        movw    es,WORD PTR [bp-14]
.Lf023:
        shl     WORD PTR [bp-72],1
        mov     ax,WORD PTR [bp-72]
        xor     al,al
        and     ah,0x1
        mov     WORD PTR [bp-68],ax
        mov     dx,ax
        shl     dx,1
        mov     ax,WORD PTR [bp-16]
        add     ah,0x2
        add     dx,ax
        mov     ax,WORD PTR [bp-28]
        shl     ax,1
        add     dx,ax
        mov     WORD PTR [bp-44],dx
        cmp     di,0x100
        jae     .Lf027
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf026:
        shl     ax,1
        rcl     dx,1
        loop    .Lf026
        call    __PIA_V04
.Lf027:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf028:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf028
        mov     bx,WORD PTR [bp-44]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-44]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-28],1
        cmp     WORD PTR [bp-68],0x0
        jne     .Lf032
        jmp     .Lf031
.Lf030:
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
        mov     ax,WORD PTR [bp-28]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-28],ax
        cmp     WORD PTR [bp-68],0x0
        je      .Lf032
.Lf031:
        cmp     WORD PTR [bp-28],0x100
        jge     .Lf032
        jmp     .Lf023
.Lf032:
        movw    es,WORD PTR [bp-14]
.Lf033:
        mov     ax,WORD PTR [bp-28]
        cmp     ax,0x100
        jl      .Lf034
        jmp     .Lf042
.Lf034:
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-42],bx
        cmp     di,0x100
        jae     .Lf038
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf037:
        shl     ax,1
        rcl     dx,1
        loop    .Lf037
        call    __PIA_V04
.Lf038:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf039:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf039
        mov     bx,WORD PTR [bp-42]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-42]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-28],1
        jmp     .Lf033
.Lf041:
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
        mov     ax,WORD PTR [bp-28]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-28],ax
        jmp     .Lf033
.Lf042:
        mov     al,BYTE PTR [bp-28]
        mov     BYTE PTR [bp-2],al
        mov     bx,WORD PTR [bp-20]
        mov     cx,WORD PTR [bp-96]
        add     WORD PTR [bp-20],0x1
        adc     WORD PTR [bp-96],0x0
        mov     ax,WORD PTR [bp+24]
        mov     dx,WORD PTR [bp+26]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR [bp-28]
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
        mov     dx,WORD PTR [bp-36]
        add     dx,0x180
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        cmp     di,0x100
        jae     .Lf049
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf048:
        shl     ax,1
        rcl     dx,1
        loop    .Lf048
        call    __PIA_V04
.Lf049:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf050:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf050
        les     bx,DWORD PTR [bp-16]
        call    __U4M_V02
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
        mov     WORD PTR [bp-76],ax
        mov     ax,WORD PTR [bp-54]
        mov     WORD PTR [bp-64],ax
        mov     ax,WORD PTR [bp-58]
        mov     WORD PTR [bp-56],ax
        mov     ax,WORD PTR [bp-52]
        mov     WORD PTR [bp-54],ax
        mov     ax,WORD PTR [bp-106]
        mov     WORD PTR [bp-58],ax
        mov     ax,WORD PTR [bp-104]
        mov     WORD PTR [bp-52],ax
        cmp     WORD PTR [bp-18],0x7
        jge     .Lf052
        xor     ax,ax
        jmp     .Lf053
.Lf052:
        mov     ax,0x3
.Lf053:
        mov     WORD PTR [bp-18],ax
        mov     ax,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-36]
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
        mov     ax,WORD PTR [bp-36]
        add     ax,0x198
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Lf058
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf057:
        shl     ax,1
        rcl     dx,1
        loop    .Lf057
        call    __PIA_V04
.Lf058:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf059:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf059
        les     bx,DWORD PTR [bp-16]
        call    __U4M_V02
        mov     ax,WORD PTR [bp-12]
        WCC_PIA_V02
        mov     WORD PTR [bp-84],ax
        mov     WORD PTR [bp-86],dx
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
        mov     dx,WORD PTR [bp-18]
        shl     dx,cl
        mov     ax,WORD PTR [bp-36]
        add     ax,0x1e0
        add     ax,dx
        mov     dx,WORD PTR [bp-60]
        shl     dx,1
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Lf066
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf065:
        shl     ax,1
        rcl     dx,1
        loop    .Lf065
        mov     bx,WORD PTR [bp-94]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     bx,WORD PTR [bp-84]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-86]
.Lf066:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf067:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf067
        les     bx,DWORD PTR [bp-16]
        call    __U4M_V02
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
        mov     ax,WORD PTR [bp-96]
        or      ax,WORD PTR [bp-20]
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
        mov     bx,WORD PTR [bp-20]
        sub     bx,WORD PTR [bp-106]
        mov     cx,WORD PTR [bp-96]
        sbb     cx,WORD PTR [bp-104]
        mov     ax,WORD PTR [bp+24]
        mov     dx,WORD PTR [bp+26]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR es:[bx]
        mov     BYTE PTR [bp-2],al
        mov     bx,WORD PTR [bp-20]
        mov     cx,WORD PTR [bp-96]
        add     WORD PTR [bp-20],0x1
        adc     WORD PTR [bp-96],0x0
        mov     ax,WORD PTR [bp+24]
        mov     dx,WORD PTR [bp+26]
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
        mov     ax,WORD PTR [bp-36]
        add     ax,0x1b0
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     ax,dx
        mov     WORD PTR [bp-16],ax
        cmp     di,0x100
        jae     .Lf078
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf077:
        shl     ax,1
        rcl     dx,1
        loop    .Lf077
        mov     bx,WORD PTR [bp-94]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
        mov     bx,WORD PTR [bp-84]
        mov     WORD PTR [bp-12],bx
        movw    ds,WORD PTR [bp-86]
.Lf078:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf079:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf079
        les     bx,DWORD PTR [bp-16]
        call    __U4M_V02
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
        mov     ax,WORD PTR [bp-58]
        mov     cx,WORD PTR [bp-52]
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
        mov     dx,WORD PTR [bp-36]
        add     dx,0x1c8
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        cmp     di,0x100
        jae     .Lf085
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf084:
        shl     ax,1
        rcl     dx,1
        loop    .Lf084
        call    __PIA_V04
.Lf085:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf086:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf086
        les     bx,DWORD PTR [bp-16]
        call    __U4M_V02
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
        mov     ax,WORD PTR [bp-76]
        mov     cx,WORD PTR [bp-64]
        mov     dx,WORD PTR [bp-56]
        mov     WORD PTR [bp-76],dx
        mov     dx,WORD PTR [bp-54]
        mov     WORD PTR [bp-64],dx
.Lf089:
        mov     dx,WORD PTR [bp-58]
        mov     WORD PTR [bp-56],dx
        mov     dx,WORD PTR [bp-52]
        mov     WORD PTR [bp-54],dx
.Lf090:
        mov     dx,WORD PTR [bp-106]
        mov     WORD PTR [bp-58],dx
        mov     dx,WORD PTR [bp-104]
        mov     WORD PTR [bp-52],dx
        mov     WORD PTR [bp-106],ax
        mov     WORD PTR [bp-104],cx
.Lf091:
        cmp     WORD PTR [bp-18],0x7
        jge     .Lf092
        mov     ax,0x8
        jmp     .Lf093
.Lf092:
        mov     ax,0xb
.Lf093:
        mov     WORD PTR [bp-18],ax
        mov     ax,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-36]
        add     bx,0xa68
.Lf094:
        mov     WORD PTR [bp-16],bx
        mov     bx,WORD PTR [bp-16]
        mov     WORD PTR [bp-98],bx
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-30],ax
        cmp     di,0x100
        jae     .Lf098
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf097:
        shl     ax,1
        rcl     dx,1
        loop    .Lf097
        call    __PIA_V04
.Lf098:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf099:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf099
        movw    es,WORD PTR [bp-30]
        mov     bx,WORD PTR [bp-98]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-98]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     cl,0x4
        mov     ax,WORD PTR [bp-60]
        shl     ax,cl
        mov     dx,WORD PTR [bp-16]
        add     dx,0x4
        mov     bx,WORD PTR [bp-14]
        mov     WORD PTR [bp-30],bx
        add     dx,ax
        mov     WORD PTR [bp-98],dx
        xor     ax,ax
        mov     WORD PTR [bp-74],ax
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
        mov     bx,WORD PTR [bp-98]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-30],ax
        mov     bx,WORD PTR [bp-16]
        inc     bx
        inc     bx
        mov     WORD PTR [bp-98],bx
        cmp     di,0x100
        jae     .Lf106
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf105:
        shl     ax,1
        rcl     dx,1
        loop    .Lf105
        call    __PIA_V04
.Lf106:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf107:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf107
        movw    es,WORD PTR [bp-30]
        mov     bx,WORD PTR [bp-98]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-98]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        mov     cl,0x4
        mov     ax,WORD PTR [bp-60]
        shl     ax,cl
        mov     dx,WORD PTR [bp-16]
        add     dx,0x104
        mov     bx,WORD PTR [bp-14]
        mov     WORD PTR [bp-30],bx
        add     dx,ax
        mov     WORD PTR [bp-98],dx
        mov     WORD PTR [bp-74],0x8
        jmp     .Lf101
.Lf109:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-98]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-14]
        mov     WORD PTR [bp-30],ax
        mov     bx,WORD PTR [bp-16]
        add     bx,0x204
        mov     WORD PTR [bp-98],bx
        mov     WORD PTR [bp-74],0x10
        mov     WORD PTR [bp-62],0x8
.Lf110:
        mov     ax,WORD PTR [bp-62]
        mov     WORD PTR [bp-80],ax
        mov     WORD PTR [bp-22],0x1
        movw    es,WORD PTR [bp-30]
.Lf111:
        mov     ax,WORD PTR [bp-22]
        shl     ax,1
        mov     bx,WORD PTR [bp-98]
        add     bx,ax
        mov     WORD PTR [bp-46],bx
        cmp     di,0x100
        jae     .Lf115
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf114:
        shl     ax,1
        rcl     dx,1
        loop    .Lf114
        call    __PIA_V04
.Lf115:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf116:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf116
        mov     bx,WORD PTR [bp-46]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-46]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-22],1
        jmp     .Lf119
.Lf118:
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
        mov     ax,WORD PTR [bp-22]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-22],ax
.Lf119:
        dec     WORD PTR [bp-80]
        je      .Lf120
        jmp     .Lf111
.Lf120:
        mov     cl,BYTE PTR [bp-62]
        mov     ax,0x1
        shl     ax,cl
        sub     WORD PTR [bp-22],ax
        mov     ax,WORD PTR [bp-74]
        add     WORD PTR [bp-22],ax
        cmp     WORD PTR [bp-18],0x4
        jl      .Lf121
        jmp     .Lf159
.Lf121:
        add     WORD PTR [bp-18],0x7
        mov     ax,WORD PTR [bp-22]
        cmp     ax,0x4
        jl      .Lf122
        mov     ax,0x3
.Lf122:
        mov     cl,0x7
        shl     ax,cl
        mov     dx,WORD PTR [bp-36]
        add     dx,0x360
        mov     bx,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],bx
        add     dx,ax
        mov     WORD PTR [bp-16],dx
        mov     WORD PTR [bp-78],0x6
        mov     WORD PTR [bp-32],0x1
        movw    es,WORD PTR [bp-14]
.Lf123:
        mov     ax,WORD PTR [bp-32]
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-40],bx
        cmp     di,0x100
        jae     .Lf127
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf126:
        shl     ax,1
        rcl     dx,1
        loop    .Lf126
        call    __PIA_V04
.Lf127:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf128:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf128
        mov     bx,WORD PTR [bp-40]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-40]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-32],1
        jmp     .Lf131
.Lf130:
        sub     si,WORD PTR [bp-10]
        sbb     di,dx
        mov     ax,WORD PTR [bp-10]
        sub     WORD PTR [bp-8],ax
        sbb     WORD PTR [bp-4],dx
        mov     cl,0x5
        mov     bx,WORD PTR [bp-40]
        mov     ax,WORD PTR es:[bx]
        shr     ax,cl
        sub     WORD PTR es:[bx],ax
        mov     ax,WORD PTR [bp-32]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-32],ax
.Lf131:
        dec     WORD PTR [bp-78]
        je      .Lf132
        jmp     .Lf123
.Lf132:
        sub     WORD PTR [bp-32],0x40
        mov     ax,WORD PTR [bp-32]
        cmp     ax,0x4
        jl      .Lf135
        sar     ax,1
        dec     ax
        mov     WORD PTR [bp-38],ax
        mov     ax,WORD PTR [bp-32]
        and     ax,0x1
        or      al,0x2
        mov     WORD PTR [bp-106],ax
        xor     ax,ax
        mov     WORD PTR [bp-104],ax
        cmp     WORD PTR [bp-32],0xe
        jge     .Lf136
        mov     cx,WORD PTR [bp-38]
        jcxz    .Lf134
.Lf133:
        shl     WORD PTR [bp-106],1
        rcl     WORD PTR [bp-104],1
        loop    .Lf133
.Lf134:
        mov     dx,WORD PTR [bp-106]
        shl     dx,1
        mov     ax,WORD PTR [bp-36]
        add     ax,0x560
        add     dx,ax
        mov     ax,WORD PTR [bp-32]
        shl     ax,1
        sub     dx,ax
        mov     ax,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],ax
        dec     dx
        dec     dx
        mov     WORD PTR [bp-16],dx
        jmp     .Lf146
.Lf135:
        jmp     .Lf157
.Lf136:
        sub     WORD PTR [bp-38],0x4
.Lf137:
        cmp     di,0x100
        jae     .Lf141
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf140:
        shl     ax,1
        rcl     dx,1
        loop    .Lf140
        call    __PIA_V04
.Lf141:
        shr     di,1
        rcr     si,1
        shl     WORD PTR [bp-106],1
        rcl     WORD PTR [bp-104],1
        mov     ax,WORD PTR [bp-4]
        cmp     di,ax
        jb      .Lf142
        jne     .Lf143
        cmp     si,WORD PTR [bp-8]
        ja      .Lf143
.Lf142:
        sub     WORD PTR [bp-8],si
        sbb     WORD PTR [bp-4],di
        or      BYTE PTR [bp-106],0x1
.Lf143:
        dec     WORD PTR [bp-38]
        je      .Lf144
        jmp     .Lf137
.Lf144:
        mov     ax,WORD PTR [bp-34]
        mov     WORD PTR [bp-14],ax
        mov     bx,WORD PTR [bp-36]
        add     bx,0x644
        mov     WORD PTR [bp-16],bx
        mov     cx,0x4
.Lf145:
        shl     WORD PTR [bp-106],1
        rcl     WORD PTR [bp-104],1
        loop    .Lf145
        mov     WORD PTR [bp-38],0x4
.Lf146:
        mov     ax,0x1
        mov     WORD PTR [bp-70],ax
        mov     WORD PTR [bp-48],ax
        movw    es,WORD PTR [bp-14]
.Lf147:
        mov     ax,WORD PTR [bp-48]
        shl     ax,1
        mov     bx,WORD PTR [bp-16]
        add     bx,ax
        mov     WORD PTR [bp-50],bx
        cmp     di,0x100
        jae     .Lf151
        call    __PTC_V01
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
        mov     BYTE PTR [bp-94],al
        mov     BYTE PTR [bp-93],0x0
        mov     ax,WORD PTR [bp-8]
        mov     dx,WORD PTR [bp-4]
        mov     cx,0x8
.Lf150:
        shl     ax,1
        rcl     dx,1
        loop    .Lf150
        call    __PIA_V04
.Lf151:
        mov     WORD PTR [bp-102],si
        mov     WORD PTR [bp-100],di
        mov     cx,0xb
.Lf152:
        shr     WORD PTR [bp-100],1
        rcr     WORD PTR [bp-102],1
        loop    .Lf152
        mov     bx,WORD PTR [bp-50]
        call    __U4M_V02
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
        mov     bx,WORD PTR [bp-50]
        sub     ax,WORD PTR es:[bx]
        mov     cl,0x5
        shr     ax,cl
        add     WORD PTR es:[bx],ax
        shl     WORD PTR [bp-48],1
        jmp     .Lf155
.Lf154:
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
        mov     ax,WORD PTR [bp-48]
        add     ax,ax
        inc     ax
        mov     WORD PTR [bp-48],ax
        mov     ax,WORD PTR [bp-70]
        cwd
        or      WORD PTR [bp-106],ax
        or      WORD PTR [bp-104],dx
.Lf155:
        shl     WORD PTR [bp-70],1
        dec     WORD PTR [bp-38]
        je      .Lf156
        jmp     .Lf147
.Lf156:
        jmp     .Lf158
.Lf157:
        cwd
        mov     WORD PTR [bp-106],ax
        mov     WORD PTR [bp-104],dx
.Lf158:
        add     WORD PTR [bp-106],0x1
        adc     WORD PTR [bp-104],0x0
        mov     ax,WORD PTR [bp-104]
        or      ax,WORD PTR [bp-106]
        jne     .Lf159
        jmp     .Lf165
.Lf159:
        add     WORD PTR [bp-22],0x2
        mov     ax,WORD PTR [bp-104]
        cmp     ax,WORD PTR [bp-96]
        jbe     .Lf161
.Lf160:
        jmp     .Lf008
.Lf161:
        jne     .Lf162
        mov     ax,WORD PTR [bp-106]
        cmp     ax,WORD PTR [bp-20]
        ja      .Lf160
.Lf162:
        mov     bx,WORD PTR [bp-20]
        sub     bx,WORD PTR [bp-106]
        mov     cx,WORD PTR [bp-96]
        sbb     cx,WORD PTR [bp-104]
        mov     ax,WORD PTR [bp+24]
        mov     dx,WORD PTR [bp+26]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR es:[bx]
        mov     BYTE PTR [bp-2],al
        mov     bx,WORD PTR [bp-20]
        mov     cx,WORD PTR [bp-96]
        dec     WORD PTR [bp-22]
        add     WORD PTR [bp-20],0x1
        adc     WORD PTR [bp-96],0x0
        mov     ax,WORD PTR [bp+24]
        mov     dx,WORD PTR [bp+26]
        call    __PIA
        mov     bx,ax
        movw    es,dx
        mov     al,BYTE PTR [bp-2]
        mov     BYTE PTR es:[bx],al
        cmp     WORD PTR [bp-22],0x0
        jne     .Lf164
.Lf163:
        jmp     .Lf009
.Lf164:
        mov     ax,WORD PTR [bp-96]
        cmp     ax,WORD PTR [bp+30]
        jb      .Lf162
        jne     .Lf163
        mov     ax,WORD PTR [bp-20]
        cmp     ax,WORD PTR [bp+28]
        jb      .Lf162
        jmp     .Lf163
.Lf165:
        cmp     di,0x100
        jae     .Lf167
        call    __PTC_V01
        jne     .Lf166
        jmp     .Lf008
.Lf166:
        call    __PIA_V03
.Lf167:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp+12]
        mov     cx,WORD PTR [bp+14]
        WCC_PTS
        lds     bx,DWORD PTR [bp+20]
        mov     WORD PTR [bx],ax
        mov     WORD PTR [bx+2],dx
        mov     ax,WORD PTR [bp-20]
        lds     bx,DWORD PTR [bp+32]
        mov     WORD PTR [bx],ax
        mov     ax,WORD PTR [bp-96]
        mov     WORD PTR [bx+2],ax
        xor     ax,ax
.Lf168:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

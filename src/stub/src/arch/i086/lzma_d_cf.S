        push    si
        push    di
        push    bp
        mov     bp,sp
        sub     sp,102
        mov     si,word ptr [bp+8]
        lea     bx,[si+4]
        mov     word ptr [bp-34],bx
        xor     ax,ax
        mov     word ptr [bp-20],ax
        mov     word ptr [bp-18],ax
        mov     byte ptr [bp-2],0
        mov     cl,byte ptr [si+2]
        mov     ax,1
        shl     ax,cl
        dec     ax
        mov     word ptr [bp-88],ax
        mov     cl,byte ptr [si+1]
        mov     ax,1
        shl     ax,cl
        dec     ax
        cwd
        mov     word ptr [bp-86],ax
        mov     word ptr [bp-84],dx
        xor     ax,ax
        mov     word ptr [bp-16],ax
        mov     word ptr [bp-100],ax
        mov     word ptr [bp-50],ax
        mov     word ptr [bp-54],ax
        mov     word ptr [bp-74],ax
        mov     bx,word ptr [bp+18]
        mov     word ptr [bx],ax
        mov     word ptr [bx+2],ax
        mov     bx,word ptr [bp+28]
        mov     word ptr [bx],ax
        mov     word ptr [bx+2],ax
        inc     ax
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-52],ax
        mov     word ptr [bp-56],ax
        mov     word ptr [bp-68],ax
        mov     al,byte ptr [si]
        mov     word ptr [bp-78],ax
        mov     dl,byte ptr [si+1]
        xor     dh,dh
        mov     cx,ax
        add     cx,dx
        mov     ax,768
        xor     dl,dl
        jcxz    .Lf001
.Lf000:
        shl     ax,1
        rcl     dx,1
        loop    .Lf000
.Lf001:
        mov     si,ax
        add     si,1846
        mov     cx,dx
        adc     cx,0
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
        mov     word ptr [bx],1024
        add     dx,1
        adc     ax,0
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
        mov     si,65535
        mov     di,si
        mov     word ptr [bp-72],ax
        jmp     .Lf006
.Lf005:
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
        inc     word ptr [bp-72]
        cmp     word ptr [bp-72],5
        jge     .Lf008
.Lf006:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf005
.Lf007:
        mov     ax,1
        jmp     .Lf125
.Lf008:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Lf011
        je      .Lf010
.Lf009:
        jmp     .Lf122
.Lf010:
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jae     .Lf009
.Lf011:
        mov     ax,word ptr [bp-20]
        and     ax,word ptr [bp-88]
        mov     word ptr [bp-58],ax
        mov     cl,5
        mov     dx,word ptr [bp-16]
        shl     dx,cl
        add     dx,word ptr [bp-34]
        shl     ax,1
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,256
        jae     .Lf012
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        je      .Lf007
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf012:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf015
        je      .Lf014
.Lf013:
        jmp     .Lf034
.Lf014:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf013
.Lf015:
        mov     word ptr [bp-26],1
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     cx,8
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
        jcxz    .Lf017
.Lf016:
        shl     ax,1
        rcl     dx,1
        loop    .Lf016
.Lf017:
        add     ax,word ptr [bp-90]
        adc     dx,bx
        M_U4M_dxax_0x0600
        mov     dx,word ptr [bp-34]
        add     dx,3692
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     word ptr [bp-16],7
        jge     .Lf018
        jmp     .Lf025
.Lf018:
        mov     bx,word ptr [bp-20]
        sub     bx,word ptr [bp-102]
        mov     cx,word ptr [bp-18]
        sbb     cx,word ptr [bp-100]
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        mov     es,dx
        mov     al,byte ptr es:[bx]
        xor     ah,ah
        mov     word ptr [bp-76],ax
.Lf019:
        shl     word ptr [bp-76],1
        mov     ax,word ptr [bp-76]
        xor     al,al
        and     ah,1
        mov     word ptr [bp-66],ax
        shl     ax,1
        mov     dx,word ptr [bp-14]
        add     dh,2
        add     dx,ax
        mov     ax,word ptr [bp-26]
        shl     ax,1
        add     dx,ax
        mov     word ptr [bp-46],dx
        cmp     di,256
        jae     .Lf021
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf020
        jmp     .Lf007
.Lf020:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf021:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-46]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf022
        jne     .Lf023
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf023
.Lf022:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-46]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        cmp     word ptr [bp-66],0
        jne     .Lf025
        jmp     .Lf024
.Lf023:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-46]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-26]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-26],ax
        cmp     word ptr [bp-66],0
        je      .Lf025
.Lf024:
        cmp     word ptr [bp-26],256
        jge     .Lf025
        jmp     .Lf019
.Lf025:
        mov     ax,word ptr [bp-26]
        cmp     ax,256
        jl      .Lf026
        jmp     .Lf031
.Lf026:
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-40],bx
        cmp     di,256
        jae     .Lf028
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf027
        jmp     .Lf007
.Lf027:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf028:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-40]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf029
        jne     .Lf030
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf030
.Lf029:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-40]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        jmp     .Lf025
.Lf030:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-40]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-26]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-26],ax
        jmp     .Lf025
.Lf031:
        mov     al,byte ptr [bp-26]
        mov     byte ptr [bp-2],al
        mov     bx,word ptr [bp-20]
        mov     cx,word ptr [bp-18]
        add     word ptr [bp-20],1
        adc     word ptr [bp-18],0
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        mov     es,dx
        mov     al,byte ptr [bp-26]
        mov     byte ptr es:[bx],al
        mov     ax,word ptr [bp-16]
        cmp     ax,4
        jge     .Lf032
        xor     ax,ax
        mov     word ptr [bp-16],ax
        jmp     .Lf008
.Lf032:
        cmp     ax,10
        jge     .Lf033
        sub     word ptr [bp-16],3
        jmp     .Lf008
.Lf033:
        sub     word ptr [bp-16],6
        jmp     .Lf008
.Lf034:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-16]
        shl     ax,1
        mov     dx,word ptr [bp-34]
        add     dx,384
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,256
        jae     .Lf036
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf035
        jmp     .Lf007
.Lf035:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf036:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf037
        jne     .Lf040
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf040
.Lf037:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,5
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
        cmp     word ptr [bp-16],7
        jge     .Lf038
        xor     ax,ax
        jmp     .Lf039
.Lf038:
        mov     ax,3
.Lf039:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,1636
        jmp     .Lf068
.Lf040:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     dx,word ptr [bp-16]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,408
        add     ax,dx
        mov     word ptr [bp-14],ax
        cmp     di,256
        jae     .Lf042
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf041
        jmp     .Lf007
.Lf041:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf042:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        M_PIA1
        mov     word ptr [bp-82],ax
        mov     word ptr [bp-80],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,word ptr [bp-94]
        jb      .Lf045
        je      .Lf044
.Lf043:
        jmp     .Lf054
.Lf044:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf043
.Lf045:
        mov     si,word ptr [bp-8]
        mov     di,word ptr [bp-94]
        mov     ax,2048
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-16]
        shl     ax,cl
        mov     dx,word ptr [bp-34]
        add     dx,480
        add     dx,ax
        mov     ax,word ptr [bp-58]
        shl     ax,1
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,256
        jae     .Lf047
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf046
        jmp     .Lf007
.Lf046:
        mov     ax,si
        M_shld_diax_8_bxcx
        mov     si,ax
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
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
.Lf047:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf048
        jne     .Lf050
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf053
.Lf048:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-18]
        or      ax,word ptr [bp-20]
        jne     .Lf049
        jmp     .Lf007
.Lf049:
        cmp     word ptr [bp-16],7
        jge     .Lf051
        mov     ax,9
        jmp     .Lf052
.Lf050:
        jmp     .Lf053
.Lf051:
        mov     ax,11
.Lf052:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-20]
        sub     bx,word ptr [bp-102]
        mov     cx,word ptr [bp-18]
        sbb     cx,word ptr [bp-100]
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        mov     es,dx
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-2],al
        mov     bx,word ptr [bp-20]
        mov     cx,word ptr [bp-18]
        add     word ptr [bp-20],1
        adc     word ptr [bp-18],0
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        mov     es,dx
        mov     al,byte ptr [bp-2]
        mov     byte ptr es:[bx],al
        jmp     .Lf008
.Lf053:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        jmp     .Lf065
.Lf054:
        sub     si,word ptr [bp-8]
        sbb     di,word ptr [bp-94]
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        mov     ax,word ptr [bp-94]
        sbb     word ptr [bp-4],ax
        mov     cl,5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-16]
        shl     ax,1
        mov     dx,word ptr [bp-34]
        add     dx,432
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     di,256
        jae     .Lf056
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf055
        jmp     .Lf007
.Lf055:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
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
.Lf056:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf057
        jne     .Lf058
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf058
.Lf057:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-52]
        mov     cx,word ptr [bp-50]
        jmp     .Lf064
.Lf058:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-14]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     dx,word ptr [bp-16]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,456
        add     ax,dx
        mov     word ptr [bp-14],ax
        cmp     di,256
        jae     .Lf060
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf059
        jmp     .Lf007
.Lf059:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf060:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf061
        jne     .Lf062
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf062
.Lf061:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-14]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     ax,word ptr [bp-56]
        mov     cx,word ptr [bp-54]
        jmp     .Lf063
.Lf062:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
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
.Lf063:
        mov     dx,word ptr [bp-52]
        mov     word ptr [bp-56],dx
        mov     dx,word ptr [bp-50]
        mov     word ptr [bp-54],dx
.Lf064:
        mov     dx,word ptr [bp-102]
        mov     word ptr [bp-52],dx
        mov     dx,word ptr [bp-100]
        mov     word ptr [bp-50],dx
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],cx
.Lf065:
        cmp     word ptr [bp-16],7
        jge     .Lf066
        mov     ax,8
        jmp     .Lf067
.Lf066:
        mov     ax,11
.Lf067:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,2664
.Lf068:
        mov     word ptr [bp-14],bx
        mov     bx,word ptr [bp-14]
        mov     word ptr [bp-22],bx
        cmp     di,256
        jae     .Lf070
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf069
        jmp     .Lf007
.Lf069:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf070:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf071
        jne     .Lf073
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf073
.Lf071:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-22]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     cl,4
        mov     ax,word ptr [bp-58]
        shl     ax,cl
        mov     dx,word ptr [bp-14]
        add     dx,4
        add     dx,ax
        mov     word ptr [bp-22],dx
        xor     ax,ax
        mov     word ptr [bp-70],ax
.Lf072:
        mov     word ptr [bp-60],3
        jmp     .Lf078
.Lf073:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-22]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     bx,word ptr [bp-14]
        inc     bx
        inc     bx
        mov     word ptr [bp-22],bx
        cmp     di,256
        jae     .Lf075
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf074
        jmp     .Lf007
.Lf074:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf075:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf076
        jne     .Lf077
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf077
.Lf076:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-22]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        mov     cl,4
        mov     ax,word ptr [bp-58]
        shl     ax,cl
        mov     dx,word ptr [bp-14]
        add     dx,260
        add     dx,ax
        mov     word ptr [bp-22],dx
        mov     word ptr [bp-70],8
        jmp     .Lf072
.Lf077:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-22]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     bx,word ptr [bp-14]
        add     bx,516
        mov     word ptr [bp-22],bx
        mov     word ptr [bp-70],16
        mov     word ptr [bp-60],8
.Lf078:
        mov     ax,word ptr [bp-60]
        mov     word ptr [bp-62],ax
        mov     word ptr [bp-24],1
.Lf079:
        mov     ax,word ptr [bp-24]
        shl     ax,1
        mov     bx,word ptr [bp-22]
        add     bx,ax
        mov     word ptr [bp-38],bx
        cmp     di,256
        jae     .Lf081
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf080
        jmp     .Lf007
.Lf080:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf081:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-38]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf082
        jne     .Lf083
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf083
.Lf082:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-38]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-24],1
        jmp     .Lf084
.Lf083:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-38]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-24]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-24],ax
.Lf084:
        dec     word ptr [bp-62]
        je      .Lf085
        jmp     .Lf079
.Lf085:
        mov     cl,byte ptr [bp-60]
        mov     ax,1
        shl     ax,cl
        sub     word ptr [bp-24],ax
        mov     ax,word ptr [bp-70]
        add     word ptr [bp-24],ax
        cmp     word ptr [bp-16],4
        jl      .Lf086
        jmp     .Lf116
.Lf086:
        add     word ptr [bp-16],7
        mov     ax,word ptr [bp-24]
        cmp     ax,4
        jl      .Lf087
        mov     ax,3
.Lf087:
        mov     cl,7
        mov     dx,ax
        shl     dx,cl
        mov     ax,word ptr [bp-34]
        add     ax,864
        add     ax,dx
        mov     word ptr [bp-14],ax
        mov     word ptr [bp-92],6
        mov     word ptr [bp-32],1
.Lf088:
        mov     ax,word ptr [bp-32]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-42],bx
        cmp     di,256
        jae     .Lf090
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf089
        jmp     .Lf007
.Lf089:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf090:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-42]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf091
        jne     .Lf092
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf092
.Lf091:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-42]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-32],1
        jmp     .Lf093
.Lf092:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
        mov     bx,word ptr [bp-42]
        mov     ax,word ptr [bx]
        shr     ax,cl
        sub     word ptr [bx],ax
        mov     ax,word ptr [bp-32]
        add     ax,ax
        inc     ax
        mov     word ptr [bp-32],ax
.Lf093:
        dec     word ptr [bp-92]
        je      .Lf094
        jmp     .Lf088
.Lf094:
        sub     word ptr [bp-32],64
        mov     ax,word ptr [bp-32]
        cmp     ax,4
        jl      .Lf097
        sar     ax,1
        dec     ax
        mov     word ptr [bp-36],ax
        mov     ax,word ptr [bp-32]
        and     ax,1
        or      al,2
        mov     word ptr [bp-102],ax
        xor     ax,ax
        mov     word ptr [bp-100],ax
        cmp     word ptr [bp-32],14
        jge     .Lf098
        mov     cx,word ptr [bp-36]
        jcxz    .Lf096
.Lf095:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Lf095
.Lf096:
        mov     dx,word ptr [bp-102]
        shl     dx,1
        mov     ax,word ptr [bp-34]
        add     ax,1376
        add     dx,ax
        mov     ax,word ptr [bp-32]
        shl     ax,1
        sub     dx,ax
        dec     dx
        dec     dx
        mov     word ptr [bp-14],dx
        jmp     .Lf106
.Lf097:
        jmp     .Lf114
.Lf098:
        sub     word ptr [bp-36],4
.Lf099:
        cmp     di,256
        jae     .Lf101
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf100
        jmp     .Lf007
.Lf100:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf101:
        shr     di,1
        rcr     si,1
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        mov     ax,word ptr [bp-4]
        cmp     di,ax
        jb      .Lf102
        jne     .Lf103
        cmp     si,word ptr [bp-6]
        ja      .Lf103
.Lf102:
        sub     word ptr [bp-6],si
        sbb     word ptr [bp-4],di
        or      byte ptr [bp-102],1
.Lf103:
        dec     word ptr [bp-36]
        je      .Lf104
        jmp     .Lf099
.Lf104:
        mov     bx,word ptr [bp-34]
        add     bx,1604
        mov     word ptr [bp-14],bx
        mov     cx,4
.Lf105:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Lf105
        mov     word ptr [bp-36],4
.Lf106:
        mov     ax,1
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-48],ax
.Lf107:
        mov     ax,word ptr [bp-48]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-44],bx
        cmp     di,256
        jae     .Lf109
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Lf108
        jmp     .Lf007
.Lf108:
        M_shld_disi_8_bxcx
        les     bx,dword ptr [bp-12]
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-90],al
        mov     byte ptr [bp-89],0
        mov     ax,word ptr [bp-6]
        mov     dx,word ptr [bp-4]
        M_shld_8
        mov     bx,word ptr [bp-90]
        or      bx,ax
        mov     word ptr [bp-6],bx
        mov     word ptr [bp-4],dx
        mov     ax,word ptr [bp-12]
        mov     dx,es
        M_PIA1
        mov     word ptr [bp-12],ax
        mov     word ptr [bp-10],dx
.Lf109:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-44]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Lf110
        jne     .Lf111
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Lf111
.Lf110:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-44]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-48],1
        jmp     .Lf112
.Lf111:
        sub     si,word ptr [bp-8]
        sbb     di,dx
        mov     ax,word ptr [bp-8]
        sub     word ptr [bp-6],ax
        sbb     word ptr [bp-4],dx
        mov     cl,5
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
.Lf112:
        shl     word ptr [bp-64],1
        dec     word ptr [bp-36]
        je      .Lf113
        jmp     .Lf107
.Lf113:
        jmp     .Lf115
.Lf114:
        cwd
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],dx
.Lf115:
        add     word ptr [bp-102],1
        adc     word ptr [bp-100],0
        mov     ax,word ptr [bp-100]
        or      ax,word ptr [bp-102]
        jne     .Lf116
        jmp     .Lf122
.Lf116:
        add     word ptr [bp-24],2
        mov     ax,word ptr [bp-100]
        cmp     ax,word ptr [bp-18]
        jbe     .Lf118
.Lf117:
        jmp     .Lf007
.Lf118:
        jne     .Lf119
        mov     ax,word ptr [bp-102]
        cmp     ax,word ptr [bp-20]
        ja      .Lf117
.Lf119:
        mov     bx,word ptr [bp-20]
        sub     bx,word ptr [bp-102]
        mov     cx,word ptr [bp-18]
        sbb     cx,word ptr [bp-100]
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        mov     es,dx
        mov     al,byte ptr es:[bx]
        mov     byte ptr [bp-2],al
        mov     bx,word ptr [bp-20]
        mov     cx,word ptr [bp-18]
        dec     word ptr [bp-24]
        add     word ptr [bp-20],1
        adc     word ptr [bp-18],0
        mov     ax,word ptr [bp+20]
        mov     dx,word ptr [bp+22]
        M_PIA
        mov     bx,ax
        mov     es,dx
        mov     al,byte ptr [bp-2]
        mov     byte ptr es:[bx],al
        cmp     word ptr [bp-24],0
        jne     .Lf121
.Lf120:
        jmp     .Lf008
.Lf121:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Lf119
        jne     .Lf120
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jb      .Lf119
        jmp     .Lf120
.Lf122:
.Lf123:
.Lf124:
.Lf125:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

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
        jcxz    .Ls001
.Ls000:
        shl     ax,1
        rcl     dx,1
        loop    .Ls000
.Ls001:
        mov     si,ax
        add     si,1846
        mov     cx,dx
        adc     cx,0
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
        mov     word ptr [bx],1024
        add     dx,1
        adc     ax,0
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
        mov     si,65535
        mov     di,si
        mov     word ptr [bp-72],ax
        jmp     .Ls006
.Ls005:
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
        jge     .Ls008
.Ls006:
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls005
.Ls007:
        mov     ax,1
        jmp     .Ls125
.Ls008:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Ls011
        je      .Ls010
.Ls009:
        jmp     .Ls122
.Ls010:
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jae     .Ls009
.Ls011:
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
        jae     .Ls012
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC
        je      .Ls007
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
.Ls012:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls015
        je      .Ls014
.Ls013:
        jmp     .Ls034
.Ls014:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls013
.Ls015:
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
        jcxz    .Ls017
.Ls016:
        shl     ax,1
        rcl     dx,1
        loop    .Ls016
.Ls017:
        add     ax,word ptr [bp-90]
        adc     dx,bx
        M_U4M_dxax_0x0600
        mov     dx,word ptr [bp-34]
        add     dx,3692
        add     dx,ax
        mov     word ptr [bp-14],dx
        cmp     word ptr [bp-16],7
        jge     .Ls018
        jmp     .Ls025
.Ls018:
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
.Ls019:
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
        jae     .Ls021
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls020
        jmp     .Ls007
.Ls020:
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
.Ls021:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-46]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls022
        jne     .Ls023
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls023
.Ls022:
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
        jne     .Ls025
        jmp     .Ls024
.Ls023:
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
        je      .Ls025
.Ls024:
        cmp     word ptr [bp-26],256
        jge     .Ls025
        jmp     .Ls019
.Ls025:
        mov     ax,word ptr [bp-26]
        cmp     ax,256
        jl      .Ls026
        jmp     .Ls031
.Ls026:
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-40],bx
        cmp     di,256
        jae     .Ls028
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls027
        jmp     .Ls007
.Ls027:
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
.Ls028:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-40]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls029
        jne     .Ls030
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls030
.Ls029:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-40]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-26],1
        jmp     .Ls025
.Ls030:
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
        jmp     .Ls025
.Ls031:
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
        jge     .Ls032
        xor     ax,ax
        mov     word ptr [bp-16],ax
        jmp     .Ls008
.Ls032:
        cmp     ax,10
        jge     .Ls033
        sub     word ptr [bp-16],3
        jmp     .Ls008
.Ls033:
        sub     word ptr [bp-16],6
        jmp     .Ls008
.Ls034:
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
        jae     .Ls036
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls035
        jmp     .Ls007
.Ls035:
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
.Ls036:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls037
        jne     .Ls040
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls040
.Ls037:
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
        jge     .Ls038
        xor     ax,ax
        jmp     .Ls039
.Ls038:
        mov     ax,3
.Ls039:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,1636
        jmp     .Ls068
.Ls040:
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
        jae     .Ls042
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls041
        jmp     .Ls007
.Ls041:
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
.Ls042:
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
        jb      .Ls045
        je      .Ls044
.Ls043:
        jmp     .Ls054
.Ls044:
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls043
.Ls045:
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
        jae     .Ls047
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls046
        jmp     .Ls007
.Ls046:
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
.Ls047:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls048
        jne     .Ls050
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls053
.Ls048:
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
        jne     .Ls049
        jmp     .Ls007
.Ls049:
        cmp     word ptr [bp-16],7
        jge     .Ls051
        mov     ax,9
        jmp     .Ls052
.Ls050:
        jmp     .Ls053
.Ls051:
        mov     ax,11
.Ls052:
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
        jmp     .Ls008
.Ls053:
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
        jmp     .Ls065
.Ls054:
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
        jae     .Ls056
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls055
        jmp     .Ls007
.Ls055:
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
.Ls056:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls057
        jne     .Ls058
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls058
.Ls057:
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
        jmp     .Ls064
.Ls058:
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
        jae     .Ls060
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls059
        jmp     .Ls007
.Ls059:
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
.Ls060:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-14]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls061
        jne     .Ls062
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls062
.Ls061:
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
        jmp     .Ls063
.Ls062:
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
.Ls063:
        mov     dx,word ptr [bp-52]
        mov     word ptr [bp-56],dx
        mov     dx,word ptr [bp-50]
        mov     word ptr [bp-54],dx
.Ls064:
        mov     dx,word ptr [bp-102]
        mov     word ptr [bp-52],dx
        mov     dx,word ptr [bp-100]
        mov     word ptr [bp-50],dx
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],cx
.Ls065:
        cmp     word ptr [bp-16],7
        jge     .Ls066
        mov     ax,8
        jmp     .Ls067
.Ls066:
        mov     ax,11
.Ls067:
        mov     word ptr [bp-16],ax
        mov     bx,word ptr [bp-34]
        add     bx,2664
.Ls068:
        mov     word ptr [bp-14],bx
        mov     bx,word ptr [bp-14]
        mov     word ptr [bp-22],bx
        cmp     di,256
        jae     .Ls070
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls069
        jmp     .Ls007
.Ls069:
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
.Ls070:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls071
        jne     .Ls073
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls073
.Ls071:
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
.Ls072:
        mov     word ptr [bp-60],3
        jmp     .Ls078
.Ls073:
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
        jae     .Ls075
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls074
        jmp     .Ls007
.Ls074:
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
.Ls075:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-22]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls076
        jne     .Ls077
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls077
.Ls076:
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
        jmp     .Ls072
.Ls077:
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
.Ls078:
        mov     ax,word ptr [bp-60]
        mov     word ptr [bp-62],ax
        mov     word ptr [bp-24],1
.Ls079:
        mov     ax,word ptr [bp-24]
        shl     ax,1
        mov     bx,word ptr [bp-22]
        add     bx,ax
        mov     word ptr [bp-38],bx
        cmp     di,256
        jae     .Ls081
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls080
        jmp     .Ls007
.Ls080:
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
.Ls081:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-38]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls082
        jne     .Ls083
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls083
.Ls082:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-38]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-24],1
        jmp     .Ls084
.Ls083:
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
.Ls084:
        dec     word ptr [bp-62]
        je      .Ls085
        jmp     .Ls079
.Ls085:
        mov     cl,byte ptr [bp-60]
        mov     ax,1
        shl     ax,cl
        sub     word ptr [bp-24],ax
        mov     ax,word ptr [bp-70]
        add     word ptr [bp-24],ax
        cmp     word ptr [bp-16],4
        jl      .Ls086
        jmp     .Ls116
.Ls086:
        add     word ptr [bp-16],7
        mov     ax,word ptr [bp-24]
        cmp     ax,4
        jl      .Ls087
        mov     ax,3
.Ls087:
        mov     cl,7
        mov     dx,ax
        shl     dx,cl
        mov     ax,word ptr [bp-34]
        add     ax,864
        add     ax,dx
        mov     word ptr [bp-14],ax
        mov     word ptr [bp-92],6
        mov     word ptr [bp-32],1
.Ls088:
        mov     ax,word ptr [bp-32]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-42],bx
        cmp     di,256
        jae     .Ls090
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls089
        jmp     .Ls007
.Ls089:
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
.Ls090:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-42]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls091
        jne     .Ls092
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls092
.Ls091:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-42]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-32],1
        jmp     .Ls093
.Ls092:
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
.Ls093:
        dec     word ptr [bp-92]
        je      .Ls094
        jmp     .Ls088
.Ls094:
        sub     word ptr [bp-32],64
        mov     ax,word ptr [bp-32]
        cmp     ax,4
        jl      .Ls097
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
        jge     .Ls098
        mov     cx,word ptr [bp-36]
        jcxz    .Ls096
.Ls095:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Ls095
.Ls096:
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
        jmp     .Ls106
.Ls097:
        jmp     .Ls114
.Ls098:
        sub     word ptr [bp-36],4
.Ls099:
        cmp     di,256
        jae     .Ls101
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls100
        jmp     .Ls007
.Ls100:
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
.Ls101:
        shr     di,1
        rcr     si,1
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        mov     ax,word ptr [bp-4]
        cmp     di,ax
        jb      .Ls102
        jne     .Ls103
        cmp     si,word ptr [bp-6]
        ja      .Ls103
.Ls102:
        sub     word ptr [bp-6],si
        sbb     word ptr [bp-4],di
        or      byte ptr [bp-102],1
.Ls103:
        dec     word ptr [bp-36]
        je      .Ls104
        jmp     .Ls099
.Ls104:
        mov     bx,word ptr [bp-34]
        add     bx,1604
        mov     word ptr [bp-14],bx
        mov     cx,4
.Ls105:
        shl     word ptr [bp-102],1
        rcl     word ptr [bp-100],1
        loop    .Ls105
        mov     word ptr [bp-36],4
.Ls106:
        mov     ax,1
        mov     word ptr [bp-64],ax
        mov     word ptr [bp-48],ax
.Ls107:
        mov     ax,word ptr [bp-48]
        shl     ax,1
        mov     bx,word ptr [bp-14]
        add     bx,ax
        mov     word ptr [bp-44],bx
        cmp     di,256
        jae     .Ls109
        mov     ax,word ptr [bp-12]
        mov     dx,word ptr [bp-10]
        mov     bx,word ptr [bp-30]
        mov     cx,word ptr [bp-28]
        M_PTC_JNE .Ls108
        jmp     .Ls007
.Ls108:
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
.Ls109:
        M_shrd_11_disi_bp -96 -98
        mov     bx,word ptr [bp-44]
        mov     bx,word ptr [bx]
        M_U4M_dxax_00bx
        mov     word ptr [bp-8],ax
        mov     word ptr [bp-94],dx
        mov     ax,word ptr [bp-4]
        cmp     ax,dx
        jb      .Ls110
        jne     .Ls111
        mov     ax,word ptr [bp-6]
        cmp     ax,word ptr [bp-8]
        jae     .Ls111
.Ls110:
        mov     si,word ptr [bp-8]
        mov     di,dx
        mov     ax,2048
        mov     bx,word ptr [bp-44]
        sub     ax,word ptr [bx]
        mov     cl,5
        shr     ax,cl
        add     word ptr [bx],ax
        shl     word ptr [bp-48],1
        jmp     .Ls112
.Ls111:
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
.Ls112:
        shl     word ptr [bp-64],1
        dec     word ptr [bp-36]
        je      .Ls113
        jmp     .Ls107
.Ls113:
        jmp     .Ls115
.Ls114:
        cwd
        mov     word ptr [bp-102],ax
        mov     word ptr [bp-100],dx
.Ls115:
        add     word ptr [bp-102],1
        adc     word ptr [bp-100],0
        mov     ax,word ptr [bp-100]
        or      ax,word ptr [bp-102]
        jne     .Ls116
        jmp     .Ls122
.Ls116:
        add     word ptr [bp-24],2
        mov     ax,word ptr [bp-100]
        cmp     ax,word ptr [bp-18]
        jbe     .Ls118
.Ls117:
        jmp     .Ls007
.Ls118:
        jne     .Ls119
        mov     ax,word ptr [bp-102]
        cmp     ax,word ptr [bp-20]
        ja      .Ls117
.Ls119:
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
        jne     .Ls121
.Ls120:
        jmp     .Ls008
.Ls121:
        mov     ax,word ptr [bp-18]
        cmp     ax,word ptr [bp+26]
        jb      .Ls119
        jne     .Ls120
        mov     ax,word ptr [bp-20]
        cmp     ax,word ptr [bp+24]
        jb      .Ls119
        jmp     .Ls120
.Ls122:
.Ls123:
.Ls124:
.Ls125:
        mov     sp,bp
        pop     bp
        pop     di
        pop     si
        ret

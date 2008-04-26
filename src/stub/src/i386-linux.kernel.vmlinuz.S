/*
;  i386-linux.kernel.vmlinuz.S -- loader & decompressor for the vmlinuz/i386 format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
*/

#include "arch/i386/macros.S"

// include/linux/screen_info.h
si_orig_x		= 0x00
si_orig_y		= 0x01
//si_dontuse1		= 0x02  // EXT_MEM_K sits here
si_orig_video_page	= 0x04
si_orig_video_mode	= 0x06
si_orig_video_cols	= 0x07
si_vidport		= 0x08  // FIXME  upx stealing
si_orig_video_ega_bx	= 0x0a
//si_unused3		= 0x0c
si_orig_video_lines	= 0x0e
si_orig_video_isVGA	= 0x0f
si_orig_video_points	= 0x10

// VESA graphic mode -- linear frame buffer
si_lfb_width		= 0x12
si_lfb_height		= 0x14
si_lfb_depth		= 0x16
si_lfb_base		= 0x18
si_lfb_size		= 0x1c

BP_cmd_line_magic       = 0x20  // protocol < 2.02, or if null pointer
BP_cmd_line_offset      = 0x22  // protocol < 2.02, or if null pointer

si_lfb_linelength	= 0x24
si_red_size		= 0x26
si_red_pos		= 0x27
si_green_size		= 0x28
si_green_pos		= 0x29
si_blue_size		= 0x2a
si_blue_pos		= 0x2b
si_rsvd_size		= 0x2c
si_rsvd_pos		= 0x2d
si_vesapm_seg		= 0x2e
si_vesapm_off		= 0x30
si_pages		= 0x32
si_vesa_attributes	= 0x34
si_capabilities		= 0x36

si_vidmem               = 0x3c  // FIXME upx stealing

// linux/Documentation/i386/boot.txt
BP_version              = 0x206  // boot protocol 2.00+ (kernel 1.3.73)
BP_loadflags            = 0x211  // 2.00+
   LOADED_HIGH   = (1<<0)
   KEEP_SEGMENTS = (1<<6)
BP_cmd_line_ptr         = 0x228  // 2.02+ (kernel 2.4.0-test3-pre3)
BP_cmdline_size         = 0x238

// =============
// ============= ENTRY POINT
// =============

/*
  How to debug: run under qemu (http://fabrice.bellard.free.fr/qemu/)
  after un-commenting the 0xf1 opcode(s) below.  That opcode forces qemu
  to stop in gdb.  You'll have to "set $pc+=1" by hand.
  Example:
    $ qemu -S -s -kernel vmlinuz.upx -m 256 -nographic -hda <filesys_image> &
    $ gdb -
    (gdb) target remote localhost:1234
    (gdb) continue   ## qemu internal breakpoint
    (gdb) set $pc+=1   ## our 0xf1 icebp
  [ gdb-6.6-43.fc8 has a bug https://bugzilla.redhat.com/show_bug.cgi?id=436037 ]
*/

section LINUZVGA  // debug print to VGA console
#if 0  /*{*/
    ////.byte 0xf1  // qemu In-Circuit-Emulator breakpoint
// choose which VGA region and port, depending on video mode
        mov eax,0xb0000
        mov dx,0x3b4
        cmpb [esi + si_orig_video_mode],7; je 0f
        movb ah,0x80  // 0xb8000
        movb dl,0xd4  // 0x3d4
0:
        mov [esi + si_vidmem],eax
        mov [esi + si_vidport],dx
        jmp .L10  // jump around subroutine definitions

putstr:
        push ebp; mov ebp,esp; push edi

        mov edi,[ebp+ 2*4]  // ptr
.L40:
        movb al,[edi]; inc edi
        cmpb al,0; jz .L49
        cmpb al,'\n'; je .L47
.L45:
        movb dl,al
        call curpos
        add eax,[esi+ si_vidmem]
        movb [eax],dl
        incb    [esi+ si_orig_x]
        movb al,[esi+ si_orig_x]
        cmpb al,[esi+ si_orig_video_cols]; jb .L40
.L47:
        call newline
        jmp .L40
.L49:
        pop edi; pop ebp
        jmp setcursor

newline:
        movb [esi+ si_orig_x],0
        incb [esi+ si_orig_y]
        movb al,[esi+ si_orig_y]
        cmpb al,[esi+ si_orig_video_lines]; jb no_scroll
        decb [esi+ si_orig_y]
scroll:
        push esi; push edi
        mov edi,[esi+ si_vidmem]
        movb al,[esi+ si_orig_video_lines]
        movzbl ecx,[esi+ si_orig_video_cols]
        lea esi,[edi + ecx*2]
        push ecx
          dec eax; mulb cl; xchg eax,ecx  // ecx= (lines -1) * cols
          rep; movsw  // move all lines up
        pop ecx  // cols
        movw ax,[edi]; mov al,' '  // ' ' with ah= current_video_attributes
        rep; stosw  // fill last line with spaces
        pop edi; pop esi
no_scroll:
        //jmp setcursor

setcursor:
        call curpos; shr eax; xchg eax,ecx  // ecx= position
        mov edx,[esi+ si_vidport]
        mov al,14; call outb_p; inc edx
        mov al,ch; call outb_p; dec edx
        mov al,15; call outb_p; inc edx
        mov al,cl
        // jmp outb_p

outb_p:
        outb dx,al
        call 0f; 0:  // insure minimum delay between 'outb'
        call 0f; 0:
        call 0f; 0:
        call 0f; 0:
        ret

curpos:
        movzbl eax,[esi+ si_orig_y]
        movzbl ecx,[esi+ si_orig_x]
        mulb [esi+ si_orig_video_cols]  // ax= y * cols
        add eax,ecx  // + x
        add eax,eax  // * 2
        ret

put_hex:  // %eax in " %.8x";  clobbers edi,edx,ecx,eax
        sub esp,12
        mov edi,esp
        mov edx,eax

        mov al,' '
        stosb
        mov ecx,8
.L30:
        rol edx,4
        mov al,dl
        and al,0xf
        cmp al,10
        sbb al,0x69
        das
        stosb
        loop .L30
        mov al,0
        stosb

        push esp; call putstr
        add esp,4+ 3*4
        ret

print_stk8:
        push ebp; mov ebp,esp; push ebx; push edi

        push -8  // -(number of words to print)
        pop ecx
.L20:
        mov eax,[ebp + 4*ecx + 8*4 + 8]
        push ecx; call put_hex; pop ecx
        inc ecx
        jnz .L20
        pop edi; pop ebx; pop ebp
        jmp newline

.L10:
// example of specific debugging
        mov eax,[esi+ 0x228]  // cmd_line_ptr
        mov ecx,[esi+ 0x238]  // cmdline_size
        mov edx,esi

        pusha
        call print_stk8
        popa
#endif  /*}*/

section LINUZ000
    ////.byte 0xf1  // qemu In-Circuit-Emulator breakpoint
                //cli  // this must be true now (else we already lost the race)
                cld
#if 0  /*{ conflict: want to fetch memory, but might not have valid %ds */
                cmpw [BP_version + esi],0x207
                jb L01  // no KEEP_SEGMENTS in BP_loadflags
                testb [BP_loadflags + esi],KEEP_SEGMENTS
                jnz L03  // bootloader asked: no change in segments
L01:
#endif  /*}*/
/*
        ; The only facts about segments here, that are true for all kernels:
        ; %cs is a valid "flat" code segment; no other segment reg is valid;
        ; the next segment after %cs is a valid "flat" data segment, but
        ; no segment register designates it yet.
*/
                mov eax,cs
                add eax, 1<<3  // the next segment after cs:  __BOOT_DS
                mov ds,eax
                mov es,eax
                mov ss,eax
                lea esp,[stack_offset + esi]  // ancient default
L03:
// If the kernel command line ends below (0x99800 - cmd_exp), then move it up.
cmd_exp = 80  // allow kernel to expand command line "in place" by this much

                mov ebp,esi  // save register
                cmpw [esi+ BP_version],0x202; jae cl_bp202
cl_nbp202:  // use < 2.02 method.  in: ebp= real_mode_ptr
                mov esi,ebp  // absolute worst case default
                cmpw [ebp+ BP_cmd_line_magic],0xA33F; jne cl_move  // nothing at all
                movzwl esi,[ebp+ BP_cmd_line_offset]
                add esi,ebp
                jmp cl_move
cl_bp202:  // try >= 2.02 method
                mov esi,[esi+ BP_cmd_line_ptr]  // supposed pointer
                test esi,esi; jz cl_nbp202  // not really!
cl_move:  // in: esi= cmd_line_ptr; ebp= real_mode_ptr
                sub ecx,ecx  // length
0:
                lodsb; inc ecx; cmpb al,0; jne 0b  // find end and 1+strlen

                mov edx,edi  // save register
                mov edi,0x99800 - cmd_exp; lea eax,[edi + cmd_exp -1]
                cmp esi,edi; jae 8f
                dec edi; dec esi; std; rep movsb; cld  // move command line bytes
                inc edi;     mov  [ebp+ BP_cmd_line_ptr],edi
                sub eax,edi; mov  [ebp+ BP_cmdline_size],eax  // maximum strlen
                mov eax,edi
                sub eax,ebp; movw [ebp+ BP_cmd_line_offset],ax  // backward compat

                and edi,~3  // word align down
                mov [edi-8],edi
                mov [edi-4],ds
                lss esp,[edi-8]  // stack ends at command line
8:
                mov edi,edx  // restore register
                mov esi,ebp  // restore register
nbp202:
                push    0
                popf            // BIOS can leave random flags (such as NT)

// If the real mode segment is above the floor, then move it down.
// The default floor is 0x90000, but 0x10000 (64K) for confirmed bzImage.
// But due to possible complications with BP_cmd_line_offset, keep 0x90000.
                push edi; mov edi,0x90000  // save register
                cmpw [esi+ BP_version],0x200; jb 8f  // boot protocol too old
                testb [esi+ BP_loadflags],LOADED_HIGH; jz 8f  // not bzImage
                mov edi,0x90000
8:
                push edi; mov ecx,0x250>>2; rep movsd
                pop esi  // new real mode pointer
                pop edi  // restore register

section LINUZ001  // Bvmlinuz and not relocatable
                mov     eax, offset original_entry  // 0x100000 : address of startup_32
                push    cs      // MATCH00
                push    eax     // MATCH00  entry address
                push    edi     // MATCH01  save
                push    esi     // MATCH02  save

section LINUZ010
                or      ebp, -1 // decompressor assumption

section LINUZ101  // Bvmlinuz and relocatable
                .byte 0xe8
                .long compressed_length  // MATCH10  call L10

section LINUZ110
L10:
                pop ebp  // MATCH10  &compressed
                call L20  // MATCH11
moveloop:
                neg ecx
                cmp edi,esi
                jbe forw  // dst<=src ==> move ascending
                lea esi,[-4+ esi + ecx]  // other end
                lea edi,[-4+ edi + ecx]
                std  // move descending
forw:
                shr ecx,2
                rep movsd
                cld
                ret
L20:
                mov edx,esi  // save pointer to real-mode data (MATCH19)
                pop esi  // MATCH11  src  &moveloop
                mov ecx,[-4+ esi]  // byte length
                neg ecx
                and ecx,-4
                add esp,ecx  // allocate
                mov edi,esp  // dst
                call esi  // move the moveloop

section LINUZ120  // runtime relocatable
                mov edi,ebp  // bootloader put us here
                neg edi
                and edi,offset neg_config_physical_align
                neg edi  // entry address  where we belong
section LINUZ130  // fixed address (need not be 0x100000)
                mov edi,offset load_physical_address  // link address

section LINUZ140
                mov eax,esp  // &moveloop (reloc)
                push edx  // MATCH19 pointer to real-mode data (original esi)
                lea edx,[unc_length + edi]
                mov esi,ebp  // src for move
                push edx  // MATCH20  end of relocation info
                add edx,offset dec_offset  // &LZCUTPOI
                push edi  // MATCH21  runtime entry
                add edi,offset unc_offset  // dst for move ( + u_len - c_len )
                push edi  // MATCH22  &compressed (reloc)
                mov ecx,offset neg_length_mov
                push edx  // MATCH23  &decompressor (reloc)
                jmp eax  // moveloop (reloc); MATCH23 return: &LINUZ141 (reloc)
// assumed location of LZCUTPOI >>here<<  [0== length(LZCUTPOI)]

section LINUZ141  // one-time prolog to decompressor
                pop esi  // MATCH22 &compressed (reloc)
                pop edi  // MATCH21 &uncompressed
                push edi  // MATCH24  runtime entry
section LINUZ145  // only if filter
                push edi  // MATCH03  src unfilter
                //push   offset filter_cto      // MATCH04  cto unfilter
                .byte   0x6a, filter_cto        // MATCH04  cto unfilter
                push    offset filter_length    // MATCH05  len unfilter

        // decompressor and unfilter >>here<<

section LINUZ150  // fall in after unfilter
                pop ebp  // MATCH24 runtime entry
                pop esi  // MATCH20 end of relocation info
                mov ebx,ebp
                sub ebx,offset load_physical_address
                jz L40
                std
                lodsd  // point at highest relocation, not beyond it
                jmp L35
L30:
                add [neg_page_offset + ebx + eax],ebx
L35:
                lodsd
                test eax,eax
                jnz L30
                cld
L40:
                pop esi  // MATCH19 pointer to real-mode data (original esi)
                xor ebx,ebx  // processor 0
                jmp ebp  // entry to decompressed

section         LZCALLT1  // 0x40!=(0xf0 & ft->id)
                push    eax     // MATCH03  src unfilter
section         LZCKLLT1  // 0x40==(0xf0 & ft->id)
                push    eax                     // MATCH03  src unfilter
                //push   offset filter_cto      // MATCH04  cto unfilter
                .byte   0x6a, filter_cto        // MATCH04  cto unfilter
                push    offset filter_length    // MATCH05  len unfilter
section         LBZIMAGE
                mov     esi, offset copy_source
                mov     edi, offset copy_dest
                mov     ecx, offset words_to_copy

                std
                rep
                movsd
                cld

                mov     esi, offset src_for_decompressor
                xchg    eax, edi        // edi = dst for decompressor = 0x100000
                jmp     decompressor    // jump to the copied decompressor

section         LZIMAGE0

// this checka20 stuff looks very unnecessary to me
checka20:
                inc     edi             // change value
                mov     [1 + ebp], edi  // store to 0x000000 (even megabyte)
                cmp     [eax], edi      // compare  0x100000 ( odd megabyte)
                je      checka20        // addresses are [still] aliased

                cld
                mov     esi, offset src_for_decompressor
                xchg    eax, edi        // edi = dst for decompressor = 0x100000

section         LZCUTPOI

// =============
// ============= DECOMPRESSION
// =============

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"

// =============
// ============= UNFILTER
// =============

section         LZCKLLT9
                pop     ecx     // MATCH05  len
                pop     edx     // MATCH04  cto
                pop     edi     // MATCH03  src

                ctok32  edi, dl   // dl has cto8

section         LZCALLT9
                pop     edi     // MATCH03  src
                cjt32   0

section         LINUZ990
                pop     esi     // MATCH02  restore
                pop     edi     // MATCH01  restore
                xor     ebx, ebx        // booting the 1st cpu
                lret    // MATCH00  set cs

#include        "include/header.S"

// vi:ts=8:et:nowrap

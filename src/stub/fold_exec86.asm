;  fold_exec86.asm -- linkage to C code to process Elf binary
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2000-2001 John F. Reiser
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
;  Markus F.X.J. Oberhumer   Laszlo Molnar           John F. Reiser
;  markus@oberhumer.com      ml1050@cdata.tvnet.hu   jreiser@BitWagon.com
;

;; control just falls through, after this part and compiled C code
;; are uncompressed.

fold_begin:  ;; this label is known to the Makefile
        pop eax  ; discard &dstlen
        pop eax  ; discard  dstlen

                pop     eax                 ; Pop the argument count
                mov     ecx, esp            ; argv starts just at the current stack top
                lea     edx, [ecx+eax*4+4]  ; envp = &argv[argc + 1]
                push    eax                 ; Restore the stack
                push    ebp  ; argument: &decompress
                push    ebx  ; argument: &my_elfhdr
                push    edx  ; argument: envp
                push    ecx  ; argument: argv
EXTERN upx_main
                call    upx_main            ; Call the UPX main function
                hlt                         ; Crash if somehow upx_main does return

; vi:ts=8:et:nowrap

; Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details
; Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
; Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
; Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details
; Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
; Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
; -*- asm -*-
;
; KLUDGE-WARNING!
;
; So you say you want to change this file, right?  Are you really sure
; that's a good idea?  Let me tell you a bit about the pitfalls here:
;
; * Some code runs in protected mode, some in real-mode, some in both.
; * Some code must run on a 8088 without crashing it.
; * Registers and flags may be expected to survive for a long time.
; * The code is optimized for size, not for speed or readability.
; * Some comments are parsed by other programs.
;
; You still want to change it?  Oh well, go ahead, but don't come
; crying back saying you weren't warned.
;
;-----------------------------------------------------------------------------
;  djgpp extender-less stub loader
;
;  (C) Copyright 1993-1995 DJ Delorie
;
;  Redistribution and use in source and binary forms are permitted
;  provided that: (1) source distributions retain this entire copyright
;  notice and comment, (2) distributions including binaries display
;  the following acknowledgement:  ``This product includes software
;  developed by DJ Delorie and contributors to the djgpp project''
;  in the documentation or other materials provided with the distribution
;  and in all advertising materials mentioning features or use of this
;  software, and (3) binary distributions include information sufficient
;  for the binary user to obtain the sources for the binary and utilities
;  required to built and use it. Neither the name of DJ Delorie nor the
;  names of djgpp's contributors may be used to endorse or promote
;  products derived from this software without specific prior written
;  permission.
;
;  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
;  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
;  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
;
;  Revision history:
;
;  93/12/05 DJ Delorie  Initial version v2.00, requires DPMI 0.9
;  94/10/13 CW Sandmann v2.01, accumlated changes: 60K load bug, limits, cwsdpmi, optimization
;  94/10/29 CW Sandmann v2.03, M Welinder changes; cwsdpmi load anywhere, size decrease
;
   .copyright "The STUB.EXE stub loader is Copyright (C) 1993-1995 DJ Delorie. "
   .copyright "Permission granted to use for any purpose provided this copyright "
   .copyright "remains present and unmodified. "
   .copyright "This only applies to the stub, and not necessarily the whole program.\n"
   .id
;
;-----------------------------------------------------------------------------
;  Interface to 32-bit executable:
;
;    cs:eip     according to COFF header
;    ds         32-bit data segment for COFF program
;    fs         selector for our data segment (fs:0 is stubinfo)
;    ss:sp      our stack (ss to be freed)
;    <others>   All unspecified registers have unspecified values in them.
;-----------------------------------------------------------------------------
;  This is the stubinfo structure.  The presence of this structure
;  indicates that the executable is a djgpp v2.00 executable.
;  Fields will never be deleted from this structure, only obsoleted.
;
        .org    0                       ; just in case
stubinfo:
stubinfo_magic:                         ; char [16]
        .db     "go32stub, v 2.04"      ; version may change, [0..7] won't
stubinfo_size:                          ; unsigned long
        .dd     stubinfo_end            ; bytes in structure
stubinfo_minstack:                      ; unsigned long
        .dd     0x80000                 ; minimum amount of DPMI stack space (512K)
stubinfo_memory_handle:                 ; unsigned long
        .dd     0                       ; DPMI memory handle
stubinfo_initial_size:                  ; unsigned long
        .dd     0                       ; size of initial segment
stubinfo_minkeep:                       ; unsigned short
        .dw     16384                   ; amount of automatic real-mode buffer
stubinfo_ds_selector:                   ; unsigned short
        .dw     0                       ; our DS selector (used for transfer buffer)
stubinfo_ds_segment:                    ; unsigned short
        .dw     0                       ; our DS segment (used for simulated calls)
stubinfo_psp_selector:                  ; unsigned short
        .dw     0                       ; PSP selector
stubinfo_cs_selector:                   ; unsigned short
        .dw     0                       ; to be freed
stubinfo_env_size:                      ; unsigned short
        .dw     0                       ; number of bytes of environment
stubinfo_basename:                      ; char [8]
        .db     8 .dup 0                ; base name of executable to load (asciiz if < 8)
stubinfo_argv0:                         ; char [16]
        .db     16 .dup 0               ; used ONLY by the application (asciiz if < 16)
stubinfo_dpmi_server:                   ; char [16]
        .db     "CWSDPMI.EXE\0\0\0\0\0" ; used by stub to load DPMI server if no DPMI already present

        .align  4
stubinfo_end:

;-----------------------------------------------------------------------------
;  First, set up our memory and stack environment

        .start                          ; execution begins here
        push    cs
        pop     ds
        mov     [stubinfo_ds_segment], ds

        mov     [psp_segment], es       ; save the PSP segment
        cld

;-----------------------------------------------------------------------------
;  Check that we have DOS 3.00 or later.  (We need this because earlier
;  versions don't supply argv[0] to us and will scrog registers on dpmi exec).
        mov     ah, 0x30
        int     0x21
        cmp     al, 3
        jae     dos3ok
        mov     al, 109
        mov     dx, msg_bad_dos
        jmpl    error
dos3ok:
        mov     [dos_major], al
        mov     si, stubinfo_minkeep

;-----------------------------------------------------------------------------
;  Resize memory in case we need to exec a DPMI server

resize_again:
        mov     ax, [si]                ; si=&stubinfo_minkeep
        or      ax, ax
        jnz     @f1
;       mov     ax,0xfe00               ; 0 was probably 64k, so max it! (mod 512)
        mov     ah,0xfe                 ; al already 0
@f1:
        mov     bx, end_of_memory       ; does not include PSP
        cmp     bx, ax                  ; is our program big enough to hold it?
        jae     @f1
        mov     bx, ax
@f1:
        mov     [si], bx                ; si=&stubinfo_minkeep store for reference
        inc     bh                      ; add 256 bytes for PSP
        mov     cx, 0xff04              ; 0xff is for below
        shr     bx, cl                  ; bytes to paragraphs

        mov     ah, 0x4a                ; ES = PSP segment from above
        int     0x21                    ; resize our memory block
        jnc     @f1                     ; did it work?
        shl     bx,cl                   ; calculate smaller [keep] value
        dec     bh
        mov     [si], bx                ; si=&stubinfo_minkeep
        jmp     resize_again            ; and try again
@f1:

;-----------------------------------------------------------------------------
;  Scan environment for "PATH=" and the stub's full name after environment

        mov     es, es:[0x2c]           ; get environment segment
        xor     di, di                  ; begin search for NUL/NUL (di = 0)
;       mov     cx, 0xff04              ; effectively `infinite' loop
        xor     al, al
        .db     0xa9                    ; "test ax,...." -- skip 2 bytes
scan_environment:
        repne
        scasb                           ; search for NUL
        cmpw    es:[di], 0x4150         ; "PA"
        jne     not_path
        scasw
        cmpw    es:[di], 0x4854         ; "TH"
        jne     not_path
        scasw
        cmpb    es:[di], '='
        jne     not_path
        inc     di                      ; Point to PATH contents
        mov     [path_off], di          ; save for later
        dec     di                      ; in case the PATH is empty
not_path:
        scasb
        jne     scan_environment        ; no, still environment
        scasw                           ; adjust pointer to point to prog name

;; When we are spawned from a program which has more than 20 handles in use,
;; all the handles passed to us by DOS are taken (since only the first 20
;; handles are inherited), and opening the .exe file will fail.
;; Therefore, we forcefully close handles 18 and 19, to make sure at least two
;; handles are available.

        mov     ah, 0x3e
        mov     bx, 19
        int     0x21                    ; don't care about errors
        mov     ah, 0x3e
        mov     bx, 18
        int     0x21                    ; don't care about errors

;-----------------------------------------------------------------------------
;  Get DPMI information before doing anything 386-specific

        push    es
        push    di
        xor     cx, cx                  ; flag for load attempt set cx = 0
        jz      @f2                     ; We always jump, shorter than jmp
@b1:
        mov     al, 110
        mov     dx, msg_no_dpmi
        jmpl    error
@b2:
        or      cx, cx
        jnz     @b1                     ; we already tried load once before
        inc     cx
        call    load_dpmi
        jc      @b1
@f2:
        mov     ax, 0x1687              ; get DPMI entry point
        int     0x2f
        or      ax, ax
        jnz     @b2                     ; if 0 then it's there
        and     bl, 1                   ; 32 bit capable?
        jz      @b2
@f3:
        mov     [modesw], di            ; store the DPMI entry point
        mov     [modesw+2], es
        mov     [modesw_mem], si
        pop     di
        pop     es

;-----------------------------------------------------------------------------
;  Now, find the name of the program file we are supposed to load.

;       xor     ah, ah                  ; termination character (set above!)
        call    store_env_string        ; copy it to loadname, set bx

        mov     [stubinfo_env_size], di
        mov     [loadname_nul], si      ; remember nul so we can change it to $
        cmpb    stubinfo_basename[0], 0
        je      no_symlink

;-----------------------------------------------------------------------------
;  Replace the stub's file name with the link's name after the directory

        mov     cx, 8                   ; max length of basename
        mov     di, stubinfo_basename   ; pointer to new basename
@b1:
        mov     al, [di]                ; get next character
        inc     di
        or      al, al                  ; end of basename?
        je      @f1
        mov     [bx], al                ; store character
        inc     bx
        loop    @b1                     ; eight characters?
@f1:
        movd    [bx], 0x4558452e        ; append ".EXE"
        add     bx, 4
        movb    [bx], 0                 ; null terminate
        mov     [loadname_nul], bx      ; remember nul so we can change it to $

no_symlink:

;-----------------------------------------------------------------------------
;  Load the COFF information from the file

        mov     ax, 0x3d00              ; open file for reading
        mov     dx, loadname
        int     0x21
        jcl     error_no_progfile       ; do rest of error message

@f1:
        mov     [program_file], ax      ; store for future reference

        mov     bx, ax
        mov     cx, exe_header_length
        mov     dx, exe_header
        mov     ah, 0x3f                ; read EXE header
        int     0x21

        xor     dx, dx                  ; dx = 0
        xor     cx, cx                  ; offset of COFF header

        mov     ax, [exe_magic]
        cmp     ax, 0x014c              ; COFF?
        je      file_is_just_coff
        cmp     ax, 0x5a4d              ; EXE magic value
        jnel    error_not_exe

        mov     dx, [exe_sectors]
        shl     dx, 9                   ; 512 bytes per sector
        mov     bx, [exe_bytes_last_page]
        or      bx, bx                  ; is bx = 0 ?
        je      @f1
        sub     dh, 2                   ; dx -= 512
        add     dx, bx
@f1:

file_is_just_coff:                      ; cx:dx is offset
        mov     coff_offset[0], dx
        mov     coff_offset[2], cx
        mov     ax, 0x4200              ; seek from beginning
        mov     bx, [program_file]
        int     0x21

        mov     cx, coff_header_length
        mov     dx, coff_header
        mov     ah, 0x3f                ; read file (bx = handle)
        int     0x21

        cmp     ax, coff_header_length
        jne     @f2
        cmpw    coff_header[coff_magic], 0x014c
@f2:
        jnel    error_not_coff

        mov     eax, aout_header[aout_entry]
        mov     [start_eip], eax

        mov     ecx, [coff_offset]

        mov     eax, text_section[s_scnptr]
        add     eax, ecx
        mov     [text_foffset], eax

        add     ecx, data_section[s_scnptr] ; Ok to destroy ecx now: last use.
        mov     [data_foffset], ecx

        mov     ebx, bss_section[s_vaddr]
        add     ebx, bss_section[s_size]
        mov     eax, 0x00010001
        cmp     ebx, eax
        jae     @f1
        mov     ebx, eax                ; ensure 32-bit segment
@f1:
        add     ebx, 0x0000ffff         ; ensure 64K rounded
        xor     bx, bx                  ; clear rounded bits
        mov     [stubinfo_initial_size], ebx

;-----------------------------------------------------------------------------
;  Set up for the DPMI environment

        call    include_umb
        mov     bx, [modesw_mem]
        or      bx, bx
        jz      no_dos_alloc

        mov     ah, 0x48                ; allocate memory for the DPMI host
        int     0x21
        jcl     error_no_dos_memory_umb
        mov     es, ax

no_dos_alloc:
        call    restore_umb
        mov     ax, 1                   ; indicates a 32-bit client
        callf   [modesw]                ; enter protected mode

        jcl     error_in_modesw

;-----------------------------------------------------------------------------
; We're in protected mode at this point.

        mov     [stubinfo_psp_selector], es
        mov     [stubinfo_cs_selector], cs
        mov     ax, ds
        mov     [stubinfo_ds_selector], ax
        mov     es, ax

        xor     ax, ax                  ; AX = 0x0000
        mov     cx, 1
        int     0x31                    ; allocate LDT descriptor
        jc      @f2
        mov     [client_cs], ax

        xor     ax, ax                  ; AX = 0x0000
;       mov     cx, 1                   ; already set above
        int     0x31                    ; allocate LDT descriptor
@f2:
        jcl     perror_no_selectors
        mov     [client_ds], ax

; Try getting a DPMI 1.0 memory block first, then try DPMI 0.9
; Note:  This causes the Borland Windows VxD to puke, commented for now with ;*
;*      mov     ax, 0x0504
;*      xor     ebx, ebx                ; don't specify linear address
        mov     ecx, stubinfo_initial_size[0]
;*      mov     edx, 1                  ; allocate committed pages
;*      int     0x31                    ; allocate memory block
;*      jc      try_old_dpmi_alloc
;*      mov     client_memory[0], ebx
;*      mov     stubinfo_memory_handle[0], esi
;*      jmp     got_dpmi_memory
try_old_dpmi_alloc:
        mov     ax, 0x0501
        mov     bx, stubinfo_initial_size[2]
;       mov     cx, stubinfo_initial_size[0]    ;Set above
        int     0x31                    ; allocate memory block
        jcl     perror_no_dpmi_memory
        mov     client_memory[2], bx
        mov     client_memory[0], cx
        mov     stubinfo_memory_handle[2], si
        mov     stubinfo_memory_handle[0], di
got_dpmi_memory:

        mov     ax, 0x0007
        mov     bx, [client_cs]         ; initialize client CS
        mov     cx, client_memory[2]
        mov     dx, client_memory[0]
        int     0x31                    ; set segment base address

        mov     ax, 0x0009
;       mov     bx, [client_cs]         ; already set above
        mov     cx, cs                  ; get CPL
        and     cx, 0x0003
        shl     cx, 5
        push    cx                      ; save shifted CPL for below
        or      cx, 0xc09b              ; 32-bit, big, code, non-conforming, readable
        int     0x31                    ; set descriptor access rights

        mov     ax, 0x0008
;       mov     bx, [client_cs]         ; already set above
        mov     cx, stubinfo_initial_size[2]
        dec     cx
        mov     dx, 0xffff
        int     0x31                    ; set segment limit

        mov     ax, 0x0007
        mov     bx, [client_ds]         ; initialize client DS
        mov     cx, client_memory[2]
        mov     dx, client_memory[0]
        int     0x31                    ; set segment base address

        mov     ax, 0x0009
;       mov     bx, [client_ds]         ; already set above
        pop     cx                      ; shifted CPL from above
        or      cx, 0xc093              ; 32-bit, big, data, r/w, expand-up
        int     0x31                    ; set descriptor access rights

        mov     ax, 0x0008
;       mov     bx, [client_ds]         ; already set above
        mov     cx, stubinfo_initial_size[2]
        dec     cx
        mov     dx, 0xffff
        int     0x31                    ; set segment limit

;-----------------------------------------------------------------------------
;  Load the program data

        mov     ax, 0x0100
        mov     bx, 0x0f00              ; 60K DOS block size
        int     0x31                    ; allocate DOS memory
        jnc     @f1
        cmp     ax, 0x0008
        jnel    perror_no_dos_memory
        mov     ax, 0x0100              ; try again with new value in bx
        int     0x31                    ; allocate DOS memory
        jcl     perror_no_dos_memory
@f1:
        mov     [dos_block_seg], ax
        mov     [dos_block_sel], dx
        shl     bx, 4                   ; paragraphs to bytes
        mov     [dos_block_size], bx

        mov     esi, [text_foffset]     ; load text section
        mov     edi, text_section[s_vaddr]
        mov     ecx, text_section[s_size]
        call    read_section

        mov     esi, [data_foffset]     ; load data section
        mov     edi, data_section[s_vaddr]
        mov     ecx, data_section[s_size]
        call    read_section

        mov     es, [client_ds]         ; clear the BSS section
        mov     edi, bss_section[s_vaddr]
        mov     ecx, bss_section[s_size]
        xor     eax,eax
        shr     ecx,2
        .addrsize
        rep
        stosd

        mov     ah,0x3e
        mov     bx, [program_file]
        int     0x21                    ; close the file

        mov     ax, 0x0101
        mov     dx, [dos_block_sel]
        int     0x31                    ; free up the DOS memory

        push    ds
        pop     fs
        mov     ds, [client_ds]
        .opsize
        jmpf    fs:[start_eip]          ; start program

;-----------------------------------------------------------------------------
;  Read a section from the program file

read_section:
        mov     eax, esi                ; sector alignment by default
        and     eax, 0x1ff
        add     ecx, eax
        sub     si, ax                  ; sector align disk offset (can't carry)
        sub     edi, eax                ; memory maybe not aligned!

        mov     [read_size], ecx        ; store for later reference
        mov     [read_soffset], edi

        call    zero_regs
        mov     dpmi_regs[dr_dx], si    ; store file offset
        shr     esi, 16
        mov     dpmi_regs[dr_cx], si
        mov     bx, [program_file]
        mov     dpmi_regs[dr_bx], bx
        movw    dpmi_regs[dr_ax], 0x4200
        call    pm_dos                  ; seek to start of data

; Note, handle set above
        mov     ax, [dos_block_seg]
        mov     dpmi_regs[dr_ds], ax
        movw    dpmi_regs[dr_dx], 0     ; store file offset
read_loop:
        movb    dpmi_regs[dr_ah], 0x3f
        mov     ax, read_size[2]        ; see how many bytes to read
        or      ax, ax
        jnz     read_too_big
        mov     ax, read_size[0]
        cmp     ax, [dos_block_size]
        jna     read_size_in_ax         ; jna shorter than jmp
read_too_big:
        mov     ax, [dos_block_size]
read_size_in_ax:
        mov     dpmi_regs[dr_cx], ax
        call    pm_dos                  ; read the next chunk of file data

        xor     ecx, ecx
        mov     cx, dpmi_regs[dr_ax]    ; get byte count
        mov     edi, [read_soffset]     ; adjust pointers
        add     [read_soffset], ecx
        sub     [read_size], ecx

        xor     esi, esi                ; esi=0 offset for copy data
        shr     cx, 2                   ; ecx < 64K
        push    ds
        push    es
        mov     es, [client_ds]
        mov     ds, [dos_block_sel]
        .addrsize
        rep
        movsd
        pop     es
        pop     ds

        add     ecx, [read_size]        ; ecx zero from the rep movsd
        jnz     read_loop

        ret

;-----------------------------------------------------------------------------
;  Routine to check al for delimiter

test_delim:
        cmp     al, ':'                 ; watch for file name part
        je      @f3
        cmp     al, '/'
        je      @f3
        cmp     al, '\\'
@f3:
        ret

;-----------------------------------------------------------------------------
;  Copy string from environment to loadname.
;   On entry: di = environment offset
;             ah = termination character (null also does)
;   On exit:  bx = pointer to one character after last observed file delimiter
;             di = pointer to one character after last copied
;             si = pointer to the copied termination character
;             al = terminating character

store_env_string:
        mov     si, loadname            ; pointer to buffer
        mov     bx, si                  ; in case no delimiters
@b1:
        mov     al, es:[di]             ; copy a character to buffer
        inc     di
        mov     [si], al
        cmp     al, ah                  ; end of file name?
        je      @f1
        or      al, al                  ; end of file name?
        je      @f1
        inc     si
        call    test_delim
        jne     @b1
        mov     bx, si                  ; remember pointer to first char of
        je      @b1                     ; next name component (shorter than jmp)
@f1:
        ret

;-----------------------------------------------------------------------------
;  Most errors come here, early ones jump direct (8088 instructions)

error_no_progfile:
        mov     al, 102
        mov     dx, msg_no_progfile
        jmp     error_fn

error_not_exe:
        mov     al, 103
        mov     dx, msg_not_exe
        jmp     error_fn

error_not_coff:
        mov     al, 104
        mov     dx, msg_not_coff
;       jmp     error_fn

error_fn:
        push    dx
        mov     bx, [loadname_nul]      ; error, print file name
        movb    [bx], '$'
        mov     bx, loadname
        jmp     @f1

error_no_dos_memory_umb:
        call    restore_umb
error_no_dos_memory:
        mov     al, 105
        mov     dx, msg_no_dos_memory
        jmp     error

error_in_modesw:
        mov     al, 106
        mov     dx, msg_error_in_modesw
        jmp     error

perror_no_selectors:
        mov     al, 107
        mov     dx, msg_no_selectors
        jmp     error

perror_no_dpmi_memory:
        mov     al, 108
        mov     dx, msg_no_dpmi_memory
        jmp     error

perror_no_dos_memory:
        mov     al, 105
        mov     dx, msg_no_dos_memory
;       jmp     error

error:
        push    dx
        mov     bx, err_string
@f1:
        call    printstr
        pop     bx
        call    printstr
exit:
        mov     bx, crlfdollar
        call    printstr
        mov     ah, 0x4c                ; error exit - exit code is in al
        int     0x21

printstr1:
        inc     bx
        push    ax                      ; have to preserve al set by error call
        mov     ah, 2
        int     0x21
        pop     ax                      ; restore ax (John A.)
printstr:
        mov     dl, [bx]
        cmp     dl, '$'
        jne     printstr1
        ret

crlfdollar:
        .db     13,10,'$'
;-----------------------------------------------------------------------------
;  DPMI utility functions

zero_regs:
        push    ax
        push    cx
        push    di
        xor     ax, ax
        mov     di, dpmi_regs
        mov     cx, 0x19
        rep
        stosw
        pop     di
        pop     cx
        pop     ax
        ret

pm_dos:
        mov     ax, 0x0300              ; simulate interrupt
        mov     bx, 0x0021              ; int 21, no flags
        xor     cx, cx                  ; cx = 0x0000 (copy no args)
        mov     edi, dpmi_regs
        int     0x31
        ret

;-----------------------------------------------------------------------------
;  load DPMI server if not present
;   First check directory from which stub is loaded, then path, then default
;   On entry di points to image name

path_off:
        .dw     0                       ; If stays zero, no path

load_dpmi:
        xor     ah, ah                  ; Copy until this character (=0)
        call    store_env_string        ; copy stub image to "loadname"
        mov     si, bx                  ; remove name so we can add DPMI name
        mov     di, [path_off]          ; Pointer to path contents (next try)
        jmp     @f2
loadloop:
        mov     ah, ';'                 ; Copy until this character
        call    store_env_string        ; to "loadname"
        or      al,al                   ; Check terminating character
        jne     @f1                     ; If ';', continue
        dec     di                      ; else point at null for next pass.
@f1:
        cmp     si, loadname            ; anything there?
        je      do_exec                 ; final try (no path) let it return
        mov     al, [si-1]
        call    test_delim              ; is final character a path delimiter
        je      @f2
        movb    [si], '\\'              ; no, add separator between path & name
        inc     si
@f2:
        call    do_exec                 ; copy our name to string and try load
        jc      loadloop
        ret

;-----------------------------------------------------------------------------
; add the string CWSDPMI to path ending

do_exec:
        call    include_umb
        mov     bx, stubinfo_dpmi_server
@b1:
        mov     al, [bx]
        mov     [si], al
        inc     bx
        inc     si
        or      al, al
        jne     @b1
;       movw    [si], 0x0a0d            ;debug
;       movb    [si+2], '$'             ;debug

        push    es                      ; Save in case of failure
        push    di

;memory saving - use dpmi_regs as a temporary parameter block
        push    ds
        pop     es                      ;zero_regs needs es set
        call    zero_regs
        mov     bx, dpmi_regs
        mov     [bx+4], ds              ;segment of command tail
        mov     [bx+2], bx              ;offset (point to zero)

        mov     dx, loadname
;       mov     ah, 9                   ;debug
;       int     0x21                    ;debug
        mov     ax, 0x4b00              ;Do program exec
        int     0x21
        pop     di
        pop     es
        jc      @f1                     ;carry set if exec failed

        mov     ah, 0x4d                ;get return code
        int     0x21
        sub     ax, 0x300               ;ah=3 TSR, al=code (success)
        neg     ax                      ;CY, if not originally 0x300
@f1:
        jmp     restore_umb             ;called func. return for us.

;-----------------------------------------------------------------------------
; Make upper memory allocatable.  Clobbers Ax and Bx.

include_umb:
        cmpb    [dos_major], 5          ; Won't work before dos 5
        jb      @f1
        mov     ax, 0x5800              ; get allocation strategy
        int     0x21
        mov     [old_strategy],al
        mov     ax, 0x5802              ; Get UMB status.
        int     0x21
        mov     [old_umb],al
        mov     ax, 0x5801
        mov     bx, 0x0080              ; first fit, first high then low
        int     0x21
        mov     ax, 0x5803
        mov     bx, 0x0001              ; include UMB in memory chain
        int     0x21
@f1:
        ret

; Restore upper memory status.  All registers and flags preserved.

restore_umb:
        pushf
        cmpb    [dos_major], 5          ; Won't work before dos 5
        jb      @f1
        push    ax
        push    bx
        mov     ax, 0x5803              ; restore UMB status.
        mov     bl,[old_umb]
        xor     bh, bh
        int     0x21
        mov     ax, 0x5801              ; restore allocation strategy
        mov     bl,[old_strategy]
        xor     bh, bh
        int     0x21
        pop     bx
        pop     ax
@f1:
        popf
        ret

;-----------------------------------------------------------------------------
;  Stored Data
err_string:
        .db     "Load error: $"
msg_no_progfile:
        .db     ": can't open$"
msg_not_exe:
        .db     ": not EXE$"
msg_not_coff:
        .db     ": not COFF (Check for viruses)$"
msg_no_dpmi:
        .db     "no DPMI - Get csdpmi*b.zip$"
msg_no_dos_memory:
        .db     "no DOS memory$"
msg_bad_dos:
        .db     "need DOS 3$"
msg_error_in_modesw:
        .db     "can't switch mode$"
msg_no_selectors:
        .db     "no DPMI selectors$"
msg_no_dpmi_memory:
        .db     "no DPMI memory$"

;-----------------------------------------------------------------------------
;  Unstored Data, available during and after mode switch

last_generated_byte:

        .align  512                     ; Align ourselves to a sector
                                        ;  boundary for startup speed.
        .bss                            ; data after this isn't in file.

modesw:                                 ; address of DPMI mode switch
        .dd     0
modesw_mem:                             ; amount of memory DPMI needs
        .dw     0

program_file:                           ; file ID of program data
        .dw     0

text_foffset:                           ; offset in file
        .dd     0

data_foffset:                           ; offset in file
        .dd     0

start_eip:                              ; EIP value to start at
        .dd     0
client_cs:                              ; must follow start_eip
        .dw     0
client_ds:
        .dw     0

client_memory:
        .dd     0

dos_block_seg:
        .dw     0
dos_block_sel:
        .dw     0
dos_block_size:
        .dw     0

read_soffset:
        .dd     0
read_size:
        .dd     0

dpmi_regs:
        .db     0x32 .dup 0
dr_edi = 0x00
dr_di  = 0x00
dr_esi = 0x04
dr_si  = 0x04
dr_ebp = 0x08
dr_bp  = 0x08
dr_ebx = 0x10
dr_bx  = 0x10
dr_bl  = 0x10
dr_bh  = 0x11
dr_edx = 0x14
dr_dx  = 0x14
dr_dl  = 0x14
dr_dh  = 0x15
dr_ecx = 0x18
dr_cx  = 0x18
dr_cl  = 0x18
dr_ch  = 0x19
dr_eax = 0x1c
dr_ax  = 0x1c
dr_al  = 0x1c
dr_ah  = 0x1d
dr_efl = 0x20
dr_es  = 0x22
dr_ds  = 0x24
dr_fs  = 0x26
dr_gs  = 0x28
dr_ip  = 0x2a
dr_cs  = 0x2c
dr_sp  = 0x2e
dr_ss  = 0x30

;-----------------------------------------------------------------------------

        .align  16                      ; so that stack ends on para boundary
        .dw     128 .dup 0
        .stack

;-----------------------------------------------------------------------------
; At one time real mode only data.  Header stuff now used during image load.

psp_segment:
        .dw     0

loadname_nul:                           ; offset of NUL so it can become '$'
        .dw     0
loadname:                               ; name of program file to load, if it
        .db     81 .dup 0               ; gets really long ok to overwrite next

exe_header:                             ; loaded from front of loadfile
exe_magic:
        .dw     0
exe_bytes_last_page:
        .dw     0
exe_sectors:
        .dw     0
exe_header_length = . - exe_header

coff_offset:
        .dd     0                       ; from start of file

coff_header:                            ; loaded from after stub
        .db     20 .dup 0
aout_header:
        .db     28 .dup 0
text_section:
        .db     40 .dup 0
data_section:
        .db     40 .dup 0
bss_section:
        .db     40 .dup 0
coff_header_length = . - coff_header

old_strategy:
        .db     0
old_umb:
        .db     0

dos_major:
        .db     0

        .align  16                      ; Align ourselves to a paragraph
end_of_memory:                          ; resize is done early so must keep all

;-----------------------------------------------------------------------------
;  structure definitions
;

coff_magic      = 0                     ; from coff header

aout_entry      = 16                    ; from aout header

s_paddr         = 8                     ; from section headers
s_vaddr         = 12
s_size          = 16
s_scnptr        = 20



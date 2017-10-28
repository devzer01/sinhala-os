use16       ; 16-bit mode

org 0x7C00 ; address of the boot sector

BootStageOne:
    ;mov ax, 12h ; AH=0 (Change video mode), AL=13h (Mode)
    ;int 10h ; Video BIOS interrupt
    ;mov ah, 09h
    ;mov dl, 80h
    ;int 13h

    ;mov ah, 08h
    ;mov dl, 80h
    ;mov di, 0x500
    ;int 13h

    mov ah, 02h
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 80h
    mov bx, fontdata
    int 13h
    ;mov bx, data
    ;int 13h
    jmp fontdata
    times 510-($-$$) db 0
end:
    dw 0xAA55

; Program to illustrate one use of write mode 2 of the VGA and EGA by
; animating the image of an “A” drawn by copying it from a chunky
; bit-map in system memory to a planar bit-map in VGA or EGA memory.
;
; Assemble with MASM or TASM
;
; By Michael Abrash
;

section .text
fontdata:

        mov     dx, mtitle
        mov     ah, 9
        int     21h

        push    es
        push    byte 40h
        pop     es
        mov     cx, 54                  ; approx. 3 seconds delay
mtwait:
        mov     ax, [es:6ch]
mttick:
        cmp     ax, [es:6ch]
        je      mttick
        loop    mtwait                  ; wait for a while...
        pop     es

        smsw    ax
        shr     ax, 1
        jnc     cont0                   ; check for PMode
        mov     dx, merr0
        jmp     error

cont0:

        mov     di, vesa_info
        mov     ax, 4f00h
        int     10h                     ; get VESA info
        cmp     ax, 4fh
        je      cont1
        mov     dx, merr1
        jmp     error

cont1:

        mov     di, mode_info
        mov     cx, 101h
        mov     ax, 4f01h
        int     10h                     ; get VESA mode 101h info
        cmp     ax, 4fh
        je      cont2
        mov     dx, merr2
        jmp     error

cont2:

        mov     bx, 101h
        mov     ax, 4f02h
        int     10h                     ; set VESA gfx mode
        cmp     ax, 4fh
        je      cont3
        mov     dx, merr3
        jmp     error

cont3:

;        xor     edi, edi
;        jmp     cont6                   ; test v86 way

        mov     bx, 0
        mov     ax, 4f0ah
        int     10h                     ; get VESA 2.0+ PMode Interface Info
        cmp     ax, 4fh
        je      cont4

        xor     edi, edi
        jmp     cont6                   ; use v86 instead

cont4:

        cld
        mov     si, [es:di+6]
        or      si, si
        jz      cont5                   ; no extra ports/memory required

        add     si, di
lports:
        mov     ax, [es:si]
        add     si, 2
        cmp     ax, 0ffffh              ; end of port list?
        jne     lports
        cmp     word [es:si], 0ffffh    ; end of memory list?
        je      cont5                   ; no memory required, just ports

        xor     edi, edi
        jmp     cont6                   ; use v86 instead

cont5:

        add     di, [es:di]
        movzx   edi, di
        mov     eax, es
        shl     eax, 4
        add     edi, eax        ; edi = entry point to the VBE fn 5

cont6:

        mov     edx, mode_info  ; edx = pointer to mode info

;;;;;;;;;;;;;;;;;;;;;;;
;; let's enter PMode ;;
;;;;;;;;;;;;;;;;;;;;;;;

        mov     eax, esp
        and     eax, 0ffffh
        mov     esp, eax

        mov     ebx, cs
        mov     ecx, ebx        ; ecx = RMode CS
        shl     ebx, 4          ; ebx = 32-bit base address of segments

        add     [gdtr+2], ebx   ; fixup gdtr.base

        mov     [gdt.Code32+2], bx
        mov     [gdt.Data32+2], bx
        mov     [gdt.Code16+2], bx
        mov     [gdt.Data16+2], bx
        ror     ebx, 16
        mov     [gdt.Code32+4], bl
        mov     [gdt.Data32+4], bl
        mov     [gdt.Code16+4], bl
        mov     [gdt.Data16+4], bl
        ror     ebx, 16

        lgdt    [gdtr]

        push    dword 0
        popfd

        mov     eax, cr0
        inc     al
        mov     cr0, eax

        jmp     skip_data

error_gfx:
        mov     ax, 3
        int     10h
error:
        mov     ah, 9
        int     21h
        mov     ax, 4c00h
        int     21h

;;;;;;;;;;
;; data ;;
;;;;;;;;;;

mtitle  db      "LFBemu v 2.2 - PMode LFB emulator for VESA cards that have no LFB",13,10
        db      "               by Alexei A. Frounze (c) Apr 2001",13,10
        db      "               e-mail  : alexfru@chat.ru",13,10
        db      "               homepage: http://alexfru.chat.ru",13,10,"$"
merr0   db      "CPU is already in protected mode. can not continue.",13,10,"$"
merr1   db      "no VESA BIOS found.",13,10,"$"
merr2   db      "no info on 640x480x8bpp VESA mode available.",13,10,"$"
merr3   db      "could not set up 640x480x8bpp VESA mode.",13,10,"$"

gdt             dd      00000000h, 00000000h    ;0000
gdt.Code32      dd      0000ffffh, 00cf9a00h    ;0008
gdt.Data32      dd      0000ffffh, 00cf9200h    ;0010
gdt.Code16      dd      0000ffffh, 00009a00h    ;0018
gdt.Data16      dd      0000ffffh, 00009200h    ;0020
gdt.Flat        dd      0000ffffh, 00cf9200h    ;0028
gdt.TSS         dd      00002067h, 00008900h    ;0030

gdtr            dw      gdtr - gdt - 1
                dd      gdt

vesa_info       times   512 db 0
mode_info       times   256 db 0

skip_data:
    nop
    jmp skip_data

times 0x3f00 - ($ - $$) db 0x99
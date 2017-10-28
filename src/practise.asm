use16       ; 16-bit mode

org 0x7C00 ; address of the boot sector

HEIGHT equ 0x10
WIDTH equ 0x08

BootStageOne:
    mov ah, 0h
    mov al, 11h ; AH=0 (Change video mode), AL=13h (Mode)
    int 10h ; Video BIOS interrupt

    mov ah, 02h
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 80h
    mov bx, fontdata
    int 13h
    mov bx, font
    int 13h
    jmp fontdata
    times 510-($-$$) db 0
end:
    dw 0xAA55

SECTION .text
fontdata:
   mov si, font+16
   mov al, [font]

hl:
    nop
    jmp hl

font:
    times 20 db 0x10
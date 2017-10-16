;
; label = address (ptr)
; [label] = value (*ptr)
[BITS 16]
[ORG  0xc0000]

db 0x55, 0xaa
db 0x4

    push ax
    push si
    mov si, msg
ch_loop:
    lodsb
    or al, al ; zero=end or str
    jz hang   ; get out
    mov ah, 0x0E
    int 0x10
    jmp ch_loop

hang:
    mov ah, fonts
    int 0x10
    retf

msg   db 'Fonts loaded at address', 13, 10, 0
fonts incbin "NEXT.FNT.asm"

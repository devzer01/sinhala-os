;
; Note: this example is written in Intel Assembly syntax
;
[BITS 16]
[ORG  0x7c00]

boot:
    mov ax,0x1112
    mov bl, 0x0
    int 0x10
    
    mov ax, 0x1300
    mov bh, 0x00
    mov bl, 0x07
    mov cx, 0x0a
    mov dx, 0x0000
    mov bp, textp
    int 0x10

    ;mov al, 0x43
    ;mov ah, 0x0e
    ;mov cx, 0x0f
    ;int 0x10
    
    jmp $

times 510-($-$$) db 0

db 0x55
db 0xaa



textp db 'abcdefghij'


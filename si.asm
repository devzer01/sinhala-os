;
; Note: this example is written in Intel Assembly syntax
;
[BITS 16]
[ORG  0x7c00]

boot:
    
    ; video mode
    mov al, 87h
    mov ah, 00h
    int 10h

    ; load font
    mov ax, 1112h
    mov bl, 00h
    int 10h
    
    mov al, 0h
    mov ah, 05h
    int 10h

    ; set cursor 
    mov ah, 02h
    mov bh, 00h
    mov dh, 10h
    mov dl, 00h
    int 10h

    mov al, 43h
    mov ah, 09h
    mov bh, 00h
    mov bl, 07h
    mov cx, 1h
    int 10h
    
    mov ah, 02h
    inc dl
    int 10h

     mov al, 44h
     mov ah, 09h
     mov bh, 00h
     mov bl, 07h
     mov cx, 1h
     int 10h

    
    jmp $

times 510-($-$$) db 0

db 0x55
db 0xaa



textp db 'abcdefghij'


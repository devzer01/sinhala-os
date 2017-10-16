;
; label = address (ptr)
; [label] = value (*ptr)
[BITS 16]
;[ORG  0x0009]
;    dw kbrd
; 40:0010 => 0410
; f000:fff0 -> f0fff0
;section .text start=0x7c00
[ORG  0x7c00]
boot:

    mov ah, 4fh
    mov al, 02h
    mov bx, 108h
    mov cx, 0a0h
    int 10h

    mov ah, 4fh
    mov al, 06h
    mov bl, 00h
    mov cx, 0a8h
    int 10h

    xor ax, ax
    mov ds, ax,
    mov ss, ax
    mov sp, 0x9c00
    mov ax, 0xb800
    mov es, ax
    xor di, di
start:
    mov ah, byte [beta]
    mov bh, byte [band]
    cmp ah, bh
    jz done
    call charwrite
    inc di
    inc di
    mov bh, [beta]
    inc bh
    mov [beta], bh
    jmp start

charwrite:
    mov al, [beta]
    mov ah, 07h
    mov [es:di], ax
    ret

done:
    hlt

band 	   DB  0ffh, 0
beta       db  0a0h, 0

    times 510-($-$$) db 0
end:
    dw 0xAA55

retval: times 256 db 0
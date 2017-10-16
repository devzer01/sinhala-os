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
    mov ah, 00h
    mov al, 11h
    int 10h
    call fntinfo
    add  bp, 65

start:
    call charwrite
    inc byte [base]
    cmp byte [base], 0x08
    jne start
backnc:
nextchr:
    inc bp
    jmp start

charwrite:
    mov ds, word [fseg]
    mov si, word [fidx]
    mov bh, byte [base]
    mov al, 00h
    mov ah, byte [ds:si]
    and bx, 0x00FF
    and ax, 0x00FF
    bt ax, bx
    jnc keepbit
    mov al, 01h
    jmp skipbit
keepbit:
    mov al, 00h
skipbit:
    mov bh, 00h
    mov ah, 0ch
    mov cx, [col]
    mov dx, [row]
    int 10h
    inc byte [ptr]
    inc word [col]
    push ax
    mov ax, [maxcol]
    cmp [col], ax
    je incx
back:
    pop ax
    cmp byte [ptr], 04h
    jne skipbit
    ret
incx:
    inc word [row]
    mov word [col], 0x00
    jmp back

fntinfo:
    mov ax, 1130h
    mov bh, 02h
    int 10h
    mov [base], es
    mov [ptr], bp
    mov [lines], dl
    mov [bpch], cx
    ret

done:
    hlt

;;band 	   dw  0ffffh, 0
;;beta       dw  0a0h, 0
maxrow  dw 480
maxcol  dw 80*8
bpch db 00h
lines db 00h
fseg dw 0000h
fidx dw 0000h
col  dw 00h
row  dw 00h
base db 00h
ptr  db 00h

    times 510-($-$$) db 0
end:
    dw 0xAA55

retval: times 256 db 0
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
    ; video mode; video mode 02h worked
    ;cli
    ;mov bx, 0024h
    ;mov cx, kbrd
    ;mov [bx], cx ;only bx can be used for indexing
    ;mov cx, 0000h
    ;mov [bx+2], cx
    ;sti
    ;call loadfont
    mov al, 00h
    call videomode
    call loadfont
    call selectpage
    mov dx, 0001h ;move to begin
    call cursur
    push dx
    lea si,[alpha]
start:
    mov al,[si]
    cmp al, 0
    jz done
    call charwrite
    pop dx
    inc dx
    call cursur
    push dx
    inc si
    jmp start

kbrd:
    push ax
    call charwrite
    pop ax
    ret

; wait for 'any key':
done:
    mov ah,0h
    int 16h
    lea si,[alpha]
    jmp start

loop:
    mov ax, 0000h
    inc ax
    push ax
    call videomode
    call waitkey
    jmp loop

waitkey:
    mov ax, 0000h
    int 16h
    ret

charwrite:
    ;pop cx
    ;mov al, cl
    ;mov al, [char]
    mov ah, [intPrint]
    mov bh, [page]
    mov bl, 07h; 0x07 white ; 10h blue
    mov cx, 01h
    int 10h
    mov [mode], al
    ret

selectpage:
    mov al, [page]
    mov ah, [intPage]
    int 10h
    ret

videomode:
    ;pop cx
    ;mov al, cl
    mov ah, [intMode]
    int 10h
    ret

loadfont:
    mov ax, [intFont]
    mov bl, 00h
    int 10h
    ret

cursur:
    ; pop dx
    mov ah, [intCursor]
    mov bh, [page]
    ;mov dh, [row]
    ;mov dl, [col]
    int 10h
    ret
            ;a  , aa         ,ae        ,aee        ,e    ,ee
alpha 	DB  0a5h, 0a5h, 0a6h, 0a5h, 0a7h, 0a5h, 0a8h, 0a9h, 0aah, 0abh, 0abh, 0ach, 0b1h, 0b4h, 0b9h, 0bah, 0

col        db 00h
char       db 43h
mode7      db 07h
mode0      db 00h
mode1      db 01h
mode2      db 02h
mode3      db 03h
page       db 00h
repeat     db 01h
intPrint   db 09h ; calls int 10h 9h
intPage    db 05h ; calls int 10h 5h
intFont    dw 1112h ; load font
intMode    db 00h ; mode
intCursor  db 02h
mode       db 00h
row        db 00h

    times 510-($-$$) db 0
end:
    dw 0xAA55

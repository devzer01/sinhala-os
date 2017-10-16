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
    mov al, 11h
    call videomode
    ;call loadfont
    ;call selectpage
    call gfxfont
    mov dx, 0001h ;move to begin
    call cursur
    push dx
start:
    mov ah, byte [beta]
    mov bh, byte [band]
    cmp ah, bh
    jz done
    call charwrite
    pop dx
    inc dx
    call cursur
    push dx
    mov bh, [beta]
    inc bh
    mov [beta], bh
    jmp start

; wait for 'any key':
done:
    mov ah,0h
    int 16h
    jmp start

charwrite:
    ;pop cx
    ;mov al, cl
    mov al, [beta]
    mov ah, [intPrint]
    mov bh, [page]
    mov bl, 07h; 0x07 white ; 10h blue
    mov cx, 01h
    int 10h
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

gfxfont8:
    mov ax, 1123h
    mov bl, 03h
    int 10h
    ret

gfxfont:
    mov ax, 1124h
    mov bl, 03h
    int 10h
    ret

loadfont8:
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
band 	   DB  0ffh, 00a5h, 0a5h, 0a6h, 0a5h, 0a7h, 0a5h, 0a8h, 0a9h, 0aah, 0abh, 0abh, 0ach, 0b1h, 0b4h, 0b9h, 0bah, 0
beta       db  0a0h, 0

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
mode       db 11h
row        db 00h

    times 510-($-$$) db 0
end:
    dw 0xAA55

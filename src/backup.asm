use16       ; 16-bit mode

org 0x7C00 ; address of the boot sector

BootStageOne:
    mov ax, 12h ; AH=0 (Change video mode), AL=13h (Mode)
    int 10h ; Video BIOS interrupt

    mov ah, 09h
    mov dl, 80h
    int 13h

    mov ah, 08h
    mov dl, 80h
    mov di, 0x500
    int 13h

    mov ah, 02h
    mov al, 02h
    mov ch, 0
    mov cl, 2
    mov dh, 1
    mov dl, 80h
    mov bx, 0x500
    int 13h

    mov ax, 0A000h ; The offset to video memory
    mov es, ax ; We load it to ES through AX, becouse immediate operation is not allowed on ES
    mov ax, 0 ; 0 will put it in top left corner. To put it in top right corner load with 320, in the middle of the screen 32010.
    mov di, ax ; load Destination Index register with ax value (the coords to put the pixel)
    mov dl, 7 ; Grey color.
    mov [es:di], dl ; And we put the pixel

    mov ax,67 ; Y coord
    mov bx,112 ; X coord
    mov cx,320
    mul cx; multiply AX by 320 (cx value)

    add ax,bx ; and add X
    mov di,ax
    mov dl,7
    mov [es:di],dl
    inc ax
    inc bx
    mov di, 1000
    push di
drawmore:
    mov cx, 32          ;set font width to 12
    mov si, 0
    mov ax, word fontdata
    mov ds, ax
    repz

again:
    pop di
    mov bx,320
    mov ax,di
    add ax,bx
    mov di,ax
    push di
    mov cx,32

    repz
    jmp again
e:
    nop
    jmp e
    times 510-($-$$) db 0
end:
    dw 0xAA55

fontdata: db 7, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 5, 5, 5
             db 7, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5
                          db 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5
                          db 5, 5, 5, 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5
                          db 5, 5, 5, 5, 5, 5, 5, 5, 5
    db 7, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5
                 db 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5
                 db 5, 5, 5, 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5
                 db 5, 5, 5, 5, 5, 5, 5, 5, 5
db 7, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 5, 5, 5
db 7, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 5, 5, 5
db 7, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 6, 7, 5, 6, 8, 5, 5, 5
             db 5, 5, 5, 5, 5, 5, 5, 5, 5

              times 4096-($-$$) db 9
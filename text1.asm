;
; label = address (ptr)
; [label] = value (*ptr)
[BITS 16]

[ORG  0x7c00]
boot:
    mov ah, 00h
    mov al, 11h
    int 10h
    call displayinfo
start:
    call charwrite
    mov ch, [bCrtRows]
    cmp byte [row], ch
    jne start
    jmp done

charwrite:
    mov al, [pattern]
    mov bh, 00h
    mov ah, 0ch
    mov cx, [col]
    mov dx, [row]
    xor al, cl
    int 10h
    inc byte [col]
    mov al, [bCrtClms]
    cmp [col], al
    je nextline
back:
    ret
nextline:
    inc byte [row]
    mov byte [col], 0x00
    jmp back

displayinfo:
    mov ah, 1bh
    mov bx, 000h
    lea di, [retval]
    int 10h
    ret
done:
    mov ch, [pattern]
    add ch, 1
    cmp ch, 0xff
    je halt
    mov [pattern], ch
    xor ch, ch
    mov [row], ch
    mov [col], ch
    jmp start
halt:
    hlt

pattern db 0x01
row db 00h
col db 00h

    times 510-($-$$) db 0
end:
    dw 0xAA55

retval:
    pfrFnality times 4 db 0
    bCursorMode db 0
    bCrtClms times 2 db 0
    wCrtBufLen dw 0h
    pCrtPgStart dw 0h
    awCrsrPos times 16 db 0
    wCrsrType times 2 db 0
    bCurPg db 0
    wCrtcPort times 2 db 0
    bModeSetReg db 0
    bClrSetReg db 0
    bCrtRows db 0
    bCrtPoints dw 0
    bCurDcc db 0
    bAltDcc db 0
    wMaxClrs dw 0
    bMaxPgs db 0
    bScanLnsCode db 0
    bFont1 db 0
    bFont2 db 0
    rMiscFlags db 0
    resv times 3 db 0
    bVidMemCode db 0
    rSaveStatus db 0
    resv2 times 0dh db 0
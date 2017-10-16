;
; label = address (ptr)
; [label] = value (*ptr)
[BITS 16]
[ORG  0x7c00]

boot:
    ; video mode; video mode 02h worked
    call loadfont
    mov al, 00h
    call videomode
    call loadfont
    call selectpage
    mov al, 42h
    call charwrite
    mov dx, 0001h
    call cursur
    mov al, 0A6h
    call charwrite
    mov dx, 0002h
    call cursur
    mov al, 0x44
    call charwrite
    mov cx, 44h
    mov al, 0x80
    push cx
    push ax
lp:
    inc dx
    call cursur
    pop ax
    inc al
    call charwrite
    pop cx
    push ax
    inc cx
    cmp cx, 90h
    jne lp
; load message address into SI register:
;	LEA SI,[msg]
; screen function:
;	MOV AH,0Eh
;print:  MOV AL,[SI]
;	CMP AL,0
;	JZ done		; zero byte at end of string
;	INT 10h		; write character to screen.
;     	INC SI
;	JMP print

; wait for 'any key':
done:   MOV AH,0
    	INT 16h		; waits for key press
			; AL is ASCII code or zero
			; AH is keyboard code
			; store magic value at 0040h:0072h to reboot:
            ;		0000h - cold boot.
            ;		1234h - warm boot.
            	MOV  AX,0040h
            	MOV  DS,AX
            	MOV  word[0072h],0000h   ; cold boot.
            	JMP  0FFFFh:0000h	 ; reboot!

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
msg 	DB  'Welcome, I have control of the computer.',13,10
	    DB  'Press any key to reboot.',13,10
	    DB  '(after removing the floppy)',13,10,0
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

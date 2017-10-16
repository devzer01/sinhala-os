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


	push ds			;
	pop	es			; make sure ES = DS
	mov	bp,OurFont	;
	mov	cx,01			; we'll change just 2 of them
	mov	dx,65			;   A and B --> our A and B
	mov	bh,28			; 14 bytes per char
	xor	bl,bl			; RAM block
	mov	ax,1100h		; change font to our font
	int	10h			; video interrupt

    mov al, 65
	mov ah, 09h
        mov bh, 0h
        mov bl, 07h; 0x07 white ; 10h blue
        mov cx, 01h
        int 10h


OurFont:
	    db	00000000b, 00000000b
    	db	01111111b, 01111110b
    	db	01100011b, 01100110b
    	db	01100011b, 01100110b
    	db	01100011b, 01100110b
    	db	01111111b, 01111111b
    	db	01100011b, 01100011b
    	db	01100011b, 01100011b
    	db	01100011b, 01100011b
    	db	01100011b, 01100011b
    	db	01100011b, 01100011b
    	db	01100011b, 01111111b
    	db	00000000b, 00000000b
    	db	00000000b, 00000000b


















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

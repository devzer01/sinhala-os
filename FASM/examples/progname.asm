
; see http://www.cybertrails.com/~fys/progname.htm for more information

;This example shows how to retrieve the programs name and path (dir) where it is located on the disk.

.model tiny
.code
		org	100h			; we know that on entry ds=es=cs=ss
start:		mov	ax,es:[002Ch]		; get environment address from 'PSP'
		mov	es,ax			; es:di = environment
		xor	di,di			; start at beginning
		xor	al,al			; clear AL
		mov	cx,-1			; cx = -1
p1_loop1:	repne
		scasb				; Find double zero
		scasb				;
		jne	short p1_loop1		;
		push	ds			; save data seg
		push	ds			; swap es,ds
		push	es			;
		pop	ds			;
		pop	es			;
		lea	si,[di+2]		; DS:SI = string
		mov	di,offset buffer	; ES:DI = buffer
p1_loop2:	lodsb				; Copy the string
		stosb
		or	al,al
		jnz	short p1_loop2
		mov	al,36			; put a '$' for service 09 below
		stosb				;
		pop	ds			; restore Data Seg

		mov	dx,offset Buffer	; DOS print string service
		mov	ah,09			;
		int	21h			;

		int	20h			; exit to DOS

Buffer:						; we don't need to define the len
.end	start


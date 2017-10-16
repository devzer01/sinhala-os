
; see http://www.cybertrails.com/~fys/vesa.htm for more information

.model tiny
.code
	org	100h
	mov	ax,cs		; free unused part of Mem Block
	mov	es,ax		;   for .COM file format
	mov	bx,4096		;
	mov	ah,4Ah		;
	int	21h		;

	mov	ax,4F00h	; Service 4Fh of INT 10h
	mov	di,offset Buff	;  returns info (need place for it)
	int	10h		;
	cmp	al,4Fh		; if al = 4Fh then success
	je	short IsSup	;
	mov	dx,offset NotSupS ; else fail.
	mov	ah,09		;
	int	21h		;
	jmp	short Done	;
IsSup:	push	ax		; save RC
	mov	dx,offset SupS	;
	mov	ah,09		;
	int	21h		;
	pop	ax		; restore RC
	mov	dx,offset SuccS	; assume Successful
	xor	ah,ah		; completely successful??
	jz	short PrntIt	;
	mov	dx,offset FailS	; Failed
PrntIt:	push	ax		; save RC
	mov	ah,09		;
	int	21h		;
	pop	ax		; restore RC
	xor	ah,ah		; if success then DONE
	jz	short Done	;
	mov	dx,offset FailS1 ; Assume hardware
	cmp	ah,02		;
	je	short PrntIt1	;
	mov	dx,offset FailS2 ; video mode
PrntIt1:	mov	ah,09	;
	int	21h		;
Done:	int	20h		; exit to DOS

NotSupS	db  13,10,'This system does not support VESA comp. video',36
SupS	db  13,10,'This system supports VESA comp. video'	
	db  13,10,'  status: ',36
SuccS	db  'OK',36
FailS	db  'Failed',36
FailS1	db  '  Function not supported by current hardware config.',36
FailS2	db  '  Function invalid in current mode',36
Buff:

.end

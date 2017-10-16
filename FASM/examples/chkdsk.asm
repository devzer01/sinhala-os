
; see http://www.cybertrails.com/~fys/chkdsk.htm for more information

.MODEL tiny
.CODE
	org 100h
	
	mov	ah,04h			; Verify Disk Sectors
	mov	al,01h			; Amount to verify
	mov	ch,01h			; High order cylinder number (8 bits)
	mov	cl,01h			; Sector Number in bits 0-5
	xor	dl,dl			; use drive A:
	xor	dh,dh			;    remaining 6-7 bits are low order for CH
	int	13h			; DH is the head number
	xor	al,al 			; DOS' ERRORLEVEL
	jnc	Done			;
	mov	dl,07h			; simple beep
	mov	ah,02h			;
	int	21h			;
	mov	al,01h			; DOS' ERRORLEVEL
Done:	mov	ah,4Ch			; exit to DOS
	int	21h			;
.end					; End of assembly code


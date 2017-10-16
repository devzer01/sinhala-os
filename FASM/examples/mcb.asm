
; see http://www.cybertrails.com/~fys/mcb.htm for more information

;
; This routine simply sets an ENVIRONMENT VARIABLE to
;  the current date in YYYMMDD form using COMMAND.COM and SET.
;
; assembled with NBASM 00.24.42
;
.model tiny
.code
.186

           .start                       ; free unused part of memory block

           mov  ah,2Ah                  ; DOS get date function
           int  21h                     ; cx=year, dh=month, dl=day

           push cs                      ; make sure es=cs
           pop  es                      ;

           cld                          ; make sure going forward

           mov  di,offset our_strng     ;
           mov  ax,cx                   ; year
           call prtdec                  ;
           xor  ah,ah                   ;
           mov  al,dh                   ; month
           call prtdec                  ;
           mov  al,dl                   ; day
           call prtdec                  ;

           mov  si,offset commnd_str    ; call command.com to run it
           int  2Eh                     ;

           .exit

; Prints a number to ES:DI in decimal asciiz representation
;  if the number is a single digit, left pads with '0'
; on entry:
;     ax =  number
;  es:di -> position to write to
PrtDec     proc near uses ax cx dx
           cmp  ax,09                   ; if 0 - 9, left pad it
           ja   short prtdecnp          ;
           mov  byte es:[di],'0'        ; left pad a '0'
           inc  di                      ;
prtdecnp:  mov  cx,0FFFFh               ; Ending flag
           push cx
           mov  cx,10
PD1:       xor  dx,dx
           div  cx                      ; Divide by 10
           add  dl,30h                  ; Convert to ASCII
           push dx                      ; Store remainder
           or   ax,ax                   ; Are we done?
           jnz  short PD1               ; No, so continue
PD2:       pop  ax                      ; Character is now in AL
           cmp  ax,0FFFFh               ; Is it the ending flag?
           je   short PD3               ; Yes, so continue
           stosb                        ;
           jmp  short PD2               ; Keep doing it
PD3:       ret
PrtDec     endp

commnd_str db  21
           db  'set ourdate='
our_strng  db  'yyyymmdd'
           db  13

.end

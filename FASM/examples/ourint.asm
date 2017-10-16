
; see http://www.cybertrails.com/~fys/ourint.htm for more information

; This is a small util. to show how to create your own
; interrupt.  
;
; (version 1.00b - original release)
;
; OUTINT.ASM
; Forever Young Software
; Copyright 1997
; All rights reserved
; Version 1.00b

.model tiny
.186
           org 100h
start:     jmp short InstallIt
;*******************************************************************
;;;; our interrupt code and data goes here
.algnw   ; lets align it on the next word boundary
OurStart:  cli
           push ds                      ; save data seg
           push cs                      ; point ds to cs
           pop  ds
           or   ah,ah                   ; if ah = 0 then service 0
           je   short Service0
           cmp  ah,01                   ; if ah = 1 then service 1
           je   short Service1
           cmp  ah,02                   ; if ah = 2 then service 2
           je   short Service2
JustExit:  pop  ds                      ; else just return
           sti
           iret

;  our service #0  (ah = 00h)
Service0:  mov  dx,00h                  ; do something simple for example
           jmp  short JustExit

;  our service #1  (ah = 01h)
Service1:  mov  dx,01h                  ; do something simple for example
           jmp  short JustExit

;  our service #2  (ah = 02h)
Service2:  mov  dx,02h                  ; do something simple for example
           jmp  short JustExit
;  other services go here

OurEnd:
;;;; end of our interrupt code and data
;*******************************************************************

InstallIt: mov  si,offset StartIt       ; display start up string
           call prtstring               ;

           xor  bx,bx                   ; make es:bx point to 0000:00D0h
           mov  es,bx                   ;
           mov  bx,00D0h                ;

           mov  cx,204                  ; 256 interrupts (00-FF)(-34h)
MainLoop:  mov  si,es:[bx]              ; get interrupt vector address
           inc  bx                      ;  offset first (little endian)
           inc  bx                      ;
           mov  ax,es:[bx]              ;  segment second (little endian)
           inc  bx                      ;
           inc  bx                      ;
           cmp  ax,0F000h               ; if segment => F000h then in ROM-BIOS
           jae  short NotIRET           ; (if in BIOS then skip it)
           push es                      ;
           mov  es,ax                   ; see if IRET
           mov  dl,es:[si]              ;
           pop  es                      ;
           cmp  dl,0CFh                 ; is it IRET
           je   short FndOne            ;  if so, then not used
NotIRET:   inc  byte CIntVNum           ; inc for next time
           loop MainLoop                ;
           mov  si,offset NoFndOneS     ; didn't find one
           call prtstring
           jmp  short DoneErr

FndOne:    mov  si,offset FndOneS
           call prtstring
           mov  ax,bx
           sub  ax,04                   ; backup to first of vector
           call prthex
           mov  si,offset IntNumS
           call prtstring
           mov  al,CIntVNum             ; display current int number
           call prthexs                 ;
           mov  dl,'h'                  ; print 'h'
           mov  ah,02                   ;
           int  21h                     ;

           push cs
           pop  ds
           mov  al,CIntVNum             ; vector to change
           mov  ah,25h
           mov  dx,offset cs:OurStart
           int  21h

           mov  ah,62h                  ; get and free environment block
           int  21h                     ;
           mov  es,bx                   ;
           mov  ax,es:[2Ch]             ;
           mov  es,ax                   ;
           mov  ah,49h                  ;
           int  21h                     ;

           mov  dx,offset cs:OurEnd     ; offset of end of our code
           shr  dx,04                   ; div by 16 (make it a paragraph #)
           inc  dx                      ; just in case
           add  dx,16                   ; The PSP
           mov  ax,3100h                ; service number and RC = 0
           int  21h                     ; TSR service
                      
DoneErr:   mov  ah,4Ch                  ; exit to DOS (no TSR)
           int  21h                     ; 


prtstring  proc near uses ax dx si
ps1:       mov  dl,[si]                 ; Get character
           inc  si                      ; Point to next one
           or   dl,dl                   ; End of string?
           jz   short ps2               ; Yes, so exit
           mov  ah,02h                  ; Output a character
           int  21h
           jmp  short ps1               ; Keep doing it
ps2:       ret
prtstring  endp

PrtHexs    proc near uses ax bx dx cx
           mov  bx,offset Hex
           push ax
           shr  al,04h
           xlatb
           mov  dl,al
           mov  ah,02
           int  21h
           pop  ax
           and  al,0Fh
           xlatb
           mov  dl,al
           mov  ah,02
           int  21h
           ret
PrtHexs    endp

PrtHex     proc near uses ax bx cx dx
           mov  bx,offset Hex
           mov  cx,04h
HexLoop:   push ax
           mov  al,ah
           shr  al,04h
           xlatb
           mov  dl,al
           mov  ah,02
           int  21h
           pop  ax
           shl  ax,04h
           loop HexLoop
           ret
PrtHex     endp

CIntVNum   db  34h           ; starting with int 34h
StartIt    db  13,10,'Interrupt Install Util              Version   01.00b'
           db  13,10,'Forever Young Software(r)   (C)opyright 1984-2000',13,10
           db  13,10,' Finding unused Interrupt vector number... (starting with 34h)',13,10,0
NoFndOneS  db  13,10,"****Didn't find an unused interrupt vector number***"
FndOneS    db  13,10,'  Found an unused interrupt at:  0000:',0
IntNumS    db  13,10,'  Interrupt number:  ',0
Hex        db  '0123456789ABCDEF'

.end  start

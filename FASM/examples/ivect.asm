
; see http://www.cybertrails.com/~fys/ivect.htm for more information

; This is a small util. to display the segment address of
; each interrupt vector.  You can take this address of
; a given interrupt number and use it in DEBUG with the
; -u command to view the code of this interrupt.

.model tiny
.186
           org  100h
start:     mov  ax,cs                   ; free unused part of Mem Block
           mov  es,ax                   ;   for .COM file format
           mov  bx,4096                 ;
           mov  ah,4Ah                  ;
           int  21h                     ;

           mov  si,offset StartIt       ; display start up string
           call prtstring               ;

           xor  bx,bx                   ; make es:bx point to 0000:0000
           mov  es,bx

           mov  cx,256                  ; 256 interrupts (00-FF)
MainLoop:  push cx
           mov  si,offset InvectS       ; display string
           call prtstring               ;
           mov  al,CIntVNum             ; display current int number
           call prthexs                 ;
           inc  byte CIntVNum           ; inc for next time
           mov  dl,'h'                  ; print 'h'
           mov  ah,02                   ;
           int  21h                     ;
           mov  si,offset InINumS       ; display string
           call prtstring               ;
           mov  ax,es:[bx]              ; get interrupt vector address
           inc  bx                      ;  offset first (little endian)
           inc  bx                      ;
           push ax                      ;
           mov  ax,es:[bx]              ;  segment second (little endian)
           mov  cx,ax                   ;  save segment
           inc  bx                      ;
           inc  bx                      ;
           call prthex                  ; and print segment
           mov  dl,':'                  ; print ':'
           mov  ah,02                   ;
           int  21h                     ;
           pop  ax                      ;
           push ax                      ; save offset for IRET test
           call prthex                  ; print offset
           mov  dl,'h'                  ; print 'h'
           mov  ah,02                   ;
           int  21h                     ;
           cmp  cx,0F000h               ; if segment => F000h then in ROM-BIOS
           jb   short NotinBIOS         ;
           mov  si,offset InBIOSS       ;
           call prtstring               ;
NotinBIOS: pop  si                      ; restore offset
           push es                      ;
           mov  es,cx                   ; see if IRET
           mov  al,es:[si]              ;
           pop  es                      ;
           cmp  al,0CFh                 ; is it IRET
           jne  short NotIRET           ;  if so, then not used
           mov  si,offset NotUsedS      ;
           call prtstring               ;
NotIRET:   mov  si,offset CRLF          ;
           call prtstring               ;
           pop  cx                      ;
           loop MainLoop                ;
Done:      mov  ah,4Ch                  ; exit to DOS
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

CIntVNum   db  00h

StartIt    db  13,10,'Interrupt Vector Display Util        Version   01.10b'
           db  13,10,'Forever Young Software(r)   (C)opyright 1984-2000',13,10,10,0
InvectS    db  13,10,'   Interrupt Vector Number:  ',0
InINumS    db  13,10,'        at segment address:  ',0
InBIOSS    db  13,10,'     In ROM-BIOS memory area.',0
NotUsedS   db  13,10,' Interrupt not used.',0
CRLF       db  13,10,0
Hex        db  '0123456789ABCDEF'

.end  start


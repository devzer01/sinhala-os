
; see http://www.cybertrails.com/~fys/gameport.htm for more information

; ************************************************************************
; *  This is a small snip of code to detect the game port.
; *
; *  You may use this code as you would like.
; *  As always, If this code crashes you machine in anyway I am
; *  not held responsible.  Use at your own risk.
; *

.model tiny
.code
           org  100h
start:     mov  ax,cs                   ; free unused part of Mem Block
           mov  es,ax                   ;   for .COM file format
           mov  bx,4096                 ;
           mov  ah,4Ah                  ;
           int  21h                     ;

           mov  al,01h                  ; value to write to
           mov  dx,0201h                ; port number 0201h
           out  dx,al                   ; 

           mov  bx,offset yes           ; assume gameport installed
           mov  cx,0F00h                ; number of loops
port_loop: in   al,dx                   ; read from port
           and  al,0Fh                  ; if joystick present, then AL should
           cmp  al,0Fh                  ; be 0Fh after ANDing with 0Fh.
           je   short done
           loop port_loop
           mov  bx,offset no            ; gameport not installed
done:      mov  dx,bx                   ; bx = offset of string to print
           mov  ah,09h
           int  21h
           ret

yes        db  13,10,'Game port installed.',13,10,36
no         db  13,10,'Game port not installed.',13,10,36

.end  start


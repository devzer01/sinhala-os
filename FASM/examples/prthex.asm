
; see http://www.cybertrails.com/~fys/prthex.htm for more information

.model tiny
.code
.186
           org  100h

           mov  ax,1234h
           call PrtHex
           mov  ax,4C00h
           int  21h

PrtHex     proc near uses ax bx cx

           mov  bx,offset Hex
           mov  cx,04h
HexLoop:   push  ax
           mov  al,ah
           shr  al,04h
           xlatb
           mov  dl,al
           mov  ah,02
           int  21h
           pop  ax
           shl  ax,04h
           loop HexLoop
           mov  ah,02
           mov  dl,'h'
           int  21h
           ret
PrtHex     endp

Hex        db  '0123456789ABCDEF'

.end



; see http://www.cybertrails.com/~fys/snumber.htm for more information

; This is a small snip of code to get the Serial Number from a disk.
.model tiny
.code
.186      
           org  100h
start:     mov  ax,cs                   ; free unused part of Mem Block
           mov  es,ax                   ;   for .COM file format
           mov  bx,4096                 ;
           mov  ah,4Ah                  ;
           int  21h                     ;

           mov  bx,01h                  ; 1 = a:, 2 = b:, 3 = c:, etc
           mov  cx,0866h                ; 08h = disk, 66h = get Media ID
           mov  dx,offset InfoLevel     ; dx = offset of buffer
           mov  ax,440Dh                ; 440dh = block device generic IOCTL
           int  21h                     ; do it

           mov  si,offset SrNumS        ; print string
           call prtstring               ;
           mov  bx,SerialN
           mov  ax,[bx+2]               ; get LO order of double word
           call prthex                  ; print as hex
           mov  al,'-'                  ;   -
           int  29h                     ;
           mov  ax,[bx]                 ; get HI order of double word
           call prthex                  ; print as hex

           mov  si,offset LabelS        ; print string
           call prtstring               ;
           mov  si,offset DLabel        ; print fixed string
           mov  cx,11                   ; 11 chars
           call prtfixstr               ;

           mov  si,offset FileSS        ; print string
           call prtstring               ;
           mov  si,offset FSys          ; print fixed string
           mov  cx,08                   ; 08 chars
           call prtfixstr               ;

           mov  si,offset CRLF          ; print string
           call prtstring               ;

           int  20h                     ; exit to DOS


Prtstring  proc near
Ps1:       mov  al,[si]                 ; Get character
           inc  si                      ; Point to next one
           or   al,al                   ; End of string?
           jz   short ps2                     ; Yes, so exit
           int  29h                     ; Output a character
           jmp  short Ps1               ; Keep doing it
Ps2:       ret
Prtstring  endp

prtfixstr  proc near
Psf1:      mov  al,[si]                 ; Get character
           inc  si                      ; Point to next one
           int  29h                     ; Output a character
           loop Psf1                    ; Keep doing it
           ret
prtfixstr  endp

PrtHex     proc near uses ax bx cx
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

SrNumS     db  13,10,'Serial Number:  ',0
LabelS     db  13,10,'        Label:  ',0
FileSS     db  13,10,'  File System:  ',0
CRLF       db  13,10,0
InfoLevel  dw  00h                    ; info level
SerialN    dw  00h,00h                ; serial number
DLabel     dup 11,0                   ; vol label
FSys       dup 8,0                    ; file system
Hex        db  '0123456789ABCDEF'

.end  start


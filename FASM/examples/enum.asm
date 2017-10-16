
; see http://www.cybertrails.com/~fys/enum.htm for more information

;   ENUM
;  Enumerates all open files in DOS 7.x

.model tiny
.code
           org  100h

           xor  si,si             ; start with number 1  (si = index - 1)
More:      mov  ax,440Dh          ; generic IOCTR / drive function 0Dh
           mov  cx,086Dh          ; enumerate open files
           xor  bl,bl             ; current drive
           mov  dx,offset Buffer  ; Buffer
           xor  di,di             ; all files
           int  21h               ; do it
           jc   short Done        ; carry set if error (no more)

           push si                ; print the file returned
           mov  si,offset CRLF    ;  starting with a CRLF
           call prtstring         ;

           mov  bx,offset FTypeO  ; get file type (CX on return from above)
           shl  cx,1              ; index it
           add  bx,cx             ; add to bx
           mov  si,[bx]           ; and get it into si
           call prtstring         ; print it
           pop  si                ; restore index number

           inc  si                ; inc to next index number

           jmp  short More        ; do it again

Done:      .exit                  ; exit to DOS

Prtstring  proc near uses ax dx si
           mov  ah,02                   ; DOS print char service
Ps1:       lodsb                        ; Get character & point to next one
           or   al,al                   ; End of string?
           jz   short ps2               ; Yes, so exit
           mov  dl,al                   ; 
           int  21h                     ; Output a character
           jmp  short Ps1               ; Keep doing it
Ps2:       ret
Prtstring  endp

FTypeO     dw  offset Type1
           dw  offset Type2
           dw  offset Type3
           dw  offset Type4    ; filler
           dw  offset Type5
Type1      db  '  [normal file]',0
Type2      db  '  [memory-mapped file (unmovable)]',0
Type3      db  '  [unmovable file]',0
Type4      db  '  [*filler*]',0   ; shouldn't get this one
Type5      db  '  [swap file]',0


CRLF       db  13,10,0            ; CRLF
Buffer     dup 128,?              ; a 128 byte buffer

.end

; Demo program assembled with the NewBasic assembler
; (C)opyright  1984-2017  Forever Young Software
; Benjamin David Lunt
; 31 May 2017
;

One    equ  01h                         ; test EQU

.dosseg                                 ; DOS segment ordering
.8086                                   ; allow 8086 intructions
.model small                            ; create .OBJ file
.stack 100h                             ; stack of 256 bytes
.data

One1       db   01h
msg1       db   13,0Ah
           db    'ษ'
           dup   38,'อ'       ; test DUP
           db    'ป',13,10
           db   'บ              NewBasic++              บ',13,10
           db   'บ        Forever Young Software        บ',13,10
           db   'บ          Benjamin David Lunt         บ',13,10
           db   'บ                  ',157,'                   บ',13,10
           db   'บ         copyright  1984-2016         บ',13,10
           db   'บ           version 00.26.73           บ',13,10
           db   'ศออออออออออออออออออออออออออออออออออออออผ',13,10,36
scrllmsg   db   157
           db   '  NewBasic++  **  Forever Young Software  **  Benjamin David Lunt  **  2015 '
           db   157,13,36
CRLF       db   13,10,36
ThnkS      db   13,10,10,"      Forever Young thanks you...",36

.code                                   ; start of code segment
start:     mov  ax,%data                ; we need to set up DS and ES
           mov  ds,ax                   ;
           mov  es,ax                   ;
           mov  ah,One                  ; turn off cursor (test EQU)
           mov  ch,00100000b            ; bit number 5 (test BIN num)
           int  10h                     ;
           mov  dx,offset msg1          ; string output
           mov  ah,09h                  ;
           int  21h                     ;
scrolls:   mov  dx,offset scrllmsg      ; scroll section
           mov  ah,09h                  ;
           int  21h                     ;
           mov  si,dx                   ; move scrllmsg into si from dx above
           mov  di,dx                   ; move desination into di
           lodsb                        ; move 1st letter -> al move si to 2nd letter (save letter)
           mov  cx,'M'                  ; do it 77 times (test ischar)
           rep
           movsb                        ; move from source to destination
           stosb                        ; store string byte into destination (saved letter)
           mov  cx,2                    ;
           call sec_delay               ; pause execution
           mov  ah,06h                  ; see if key pressed
           mov  dl,0FFh                 ;
           int  21h                     ;
           jz   scrolls                 ; loop if no key pressed
           call PrtThnx                 ; test calling proc and ret - endp
           mov  dx,offset CRLF          ; Move down a line
           mov  ah,09h                  ;
           int  21h                     ;
           mov  ah,One1                 ; turn on cursor (test mem ref)
.186                                    ; allow 186 intructions
           push 0607h                   ; test the push of a const
.8086                                   ; allow 8086 intructions
           pop  cx                      ; cx = 0607h start = 6  finish = 7
           int  10h                     ;
           mov  ax,4C00h
           int  21h                     ; exit to DOS

PrtThnx    proc near uses ax dx
           mov  dx,offset ThnkS
           mov  ah,09
           call PrtThnxI                     
           ret
           endp

PrtThnxI   proc near uses ax            ; check the nesting of uses
           int  21h
           ret
           endp

; on entry, cx = X/18.2 seconds to wait (cx = 18 = 1 second)
sec_delay  proc near uses ax es
           xor  ax,ax
           mov  es,ax
outerloop: mov  ax,es:[046Ch]
innerloop: cmp  ax,es:[046Ch]
           je   short innerloop
           loop outerloop
           ret
sec_delay  endp

.end start

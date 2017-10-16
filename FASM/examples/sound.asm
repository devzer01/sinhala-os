
; see http://www.cybertrails.com/~fys/snd.htm for more information

; This routine shows how to send sound (freq.) to the internal speaker.
; You can sound a frequency between 1 and 4000+.  Please Note that the
; human ear has a hard time hearing a frequency less than about 440.
; I use a timer function to wait for the duration.  I also have
; the freq. and duration in a buffer and get a single freq. and duration
; value each time.  This is so that you can make quite a few different
; sounds and just point SI to that buffer and then call this routine.
; The 00h,00h (asciiz) at the end of the buffer tells this routine to
; quite.
;

.model tiny
.code
           org  100h
start:     push ds
           pop  es
           mov  si,offset SomeTune
           
           mov  dx,61h                  ; turn speaker on
           in   al,dx                   ;
           or   al,03h                  ;
           out  dx,al                   ;

           mov  dx,43h                  ; get the timer ready
           mov  al,0B6h                 ;
           out  dx,al                   ;

LoopIt:    lodsw                        ; load desired freq.
           or   ax,ax                   ; if freq. = 0 then done
           jz   short LDone             ;
           mov  dx,42h                  ; port to out
           out  dx,al                   ; out low order
           xchg ah,al                   ;
           out  dx,al                   ; out high order
           lodsw                        ; get duration
           mov  cx,ax                   ; put it in cx (16 = 1 second)
           call PauseIt                 ; pause it
           jmp  short LoopIt

LDone:     mov  dx,61h                  ; turn speaker off
           in   al,dx                   ;
           and  al,0FCh                 ;
           out  dx,al                   ;

           int  20h                     ; exit to DOS

  ; this routine waits for specified milliseconds
PauseIt    proc near uses ax bx cx dx
           xor  dx,dx                   ; cx = 16 = 1 second
           mov  ax,62500
           mul  cx
           mov  cx,dx
           xor  dx,dx
           mov  bx,offset PFlag
           mov  ax,8300h
           mov  [bx],al                 ; clear flag
           int  15h
WaitForIt: cmp  byte PFlag,00h
           je   short WaitForIt
           ret
PauseIt    endp

PFlag      db  00h          ; flag for pauseit routine
SomeTune   dw  1397,08
           dw  1397,08
           dw  1397,08
           dw  1318,06
           dw  1244,16
           dw  1046,04
           dw  1108,04
           dw  1174,04
           dw  1244,04
           dw  1174,08
           dw  1244,08
           dw  1318,08
           dw  1396,08
           dw  1480,08
           dw  1568,16
           dw  00h,00h

.end  start


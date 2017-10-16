
; see http://www.cybertrails.com/~fys/int24h.htm for more information

;
;  assembled with NBASM 00.23.xx
;
.model tiny
.code
.186
           org 100h


start:     mov  ax,3524h                ; get and save original vector
           int  21h                     ;
           mov  OldSeg,es               ;
           mov  OldOff,bx               ;
          
           mov  ax,2524h                ; set int24h vector to our handler
           mov  dx,offset OurHandler    ;
           int  21h                     ;

           call DoErr                   ; make an error (deliberately)
          
           mov  ax,2524h                ; restore settings
           mov  dx,OldOff               ;
           mov  ds,OldSeg               ;
           int  21h                     ;
          
Done:      mov  ah,4Ch                  ; exit to DOS
           int  21h                     ; 
          
DoErr      proc near uses ax dx         ; make an error (deliberately)
           mov  ah,19h                  ; get disk drive
           int  21h                     ;
           push ax                      ; and save it for later
           mov  ah,0Eh                  ; set drive to a:
           xor  dl,dl                   ;
           int  21h                     ;
           mov  ah,39h                  ; try to change a dir to on drv a:
           mov  dx,offset testfile      ;
           int  21h                     ;
           pop  dx                      ; restore current drive
           mov  ah,0Eh                  ;
           int  21h                     ;
           ret                          ;
DoErr      endp
          
OurHandler proc near                    ; our handler
           pusha                        ; save all registers
           mov  ax,cs                   ; make sure ds = cs
           mov  ds,ax                   ;
          
           mov  dx,offset OurErrS       ; we can make it do what we want
           mov  ah,09h                  ; as long as we don't use non-
           int  21h                     ;  conventional interrupts
          
           popa                         ; restore registers
           xor al,al                    ; tell DOS 'NO ERROR'
           iret                         ; interrupt return
OurHandler endp

; this must be in code segment for our handler to work
;
OurErrS    db  'This should print when our error handler is active',36
OldSeg     dw  00h
OldOff     dw  00h
testfile   db  'testcd',0

.end       start

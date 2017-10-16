
; see http://www.cybertrails.com/~fys/redir.htm for more information

; An assembly code snippet to show how to create
;  redirection in your programs.

stdout  equ 01h

.model tiny
.code
           org  100h
           
           mov  ah,3Ch                  ; create/truncate file
           mov  dx,offset FileName
           mov  cx,20h
           int  21h
           jc   short DoError
           mov  Handle1,ax              ; save handle of open file

           mov  bx,stdout
           mov  ah,54h
           int  21h
           jc   short DoError

           push ax                      ; save handle returned
           mov  bx,Handle1
           mov  cx,stdout
           mov  ah,46h                  ; force dup handle
           int  21h
           jc   short DoError

; redirection should start here
;  You could shell out to another program, all text output
;   would be sent to the handle denoted above.
;  For instance, you could call a DOS utility and then check
;   the file for the output, or just delete the file and
;   no text will show up on the screen.

           mov  dx,offset String
           mov  ah,09
           int  21h

; redirection should end here with the following code
           pop  bx                      ; restore previous handle
           mov  cx,stdout
           mov  ah,46h
           int  21h
           jc   short DoError

           mov  ah,3Eh                  ; and close it
           int  21h

           mov  bx,Handle1              ; and close output file
           mov  ah,3Eh                  ;
           int  21h

           jmp  short Done

DoError:   mov  si,offset ErrorS
DoEL:      lodsb
           or   al,al
           jz   short Done
           int  29h
           loop DoEL           

Done:      ret

ErrorS     db  13,10,'There was an error',13,10,0
FileName   db  'tempfile.tmp',0
Handle1    dw  00h
String     db  'This is a string to print to the stdout',13,10,36

.end

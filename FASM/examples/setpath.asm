
; see http://www.cybertrails.com/~fys/mcb.htm for more information

comment %
 Demo program assembled with the NBASM  
 22 Oct 2000

 It is quite simple really.  All we have to do is the following items:
  1. Find the master environment segment.
  2. Syphon out the PATH=... variable string
  3. Put the PATH= at the end of the rest of the variable strings
  4. Add the command line paramter to the end of the PATH
     (adding a ';' if needed)
  5. Resize the environment block to allow our new path to fit
  6. rep movsb it
  7. done
%

.186                                    ; allow 186 intructions
.model tiny                             ; create COM file
.code                                   ; start of code segment
           .start                       ; tell NBASM to free unused mem
                                        ;  so we can resize the env block
           mov  ah,52h                  ; (undocumented) get 1st MCB
           int  21h                     ; .
           mov  ax,es:[bx-2]            ; .
           mov  es,ax                   ; .

           add  ax,es:[0003h]           ; add length of 1st block
           inc  ax                      ;  (paragraphs)
           inc  ax                      ;  +2 to get command psp
           mov  es,ax                   ; and put in es

           mov  ax,es:[002Ch]           ; 2Ch = offset in PSP to environment
           or   ax,ax                   ; if = 0, 
           jnz  short validoff          ;  then must be DOS 3.x or below
           mov  ax,es                   ; dec  es
           dec  ax                      ;
           mov  es,ax                   ;
           add  ax,es:[0003h]           ; get address
           inc  ax                      ;

validoff:  dec  ax                      ; mov one paragraph back
           mov  es,ax                   ; and put in es
           mov  cx,es:[0003h]           ; cx = size of environment
           shl  cx,04                   ;   (in bytes)

           add  ax,04                   ; for Windows9x DOS session
           ;add  ax,02                   ; for True DOS 7.x
           ;inc  ax                      ; for DOS 4.01 and less

           mov  cs:EnvStrSeg,ax         ; save envirnment seg
           mov  ds,ax                   ; make ds:si point to first
           xor  si,si                   ;  envirnment var
           push cs                      ; make sure es = cs
           pop  es
           mov  di,offset cs:Buffer1

PrtLoop:   mov  al,ds:[si]              ; if null then we're done
           or   al,al                   ;
           jz   short DoneG             ;
PrtLoop1:  push cx                      ; see if it is PATH=
           push si                      ;
           push di                      ;
           mov  di,offset cs:PathStr    ;
           mov  cx,5                    ;
           repe                         ;
           cmpsb                        ;
           pop  di                      ;
           pop  si                      ;
           pop  cx                      ;
           jne  short NotPath           ;
           push di                      ; if it is PATH= then mov to buffer2
           mov  di,offset cs:Buffer2    ; (this will put all of the env in
IsPath:    lodsb                        ;  buffer1 and the PATH in buffer2)
           stosb                        ;
           dec  cx                      ; one less byte to get
           or   al,al                   ;
           jnz  short IsPath            ;
           pop  di                      ;
           jmp  short PrtLoop           ;

NotPath:   lodsb                        ; get the char
           stosb                        ; put in buffer1
           or   al,al                   ; if null then end of string
           jz   short PrtLoop           ;
           loop PrtLoop1                ; loop to next one
; if for some reason we don't find a double null string
;  we will only go cx times with the loop instruction above

DoneG:     push cs                      ; point ds to cs  (PSP)
           pop  ds
           mov  si,81h                  ; point si to command line
SkpSpcs:   lodsb                        ; get parameters
           cmp  al,20h                  ; skip leading spaces
           je   short SkpSpcs           ;
           cmp  al,'+'                  ; if '+' then go to added to path
           je   short addtopath         ;
           cmp  al,'v'                  ; if 'v' then just print current path
           je   short printpath         ;
           mov  si,offset ErrCmdL       ; else print error with command com
           call prtstring               ;
           jmp  short ExitIt            ;
           
printpath: mov  si,offset Buffer2       ; print PATH=........
           call prtstring               ;
           jmp  short ExitIt            ;

addtopath: push cs                      ; add to path
           push cs                      ; make es and ds = cs
           pop  es                      ;
           pop  ds                      ;
           mov  di,offset Buffer1       ; find end of env
           xor  ax,ax                   ;
           mov  cx,2000                 ; up to 2000 bytes
           repne                        ;
           scasw                        ;
           dec  di                      ; go back
           dec  di                      ;
           push si                      ; save pos of addstr
           mov  si,offset Buffer2       ; put PATH=..... at end of env
AdditL:    lodsb                        ;
           stosb                        ;
           or   al,al                   ;
           jnz  short AdditL            ;

           dec  di                      ; go back
           cmp  byte [di-1],';'         ; if PATH doesnt end with ';' then
           je   short IsSc              ;  add one
           mov  byte [di-1],';'         ;
IsSc:      pop  si                      ; restore pos of addstr
IsScL:     lodsb                        ; now add parameter string to PATH
           cmp  al,0Dh                  ;
           je   short AddItD            ;
           cmp  al,20h                  ;
           je   short AddItD            ;
           stosb                        ;
           jmp  short IsScL             ;
AddItD:    xor  ax,ax                   ; put the double null
           stosw                        ;

           mov  bx,di                   ; length of full environment needed
           sub  bx,offset Buffer1       ;
           mov  ESize,bx                ; save size
           shr  bx,4                    ; resize the environment block
           inc  bx                      ;  to allow our addition
           mov  es,cs:EnvStrSeg         ;
           mov  ah,4Ah                  ; resize mem block
           int  21h                     ;
           jnc  short ResizeOK          ; if error then say so and
           mov  si,offset RszMemErr     ;  leave env untouched
           call prtstring               ;
           jmp  short ExitIt            ;
ResizeOK:  push cs                      ; make sure ds=cs
           pop  ds                      ;

           mov  cx,ESize                ; cx = length of new env
           mov  ax,EnvStrSeg            ;
           mov  es,ax                   ; point es to mstr env block
           xor  di,di                   ; start at first
           mov  si,offset Buffer1       ;  our new env
           rep                          ; *change it*
           movsb                        ;

ExitIt:    mov  ah,4Ch                  ; exit to DOS
           int  21h                     ;

prtstring  proc near uses ax dx
           mov  ah,02                   ; DOS print char service
Ps1:       lodsb                        ; Get character & point to next one
           or   al,al                   ; End of string?
           jz   short ps2               ; Yes, so exit
           mov  dl,al                   ; 
           int  21h                     ; Output a character
           jmp  short Ps1               ; Keep doing it
Ps2:       ret
           endp


EnvStrSeg  dw  00h
ESize      dw  00h
ErrCmdL    db  'Error with command line.  Environment unchanged.',13,10,0
RszMemErr  db  'Error resizing Environment space.',13,10,0
PathStr    db  'PATH=',0
Buffer1    dup 2000,?     ; holds original environment
Buffer2    dup 2000,?     ; holds original path
.end

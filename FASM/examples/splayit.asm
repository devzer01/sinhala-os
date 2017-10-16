
; see http://www.cybertrails.com/~fys/splayit.htm for more information

Comment |=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  Compress and decompress files using the "splay tree" technique.
  Based on the article by Douglas W. Jones, "Application of Splay Trees to
  Data Compression" in Communications of the ACM, August 1988.

  Usage:
    SPLAYIT     (no filenames or parameters)

  SPLAYIT simply compresses a predefined file to a predefined file and then
  uncompresses back to another predefined file.  You can change the filenames
  in the code or add a command-line parser.
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=|

.model tiny
.186
.code
           org  100h

           mov  dx,offset CompStr       ; ** compress file
           mov  ah,09
           int  21h
           mov  dx,offset InName
           call OpenSFile
           mov  dx,offset OutName
           call OpenTFile
           call InitSplay
           call CompFile
           mov  bx,Handle1
           call CloseFile
           mov  bx,Handle2
           call CloseFile               ; ** done compress file


           mov  dx,offset ExpndStr      ; ** expand file
           mov  ah,09
           int  21h
           mov  dx,offset OutName
           call OpenSFile
           mov  dx,offset OrgName
           call OpenTFile
           call InitSplay
           call ExpndFile
           mov  bx,Handle1
           call CloseFile
           mov  bx,Handle2
           call CloseFile               ; ** done expand file

           mov  ax,4C00h
           int  21h

InitSplay  proc near

           ; nbasm does not support the local directive (yet)
           ;local I:word,K:word,J:byte

           jmp  short ISkipLocal
I          dw 00h
J          dw 00h
K          dw 00h
ISkipLocal:

           mov  cx,512
           xor  ax,ax
           mov  di,offset Up
           stosb
           inc  ax
UpLoop:    push ax
           dec  ax
           shr  ax,1
           stosb
           pop  ax
           inc  ax
           loop UpLoop

           mov  cx,256
           xor  ax,ax
           mov  si,offset Left
           mov  di,offset Right
LRLoop:    push ax
           inc  ax
           shl  ax,1
           stosw
           dec  ax
           mov  [si],ax
           inc  si
           inc  si
           pop  ax
           inc  ax
           loop LRLoop
           ret
InitSplay  endp

OpenSFile  proc near
           mov  ax,3D00h                ; open RO file
           int  21h                     ;
           mov  Handle1,ax
           ret
OpenSFile  endp

OpenTFile  proc near
           mov  ah,5Bh                  ; create new file
           xor  ch,ch                   ; normal file
           mov  cl,00100000b            ;
           int  21h                     ;
           mov  Handle2,ax
           ret
OpenTFile  endp

CloseFile  proc near
           mov  ah,3Eh                  ; Close File
           int  21h                     ;
           ret
CloseFile  endp

SPlay      proc near uses ax bx         ; uses passed ax
           
           ; nbasm does not support the local directive (yet)
           ;local A:word,B:word,C:byte,D:byte

           jmp  short SSkipLocal
A          dw 00h
B          dw 00h
C          dw 00h
D          dw 00h
SSkipLocal:

           add  ax,256                  ; A = Plain + 256;
           mov  A,ax                    ;
                                        ;
SLoop:     mov  bx,offset Up            ; C = Up[A]
           add  bx,ax                   ;
           mov  al,[bx]                 ;
           mov  C,al                    ;

           or   al,al                   ; if C <> Root then begin
           jnz  NotSElse
           jmp  SElse                   ;
NotSElse:  mov  bx,offset Up            ; D = Up[C]
           xor  ah,ah                   ;
           add  bx,ax                   ;
           mov  al,[bx]                 ;
           mov  D,al                    ;

           mov  bx,offset Left          ; B = Left[D]
           xor  ah,ah                   ;
           shl  ax,1                    ; double for word
           add  bx,ax                   ;
           mov  ax,[bx]                 ;
           mov  B,ax                    ;

           mov  al,C                    ; if C = B then begin
           xor  ah,ah                   ;
           cmp  ax,B                    ;
           jne  short NotCBE            ;
           mov  bx,offset Right         ; B = Right[D]
           mov  al,D                    ;
           xor  ah,ah                   ;
           shl  ax,1                    ; double for word
           add  bx,ax                   ;
           mov  ax,[bx]                 ;
           mov  B,ax                    ;
           mov  ax,A                    ; Right[D] = A
           mov  [bx],ax                 ;
           jmp  short NextIf            ;
NotCBE:    mov  al,D                    ; Left[D] = A
           xor  ah,ah                   ;
           shl  ax,1                    ; double for word
           mov  bx,offset Left          ;
           add  bx,ax                   ;
           mov  ax,A                    ;
           mov  [bx],ax                 ;
NextIf:    mov  al,C                    ; if A = Left[C] then
           xor  ah,ah                   ;
           shl  ax,1                    ; double for word
           mov  bx,offset Left          ;
           add  bx,ax                   ;
           mov  ax,[bx]                 ;
           cmp  ax,A                    ;
           jne  short RCEB              ;
           mov  ax,B                    ; Left[C] = B
           mov  [bx],ax                 ;
           jmp  short DoUps             ;
RCEB:      mov  al,C                    ; Right[C] = B
           xor  ah,ah                   ;
           shl  ax,1                    ; double for word
           mov  bx,offset Right         ;
           add  bx,ax                   ;
           mov  ax,B                    ;
           mov  [bx],ax                 ;
DoUps:     mov  ax,A                    ; Up[A] = D
           mov  bx,offset UP            ;
           add  bx,ax                   ;
           mov  al,D                    ;
           mov  [bx],al                 ;
           mov  ax,B                    ; Up[B] = C
           mov  bx,offset UP            ;
           add  bx,ax                   ;
           mov  al,C                    ;
           mov  [bx],al                 ;
           mov  al,D                    ; A = D
SElse:     xor  ah,ah                   ; A = C
           mov  A,ax                    ;
           or   ax,ax                   ; until A = Root   (see next comment)
           jz   short SPlayRet
           jmp  SLoop                   ;     (A=ax)(faster to test ax then A)
SPlayRet:  ret
SPlay      endp

FOBuffer   proc near uses ax bx cx dx

           cmp  word OutSize,00h        ; if OutSize > 0 then begin
           je   short DFO               ;
           mov  ah,40h                  ; write to file
           mov  bx,Handle2
           mov  cx,OutSize
           mov  dx,offset BufferOut
           int  21h
           mov  word OutSize,00h
DFO:       ret
FOBuffer   endp

WriteByte  proc near uses ax bx

           cmp  word OutSize,16384           ; if OutSize = 16384 then
           jb   short NoFLO
           call FOBuffer                ; FlushOutBuffer
NoFLO:     mov  bx,offset BufferOut     ; OutBuffer[OutSize] = OutByte
           add  bx,OutSize              ;
           mov  al,OutByte              ;
           mov  [bx],al                 ;
           inc  word OutSize            ; inc OutSize
           ret
WriteByte  endp

Compress   proc near uses cx si di      ; uses passed ax
           
           ; nbasm does not support the local directive (yet)
           ;local PSDAX:word,A1:word,U:byte

           jmp  short CSkipLocal
PSDAX      dw 00h
A1         dw 00h
U          dw 00h
CSkipLocal:

           mov  PSDAX,ax                ; A1 = Plain + 256
           add  ax,256                  ;
           mov  A1,ax                   ;

           push ax                      ; save A1
           mov  di,offset Stck          ; our stack
           mov  al,0FFh                 ; bottom of stack flag
           stosb                        ;
           pop  ax                      ; restore A1
                                        ; repeat
RCLoop:    mov  bx,offset Up            ; U = Up[A1]
           add  bx,ax                   ;
           mov  al,[bx]                 ;
           mov  U,al                    ;

           mov  bx,offset Right         ; Stack[Sp] = (Right[U] = A1)
           xor  ah,ah                   ;
           shl  ax,1                    ; double for word
           add  bx,ax                   ;
           mov  ax,[bx]                 ;
           cmp  ax,A1                   ;
           jne  short NotTrue           ;
           mov  al,01h                  ; TRUE
           jmp  short IsTrue            ;
NotTrue:   xor  al,al                   ; FALSE
IsTrue:    stosb                        ; save on stack
           mov  al,U                    ; A1 = U
           xor  ah,ah                   ;
           mov  A1,ax                   ;
           or   ax,ax                   ; until A1 = Root
           jnz  short RCLoop            ;

UnStack:   dec  di                      ; if Stack[Sp] then
           mov  al,[di]                 ;
           cmp  al,0FFh
           je   short DoneStck
           cmp  al,01h                  ; TRUE
           jne  short NotOrIt           ;
           mov  bl,OutByte              ;
           or   bl,OrIt                 ;
           mov  OutByte,bl              ;
NotOrIt:   cmp  byte OrIt,10000000b
           jne  short NotPos7
           call WriteByte               ;
           mov  byte OutByte,00h        ;
NotPos7:   rol  byte OrIt,1
           jmp  short UnStack           ; go to the next one
DoneStck:  mov  ax,PSDAX
           call SPlay                   ; Splay(Plain)
           ret
Compress   endp

CompFile   proc near
           mov  byte OrIt,00000001b
DoIt:      mov  ah,3Fh                  ; BlockRead.....
           mov  cx,16384                ;
           mov  bx,Handle1              ;
           mov  dx,offset BufferIn      ;
           int  21h                     ;
           or   ax,ax                   ; if none read finish up
           jz   short NoMore            ;
           push ax                      ;
           mov  cx,ax                   ; Index = 1 to InSize
           mov  si,offset BufferIn      ;
DoFor:     lodsb                        ;
           xor  ah,ah                   ;
           call Compress                ; Compress(InBuffer[index])
           loop DoFor                   ;
           pop  ax                      ;
           cmp  ax,16384                ; until InSize < 16384
           je   short DoIt              ;

NoMore:    mov  ax,256                  ; Compress(EofChar)
           call Compress                ;
           cmp  byte OrIt,00000001b          ; if BitPos <> 0 then
           je   short FlushIt
           call WriteByte
FlushIt:   call FOBuffer
           ret
CompFile   endp


;; **  now begins the expansion part  ** ;;
GetByte    proc near uses bx cx dx

           inc  word Index
           mov  ax,Index
           cmp  ax,InSize
           jb   short GetIt
           mov  ah,3Fh                  ; BlockRead.....
           mov  cx,16384                ;
           mov  bx,Handle1              ;
           mov  dx,offset BufferIn      ;
           int  21h                     ;
           mov  InSize,ax
           mov  word Index,00h
GetIt:     mov  bx,offset BufferIn
           add  bx,Index
           mov  al,[bx]
           ret
GetByte    endp

Expand     proc near uses bx 

           xor  dx,dx                   ; A = Root;
ExLoop:    cmp  byte OrIt,10000000b
           jne  short ENot07
           call GetByte                 ; returns in al
           mov  InByte,al
ENot07:    rol  byte OrIt,1
           mov  al,InByte
           and  al,OrIt
           or   al,al
           jnz  short NBitEZ
           mov  bx,offset Left          ; A = left[A]
           jmp  short DoUntA
NBitEZ:    mov  bx,offset Right         ; A = right[A]
DoUntA:    mov  ax,dx
           shl  ax,1
           add  bx,ax
           mov  ax,[bx]
           mov  dx,ax                   ; A
           cmp  ax,255                  ; until A > PredMax
           jbe  short ExLoop

           sub  ax,256
           call Splay
ReadErr:   ret
Expand     endp

ExpndFile  proc near uses ax bx cx dx

           mov  byte OrIt,10000000b
EoCLoop:   call Expand                  ; returns in ax
           cmp  ax,256
           je   short XEndIt
           mov  OutByte,al
           call WriteByte
           jmp  short EoCLoop
XEndIt:    call FOBuffer
           ret
ExpndFile  endp

Handle1    dw  00h        ; handle for source file
Handle2    dw  00h        ; handle for target file
OutByte    db  00h        ; the byte to write
InByte     db  00h        ; the byte read
OrIt       db  00h        ; AND var
InSize     dw  00h        ; len of data in source buffer (amount read in)
Index      dw  00h        ; location to put next byte in buffer
OutSize    dw  00h        ; len of data in target buffer (amount to write)
InName     db  'name.org',0   ; *** filename to use here (source)(orig. file)
OutName    db  'name.spl',0   ; ***  and here  (compressed file)(splayed)
OrgName    db  'name.uns',0   ; ***  and here  (uncompressed file)(unsplayed)
CompStr    db  13,10,'Compressing...$'
ExpndStr   db  13,10,'Expanding...$'

Stck       dup 256,?
Left       dup 512,?
Right      dup 512,?
Up         dup 514,?  ; (2 extra for safety)
BufferIn   dup 16384,?
BufferOut  dup 16390,?
.end


AniChar1  equ 0DDh
AniChar2  equ 0DEh
DefaultC  equ 0F0h

.model tiny
.code
.186
           org 100h
           
           mov  ax,1003h                ; disable blink mode
           xor  bl,bl                   ;
           int  10h                     ;

           mov  ah,01                   ; turn off cursor
           mov  ch,00100000b            ; bit number 5
           int  10h                     ;

           mov  ax,0B800h               ;
           mov  es,ax                   ;

           mov  si,offset Screen        ;
           xor  di,di                   ;
           mov  ah,DefaultC             ; default color (black on white)
DLoop:     lodsb                        ; get next char
           or   al,al                   ; if null then change color
           jnz  short Notcolr           ;
           lodsb                        ; get the color
           or   al,al                   ; if color = 0
           jz   short DDone             ;  then we are done w/display
           mov  ah,al
           jmp  short DLoop
Notcolr:   stosw
           jmp  short DLoop
DDone:

           mov  dh,AniChar1              ; dh=first,dl=next ani char
           mov  dl,AniChar2
AniLoop1:  mov  cx,2000
           xor  si,si
AniLoop2:  mov  ax,es:[si]
           inc  si
           inc  si
           cmp  al,dh
           jne  short NotAni
           mov  al,dl
           mov  es:[si-2],ax
NotAni:    loop AniLoop2
           mov  dh,dl
           inc  dx
           cmp  dl,AniChar2
           jbe  short Wait4It
           mov  dl,AniChar1

; on entry, cx = X/18.2 seconds to wait (cx = 18 = 1 second)
Wait4It:   push es
           mov  cx,9
           xor  ax,ax
           mov  es,ax
outerloop: mov  ax,es:[046Ch]
innerloop: cmp  ax,es:[046Ch]
           je   short innerloop
           loop outerloop
           pop  es
                                        
           mov  ah,01                   ; was there a key press
           int  16h                     ;
           jz   short AniLoop1          ;
           xor  ah,ah                   ; 'eat' the key
           int  16h                     ;

           mov  ax,1003h                ; enable blink mode
           mov  bl,01h                  ;
           int  10h                     ;

           mov  ax,0003                 ; clear the screen
           int  10h

           mov  ah,01                   ; turn on cursor
           mov  cx,0607h                ; cx = 0607h start = 6  finish = 7
           int  10h                     ;

           .exit

Screen     db  'ี-------------==============ออออออออออออออออออออออออ==============-------------ธ'
           db  'ณ                                                                              ณ'
           db  'ณ    ',0,0F5h,'NewBasic',0,DefaultC,'     version 00.97.86        ',AniChar1,'  Freeware Release  ',AniChar1,'               ณ'
           db  'ณ                                                                              ณ'
           db  'ณ    ',AniChar1,' Basic compiler and interpreter                                          ณ'
           db  'ณ    ',AniChar1,' Intel x86 assembler                                                     ณ'
           db  'ณ    ',AniChar1,' Linker  and                                                             ณ'
           db  'ณ    ',AniChar1,' Utilities                                                               ณ'
           db  'ณ                                                                              ณ'
           db  'ณ    ',0,0F5h,'Forever Young Software',0,DefaultC,'                                                    ณ'
           db  'ณ    Copyright 1984-2017                                                       ณ'
           db  'ณ    All Rights Reserved                                                       ณ'
           db  'ณ    31 May 2017                                                               ณ'
           db  'ณ                                                                              ณ'
           db  'ณ    As of this release, the compiler and interpreter are not included.        ณ'
           db  'ณ                                                                              ณ'
           db  'ณ    This release of ',0,0F1h,'NewBasic',0,DefaultC,' is freeware.  Do as you wish as long as you      ณ'
           db  'ณ     do not do it for profit.                                                 ณ'
           db  'ณ                                                                              ณ'
           db  'ณ    Please send a small note to me at ',0,0F9h,'fys@fysnet.net',0,DefaultC,' telling me if you        ณ'
           db  'ณ     use/like/dislike/hate this package  or  subscribe to the mailing list:   ณ'
           db  'ณ                     ',0,0F9h,'nbasm-subscribe@yahoogroups.com',0,DefaultC,'                          ณ'
           db  'ณ                                                                              ณ'
           db  'ณ                 ',0,0F9h,'http://www.fysnet.net/newbasic.htm',0,DefaultC,'                           ณ'
           db  'ิ-------------==============ออออออออออออออออออออออออ==============-------------พ'
           db  0,0

.end
   

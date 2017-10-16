comment %
 Demo program assembled with the NewBasic assembler
 (C)opyright  1984-2017  Forever Young Software
 Benjamin David Lunt
 31 May 2017

 This is a simple demo program showing how to display
  the given .ICO file.  Thie given .ICO file is actually
  a 32x32 .BMP file with the .ICO extention.  This way,
  you can use this file as an icon for NewBasic.  Simply
  modify the shortcut (.PIF file) to point to this ICO
  under the "program" tab / "change icon" button.
 %

.model tiny                             ; we want a .COM file
.code
           org  100h                    ; all .COM files should start at 100h

           mov  dx,offset File1         ; open NBASM.ICO
           mov  ax,3D00h                ;  for read only
           int  21h                     ;
           jc   short FError            ; if error, print error line and exit
                                        ; ax is returned as handle
           mov  bx,ax                   ;  we need this handle in bx
           mov  dx,offset Buffer        ; file name (NBASM.ICO)
           mov  cx,2102                 ; read 2102 bytes
           mov  ah,3Fh                  ;
           int  21h                     ;
           jc   short FError            ; if error, print error line and exit

           mov  ah,3Eh                  ; close file handle
           int  21h                     ; 

           mov  ax,0013h                ; change to screen mode 13h
           int  10h                     ;

           mov  si,offset Buffer        ; set up our palette with the one
           add  si,(14+40)              ;  included in the .ICO file
           mov  di,offset Palette       ;  palette starts at offset 54
           mov  cx,256                  ;  256 colors
PLoop1:    lodsb                        ; palette in BMP file is BGRz
           mov  [di+2],al               ;  while we need RGB for service
           lodsb                        ;   1012h of 10h
           mov  [di+1],al               ;
           movsb                        ;
           inc  di                      ;
           inc  di                      ;
           inc  si                      ;
           loop PLoop1                  ;
           mov  ax,1012h                ; use the BIOS to set the palette
           mov  dx,offset Palette       ;
           xor  bx,bx                   ;
           mov  cx,256                  ;
           int  10h                     ;

           mov  ax,0A000h               ; point es to video screen memory
           mov  es,ax                   ;

           mov  al,255                  ; 255 = white in the palette
           mov  cx,(320*200)            ;  set background as white
           xor  di,di                   ;
           rep                          ;
           stosb                        ;

           mov  di,(31*320+28944)       ; (semi)-center our image
           mov  si,offset Buffer        ;
           add  si,(14+40+1024)         ; skip header, color header, and 
           mov  cx,32                   ;  palette
Loop1:     push cx                      ; print image from bottom up
           mov  cx,32                   ;
           rep                          ;
           movsb                        ;
           pop  cx                      ;
           sub  di,(320+32)             ;
           loop Loop1                   ;

           xor  ah,ah                   ; wait for key press
           int  16h                     ;

           mov  ax,0003h                ; change screen back to mode 3 (text)
           int  10h                     ;

Done:      .exit                        ; exit to DOS

FError:    mov  dx,offset FErrorS       ; print error string
           mov  ah,09                   ;
           int  21h                     ;
           jmp  short Done              ;

FErrorS    db  13,10,'There was a file error...$'
File1      db  'nbasm.ico',0
Buffer     dup 2102,?
Palette    dup (256*3),?

.end

;.bmp files are:
;   14 byte header
;   40 byte bitmap header
; 4*N  color map (N = dword at offset 46 in bitmap header)
;    Color map is four bytes per color (BGRr)
; Bitmap data in rows starting from the bottom

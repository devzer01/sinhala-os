
; see http://www.cybertrails.com/~fys/cdinfo.htm for more information

;CDINFO will retrieve the VTOC of a CDROM which includes the ID, NAME, and DATE of the CDROM
;Version: 1.10b
;Author: Ben Lunt (Forever Young Software(R))
;Date: 22 Nov 1999
;Assembler: NBASM 00.23.xx
;
;CDINFO will not display the ID, Name, or Date of an Audio disc because they are 'formatted' ;differently.
;
;    a small util to check for the MSCDEX driver and get
;    the CDs ID, Name, and date given.
;   See end of this source for info on the VTOC
;

.model tiny
.code
start:     .start
           push ds
           pop  es

           mov  si,offset PCDinfo       ; print CDinfo string
           call prtstring               ;

           mov  ax,01100h               ; check to see if MSCDEX is installed
           int  2Fh                     ;
           cmp  al,0FFh                 ;
           je   short instld
           jmp  notinstld               ;
instld:    mov  si,offset PYes          ;
           call prtstring               ;

           mov  si,offset PVersion      ; get and print MSCDEX version
           call prtstring               ;
           mov  ax,150Ch                ;
           int  2Fh                     ;
           xor  ah,ah                   ;
           mov  al,bh                   ;
           call prtdec                  ;
           mov  ah,02                   ;
           mov  dl,46                   ;
           int  21h                     ;
           xor  ah,ah                   ;
           mov  al,bl                   ;
           call prtdec                  ;

           mov  ax,1500h                ; return how many cd's present
           xor  bx,bx                   ;  bx = # of cd's
           int  2Fh                     ;  cl = # of first cd drive
           mov  NumCDs,bx               ;     (0-a:,1-b:,etc.)
           add  cl,65                   ;
           xor  ch,ch                   ;
           mov  FirstCD,cx              ;
           mov  si,offset PTtlDrvs      ; print total drives string
           call prtstring               ;
           mov  ax,NumCDs               ;
           call prtdec                  ;
           mov  si,offset PFrstDrv      ; print first drive string
           call prtstring               ;
           mov  dx,FirstCD              ;
           mov  ah,02h                  ;
           int  21h                     ;

           mov  ax,1501h                ; get name of cdrom device driver
           mov  bx,offset DrvData       ;
           int  2Fh                     ;
           mov  di,offset DrvrName      ; buffer to store returned name
           push ds                      ; save ds (lds destroys it)
           mov  bx,offset DrvData       ; returned:
           inc  bx                      ;   offset   size  contents
           lds  si,[bx]                 ;     00h    byte  subunit number
           add  si,10                   ;     01h   dword  address of driver
           mov  cx,08h                  ;  not more than 8 bytes long
DNGet:     lodsb                        ;
           cmp  al,20h                  ; if < 8 bytes then truncated w/20h
           je   short DNGDn             ;
           stosb                        ;
           loop DNGet                   ;
DNGDn:     xor  al,al                   ; make sure it is an asciiz string
           stosb                        ;
           pop  ds                      ; restore ds (from lds)
           mov  si,offset PDrvrname     ; print Driver name status
           call prtstring               ;
           mov  si,offset Drvrname      ;
           call prtstring               ;

           mov  ax,1503h                ; get abstract file name
           mov  bx,offset AbsName
           mov  cx,03h
           int  2Fh
           mov  si,offset AbsNameS      ; and print it (if all spaces,
           call prtstring               ;   then none there)
           mov  si,offset AbsName       ;
           call prtstring               ;

           mov  ax,1504h                ; get Bibliography file name
           mov  bx,offset AbsName
           mov  cx,03h
           int  2Fh
           mov  si,offset BibNameS      ; and print it (if all spaces,
           call prtstring               ;   then none there)
           mov  si,offset AbsName       ;
           call prtstring               ;

           mov  ax,1502h                ; get Copyright file name
           mov  bx,offset AbsName
           mov  cx,03h
           int  2Fh
           mov  si,offset CpyNameS      ; and print it (if all spaces,
           call prtstring               ;   then none there)
           mov  si,offset AbsName       ;
           call prtstring               ;
           mov  si,offset CRLF          ;
           call prtstring               ; print CR LF

VTOCLoop:  mov  si,offset VTOCNumS
           call prtstring
           mov  ax,VTOCNum
           call prtdec
           mov  bx,offset VTOCBuff      ; get VTOC info
           mov  cx,FirstCD
           sub  cx,65
           mov  dx,VTOCNum
           inc  word VTOCNum
           mov  ax,1505h
           int  2Fh
           mov  VTOCStat,al
           jc   short VTOCErr
           cmp  al,0FFh
           je   short VTOCDoIt
           cmp  al,01h
           jna  short VToCDoIt
           jmp  Done
VTOCDoIt:  mov  al,VTOCBuff             ; print volume descriptor
           xor  ah,ah                   ;
           cmp  ax,0FFh                 ;
           jne  short Not0FF            ;
           mov  ax,04h                  ;
Not0FF:    cmp  ax,04h                  ;
           jbe  short DescOffOK         ;
           mov  ax,05h                  ;
DescOffOK: mov  bx,offset DescOff       ;
           shl  ax,01                   ;
           add  bx,ax                   ;
           mov  si,[bx]                 ;
           call prtstring               ;

           mov  si,offset CDIDS         ; print CD ID Name
           call prtstring               ;
           mov  si,offset CDID          ;
           mov  cx,05                   ;
           mov  ah,02                   ;
CDIDLoop:  lodsb                        ;
           mov  dl,al                   ;
           int  21h                     ;
           loop CDIDLoop                ;

           mov  si,offset CDNameS       ; print CD Name
           call prtstring               ;
           mov  si,offset CDName        ;
           mov  cx,32                   ;
           mov  ah,02                   ;
CDNmLoop:  lodsb                        ;
           mov  dl,al                   ;
           int  21h                     ;
           loop CDNmLoop                ;

           mov  si,offset CDDateS       ; print CD date
           call prtstring               ;
           mov  si,offset CDDate        ;
           mov  di,offset DateFrmt      ;
           movsw
           inc  di                      ;
           movsw
           inc  di                      ;
           movsw
           mov  si,offset DateFrmt      ;
           call prtstring               ;
           mov  si,offset CRLF          ; print CR LF
           call prtstring               ;
           cmp  byte VTOCStat,01h       ; check status
           jnbe short Done              ;   if 0 or 1 then loop
           jmp  VTOCLoop  
VTOCErr:   mov  si,offset VTOCErrS
           call prtstring
           jmp  short Done
notinstld: mov  si,offset PNo
           call prtstring
           call prtdec
Done:      mov  ah,4Ch                  ; exit to DOS
           int  21h                     ; 

PrtDec     proc near uses ax cx dx
           mov  cx,0FFFFh               ; Ending flag
           push cx
           mov  cx,10
PD1:       xor  dx,dx
           div  cx                      ; Divide by 10
           add  dl,30h                  ; Convert to ASCII
           push dx                      ; Store remainder
           or   ax,ax                   ; Are we done?
           jnz  PD1                     ; No, so continue
PD2:       pop  dx                      ; Character is now in DL
           cmp  dx,0FFFFh               ; Is it the ending flag?
           je   PD3                     ; Yes, so continue
           mov  ah,02h                  ; Output a character
           int  21h
           jmp  short PD2               ; Keep doing it
PD3:       ret
PrtDec     ENDP

Prtstring  proc near uses ax
Ps1:       mov  dl,[si]                 ; Get character
           inc  si                      ; Point to next one
           or   dl,dl                   ; End of string?
           jz   ps2                     ; Yes, so exit
           mov  ah,02h                  ; Output a character
           int  21h
           jmp  short Ps1               ; Keep doing it
Ps2:       ret
Prtstring  endp

NumCDs     dw  00h
FirstCD    dw  00h
DDfhandle  dw  00h
DrvrName   dup 9,0             ; 8 + asciiz
Command    db  00h

PCDinfo    db  13,10,' CD Information routine  v1.10b   Forever Young Software   1997'
           db  13,10,'----------------------------------------------------------------'
           db  13,10,'      MSCDEX installed?  ',0
PVersion   db  13,10,'        MSCDEX version:  ',0
PTtlDrvs   db  13,10,'          Total drives:  ',0
PFrstDrv   db  13,10,'    First Drive letter:  ',0
PDrvrname  db  13,10,'           Driver Name:  ',0
CDIDS      db  13,10,'            CD ID Name:  ',0
CDNameS    db  13,10,'               CD Name:  ',0
CDDateS    db  13,10,'               CD Date:  ',0
DateFrmt   db  '  /  /  ',0
CRLF       db  13,10,0

PNo        db  'No',0
PYes       db  'Yes',0
VTOCErrS   db  13,10,10,'Error getting VTOC info.  Could be an Audio disc',0
DrvData    dup 75             ; allows 15 drives
VTOCNumS   db  13,10,'Volume Table of Contents #',0
VTOCNum    dw  00h
VTOCStat   db  00h
VTOCBuff   db  00h
CDID       db  '     '
           dw  00h
           dup 32,0
CDName     dup 32,32
           dup 743,0
CDDate     dup 6,32
           dup 1231,0
AbsNameS   db  13,10,'    Abstract file name:  ',0
BibNameS   db  13,10,'Bibliography file name:  ',0
CpyNameS   db  13,10,'   Copyright file name:  ',0
AbsName    dup 39,0
DescOff    dw  offset DescBootS
           dw  offset DescPrmryS
           dw  offset DescSuppS
           dw  offset DescPartS
           dw  offset DescTermS
           dw  offset DescUnknS
DescBootS  db  13,10,'   *Boot Record',0
DescPrmryS db  13,10,'   *Primary volume descriptor',0
DescSuppS  db  13,10,'   *Supplementary volume descriptor',0
DescPartS  db  13,10,'   *Volume partition descriptor',0
DescTermS  db  13,10,'   *Volume descriptor set terminator',0
DescUnknS  db  13,10,'****Unknown descriptor',0

.end  start


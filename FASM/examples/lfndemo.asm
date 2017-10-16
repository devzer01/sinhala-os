
; see http://www.cybertrails.com/~fys/longinfo.htm for more information

;How to get the short file name of a long file name:
; you must have a file named "thisisalongfilename.extension"
; for this demo to work

.model tiny
.code
            push cs      ; make sure es=ds=cs
            push cs
            pop  ds
            pop  es
            mov  ax,714Eh
            mov  dx,offset FileName
            mov  di,offset FDRec
            xor  cx,cx      ; <--- Use this for file names
            ;mov  cx,0010h  ; <--- Use this for directory names
            mov  si,01h
            int  21h

            mov  si,offset ShortName
PLoop1:     lodsb
            or   al,al
            jz   short PLoopD
            int  29h
            jmp  short PLoop1
PLoopD:     ret

FileName    db  'thisisalongfilename.extension',0
FDRec       dw  00h,00h          ; File attribs
FileCrt     dw  00h,00h,00h,00h  ; File Creation date
LastAcc     dw  00h,00h,00h,00h  ; Last Access
LastMod     dw  00h,00h,00h,00h  ; Last Modified
FileSizeH   dw  00h,00h          ; File Size Hi
FileSizeL   dw  00h,00h          ; File Size Lo
            dup 8,?              ; reserved
FullName    dup 260,?            ; Full Long name
ShortName   dup 14,?             ; Short name

.end


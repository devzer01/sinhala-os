
; see http://www.cybertrails.com/~fys/detectsb.htm for more information

; DETECTSB is Sound Blaster or compatible sound card detection routine for DOS
; Version: 1.00
; Author: Ben Lunt (Forever Young Software(R))
; Refernce:  "Programming the SoundBlaster 16 DSP" by Ethan Brodsky
; Date: 18 Nov 2000
; Assembler: NBASM 00.24.xx

.model tiny
.code
           org  100h                    ; we need to make a .COM file
start:     mov  dx,offset DSound        ; text output
           mov  ah,09h
           int  21h

           call Reset_DSP               ; reset it
           jc   short RstError          ; if carry then error

           mov  dx,offset SoundBD       ; text output
           mov  ah,09h
           int  21h
           
           xor  al,al                   ; no error   ERRORLEVEL = 0
           jmp  short Done

RstError:  mov  dx,offset Sberror       ; display information if Soundblaster
           mov  ah,09h                  ;    is not on this baseaddress
           int  21h                     ; text output
           xor  al,al                   ; there was an error
           dec  al                      ;     ERRORLEVEL = 0FFh
Done:      .exit


Reset_DSP  proc near

;  Write a 1 to the reset port
           mov  dx,226h
           mov  al,1           ; al = 1
           out  dx,al          ; start DSP reset

;  Wait
           in   al,dx          ; wait for slow ports
           in   al,dx          ;  .
           in   al,dx          ;  .
           in   al,dx          ;  .

;  Write a 0 to the reset port
           xor  al,al
           out  dx,al          ; end DSP Reset

;  Poll the read-buffer status port until bit 7 is set
           mov  dx,22Eh        ; dx = DSP data available
           call WaitRead

;  Poll the read data port until you receive an AA
           mov  dx,22Ah        ; dx = DSP Read Data
           in   al,dx
           cmp  al,0AAh        ; if there is a SB then it returns 0AAh
           je   short SBthere
           stc
           ret
SBthere:   clc
           ret

; The DSP usually takes about 100 microseconds to initialized itself.  After
; this period of time, if the return value is not AA or there is no data at
; all, then the SB card may not be installed or an incorrect I/O address is
; being used.

Reset_DSP  ENDP

WaitRead   proc near
           push cx
           xor  cx,cx                   ; need that for slow SBs !
loopWaitR: dec  cx
           jz   short endloopR
           in   al,dx                   ; al = data available status
           or   al,al
           jns  short loopWaitR         ; jump if bit7=0 - no data available
endloopR:  pop  cx
           ret
WaitRead   endp

WaitWrite  proc near
           push cx
           xor  cx,cx                   ; need that for slow SBs !
loopWaitW: dec  cx
           jz   short endloopW
           in   al,dx                   ; al = write command status
           or   al,al
           js   short loopWaitW         ; jump if bit7=1 - writing not allowed
endloopW:  pop  cx
           ret
WaitWrite  endp

DSound     db 13,10,'Trying to detect a Sound Blaster by initalizing it...$'
Sberror    db 13,10,'A SoundBlaster is not detected on this system.$'
SoundBD    db 13,10,'SoundBlaster Detected.',13,10,'$'

.end  start

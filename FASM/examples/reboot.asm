
; see http://www.cybertrails.com/~fys/reboot.htm for more information

.model tiny
.code

; Check if SMARTDRV has been installed,
;  if so flush its buffers before we reboot.
; Interesting thought:  This checks for only SMARTDRV 4.00 or higher
;  What if a lower version of SMARTDRV is installed?
start:     push ds                      ; save ds and es per a bug
           push es                      ;  in some versions of SMARTDRV
           xor  cx,cx                   ; make sure cx != 0EDCh to avoid any
                                        ;  interaction with NWCACHE
           mov  ax,4A10h                ; installation check for SMARTDRV
           xor  bx,bx                   ;
           int  2Fh                     ; (destroys dx,bx,di,si,bp)
           pop  es                      ;
           pop  ds                      ;
           cmp  ax,0BABEh               ; Is it installed? 
           jne  short SkpFlush          ; No, so skip the flush

           mov  ax,4A10h                ; else, yes, so flush it
           mov  bx,0001h                ;  flush cache function call
           int  2Fh                     ;

; Clear system reset flag for cold boot
; For a warm reboot (no memory check), place a 1234h instead of the zero.
SkpFlush:  xor  ax,ax                   ; make 0000:0472h != 1234h
           mov  es,ax                   ;  remeber that:
           mov  es:[0472h],ax           ;     0000:0472h = 0040:0072h

; Check for 8088/8086, these use a different reboot procedure
           cli                          ; interrupts off
           pushf                        ; save original flags
           pushf                        ; put flags in ax
           pop  ax                      ;
           and  ax,0FFFh                ; clear bits 15 & 13
           or   ax,5000h                ;  and set bits 14 & 12
           push ax                      ; place them back in flags
           popf                         ;
           pushf                        ; now get flags back in ax
           pop  ax                      ;
           popf                         ; restore original flags
           sti                          ; allow interrupts again
           and  ax,0F000h               ; clear bits 11 - 0
           cmp  ax,0F000h               ; are bits 15 - 12 set?
           jz   short UseJmpCmnd        ;  nope

; AT+ CPU's should be in reset condition and key registers in known state
;  before entering reboot BIOS code
; Check the keyboard controller to see if ready to accept command
           xor  cx,cx                   ; time out counter
CntrlrL1:  in   al,64h                  ; get controller status
           jmp  short ($+2)             ;  short delay for 8086s and 80186s
           jmp  short ($+2)             ;
           test al,02h                  ; input buffer full?
           jz   short Ok2Send           ;
           loop CntrlrL1                ; if counter expires, fall thru
           jmp  short UseJmpCmnd        ; Failed (controller is busy)

; Hardware reset by setting system reset line low for ~6ms
Ok2Send:   mov  al,0FEh                 ; 'reset system' command
           out  64h,al                  ; send the command
           jmp  short ($+2)             ;  short delay for 8086s and 80186s
           jmp  short ($+2)             ;
           xor  cx,cx                   ; Timeout counter
Accepted:  in   al,64h                  ; get controller status
           jmp  short ($+2)             ;  short delay for 8086s and 80186s
           jmp  short ($+2)             ;
           test al,02                   ; input buffer full?
           jz   short UseJmpCmnd        ; jmp if accepted (i.e: stop trying)
           loop Accepted                ; try again, fall thru if fail
   ; if above was successful, then should be in reboot process now

; Use for XT a different reboot procedure, jump to reset start address
UseJmpCmnd:
           jmp  far 0FFF0h,0F000h       ; jmp to F000:FFF0h

.end


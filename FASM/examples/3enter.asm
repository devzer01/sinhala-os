comment /***************************************************************\
*                                                                      *
*   File name: 3Enter.asm                                              *
*                                                                      *
*   Assemble with NBASM 00.25.xx                                       *
*    NBASM 3enter                                                      *
*                                                                      *
*   This "stuffs" the keyboard with 3 enter keys so you don't have     *
*    to press the enter key three times using bochs, if your setup     *
*    is always the same.                                               *
*                                                                      *
*   Put in your .bat file before the BOCHS line.                       *
*    (Bochs is found at: bochs.sourceforge.net)                        *
*                                                                      *
*   Thanks to lewi9908 @ ghc.ctc.edu for writing up this snippet.      *
*                                                                      *
*   As always, use at your own risk.                                   *
*                                                                      *
\**********************************************************************/

.model tiny  ; make .com program
.code        ; start of code segment
.186         ; allow 80x186 instructions (though none are used below :)

           ; all .COM programs start at 100h, just passed the PSP
           org  100h

           ; this line frees the unused memory at the end of our
           ;  program and sets the stack to the end of this segment.
           ; not neccassarily needed in this program, but good
           ;  programming practice.
           .start

           ; stuff the keyboard service once
           mov ah,05h
           mov cx,1C0Dh 
           int 16h
           ;  ... twice
           mov ah,05h
           mov cx,1C0Dh 
           int 16h
           ;  ... three time a lady.
           mov ah,05h
           mov cx,1C0Dh 
           int 16h

           ; this simply mimics the exit to DOS service
           ;  it allows a value in the destination operand
           ;  for the RC = ERRORLEVEL.
           ;  If none, the current value AL is used.
           .exit

; done assembling this file.
.end


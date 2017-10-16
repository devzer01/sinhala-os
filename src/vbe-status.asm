;
; label = address (ptr)
; [label] = value (*ptr)
[BITS 16]
;[ORG  0x0009]
;    dw kbrd
; 40:0010 => 0410
; f000:fff0 -> f0fff0
;section .text start=0x7c00
[ORG  0x7c00]
boot:

    mov ah, 4fh
    mov al, 01h
    mov cx, 10Ch
    lea di, [retval]
    int 10h
    hlt

    times 510-($-$$) db 0
end:
    dw 0xAA55

retval: times 256 db 0
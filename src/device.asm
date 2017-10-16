use16

org 0x00

next:       dw  0xffff, 0xffff ;segment + offset

;The device attributes word contains the following fields:
;bit 15: set if character device, clear if block device
;bit 14: set if I/O control supported
;bit 13: for a block device, set if not IBM format; for a
;character device, set if output­until­busy call supported
;bit 12: reserved
;bit 11: set if open/close/removable media calls
;supported
;bits 5­10: reserved
;bit 4: set if CON driver
;bit 3: set if current clock device
;bit 2: set if current NUL device
;bit 1: set if current standard output device
;bit 0: set if current standard input device
;                      ;10000000
;                      ;1000000000000000
attributes: dw  0x0000 ;0x8000 (font.sys)
strategy:   dw  0x0000 ;0x1700 (font.sys)
routine:    dw  0x0000 ;0x2200 (routine)

logical:    db 'ATESTDEV'
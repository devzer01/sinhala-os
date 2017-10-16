
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; General equates
;
TRUE       equ   1
FALSE      equ   0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  DISK

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; BIOS Parameter Block structure
;
STRUCT_BPB struct
bytessec   word       ; number of bytes per sector
secclust   byte       ; sectors per cluster
resvsect   word       ; number of reserved sectors
fats       byte       ; number of FATs
rootentry  word       ; number of entries in root directory
totsects   word       ; totol number of sectors (below v4.0)
mediaid    byte       ; media ID byte
sectfat    word       ; number of sectors per FAT
secttrack  word       ; number of sectors per track
heads      word       ; number of heads
hidden     dword      ; number of hidden sectors
totsectsl  dword      ; totol number of sectors (v4.0+)
resv1      dup    6   ;
cylinders  word       ; number of cylinders
devicetype byte       ; device type
deviceattr word       ; device attributes
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Extended BIOS Parameter Block structure
;
STRUCT_EBPB struct
bytessec   word       ; number of bytes per sector
secclust   byte       ; sectors per cluster
resvsect   word       ; number of reserved sectors
fats       byte       ; number of FATs
rootentry  word       ; number of entries in root directory
totsects   word       ; totol number of sectors (below v4.0)
mediaid    byte       ; media ID byte
sectfat    word       ; number of sectors per FAT
secttrack  word       ; number of sectors per track
heads      word       ; number of heads
hidden     dword      ; number of hidden sectors
totsectsl  dword      ; totol number of sectors (v4.0+)
sectfatl   dword      ; sectors per FAT if 'sectfat' is 0000h
extflags   word       ; extended flags
fsversion  word       ; file system version
rootstart  dword      ; starting cluster number of root directory
fsinfosect word       ; file system information sector number
bootbak    word       ; sector number of backup boot sector
resv1      dup    6   ; reserved
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; diskette parameter table structure
;
STRUCT_DPT struct
specify1   byte       ; first specify byte
specify2   byte       ; second specify byte
delay      byte       ; delay until motor turned off (in clock ticks)
bytessect  byte       ; bytes per sector (00h = 128, 01h = 256, 02h = 512, 03h = 1024)
secttrack  byte       ; sectors per track
gaplen     byte       ; length of gap between sectors (2Ah for 5.25", 1Bh for 3.5")
datalen    byte       ; data length (ignored if bytes-per-sector field nonzero)
gaplenf    byte       ; gap length when formatting (50h for 5.25", 6Ch for 3.5")
filler     byte       ; format filler byte (default F6h)
settle     byte       ; head settle time in milliseconds
motorst    byte       ; motor start time in 1/8 seconds
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; diskette parameter table structure (IBM SurePath BIOS)
;
STRUCT_DPTS struct
specify1   byte       ; first specify byte
specify2   byte       ; second specify byte
delay      byte       ; delay until motor turned off (in clock ticks)
bytessect  byte       ; bytes per sector (00h = 128, 01h = 256, 02h = 512, 03h = 1024)
secttrack  byte       ; sectors per track
gaplen     byte       ; length of gap between sectors (2Ah for 5.25", 1Bh for 3.5")
datalen    byte       ; data length (ignored if bytes-per-sector field nonzero)
gaplenf    byte       ; gap length when formatting (50h for 5.25", 6Ch for 3.5")
filler     byte       ; format filler byte (default F6h)
settle     byte       ; head settle time in milliseconds
motorst    byte       ; motor start time in 1/8 seconds
maxtrack   byte       ; maximum track number
rate       byte       ; data transfer rate
type       byte       ; drive type in CMOS
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; hard disk master boot sector structure
;
STRUCT_MBS struct
bootstrap  dup  446   ; Master bootstrap loader code
partrec1   dup   16   ; partition record for partition 1
partrec2   dup   16   ; partition record for partition 2
partrec2   dup   16   ; partition record for partition 3
partrec3   dup   16   ; partition record for partition 4
signature  word       ; signature, AA55h indicates valid boot block
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; hard disk partition record structure
;
STRUCT_PART struct
bootid     byte       ; boot indicator (80h = active partition)
starthead  byte       ; partition start head
startsect  byte       ; partition start sector (bits 0-5)
starttrck  byte       ; partition start track (bits 8,9 in bits 6,7 of sector)
osid       byte       ; operating system indicator
endhead    byte       ; partition end head
endsect    byte       ; partition end sector (bits 0-5)
endtrack   byte       ; partition end track (bits 8,9 in bits 6,7 of sector)
preceding  dword      ; DWORD sectors preceding partition
partlen    dword      ; length of partition in sectors
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; disk info structure
;
STRUCT_DINF struct
infolevel  word       ; (info level)
serialnum  dword      ; disk serial number (binary)
volume     dup   11   ; volume label or "NO NAME    " if none present
filesys    dup    8   ; filesystem type
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  DOS

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; PSP structure
;
STRUCT_PSP struct
int20h     word
lastpara   word
resv1      byte
dosfuctn   dup    5
int22h     dword
int23h     dword
int24h     dword
paraseg    word
farray     dup   20
envseg     word
sssp       dword
farrsize   word
farrptr    dword
prevpsp    dword
resv2      dword
dosver     word
resv3      dup   14
int21retf  dup    3
resv4      dup    9
fcb1       dup   16
fcb2       dup   20
cmmndline  dup  128
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; command line structure
;
STRUCT_CMNDLINE struct
length     byte       ; length of comand line not counting trailing CR
line       dup  127   ; command line
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; EXEC parameter block structure (AL=00h,01h,04h)
;
STRUCT_EXEC0 struct
environ    word       ; segment of environment to copy for child process
pcomtail   dword      ; pointer to command tail to be copied into child's PSP
pfcb1      dword      ; pointer to first FCB to be copied into child's PSP
pfcb2      dword      ; pointer to second FCB to be copied into child's PSP
initsssp   dword      ; will hold subprogram's initial SS:SP on return
entrycsip  dword      ; will hold entry point (CS:IP) on return
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; EXEC parameter block structure (AL=03h)
;
STRUCT_EXEC3 struct
overlay    word       ; segment at which to load overlay
relofact   word       ; relocation factor to apply to overlay (.EXE format)
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; EXEC parameter block structure (FlashTek X-32)
;
STRUCT_EXECX struct
fpenviron  fword      ; 48-bit far pointer to environment string
fpcomtail  fword      ; 48-bit far pointer to command tail string
           ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Format of .EXE file header structure (DOS)
;
STRUCT_EXE struct
signature  word       ; .EXE signature, either "MZ" or "ZM" (5A4Dh or 4D5Ah)
lpagelen   word       ; number of bytes in last 512-byte page of executable
pages      word       ; total number of 512-byte pages in executable (includes any
relos      word       ; number of relocation entries
headsize   word       ; header size in paragraphs
minparas   word       ; minimum paragraphs of memory required to allocate
maxparas   word       ; maximum paragraphs to allocate in addition to executable's size
initss     word       ; initial SS relative to start of executable
initsp     word       ; initial SP
checksum   word       ; checksum (one's complement of sum of all words in executable)
initcsip   dword      ; initial CS:IP relative to start of executable
relooff    word       ; offset within header of relocation table
overlay    word       ; overlay number (normally 0000h = main program)
           ends

.end

; more to come.

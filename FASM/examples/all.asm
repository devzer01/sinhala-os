
;
;
; This is a list of all instructions that NBASM supports,
; This is used for an example on how to use each instruction
; I hope to add the x87 instruction next.
; This file is dated: 4 Sept 2005
;


immed8   equ  12h
immed16  equ  1234h
immed32  equ  12345678h

temp     equ  1000h


.model tiny
.code
.rmode
.586
.387

           org 100h

           ; make sure this stuff doesn't get executed
           mov  ax,4C01h
           int  21h


           aaa
           aad
           aad  10h
           aam
           aam  10h
           aas
           daa
           das

           adc  al,immed8
           adc  ax,immed16
           adc  eax,immed32
           adc  byte [bx],immed8
           adc  word [bx],immed16
           adc  dword [bx],immed32
           adc  byte [temp],immed8
           adc  word [temp],immed16
           adc  dword [temp],immed32
           adc  [temp],al
           adc  [temp],ax
           adc  [temp],eax
           adc  al,[temp]
           adc  ax,[temp]
           adc  eax,[temp]
           adc  bl,al
           adc  bx,ax
           adc  ebx,eax
           adc  eax,[eax+1234h]
           adc  eax,[eax+ebx+1234h]
           adc  eax,[eax*2+1234h]
           adc  eax,[eax+esi*2+1234h]

           add  al,immed8
           add  ax,immed16
           add  eax,immed32
           add  byte [bx],immed8
           add  word [bx],immed16
           add  dword [bx],immed32
           add  byte [temp],immed8
           add  word [temp],immed16
           add  dword [temp],immed32
           add  [temp],al
           add  [temp],ax
           add  [temp],eax
           add  al,[temp]
           add  ax,[temp]
           add  eax,[temp]
           add  bl,al
           add  bx,ax
           add  ebx,eax
           add  eax,[eax+1234h]
           add  eax,[eax+ebx+1234h]
           add  eax,[eax*2+1234h]
           add  eax,[eax+esi*2+1234h]

           and  al,immed8
           and  ax,immed16
           and  eax,immed32
           and  byte [bx],immed8
           and  word [bx],immed16
           and  dword [bx],immed32
           and  byte [temp],immed8
           and  word [temp],immed16
           and  dword [temp],immed32
           and  [temp],al
           and  [temp],ax
           and  [temp],eax
           and  al,[temp]
           and  ax,[temp]
           and  eax,[temp]
           and  bl,al
           and  bx,ax
           and  ebx,eax
           and  eax,[eax+1234h]
           and  eax,[eax+ebx+1234h]
           and  eax,[eax*2+1234h]
           and  eax,[eax+esi*2+1234h]

           arpl [bx],ax
           arpl [temp],ax

           bound ax,[bx]
           bound eax,[ebx]
           bound ax,[temp]
           bound eax,[temp]

           bsf  ax,bx
           bsf  ax,[bx]
           bsf  ax,[temp]
           bsf  eax,ebx
           bsf  eax,[ebx]
           bsf  eax,[temp]

           bsr  ax,bx
           bsr  ax,[bx]
           bsr  ax,[temp]
           bsr  eax,ebx
           bsr  eax,[ebx]
           bsr  eax,[temp]
  
           bswap eax
           bswap ebx

           bt   bx,ax
           bt   [bx],ax
           bt   [temp],ax
           bt   ebx,eax
           bt   [ebx],eax
           bt   [temp],eax
           bt   bx,immed8
           bt   word [bx],immed8
           bt   word [temp],immed8
           bt   ebx,immed8
           bt   word [ebx],immed8

           btc  bx,ax
           btc  [bx],ax
           btc  [temp],ax
           btc  ebx,eax
           btc  [ebx],eax
           btc  [temp],eax
           btc  bx,immed8
           btc  word [bx],immed8
           btc  word [temp],immed8
           btc  ebx,immed8
           btc  word [ebx],immed8

           btr  bx,ax
           btr  [bx],ax
           btr  [temp],ax
           btr  ebx,eax
           btr  [ebx],eax
           btr  [temp],eax
           btr  bx,immed8
           btr  word [bx],immed8
           btr  word [temp],immed8
           btr  ebx,immed8
           btr  word [ebx],immed8

           bts  bx,ax
           bts  [bx],ax
           bts  [temp],ax
           bts  ebx,eax
           bts  [ebx],eax
           bts  [temp],eax
           bts  bx,immed8
           bts  word [bx],immed8
           bts  word [temp],immed8
           bts  ebx,immed8
           bts  word [ebx],immed8

           call temp
           call bx
           call [bx]
           call ebx
           call [ebx]
           callf immed16,immed16

           cbw
           cwde
           clc
           cli
           clts
  
           cmova ax,bx
           cmova ax,[bx]
           cmova eax,ebx
           cmova eax,[ebx]
           cmovae ax,bx
           cmovae ax,[bx]
           cmovae eax,ebx
           cmovae eax,[ebx]
           cmovb ax,bx
           cmovb ax,[bx]
           cmovb eax,ebx
           cmovb eax,[ebx]
           cmovbe ax,bx
           cmovbe ax,[bx]
           cmovbe eax,ebx
           cmovbe eax,[ebx]
           cmovc ax,bx
           cmovc ax,[bx]
           cmovc eax,ebx
           cmovc eax,[ebx]
           cmove ax,bx
           cmove ax,[bx]
           cmove eax,ebx
           cmove eax,[ebx]
           cmovg ax,bx
           cmovg ax,[bx]
           cmovg eax,ebx
           cmovg eax,[ebx]
           cmovge ax,bx
           cmovge ax,[bx]
           cmovge eax,ebx
           cmovge eax,[ebx]
           cmovl ax,bx
           cmovl ax,[bx]
           cmovl eax,ebx
           cmovl eax,[ebx]
           cmovle ax,bx
           cmovle ax,[bx]
           cmovle eax,ebx
           cmovle eax,[ebx]
           cmovna ax,bx
           cmovna ax,[bx]
           cmovna eax,ebx
           cmovna eax,[ebx]
           cmovnae ax,bx
           cmovnae ax,[bx]
           cmovnae eax,ebx
           cmovnae eax,[ebx]
           cmovnb ax,bx
           cmovnb ax,[bx]
           cmovnb eax,ebx
           cmovnb eax,[ebx]
           cmovnbe ax,bx
           cmovnbe ax,[bx]
           cmovnbe eax,ebx
           cmovnbe eax,[ebx]
           cmovnc ax,bx
           cmovnc ax,[bx]
           cmovnc eax,ebx
           cmovnc eax,[ebx]
           cmovne ax,bx
           cmovne ax,[bx]
           cmovne eax,ebx
           cmovne eax,[ebx]
           cmovng ax,bx
           cmovng ax,[bx]
           cmovng eax,ebx
           cmovng eax,[ebx]
           cmovnge ax,bx
           cmovnge ax,[bx]
           cmovnge eax,ebx
           cmovnge eax,[ebx]
           cmovnl ax,bx
           cmovnl ax,[bx]
           cmovnl eax,ebx
           cmovnl eax,[ebx]
           cmovnle ax,bx
           cmovnle ax,[bx]
           cmovnle eax,ebx
           cmovnle eax,[ebx]
           cmovno ax,bx
           cmovno ax,[bx]
           cmovno eax,ebx
           cmovno eax,[ebx]
           cmovnp ax,bx
           cmovnp ax,[bx]
           cmovnp eax,ebx
           cmovnp eax,[ebx]
           cmovns ax,bx
           cmovns ax,[bx]
           cmovns eax,ebx
           cmovns eax,[ebx]
           cmovnz ax,bx
           cmovnz ax,[bx]
           cmovnz eax,ebx
           cmovnz eax,[ebx]
           cmovo  ax,bx
           cmovo  ax,[bx]
           cmovo  eax,ebx
           cmovo  eax,[ebx]
           cmovp  ax,bx
           cmovp  ax,[bx]
           cmovp  eax,ebx
           cmovp  eax,[ebx]
           cmovpe ax,bx
           cmovpe ax,[bx]
           cmovpe eax,ebx
           cmovpe eax,[ebx]
           cmovpo ax,bx
           cmovpo ax,[bx]
           cmovpo eax,ebx
           cmovpo eax,[ebx]
           cmovs  ax,bx
           cmovs  ax,[bx]
           cmovs  eax,ebx
           cmovs  eax,[ebx]
           cmovz  ax,bx
           cmovz  ax,[bx]
           cmovz  eax,ebx
           cmovz  eax,[ebx]

           cmp  al,immed8
           cmp  ax,immed16
           cmp  eax,immed32
           cmp  byte [bx],immed8
           cmp  word [bx],immed16
           cmp  dword [bx],immed32
           cmp  byte [temp],immed8
           cmp  word [temp],immed16
           cmp  dword [temp],immed32
           cmp  [temp],al
           cmp  [temp],ax
           cmp  [temp],eax
           cmp  al,[temp]
           cmp  ax,[temp]
           cmp  eax,[temp]
           cmp  bl,al
           cmp  bx,ax
           cmp  ebx,eax
           cmp  eax,[eax+1234h]
           cmp  eax,[eax+ebx+1234h]
           cmp  eax,[eax*2+1234h]
           cmp  eax,[eax+esi*2+1234h]

           cmpsb
           cmpsw
           cmpsd

           cmpxchg al,bl
           cmpxchg [bx],al
           cmpxchg ax,bx
           cmpxchg [bx],ax
           cmpxchg eax,ebx
           cmpxchg [ebx],eax

           cpuid
           cwd
           cdq

           dec bl
           dec byte [bx]
           dec byte [temp]
           dec bx
           dec word [bx]
           dec word [temp]
           dec ebx
           dec dword [ebx]
           dec dword [temp]
           dec dword [eax+1234h]
           dec dword [eax+ebx+1234h]
           dec dword [eax*2+1234h]
           dec dword [eax+esi*2+1234h]

           div bl
           div byte [bx]
           div byte [temp]
           div bx
           div word [bx]
           div word [temp]
           div ebx
           div dword [ebx]
           div dword [temp]

           emms

           enter immed16,0
           enter immed16,1
           enter immed16,immed8

           fxrstor [bx]
           fxrstor [ebx]
           fxrstor [temp]
           fxsave  [bx]
           fxsave  [ebx]
           fxsave  [temp]
           fxtract

           hlt

           idiv bl
           idiv byte [bx]
           idiv bx
           idiv word [bx]
           idiv ebx
           idiv dword [bx]

           imul bl
           imul byte [bx]
           imul byte [temp]
           imul bx
           imul word [bx]
           imul word [temp]
           imul ebx
           imul dword [bx]
           imul dword [temp]
           imul bx,ax
           imul ax,[bx]
           imul ax,[temp]
           imul ebx,eax
           imul eax,[ebx]
           imul eax,[temp]
           imul bx,ax,immed8
           imul ax,[bx],immed8
           imul ax,[temp],immed8
           imul ebx,eax,immed8
           imul eax,[ebx],immed8
           imul eax,[temp],immed8
           imul bx,immed8
           imul ebx,immed8
           imul bx,ax,immed16
           imul ax,[bx],immed16
           imul ax,[temp],immed16
           imul ebx,eax,immed32
           imul eax,[ebx],immed32
           imul eax,[temp],immed32
           imul bx,immed16
           imul ebx,immed32

           in   al,immed8
           in   ax,immed8
           in   eax,immed8
           in   al,dx
           in   ax,dx
           in   eax,dx
  
           inc bl
           inc byte [bx]
           inc byte [temp]
           inc bx
           inc word [bx]
           inc word [temp]
           inc ebx
           inc dword [ebx]
           inc dword [temp]
           inc dword [eax+1234h]
           inc dword [eax+ebx+1234h]
           inc dword [eax*2+1234h]
           inc dword [eax+esi*2+1234h]
  
           insb
           insw
           insd

           int 3
           int immed8
           into
  
           invd

           iret
           iretd

temp1:     ja   temp1
           jae  temp1
           jb   temp1
           jbe  temp1
           jc   temp1
           jcxz  temp1
	   .adsize
           jcxz temp1
           je   temp1
           jg   temp1
           jge  temp1
           jl   temp1
           jle  temp1
           jna  temp1
           jnae temp1
           jnb  temp1
           jnbe temp1
           jnc  temp1
           jne  temp1
           jng  temp1
           jnge temp1
           jnl  temp1
           jnle temp1
           jno  temp1
           jnp  temp1
           jns  temp1
           jnz  temp1
           jo   temp1
           js   temp1
           jz   temp1
           ja   temp
           jae  temp
           jb   temp
           jbe  temp
           jc   temp
tempp:     jcxz  tempp
	   .adsize
           jcxz tempp
           je   temp
           jg   temp
           jge  temp
           jl   temp
           jle  @f
           jna  temp
           jnae temp
           jnb  temp
@@:        jnbe temp
           jnc  temp
           jne  temp
           jng  temp
           jnge @b
           jnl  temp
           jnle temp
           jno  temp
           jnp  temp
           jns  temp
           jnz  temp
           jo   temp
           js   temp
           jz   temp

temp3:     jmp  short temp3
           jmp  temp
           jmp  bx
           jmp  near [bx]
           jmp  near [ebx]
           jmp  far  [bx]
           jmp  far  [ebx]

           lahf

           lar ax,bx
           lar ax,[bx]
           lar ebx,eax
           lar ebx,[eax]

           ldmxcsr [bx]
           ldmxcsr [ebx]
           ldmxcsr [temp]

           lds si,[temp]
           les si,[temp]
           lfs si,[temp]
           lgs si,[temp]
           lss si,[temp]

           lea ax,[bx]
           lea ax,[temp]
           lea eax,[ebx]
           lea eax,[temp]

           leave

           ; currently broken
           ;lfence

           lgdt far  [temp]
           lidt far  [temp]
           lldt word [temp]

           lmsw bx
           lmsw word [ebx]
           lmsw word [temp]

           lock

           lodsb
           lodsw
           lodsd

temp2:     loop   temp2
           loope  temp2
           loopz  temp2
           loopne temp2
           loopnz temp2

           lsl bx,ax
           lsl ax,[bx]
           lsl cx,[temp]
           lsl ebx,eax
           lsl eax,[ebx]
           lsl ecx,[temp]

           ltr bx
           ltr word [bx]
           ltr word [temp]

           ; currently broken
           ;mfence

           mov  al,immed8
           mov  ax,immed16
           mov  eax,immed32
           mov  byte [bx],immed8
           mov  word [bx],immed16
           mov  dword [bx],immed32
           mov  byte [temp],immed8
           mov  word [temp],immed16
           mov  dword [temp],immed32
           mov  [temp],al
           mov  [temp],ax
           mov  [temp],eax
           mov  al,[temp]
           mov  ax,[temp]
           mov  eax,[temp]
           mov  bl,al
           mov  bx,ax
           mov  ebx,eax
           mov  eax,[eax+1234h]
           mov  eax,[eax+ebx+1234h]
           mov  eax,[eax*2+1234h]
           mov  eax,[eax+esi*2+1234h]

           mov cr0,eax
           mov cr2,ebx
           mov cr3,ecx
           mov cr4,edx
           mov eax,cr0
           mov ebx,cr2
           mov ecx,cr3
           mov edx,cr4

           mov dr0,eax
           mov dr1,ebx
           mov dr2,ebx
           mov dr3,ecx
           mov dr4,edx
           mov dr5,edx
           mov dr6,edx
           mov dr7,edx
           mov eax,dr0
           mov eax,dr1
           mov ebx,dr2
           mov ecx,dr3
           mov edx,dr4
           mov edx,dr5
           mov edx,dr6
           mov edx,dr7

           movsb
           movsw
           movsd

           mul bl
           mul byte [bx]
           mul byte [temp]
           mul bx
           mul word [bx]
           mul word [temp]
           mul ebx
           mul dword [ebx]
           mul dword [temp]

           neg bl
           neg byte [bx]
           neg byte [temp]
           neg ax
           neg word [bx]
           neg word [temp]
           neg ecx
           neg dword [ebx]
           neg dword [temp]

           nop

           not bl
           not byte [bx]
           not byte [temp]
           not ax
           not word [bx]
           not word [temp]
           not ecx
           not dword [ebx]
           not dword [temp]

           or   al,immed8
           or   ax,immed16
           or   eax,immed32
           or   byte [bx],immed8
           or   word [bx],immed16
           or   dword [bx],immed32
           or   byte [temp],immed8
           or   word [temp],immed16
           or   dword [temp],immed32
           or   [temp],al
           or   [temp],ax
           or   [temp],eax
           or   al,[temp]
           or   ax,[temp]
           or   eax,[temp]
           or   bl,al
           or   bx,ax
           or   ebx,eax

           out  immed8,al
           out  immed8,ax
           out  immed8,eax
           out  dx,al
           out  dx,ax
           out  dx,eax

           outsb
           outsw
           outsd

           pop  bx
           pop  word [bx]
           pop  ebx
           pop  dword [ebx]
           pop  ds
           pop  es
           pop  fs
           pop  gs
           pop  ss
           popa
           popad
           popf
           popfd

           push bx
           push word [bx]
           push ebx
           push dword [ebx]
           push immed8
           push immed16
           push immed32
           push cs
           push ds
           push es
           push fs
           push gs
           push ss
           pusha
           pushad
           pushf
           pushfd

           rcl bl,1
           rcl byte [bx],1
           rcl byte [temp],1
           rcl bl,cl
           rcl byte [bx],cl
           rcl byte [temp],cl
           rcl bl,5
           rcl byte [bx],5
           rcl byte [temp],5
           rcl bx,1
           rcl word [bx],1
           rcl word [temp],1
           rcl bx,cl
           rcl word [bx],cl
           rcl word [temp],cl
           rcl bx,5
           rcl word [bx],5
           rcl word [temp],5
           rcl ebx,1
           rcl dword [ebx],1
           rcl dword [temp],1
           rcl ebx,cl
           rcl dword [ebx],cl
           rcl dword [temp],cl
           rcl ebx,5
           rcl dword [ebx],5
           rcl dword [temp],5
           rcr bl,1
           rcr byte [bx],1
           rcr byte [temp],1
           rcr bl,cl
           rcr byte [bx],cl
           rcr byte [temp],cl
           rcr bl,5
           rcr byte [bx],5
           rcr byte [temp],5
           rcr bx,1
           rcr word [bx],1
           rcr word [temp],1
           rcr bx,cl
           rcr word [bx],cl
           rcr word [temp],cl
           rcr bx,5
           rcr word [bx],5
           rcr word [temp],5
           rcr ebx,1
           rcr dword [ebx],1
           rcr dword [temp],1
           rcr ebx,cl
           rcr dword [ebx],cl
           rcr dword [temp],cl
           rcr ebx,5
           rcr dword [ebx],5
           rcr dword [temp],5
           rol bl,1
           rol byte [bx],1
           rol byte [temp],1
           rol bl,cl
           rol byte [bx],cl
           rol byte [temp],cl
           rol bl,5
           rol byte [bx],5
           rol byte [temp],5
           rol bx,1
           rol word [bx],1
           rol word [temp],1
           rol bx,cl
           rol word [bx],cl
           rol word [temp],cl
           rol bx,5
           rol word [bx],5
           rol word [temp],5
           rol ebx,1
           rol dword [ebx],1
           rol dword [temp],1
           rol ebx,cl
           rol dword [ebx],cl
           rol dword [temp],cl
           rol ebx,5
           rol dword [ebx],5
           rol dword [temp],5
           ror bl,1
           ror byte [bx],1
           ror byte [temp],1
           ror bl,cl
           ror byte [bx],cl
           ror byte [temp],cl
           ror bl,5
           ror byte [bx],5
           ror byte [temp],5
           ror bx,1
           ror word [bx],1
           ror word [temp],1
           ror bx,cl
           ror word [bx],cl
           ror word [temp],cl
           ror bx,5
           ror word [bx],5
           ror word [temp],5
           ror ebx,1
           ror dword [ebx],1
           ror dword [temp],1
           ror ebx,cl
           ror dword [ebx],cl
           ror dword [temp],cl
           ror ebx,5
           ror dword [ebx],5
           ror dword [temp],5

           rdmsr
           rdpmc
           rdtsc

           ret
           retf
           ret  4
           retf 4

           rsm

           sal bl,1
           sal byte [bx],1
           sal byte [temp],1
           sal bl,cl
           sal byte [bx],cl
           sal byte [temp],cl
           sal bl,5
           sal byte [bx],5
           sal byte [temp],5
           sal bx,1
           sal word [bx],1
           sal word [temp],1
           sal bx,cl
           sal word [bx],cl
           sal word [temp],cl
           sal bx,5
           sal word [bx],5
           sal word [temp],5
           sal ebx,1
           sal dword [ebx],1
           sal dword [temp],1
           sal ebx,cl
           sal dword [ebx],cl
           sal dword [temp],cl
           sal ebx,5
           sal dword [ebx],5
           sal dword [temp],5
           sar bl,1
           sar byte [bx],1
           sar byte [temp],1
           sar bl,cl
           sar byte [bx],cl
           sar byte [temp],cl
           sar bl,5
           sar byte [bx],5
           sar byte [temp],5
           sar bx,1
           sar word [bx],1
           sar word [temp],1
           sar bx,cl
           sar word [bx],cl
           sar word [temp],cl
           sar bx,5
           sar word [bx],5
           sar word [temp],5
           sar ebx,1
           sar dword [ebx],1
           sar dword [temp],1
           sar ebx,cl
           sar dword [ebx],cl
           sar dword [temp],cl
           sar ebx,5
           sar dword [ebx],5
           sar dword [temp],5
           shl bl,1
           shl byte [bx],1
           shl byte [temp],1
           shl bl,cl
           shl byte [bx],cl
           shl byte [temp],cl
           shl bl,5
           shl byte [bx],5
           shl byte [temp],5
           shl bx,1
           shl word [bx],1
           shl word [temp],1
           shl bx,cl
           shl word [bx],cl
           shl word [temp],cl
           shl bx,5
           shl word [bx],5
           shl word [temp],5
           shl ebx,1
           shl dword [ebx],1
           shl dword [temp],1
           shl ebx,cl
           shl dword [ebx],cl
           shl dword [temp],cl
           shl ebx,5
           shl dword [ebx],5
           shl dword [temp],5
           shr bl,1
           shr byte [bx],1
           shr byte [temp],1
           shr bl,cl
           shr byte [bx],cl
           shr byte [temp],cl
           shr bl,5
           shr byte [bx],5
           shr byte [temp],5
           shr bx,1
           shr word [bx],1
           shr word [temp],1
           shr bx,cl
           shr word [bx],cl
           shr word [temp],cl
           shr bx,5
           shr word [bx],5
           shr word [temp],5
           shr ebx,1
           shr dword [ebx],1
           shr dword [temp],1
           shr ebx,cl
           shr dword [ebx],cl
           shr dword [temp],cl
           shr ebx,5
           shr dword [ebx],5
           shr dword [temp],5

           sbb  al,immed8
           sbb  ax,immed16
           sbb  eax,immed32
           sbb  byte [bx],immed8
           sbb  word [bx],immed16
           sbb  dword [bx],immed32
           sbb  byte [temp],immed8
           sbb  word [temp],immed16
           sbb  dword [temp],immed32
           sbb  [temp],al
           sbb  [temp],ax
           sbb  [temp],eax
           sbb  al,[temp]
           sbb  ax,[temp]
           sbb  eax,[temp]
           sbb  bl,al
           sbb  bx,ax
           sbb  ebx,eax

           scasb
           scasw
           scasd

           seta  bl
           seta  [bx]
           setae bl
           setae [bx]
           setb  bl
           setb  [bx]
           setbe bl
           setbe [bx]
           setc  bl
           setc  [bx]
           sete  bl
           sete  [bx]
           setg  bl
           setg  [bx]
           setge bl
           setge [bx]
           setl  bl
           setl  [bx]
           setle bl
           setle [bx]
           setna  bl
           setna  [bx]
           setnae bl
           setnae [bx]
           setnb  bl
           setnb  [bx]
           setnbe bl
           setnbe [bx]
           setnc  bl
           setnc  [bx]
           setne  bl
           setne  [bx]
           setng  bl
           setng  [bx]
           setnge bl
           setnge [bx]
           setnl  bl
           setnl  [bx]
           setnle bl
           setnle [bx]
           setno bl
           setno [bx]
           setnp bl
           setnp [bx]
           setns bl
           setns [bx]
           setnz bl
           setnz [bx]
           seto  bl
           seto  [bx]
           setp  bl
           setp  [bx]
           setpe bl
           setpe [bx]
           setpo bl
           setpo [bx]
           sets  bl
           sets  [bx]
           setz  bl
           setz  [bx]

           ; currently broken
           ;sfence

           sgdt far  [temp]
           sidt far  [temp]
           sldt word [temp]

           shld  bx,ax,immed8
           shld  [bx],ax,immed8
           shld  [temp],ax,immed8
           shld  bx,ax,cl
           shld  [bx],ax,cl
           shld  [temp],ax,cl
           shld  ebx,eax,immed8
           shld  [ebx],eax,immed8
           shld  [temp],eax,immed8
           shld  ebx,eax,cl
           shld  [ebx],eax,cl
           shld  [temp],eax,cl
           shrd  bx,ax,immed8
           shrd  [bx],ax,immed8
           shrd  [temp],ax,immed8
           shrd  bx,ax,cl
           shrd  [bx],ax,cl
           shrd  [temp],ax,cl
           shrd  ebx,eax,immed8
           shrd  [ebx],eax,immed8
           shrd  [temp],eax,immed8
           shrd  ebx,eax,cl
           shrd  [ebx],eax,cl
           shrd  [temp],eax,cl

           smsw  ax
           smsw  word [bx]
           smsw  eax
           smsw  word [ebx]

           stmxcsr [bx]
           stmxcsr [ebx]
           stmxcsr [temp]

           stc
           sti

           stosb
           stosw
           stosd

           sub  al,immed8
           sub  ax,immed16
           sub  eax,immed32
           sub  byte [bx],immed8
           sub  word [bx],immed16
           sub  dword [bx],immed32
           sub  byte [temp],immed8
           sub  word [temp],immed16
           sub  dword [temp],immed32
           sub  [temp],al
           sub  [temp],ax
           sub  [temp],eax
           sub  al,[temp]
           sub  ax,[temp]
           sub  eax,[temp]
           sub  bl,al
           sub  bx,ax
           sub  ebx,eax

           sysenter
           sysexit

           test al,immed8
           test ax,immed16
           test eax,immed32
           test byte [bx],immed8
           test word [bx],immed16
           test dword [bx],immed32
           test byte [temp],immed8
           test word [temp],immed16
           test dword [temp],immed32
           test [temp],al
           test [temp],ax
           test [temp],eax
           test al,[temp]
           test ax,[temp]
           test eax,[temp]
           test bl,al
           test bx,ax
           test ebx,eax

           verr bx
           verr word [bx]
           verr word [temp]
           verr word [ebx]
           verw bx
           verw word [bx]
           verw word [temp]
           verw word [ebx]

           wait
           fwait

           wbinvd
           wrmsr

           xadd bl,cl
           xadd [bx],cl
           xadd [temp],cl
           xadd bx,cx
           xadd [bx],cx
           xadd [temp],cx
           xadd ebx,ecx
           xadd [ebx],ecx
           xadd [temp],ecx

           xchg ax,bx
           xchg bx,ax
           xchg eax,ebx
           xchg ebx,eax
           xchg bl,cl
           xchg [bx],cl
           xchg [temp],cl
           xchg bx,cx
           xchg [bx],cx
           xchg [temp],cx
           xchg ebx,ecx
           xchg [ebx],ecx
           xchg [temp],ecx

           xlatb
           xlat  cs:[bx]

           xor  al,immed8
           xor  ax,immed16
           xor  eax,immed32
           xor  byte [bx],immed8
           xor  word [bx],immed16
           xor  dword [bx],immed32
           xor  byte [temp],immed8
           xor  word [temp],immed16
           xor  dword [temp],immed32
           xor  [temp],al
           xor  [temp],ax
           xor  [temp],eax
           xor  al,[temp]
           xor  ax,[temp]
           xor  eax,[temp]
           xor  bl,al
           xor  bx,ax
           xor  ebx,eax

.end



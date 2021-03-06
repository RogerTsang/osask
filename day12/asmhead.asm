; haribote-os
; TAB=4

BOTPAK  EQU 0x00280000
DSKCAC  EQU 0x00100000
DSKCAC0 EQU 0x00008000

; Boot info
CYLS    EQU 0x0ff0 ; cylinder number
LEDS    EQU 0x0ff1 ; keyboard leds
VMODE   EQU 0x0ff2 ; video mode
SCRNX   EQU 0x0ff4 ; screen rander x axis
SCRNY   EQU 0x0ff6 ; screen rander y axis
VRAM    EQU 0x0ff8 ; video ram

    ; program entry in memory location
    ORG 0xc200

    ; video init
    MOV AH, 0x00 ;set video mode
    MOV AL, 0x13 ;VGA 320x200x8
    INT 0x10

    MOV BYTE [VMODE], 8
    MOV WORD [SCRNX], 320
    MOV WORD [SCRNY], 200
    MOV DWORD [VRAM], 0x000a0000

    ; keyboard init
    MOV AH, 0x02
    INT 0x16
    MOV [LEDS], AL

    ; Disable CPU interrupt
    MOV AL, 0xff
    OUT 0x21, AL ; io_out(PIC0_IMR, 0xff)
    NOP

    OUT 0xa1, AL ; io_out(PIC1_IMR, 0xff)
    CLI          ; Disable CPU interrupt

    ; Setup A20GATE to enable access > 1MB mem
    CALL waitkbdout
    MOV AL, 0xd1 
    OUT 0x64, AL ; io_out(PORT_KEYCMD, KEYCMD_WRITE_OUTPORT);
    CALL waitkbdout
    MOV AL, 0xdf ; 0xdf is a port attached to keyboard controller
    OUT 0x60, AL ; io_out(PORT_KEYDAT, KBC_OUTPORT_A20G_ENABLE);
    CALL waitkbdout

[INSTRSET "i486p"] ; Enable 'LGDT' 'EAX' 'CR0' keywords
    LGDT [GDTR0]   ; temporary GDT
    MOV EAX, CR0
    AND EAX, 0x7fffffff ; bit31 => 0 (disable paging)
    OR EAX, 0x00000001  ; bit0  => 1 (protection mode => virtual address)
    ; protect: prevent application from overriding segment permission

    MOV CR0, EAX   ; write to status register Control Register 0
    JMP pipelineflush

pipelineflush:
    MOV AX, 1*8
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX

    MOV ESI, bootpack
    MOV EDI, BOTPAK
    ; code segment register is changed at the end
    ; to prevent code segment corruption
    MOV ECX, 512*1024/4
    CALL memcpy

; transfer disk data to memory
    MOV ESI, 0x7c00
    MOV EDI, DSKCAC
    MOV ECX, 512/4
    CALL memcpy

; all the remaining blocks
    MOV ESI, DSKCAC0+512 ; source
    MOV EDI, DSKCAC+512  ; destination
    MOV ECX, 0
    MOV CL, BYTE [CYLS]
    IMUL ECX, 512*18*2/4 ; cylinder unit -> byte unit / 4

    SUB ECX, 512/4 ; don't copy IPL
    CALL memcpy; void memcpy(source ESI, destination EDI, size ECX);

;start BOOTPACK
    MOV EBX, BOTPAK
    MOV ECX, [EBX+16]
    ADD ECX, 3
    SHR ECX, 2
    JZ skip

    MOV ESI, [EBX+20]
    ADD ESI, EBX
    MOV EDI, [EBX+12]
    CALL memcpy

skip:
    MOV ESP, [EBX+12]
    JMP DWORD 2*8:0X0000001b

waitkbdout:
    IN AL, 0x64
    AND AL, 0x02
    IN AL, 0x60 ; read from keyboard and mouse
    JNZ waitkbdout ; if (PORT 0x64 AND 0x02) != 0 (NOTREADY bit is set)
    RET

memcpy:
    MOV EAX, [ESI] ;copy source [ESI] to temp EAX
    ADD ESI, 4 ;increment source addr
    MOV [EDI], EAX ;paste dest [EDI]
    ADD EDI, 4 ;increment dest addr
    SUB ECX, 1 ;decrease size
    JNZ memcpy 
    RET ;return when finish
    alignb 16

GDT0:
    RESB 8
    DW 0xffff, 0x0000, 0x9200, 0x00cf ; rw-
    DW 0xffff, 0x0000, 0x9a28, 0x0047 ; --x
    DW 0

GDTR0:
    DW 8*3-1
    DD GDT0

    alignb 16

bootpack:

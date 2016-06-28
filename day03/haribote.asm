; haribote-os
; TAB=4

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

fin:

    HLT
    JMP fin

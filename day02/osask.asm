; hello-os
; TAB=4

ORG 0x7c00
JMP entry

DB 0x90
DB "HELLOIPL" ; name for starting sector
DW 512 ; sector size
DB 1 ; cluster size

DW 1 ; FAT starting position
DB 2 ; FAT number in total
DW 224 ; Root Folder Size
DW 2880 ; Disk Size (in sector)
DB 0xf0 ; Disk Property
DW 9 ; FAT length (in sector)
DW 18 ; Number of sector in track
DW 2 ; Disk Scaner number in total
DD 0 ;
DD 2880;

DB 0x00, 0x00, 0x29; Magic Number
DD 0xffffffff
DB "HELLO-OS   " ; Name of the disk
DB "FAT12   "    ; Name of the disk format
RESB 18

; Main Program
entry:
    MOV AX, 0 ; init AX reg
    ; stack segment, data segment, extra segment
    MOV SS, AX
    MOV DS, AX
    MOV ES, AX
    MOV SP, 0X7c00 ; init stack pointer

    MOV SI, msg ; source index

putchar:
    MOV AL, [SI] ; fetch character ASCII code to AL reg
    ADD SI, 1 ; increment SI to print msg byte by byte

    CMP AL, 0
    JE  finish
    ; BIOS interrupt setting
    MOV AH, 0x0E ; Write Char in TTY mode
   ;MOV AL, AL ; Character is set already
    MOV BH, 0 
    MOV BL, 15; Colour
    INT 0x10 ; Call BIOS video Intertupt
    JMP putchar

; End
finish:
    HLT
    JMP finish

; Display
msg:
    DB 0x0a, 0x0a ; 2 carriage returns
    DB "hello, world"
    DB 0x0a
    DB 0x00
    
; RESB 0x1fe-$
; $ stands for current pos
;$$ stands for begin pos
; fill until 0x1fe
TIMES 0x1fe - ($-$$) DB 0
DB 0x55, 0xaa

DB 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB 4600
DB 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB 1469432

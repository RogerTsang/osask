; hello-os
; TAB=4
CYLS EQU 10

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
    ; errmsg: floppy
    MOV SP, 0X7c00 ; init stack pointer

    MOV AX, 0 ; init AX reg
    ; stack segment, data segment, extra segment
    MOV SS, AX
    MOV DS, AX
    ; set extra segment to 0x0820
    MOV AX, 0x0820;
    MOV ES, AX
    
    ; BIOS low level disk service
    ; Positioning
    MOV CH, 0 ; cylinder 
    MOV CL, 2 ; sector
    MOV DH, 0 ; head
    MOV DL, 0x00 ; disk drive: A

readloop:
    MOV SI, 0 ; init record counter
retry:
    MOV AH, 0x02 ; read sector command
    MOV AL, 1 ; read 1 sector (512 byte)
    ; Invoke
    INT 0x13
    JNC readnext; jump not carry
    ; failure
    ADD SI, 1
    CMP SI, 5
    JAE error; report error after 5 retries
    MOV AH, 0x00; reset disk drive command
    MOV DL, 0x00; reset disk drive A
    INT 0x13
    JMP retry

readnext:
    ; reading one by one can jump to next track
    MOV AX, ES
    ADD AX, 0x20 ; add 0x200 (512 byte) offset to memory buffer
    MOV ES, AX

    ADD CL, 1 ; next sector
    CMP CL, 18 ; one cylinder contains 18 sectors
    JBE readloop ; jump if below or equal
    ; switch head
    MOV CL, 1
    ADD DH, 1 ; head
    CMP DH, 2
    JB readloop
    ; switch cylinder
    MOV CL, 1
    MOV DH, 0
    ADD CH, 1
    CMP CH, CYLS ; final cylinder
    JB readloop
    ; indicate success
    JMP success

error:
    MOV SI, errmsg
    JMP putchar

success:
    MOV [0x0ff0], CH ; Copy Constant CYLS to memory 0x0ff0
    JMP 0xc200

putchar:
    MOV AL, [SI] ; fetch character ASCII code to AL reg
    ADD SI, 1 ; increment SI to print msg byte by byte

    CMP AL, 0
    JE  finish
    ; BIOS interrupt setting
    MOV AH, 0x0E ; Write Char in TTY mode command
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
errmsg:
    DB 0x0a, 0x0a ; 2 carriage returns
    DB "Err: Cannot Load Floppy Disk"
    DB 0x0a
    DB 0x00
    
; RESB 0x1fe-$
; $ stands for current pos
;$$ stands for begin pos
; fill until 0x1fe
TIMES 0x1fe - ($-$$) DB 0
DB 0x55, 0xaa

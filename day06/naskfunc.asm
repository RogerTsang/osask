; naskfunc
; TAB=4

[FORMAT "WCOFF"]
[INSTRSET "i486p"] ;processor family hint for nask
[BITS 32]

[FILE "naskfunc.asm"]
    GLOBAL _write_mem8
    GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt
    GLOBAL _io_in8, _io_in16, _io_in32
    GLOBAL _io_out8, _io_out16, _io_out32
    GLOBAL _io_load_eflags, _io_store_eflags
    GLOBAL _load_gdtr, _load_idtr
    ; Interrupt Handler Function
    GLOBAL _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
    ; External C function for OS dev
    EXTERN _inthandler21, _inthandler27, _inthandler2c

[SECTION .text]

_write_mem8: ;void write_mem8(int addr, int data)
    MOV ECX, [ESP+4] ;move first arg (int addr) to ECX reg (32-bit)
    MOV AL, [ESP+8] ;move second arg (int data) to AL (8-bit)
    MOV [ECX], AL
    RET

_io_hlt:
    HLT
    RET

_io_cli:
    CLI
    RET

_io_sti:
    STI
    RET

_io_stihlt:
    STI
    HLT
    RET

; EAX acts as return register
; [ESP + argno / 4] acts as arguments

_io_in8: ;int io_in8(int port); 
    MOV EDX, [ESP+4] ; port move to EDX
    MOV EAX, 0 ; clear result buffer
    IN  AL, DX ; IN retvalue, port
    RET

_io_in16: ;int io_in16(int port);
    MOV EDX, [ESP+4] ; port move to EDX
    MOV EAX, 0 ; clear result buffer
    IN  AX, DX ; IN retvalue, port
    RET

_io_in32: ;int io_in32(int port);
    MOV EDX, [ESP+4] ; port move to EDX
    MOV EAX, 0 ; clear result buffer
    IN  EAX, DX ; IN retvalue, port
    RET
    
_io_out8: ;void io_out8(int port, int data);
    MOV EDX, [ESP+4] ; port move to EDX
    MOV AL, [ESP+8] ; move 8bit data to AL
    OUT DX, AL ; OUT port, outvalue
    RET
    
_io_out16: ;void io_out16(int port, int data);
    MOV EDX, [ESP+4] ; port move to EDX
    MOV AX, [ESP+8] ; move 16bit data to AL
    OUT DX, AX ; OUT port, outvalue
    RET

_io_out32: ;void io_out32(int port, int data);
    MOV EDX, [ESP+4] ; port move to EDX
    MOV AL, [ESP+8] ; move 32bit data to AL
    OUT DX, EAX ; OUT port, outvalue
    RET

_io_load_eflags: ;int io_load_eflags(void);
    PUSHFD ; Push eflags to stack
    POP EAX; pop eflags to EAX, EAX acts as ret reg
    RET

_io_store_eflags: ;void io_store_eflags(int eflags);
    MOV EDX, [ESP+4]
    PUSH EDX
    POPFD ; restore eflags register
    RET

; Global segmentation descriptor table
_load_gdtr: ;void load_gdtr(int limit, int addr);
    MOV AX, [ESP+4] ;int limit
    MOV [ESP+6], AX ; 6-9 limit 10-11 addr(high)
    LGDT [ESP+6]
    RET

; Interrupt descriptor table
_load_idtr:
    MOV AX, [ESP+4]
    MOV [ESP+6], AX
    LIDT [ESP+6]
    RET

; Interrupt Handlers
_asm_inthandler21:
    PUSH ES
    PUSH DS
    PUSHAD
    MOV EAX, ESP
    PUSH EAX
    ; Backup-stack: ES DS AD ESP
    MOV AX, SS
    MOV DS, AX
    MOV ES, AX
    CALL _inthandler21
    POP EAX
    POPAD
    POP DS
    POP ES
    IRETD

_asm_inthandler27:
    PUSH ES
    PUSH DS
    PUSHAD
    MOV EAX, ESP
    PUSH EAX
    ; Backup-stack: ES DS AD ESP
    MOV AX, SS
    MOV DS, AX
    MOV ES, AX
    CALL _inthandler27
    POP EAX
    POPAD
    POP DS
    POP ES
    IRETD

_asm_inthandler2c:
    PUSH ES
    PUSH DS
    PUSHAD
    MOV EAX, ESP
    PUSH EAX
    ; Backup-stack: ES DS AD ESP
    MOV AX, SS
    MOV DS, AX
    MOV ES, AX
    CALL _inthandler2c
    POP EAX
    POPAD
    POP DS
    POP ES
    IRETD

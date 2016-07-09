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
    GLOBAL _load_cr0, _store_cr0
    GLOBAL _load_gdtr, _load_idtr
    ; Interrupt Handler Function
    GLOBAL _asm_inthandler20, _asm_inthandler21
    GLOBAL _asm_inthandler27, _asm_inthandler2c
    ; External C function for OS dev
    EXTERN _inthandler20, _inthandler21
    EXTERN _inthandler27, _inthandler2c
    ; Memory Test Function
    GLOBAL _memtest_sub
    ; Task Register
    GLOBAL _load_tr, _farjmp

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

; Eflags
_io_load_eflags: ;int io_load_eflags(void);
    PUSHFD ; Push eflags to stack
    POP EAX; pop eflags to EAX, EAX acts as ret reg
    RET

_io_store_eflags: ;void io_store_eflags(int eflags);
    MOV EDX, [ESP+4]
    PUSH EDX
    POPFD ; restore eflags register
    RET

; CR0
_load_cr0: ;int load_cr0(void);
    MOV EAX, CR0
    RET

_store_cr0: ;void store_cr0(int cr0);
    MOV EAX, [ESP+4]
    MOV CR0, EAX
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
_asm_inthandler20:
    PUSH ES
    PUSH DS
    PUSHAD
    MOV EAX, ESP
    PUSH EAX
    ; Backup-stack: ES DS AD ESP
    MOV AX, SS
    MOV DS, AX
    MOV ES, AX
    CALL _inthandler20
    POP EAX
    POPAD
    POP DS
    POP ES
    IRETD

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

; Memory Test Function
_memtest_sub: ;unsigned int memtest_sub(unsigned int start, unsigned int end);
    PUSH EDI
    PUSH ESI
    PUSH EBX
    MOV ESI, 0xaa55aa55 ;pat0 = 0xaa55aa55
    MOV EDI, 0x55aa55aa ;pat1 = 0x55aa55aa
    MOV EAX, [ESP+12+4] ;i = start (return i)
mts_loop:
    MOV EBX, EAX ; p = i + 0xffc
    ADD EBX, 0xffc
    MOV EDX, [EBX] ; old = *p
    MOV [EBX], ESI ; *p = pat0
    XOR DWORD [EBX], 0xffffffff ; *p ^= 0xffffffff
    CMP [EBX], EDI ; if(*p != pat1)
    JNE mts_fin
    
    XOR DWORD [EBX], 0xffffffff ; *p ^= 0xffffffff
    CMP [EBX], ESI ; if(*p != pat0)
    JNE mts_fin

    MOV [EBX], EDX ; *p = old
    ADD EAX, 0x1000
    CMP EAX, [ESP+12+8]
    JBE mts_loop

    POP EBX
    POP ESI
    POP EDI
    RET

mts_fin:
    MOV [EBX], EDX; *p = old
    POP EBX
    POP ESI
    POP EDI
    RET

; Task Register
_load_tr: ;_load_tr(int tr);
    LTR [ESP + 4]
    RET

_farjmp: ;void farjmp(int eip, int cs);
    JMP FAR [ESP+4] ;automatically fetch 2 arguments
    RET

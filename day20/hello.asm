[INSTRSET "i486p"]
[BITS 32]
    MOV EDX, 1 ;cons_putchar(cons, eax & 0xff, 1);
    MOV ECX, msg
putloop:
    MOV AL, [CS:ECX]
    CMP AL, 0
    JE fin
    INT 0x40 ;would fail because INT change ECX
    ADD ECX, 1
    JMP putloop
fin:
    RETF
msg:
    DB "hello", 0

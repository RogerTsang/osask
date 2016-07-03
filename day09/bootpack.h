#ifndef BOOTPACK_H
#define BOOTPACK_H

#define ADR_BOOTINFO	0x00000ff0

#define EFLAGS_AC_BIT   0x00040000 /* Used for testing i486 */
#define CR0_CACHE_DISABLE 0x60000000

/* Low Level Operations */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);

int io_load_eflags(void);
void io_store_eflags(int eflags);
int load_cr0();
void store_cr0(int cr0);

void write_mem8(int addr, int data);

/* Boot info from asmhead.asm:0x0ff0 */
struct _bootinfo {
    char cyls;
    char leds; 
    char vmode;
    char reserve;
    short scrnx;
    short scrny;
    unsigned char* vram;
};

#endif

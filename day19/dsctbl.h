#ifndef DSCTBL_H
#define DSCTBL_H

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
#define AR_TSS32        0x0089

/* Segmentation Descriptor */
struct _segmdes {
    short limit_low;
    short base_low;
    char base_mid;
    char access_right;
    char limit_high;
    char base_high;
};

/* Gate Descriptor */
struct _gatedes {
    short offset_low;
    short selector;
    char dw_count;
    char access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct _segmdes * sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct _gatedes *gd, int offset, int selector, int ar);

void load_gdtr(int limit, int base);
void load_idtr(int limit, int base);

/* ASM helper */
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

#endif

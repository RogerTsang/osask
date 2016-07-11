#include "bootpack.h"
#include "dsctbl.h"

extern struct _tss32 tss_a, tss_b;

void init_gdtidt(void) {
    struct _segmdes *gdt = (struct _segmdes *) ADR_GDT;
    struct _gatedes *idt = (struct _gatedes *) ADR_IDT;

    int i;
    /* global segementation descriptor table */
    for (i = 0; i < LIMIT_GDT / 8; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
    load_gdtr(LIMIT_GDT, ADR_GDT);

    /* interrupt descriptor table */
    for (i = 0; i < LIMIT_IDT / 8; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);

    /* setup interrupt handlers */
    set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2*8, AR_INTGATE32);
    set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2*8, AR_INTGATE32);
    set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2*8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2*8, AR_INTGATE32);

    return;
}

void set_segmdesc(struct _segmdes *sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000; /* Turn on granularity bit */
        limit /= 0x1000;
    }
    /* Match CPU data structure 8 bytes*/
    sd->limit_low = limit & 0xffff; /* 2 bytes */
    sd->base_low  = base & 0xffff; /* 2 bytes */
    sd->base_mid  = (base >> 16) & 0xff; /* 1 byte */
    sd->access_right = ar & 0xff; /* 1 byte */
    sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0); /* 1 byte */
    // sd->base_high    = (base > 24) & 0xff; <- such mistake, much learning
    sd->base_high = (base >> 24) & 0xff;
    return;
}

void set_gatedesc(struct _gatedes *gd, int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff; /* 2 bytes */
    gd->selector = selector; /* 2 bytes */
    gd->dw_count = (ar >> 8) & 0xff; /* 1 byte */
    gd->access_right = ar & 0xff; /* 1 byte */
    gd->offset_high = (offset >> 16) & 0xffff; /* 2 bytes */
    return;
}


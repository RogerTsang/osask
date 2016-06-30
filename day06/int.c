#include "int.h"
#include "bootpack.h"
#include "graphic.h"

void init_pic(void) {

    io_out8(PIC0_IMR, 0xff); /* Disable all interrupts */
    io_out8(PIC1_IMR, 0xff); /* Disable all interrupts */

    io_out8(PIC0_ICW1, 0x11); /* Edge Trigger Mode */
    io_out8(PIC0_ICW2, 0x20); /* IRQ0-7 map to INT20-27 */
    io_out8(PIC0_ICW3, 1<<2); /* PIC1 connects to IRQ2 */
    io_out8(PIC0_ICW4, 0x01); /* No Buffer */

    io_out8(PIC1_ICW1, 0x11); /* Edge Trigger Mode */
    io_out8(PIC1_ICW2, 0x28); /* IRQ8-15 map to INT28-2f */
    io_out8(PIC1_ICW3, 2);    /* PIC1 connects to IRQ2 */
    io_out8(PIC1_ICW4, 0x01); /* No Buffer */

    io_out8(PIC0_IMR, 0xfb); /* Disable all interrupts except PIC1 */
    io_out8(PIC1_IMR, 0xff); /* Disable all interrupts */
    return;
}

void inthandler21(int *esp) {
    struct _bootinfo *binfo = (struct _bootinfo *) ADR_BOOTINFO;
    draw_retangle8(binfo->vram, binfo->scrnx, COLOUR_BLACK, 0, 0, 32*8-1, 15);
    putstr8_asc(binfo->vram, binfo->scrnx, 1, 1, COLOUR_WHITE, "IRQ[21] PS/2 keyboard");

    while(1) io_hlt();
}

void inthandler2c(int *esp) {
    struct _bootinfo *binfo = (struct _bootinfo *) ADR_BOOTINFO;
    draw_retangle8(binfo->vram, binfo->scrnx, COLOUR_BLACK, 0, 0, 32*8-1, 15);
    putstr8_asc(binfo->vram, binfo->scrnx, 1, 1, COLOUR_WHITE, "IRQ[2C] PS/2 mouse");

    while(1) io_hlt();
}

void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67); /* STI */
}


#include "int.h"
#include "bootpack.h"

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

    /* Init Buffer */
    
    return;
}

/* Compulsory OS init interrupt */
void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67); /* STI */
}


#include <stdio.h>
#include "bootpack.h"
#include "dsctbl.h"
#include "graphic.h"
#include "int.h"

void HariMain(void) {
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    char dbmsg[40];
    char mcursor[256];

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Interrupt Programable Controller */
    init_pic();
    io_sti();

    /* Palette Setting */
    init_palette();

    /* Init Screen */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COLOUR_DCYAN);
    
    /* Mouse */
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 50, 50, mcursor, 16);

    /* Debug */
    sprintf(dbmsg, "scrnx = %d", binfo->scrnx);
    putstr8_asc(binfo->vram, binfo->scrnx, 26, 64, COLOUR_WHITE, dbmsg);

    /* Enable Interrupt */
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    while (1) io_hlt();
}

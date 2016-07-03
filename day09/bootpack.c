#include <stdio.h>
#include "bootpack.h"
#include "dsctbl.h"
#include "graphic.h"
#include "int.h"
#include "fifo.h"
#include "mouse.h"
#include "keyboard.h"
#include "memory.h"

extern struct _fifo8 keyfifo;
extern char keybuf[KB_BUFSIZE];
extern struct _fifo8 moufifo;
extern char moubuf[MO_BUFSIZE];
extern struct _mousedec mdec;

void HariMain(void) {
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    int i, mx, my;
    char dbmsg[40];
    char mcursor[256];
    char string[16];

    /* Memory info */
    unsigned int memtotal;
    struct _memman *memman = (struct _memman *) MEMMAN_ADDR;

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Interrupt Programable Controller */
    init_pic();
    io_sti();

    /* Init Keyboard and Mouse */
    init_keyboard();
    mx = binfo->scrnx / 2;
    my = binfo->scrny / 2;

    /* Palette Setting */
    init_palette();

    /* Init Screen */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COLOUR_DCYAN);
    
    /* Hello Message */
    sprintf(dbmsg, "scrnx = %d", binfo->scrnx);
    putstr8_asc(binfo->vram, binfo->scrnx, 26, 64, COLOUR_WHITE, dbmsg);

    /* Memory Check */
    memtotal = memtest(0x00400000, 0xbfffffff); /* 3GB */
    memman_init(memman);
    memman_free(memman,0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
    sprintf(string, "memory = %dKB", memtotal / 1024);
    putstr8_asc(binfo->vram, binfo->scrnx, 26, 84, COLOUR_WHITE, string);
    sprintf(string, " free  = %dKB", memman_total(memman) / 1024);
    putstr8_asc(binfo->vram, binfo->scrnx, 26, 104, COLOUR_WHITE, string);

    /* Enable Interrupt */
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    /* Mouse */
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    /* Init Keyboard and Mouse */
    enable_mouse(&mdec);

    while (1) {
        io_cli(); /* Disable Interrupt */
        if (fifo8_status(&keyfifo) + fifo8_status(&moufifo) == 0) {
            io_stihlt(); /* Enable Interrupt and halt - no interrupt */
        } else {
            if (fifo8_status(&keyfifo) != 0) {
                /* Read key from buffer */
                i = fifo8_get(&keyfifo);
                io_sti(); /* Re-enable Interrupt - handling interrupt buf*/
                sprintf(string, "%02x", i);
                draw_retangle8(binfo->vram, binfo->scrnx, COLOUR_DCYAN, 200, 20, 216, 52);
                putstr8_asc(binfo->vram, binfo->scrnx, 200, 20, COLOUR_WHITE, string);
            } else if (fifo8_status(&moufifo) != 0) {
                /* Read mouse from buffer */
                i = fifo8_get(&moufifo);
                io_sti(); /* Re-enable Interrupt - handling interrupt buf*/

                /* Decode Mouse */
                if (mouse_decode(&mdec, i) == 3) {
                    /* Print Out */
                    sprintf(string, "lcr%4d%4d", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) {
                        string[0] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0) {
                        string[2] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0) {
                        string[1] = 'C';
                    }
                    /* Hide Mouse */
                    draw_retangle8(binfo->vram, binfo->scrnx, COLOUR_DCYAN, mx, my, mx+16, my+16);
                    /* Position Mouse */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) mx = 0;
                    if (my < 0) my = 0;
                    if (mx > binfo->scrnx - 16) mx = binfo->scrnx - 16;
                    if (my > binfo->scrny - 16) my = binfo->scrny - 16;
                    /* Display Mouse */
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                }
            }
        }
    }
}

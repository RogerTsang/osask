#include <stdio.h>
#include "bootpack.h"
#include "dsctbl.h"
#include "graphic.h"
#include "int.h"
#include "fifo.h"
#include "mouse.h"
#include "keyboard.h"
#include "memory.h"
#include "layer.h"

extern struct _fifo8 keyfifo;
extern char keybuf[KB_BUFSIZE];
extern struct _fifo8 moufifo;
extern char moubuf[MO_BUFSIZE];
extern struct _mousedec mdec;

void HariMain(void) {
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    int i, mx, my;
    char string[32];
    struct _layerctl *lyrctl;
    struct _layer *lyr_back, *lyr_mouse;
    unsigned char *buf_back, buf_mouse[MOU_SIZE * MOU_SIZE];

    /* Memory info */
    unsigned int memtotal, memfree;
    struct _memman *memman = (struct _memman *) MEMMAN_ADDR;

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Interrupt Programable Controller */
    init_pic();
    io_sti();

    /* Enable Interrupt */
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    /* Init Keyboard and Mouse */
    init_keyboard();
    enable_mouse(&mdec);

    /* Memory Check */
    memtotal = memtest(0x00400000, 0xbfffffff); /* 3GB */
    memman_init(memman);
    memman_free(memman,0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
    memfree = memman_total(memman);

    /* Palette Setting */
    init_palette();
    
    /* Layer Init */
    lyrctl = layerctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    lyr_back = layer_alloc(lyrctl);
    lyr_mouse = layer_alloc(lyrctl);
    buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    layer_setbuf(lyr_back, buf_back, binfo->scrnx, binfo->scrny, COLOUR_NULL);
    layer_setbuf(lyr_mouse, buf_mouse, MOU_SIZE, MOU_SIZE, COLOUR_INVIS);

    /* Init Screen */
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(buf_mouse, COLOUR_INVIS);
    layer_slide(lyrctl, lyr_back, 0, 0);

    /* Mouse Positioning and drawing */
    mx = (binfo->scrnx - MOU_SIZE * 2) / 2;
    my = (binfo->scrny - MOU_SIZE * 2) / 2;
    layer_slide(lyrctl, lyr_mouse, mx, my);

    /* Layer Ordering */
    layer_setheight(lyrctl, lyr_back, 0);
    layer_setheight(lyrctl, lyr_mouse, 1);
    
    /* Information Display */
    sprintf(string, "scrnx %d x %d", binfo->scrnx, binfo->scrny);
    putstr8_asc(buf_back, binfo->scrnx, 26, 64, COLOUR_WHITE, string);
    sprintf(string, "memory = %dKB", memtotal / 1024);
    putstr8_asc(buf_back, binfo->scrnx, 26, 84, COLOUR_WHITE, string);
    sprintf(string, " free  = %dKB", memfree / 1024);
    putstr8_asc(buf_back, binfo->scrnx, 26, 104, COLOUR_WHITE, string);

    /* Refresh Screen */
    layerctl_refresh(lyrctl, lyr_back, 0, 0, binfo->scrnx, binfo->scrny);

    while (1) {
        io_cli(); /* Disable Interrupt */
        if (fifo8_status(&keyfifo) + fifo8_status(&moufifo) == 0) {
            io_stihlt(); /* Enable Interrupt and halt - no interrupt */
        } else {
            if (fifo8_status(&keyfifo) != 0) {
                /* Read key from buffer */
                i = fifo8_get(&keyfifo);
                /* Re-enable Interrupt - handling interrupt buf*/
                io_sti();
                /* Keyboard Info */
                sprintf(string, "%02x", i);
                draw_retangle8(buf_back, binfo->scrnx, COLOUR_DCYAN, 200, 0, 216, 18);
                putstr8_asc(buf_back, binfo->scrnx, 200, 0, COLOUR_WHITE, string);
                /* Refresh Screen */
                layerctl_refresh(lyrctl, lyr_back, 200, 0, 216, 18);
            } else if (fifo8_status(&moufifo) != 0) {
                /* Read mouse from buffer */
                i = fifo8_get(&moufifo);
                /* Re-enable Interrupt - handling interrupt buf*/
                io_sti(); 

                /* Decode Mouse */
                if (mouse_decode(&mdec, i) == 3) {
                    /* Print Out LCR status */
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
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_DCYAN, 200, 20, 300, 38);
                    putstr8_asc(buf_back, binfo->scrnx, 200, 20, COLOUR_WHITE, string);
                    layerctl_refresh(lyrctl, lyr_back, 200, 20, 300, 38);
                    /* Position Mouse */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) mx = 0;
                    if (my < 0) my = 0;
                    if (mx > binfo->scrnx - MOU_SIZE) mx = binfo->scrnx - MOU_SIZE;
                    if (my > binfo->scrny - MOU_SIZE) my = binfo->scrny - MOU_SIZE;
                    /* Display Mouse */
                    sprintf(string, "(%3d %3d)", mx, my);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_DCYAN, 200, 40, 300, 58);
                    putstr8_asc(buf_back, binfo->scrnx, 200, 40, COLOUR_WHITE, string);
                    layerctl_refresh(lyrctl, lyr_back, 200, 40, 300, 58);
                    layer_slide(lyrctl, lyr_mouse, mx, my);
                }
            }
        }
    }
}

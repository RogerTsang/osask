#include <stdio.h>
#include "bootpack.h"
#include "dsctbl.h"
#include "graphic.h"
#include "int.h"
#include "fifo.h"

extern struct _fifo8 keyfifo; extern char keybuf[KB_BUFSIZE];
extern struct _fifo8 moufifo; extern char moubuf[KB_BUFSIZE];
extern struct _mousedec mdec;

void HariMain(void) {
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    int i;
    char dbmsg[40];
    char mcursor[256];
    char string[16];

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Interrupt Programable Controller */
    init_pic();
    io_sti();

    /* Init Keyboard and Mouse */
    init_keyboard();

    /* Palette Setting */
    init_palette();

    /* Init Screen */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COLOUR_DCYAN);
    
    /* Debug */
    sprintf(dbmsg, "scrnx = %d", binfo->scrnx);
    putstr8_asc(binfo->vram, binfo->scrnx, 26, 64, COLOUR_WHITE, dbmsg);

    /* Enable Interrupt */
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    /* Mouse */
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 50, 50, mcursor, 16);

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
                    
                    draw_retangle8(binfo->vram, binfo->scrnx, COLOUR_DCYAN, 200, 60, 300, 92);
                    putstr8_asc(binfo->vram, binfo->scrnx, 200, 60, COLOUR_WHITE, string);
                }
            }
        }
    }
}

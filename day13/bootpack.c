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
#include "window.h"
#include "timer.h"

extern struct _fifo8 keyfifo;
extern char keybuf[KB_BUFSIZE];
extern struct _fifo8 moufifo;
extern char moubuf[MO_BUFSIZE];
extern struct _mousedec mdec;
extern struct _timerctl timerctl;

void HariMain(void) {
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    int i, mx, my;
    char string[32];
    /* Timer */
    struct _fifo8 timerfifo, timerfifo1, timerfifo2;
    char timerbuf[TIMER_BUFSIZE], timerbuf1[TIMER_BUFSIZE], timerbuf2[TIMER_BUFSIZE];
    struct _timer *timer, *timer1, *timer2;
    /* Layer */
    struct _layerctl *lyrctl;
    struct _layer *lyr_back, *lyr_mouse, *lyr_win;
    unsigned char *buf_back, buf_mouse[MOU_SIZE * MOU_SIZE], *buf_win;

    /* Memory info */
    unsigned int memtotal, memfree;
    struct _memman *memman = (struct _memman *) MEMMAN_ADDR;

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Programable Interval Timer */
    init_pit();
    /* Timer 0 10sec */
    fifo8_init(&timerfifo, TIMER_BUFSIZE, timerbuf);
    timer = timer_alloc();
    timer_init(timer, &timerfifo, 1);
    timer_settime(timer, 1000);
    /* Timer 1 3sec */
    fifo8_init(&timerfifo1, TIMER_BUFSIZE, timerbuf1);
    timer1 = timer_alloc();
    timer_init(timer1, &timerfifo1, 1);
    timer_settime(timer1, 300);
    /* Timer 2 cursor */
    fifo8_init(&timerfifo2, TIMER_BUFSIZE, timerbuf2);
    timer2 = timer_alloc();
    timer_init(timer2, &timerfifo2, 1);
    timer_settime(timer2, 50);

    /* Init Interrupt Programable Controller */
    init_pic();
    io_sti();

    /* Enable Interrupt */
    io_out8(PIC0_IMR, 0xf8);
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
    lyr_win = layer_alloc(lyrctl);

    /* Allocate Buf Memory */
    buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    buf_win  = (unsigned char *) memman_alloc_4k(memman, 160 * 68);
    layer_setbuf(lyr_back, buf_back, binfo->scrnx, binfo->scrny, COLOUR_NULL);
    layer_setbuf(lyr_mouse, buf_mouse, MOU_SIZE, MOU_SIZE, COLOUR_INVIS);
    layer_setbuf(lyr_win, buf_win, 160, 68, COLOUR_NULL);

    /* Init Screen */
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(buf_mouse, COLOUR_INVIS);

    /* Windows */
    make_window8(buf_win, 160, 68, "counter");

    /* Mouse Positioning and drawing */
    mx = (binfo->scrnx - MOU_SIZE * 2) / 2;
    my = (binfo->scrny - MOU_SIZE * 2) / 2;
    layer_slide(lyr_back, 0, 0);
    layer_slide(lyr_win, 160, 69);
    layer_slide(lyr_mouse, mx, my);

    /* Layer Ordering */
    layer_setheight(lyr_back, 0);
    layer_setheight(lyr_win, 1);
    layer_setheight(lyr_mouse, 2);
    
    /* Information Display */
    sprintf(string, "screen = %3d x %3d", binfo->scrnx, binfo->scrny);
    putstr8_asc_lyr(lyr_back, 6, 44, COLOUR_WHITE, COLOUR_DCYAN, string, 18);
    sprintf(string, "memory = %dKB", memtotal / 1024);
    putstr8_asc_lyr(lyr_back, 6, 64, COLOUR_WHITE, COLOUR_DCYAN, string, 18);
    sprintf(string, " free  = %dKB", memfree / 1024);
    putstr8_asc_lyr(lyr_back, 6, 84, COLOUR_WHITE, COLOUR_DCYAN, string, 18);

    /* Refresh Screen */
    layerctl_refresh(lyr_back, 0, 0, binfo->scrnx, binfo->scrny);

    while (1) {
        /* Window Counter */
        sprintf(string, "%10d", timerctl.count);
        draw_retangle8(buf_win, 160, COLOUR_GREY, 40, 28, 119, 43);
        putstr8_asc(buf_win, 160, 40, 28, COLOUR_BLACK, string);
        layerctl_refresh(lyr_win, 40, 28, 120, 44);
        /* Disable Interrupt */
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&moufifo) + 
                fifo8_status(&timerfifo) + fifo8_status(&timerfifo1) + fifo8_status(&timerfifo2) == 0) {
            io_sti(); /* Enable Interrupt and halt - no interrupt */
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
                layerctl_refresh(lyr_back, 200, 0, 216, 18);
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
                    layerctl_refresh(lyr_back, 200, 20, 300, 38);
                    /* Position Mouse */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) mx = 0;
                    if (my < 0) my = 0;
                    if (mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                    if (my > binfo->scrny - 1) my = binfo->scrny - 1;
                    /* Display Mouse */
                    sprintf(string, "(%3d %3d)", mx, my);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_DCYAN, 200, 40, 300, 58);
                    putstr8_asc(buf_back, binfo->scrnx, 200, 40, COLOUR_WHITE, string);
                    layerctl_refresh(lyr_back, 200, 40, 300, 58);
                    layer_slide(lyr_mouse, mx, my);
                }
            } else if (fifo8_status(&timerfifo) != 0) {
                i = fifo8_get(&timerfifo);
                io_sti();
                putstr8_asc(buf_back, binfo->scrnx, 0, 104, COLOUR_WHITE, "10[sec]");
                layerctl_refresh(lyr_back, 0, 104, 56, 120);
            } else if (fifo8_status(&timerfifo1) != 0) {
                i = fifo8_get(&timerfifo1);
                io_sti();
                putstr8_asc(buf_back, binfo->scrnx, 0, 120, COLOUR_WHITE, "3[sec]");
                layerctl_refresh(lyr_back, 0, 120, 48, 136);
            } else if (fifo8_status(&timerfifo2) != 0) {
                i = fifo8_get(&timerfifo2);
                io_sti();
                if (i != 0) {
                    timer_init(timer2, &timerfifo2, 0);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_WHITE, 8, 136, 15, 151);
                } else {
                    timer_init(timer2, &timerfifo2, 1);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_DCYAN, 8, 136, 15, 151);
                }
                timer_settime(timer2, 50);
                layerctl_refresh(lyr_back, 8, 136, 16, 152);
            }
        }
    }
}

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

extern struct _timerctl timerctl;

void HariMain(void) {
    /* FIFO buffer */
    struct _fifo32 fifo;
    int fifobuf[FIFO_BUFSIZE];
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    int i, mx, my, bmtimer = 0;
    char string[32];
    /* Timer */
    struct _timer *timer, *timer1, *timer2;
    /* Layer */
    struct _layerctl *lyrctl;
    struct _layer *lyr_back, *lyr_mouse, *lyr_win;
    unsigned char *buf_back, buf_mouse[MOU_SIZE * MOU_SIZE], *buf_win;
    /* Mouse */
    struct _mousedec mdec;
    /* Memory info */
    unsigned int memtotal, memfree;
    struct _memman *memman = (struct _memman *) MEMMAN_ADDR;

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Programable Interval Timer */
    init_pit();

    /* Init FIFO buffer */
    fifo32_init(&fifo, FIFO_BUFSIZE, fifobuf);

    /* Timer 0 10sec */
    timer = timer_alloc();
    timer_init(timer, &fifo, TDATA_10SEC);
    timer_settime(timer, 1000);
    /* Timer 1 3sec */
    timer1 = timer_alloc();
    timer_init(timer1, &fifo, TDATA_3SEC);
    timer_settime(timer1, 300);
    /* Timer 2 cursor */
    timer2 = timer_alloc();
    timer_init(timer2, &fifo, TDATA_CURSOR_L);
    timer_settime(timer2, 50);

    /* Init Interrupt Programable Controller */
    init_pic();
    io_sti();

    /* Enable Interrupt */
    io_out8(PIC0_IMR, 0xf8);
    io_out8(PIC1_IMR, 0xef);

    /* Init Keyboard and Mouse */
    init_keyboard(&fifo, FIFO_KEYBOARD_L);
    enable_mouse(&fifo, FIFO_MOUSE_L, &mdec);

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
    make_window8(buf_win, 160, 68, "Benchmark");

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
        /* Benchmark Timer */
        bmtimer++;
        /* Disable Interrupt */
        io_cli();
        if (fifo32_status(&fifo) == 0) {
            io_sti(); /* Enable Interrupt and halt - no interrupt */
        } else {
            /* Read data from fifo buffer */
            i = fifo32_get(&fifo);
            /* Re-enable Interrupt */
            io_sti();
            /* Interrupt Dispatch */
            if (FIFO_TIMER_L <= i && i <= FIFO_TIMER_H) {
                if (i == TDATA_10SEC) {
                    sprintf(string, "10[sec]", mx, my);
                    putstr8_asc_lyr(lyr_back, 0, 104, COLOUR_WHITE, COLOUR_DCYAN, string, 7);
                    sprintf(string, "%10d", bmtimer);
                    putstr8_asc_lyr(lyr_win, 40, 28, COLOUR_BLACK, COLOUR_GREY, string, 10);
                } else if (i == TDATA_3SEC) {
                    sprintf(string, "3[sec]", mx, my);
                    putstr8_asc_lyr(lyr_back, 0, 120, COLOUR_WHITE, COLOUR_DCYAN, string, 7);
                    bmtimer = 0;
                } else if (i == TDATA_CURSOR_L) {
                    /* Change the next data to TDATA_CURSOR_HIGH */
                    timer_init(timer2, &fifo, TDATA_CURSOR_H);
                    timer_settime(timer2, 50);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_DCYAN, 8, 136, 15, 151);
                    layerctl_refresh(lyr_back, 8, 136, 16, 152);
                } else if (i == TDATA_CURSOR_H) {
                    /* Change the next data to TDATA_CURSOR_HIGH */
                    timer_init(timer2, &fifo, TDATA_CURSOR_L);
                    timer_settime(timer2, 50);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_WHITE, 8, 136, 15, 151);
                    layerctl_refresh(lyr_back, 8, 136, 16, 152);
                }
            } else if (FIFO_KEYBOARD_L <= i && i <= FIFO_KEYBOARD_H) {
                /* Keyboard Info */
                sprintf(string, "%02x", i - FIFO_KEYBOARD_L);
                putstr8_asc_lyr(lyr_back, 200, 0, COLOUR_WHITE, COLOUR_DCYAN, string, 2);
            } else if (FIFO_MOUSE_L <= i && i <= FIFO_MOUSE_H) {
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
                    putstr8_asc_lyr(lyr_back, 200, 20, COLOUR_WHITE, COLOUR_DCYAN, string, 11);
                    /* Position Mouse */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) mx = 0;
                    if (my < 0) my = 0;
                    if (mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                    if (my > binfo->scrny - 1) my = binfo->scrny - 1;
                    /* Display Mouse */
                    sprintf(string, "(%3d %3d)", mx, my);
                    putstr8_asc_lyr(lyr_back, 200, 40, COLOUR_WHITE, COLOUR_DCYAN, string, 9);
                    layer_slide(lyr_mouse, mx, my);
                }
            }
        }
    }
}

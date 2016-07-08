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
#include "task.h"

/* Keytable */
static char keytable[KB_NUMCHAR] = {
      0,   0, '1', '2', '3', '4', '5', '6', 
    '7', '8', '9', '0', '-', '=',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
   '\'',   0,   0,'\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',
      0, ' ',   0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0,   0, '7', 
    '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.'
};

extern struct _timerctl timerctl;
extern struct _memman *memman;

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
    int cursor_x, cursor_c;
    struct _layerctl *lyrctl;
    struct _layer *lyr_back, *lyr_mouse, *lyr_win, *lyr_win_b[3];
    unsigned char *buf_back, buf_mouse[MOU_SIZE * MOU_SIZE], *buf_win, *buf_win_b;
    /* Task */
    struct _task *task_a, *task_b[3];
    /* Mouse */
    struct _mousedec mdec;
    /* Memory info */
    unsigned int memtotal, memfree;

    /* Init GDT IDT */
    init_gdtidt();

    /* Init Programable Interval Timer */
    init_pit();

    /* Init FIFO buffer */
    fifo32_init(&fifo, FIFO_BUFSIZE, fifobuf, 0);

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

    /* Init Task */
    task_a = task_init(memman);
    fifo.task = task_a;
    
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

    /* Init all other task */
    for (i = 0; i < 3; i++) {
        /* Layers */
        lyr_win_b[i] = layer_alloc(lyrctl);
        buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
        layer_setbuf(lyr_win_b[i], buf_win_b, 144, 52, COLOUR_NULL);
        /* Windows */
        sprintf(string, "task_b%d", i);
        make_window8(lyr_win_b[i], 144, 52, string, 0);
        /* Task */
        task_b[i] = task_alloc();
        task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
        task_b[i]->tss.eip = (int) &task_b_main; /* Run the same code */
        task_b[i]->tss.es = 1 * 8;
        task_b[i]->tss.cs = 2 * 8;
        task_b[i]->tss.ss = 1 * 8;
        task_b[i]->tss.ds = 1 * 8;
        task_b[i]->tss.fs = 1 * 8;
        task_b[i]->tss.gs = 1 * 8;
        *((int *) (task_b[i]->tss.esp + 4)) = (int) lyr_win_b[i]; /* Arg0 (ESP+4) as lyr_back */
        task_run(task_b[i]);
    }

    /* Windows */
    make_window8(lyr_win, 160, 68, "task_a", 1);
    make_textbox8(lyr_win, 8, 28, 144, 16, COLOUR_WHITE);
    cursor_c = COLOUR_BLACK;
    cursor_x = 8;

    /* Mouse Positioning and drawing */
    mx = (binfo->scrnx - MOU_SIZE * 2) / 2;
    my = (binfo->scrny - MOU_SIZE * 2) / 2;

    /* Layer Ordering */
    layer_slide(lyr_back, 0, 0);
    layer_slide(lyr_mouse, mx, my);
    layer_slide(lyr_win, 8, 56);
    layer_slide(lyr_win_b[0], 168, 56);
    layer_slide(lyr_win_b[1], 8, 116);
    layer_slide(lyr_win_b[2], 168, 116);
    layer_setheight(lyr_back, 0);
    layer_setheight(lyr_win, 1);
    layer_setheight(lyr_win_b[0], 2);
    layer_setheight(lyr_win_b[1], 3);
    layer_setheight(lyr_win_b[2], 4);
    layer_setheight(lyr_mouse, 5);
    
    /* Information Display */
    sprintf(string, "screen = %3d x %3d", binfo->scrnx, binfo->scrny);
    putstr8_asc_lyr(lyr_back, 6, 44, COLOUR_WHITE, COLOUR_DCYAN, string, 18);
    sprintf(string, "memory = %dKB", memtotal / 1024);
    putstr8_asc_lyr(lyr_back, 6, 64, COLOUR_WHITE, COLOUR_DCYAN, string, 18);
    sprintf(string, " free  = %dKB", memfree / 1024);
    putstr8_asc_lyr(lyr_back, 6, 84, COLOUR_WHITE, COLOUR_DCYAN, string, 18);

    while (1) {
        /* Benchmark Timer */
        //bmtimer++;
        /* Disable Interrupt */
        io_cli();
        if (fifo32_status(&fifo) == 0) {
            /* Change io_hlt() to task sleep */
            task_sleep(task_a);
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
                    /* Change the next data to TDATA_CURSOR_LOW */
                    timer_init(timer2, &fifo, TDATA_CURSOR_L);
                    timer_settime(timer2, 50);
                    draw_retangle8(buf_back, binfo->scrnx, COLOUR_WHITE, 8, 136, 15, 151);
                    layerctl_refresh(lyr_back, 8, 136, 16, 152);
                }
            } else if (FIFO_KEYBOARD_L <= i && i <= FIFO_KEYBOARD_H) {
                /* Keyboard Info */
                sprintf(string, "%02x", i - FIFO_KEYBOARD_L);
                putstr8_asc_lyr(lyr_back, 200, 0, COLOUR_WHITE, COLOUR_DCYAN, string, 2);
                /* Character */
                if (i < FIFO_KEYBOARD_L + KB_NUMCHAR) {
                    if (keytable[i - FIFO_KEYBOARD_L] != 0 && cursor_x < 144) {
                        string[0] = keytable[i - FIFO_KEYBOARD_L];
                        string[1] = '\0';
                        putstr8_asc_lyr(lyr_win, cursor_x, 28, COLOUR_BLACK, COLOUR_WHITE, string, 1);
                        cursor_x += 8;
                    }
                }
                /* BackSpace */
                if (i == FIFO_KEYBOARD_L + 0x0e && cursor_x > 8) {
                    putstr8_asc_lyr(lyr_win, cursor_x, 28, COLOUR_WHITE, COLOUR_WHITE, " ", 1);
                    cursor_x -= 8;
                }
                draw_retangle8(lyr_win->buf, lyr_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                layerctl_refresh(lyr_win, cursor_x, 28, cursor_x + 8, 44);
            } else if (FIFO_MOUSE_L <= i && i <= FIFO_MOUSE_H) {
                /* Decode Mouse */
                if (mouse_decode(&mdec, i) == 3) {
                    /* Print Out LCR status */
                    sprintf(string, "lcr%4d%4d", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) {
                        string[0] = 'L';
                        /* Move Window */
                        layer_slide(lyr_win, mx - 80, my - 8);
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

void task_b_main(int arg0) {
	struct _fifo32 fifo;
	struct _timer *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
    char string[12];
    struct _layer *lyr = (struct _layer *) arg0;

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, TDATA_1SEC);
	timer_settime(timer_1s, 100);

	while(1) {
        /* Calculate TimeSlice Interval */
        count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == TDATA_1SEC) {
                /* Benchmark Each Task */
                sprintf(string, "%10d", count - count0);
                putstr8_asc_lyr(lyr, 24, 28, COLOUR_BLACK, COLOUR_GREY, string, 11);
                count0 = count;
	            timer_settime(timer_1s, 100);
			}
		}
	}
}

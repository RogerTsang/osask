#include <stdio.h>
#include <string.h>
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
static char keytable0[KB_NUMCHAR] = {
      0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';','\'',   0,   0,'\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,0x5c,   0,   0,   0,   0,   0,   0,   0,   0,   0,0x5c,   0,   0
};

static char keytable1[KB_NUMCHAR] = {
      0,   0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':','\"',   0,   0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};
extern struct _timerctl timerctl;
extern struct _memman *memman;

int cons_newline(int cursor_y, struct _layer * lyr);

void HariMain(void) {
    /* FIFO buffer */
    struct _fifo32 fifo, keycmd;
    int fifobuf[FIFO_BUFSIZE], keycmd_buf[FIFO_BUFSIZE];
    /* Fetch video info from asmhead */
    struct _bootinfo * binfo = (struct _bootinfo *) ADR_BOOTINFO;
    int i, mx, my;
    unsigned char string[32];
    /* Timer */
    struct _timer *timer, *timer1;
    /* Layer */
    int cursor_x, cursor_c;
    struct _layerctl *lyrctl;
    struct _layer *lyr_back, *lyr_mouse, *lyr_win, *lyr_cons;
    unsigned char *buf_back, *buf_mouse, *buf_win, *buf_cons;
    /* Task */
    struct _task *task_a, *task_cons;
    int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
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
    fifo32_init(&keycmd, FIFO_BUFSIZE, keycmd_buf, 0);
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);

    /* Timer 0 10sec */
    timer = timer_alloc();
    timer_init(timer, &fifo, FIFO_10SEC);
    timer_settime(timer, 1000);
    /* Timer 1 3sec */
    timer1 = timer_alloc();
    timer_init(timer1, &fifo, FIFO_3SEC);
    timer_settime(timer1, 300);

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

    /* Init Task Mouse & Keyboard*/
    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 0);
    
    /* Layer Init */
    lyrctl = layerctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    lyr_back = layer_alloc(lyrctl);
    lyr_mouse = layer_alloc(lyrctl);
    lyr_win = layer_alloc(lyrctl);
    lyr_cons = layer_alloc(lyrctl);

    /* Allocate Buf Memory */
    buf_mouse = (unsigned char *) memman_alloc_4k(memman, MOU_SIZE * MOU_SIZE);
    buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    buf_win  = (unsigned char *) memman_alloc_4k(memman, 160 * 68);
    buf_cons  = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
    layer_setbuf(lyr_back, buf_back, binfo->scrnx, binfo->scrny, COLOUR_NULL);
    layer_setbuf(lyr_mouse, buf_mouse, MOU_SIZE, MOU_SIZE, COLOUR_INVIS);
    layer_setbuf(lyr_win, buf_win, 160, 68, COLOUR_NULL);
    layer_setbuf(lyr_cons, buf_cons, 256, 165, COLOUR_NULL);

    /* Init Screen */
    init_screen(buf_back, binfo->scrnx, binfo->scrny);

    /* lyr_win */
    make_window8(lyr_win, 160, 68, "task_a", 1);
    make_textbox8(lyr_win, 8, 28, 144, 16, COLOUR_WHITE);
    cursor_c = COLOUR_BLACK;
    cursor_x = 8;

    /* lyr_cons */
    make_window8(lyr_cons, 256, 165, "console", 0);
    make_textbox8(lyr_cons, 8, 28, 240, 128, COLOUR_BLACK);
    task_cons = task_alloc();
    task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
    task_cons->tss.eip = (int) &console_task; /* Run the same code */
    task_cons->tss.es = 1 * 8;
    task_cons->tss.cs = 2 * 8;
    task_cons->tss.ss = 1 * 8;
    task_cons->tss.ds = 1 * 8;
    task_cons->tss.fs = 1 * 8;
    task_cons->tss.gs = 1 * 8;
    /* Task Memory */
    *((int *) (task_cons->tss.esp + 4)) = (int) lyr_cons;
    *((int *) (task_cons->tss.esp + 8)) = (int) memtotal;
    *((int *) (task_cons->tss.esp + 12)) = (int) memfree;
    task_run(task_cons, 2, 2);

    /* Mouse Positioning and drawing */
    init_mouse_cursor8(buf_mouse, COLOUR_INVIS);
    mx = (binfo->scrnx - MOU_SIZE * 2) / 2;
    my = (binfo->scrny - MOU_SIZE * 2) / 2;

    /* Layer Ordering */
    layer_slide(lyr_back, 0, 0);
    layer_slide(lyr_mouse, mx, my);
    layer_slide(lyr_win, 50, 100);
    layer_slide(lyr_cons, 200, 200);

    layer_setheight(lyr_back, 0);
    layer_setheight(lyr_win, 1);
    layer_setheight(lyr_cons, 2);
    layer_setheight(lyr_mouse, 3);
    
    /* Information Display */
    sprintf(string, "screen = %3d x %3d", binfo->scrnx, binfo->scrny);
    putstr8_asc_lyr(lyr_back, 6, 44, COLOUR_WHITE, COLOUR_DCYAN, string, 18);
    sprintf(string, "memory = %dKB", memtotal / 1024);
    putstr8_asc_lyr(lyr_back, 6, 64, COLOUR_WHITE, COLOUR_DCYAN, string, 18);
    sprintf(string, " free  = %dKB", memfree / 1024);
    putstr8_asc_lyr(lyr_back, 6, 84, COLOUR_WHITE, COLOUR_DCYAN, string, 18);

    while (1) {
        if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
        }
        /* Disable Interrupt */
        io_cli();
        if (fifo32_status(&fifo) == 0) {
            /* Change io_hlt() to task sleep */
            task_sleep(task_a); /* task_a sleeps when there is no interrupt */
            io_sti(); /* Enable Interrupt - someone calls fifo32_put() */
        } else {
            /* Read data from fifo buffer */
            i = fifo32_get(&fifo);
            /* Re-enable Interrupt */
            io_sti();
            /* Interrupt Dispatch */
            if (FIFO_TIMER_L <= i && i <= FIFO_TIMER_H) {
                if (i == FIFO_10SEC) {
                    sprintf(string, "10[sec]", mx, my);
                    putstr8_asc_lyr(lyr_back, 0, 104, COLOUR_WHITE, COLOUR_DCYAN, string, 7);
                } else if (i == FIFO_3SEC) {
                    sprintf(string, "3[sec]", mx, my);
                    putstr8_asc_lyr(lyr_back, 0, 120, COLOUR_WHITE, COLOUR_DCYAN, string, 7);
                }
            } else if (FIFO_KEYBOARD_L <= i && i <= FIFO_KEYBOARD_H) {
                /* Keyboard Info */
                sprintf(string, "%02x", i - FIFO_KEYBOARD_L);
                putstr8_asc_lyr(lyr_back, 200, 0, COLOUR_WHITE, COLOUR_DCYAN, string, 2);
                if (i < FIFO_KEYBOARD_L + KB_NUMCHAR) {
                    /* Shift key handler */
                    if (key_shift == 0) {
                        string[0] = keytable0[i - FIFO_KEYBOARD_L];
                    } else {
                        string[0] = keytable1[i - FIFO_KEYBOARD_L];
                    }
                } else {
                    string[0] = 0;
                }
                /* A->a */
                if ('A' <= string[0] && string[0] <= 'Z') {
                    if (((key_leds & 4) == 0 && key_shift == 0) ||
                        ((key_leds & 4) > 0 && key_shift > 0)) {
                        putstr8_asc_lyr(lyr_back, 200, 200, COLOUR_WHITE, COLOUR_DCYAN, string, 1);
                        string[0] += 'a' - 'A'; /* Change to lower case */
                    }
                }
                string[1] = 0;
                /* Character */
                if (string[0] != 0) {
                    /* Data stream dispatcher */
                    if (key_to == 0) {
                        if (cursor_x < 128) {
                            putstr8_asc_lyr(lyr_win, cursor_x, 28, COLOUR_BLACK, COLOUR_WHITE, string, 1);
                            cursor_x += 8;
                        }
                    } else {
                        fifo32_put(&task_cons->fifo, string[0] + 256);
                    }
                }
                /* BackSpace */
                else if (i == FIFO_KEYBOARD_L + 0x0e) {
                    if (key_to == 0) {
                        if (cursor_x > 8) {
                            putstr8_asc_lyr(lyr_win, cursor_x, 28, COLOUR_WHITE, COLOUR_WHITE, " ", 1);
                            cursor_x -= 8;
                        }
                    } else {
                        fifo32_put(&task_cons->fifo, 0x08/* Backspace ASCII */ + 256);
                    }
                }
                /* Tab */
                else if (i == FIFO_KEYBOARD_L + 0x0f) {
                    if (key_to == 0) {
                        key_to = 1;
                        make_wtitle8(lyr_win, lyr_win->bxsize, "task_a", 0);
                        make_wtitle8(lyr_cons, lyr_cons->bxsize, "console", 1);
                    } else {
                        key_to = 0;
                        make_wtitle8(lyr_win, lyr_win->bxsize, "task_a", 1);
                        make_wtitle8(lyr_cons, lyr_cons->bxsize, "console", 0);
                    }
                    layerctl_refresh(lyr_win, 0, 0, lyr_win->bxsize, 21);
                    layerctl_refresh(lyr_cons, 0, 0, lyr_cons->bxsize, 21);
                }
                /* Shift */
                else if (i == FIFO_KEYBOARD_L + 0x2a /* LS Down */) {
                    key_shift |= 1;
                }
                else if (i == FIFO_KEYBOARD_L + 0x36 /* RS Down */) {
                    key_shift |= 2;
                }
                else if (i == FIFO_KEYBOARD_L + 0xaa /* LS Up   */) {
                    key_shift &= ~1;
                }
                else if (i == FIFO_KEYBOARD_L + 0xb6 /* RS Up   */) {
                    key_shift &= ~2;
                }
                /* Caps Lock*/
                else if (i == FIFO_KEYBOARD_L + 0x3a) {
                    key_leds ^= 4; /* Toggle CapsLock */
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                /* Num Lock */
                else if (i == FIFO_KEYBOARD_L + 0x45) {
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                /* Scroll Lock */
                else if (i == FIFO_KEYBOARD_L + 0x46) {
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                /* Successfully Receive Data */
                else if (i == FIFO_KEYBOARD_L + 0xfa) {
                    keycmd_wait = -1;
                }
                /* Waiting */
                else if (i == FIFO_KEYBOARD_L + 0xfe) {
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
                }
                /* Enter Key */
                else if (i == FIFO_KEYBOARD_L + 0x1c) {
                    if (key_to != 0) { /* To Console */
                        fifo32_put(&task_cons->fifo, 10 + 256);
                    }
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

void console_task(int arg0, int arg1, int arg2) {
    struct _timer *timer;
    struct _task *task = task_now();
    int i, fifobuf[128], x, y;
    int cursor_x = 16, cursor_y = 28, cursor_c = COLOUR_BLACK;
    char str[30], cmdline[30], *p;

    /* Diskimg file info */
    struct _fileinfo *finfo = (struct _fileinfo *) (ADR_DISKIMG + 0x2600);
    struct _layer *lyr = (struct _layer *) arg0;
    unsigned int memtotal = (unsigned int) arg1; 
    unsigned int memfree  = (unsigned int) arg2; 

    /* File System */
    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);

    /* A fifo32 alarm for current task */ 
    fifo32_init(&task->fifo, 128, fifobuf, task);

    timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);

    /* Prompt */
    putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, ">", 1);

	while(1) {
        /* Calculate TimeSlice Interval */
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
            task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
            if (i == FIFO_CURSOR_H || i == FIFO_CURSOR_L) {
                if (i == FIFO_CURSOR_H) {
                    timer_init(timer, &task->fifo, FIFO_CURSOR_L);
                    cursor_c = COLOUR_WHITE;
                } else /* i == FIFO_CURSOR_L */ {
                    timer_init(timer, &task->fifo, FIFO_CURSOR_H);
                    cursor_c = COLOUR_BLACK;
                }
            } else if (FIFO_KEYBOARD_L <= i && i <= FIFO_KEYBOARD_H) {
                if (i == 256 + 0x08 /* Backspace */) {
                    if (cursor_x > 16) {
                        putstr8_asc_lyr(lyr, cursor_x, cursor_y, COLOUR_WHITE, COLOUR_BLACK, " ", 1);
                        cursor_x -= 8;
                    }
                } else if (i == 256 + 0x0a /* Enter */) {
                    putstr8_asc_lyr(lyr, cursor_x, cursor_y, COLOUR_WHITE, COLOUR_BLACK, " ", 1);
                    cmdline[cursor_x / 8 - 2] = '\0'; /* End command line from length(cursor_x/8 - 2) */
                    cursor_y = cons_newline(cursor_y, lyr);
                    /* Cmdline */
                    if (strcmp(cmdline, "mem") == 0) {
                        sprintf(str, "total %dKB", memtotal / 1024);
                        putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, str, 30);
                        cursor_y = cons_newline(cursor_y, lyr);
                        sprintf(str, " free %dKB", memfree / 1024);
                        putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, str, 30);
                        cursor_y = cons_newline(cursor_y, lyr);
                        cursor_y = cons_newline(cursor_y, lyr);
                    } else if (strcmp(cmdline, "clear") == 0) { 
                        draw_retangle8(lyr->buf, 240, COLOUR_BLACK, 8, 28, 248, 164);
                        layerctl_refresh(lyr, 8, 28, 8 + 240, 28 + 128);
                        cursor_y = 28;
                    } else if (strcmp(cmdline, "ls") == 0) { 
                        for (x = 0; x < 244; x++) {
                            if (finfo[x].name[0] == 0x00) {
                                break;
                            }
                            if (finfo[x].name[0] != 0xe5) {
                                if ((finfo[x].type & 0x18) == 0 /* Neither DIR nor DISKINFO */) {
                                    /* Copy filename format to str starts from 0x2600 */
                                    sprintf(str, "filename.ext %7d", finfo[x].size);
                                    /* filename */
                                    for (y = 0; y < 8; y++) {
                                        str[y] = finfo[x].name[y];
                                    }
                                    /* ext */
                                    str[9] = finfo[x].ext[0];
                                    str[10] = finfo[x].ext[1];
                                    str[11] = finfo[x]. ext[2];
                                    putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, str, 30);
                                }
                            }
                            cursor_y = cons_newline(cursor_y, lyr);
                        }
                    } else if (strncmp(cmdline, "cat ", 4) == 0) {
                        /* Find filename */
                        for (y = 0; y < 11; y++) {
                            str[y] = ' ';
                        }
                        str[y] = '\0';
                        y = 0;
                        for (x = 4; y < 11 && cmdline[x] != 0; x++) {
                            /* Copy Filename & Ext */
                            if (cmdline[x] == '.' && y <= 8) {
                                y = 8;
                            } else {
                                str[y] = cmdline[x];
                                if ('a' <= str[y] && str[y] <= 'z') {
                                    str[y] -= 'a' - 'A';
                                }
                                y++;
                            }
                        }
                        /* Find file */
                        for (x = 0; x < 224; x++) {
                            if (finfo[x].name[0] == 0x00) {
                                break;
                            }
                            if ((finfo[x].type & 0x18) == 0) {
                                if (strcmp(finfo[x].name, str) == 0) {
                                    break;
                                }
                            }
                        }
                        /* Extract file data */
                        if (x < 224 && finfo[x].name[0] != 0) {
                            /* File Found */
                            y = finfo[x].size;
                            p = (char *) (finfo[x].clustno * 512 + 0x3e00 + ADR_DISKIMG);
                            //file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x3e00));
                            cursor_x = 8;
                            /* Print char by char */
                            for (x = 0; x < y; x++) {
                                str[0] = p[x];
                                str[1] = '\0';
                                if (str[0] == '\t') {
                                    while(1) {
                                        putstr8_asc_lyr(lyr, cursor_x, cursor_y, COLOUR_WHITE, COLOUR_BLACK, " ", 1);
                                        cursor_x += 8;
                                        if (cursor_x == 8 + 240) {
                                            cursor_x = 8;
                                            cursor_y = cons_newline(cursor_y, lyr);
                                        }
                                        if ((cursor_x - 8) % 32 == 0) break;
                                    }
                                } else if (str[0] == '\n') {
                                    cursor_x = 8;
                                    cursor_y = cons_newline(cursor_y, lyr);
                                } else if (str[0] == '\r') {

                                } else {
                                    putstr8_asc_lyr(lyr, cursor_x, cursor_y, COLOUR_WHITE, COLOUR_BLACK, str, 1);
                                    cursor_x += 8;
                                    if (cursor_x >= 8 + 240) {
                                        cursor_x = 8;
                                        cursor_y = cons_newline(cursor_y, lyr);
                                    }
                                }
                            }
                            //memman_free_4k(memman, (int) p, finfo[x].size);
                        } else {
                            /* File Not Found */
                            putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, "File not found.", 15);
                            cursor_y = cons_newline(cursor_y, lyr);
                        }
                        cursor_y = cons_newline(cursor_y, lyr);
                    } else if (cmdline[0] != 0) {
                        putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, "Bad Command.", 12);
                        cursor_y = cons_newline(cursor_y, lyr);
                        cursor_y = cons_newline(cursor_y, lyr);
                    }
                    putstr8_asc_lyr(lyr, 8, cursor_y, COLOUR_WHITE, COLOUR_BLACK, ">", 1);
                    cursor_x = 16;
                } else {
                    if (cursor_x < 240) {
                        str[0] = i - 256;
                        str[1] = '\0';
                        cmdline[cursor_x / 8 - 2] = i - 256;
                        putstr8_asc_lyr(lyr, cursor_x, cursor_y, COLOUR_WHITE, COLOUR_BLACK, str, 1);
                        cursor_x += 8;
                    }
                }
            }
            if (cursor_c >= 0) {
                draw_retangle8(lyr->buf, lyr->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
            }
            layerctl_refresh(lyr, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

int cons_newline(int cursor_y, struct _layer *lyr) {
    int x, y;
    if (cursor_y < 28 + 112) {
        cursor_y += 16;
    } else {
        for (y = 28; y < 28 + 112; y++) {
            for (x = 8; x < 8 + 240; x++) {
                /* Move up all characters 1 line */
                lyr->buf[x + y * lyr->bxsize] = lyr->buf[x + (y + 16) * lyr->bxsize];
            }
        }
        draw_retangle8(lyr->buf, 240, COLOUR_BLACK, 8, 148, 248, 164);
        layerctl_refresh(lyr, 8, 28, 248, 164);
    }
    return cursor_y;
}

/* FAT: 9 sectors = 9 * 512 = 4608 bytes */
void file_readfat(int *fat, unsigned char *img) {
    int i, j = 0;
    /* AB CD EF -> DAB EFC */
    for (i = 0; i < 2880; i += 2) {
        fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
        fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
        j += 3;
    }
    return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img) {
    int i;
    while (1) {
        if (size <= 512) {
            /* Last Sector */
            for (i = 0; i < size; i++) {
                buf[i] = img[clustno * 512 + i];
            }
            break; /* Hit end of file */
        } else {
            /* Remaining Sector > 1 */
            for (i = 0; i < 512; i++) {
                buf[i] = img[clustno * 512 + i];
            }
            size -= 512;
            buf += 512;
            clustno = fat[clustno];
        }
    }
    return;
}

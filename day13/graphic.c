#include "bootpack.h"
#include "graphic.h"

void init_palette() {
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, /* Black  */
        0xff, 0x00, 0x00, /* Red    */
        0x00, 0xff, 0x00, /* Green  */
        0x00, 0x00, 0xff, /* Blue   */
        0xff, 0xff, 0x00, /* Yellow */
        0x00, 0xff, 0xff, /* Cyan   */
        0xff, 0x00, 0xff, /* Purple */
        0xff, 0xff, 0xff, /* White  */
        0xE6, 0xE6, 0xE6, /* Grey   */
        0x94, 0x00, 0x00, /* DRed   */
        0x00, 0x94, 0x00, /* DGreen */
        0x00, 0x00, 0x94, /* DBlue  */
        0x94, 0x94, 0x00, /* DYellow*/
        0x00, 0x94, 0x94, /* DCyan  */
        0x94, 0x00, 0x94, /* DPurple*/
        0x94, 0x94, 0x94, /* DGrey  */
    };

    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start, int end, unsigned char *rgb) {
    int i, eflags /* A special status register */;
    /* Backup eflags before disabling interrupt */
    eflags = io_load_eflags(); 

    /* Disable Interrupt */
    io_cli();
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        /* Output RGB value to device */
        io_out8(0x03c9, rgb[0]);
        io_out8(0x03c9, rgb[1]);
        io_out8(0x03c9, rgb[2]);
        /* Next colour value */
        rgb += 3;
    }

    io_store_eflags(eflags); /* Restore eflags */
    return;
}

void init_screen(unsigned char *vram, int xsize, int ysize) {
    /* Draw Background */
    draw_retangle8(vram, xsize, COLOUR_DCYAN, 0, 0, xsize-1, ysize-29);
    draw_retangle8(vram, xsize, COLOUR_GREY , 0, ysize-28, xsize-1, ysize-28);
    /* Draw Task Bar */
    draw_retangle8(vram, xsize, COLOUR_WHITE, 0, ysize-27, xsize-1, ysize-27);
    draw_retangle8(vram, xsize, COLOUR_GREY , 0, ysize-26, xsize-1, ysize-1);
    /* Draw Start Button */
    draw_circle8(vram, xsize, COLOUR_WHITE, 15, ysize-15, 12);
    draw_circle8(vram, xsize, COLOUR_DGREY, 17, ysize-13, 12);
    draw_circle8(vram, xsize, COLOUR_GREY, 16, ysize-14, 12);
    /* Draw Dock */
    draw_retangle8(vram, xsize, COLOUR_DGREY, xsize-67, ysize-24, xsize-4, ysize-24);
    draw_retangle8(vram, xsize, COLOUR_DGREY, xsize-67, ysize-23, xsize-67, ysize-4);
    draw_retangle8(vram, xsize, COLOUR_WHITE, xsize-67, ysize-3, xsize-4, ysize-3);
    draw_retangle8(vram, xsize, COLOUR_WHITE, xsize-3, ysize-24, xsize-3, ysize-3);
    /* Draw Dots */
    draw_circle8(vram, xsize, COLOUR_RED, 45, ysize-21, 2);
    draw_circle8(vram, xsize, COLOUR_GREEN, 45, ysize-14, 2);
    draw_circle8(vram, xsize, COLOUR_BLUE, 45, ysize-7, 2);
    /* Put Font */
    putstr8_asc(vram, xsize, 11, 11, COLOUR_GREY, "Haribote OS");
    putstr8_asc(vram, xsize, 10, 10, COLOUR_WHITE, "Haribote OS");
}

void init_mouse_cursor8(char *mouse, char bc) {
    static char cursor[MOU_SIZE][MOU_SIZE] = {
        "************",
        "*OOOOOOOOO*.",
        "*OOOOOOOO*..",
        "*OOOOOOO*...",
        "*OOOOOO*....",
        "*OOOOO*.....",
        "*OOOO*......",
        "*OOO*.......",
        "*OO*........",
        "*O*.........",
        "**..........",
        "*...........",
    };
    int x, y;
    for (y = 0; y < MOU_SIZE; y++) {
        for (x = 0; x < MOU_SIZE; x++) {
            switch (cursor[y][x]) {
                case '*' : mouse[y * MOU_SIZE + x] = COLOUR_BLACK; break;
                case 'O' : mouse[y * MOU_SIZE + x] = COLOUR_WHITE; break;
                case '.' : mouse[y * MOU_SIZE + x] = bc; break;
            }
        }
    }
    return;
}

void draw_retangle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            vram[y * xsize + x] = c;
        }
    }
    return;
}

void draw_circle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int radius) {
    int x, y;
    for (y = y0 - radius; y <= y0 + radius; y++) {
        for (x = x0 - radius; x <= x0 + radius; x++) {
            if ((x-x0)*(x-x0) + (y-y0)*(y-y0) <= radius * radius) {
                vram[y * xsize + x] = c;
            } else {
                continue;
            }
        }
    }
}

void putfont8(char *vram, int xsize, int x0, int y0, char c, char *font) {
    int i;
    char *p, d;
    for (i = 0; i < 16; i++) {
        /* Positioning */
        p = vram + (y0 + i) * xsize + x0;
        /* Load in font data for each row */
        d = font[i];
        if ((d & 0x80) != 0) p[0] = c;
        if ((d & 0x40) != 0) p[1] = c;
        if ((d & 0x20) != 0) p[2] = c;
        if ((d & 0x10) != 0) p[3] = c;
        if ((d & 0x08) != 0) p[4] = c;
        if ((d & 0x04) != 0) p[5] = c;
        if ((d & 0x02) != 0) p[6] = c;
        if ((d & 0x01) != 0) p[7] = c;
    }
    return;
}

void putstr8_asc(char *vram, int xsize, int x0, int y0, char c, unsigned char *str) {
    for (; *str != '\0'; str++) {
        putfont8(vram, xsize, x0, y0, c, CHARADDR(*str));
        x0 += 8;
    }
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) {
    int x, y;
    for (y = 0; y < pysize; y++) {
        for (x = 0; x < pxsize; x++) {
            vram[(y + py0) * vxsize + (x + px0)] = buf[y * bxsize + x];
        }
    }
    return;
}

void putstr8_asc_lyr(struct _layer *lyr, int x, int y, int c, int b, char *s, int l) {
    draw_retangle8(lyr->buf, lyr->bxsize, b, x, y, x + l * 8 - 1, y + 15);
    putstr8_asc(lyr->buf, lyr->bxsize, x, y, c, s);
    layerctl_refresh(lyr, x, y, x + l * 8 - 1, y + 16);
    return;
}

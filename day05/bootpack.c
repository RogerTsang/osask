#define COLOUR_BLACK   0
#define COLOUR_RED     1
#define COLOUR_GREEN   2
#define COLOUR_BLUE    3
#define COLOUR_YELLOW  4
#define COLOUR_CYAN    5
#define COLOUR_PURPLE  6
#define COLOUR_WHITE   7
#define COLOUR_GREY    8
#define COLOUR_DRED    9
#define COLOUR_DGREEN  10
#define COLOUR_DBLUE   11
#define COLOUR_DYELLOW 12
#define COLOUR_DCYAN   13
#define COLOUR_DPURPLE 14
#define COLOUR_DGREY   15

#define CHARADDR(ch) (hankaku + (ch) * 16)

#include <stdio.h>

/* Low Level Operations */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);

int io_load_eflags(void);
void io_store_eflags(int eflags);

void write_mem8(int addr, int data);

/* Boot info from asmhead.asm:0x0ff0 */
struct _bootinfo {
    char cyls;
    char leds; 
    char vmode;
    char reserve;
    short scrnx;
    short scrny;
    unsigned char* vram;
};

/* Segmentation Descriptor */
struct _segmdes {
    short limit_low;
    short base_low;
    char base_mid;
    char access_right;
    char limit_high;
    char base_high;
};

/* Gate Descriptor */
struct _gatedes {
    short offset_low;
    short selector;
    char dw_count;
    char access_right;
    short offset_high;
};

/* Font File */
extern char hankaku[4096];

/* Functions */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void init_gdtidt(void);
void init_mouse_cursor8(char *mouse, char bc);
void draw_retangle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void draw_circle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int radius);
void putfont8(char *vram, int xsize, int x0, int y0, char c, char *font);
void putstr8_asc(char *vram, int xsize, int x0, int y0, char c, unsigned char *str); 
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

void set_segmdesc(struct _segmdes * sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct _gatedes *gd, int offset, int selector, int ar);

void HariMain(void) {
    struct _bootinfo * binfo;
    char dbmsg[40];
    char mcursor[256];

    /* Palette Setting */
    init_palette();

    /* Fetch video info from asmhead */
    binfo = (struct _bootinfo *) 0x0ff0;

    /* Init Screen */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COLOUR_DCYAN);
    
    /* Debug */
    sprintf(dbmsg, "scrnx = %d", binfo->scrnx);
    putstr8_asc(binfo->vram, binfo->scrnx, 26, 64, COLOUR_WHITE, dbmsg);

    /* Mouse */
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 50, 50, mcursor, 16);

    while (1) {
        io_hlt();
    }
}

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
    eflags = io_load_eflags(); //Backup eflags before disabling interrupt

    io_cli(); //Disable Interrupt
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        //Output RGB value to device
        io_out8(0x03c9, rgb[0]);
        io_out8(0x03c9, rgb[1]);
        io_out8(0x03c9, rgb[2]);
        //Next colour value
        rgb += 3;
    }

    io_store_eflags(eflags); //Restore eflags
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
    putstr8_asc(vram, xsize, 11, 11, COLOUR_GREY, "Hello World\nHaribote OS");
    putstr8_asc(vram, xsize, 10, 10, COLOUR_WHITE, "Hello World\nHaribote OS");
}

void init_gdtidt(void) {
    struct _segmdes *gdt = (struct _segmdes *) 0x00270000;
    struct _gatedes *idt = (struct _gatedes *) 0x0026f800;

    int i;
    /* global segementation descriptor table */
    for (i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);

    /* interrupt descriptor table */
    for (i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);

    return;
}

void set_segmdesc(struct _segmdes *sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000;
        limit /= 0x1000;
    }
    /* Match CPU data structure 8 bytes*/
    sd->limit_low = limit & 0xffff; /* 2 bytes */
    sd->base_low  = base & 0xffff; /* 2 bytes */
    sd->base_mid  = (base >> 16) & 0xff; /* 1 byte */
    sd->access_right = ar & 0xff; /* 1 byte */
    sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0); /* 1 byte */
    sd->base_high    = (base > 24) & 0xff; /* 1 byte */
    return;
}

void set_gatedesc(struct _gatedes *gd, int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff; /* 2 bytes */
    gd->selector = selector & 0xffff; /* 2 bytes */
    gd->dw_count = (ar >> 8) & 0xff; /* 1 byte */
    gd->access_right = ar & 0xff; /* 1 byte */
    gd->offset_high = (offset >> 16) & 0xffff; /* 2 bytes */
    return;
}

void init_mouse_cursor8(char *mouse, char bc) {
    static char cursor[16][16] = {
        "************....",
        "*OOOOOOOOO*.....",
        "*OOOOOOOO*......",
        "*OOOOOOO*.......",
        "*OOOOOO*........",
        "*OOOOO*.........",
        "*OOOO*..........",
        "*OOO*...........",
        "*OO*............",
        "*O*.............",
        "**..............",
        "*...............",
        "................",
        "................",
        "................",
        "................",
    };
    int x, y;
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            switch (cursor[y][x]) {
                case '*' : mouse[y * 16 + x] = COLOUR_BLACK; break;
                case 'O' : mouse[y * 16 + x] = COLOUR_WHITE; break;
                case '.' : mouse[y * 16 + x] = bc; break;
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
    int curx = x0;
    int cury = y0;
    for (; *str != '\0'; str++) {
        if (*str == '\n') {
            curx = x0;
            cury += 20;
            continue;
        }
        putfont8(vram, xsize, curx, cury, c, CHARADDR(*str));
        curx += 8;
        if (curx > xsize) {
            curx = 8;
            cury += 20;
        }
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

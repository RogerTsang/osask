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


void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);

int io_load_eflags(void);
void io_store_eflags(int eflags);

void write_mem8(int addr, int data);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void draw_retangle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void draw_circle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int radius);

void HariMain(void) {
    int xsize, ysize;
    unsigned char* vram;

    /* Palette Setting */
    init_palette();

    vram = (unsigned char *) 0xa0000;
    xsize = 320;
    ysize = 200;

    /* Draw Background */
    draw_retangle8(vram, xsize, COLOUR_DCYAN, 0, 0, xsize-1, ysize-29);
    draw_retangle8(vram, xsize, COLOUR_GREY , 0, ysize-28, xsize-1, ysize-28);
    /* Draw Task Bar */
    draw_retangle8(vram, xsize, COLOUR_WHITE, 0, ysize-27, xsize-1, ysize-27);
    draw_retangle8(vram, xsize, COLOUR_GREY , 0, ysize-26, xsize-1, ysize-1);
    /* Draw Start Button */
    /*
    draw_retangle8(vram, xsize, COLOUR_WHITE, 3 , ysize-24, 59, ysize-24);
    draw_retangle8(vram, xsize, COLOUR_WHITE, 2 , ysize-24, 2 , ysize-4);
    draw_retangle8(vram, xsize, COLOUR_DGREY, 3 , ysize-4 , 59, ysize-4);
    draw_retangle8(vram, xsize, COLOUR_DGREY, 59, ysize-23, 59, ysize-5);
    */
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

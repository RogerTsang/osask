#ifndef GRAPHIC_H
#define GRAPHIC_H

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

/* Font File */
extern char hankaku[4096];

/* Functions */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void init_mouse_cursor8(char *mouse, char bc);
void draw_retangle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void draw_circle8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int radius);
void putfont8(char *vram, int xsize, int x0, int y0, char c, char *font);
void putstr8_asc(char *vram, int xsize, int x0, int y0, char c, unsigned char *str); 
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

#endif

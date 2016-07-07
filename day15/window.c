#include "window.h"

void make_window8(struct _layer *lyr, int xsize, int ysize, char *title)
{
    unsigned char *buf = lyr->buf;
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	draw_retangle8(buf, xsize, COLOUR_GREY, 0,         0,         xsize - 1, 0        );
	draw_retangle8(buf, xsize, COLOUR_WHITE, 1,         1,         xsize - 2, 1        );
	draw_retangle8(buf, xsize, COLOUR_GREY, 0,         0,         0,         ysize - 1);
	draw_retangle8(buf, xsize, COLOUR_WHITE, 1,         1,         1,         ysize - 2);
	draw_retangle8(buf, xsize, COLOUR_GREY, xsize - 2, 1,         xsize - 2, ysize - 2);
	draw_retangle8(buf, xsize, COLOUR_BLACK, xsize - 1, 0,         xsize - 1, ysize - 1);
	draw_retangle8(buf, xsize, COLOUR_GREY, 2,         2,         xsize - 3, ysize - 3);
	draw_retangle8(buf, xsize, COLOUR_DBLUE, 3,         3,         xsize - 4, 20       );
	draw_retangle8(buf, xsize, COLOUR_DGREY, 1,         ysize - 2, xsize - 2, ysize - 2);
	draw_retangle8(buf, xsize, COLOUR_BLACK, 0,         ysize - 1, xsize - 1, ysize - 1);
	putstr8_asc(buf, xsize, 24, 4, COLOUR_WHITE, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COLOUR_BLACK;
			} else if (c == '$') {
				c = COLOUR_DGREY;
			} else if (c == 'Q') {
				c = COLOUR_GREY;
			} else {
				c = COLOUR_WHITE;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void make_textbox8(struct _layer *lyr, int x0, int y0, int sx, int sy, int c) {
	int x1 = x0 + sx, y1 = y0 + sy;
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_DGREY, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_DGREY, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_WHITE, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_WHITE, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_BLACK, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_BLACK, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_GREY, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	draw_retangle8(lyr->buf, lyr->bxsize, COLOUR_GREY, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	draw_retangle8(lyr->buf, lyr->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

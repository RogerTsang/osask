#include "window.h"
#include "graphic.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
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

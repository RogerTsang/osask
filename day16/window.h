#ifndef WINDOW_H
#define WINDOW_H

#include "graphic.h"

void make_window8(struct _layer *lyr, int xsize, int ysize, char *title, char act);
void make_textbox8(struct _layer *lyr, int x0, int y0, int sx, int sy, int c);

#endif

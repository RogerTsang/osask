#ifndef LAYER_H
#define LAYER_H

#include "memory.h"

#define LAYER_HIDDEN -1
#define LAYER_EMPTY -1
#define LAYER_UNUSED 0
#define LAYER_USED 1

#define MAX_LAYERS 256

struct _layer {
    unsigned char *buf;
    int bxsize, bysize;
    int vx0, vy0;
    int col_inv;
    int height, flags;
    struct _layerctl *ctl;
};

struct _layerctl {
    unsigned char *vram;
    int xsize, ysize, top;
    struct _layer *layers[MAX_LAYERS];
    struct _layer layers0[MAX_LAYERS];
};

/* Layer Functions */
struct _layer *layer_alloc(struct _layerctl *ctl);
void layer_setbuf(struct _layer *lyr, unsigned char *buf, int xsize, int ysize, int col_inv);
void layer_setheight(struct _layer *lyr, int height);
void layer_slide(struct _layer *lyr, int vx0, int vy0);
void layer_free(struct _layer *lyr);

/* Layer Control Functions */
struct _layerctl *layerctl_init(struct _memman *man, unsigned char *vram, int xsize, int ysize);
void layerctl_refresh(struct _layer *lyr, int bx0, int by0, int bx1, int by1);
void layerctl_refreshsub(struct _layerctl *ctl, int vx0, int vy0, int vx1, int vy1);

#endif

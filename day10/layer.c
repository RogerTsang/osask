#include "layer.h"

struct _layer *layer_alloc(struct _layerctl *ctl) {
    struct _layer *lyr = 0x00000000;
    int i;
    for (i = 0; i < MAX_LAYERS; i++) {
        if (ctl->layers0[i].flags == LAYER_UNUSED) {
            lyr = &(ctl->layers0[i]);
            lyr->flags = LAYER_USED;
            lyr->height = LAYER_HIDDEN;
        }
    }
    return lyr;
}

void layer_setbuf(struct _layer *lyr, unsigned char *buf, int xsize, int ysize, int col_inv) {
    lyr->buf = buf;
    lyr->bxsize = xsize;
    lyr->bysize = ysize;
    lyr->col_inv = col_inv;
}

void layer_setheight(struct _layerctl *ctl, struct _layer *lyr, int height) {
    int h, old = lyr->height;

    /* Range Constrain */
    if (height > ctl->top) height = ctl->top + 1;
    if (height < -1) height = -1;
    lyr->height = height;

    /* Reorder layers */
    if (height < old) {
        /* Going Lower */
        if (height >= 0) {
            /* Push all layers up and spare a slot */
            for (h = old; h > height; h--) {
                ctl->layers[h] = ctl->layers[h-1];
                ctl->layers[h]->height = h;
            }
            ctl->layers[height] = lyr;
        } else /* hide layer */{
            if (ctl->top > old) {
                for (h = old; h < ctl->top; h++) {
                    ctl->layers[h] = ctl->layers[h+1];
                    ctl->layers[h]->height = h;
                }
            }
            ctl->top--;
        }
        /* Refresh Display */
        layerctl_refresh(ctl);
    } else if (height > old) {
        /* Going Higher */
        if (old >= 0) {
            /* Push all layers down and spare a slot */
            for (h = old; h < height; h++) {
                ctl->layers[h] = ctl->layers[h+1];
                ctl->layers[h]->height = h;
            }
            ctl->layers[height] = lyr;
        } else /* display layer */ {
            /* Push all layers up and spare a slot */
            for (h = ctl->top; h >= height; h--) {
                ctl->layers[h+1] = ctl->layers[h];
                ctl->layers[h]->height = h;
            }
            ctl->layers[height] = lyr;
            ctl->top++;
        }
        layerctl_refresh(ctl);
    }
    /* If nothing change, don't refresh */
    return;
}

void layer_slide(struct _layerctl *ctl, struct _layer *lyr, int vx0, int vy0) {
    lyr->vx0 = vx0;
    lyr->vy0 = vy0;
    if (lyr->height != LAYER_HIDDEN) {
        layerctl_refresh(ctl);
    }
    return;
}

void layer_free(struct _layerctl *ctl, struct _layer *lyr) {
    if (lyr->height != LAYER_HIDDEN) {
        layer_setheight(ctl, lyr, LAYER_HIDDEN);
        layerctl_refresh(ctl);
    }
    lyr->flags = LAYER_UNUSED;
    return;
}

struct _layerctl *layerctl_init(struct _memman *man, unsigned char *vram, int xsize, int ysize) {
    struct _layerctl *ctl;
    int i;
    
    /* Allocate Memory For Layer Management Unit */
    ctl = (struct _layerctl *) memman_alloc_4k(man, sizeof(struct _layerctl));
    if (ctl == 0) return 0x00000000;

    /* Setup Variable */
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = LAYER_EMPTY;
    for (i = 0; i < MAX_LAYERS; i++) {
        ctl->layers0[i].flags = LAYER_UNUSED;
    }
    return ctl;
}

void layerctl_refresh(struct _layerctl *ctl) {
    int h, bx /* LayerX */, by /* LayerY */;
    int vx /* ScreenX */, vy /* ScreenY */; 
    unsigned char *buf, c, *vram = ctl->vram;
    struct _layer *lyr;
    /* Scheme: Print layer by layer from bottom to top */
    for (h = 0; h <= ctl->top; h++) {
        lyr = ctl->layers[h];
        buf = lyr->buf;
        /* Print By Region */
        for (by = 0; by < lyr->bysize; by++) {
            vy = lyr->vy0 + by;
            for (bx = 0; bx < lyr->bxsize; bx++) {
                vx = lyr->vx0 + bx;
                /* Positioning Done, Find the colour from buf*/
                c = buf[by * lyr->bxsize + bx];
                if (c != lyr->col_inv) {
                    vram[vy * ctl->xsize + vx] = c;
                }
            }
        }
    }
    return;
}

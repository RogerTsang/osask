#include "layer.h"

struct _layer *layer_alloc(struct _layerctl *ctl) {
    struct _layer *lyr;
    int i;
    for (i = 0; i < MAX_LAYERS; i++) {
        if (ctl->layers0[i].flags == LAYER_UNUSED) {
            lyr = &ctl->layers0[i];
            lyr->flags = LAYER_USED;
            lyr->height = LAYER_HIDDEN;
            return lyr;
        }
    }
    return 0;
}

void layer_setbuf(struct _layer *lyr, unsigned char *buf, int xsize, int ysize, int col_inv) {
    lyr->buf = buf;
    lyr->bxsize = xsize;
    lyr->bysize = ysize;
    lyr->col_inv = col_inv;
}

void layer_setheight(struct _layer *lyr, int height) {
    struct _layerctl *ctl = lyr->ctl;
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
        layerctl_refresh(lyr, 0, 0, lyr->bxsize, lyr->bysize);
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
        layerctl_refresh(lyr, 0, 0, lyr->bxsize, lyr->bysize);
    }
    /* If nothing change, don't refresh */
    return;
}

void layer_slide(struct _layer *lyr, int vx0, int vy0) {
    int old_vx0 = lyr->vx0;
    int old_vy0 = lyr->vy0;
    lyr->vx0 = vx0;
    lyr->vy0 = vy0;
    if (lyr->height != LAYER_HIDDEN) {
        /* Refresh Old Block */
        layerctl_refreshsub(lyr->ctl, old_vx0, old_vy0, old_vx0 + lyr->bxsize, old_vy0 + lyr->bysize);
        /* Refresh New Block */
        layerctl_refreshsub(lyr->ctl, vx0, vy0, vx0 + lyr->bxsize, vy0 + lyr->bysize);
    }
    return;
}

void layer_free(struct _layer *lyr) {
    struct _layerctl *ctl = ctl;
    if (lyr->height != LAYER_HIDDEN) {
        layer_setheight(lyr, LAYER_HIDDEN);
    }
    lyr->flags = LAYER_UNUSED;
    return;
}

struct _layerctl *layerctl_init(struct _memman *man, unsigned char *vram, int xsize, int ysize) {
    struct _layerctl *ctl;
    int i;
    
    /* Allocate Memory For Layer Management Unit */
    ctl = (struct _layerctl *) memman_alloc_4k(man, sizeof(struct _layerctl));
    if (ctl == 0) return ctl;

    /* Setup Variable */
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = LAYER_EMPTY;
    for (i = 0; i < MAX_LAYERS; i++) {
        ctl->layers0[i].flags = LAYER_UNUSED;
        ctl->layers0[i].ctl = ctl;
    }
    return ctl;
}

void layerctl_refresh(struct _layer *lyr, int bx0, int by0, int bx1, int by1) {
    struct _layerctl *ctl = lyr->ctl;
    if (lyr->height != LAYER_HIDDEN) {
        layerctl_refreshsub(ctl, lyr->vx0 + bx0, lyr->vy0 + by0, lyr->vx0 + bx1, lyr->vy0 + by1);
    }
    return;
}

void layerctl_refreshsub(struct _layerctl *ctl, int vx0, int vy0, int vx1, int vy1) {
    int h, bx, by, vx, vy;
    int bx0, by0, bx1, by1;
    unsigned char *buf, c;
    struct _layer *lyr;
    unsigned char *vram = ctl->vram;
    /* Prohibit Off Screen Rendering */
    if (vx0 < 0) vx0 = 0;
    if (vy0 < 0) vy0 = 0;
    if (vx1 > ctl->xsize) vx1 = ctl->xsize;
    if (vy1 > ctl->ysize) vy1 = ctl->ysize;
    for (h = 0; h <= ctl->top; h++) {
        lyr = ctl->layers[h];
        buf = lyr->buf;
        /* Calculate Refresh Region */
        bx0 = vx0 - lyr->vx0;
        by0 = vy0 - lyr->vy0;
        bx1 = vx1 - lyr->vx0;
        by1 = vy1 - lyr->vy0;
        /* Constrain Overlap Region */
        if (bx0 < 0) bx0 = 0;
        if (by0 < 0) by0 = 0;
        if (bx1 > lyr->bxsize) bx1 = lyr->bxsize;
        if (by1 > lyr->bysize) by1 = lyr->bysize;
        /* Update Vram inside the region */
        for (by = by0; by < by1; by++) {
            vy = lyr->vy0 + by;
            for (bx = bx0; bx < bx1; bx++) {
                vx = lyr->vx0 + bx;
                c = buf[by * lyr->bxsize + bx];
                if (c != lyr->col_inv) {
                    vram[vy * ctl->xsize + vx] = c;
                }
            }
        }
    }
    return;
}

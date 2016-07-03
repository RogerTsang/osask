#ifndef FIFO_H
#define FIFO_H

#define FLAGS_OVERRUN 0x0001

struct _fifo8 {
    unsigned char *buf;
    int p, q, size, free, flags;
};

void fifo8_init(struct _fifo8 *fifo, int size, unsigned char *buf);
int  fifo8_put(struct _fifo8 *fifo, unsigned char data);
int  fifo8_get(struct _fifo8 *fifo);
int  fifo8_status(struct _fifo8 *fifo);

#endif

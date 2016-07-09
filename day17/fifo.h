#ifndef FIFO_H
#define FIFO_H

#define FLAGS_OVERRUN 0x0001

#define FIFO_PUT      100
#define FIFO_10SEC    10
#define FIFO_3SEC     3
#define FIFO_TS       2
#define FIFO_1SEC     1
#define FIFO_CURSOR_H 1
#define FIFO_CURSOR_L 0

struct _fifo8 {
    unsigned char *buf;
    int p, q, size, free, flags;
};

struct _fifo32 {
    unsigned int *buf;
    int p, q, size, free, flags;
    struct _task *task;
};


void fifo8_init(struct _fifo8 *fifo, int size, unsigned char *buf);
int  fifo8_put(struct _fifo8 *fifo, unsigned char data);
int  fifo8_get(struct _fifo8 *fifo);
int  fifo8_status(struct _fifo8 *fifo);

void fifo32_init(struct _fifo32 *fifo, int size, unsigned int *buf, struct _task *task);
int  fifo32_put(struct _fifo32 *fifo, unsigned int data);
int  fifo32_get(struct _fifo32 *fifo);
int  fifo32_status(struct _fifo32 *fifo);
#endif

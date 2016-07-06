#ifndef MOUSE_H
#define MOUSE_H

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE     0xf4

#define MO_BUFSIZE     128

#include "fifo.h"
#include "int.h"

/* Mouse Decode Buffer */
struct _mousedec {
    unsigned char phase;
    unsigned char buf[3];
    int x;
    int y;
    int btn;
};

void inthandler2c(int *esp);
void enable_mouse(struct _fifo32 *fifo, int data0, struct _mousedec *mdec);
int mouse_decode(struct _mousedec *mdec, unsigned char dat);

#endif

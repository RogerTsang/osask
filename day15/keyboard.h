#ifndef KEYBOARD_H
#define KEYBOARD_H

#define PORT_KEYDAT     0x0060
#define PORT_KEYSTA     0x0064
#define PORT_KEYCMD     0x0064
#define KEYSTA_SEND_NOTREADY  0x02
#define KEYCMD_WRITE_MODE     0x60
#define KBC_MODE              0x47

#define KB_BUFSIZE     32

#include "fifo.h"
#include "int.h"


void wait_KBC_sendready(void);
void inthandler21(int *esp);
void init_keyboard(struct _fifo32 *fifo, int data0);

#endif

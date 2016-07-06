#include "mouse.h"
#include "keyboard.h"
#include "bootpack.h"

/* Buffer now shared with others */
struct _fifo32 *moufifo;
unsigned int mousedata0;

/* Mouse Interrupt */
void inthandler2c(int *esp) {
    unsigned int data;
    /* Indicates PIC1 IRQ-12 is handled */
    io_out8(PIC1_OCW2, 0x64);
    /* Indicates PIC0 IRQ-2  is handled */
    io_out8(PIC0_OCW2, 0x62);
    /* Write data to buffer (same port as KB) */
    data = io_in8(PORT_KEYDAT);
    fifo32_put(moufifo, data + mousedata0);
    return;
}

void enable_mouse(struct _fifo32 *fifo, int data0, struct _mousedec *mdec) {
    /* FIFO now is global variable (bootpack.c) */
    moufifo = fifo;
    mousedata0 = data0;
    /* Indicate KBC to propagate next instruction to mouse */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    /* Mouse CMD: Enable mouse */
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    /* Set phase to 0 */
    mdec->phase = 0;
    return;
}

int mouse_decode(struct _mousedec *mdec, unsigned char dat) {
    
    /* Parse Mouse Data */
    if (mdec->phase == 0) {
        /* Init */
        if (dat == 0xfa) mdec->phase = 1;
        return 0;
    } else if (mdec->phase == 1) {
        /* Check the first digit 
         * Usually the first byte contains 0b00YX1CRL (Yneg Xneg Cbut Rbut Lbut)
         * The fourth bit is always set to 1 if there is no data corruption
         * 
         * If there's an accident, say dat[0] = 0b00111010 -> 0b0111010_ (left shift 1)
         * This can be detected immediately and interleave movement operations
         */
        if ((dat & 0xc8) == 0x08) { /* 0b11001000 */
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 1;
    } else if (mdec->phase == 2) {
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 2;
    } else if (mdec->phase == 3) {
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07; /* Last 3bit for button */
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];

        if ((mdec->buf[0] & 0x10) != 0) {
            mdec->x |= 0xffffff00; /* Signed (Negative) Extension */
        }
        if ((mdec->buf[0] & 0x20) != 0) {
            mdec->y |= 0xffffff00;
        }
        mdec->y = -mdec->y; /* Y axis is inversed */
        return 3;
    }
    return -1; /* Never Reach */
}


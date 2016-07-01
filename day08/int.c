#include "int.h"
#include "bootpack.h"
#include "graphic.h"
#include "fifo.h"

/* Buffer */
struct _fifo8 keyfifo; char keybuf[KB_BUFSIZE];
struct _fifo8 moufifo; char moubuf[MO_BUFSIZE];
struct _mousedec mdec;

void init_pic(void) {

    io_out8(PIC0_IMR, 0xff); /* Disable all interrupts */
    io_out8(PIC1_IMR, 0xff); /* Disable all interrupts */

    io_out8(PIC0_ICW1, 0x11); /* Edge Trigger Mode */
    io_out8(PIC0_ICW2, 0x20); /* IRQ0-7 map to INT20-27 */
    io_out8(PIC0_ICW3, 1<<2); /* PIC1 connects to IRQ2 */
    io_out8(PIC0_ICW4, 0x01); /* No Buffer */

    io_out8(PIC1_ICW1, 0x11); /* Edge Trigger Mode */
    io_out8(PIC1_ICW2, 0x28); /* IRQ8-15 map to INT28-2f */
    io_out8(PIC1_ICW3, 2);    /* PIC1 connects to IRQ2 */
    io_out8(PIC1_ICW4, 0x01); /* No Buffer */

    io_out8(PIC0_IMR, 0xfb); /* Disable all interrupts except PIC1 */
    io_out8(PIC1_IMR, 0xff); /* Disable all interrupts */

    /* Init Buffer */
    fifo8_init(&keyfifo, KB_BUFSIZE, keybuf);
    fifo8_init(&moufifo, MO_BUFSIZE, moubuf);
    
    return;
}

/* Keyboard Controller Util */
void wait_KBC_sendready(void) {
    while (1) {
        /* Keep reading keyboard status and checking NOTREADY bit */
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            /* NOTREADY bit is set to 0 */
            /* Ready to send another command to keyboard */
            break;
        }
    }
    return;
}

/* Keyboard Interrupt */
void inthandler21(int *esp) {
    unsigned char data;

    /* Indicates PIC0 IRQ-01 signal is handled */
    io_out8(PIC0_OCW2, 0x61); 

    /* Receive Key Num from PORT_KEYDAT */
    data = io_in8(PORT_KEYDAT);

    /* Write data to buffer */
    fifo8_put(&keyfifo, data);

    return;
}

void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

/* Mouse Interrupt */
void inthandler2c(int *esp) {
    unsigned char data;
    /* Indicates PIC1 IRQ-12 is handled */
    io_out8(PIC1_OCW2, 0x64);
    /* Indicates PIC0 IRQ-2  is handled */
    io_out8(PIC0_OCW2, 0x62);
    /* Write data to buffer (same port as KB) */
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&moufifo, data);
    return;
}

void enable_mouse(struct _mousedec *mdec) {
    wait_KBC_sendready();
    /* Indicate KBC to propagate next instruction to mouse */
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    /* Mouse CMD: Enable mouse */
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

/* Compulsory OS init interrupt */
void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67); /* STI */
}


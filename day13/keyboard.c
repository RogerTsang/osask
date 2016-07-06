#include "keyboard.h"
#include "bootpack.h"

/* Keyboard Buffer now shared with other facilities */
struct _fifo32 *keyfifo;
unsigned int keydata0;

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
    unsigned int data;

    /* Indicates PIC0 IRQ-01 signal is handled */
    io_out8(PIC0_OCW2, 0x61); 

    /* Receive Key Num from PORT_KEYDAT */
    data = io_in8(PORT_KEYDAT);

    /* Write data to buffer */
    fifo32_put(keyfifo, data + keydata0);

    return;
}

void init_keyboard(struct _fifo32 *fifo, int data0) {
    keyfifo = fifo;
    keydata0 = data0;
    /* Init Keyboard Buffer */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}


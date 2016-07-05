#include "keyboard.h"
#include "bootpack.h"

/* Keyboard Buffer */
struct _fifo8 keyfifo;
char keybuf[KB_BUFSIZE];

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
    /* Init Keyboard Buffer */
    fifo8_init(&keyfifo, KB_BUFSIZE, keybuf);
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}


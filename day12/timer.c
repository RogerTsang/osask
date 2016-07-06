#include "timer.h"
#include "int.h"
#include "bootpack.h"

struct _timerctl timerctl;
struct _fifo8 timerfifo;
unsigned char timerbuf[TIMER_BUFSIZE];

void init_pit(void) {
    int i;
    /* Enable PIT */
    io_out8(PIT_CTRL, 0x34);
    /* Set timer interval 0x2e9c = 11932 cycles/irq */
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    /* Initialise Timer */
    timerctl.count = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timer[i].flags = TIMER_FLAGS_UNUSED;
    }
    fifo8_init(&timerfifo, TIMER_BUFSIZE, timerbuf);
    return;
}

struct _timer *timer_alloc(void) {
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer[i].flags == TIMER_FLAGS_UNUSED) {
            timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];
        }
    }
    return 0;
}

void timer_free(struct _timer *timer) {
    timer->flags = 0;
    return;
}

void timer_init(struct _timer *timer, struct _fifo8 *fifo, unsigned char data) {
    timer->fifo = fifo;
    timer->data = data;
    return;
}

void timer_settime(struct _timer *timer, unsigned int timeout) {
    timer->timeout = timeout;
    timer->flags = TIMER_FLAGS_USING;
}

void inthandler20(int *esp) {
    int i;
    /* Indicates PIC that IRQ0 is handled */
    io_out8(PIC0_OCW2, 0x60);
    /* Increment Timer */
    timerctl.count++;
    for (i = 0; i < MAX_TIMER; i++) {
        /* Count down if timeout is set */
        if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {
            timerctl.timer[i].timeout--;
            if (timerctl.timer[i].timeout == 0) {
                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            }
        }
    }
}


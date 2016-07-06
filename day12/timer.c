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
    timerctl.next = 0xffffffff;
    timerctl.using = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = TIMER_FLAGS_UNUSED;
    }
    fifo8_init(&timerfifo, TIMER_BUFSIZE, timerbuf);
    return;
}

struct _timer *timer_alloc(void) {
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timers0[i].flags == TIMER_FLAGS_UNUSED) {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];
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
    int e, i, j;
    /* Register timer in timerctl */
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    e = io_load_eflags();
    io_cli();
    /* Find *timer slot */
    for (i = 0; i < timerctl.using; i++) {
        if (timerctl.timers[i]->timeout >= timer->timeout) {
            break;
        }
    }
    /* Spare a slot by shifting all timers to right */
    for (j = timerctl.using; j > i; j--) {
        timerctl.timers[j] = timerctl.timers[j - 1];
    }
    timerctl.using++;
    /* Set timeout if current timer has the most recent deadline */
    timerctl.timers[i] = timer;
    timerctl.next = timerctl.timers[0]->timeout;
    io_store_eflags(e);
    return;
}

void inthandler20(int *esp) {
    int i, j;
    /* Indicates PIC that IRQ0 is handled */
    io_out8(PIC0_OCW2, 0x60);
    /* Increment Timer */
    timerctl.count++;
    if (timerctl.next > timerctl.count) {
        return; /* Interleave if no deadline */
    }
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timers[i]->timeout > timerctl.count) {
            break; /* Interleave if timeout is met */
        }
        timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
        fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    /* Shift out timeouted timer */
    timerctl.using -= i;
    for (j = 0; j < timerctl.using; j++) {
       timerctl.timers[j] = timerctl.timers[i + j]; 
    }
    /* Mark the first timeout in the queue as next */
    if (timerctl.using > 0) {
        timerctl.next = timerctl.timers[0]->timeout;
    } else {
        timerctl.next = 0xffffffff;
    }
    return;
}


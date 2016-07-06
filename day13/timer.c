#include "timer.h"
#include "int.h"
#include "bootpack.h"

struct _timerctl timerctl;
struct _fifo32 timerfifo;

void init_pit(void) {
    int i;
    /* Enable PIT */
    io_out8(PIT_CTRL, 0x34);
    /* Set timer interval 0x2e9c = 11932 cycles/irq */
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    /* Initialise Timer */
    timerctl.count = 0;
    timerctl.nextto = 0xffffffff;
    timerctl.using = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = TIMER_FLAGS_UNUSED;
    }
    return;
}

struct _timer *timer_alloc(void) {
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timers0[i].flags == TIMER_FLAGS_UNUSED) {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            timerctl.timers0[i].next = 0;
            return &timerctl.timers0[i];
        }
    }
    return 0;
}

void timer_free(struct _timer *timer) {
    timer->flags = 0;
    return;
}

void timer_init(struct _timer *timer, struct _fifo32 *fifo, unsigned char data) {
    timer->fifo = fifo;
    timer->data = data;
    return;
}

void timer_settime(struct _timer *timer, unsigned int timeout) {
    int e;
    struct _timer *t, *s;
    /* Register timer in timerctl */
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    e = io_load_eflags();
    io_cli();
    timerctl.using++;
    /* Find *timer slot */
    /* The only one */
    if (timerctl.using == 1) { 
        timerctl.timersHead = timer;
        timer->next = 0;
        timerctl.nextto = timer->timeout;
        io_store_eflags(e);
        return;
    }
    /* Insert at the beginning */
    t = timerctl.timersHead;
    if (timer->timeout <= t->timeout) {
        timerctl.timersHead = timer;
        timer->next = t;
        timerctl.nextto = timer->timeout;
        io_store_eflags(e);
        return;
    }
    /* Insert between s and t*/
    while(1) {
        s = t;
        t = t->next;
        if (t == 0) break; /* End of the list */

        if (timer->timeout <= t->timeout) {
            s->next = timer;
            timer->next = t;
            io_store_eflags(e);
            return;
        }
    }
    /* Insert at the end */
    s->next = timer;
    timer->next = 0;
    io_store_eflags(e);
    return;
}

void inthandler20(int *esp) {
    int i;
    struct _timer *curr;
    /* Indicates PIC that IRQ0 is handled */
    io_out8(PIC0_OCW2, 0x60);
    /* Increment Timer */
    timerctl.count++;
    if (timerctl.nextto > timerctl.count) {
        return; /* Interleave if no deadline */
    }
    curr = timerctl.timersHead;
    /* Traverse timer in "using" array */
    for (i = 0; i < timerctl.using; i++) {
        if (curr->timeout > timerctl.count) {
            break; /* Interleave if timeout does not meet */
        }
        /* Handle Timeout */
        curr->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(curr->fifo, curr->data);
        curr = curr->next;
    }
    /* Shift out timeouted timer */
    timerctl.using -= i;
    timerctl.timersHead = curr;
    /* Mark the first timeout in the queue as next */
    if (timerctl.using > 0) {
        timerctl.nextto = timerctl.timersHead->timeout;
    } else {
        timerctl.nextto = 0xffffffff;
    }
    return;
}


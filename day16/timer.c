#include "timer.h"
#include "int.h"
#include "task.h"
#include "bootpack.h"

extern struct _timer *task_timer;
struct _timerctl timerctl;
struct _fifo32 timerfifo;

void init_pit(void) {
    int i;
    struct _timer *scout;
    /* Enable PIT */
    io_out8(PIT_CTRL, 0x34);
    /* Set timer interval 0x2e9c = 11932 cycles/irq */
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    /* Initialise Timer */
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = TIMER_FLAGS_UNUSED;
    }
    /* Initialise scout (always stands at the end) */
    scout = timer_alloc();
    scout->timeout = 0xffffffff;
    scout->flags = TIMER_FLAGS_USING;
    scout->next = 0;
    /* Initialise Timer Controller Variables */
    timerctl.count = 0;
    timerctl.nextto = 0xffffffff;
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
    /* Find *timer slot */
    t = timerctl.timersHead;
    /* Insert at the beginning */
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
        if (timer->timeout <= t->timeout) {
            s->next = timer;
            timer->next = t;
            io_store_eflags(e);
            return;
        }
    }
}

void inthandler20(int *esp) {
    char ts = 0 /* Task switch */;
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
    while (1) {
        if (curr->timeout > timerctl.count) {
            break; /* Interleave if timeout does not meet */
        }
        /* Handle Timeout */
        curr->flags = TIMER_FLAGS_ALLOC;
        if (curr != task_timer) {
            /* Normal Timeout */
            fifo32_put(curr->fifo, curr->data);
        } else {
            /* Timeout from task switch */
            ts = 1;
        }
        curr = curr->next;
    }
    /* Shift out timeouted timer */
    timerctl.timersHead = curr;
    /* Mark the first timeout in the queue as next */
    timerctl.nextto = timerctl.timersHead->timeout;
    if (ts != 0) {
        task_switch();
    }
    return;
}


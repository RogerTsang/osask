#ifndef TIMER_H
#define TIMER_H

#include "fifo.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_FLAGS_UNUSED 0
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

#define TIMER_BUFSIZE 8
#define MAX_TIMER 500

struct _timer {
    /* Each Timer has different interval */
    unsigned int timeout;
    unsigned int flags;
    /* Each Timer has its own queue */
    struct _fifo8 *fifo;
    /* Each Timer can signal different data */
    unsigned char data;
};

struct _timerctl {
    /* Current Time */
    unsigned int count;
    struct _timer timer[MAX_TIMER];
};

void init_pit(void);

void inthandler20(int *esp);
struct _timer *timer_alloc(void);
void timer_free(struct _timer *timer);
void timer_init(struct _timer *timer, struct _fifo8 *fifo, unsigned char data);
void timer_settime(struct _timer *timer, unsigned int timeout);

#endif

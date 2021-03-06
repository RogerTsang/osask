#ifndef TIMER_H
#define TIMER_H

#include "fifo.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_FLAGS_UNUSED 0
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

#define TDATA_TS 2
#define TDATA_PUT 100
#define TDATA_10SEC 10
#define TDATA_3SEC  3
#define TDATA_CURSOR_H 1
#define TDATA_CURSOR_L 0
#define TIMER_TS 2 /* Task switch in 10ms */

#define TIMER_BUFSIZE 8
#define MAX_TIMER 500

struct _timer {
    /* Use LinkedList to store next deadline */
    struct _timer *next;
    unsigned int timeout;
    unsigned int flags;
    /* Each Timer has its own queue */
    struct _fifo32 *fifo;
    /* Each Timer can signal different data */
    unsigned char data;
};

struct _timerctl {
    /* Current Time */
    unsigned int count;
    /* Nearest furture timeout */
    unsigned int nextto;
    /* Number of timer in the queue */
    /* unsigned int using; */
    struct _timer *timersHead;
    struct _timer timers0[MAX_TIMER];
};

void init_pit(void);

void inthandler20(int *esp);
struct _timer *timer_alloc(void);
void timer_free(struct _timer *timer);
void timer_init(struct _timer *timer, struct _fifo32 *fifo, unsigned char data);
void timer_settime(struct _timer *timer, unsigned int timeout);

#endif

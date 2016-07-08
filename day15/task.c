#include "task.h"
#include "memory.h"
#include "bootpack.h"
#include "fifo.h"
#include "timer.h"
#include "dsctbl.h"

struct _tss32 tss_a, tss_b;
extern struct _memman *memman;

/* MultiTask-Timer */
struct _timer *mt_timer;
int mt_tr;

struct _taskctl *taskctl;
struct _timer *task_timer;

void init_mt(void) {
    mt_timer = timer_alloc();
    timer_settime(mt_timer, 2);
    mt_tr = 3 * 8;
    return;
}

void mt_taskswitch(void) {
    if (mt_tr == 3 * 8) {
        /* JMP task a */
        mt_tr = 4 * 8;
    } else {
        /* JMP task b */
        mt_tr = 3 * 8;
    }
    /* Reset timer for next timeout */
    timer_settime(mt_timer, 2);
    farjmp(0, mt_tr);
    return;
}

void init_tasks(void) {
    /* Task a is the initial task 
     * It is a slot for backup curent task */
    tss_a.ldtr = 0;
    /* VA 0x40000000 will be mapped as IO port */
    tss_a.iomap = 0x40000000;

    /* Task b */
    int task_b_esp;
    /* Allocate memory for task_b_stack */
    task_b_esp = memman_alloc_4k(memman, 64 * 1024);
    /* Change task_b stack_ptr points to bottom of the stack */
    task_b_esp += 64 * 1024;
    tss_b.ldtr = 0;
    tss_b.iomap = 0x40000000;
    tss_b.eip = (int) &task_b_main;
    tss_b.eflags = 0x0000202; /* bit2 is compulsory 1, bit9 is interrupt flag */
    tss_b.eax = 0;
    tss_b.ecx = 0;
    tss_b.edx = 0;
    tss_b.ebx = 0;
    tss_b.esp = task_b_esp;
    tss_b.ebp = 0;
    tss_b.esi = 0;
    tss_b.edi = 0;
    tss_b.es = 1 * 8; /* 0x1000 0000 */
    tss_b.cs = 2 * 8; /* 0x2000 0000 */
    tss_b.ss = 1 * 8;
    tss_b.ds = 1 * 8;
    tss_b.fs = 1 * 8;
    tss_b.gs = 1 * 8;
}

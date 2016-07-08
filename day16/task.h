#ifndef TASK_H
#define TASK_H

#include "memory.h"

#define TASK_RUNNING 2
#define TASK_READY   1
#define TASK_UNUSED  0

#define MAX_TASKS 10
#define TASK_GDT0 3

struct _tss32 /* task status segment */ {
    /* Task Info */
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    /* Register Bank */
    int eip /* insn pointer (PC) */, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    /* Segment Bank */
    int es, cs, ss, ds, fs, gs;
    /* Task Info */
    int ldtr, iomap;
};

struct _task {
    int sel, flags;
    struct _tss32 tss;
};

struct _taskctl {
    /* Number of running tasks */
    int running;
    /* Current running task number */
    int now;
    struct _task *tasks[MAX_TASKS];
    struct _task tasks0[MAX_TASKS];
};

struct _task *task_init(struct _memman *memman);
struct _task *task_alloc(void);
void task_run(struct _task *task);
void task_sleep(struct _task *task);
void task_switch(void);
#endif

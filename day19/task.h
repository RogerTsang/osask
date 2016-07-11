#ifndef TASK_H
#define TASK_H

#include "fifo.h"
#include "memory.h"

#define TASK_RUNNING 2
#define TASK_READY   1
#define TASK_UNUSED  0

#define MAX_TASKS 40
#define MAX_TASKPLV 8
#define MAX_TASKLEVELS 5
#define TASK_GDT0 3

#define TASK_MEMORY 4096

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
    int level, priority;
    struct _fifo32 fifo;
    struct _tss32 tss;
};

struct _tasklevel {
    /* Number of running tasks */
    int running;
    /* Current running task number */
    int now;
    struct _task *tasks[MAX_TASKPLV];
};

struct _taskctl {
    /* Current running level */
    int now_lv;
    /* Level Change Indicator */
    char lv_change;
    /* Total number of levels in taskmgr */
    struct _tasklevel level[MAX_TASKLEVELS];
    /* Primitive Task Struct */
    struct _task tasks0[MAX_TASKS];
};

struct _task *task_init(struct _memman *memman);
struct _task *task_alloc(void);
void task_run(struct _task *task, int level, int priority);
void task_sleep(struct _task *task);
void task_switch(void);
struct _task *task_now(void);
void task_add(struct _task *task);
void task_switchsub(void);
void task_remove(struct _task *task);

void task_idle(void);
#endif

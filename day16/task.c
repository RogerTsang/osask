#include "task.h"
#include "bootpack.h"
#include "fifo.h"
#include "timer.h"
#include "dsctbl.h"

/* MultiTask-Timer */
struct _taskctl *taskctl;
struct _timer *task_timer;

struct _task *task_init(struct _memman *memman) {
    int i;
    struct _task *task;
    struct _segmdes *gdt = (struct _segmdes *) ADR_GDT;
    taskctl = (struct _taskctl *) memman_alloc_4k(memman, sizeof (struct _taskctl));
    for (i = 0; i < MAX_TASKS; i++) {
        /* Register all task slots on segment descriptor table */
        taskctl->tasks0[i].flags = TASK_UNUSED;
        taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8; /* Setup selector (GDT segment) number */
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
    }
    /* Allocate new task for task_init caller */
    task = task_alloc();
    task->flags = TASK_RUNNING;
    /* Task Controller Init */
    taskctl->running = 1;
    taskctl->now = 0;
    taskctl->tasks[0] = task;
    /* Activate Current Task */
    load_tr(task->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, TIMER_TS);
    return task;
}

struct _task *task_alloc(void) {
    int i;
    struct _task *task = 0;
    /* Find an empty slot */
    for (i = 0; i < MAX_TASKS; i++) {
        if (taskctl->tasks0[i].flags == TASK_UNUSED) {
            task = &(taskctl->tasks0[i]);
            task->flags = TASK_READY;
            task->tss.eflags = 0x00000202; /* Turn on interrupt */
            task->tss.eax = 0;
            task->tss.ecx = 0;
            task->tss.ebx = 0;
            task->tss.edx = 0;
            task->tss.ebp = 0;
            task->tss.esi = 0;
            task->tss.edi = 0;
            task->tss.es  = 0;
            task->tss.ds  = 0;
            task->tss.fs  = 0;
            task->tss.gs  = 0;
            task->tss.ldtr = 0;
            task->tss.iomap = 0x40000000;
            break;
        }
    }
    return task;
}

void task_run(struct _task *task) {
    task->flags = TASK_RUNNING;
    taskctl->tasks[taskctl->running] = task;
    taskctl->running++;
    return;
}

void task_sleep(struct _task *task) {
    int i;
    char ts = 0;
    if (task->flags == TASK_RUNNING) {
        /* If current task is running */
        if (task == taskctl->tasks[taskctl->now]) {
            ts = 1; /* Indicate Task Switch */
        }

        for (i = 0; i < taskctl->running; i++) {
            /* Find current task in the list */
            if (taskctl->tasks[i] == task) {
                break;
            }
        }

        taskctl->running--;
        /* In the task list
         * [0] [1] [2] [3] [4] [5] [6]
         *      i          now
         * You need to shift now to left
         * [0] [X] [1] [2] [3] [4] [5]
         *      i          now
         */
        if (i < taskctl->now) {
            taskctl->now--;
        }
        /* For the final element, 
         * i+1 should be equal to taskctl->running
         * which causes buffer overflow
         */
        for (; i < taskctl->running; i++) {
            taskctl->tasks[i] = taskctl->tasks[i+1];
        }
        task->flags = TASK_READY;
        if (ts != 0) {
            if (taskctl->now >= taskctl->running) {
                taskctl->now = 0;
            }
            farjmp(0, taskctl->tasks[taskctl->now]->sel);
        }
    }
    return;
}

void task_switch(void) {
    timer_settime(task_timer, TIMER_TS);
    /* Only Enable task switching when there are multitasks */
    if (taskctl->running >= 2) {
        /* Loop Through all tasks */
        taskctl->now++;
        if (taskctl->now == taskctl->running) {
            taskctl->now = 0;
        }
        farjmp(0, taskctl->tasks[taskctl->now]->sel);
    }
    return;
}

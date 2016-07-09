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
    taskctl->now_lv = 0;
    taskctl->lv_change = 0;
    for (i = 0; i < MAX_TASKS; i++) {
        /* Register all task slots on segment descriptor table */
        taskctl->tasks0[i].flags = TASK_UNUSED;
        taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8; /* Setup selector (GDT segment) number */
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
    }
    for (i = 0; i < MAX_TASKLEVELS; i++) {
        taskctl->level[i].running = 0;
        taskctl->level[i].now = 0;
    }
    /* Allocate new task for task_init caller */
    task = task_alloc();
    task->flags = TASK_RUNNING;
    /* Priority and level */
    task->priority = 2; /* Timeslice = 10ms * pri */
    task->level = 0; /* Highest Level */
    /* Add task to level control */
    task_add(task);
    /* Recalebrate Running Level */
    task_switchsub();
    /* Activate Current Task */
    load_tr(task->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, task->priority);
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
            task->priority = 2;
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

struct _task *task_now(void) {
    struct _tasklevel *tl = &taskctl->level[taskctl->now_lv];
    return tl->tasks[tl->now];
}

void task_add(struct _task *task) {
    struct _tasklevel *tl = &taskctl->level[task->level];
    /* Level0 [0] [1] [2] [3] [4] [5] [6] [ ]
     * Level1 [0] [1] [2] [ ] [ ] [ ] [ ] [ ]
     * Level2 [0] [1] [2] [3] [4] [X] [ ] [ ]
     *                             tl2->running = 5
     */
    tl->tasks[tl->running] = task;
    tl->running++; /* if running >= MAX_TASKPLV, overflow */
    if (tl->running >= MAX_TASKPLV) {
        tl->running = MAX_TASKPLV - 1;
    }
    task->flags = TASK_RUNNING;
    return;
}

void task_remove(struct _task *task) {
    int i;
    struct _tasklevel *tl = &taskctl->level[task->level];

    /* Search for task */
    for (i = 0; i < tl->running; i++) {
        if (tl->tasks[i] == task) {
            break;
        }
    }
    
    /* Change current running task index */
    tl->running--;
    if (i < tl->now) {
        tl->now--;
    }
    /* Running task out of range */
    if (tl->now >= tl->running) {
        tl->now = 0;
    }

    /* Remove and shift array */
    task->flags = TASK_READY;
    for (; i < tl->running; i++) {
        tl->tasks[i] = tl->tasks[i+1];
    }

    return;
}

void task_switchsub(void) {
    int i;
    /* Search for highest level */
    for (i = 0; i < MAX_TASKLEVELS; i++) {
        if (taskctl->level[i].running > 0) {
            break;
        }
    }
    /* Register highest level in task controller */
    taskctl->now_lv = i;
    /* Reset lv_change indicator */
    taskctl->lv_change = 0;
    return;
}

void task_run(struct _task *task, int level, int priority) {
    if (level < 0) {
        level = task->level;
    }
    if (priority > 0) {
        task->priority = priority;
    }

    /* Switch Level Case 1: detach task from a running level */
    if (task->flags == TASK_RUNNING && task->level != level) {
        /* Set task state back to READY */
        task_remove(task);
    }
    /* Switch Level Case 2: mount READY task to a running level */
    if (task->flags != TASK_RUNNING) {
        task->level = level;
        task_add(task);
    }
    taskctl->lv_change = 1;
    return;
}

void task_sleep(struct _task *task) {
    struct _task *now_task;
    if (task->flags == TASK_RUNNING) {
        /* If current task is running */
        now_task = task_now();
        /* Set 'task' back to READY state */
        task_remove(task);
        /* Sleep yourself */
        if (task == now_task) {
            /* Recalebrate Level */
            task_switchsub();
            /* Switch Current Task */
            now_task = task_now();
            /* JMP */
            farjmp(0, now_task->sel);
        }
    }
    return;
}

void task_switch(void) {
    struct _tasklevel *tl = &taskctl->level[taskctl->now_lv];
    struct _task *new_task, *now_task = tl->tasks[tl->now];
    /* Switch to current level:next task */
    tl->now++;
    /* Reset to beginning when proceed to the end */
    if (tl->now == tl->running) {
        tl->now = 0;
    }
    /* If it requires level switching */
    if (taskctl->lv_change != 0) {
        task_switchsub();
        tl = &taskctl->level[taskctl->now_lv];
    }
    new_task = tl->tasks[tl->now];
    /* Timeslice allocation depends on its priority */
    timer_settime(task_timer, new_task->priority);
    /* Only do JMP when there is different task */
    if (new_task != now_task) {
        farjmp(0, new_task->sel);
    }
    return;
}

#ifndef TASK_H
#define TASK_H

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

void init_tasks(void);
void init_mt(void);
void mt_taskswitch(void);
#endif

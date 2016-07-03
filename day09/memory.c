#include "memory.h"
#include "bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0, i;
    
    /* Check CPU i386 or i486 w/ AC-bit */
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /* Set AC-bit */
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    /* If CPU = i386, AC flag will be cleared */
    if ((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; /* Clear AC-bit */
    io_store_eflags(eflg);

    if (flg486 != 0) {
        /* i486 CPU has cache */
        /* Disable cache through rewriting CR0 reg */
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486 != 0) {
        /* Re-enable cache after memory test */ 
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    return i;
}

/*
unsigned int memtest_sub(unsigned int start, unsigned int end) {
    unsigned int i, *p, old;
    unsigned int pat0 = 0xaa55aa55;
    unsigned int pat1 = 0x55aa55aa;

    for (i = start; i <= end; i += 0x1000) {
        p = (unsigned int *) (i + 0xffc);
        old = *p;
        *p = pat0;
        *p ^= 0xffffffff;
        if (*p != pat1) {
            *p = old;
            break;
        }

        *p ^= 0xffffffff;
        if (*p != pat0) {
            *p = old;
            break;
        }
        *p = old;
    }
    return i;
}
*/

void memman_init(struct _memman *man) {
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
    return;
}

unsigned int memman_total(struct _memman *man) {
    unsigned int i, t;
    for (i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct _memman *man, unsigned int size) {
    unsigned int i, a;
    /* First Fit Scheme */
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].size >= size) {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            /* Remove Free Record if remain mem == 0 */
            if (man->free[i].size == 0) {
                man->frees--;
                for (; i < man->frees; i++) {
                    /* Fill record in current empty slot */
                    man->free[i] = man->free[i+1];
                }
            }
            return a;
        }
    }
    return 0;
}

int memman_free(struct _memman *man, unsigned int addr, unsigned int size) {
    int i, j;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) {
            break; /* Find the next memory region */
        }
    }

    /* Merge Free Memory (has previous memory block) */
    if (i > 0) {
        /* Can be fully Merged with previous block */
        if (man->free[i-1].addr + man->free[i-1].size == addr) {
            man->free[i-1].size += size;
            
            /* Merge Free Memory (has next memory block) */
            if (i < man->frees) {
                if (addr+size == man->free[i].addr) {
                    man->free[i-1].size += man->free[i].size;
                    man->frees--;
                    for (; i < man->frees; i++) {
                        man->free[i] = man->free[i+1];
                    }
                }
            }
            return 0;
        }
    }


    /* Merge with next block */
    if (i < man->frees) {
        if (addr+size == man->free[i].addr) {
            man->free[i].size += size;
            man->free[i].addr = addr;
            return 0;
        }
    }

    /* Create a new block */
    if (man->frees < MEMMAN_FREES) {
        /* Spare a slot at place i */
        for (j = man->frees; j > i; j--) {
            man->free[j] = man->free[j-1];
        }
        man->frees++;

        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }

    /* Lost */
    man->losts++;
    man->lostsize += size;
    return -1;
}

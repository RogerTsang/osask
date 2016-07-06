#ifndef MEMORY_H
#define MEMORY_H

#define MEMMAN_ADDR  0x003c000
#define MEMMAN_FREES 4096

/* Memory Manager Struct Size 4 * 4 + (8 * 4096) = 32784 */
struct _freeinfo {
    unsigned int addr;
    unsigned int size;
};

struct _memman {
    int frees;
    int maxfrees;
    int lostsize;
    int losts;
    struct _freeinfo free[MEMMAN_FREES];
};

/* Memory Test Functions */
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

/* Memory Manage Functions */
void memman_init(struct _memman *man);
unsigned int memman_total(struct _memman *man);
unsigned int memman_alloc(struct _memman *man, unsigned int size);
int memman_free(struct _memman *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct _memman *man, unsigned int size);
int memman_free_4k(struct _memman *man, unsigned int addr, unsigned int size);

#endif

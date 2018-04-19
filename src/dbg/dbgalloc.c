/*
 * dbgalloc.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include <config.h>

#ifndef NDEBUG

#include <spinlock.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <assert.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static spinlock_t __dbg_alloc_spinlock = 0L;

#define DBG_MEM_BLOCK_MAGIC 1975

typedef struct __dbg_block {
    uint32_t magic;
    struct __dbg_block *prev;
    struct __dbg_block *next;
    size_t size;
    const char *file;
    int line;
} * dbg_block_t;

static dbg_block_t __dbg_block_first = NULL;
static dbg_block_t __dbg_block_last = NULL;
static size_t __dbg_block_size = 0;

void *dbg_malloc(size_t size, const char *file, int line) {
    assert(size);
    size_t alloc_size = sizeof(struct __dbg_block) + size;
    dbg_block_t block = malloc(alloc_size);
    block->magic = DBG_MEM_BLOCK_MAGIC;
    block->size = size;
    block->file = file;
    block->line = line;
    block->prev = NULL;

    spin_lock(&__dbg_alloc_spinlock);
    block->next = __dbg_block_first;
    if (__dbg_block_first)
        __dbg_block_first->prev = block;
    __dbg_block_first = block;
    if (!__dbg_block_last)
        __dbg_block_last = block;
    __dbg_block_size++;
    spin_unlock(&__dbg_alloc_spinlock);

    return (block + 1);
}
void *dbg_calloc(size_t nmemb, size_t size, const char *file, int line) {
    void *data = dbg_malloc(nmemb * size, file, line);
    memset(data, 0, nmemb * size);
    return data;
}
void dbg_free(void *ptr, const char *file, int line) {
    assert(ptr);
    dbg_block_t block = (dbg_block_t)((dbg_block_t) ptr - 1);
    if (block->magic == DBG_MEM_BLOCK_MAGIC) {
        block->magic = -1;

        spin_lock(&__dbg_alloc_spinlock);
        if (block->prev)
            block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;
        if (block == __dbg_block_first)
            __dbg_block_first = block->next;
        if (block == __dbg_block_last)
            __dbg_block_last = block->prev;
        __dbg_block_size--;
        spin_unlock(&__dbg_alloc_spinlock);

        ptr = block;
    }
    free(ptr);
}

void dbg_leaks_report(FILE *file) {
    assert(file);

    spin_lock(&__dbg_alloc_spinlock);
    dbg_block_t block = __dbg_block_first;
    fprintf(file, "---- LEAK REPORT BEGIN ----\n");
    while (block) {
        fprintf(file, "---- leak address:%p, size:%lu, file:%s (%d)\n", block + 1, (unsigned long) block->size, block->file, block->line);
        block = block->next;
    }
    fprintf(file, "---- LEAK REPORT END ----\n");
    spin_unlock(&__dbg_alloc_spinlock);
}

#endif // !NDEBUG

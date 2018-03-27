/*
 * co_proc.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include "pch.h"
#include <string.h>
#include <unistd.h>

#define LOG_TAG "PROC"

co_proc_t co_proc_create(co_sched_t sched) {
    LENTRY("()");
    assert(sched);
    co_proc_t proc = calloc(1, sizeof(struct __co_proc));
    proc->sched = sched;
    LEXIT("(%p)", proc);
    return proc;
}
void co_proc_destroy(co_proc_t proc) {
    // destroy all running tasks.
    while (proc->first_task) {
        co_task_t task = proc->first_task;
        proc->first_task = task->next;
        co_task_destroy(task);
    }
    free(proc);
}

int co_proc_switch_task(co_proc_t proc, co_task_t task) {
    LENTRY("(proc:%p,task:%p)", proc, task);
    assert(proc);
    assert(task);

    // add to runnable queue.
    task->prev = proc->last_task;
    task->next = NULL;
    if (task->prev)
        task->prev->next = task;
    proc->last_task = task;
    if (!proc->first_task) {
        proc->first_task = task;
    }
    proc->task_count++;
    LEXIT("(%d)", 0);
    return 0;
}

co_task_t co_proc_pop_task(co_proc_t proc) {
    co_task_t task;
    LENTRY("(proc:%p)", proc);
    assert(proc);
    task = proc->first_task;
    if (task) {
        proc->first_task = task->next;
        if (task->next)
            task->next->prev = NULL;
        if (task == proc->last_task)
            proc->last_task = NULL;

        assert(!task->prev);
        task->next = NULL;
    }
    LEXIT("(%p)", task);
    return task;
}

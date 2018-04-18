/*
 * co_task.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include <pch.h>

#define LOG_TAG "TASK"

static void co_task_cb(void *arg) {
    co_task_t task = (co_task_t) arg;
    task->start_routine(task->arg);
    task->state = CO_TASK_STATE_DONE;
    co_yield();
}

co_task_t co_task_create(co_sched_t sched, size_t stack_size, void (*start_routine)(void *), void *arg) {
    LENTRY("(sched:%p, stack_size:%u, start_routine:%p, arg:%p)", sched, stack_size, start_routine, arg);

    assert(sched);
    assert(stack_size >= 0);
    assert(start_routine);

    // adjust stack size.
    if (stack_size == 0)
        stack_size = 64 * 1024; // default 64k
    if (stack_size & 0xff) {
        stack_size &= ~0xff;
        stack_size += 0x100; // aligned to 256
    }
    // create the task.
    co_task_t task = calloc(1, sizeof(struct __co_task));
    task->sched = sched;
    task->state = CO_TASK_STATE_INITIAL;
    task->start_routine = start_routine;
    task->arg = arg;
    // creates platform dependent context
    task->ctx = co_context_create(stack_size, co_task_cb, task);

    LEXIT("(%p)", task);
    return task;
}

void co_task_destroy(co_task_t task) {
    LENTRY("(task:%p)", task);

    assert(task);
    assert(!task->next && !task->prev);
    co_context_destroy(task->ctx);
    free(task);

    LEXIT("()");
}

int co_task_swapin(co_task_t task) {
    int retval;
    LENTRY("(task:%p)", task);
    assert(task);
    retval = co_context_swapin(task->ctx);
    LEXIT("(%d)", retval);
    return retval;
}

int co_task_swapout(co_task_t task) {
    int retval;
    LENTRY("(task:%p)", task);
    assert(task);
    retval = co_context_swapout(task->ctx);
    LEXIT("(%d)", retval);
    return retval;
}

co_task_t co_task_self() {
    co_sched_t sched = co_sched_self();
    return sched ? sched->current_task : NULL;
}

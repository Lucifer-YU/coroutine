/*
 * co_task.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#ifndef __COROUTINE_TASK_H__
#define __COROUTINE_TASK_H__

#include <co.h>
#include <co_context.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CO_TASK_STATE_INITIAL 0
#define CO_TASK_STATE_RUNNABLE 1
#define CO_TASK_STATE_SLEEP 2
#define CO_TASK_STATE_DONE 3
#define CO_TASK_STATE_FATAL 4

struct __co_task {
    co_sched_t sched;
    co_context_t ctx;
    void (*start_routine)(void *);
    void *arg;
    int state;
    // sleep
    uint32_t sleep_msec;
    // doubly linked node
    struct __co_task *prev;
    struct __co_task *next;
};

co_task_t co_task_create(co_sched_t sched, size_t stack_size, void (*start_routine)(void *), void *arg);
void co_task_destroy(co_task_t task);

// already defined in co.h -- co_task_t co_task_self();
int co_task_swapin(co_task_t task);
int co_task_swapout(co_task_t task);

#ifdef __cplusplus
}
#endif

#endif // __COROUTINE_TASK_H__

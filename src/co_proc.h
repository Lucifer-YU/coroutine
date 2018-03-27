/*
 * co_proc.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#ifndef __CO_PROC_H__
#define __CO_PROC_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __co_task* co_task_t;
typedef struct __co_sched* co_sched_t;

typedef struct __co_proc {
    co_sched_t sched;
    // task queue
    size_t task_count;
    co_task_t first_task;
    co_task_t last_task;

}* co_proc_t;

co_proc_t co_proc_create(co_sched_t sched);
void co_proc_destroy(co_proc_t proc);

int co_proc_switch_task(co_proc_t proc, co_task_t task);
co_task_t co_proc_pop_task(co_proc_t proc);

#ifdef __cplusplus
}
#endif

#endif  // __CO_PROC_H__

/*
 * co_sleep.h
 *
 *  Created on: Feb 1, 2018
 *      Author: Lucifer
 */

#ifndef SRC_CO_SLEEP_H_
#define SRC_CO_SLEEP_H_

#include <co_sched.h>
#include <co_task.h>
#include <co_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __co_sleep {
    co_sched_t sched;
    co_timer_mgr_t timer_mgr;
    // Doubly linked list
    co_task_t first_task;
    co_task_t last_task;
    size_t task_count;
} * co_sleep_t;

co_sleep_t co_sleep_create(co_sched_t sched);
void co_sleep_destroy(co_sleep_t sleep);

int co_sleep_switch_task(co_sleep_t sleep, co_task_t task);
int co_sleep_tick(co_sleep_t sleep, int *next_delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* SRC_CO_SLEEP_H_ */

/*
 * co_sched.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#ifndef __COROUTINE_SCHED_H__
#define __COROUTINE_SCHED_H__

#include <co.h>
#include <co_sleep.h>
#include <co_mux.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CO_RF_TASKS    0x01
#define CO_RF_TIMERS   0x02
#define CO_RF_SLEEPS   0x04
#define CO_RF_IDLE_CPU 0x08
// ...
#define CO_RF_ALL      0x7f

struct __co_sched {
    co_task_t current_task;
    size_t runnable_count;

    co_proc_t proc_mgr;
    co_sleep_t sleep_mgr;
    co_mux_t iomux_mgr;
};

int co_sched_resume_task(co_sched_t sched, co_task_t task);

// already defined in co.h -- co_sched_t co_sched_self();
int co_sched_yield(co_sched_t sched);
int co_sched_sleep(co_sched_t sched, uint32_t msec);

// internal helpers
co_mux_t co_sched_get_iomux_mgr(co_sched_t sched, int force);

#ifdef __cplusplus
}
#endif

#endif // __COROUTINE_SCHED_H__

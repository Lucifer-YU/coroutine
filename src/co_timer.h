/*
 * co_timer.h
 *
 *  Created on: Feb 1, 2018
 *      Author: Lucifer
 */

#ifndef SRC_CO_TIMER_H_
#define SRC_CO_TIMER_H_

#include <stddef.h>
#include <stdint.h>

struct __co_timer_mgr;

typedef struct __co_timer {
    int64_t time_point;
    void (*pfn)(void *, int);
    void *arg;
    int completed;
    int canceled;
    struct __co_timer *next;
} * co_timer_t;

void co_timer_destroy(co_timer_t timer);

int co_timer_cancel(co_timer_t timer);
int co_timer_run(co_timer_t timer);

typedef struct __co_timer_deadline {
    int64_t time_point;
    co_timer_t timers;
    struct __co_timer_deadline *next;
} * co_timer_deadline_t;

typedef struct __co_timer_mgr {
    int64_t next_trigger_time;
    co_timer_deadline_t deadlines;
} * co_timer_mgr_t;

co_timer_mgr_t co_timer_mgr_create();
void co_timer_mgr_destroy(co_timer_mgr_t tm);

co_timer_t co_timer_mgr_expire_at(co_timer_mgr_t tm, int duration, void (*pfn)(void *, int), void *arg);
co_timer_t co_timer_mgr_pop_expired(co_timer_mgr_t tm, size_t n);
int co_timer_mgr_get_next_delay(co_timer_mgr_t tm);

int co_timer_mgr_cancel_timer(co_timer_mgr_t tm, co_timer_t t);


#endif /* SRC_CO_TIMER_H_ */

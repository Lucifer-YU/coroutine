/*
 * co_sleep.c
 *
 *  Created on: Feb 1, 2018
 *      Author: Lucifer
 */

#include <pch.h>

#define LOG_TAG "SLEEP"

co_sleep_t co_sleep_create(co_sched_t sched) {
    LENTRY("(sched:%p)", sched);
    assert(sched);
    co_sleep_t sw = calloc(1, sizeof(struct __co_sleep));
    sw->sched = sched;
    sw->timer_mgr = co_timer_mgr_create();
    /// sw->timer_mgr
    LEXIT("(%p)", sw);
    return sw;
}
void co_sleep_destroy(co_sleep_t sleep) {
    LENTRY("(sleep:%p)", sleep);
    assert(sleep);
    assert(!sleep->task_count);
    co_timer_mgr_destroy(sleep->timer_mgr);
    free(sleep);
    LEXIT("()");
}

static void co_sleep_wakeup_task(co_sleep_t sleep, co_task_t task);
static void co_sleep_expire_cb(void *arg, int canceled);

int co_sleep_switch_task(co_sleep_t sleep, co_task_t task) {
    LENTRY("(sleep:%p,task:%p)", sleep, task);
    assert(sleep);
    assert(task);

    // add to task queue.
    task->prev = sleep->last_task;
    task->next = NULL;
    if (sleep->last_task)
        sleep->last_task->next = task;
    sleep->last_task = task;
    if (!sleep->first_task)
        sleep->first_task = task;
    sleep->task_count++;

    // register to timer.
    void **args = (void **) malloc(sizeof(void *) * 2); // will be deleted in 'co_sleep_expire_cb'
    args[0] = sleep;
    args[1] = task;
    co_timer_mgr_expire_at(sleep->timer_mgr, task->sleep_msec, co_sleep_expire_cb, args);

    LEXIT("()");
    return 0;
}

int co_sleep_tick(co_sleep_t sleep, int *next_delay_ms) {
    int retval = 0;
    LENTRY("(sleep:%p,next_delay_ms:%d)", sleep, *next_delay_ms);

    co_timer_t timer_cur, timer_next;
    for (;;) {
        timer_cur = co_timer_mgr_pop_expired(sleep->timer_mgr, 256);
        if (!timer_cur)
            break;
        *next_delay_ms = co_timer_mgr_get_next_delay(sleep->timer_mgr);
        while (timer_cur) {
            LOGI("entering timer(%p) callback.", timer_cur);
            co_timer_run(timer_cur);
            LOGI("leaving timer(%p) callback.", timer_cur);
            timer_next = timer_cur->next;
            co_timer_destroy(timer_cur);
            timer_cur = timer_next;
            retval++;
        }
    }
    LEXIT("(%d)", retval);
    return retval;
}

static void co_sleep_wakeup_task(co_sleep_t sleep, co_task_t task) {
    LENTRY("(sleep:%p,task:%p)", sleep, task);
    assert(sleep);
    assert(task);

#ifndef NDEBUG
    // validate the given task exists in the queue.
    co_task_t t = sleep->first_task;
    while (t) {
        if (t == task) break;
        t = t->next;
    }
    if (t == NULL) {
        printf("what's going on?");
    }
    assert(t);
#endif // NDEBUG

    // remove it from the task queue.
    if (task->prev)
        task->prev->next = task->next;
    else
        sleep->first_task = task->next;
    if (task->next)
        task->next->prev = task->prev;
    else
        sleep->last_task = task->prev;
    sleep->task_count--;

    // put it back to scheduler's runnable queue.
    task->next = task->prev = NULL;
    co_sched_resume_task(sleep->sched, task);

    LEXIT("()");
}

static void co_sleep_expire_cb(void *arg, int canceled) {
    LENTRY("(arg:%p,canceled:%d)", arg, canceled);
    void **args = (void **) arg;
    co_sleep_t sleep = (co_sleep_t) args[0];
    co_task_t task = (co_task_t) args[1];
    free(arg);
    if (!canceled)
        co_sleep_wakeup_task(sleep, task);
    LEXIT("()");
}

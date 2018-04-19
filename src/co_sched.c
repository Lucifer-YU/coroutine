/*
 * co_sched.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include "pch.h"
#include <string.h>
#include <unistd.h>

#define LOG_TAG "SCHED"

static __thread co_sched_t __co_sched_ct = NULL;

static int co_sched_run(co_sched_t sched, int flags);

co_sched_t co_sched_create() {
    LENTRY("()");
    co_sched_t sched = calloc(1, sizeof(struct __co_sched));
    LEXIT("(%p)", sched);
    return sched;
}

void co_sched_destroy(co_sched_t sched) {
    LENTRY("(sched:%p)", sched);
    assert(sched);
    if (sched->proc_mgr) {
        co_proc_destroy(sched->proc_mgr);
    }
    if (sched->sleep_mgr) {
        // TODO cancel & destroy all sleeping tasks.
        co_sleep_destroy(sched->sleep_mgr);
    }
    if (sched->iomux_mgr) {
        // TODO awaken all blocking I/O operations.
        co_mux_destroy(sched->iomux_mgr);
    }
    free(sched);

    LEXIT("()");
}

int co_sched_create_task(co_sched_t sched, size_t stack_size, void (*start_routine)(void *), void *arg) {
    assert(sched);
    // create new task
    co_task_t task = co_task_create(sched, stack_size, start_routine, arg);

    // add to scheduler
    int retval = co_sched_resume_task(sched, task);
    // update runnable count.
    sched->runnable_count++;

    return retval;
}

int co_sched_yield(co_sched_t sched) {
    int retval = ENOTSUP;
    LENTRY("(sched:%p)", sched);
    if (sched->current_task) {
        retval = co_task_swapout(sched->current_task);
        if (0 != retval) {
            LOGE("swapcontext() error:%s", strerror(errno));
        }
    }
    LEXIT("(%d)", retval);
    return retval;
}

int co_sched_sleep(co_sched_t sched, uint32_t msec) {
    int retval = -1;
    LENTRY("(sched:%p, timeout)", sched, msec);
    assert(sched);
    if (msec) {
        co_task_t task = sched->current_task;
        if (task) {
            task->sleep_msec = msec;
            task->state = CO_TASK_STATE_SLEEP;
            LOGI("task(%p) will sleep %u ms.", task, msec);
        }
    }
    retval = co_sched_yield(sched);
    LEXIT("(%d)", retval);
    return retval;
}

int co_sched_runloop(co_sched_t sched) {
    int retval = 0;
    LENTRY("(sched:%p)", sched);
    if (co_task_self()) {
        LOGW("cannot call runloop within coroutine task.");
    } else {
        while (sched->runnable_count) {
            co_sched_run(sched, CO_RF_ALL);
        }
    }

    LEXIT("(%d)", retval);
    return retval;
}

int co_sched_resume_task(co_sched_t sched, co_task_t task) {
    LENTRY("(sched:%p,task:%p)", sched, task);
    assert(sched);
    assert(task);

    /// put the task to processor.
    task->state = CO_TASK_STATE_RUNNABLE;
    if (!sched->proc_mgr) {
        sched->proc_mgr = co_proc_create(sched);
    }
    co_proc_switch_task(sched->proc_mgr, task);

    LEXIT("(%d)", 0);
    return 0;
}

co_sched_t co_sched_self() {
    LENTRY("()");
    LEXIT("(%p)", __co_sched_ct);
    return __co_sched_ct;
}

static int co_sched_do_runnables(co_sched_t sched) {
    int run_count = 0;
    int done_count = 0;
    LENTRY("(sched:%p)", sched);
    assert(sched);
    co_context_scope_enter();

    LOGI("tasks remaining:%u", sched->runnable_count);
    run_count = 0;
    for (;;) {
        if (run_count >= (int) sched->runnable_count)
            break;
        // dequeue task.
        co_task_t task = co_proc_pop_task(sched->proc_mgr);
        if (!task)
            break;

        run_count++;
        // running task
        sched->current_task = task;
        LOGI("entering task:%p", task);
        if (0 != co_task_swapin(task)) {
            LOGE("swap context error:%s", strerror(errno));
            run_count = -1;
            break;
        }
        LOGI("leaving task:%p, state:%d", task, task->state);
        sched->current_task = NULL;

        // update task
        switch (task->state) {
            case CO_TASK_STATE_RUNNABLE:
                // the task still running, put it back.
                assert (sched->proc_mgr);
                co_proc_switch_task(sched->proc_mgr, task);
                break;
            case CO_TASK_STATE_SLEEP:
                // goto sleep.
                if (!sched->sleep_mgr) {
                    // create sleep_mgr if required.
                    sched->sleep_mgr = co_sleep_create(sched);
                }
                co_sleep_switch_task(sched->sleep_mgr, task);
                break;

            // XXX
            case CO_TASK_STATE_DONE:
            default:
                done_count++;
                LOGI("task(%p) done", task);
                co_task_destroy(task);
                break;
        }
    }
    sched->runnable_count -= done_count;

    co_context_scope_leave();

    LEXIT("(%d)", run_count);
    return run_count;
}
static int co_sched_do_sleeps(co_sched_t sched, int *next_delay_ms) {
    int retval = 0;
    assert(sched);
    if (sched->sleep_mgr) {
        retval = co_sleep_tick(sched->sleep_mgr, next_delay_ms);
    }
    return retval;
}
static int co_sched_run(co_sched_t sched, int flags) {
    LENTRY("(sched:%p, flags:%d)", sched, flags);
    assert(sched);
    assert(!co_task_self());

    // set thread local scheduler.
    __co_sched_ct = sched;

    int runnable_count = 0;
    if (flags & CO_RF_TASKS) {
        runnable_count = co_sched_do_runnables(sched);
    }
    if (flags & CO_RF_TIMERS) {
        // TODO co_sched_run_timers(sched);
    }
    int sleep_count = 0;
    int sleep_next_delay_ms = 15; // default 15 ms.
    if (flags & CO_RF_SLEEPS) {
        sleep_count = co_sched_do_sleeps(sched, &sleep_next_delay_ms);
    }

    if (flags & CO_RF_IDLE_CPU) {
        // free CPU time if possible.
        if (!runnable_count && !sleep_count) {
            int sleep_ms = sleep_next_delay_ms < 15 ? sleep_next_delay_ms : 15;
            usleep(sleep_ms * 1000);
        }
    }

    // clear thread local scheduler.
    __co_sched_ct = NULL;

    LEXIT("(%d)", runnable_count);
    return runnable_count;
}

co_mux_t co_sched_get_iomux_mgr(co_sched_t sched, int force) {
    LENTRY("(sched:%p, force:%d)", sched, force);
    assert(sched);
    co_mux_t poller = sched->iomux_mgr;
    if (!poller && force) {
        // create the poller if necessary.
        poller = sched->iomux_mgr = co_mux_create();
    }
    LEXIT("(%p)", poller);
    return poller;
}

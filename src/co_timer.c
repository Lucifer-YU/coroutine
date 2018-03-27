/*
 * co_timer.c
 *
 *  Created on: Feb 1, 2018
 *      Author: Lucifer
 */

#include <pch.h>
#include <limits.h>
#include <sys/time.h>

#define LOG_TAG "TIMER"

co_timer_t co_timer_create(int64_t time_point, void (*pfn)(void *, int), void *arg) {
    LENTRY("(pfn:%p,arg:%p)", pfn, arg);

    assert(pfn);
    co_timer_t timer = calloc(1, sizeof(struct __co_timer));
    timer->time_point = time_point;
    timer->pfn = pfn;
    timer->arg = arg;
    LEXIT("(%p)", timer);

    return timer;
}
void co_timer_destroy(co_timer_t timer) {
    LENTRY("(timer:%p)", timer);

    assert(timer);
    free(timer);

    LEXIT("()");
}

int co_timer_cancel(co_timer_t timer) {
    int retval = -1;
    LENTRY("(timer:%p)", timer);

    assert(timer);
    if (!timer->canceled) {
        timer->canceled = 1;
        retval = co_timer_run(timer);
    }

    LEXIT("(%d)", retval);
    return retval;
}
int co_timer_run(co_timer_t timer) {
    int retval = -1;
    LENTRY("(timer:%p)", timer);

    if (!timer->completed) {
        timer->completed = 1;
        timer->pfn(timer->arg, timer->canceled);
        retval = 0;
    }

    LEXIT("(%d)", retval);
    return retval;
}

co_timer_mgr_t co_timer_mgr_create() {
    LENTRY("()");

    co_timer_mgr_t tm = calloc(1, sizeof(struct __co_timer_mgr));
    tm->next_trigger_time = LLONG_MAX;

    LEXIT("(%p)", tm);
    return tm;
}

void co_timer_mgr_destroy(co_timer_mgr_t tm) {
    LENTRY("(tm:%p)", tm);

    assert(tm);
    free(tm);

    LEXIT("()");
}

static int64_t time_since_1970() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

co_timer_t co_timer_mgr_expire_at(co_timer_mgr_t tm, int duration, void (*pfn)(void *, int), void *arg) {
    LENTRY("(tm:%p,duration:%d,pfn:%p,arg:%p)", tm, duration, pfn, arg);

    assert(tm);
    assert(pfn);
    int64_t time_point = time_since_1970() + duration;

    co_timer_t timer = co_timer_create(time_point, pfn, arg);
    if (!tm->deadlines) {
        tm->next_trigger_time = time_point;
    }
    // insert to deadlines (sorted multimap)
    co_timer_deadline_t deadline;
    co_timer_deadline_t deadline_prev = NULL;
    for (deadline = tm->deadlines; deadline; deadline = deadline->next) {
        if (deadline->time_point < time_point)
            deadline_prev = deadline;
        else {
            if (deadline->time_point > time_point)
                deadline = NULL;
            break;
        }
    }
    if (!deadline) {
        // deadline no found, create new.
        deadline = calloc(1, sizeof(struct __co_timer_deadline));
        deadline->time_point = time_point;
        if (deadline_prev) {
            // insert to sorted position.
            deadline->next = deadline_prev->next;
            deadline_prev->next = deadline;
        } else {
            // insert to head.
            deadline->next = tm->deadlines;
            tm->deadlines = deadline;
        }
    }
    timer->next = deadline->timers;
    deadline->timers = timer;

    LEXIT("(%p)", timer);
    return timer;
}

co_timer_t co_timer_mgr_pop_expired(co_timer_mgr_t tm, size_t n) {
    LENTRY("(tm:%p, n:%u)", tm, n);

    assert(tm);
    int64_t time_point = time_since_1970();
    co_timer_t timers = NULL;
    while (n && tm->deadlines && tm->deadlines->time_point < time_point) {
        co_timer_deadline_t deadline = tm->deadlines;
        while (n && deadline->timers) {
            // move it to expired list.
            co_timer_t timer = deadline->timers;
            deadline->timers = timer->next;
            timer->next = timers;
            timers = timer;
            n--;
        }
        if (!deadline->timers) {
            // free empty deadline.
            tm->deadlines = deadline->next;
            free(deadline);
        }
    }
    // update next trigger time.
    tm->next_trigger_time = tm->deadlines ? tm->deadlines->time_point : LLONG_MAX;

    LEXIT("(%p)", timers);
    return timers;
}

int co_timer_mgr_get_next_delay(co_timer_mgr_t tm) {
    assert(tm);
    return (int) (tm->next_trigger_time - time_since_1970());
}

int co_timer_mgr_cancel_timer(co_timer_mgr_t tm, co_timer_t t) {
    int retval = -1;
    LENTRY("(tm:%p, t:%p)", tm, t);
    assert(t);

    co_timer_cancel(t);

    co_timer_deadline_t deadline_prev = tm->deadlines;
    co_timer_deadline_t deadline = deadline_prev;
    // finding deadline matches by time point.
    while (deadline) {
        if (deadline->time_point == t->time_point) {
            // finding timer in current deadline.
            co_timer_t timer_prev = deadline->timers;
            co_timer_t timer = timer_prev;
            while (timer) {
                if (timer == t) {
                    LOGI("the specified timer(%p) found in deadline(%p) by time_point:%d",
                         timer, deadline, timer->time_point);
                    // remove it from deadline (no free).
                    if (timer_prev == timer)
                        deadline->timers = timer->next;
                    else
                        timer_prev->next = timer->next;
                    retval = 0;
                    break;
                }
                timer_prev = timer;
                timer = timer->next;
            }

            // remove the deadline if it's empty.
            if (!deadline->timers) {
                LOGI("removing empty deadline(%p)", deadline);
                if (deadline_prev == deadline)
                    tm->deadlines = deadline->next;
                else
                    deadline_prev->next = deadline->next;
                free(deadline); // release it.
                deadline = deadline_prev;
            }
        } else if (deadline->time_point > t->time_point)
            break;
        // move to next deadline.
        deadline_prev = deadline;
        deadline = deadline->next;
    }

    LEXIT("(%d)", retval);
    return retval;
}

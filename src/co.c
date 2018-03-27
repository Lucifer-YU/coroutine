/*
 * co.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include <pch.h>

#define LOG_TAG "CO"


int co_yield() {
    LENTRY("()");
    co_sched_t sched = co_sched_ct();
    assert(sched);
    int retval = co_sched_yield(sched);
    LEXIT("(%d)", retval);
    return retval;
}

int co_sleep(uint32_t msec) {
    int retval;
    LENTRY("(msec:%u)", msec);
    co_sched_t sched = co_sched_ct();
    assert(sched);
    retval = co_sched_sleep(sched, msec);
    LEXIT("(%d)", retval);
    return retval;
}

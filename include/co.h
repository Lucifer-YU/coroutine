#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __co_sched *co_sched_t;
typedef struct __co_task *co_task_t;

// Coroutine scheduler.
co_sched_t co_sched_create();
void co_sched_destroy(co_sched_t sched);
int co_sched_runloop(co_sched_t sched);

int co_sched_create_task(co_sched_t sched, size_t stack_size, void (*start_routine)(void *), void *arg);

int co_yield();
int co_sleep(uint32_t msec);

#ifdef __cplusplus
}
#endif

#endif  // __COROUTINE_H__

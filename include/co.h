#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __co_sched *co_sched_t;
typedef struct __co_task *co_task_t;

// Creates a new instance of coroutine scheduler.
co_sched_t co_sched_create();
// Destroys a coroutine scheduler.
void co_sched_destroy(co_sched_t sched);
// Runs the scheduler until all tasks are done. 
int co_sched_runloop(co_sched_t sched);
// Creates a new teask and assigns to specified scheduler.
int co_sched_create_task(co_sched_t sched, size_t stack_size, void (*start_routine)(void *), void *arg);
// Gets the current scheduler (thread specific).
co_sched_t co_sched_self();
// Gets the current running task.
co_task_t co_task_self();

#ifdef _WIN32
// The 'co_yield' becomes keyword in VS 2017!
#define co_yield co_switch
#endif
int co_yield();
int co_sleep(uint32_t msec);

#ifdef __cplusplus
}
#endif

#endif  // __COROUTINE_H__

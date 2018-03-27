#include <co.h>
#include <stdio.h>
#include <unistd.h>
#include <dbg/dbgmsg.h>

void task_fn(void *arg) {
    const char *str = arg;
    for (int i = 0; i < 10; i++) {
        printf("%s (%d) \n", str, i);
        fflush(stdout);
        co_yield();
    }
}

int main(int argc, char *argv[]) {
    dbg_log_level(DLI_WARN);
    // Creates a scheduler.
    co_sched_t sched = co_sched_create();
    // Creates 10 tasks.
    co_sched_create_task(sched, 0U, task_fn, "task0");
    co_sched_create_task(sched, 0U, task_fn, "task1");
    co_sched_create_task(sched, 0U, task_fn, "task2");
    co_sched_create_task(sched, 0U, task_fn, "task3");
    co_sched_create_task(sched, 0U, task_fn, "task4");
    co_sched_create_task(sched, 0U, task_fn, "task5");
    co_sched_create_task(sched, 0U, task_fn, "task6");
    co_sched_create_task(sched, 0U, task_fn, "task7");
    co_sched_create_task(sched, 0U, task_fn, "task8");
    co_sched_create_task(sched, 0U, task_fn, "task9");
    // Scheduling
    co_sched_runloop(sched);
    // Destroys the scheduler.
    co_sched_destroy(sched);

    return 0;
}

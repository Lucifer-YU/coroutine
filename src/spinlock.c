#include <config.h>

#include <spinlock.h>

#include <sched.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

/**
 * Lock a spin lock object.
 */
void spin_lock(spinlock_t *volatile sp) {
#ifdef _WIN32
    while (InterlockedCompareExchange(sp, 1L, 0)) {
#else
    while (!__sync_val_compare_and_swap(sp, 0, 1L)) {
#endif
        sched_yield();
    }
}
/**
 * Attempt to lock a spin lock object.
 * 
 * @return 0 if successful; otherwise failed.
 */
int spin_trylock(spinlock_t *volatile sp) {
    return (int)
#ifdef _WIN32
        InterlockedCompareExchange(sp, 1L, 0);
#else
        __sync_val_compare_and_swap(sp, 0, 1L);
#endif
}
/**
 * Unlock a spin lock object.
 */
void spin_unlock(spinlock_t *volatile sp) {
#ifdef _WIN32
    InterlockedExchange(sp, 0);
#else
    __sync_val_compare_and_swap(sp, 1L, 0);
#endif
}

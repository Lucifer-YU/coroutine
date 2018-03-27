/*
 * spinlock.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Lucifer
 */

#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <pthread.h>

typedef long spinlock_t;

/**
 * Initializes a spin lock object.
 */
#define spin_init(sp) \
    { *(sp) = 0; }

/**
 * Destroys a spin lock object.
 */
#define spin_destroy(sp)

/**
 * Lock a spin lock object.
 */
void spin_lock(spinlock_t *volatile sp);
/**
 * Attempt to lock a spin lock object.
 * 
 * @return 0 if successful; otherwise failed.
 */
int spin_trylock(spinlock_t *volatile sp);
/**
 * Unlock a spin lock object.
 */
void spin_unlock(spinlock_t *volatile sp);

#endif /* __SPINLOCK_H__ */

/*
 * co_context.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#ifndef __UNIX_COROUTINE_CONTEXT_H__
#define __UNIX_COROUTINE_CONTEXT_H__

#ifdef USE_UCONTEXT

// #define _XOPEN_SOURCE // <-- Should add this before all includes.

#include <ucontext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __co_context {
    ucontext_t uctx;
    void *stack;
    size_t stack_size;
    void (*start_routine)(void *);
    void *arg;
} * co_context_t;

co_context_t co_context_create(size_t stack_size, void (*start_routine)(void *), void *arg);
void co_context_destroy(co_context_t ctx);

int co_context_swapin(co_context_t ctx);
int co_context_swapout(co_context_t ctx);

void co_context_scope_enter();
void co_context_scope_leave();

#ifdef __cplusplus
}
#endif

#endif // USE_UCONTEXT

#endif // __COROUTINE_CONTEXT_H__

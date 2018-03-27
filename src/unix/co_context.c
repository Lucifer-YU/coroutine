/*
 * co_context.c
 *
 *  Created on: Jan 30, 2018
 *      Author: Lucifer
 */
#include <pch.h>

#ifdef USE_UCONTEXT

#define LOG_TAG "UCTX"

static __thread ucontext_t __co_context_ct;

static void co_context_start_routine(void *args) {
    co_context_t ctx = (co_context_t) args;
    ctx->start_routine(ctx->arg);
}

co_context_t co_context_create(size_t stack_size, void (*start_routine)(void *), void *arg) {
    LENTRY("(stack_size:%u, start_routine:%p, arg:%p)", stack_size, start_routine, arg);

    assert(stack_size >= 0);
    assert(start_routine);

    co_context_t ctx = calloc(1, sizeof(struct __co_context));
    if (-1 == getcontext(&ctx->uctx)) {
        LOGE("getcontext() failed");
        free(ctx);
        goto exit;
    }
    ctx->start_routine = start_routine;
    ctx->arg = arg;
    ctx->stack_size = stack_size;
    ctx->stack = malloc(stack_size);

    ctx->uctx.uc_stack.ss_sp = ctx->stack;
    ctx->uctx.uc_stack.ss_size = ctx->stack_size;
    ctx->uctx.uc_link = NULL;

    makecontext(&ctx->uctx, (void (*)(void)) co_context_start_routine, 1, ctx);

exit:
    LEXIT("(%p)", ctx);
    return ctx;
}

void co_context_destroy(co_context_t ctx) {
    LENTRY("(ctx:%p)", ctx);

    assert(ctx);
    free(ctx->stack);
    free(ctx);

    LEXIT("()");
}

int co_context_swapin(co_context_t ctx) {
	assert(ctx);
	return swapcontext(&__co_context_ct, &ctx->uctx);
}

int co_context_swapout(co_context_t ctx) {
	assert(ctx);
	return swapcontext(&ctx->uctx, &__co_context_ct);
}

void co_context_scope_enter() {
    // noting to do.
}

void co_context_scope_leave() {
    // noting to do.
}

#endif // USE_UCONTEXT

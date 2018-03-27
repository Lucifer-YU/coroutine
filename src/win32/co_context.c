/*
 * co_context.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include <pch.h>
#include <minmax.h>

#ifdef USE_FIBER

#define LOG_TAG "WCTX"

static __declspec(thread) void* __co_context_ct = NULL;

static void WINAPI co_context_start_routine(void* args) {
    co_context_t ctx = (co_context_t) args;
    ctx->start_routine(ctx->arg);
}

co_context_t co_context_create(size_t stack_size, void (*start_routine)(void *), void *arg) {
    LENTRY("(stack_size:%u, start_routine:%p, arg:%p)", stack_size, start_routine, arg);

    assert(stack_size >= 0);
    assert(start_routine);

    co_context_t ctx = calloc(1, sizeof(struct __co_context));
    size_t commit_size = max(4 * 1024, stack_size);
    ctx->fctx = CreateFiberEx(commit_size, max(commit_size, stack_size), FIBER_FLAG_FLOAT_SWITCH, (LPFIBER_START_ROUTINE)co_context_start_routine, ctx);
    if (!ctx->fctx) {
        LOGE("CreateFiberEx() failed");
        free(ctx);
        goto exit;
    }
    ctx->start_routine = start_routine;
    ctx->arg = arg;
    ctx->stack_size = stack_size;

exit:
    LEXIT("(%p)", ctx);
    return ctx;
}

void co_context_destroy(co_context_t ctx) {
    LENTRY("(ctx:%p)", ctx);

    assert(ctx);
    DeleteFiber(ctx->fctx);
    free(ctx);

    LEXIT("()");
}

int co_context_swapin(co_context_t ctx) {
    LENTRY("(ctx:%p)", ctx);
    assert(ctx);
    SwitchToFiber(ctx->fctx);
    LEXIT("()");
    return 0;
}

int co_context_swapout(co_context_t ctx) {
    LENTRY("(ctx:%p)", ctx);
    assert(ctx);
    assert(__co_context_ct);
    SwitchToFiber(__co_context_ct);
    LEXIT("()");
    return 0;
}

void co_context_scope_enter() {
    assert(!__co_context_ct);
     __co_context_ct = ConvertThreadToFiber(NULL);
}

void co_context_scope_leave() {
    assert(__co_context_ct);
	ConvertFiberToThread();
    __co_context_ct = NULL;
}

#endif // USE_FIBER

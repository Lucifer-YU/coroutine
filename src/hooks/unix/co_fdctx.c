/*
 * co_fdcth.c
 *
 *  Created on: Feb 6, 2018
 *      Author: Lucifer
 */

#include <pch.h>
#include <co_epoll.h>
#include <co_fdctx.h>
#include <co_mux.h>

#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#define LOG_TAG "FDCTX"

static co_fdctx_t __co_fdctx_map[CO_FD_MAX] = {0};

co_fdctx_t co_fdctx_create(int fd) {
    LENTRY("(fd:%d)", fd);

    if (fd < 0 || fd > CO_FD_MAX) {
        errno = EBADF;
        goto exit;
    }
    if (__co_fdctx_map[fd]) {
        errno = EINVAL;
        goto exit;
    }

    co_fdctx_t ctx = NULL;
    struct stat fd_stat;
    int retval = fstat(fd, &fd_stat);
    if (retval == -1) {
        LOGE("fstat() error:%s", strerror(errno));
        goto exit;
    }
    ctx = calloc(0, sizeof(struct __co_fdctx));
    ctx->fd = fd;
    ctx->is_socket = S_ISSOCK(fd_stat.st_mode);
    if (ctx->is_socket) {
        int flags = fcntl_impl(fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK))
            fcntl_impl(fd, F_SETFL, flags | O_NONBLOCK);

        ctx->sys_blocking = 0;
    } else {
        ctx->sys_blocking = 1;
    }

    ctx->blocking = 1;
    assert(!__co_fdctx_map[fd]);
    __co_fdctx_map[fd] = ctx;

    LOGI("context of fd:%d created, is_socket:%d, sys_blocking:%d, blocking:%d", ctx->fd, ctx->is_socket,
         ctx->sys_blocking, ctx->blocking);

exit:
    LEXIT("(%p)", ctx);
    return ctx;
}

co_fdctx_t co_fdctx_get(int fd, int force) {
    LENTRY("(fd:%d,force:%d)", fd, force);
    co_fdctx_t ctx = NULL;
    do {
        if (fd < 0 || fd > CO_FD_MAX) {
            errno = EBADF;
            break;
        }
        ctx = __co_fdctx_map[fd];
        if (!ctx && force)
            ctx = co_fdctx_create(fd);
        LEXIT("(%p)", ctx);
    } while (0);
    return ctx;
}

int co_fdctx_add_poll_events(co_fdctx_t fdctx, int poll_events, co_sched_t sched) {
    int retval = 0;
    assert(fdctx);
    if (fdctx->closed) {
        // already closed.
        retval = -1, errno = EINVAL;
        goto exit;
    }

    // TODO ET-mode?

    poll_events &= (POLLIN | POLLOUT); // strip err, hup, rdhup ...
    if (poll_events & ~fdctx->pending_events) {
        uint32_t events = fdctx->pending_events | poll_events;
        co_mux_t reactor = co_sched_get_iowait_mgr(sched, 1);
        if (!fdctx->pending_events) {
            // add events
            retval = co_mux_register(reactor, fdctx->fd, events);
        } else {
            // modify events
            retval = co_mux_edit(reactor, fdctx->fd, events);
        }
        fdctx->pending_events = events;
    }

exit:
    return retval;
}
void co_fdctx_remove_poll_events(co_fdctx_t fdctx, int poll_events, co_sched_t sched) {
    int retval = 0;
    assert(fdctx);
    if (fdctx->closed) {
        // already closed.
        retval = -1, errno = EINVAL;
        goto exit;
    }

    if (!fdctx->pending_events)
        goto exit;

    poll_events &= (POLLIN | POLLOUT);
    int del_event = 0;
    if (poll_events & POLLIN) {
        del_event |= CO_EPOLLIN;
    }
    if (poll_events & POLLOUT) {
        del_event |= CO_EPOLLOUT;
    }
	if (poll_events) {
		
	}
exit:
    return retval;
}

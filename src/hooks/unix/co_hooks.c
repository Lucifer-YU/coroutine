/*
 * co_hooks.c
 *
 *  Created on: Feb 6, 2018
 *      Author: Lucifer
 */
#include <pch.h>
#include <co_fdctx.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#define LOG_TAG "HOOK"

sleep_t sleep_impl = NULL;
usleep_t usleep_impl = NULL;
nanosleep_t nanosleep_impl = NULL;

connect_t connect_impl = NULL;
fcntl_t fcntl_impl = NULL;
poll_t poll_impl = NULL;

#ifndef RTLD_NEXT
#define RTLD_NEXT RTLD_DEFAULT
#endif	// RTLD_NEXT

static int __co_hooks_enabled = 0;

void co_hooks_enable(int enable) {
	__co_hooks_enabled = enable;
}

int co_hooks_is_enabled() {
	return __co_hooks_enabled;
}

static void co_hooks_init() {
	static int __initialized = 0;
	if (!__initialized) {
		LENTRY("()");
		sleep_impl = (sleep_t) dlsym(RTLD_NEXT, "sleep");
		usleep_impl = (usleep_t) dlsym(RTLD_NEXT, "usleep");
		nanosleep_impl = (nanosleep_t) dlsym(RTLD_NEXT, "nanosleep");
		assert(sleep_impl && usleep_impl && nanosleep_impl);

		connect_impl = (connect_t) dlsym(RTLD_NEXT, "connect");
		fcntl_impl = (fcntl_t) dlsym(RTLD_NEXT, "fcntl");
		poll_impl = (poll_t) dlsym(RTLD_NEXT, "poll");
		assert(connect_impl && fcntl_impl && poll_impl);

		__initialized = 1;
		LEXIT("()");
	}
}

unsigned int sleep(unsigned int seconds) {
	LENTRY("(seconds:%u)", seconds);

	unsigned int retval = 0;
	co_hooks_init();

	co_sched_t sched = co_sched_self();
	if (__co_hooks_enabled && (sched && sched->current_task)) {
		// hook enabled & required.
		int ms = seconds * 1000;
		co_sched_sleep(sched, ms);
	} else {
		retval = sleep_impl(seconds);
	}

	LEXIT("(%u)", retval);
	return retval;
}

int usleep(useconds_t usec) {
	LENTRY("(usec:%ul)", usec);

	int retval;
	co_hooks_init();

	co_sched_t sched = co_sched_self();
	if (__co_hooks_enabled && (sched && sched->current_task)) {
		int ms = usec / 1000;
		retval = co_sched_sleep(sched, ms);
	} else {
		retval = usleep_impl(usec);
	}

	LEXIT("(%d)", retval);
	return retval;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
	LENTRY("(req:%p, rem:%p)", req, rem);
	int retval;
	co_hooks_init();

	co_sched_t sched = co_sched_self();
	if (__co_hooks_enabled && (sched && sched->current_task)) {
		int ms = req->tv_sec * 1000 + req->tv_nsec / 1000000;
		retval = co_sched_sleep(sched, ms);
	} else {
		retval = nanosleep_impl(req, rem);
	}

	LEXIT("(%d)", retval);
	return retval;
}

int connect(int fd, const struct sockaddr *addr, socklen_t addrlen) {
	LENTRY("(fd:%d, addr:%p, rem:%u)", fd, addr, addrlen);
	int retval;
	// ensure hooks initialized.
	co_hooks_init();

	// call sys api directly if hooks does not enabled, or not under coroutine task currently.
	co_sched_t sched = co_sched_self();
	if (!__co_hooks_enabled || !(sched && sched->current_task)) {
		retval = connect_impl(fd, addr, addrlen);
		goto exit;
	}

	// prepares the global accessible context for given fd.
	co_fdctx_t fdctx = co_fdctx_get(fd, 1);
	if (fdctx->closed) {
		retval = -1, errno = EBADF;
		goto exit;
	}

	retval = connect_impl(fd, addr, addrlen);
	if (!fdctx->blocking) {
		goto exit; // return immediately if non-blocking.
	}
	if (retval == 0 || errno != EINPROGRESS) {
		if (retval == 0)
			LOGI("continue task(%p) connect(%d) completed immediately.", sched->current_task, fd);
		goto exit;
	}

	// still in progress, use poll for wait connect complete.
	struct pollfd pfd = { 0 };
	pfd.fd = fd;
	pfd.events = POLLOUT;
	int poll_retval = poll(&pfd, 1, -1);	// FIXME load timeout from preset value.
	if (poll_retval <= 0 || pfd.revents != POLLOUT) {
		retval = -1, errno = ETIMEDOUT;
		goto exit;
	}

    // retrieves and resets possible so_error.
	int err = 0;
	socklen_t errlen = sizeof(err);
	retval = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen);
	if (retval == -1)
		goto exit;
	if (err) {
		retval = -1, errno = err;
	}

exit:
	LEXIT("(%d)", retval);
	return retval;
}

static int co_poll_impl(struct pollfd *fds, nfds_t nfds, int timeout, int pre_check) {
	LENTRY("(fds:%p, nfds:%u, timeout:%d, pre_check:%d)", fds, nfds, timeout, pre_check);
	int retval;
	// ensure hooks initialized.
	co_hooks_init();

	// call sys api directly if hooks does not enabled, or not under coroutine task currently.
	co_sched_t sched = co_sched_self();
	if (!__co_hooks_enabled || !(sched && sched->current_task)) {
		retval = poll_impl(fds, nfds, timeout);
		goto exit;
	}
	// call sys api directly if no timeout specified.
	if (timeout == 0) {
		retval = poll_impl(fds, nfds, 0);
		goto exit;
	}
	// check if we have available fd(s). we will use sleep to instead poll, if all fds are unavailable.
	int has_available_fds = 0;
	for (nfds_t i = 0; i < nfds; i ++) {
		if (fds[i].fd >= 0) {
			has_available_fds = 1;
			break;
		}
	}
	if (!has_available_fds) {
		co_sched_sleep(sched, timeout);
		goto exit;
	}
	// pre-check events before waiting.
	if (pre_check) {
		retval = poll_impl(fds, nfds, 0);
		if (retval != 0) {
			LOGI("poll returns %d immediately.", retval);
			goto exit;
		}
	}

	// add fd into poll.
	int added = 0, triggered = 0;
	for (nfds_t i = 0; i < nfds; i ++) {
		struct pollfd* pfd = fds + i;
		pfd->revents = 0;
		co_fdctx_t fdctx = co_fdctx_get(pfd->fd, 1);
		if (!fdctx || fdctx->closed) {
			pfd->revents = POLLNVAL;
			continue;	// skip invalid fd.
		}

		int res = co_fdctx_add_poll_events(fdctx, pfd->events, sched);
		if (res == -1) {
			// failed
			pfd->revents = POLLNVAL;
			continue;
		} else if (res == 1) {
			// completed
			triggered = 1;
		}
		added = 1;
	}
exit:
	return retval;
}

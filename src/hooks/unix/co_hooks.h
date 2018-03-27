/*
 * co_hooks.h
 *
 *  Created on: Feb 6, 2018
 *      Author: Lucifer
 */

#ifndef SRC_UNIX_CO_HOOKS_H_
#define SRC_UNIX_CO_HOOKS_H_

#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>

#ifdef __cplusplus
extern "C" {
#endif

void co_hooks_enable(int enable);
int co_hooks_is_enabled();

typedef unsigned int (*sleep_t)(unsigned int seconds);
typedef int (*usleep_t)(useconds_t usec);
typedef int (*nanosleep_t)(const struct timespec *req, struct timespec *rem);

extern sleep_t sleep_impl;
extern usleep_t usleep_impl;
extern nanosleep_t nanosleep_impl;

typedef int(*connect_t)(int, const struct sockaddr *, socklen_t);
typedef int(*fcntl_t)(int fd, int cmd, ...);
typedef int(*poll_t)(struct pollfd *fds, nfds_t nfds, int timeout);

extern connect_t connect_impl;
extern fcntl_t fcntl_impl;
extern poll_t poll_impl;

#ifdef __cplusplus
}
#endif

#endif /* SRC_UNIX_CO_HOOKS_H_ */

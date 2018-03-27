/*
 * co_epoll.c
 *
 *  Created on: Feb 8, 2018
 *      Author: Lucifer
 */

#include <config.h>
#include <co_epoll.h>
#include <dbgalloc.h>

#include <stdlib.h> // calloc
#include <assert.h>
#include <errno.h>
#include <limits.h> // INT_MAX
#include <minmax.h>
#include <string.h> // memcpy, memmove
#ifdef USE_EPOLL

int co_epoll_create(int size) {
    return epoll_create(size);
}
int co_epoll_ctl(int epfd, int op, int fd, co_epoll_events_t *ev) {
    return epoll_ctl(epfd, op, fd, ev);
}
int co_epoll_wait(int epfd, co_epoll_events_t *ev, int maxevents, int timeout) {
    return epoll_wait(epfd, ev, maxevents, timeout);
}
int co_epoll_destroy(int epfd) {
    return close(epfd);
}

#endif // USE_EPOLL

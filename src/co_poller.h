/*
 * co_poller.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */
#ifndef __CO_REACTOR_H__
#define __CO_REACTOR_H__

typedef struct __co_task * co_task_t;

typedef void (*co_poller_events_cb)(int fd, int events);

typedef struct __co_poller {
    struct pollfd* pfds;
    nfds_t npfds;
    nfds_t max_npfds;
    co_poller_events_cb* cbs;   // The twins of pfds, uses to stores event callbacks.
} * co_poller_t;

co_poller_t co_poller_create();
void co_poller_destroy(co_poller_t poller);

int co_poller_events_register(co_poller_t poller, int fd, int epoll_events, void (*cb) (int fd, int events));
int co_poller_events_unregister(co_poller_t poller, int fd);

int co_poller_events_wait(co_poller_t poller, int milliseconds);

int co_poller_switch_task(co_poller_t poller, co_task_t task);

#endif // __CO_REACTOR_H__

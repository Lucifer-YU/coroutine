/*
 * co_mux.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */
#ifndef __CO_REACTOR_H__
#define __CO_REACTOR_H__

typedef struct __co_task * co_task_t;

typedef void (*co_mux_events_cb)(int fd, int events);

typedef struct __co_mux {
#if 0   // Sys-hooks still in developing.
    struct pollfd* pfds;
    nfds_t npfds;
    nfds_t max_npfds;
#endif
    co_mux_events_cb* cbs;   // The twins of pfds, uses to stores event callbacks.
} * co_mux_t;

co_mux_t co_mux_create();
void co_mux_destroy(co_mux_t poller);

int co_mux_events_register(co_mux_t poller, int fd, int epoll_events, void (*cb) (int fd, int events));
int co_mux_events_unregister(co_mux_t poller, int fd);

int co_mux_events_wait(co_mux_t poller, int milliseconds);

int co_mux_switch_task(co_mux_t poller, co_task_t task);

#endif // __CO_REACTOR_H__

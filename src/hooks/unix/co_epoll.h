/*
 * co_epoll.h
 *
 *  Created on: Feb 8, 2018
 *      Author: Lucifer
 */

#ifndef SRC_UNIX_CO_EPOLL_H_
#define SRC_UNIX_CO_EPOLL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_EPOLL

#include <sys/epoll.h>

#define CO_EPOLLIN EPOLLIN
#define CO_EPOLLPRI EPOLLPRI
#define CO_EPOLLOUT EPOLLOUT

#define CO_EPOLLERR EPOLLERR
#define CO_EPOLLHUP EPOLLHUP

#define CO_EPOLLRDNORM EPOLLRDNORM
#define CO_EPOLLWRNORM EPOLLWRNORM

// epoll_ctl() operations
#define CO_EPOLL_CTL_ADD EPOLL_CTL_ADD
#define CO_EPOLL_CTL_DEL EPOLL_CTL_DEL
#define CO_EPOLL_CTL_MOD EPOLL_CTL_MOD

typedef struct epoll_events co_epoll_events_t;

#else // !USE_EPOLL

// epoll event types
#define CO_EPOLLIN 0x001
#define CO_EPOLLPRI 0x002
#define CO_EPOLLOUT 0x004

#define CO_EPOLLERR 0x008
#define CO_EPOLLHUP 0x010

#define CO_EPOLLRDNORM 0x40
#define CO_EPOLLWRNORM 0x004

// epoll_ctl() operations
#define CO_EPOLL_CTL_ADD 1
#define CO_EPOLL_CTL_DEL 2
#define CO_EPOLL_CTL_MOD 3

#ifdef USE_KQUEUE

#include <sys/event.h>

typedef struct co_epoll_events {
    uint32_t events;
    struct kevent *kevent;
} co_epoll_events_t;

#endif // !USE_KQUEUE

#ifdef USE_POLL

#include <sys/poll.h>

typedef union co_epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} co_epoll_data_t;

typedef struct co_epoll_events {
    uint32_t events;
    co_epoll_data_t data;
} co_epoll_events_t;

#endif // !USE_POLL

#endif

int co_epoll_create(int size);
int co_epoll_ctl(int epfd, int op, int fd, co_epoll_events_t *ev);
int co_epoll_wait(int epfd, co_epoll_events_t *ev, int maxevents, int timeout);
int co_epoll_destroy(int epfd);

#ifdef __cplusplus
}
#endif

#endif /* SRC_UNIX_CO_EPOLL_H_ */

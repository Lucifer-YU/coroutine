/*
 * co_epoll_poll.c
 *
 *  Created on: Feb 8, 2018
 *      Author: Lucifer
 */

#include <config.h>

#ifdef USE_POLL

#include <co_epoll.h>
#include <dbgalloc.h>

#include <stdlib.h> // calloc
#include <errno.h>
#include <assert.h>
#include <limits.h> // INT_MAX
#include <minmax.h>
#include <string.h> // memcpy, memmove

typedef struct __co_epoll {
    struct pollfd *fds;
    nfds_t nfds;
    nfds_t max_nfds;
} * co_epoll_t;

#ifndef CO_EPOLL_LIMIT
#define CO_EPOLL_LIMIT (1024 * 10)
#endif

static co_epoll_t __co_epoll_map[CO_EPOLL_LIMIT] = {0};

static int co_epoll_pollfd_find_index(co_epoll_t ep, int fd) {
    assert(ep);
    for (nfds_t i = 0; i < ep->nfds; i++) {
        if (ep->fds[i].fd == fd)
            return i;
    }
    return -1;
}
static struct pollfd *co_epoll_pollfd_find(co_epoll_t ep, int fd) {
    assert(ep);
    int n = co_epoll_pollfd_find_index(ep, fd);
    return n < 0 ? NULL : ep->fds + n;
}
static struct pollfd *co_epoll_pollfd_add(co_epoll_t ep, int fd) {
    assert(ep);
#ifndef NDEBUG
    assert(-1 == co_epoll_pollfd_find_index(ep, fd));
#endif
    // out of space
    if (ep->nfds == ep->max_nfds) {
        size_t max_nfds = max(ep->max_nfds * 2, 16);
        struct pollfd *fds = calloc(max_nfds, sizeof(struct pollfd));
        if (ep->nfds) {
            memcpy(fds, ep->fds, ep->nfds * sizeof(struct pollfd));
            free(ep->fds);
        }
        ep->max_nfds = max_nfds;
        ep->fds = fds;
    }
    struct pollfd *pfd = ep->fds + ep->nfds++;
    memset(pfd, 0, sizeof(struct pollfd));
    pfd->fd = fd;

    return pfd;
}

static void co_epoll_pollfd_remove_at(co_epoll_t ep, nfds_t index) {
    assert(ep);
    assert(index >= 0);
    if (index < (ep->nfds - 1)) {
        // shift old data down
        memmove(ep->fds + index, ep->fds + index + 1, sizeof(struct pollfd));
    }
    ep->nfds--;
}

static short co_epoll_events_to_poll(uint32_t events) {
    short poll_events = 0;
    if (events & CO_EPOLLIN)
        poll_events |= POLLIN;
    if (events & CO_EPOLLOUT)
        poll_events |= POLLOUT;
    if (events & CO_EPOLLHUP)
        poll_events |= POLLHUP;
    if (events & CO_EPOLLERR)
        poll_events |= POLLERR;
    if (events & CO_EPOLLRDNORM)
        poll_events |= POLLRDNORM;
    if (events & CO_EPOLLWRNORM)
        poll_events |= POLLWRNORM;
    return poll_events;
}

static uint32_t co_epoll_events_from_poll(short poll_events) {
    uint32_t events = 0;
    if (poll_events & POLLIN)
        events |= CO_EPOLLIN;
    if (poll_events & POLLOUT)
        events |= CO_EPOLLOUT;
    if (poll_events & POLLHUP)
        events |= CO_EPOLLHUP;
    if (poll_events & POLLERR)
        events |= CO_EPOLLERR;
    if (poll_events & POLLRDNORM)
        events |= CO_EPOLLRDNORM;
    if (poll_events & POLLWRNORM)
        events |= CO_EPOLLWRNORM;
    return events;
}

int co_epoll_create(int size) {
    int n = 0;
    // Since Linux 2.6.8, the size argument is ignored, but must be greater than zero.
    if (size < 1) {
        n = -1;
        errno = EINVAL;
        goto exit;
    }
    for (; n < CO_EPOLL_LIMIT; n++) {
        if (!__co_epoll_map[n])
            break;
    }
    if (n == CO_EPOLL_LIMIT) {
        n = -1;
        errno = ENOMEM;
        goto exit;
    }
    co_epoll_t ep = calloc(1, sizeof(struct __co_epoll));
    __co_epoll_map[n] = ep;

exit:
    return n;
}

int co_epoll_ctl(int epfd, int op, int fd, co_epoll_events_t *ev) {
    int retval = -1;
    // attempt to get the inner epoll object for specified epfd.
    if (epfd < 0 || epfd > CO_EPOLL_LIMIT) {
        errno = EBADF;
        goto exit;
    }
    co_epoll_t ep = __co_epoll_map[epfd];
    if (!ep) {
        errno = EBADF;
        goto exit;
    }
    if (op == CO_EPOLL_CTL_DEL) {
        // remove the specified fd from the this epoll object.
        int n = co_epoll_pollfd_find_index(ep, fd);
        if (n >= 0) {
            co_epoll_pollfd_remove_at(ep, n);
            retval = 0;
        } else
            retval = EBADF;
        goto exit;
    }
    // add or modify fd event registration.
    const int flags = (CO_EPOLLIN | CO_EPOLLOUT | CO_EPOLLERR | CO_EPOLLHUP);
    if (ev->events & ~flags) {
        errno = ENOTSUP;
        goto exit;
    }
    struct pollfd *pfd = co_epoll_pollfd_find(ep, fd);
    if (op == CO_EPOLL_CTL_ADD) {
        if (pfd) {
            errno = EEXIST;
            goto exit;
        }
        // add new
        pfd = co_epoll_pollfd_add(ep, fd);
    } else if (op == CO_EPOLL_CTL_MOD && !pfd) {
        errno = ENOENT;
        goto exit;
    }
    // convert epoll events to poll events.
    pfd->events = co_epoll_events_to_poll(ev->events);
    retval = 0;

exit:
    return retval;
}

int co_epoll_wait(int epfd, co_epoll_events_t *ev, int maxevents, int timeout) {
    int retval = -1;
    struct pollfd* fds = NULL;
    // attempt to get the inner epoll object.
    if (epfd < 0 || epfd > CO_EPOLL_LIMIT) {
        errno = EBADF;
        goto exit;
    }
    co_epoll_t ep = __co_epoll_map[epfd];
    if (!ep) {
        errno = EBADF;
        goto exit;
    }
    // make a local copy to prevent concurrent modification.
    nfds_t nfds = ep->nfds;
    fds = malloc(nfds * sizeof(struct pollfd));
    memcpy(fds, ep->fds, nfds * sizeof(struct pollfd));

    // call sys poll.
    retval = poll(fds, nfds, timeout);
    if (retval == -1) {
        goto exit;
    }
    // convert poll revents to epoll events for each fds.
    int evi = 0;
    for (nfds_t i = 0; i < nfds && evi < min(retval, maxevents); i++) {
        struct pollfd *pfd = fds + i;
        if (pfd->revents) {
            co_epoll_events_t *ev_cur = ev + evi;
            ev_cur->events = co_epoll_events_from_poll(pfd->revents);
            if (ev_cur->events) {
                ev_cur->data.fd = pfd->fd;
                evi++;
            }
        }
    }

exit:
    if (fds)
        free(fds);

    return retval;
}

int co_epoll_destroy(int epfd) {
    int retval = -1;
    if (epfd < 0 || epfd > CO_EPOLL_LIMIT || !__co_epoll_map[epfd]) {
        errno = EBADF;
        retval = -1;
        goto exit;
    }
    co_epoll_t ep = __co_epoll_map[epfd];
    __co_epoll_map[epfd] = NULL;
    // assert(!ep->nfds);
    free(ep->fds);
    free(ep);
    retval = 0;

exit:
    return retval;
}

#endif // USE_POLL

#include <config.h>

#ifdef USE_KQUEUE

#include <co_epoll.h>
#include <dbgalloc.h>

#include <assert.h>
#include <time.h>
#include <errno.h>
#include <limits.h> // INT_MAX
#include <minmax.h>
#include <string.h> // memcpy, memmove
#include <sys/types.h>

#ifndef CO_FD_MAX
#define CO_FD_MAX (65536)
#endif	// CO_FD_MAX

int co_epoll_create(int size) {
    int epfd = 0;
    // Since Linux 2.6.8, the size argument is ignored, but must be greater than zero.
    if (size < 1) {
        epfd = -1;
        errno = EINVAL;
        goto exit;
    }
    epfd = kqueue();
exit:
    return epfd;
}

int co_epoll_ctl(int epfd, int op, int fd, co_epoll_events_t *ev) {
    int retval = -1;
    if (epfd < 0 || epfd > CO_FD_MAX) {
        errno = EBADF;
        goto exit;
    }
    struct timespec ts = { 0 };

    if (op == CO_EPOLL_CTL_DEL) {
        // unset all events if exists.
        struct kevent kev = { 0 };
        EV_SET(&kev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent( epfd, &kev,1, NULL,0,&t );

        EV_SET( &kev, fd, EVFILT_WRITE, EV_DELETE,0, 0, NULL);
		kevent( epfd,&kev,1, NULL,0,&t );

        goto exit;
    }

    const int flags = ( CO_EPOLLIN | CO_EPOLLOUT | CO_EPOLLERR | CO_EPOLLHUP );
    if (ev->events & ~flags) {
        errno = ENOTSUP;
        goto exit;
    }

    if (op == EPOLL_CTL_MOD) {
        // unset all events if exists.
        struct kevent kev = { 0 };
        EV_SET(&kev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent( epfd, &kev,1, NULL,0, &ts );

        EV_SET( &kev, fd, EVFILT_WRITE, EV_DELETE,0, 0, NULL);
        ret = kevent( epfd, &kev,1, NULL,0, &ts );
    }

    // set event(s)
    if (ev->events & CO_EPOLLIN) {
        struct kevent key = { 0 };
        EV_SET(&key, fd, EVFILT_READ, EV_ADD, 0, 0, fd);
        retval = kevent(epfd, &key, 1, NULL, 0, &ts);
        if (retval)
            goto exit;
    }
    if (ev->events & CO_EPOLLOUT) {
        struct kevent key = { 0 };
        EV_SET(&key, fd, EVFILT_WRITE, EV_ADD, 0, 0, fd);
        retval = kevent(epfd, &key, 1, NULL, 0, &ts);
        if (retval)
            goto exit;
    }

exit:
    return retval;
}

int co_epoll_wait(int epfd, co_epoll_events_t *ev, int maxevents, int timeout) {
    int retval = -1;
    struct kevent *eventlist = NULL;
    if (epfd < 0 || epfd > CO_FD_MAX) {
        errno = EBADF;
        goto exit;
    }

    struct timespec ts = { 0 };
    if (timeout > 0)
        ts.tv_sec = timeout;

    // call sys kqueue
    eventlist = calloc(maxevents, sizeof(struct kevent));
    retval = kevent(epfd, NULL, 0, eventlist, maxevents, (timeout > -1) ? &ts : NULL);
    if (retval == -1) {
        goto exit;
    }
    // convert kqueue events to epoll events.
    for (int i = 0; i < retval; i ++) {
        struct kevent* pkv = = eventlist + i;
        if (pkv->filter == EVFILT_READ) {
            ev->events = CO_EPOLLIN;
        } else if (pkv->EVFILT_WRITE) {
            ev->events = CO_EPOLLOUT;
        }
        ev->data.fd = pkv->udata;
    }
exit:
    if (eventlist)
        free(eventlist);
    return retval;
}

int co_epoll_destroy(int epfd) {
    int retval = -1;
    if (epfd < 0 || epfd > CO_FD_MAX) {
        errno = EBADF;
        retval = -1;
        goto exit;
    }
    retval = close(epfd);
exit:
    return retval;
}

#endif // USE_KQUEUE

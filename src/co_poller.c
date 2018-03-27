/*
 * co_poller.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */
#include <pch.h>

#include <sys/poll.h>

#define LOG_TAG "POLLER"

static int co_poller_item_find_index(co_poller_t poller, int fd, void (*cb)(int fd, int events)) {
    assert(poller);
    for (nfds_t i = 0; i < poller->npfds; i++) {
        if (poller->pfds[i].fd == fd) {
            if (cb) {
                poller->cbs[i] = cb; // update callback if provided.
            }
            return i;
        }
    }
    return -1;
}
static struct pollfd *co_poller_item_find(co_poller_t poller, int fd, void (*cb)(int fd, int events)) {
    assert(poller);
    int n = co_poller_item_find_index(poller, fd, cb);
    return n < 0 ? NULL : poller->pfds + n;
}
static struct pollfd *co_poller_item_add(co_poller_t poller, int fd, void (*cb)(int fd, int events)) {
    assert(poller);
    assert(fd);
    assert(cb);
#ifndef NDEBUG
    assert(-1 == co_poller_item_find_index(poller, fd, NULL));
#endif
    // out of space
    if (poller->npfds == poller->max_npfds) {
        size_t max_npfds = max(poller->max_npfds * 2, 16);
        struct pollfd *pfds = calloc(max_npfds, sizeof(struct pollfd));
        co_poller_events_cb *cbs = calloc(max_npfds, sizeof(co_poller_events_cb));
        if (poller->npfds) {
            memcpy(pfds, poller->pfds, poller->npfds * sizeof(struct pollfd));
            memcpy(cbs, poller->cbs, poller->npfds * sizeof(co_poller_events_cb));
            free(poller->pfds);
            free(poller->cbs);
        }
        poller->max_npfds = max_npfds;
        poller->pfds = pfds;
        poller->cbs = cbs;
    }
    int npfds = poller->npfds++;
    struct pollfd *pfd = poller->pfds + npfds;
    pfd->fd = fd;
    poller->cbs[npfds] = cb;

    return pfd;
}

static void co_poller_item_remove_at(co_poller_t poller, nfds_t index) {
    assert(poller);
    assert(index >= 0);
    if (index < (poller->npfds - 1)) {
        // shift old data down
        memmove(poller->pfds + index, poller->pfds + index + 1, sizeof(struct pollfd));
        memmove(poller->cbs + index, poller->cbs + index + 1, sizeof(co_poller_events_cb));
    }
    poller->npfds--;
}

co_poller_t co_poller_create() {
    LENTRY("()");
    co_poller_t poller = calloc(1, sizeof(struct __co_poller));
    LEXIT("(%p)", poller);
    return poller;
}
void co_poller_destroy(co_poller_t poller) {
    LENTRY("(poller:%p)", poller);
    assert(poller);
    LEXIT("()");
    free(poller);
}

int co_poller_events_register(co_poller_t poller, int fd, int events, void (*cb)(int fd, int events)) {
    assert(poller);
    assert(fd != -1);
    assert(events);
    events &= (POLLIN | POLLOUT);
    struct pollfd *item = co_poller_item_find(poller, fd, cb);
    if (!item) {
        item = co_poller_item_add(poller, fd, cb);
    }
    item->events = events;
    return 0;
}
int co_poller_events_unregister(co_poller_t poller, int fd) {
    assert(poller);
    assert(fd != -1);
    int index = co_poller_item_find_index(poller, fd, NULL);
    if (index > -1) {
        co_poller_item_remove_at(poller, index);
    }
    return 0;
}

int co_poller_events_wait(co_poller_t poller, int milliseconds) {
    assert(poller);
}

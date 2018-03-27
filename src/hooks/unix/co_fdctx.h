/*
 * co_fdctx.h
 *
 *  Created on: Feb 8, 2018
 *      Author: Lucifer
 */

#ifndef SRC_UNIX_CO_FDCTX_H_
#define SRC_UNIX_CO_FDCTX_H_

#ifndef CO_FD_MAX
#define CO_FD_MAX (65536)
#endif	// CO_FD_MAX

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __co_sched *co_sched_t;

/**
 * The fd context.
 */
typedef struct __co_fdctx {
	int fd;
	int is_socket;
	int closed;
	int sys_blocking;// actual sys state of the file descriptor
	int blocking;	// desired state of the file descriptor for the user

	int pending_events;
}* co_fdctx_t;

/**
 * Creates a new fd context.
 * 
 * @param fd
 * 		The given fd (file descriptor).
 * @return The fd context. Or NULL if failed, and errno is set appropriately.
 */
co_fdctx_t co_fdctx_create(int fd);
/**
 * Gets the fd context of given fd.
 * 
 * @param fd
 * 		The given fd (file descriptor).
 * @param force
 * 		The flag indicates whether creates new context if not exists (1: YES, 0: NO).
 * @return The fd context. Or NULL if the context does not exists or failed.
 */
co_fdctx_t co_fdctx_get(int fd, int force);

int co_fdctx_add_poll_events(co_fdctx_t fdctx, int poll_events, co_sched_t sched);
void co_fdctx_remove_poll_events(co_fdctx_t fdctx, int poll_events, co_sched_t sched);

#ifdef __cplusplus
}
#endif

#endif // SRC_UNIX_CO_FDCTX_H_

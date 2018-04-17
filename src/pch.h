/*
 * pch.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#ifndef SRC_PCH_H_
#define SRC_PCH_H_

#ifdef __APPLE__
#define _XOPEN_SOURCE // <-- Must be add it before all includes, or you will get segmentation fault: 11.
#endif

#ifdef _WIN32
#define __thread __declspec(thread)
#include <winsock2.h>
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> // strerror
#include <assert.h>

#include <config.h>

#include <co_proc.h>
#include <co_sched.h>
#include <co_task.h>
#include <co_sleep.h>
#include <co_timer.h>
#include <co_mux.h>

#ifdef ENABLE_SYSHOOKS
#include <co_hooks.h>
#endif // ENABLE_SYSHOOKS

#include <dbg/dbgmsg.h>
#include <dbg/dbgalloc.h>

#endif /* SRC_PCH_H_ */

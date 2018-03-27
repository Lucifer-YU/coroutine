/*
 * sched.c
 *
 *  Created on: Feb 7, 2018
 *      Author: Lucifer
 */
#ifdef WIN32
#include <Windows.h>

int sched_yield(void) {
	return SwitchToThread() ? 0 : -1;
}

#endif

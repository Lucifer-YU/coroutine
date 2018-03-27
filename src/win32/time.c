/*
 * time.c
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#include "sys/time.h"

#include <winsock2.h>
#include <windows.h>

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval * __p, struct timezone *__tz) {
	if (__p) {
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;

		GetSystemTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm. tm_isdst = -1;
		clock = mktime(&tm);
		__p->tv_sec = (long)clock;
		__p->tv_usec = wtm.wMilliseconds * 1000;
	}
	if (__tz) {
		static int _tzflag = 0;
		if (!_tzflag) {
			_tzset();
			_tzflag++;
		}
		__tz->tz_minuteswest = _timezone / 60;
		__tz->tz_dsttime = _daylight;
	}
	return (0);
}

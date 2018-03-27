#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int gettimeofday(struct timeval * __p, struct timezone *__tz);

#ifdef __cplusplus
}
#endif

#endif  // __SYS_TIME_H__

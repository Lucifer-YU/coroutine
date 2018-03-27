/*
 * dbgalloc.h
 *
 *  Created on: Feb 5, 2018
 *      Author: Lucifer
 */

#ifndef __DBGALLOC_H__
#define __DBGALLOC_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Memory leaks detection.

#ifndef NDEBUG

#ifdef malloc
#undef malloc
#endif
#ifdef calloc
#undef calloc
#endif
#ifdef free
#undef free
#endif

#define malloc(__size) \
    dbg_malloc(__size, __FILE__, __LINE__)

#define calloc(__nmemb, __size) \
    dbg_calloc(__nmemb, __size, __FILE__, __LINE__)

#define free(__ptr) \
    dbg_free(__ptr, __FILE__, __LINE__)

#endif // NDEBUG

void *dbg_malloc(size_t __size, const char *file, int line);
void *dbg_calloc(size_t __nmemb, size_t __size, const char *file, int line);
void dbg_free(void *__ptr, const char *file, int line);

void dbg_leaks_report(FILE *file);

#ifdef __cplusplus
}

#endif

#endif /* __DBGALLOC_H__ */

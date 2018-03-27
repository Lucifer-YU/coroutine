/*
 * dbgmsg.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Lucifer
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG

#define NOTRACE(...)

#define LENTRY NOTRACE
#define LEXIT NOTRACE

#define LOGT NOTRACE
#define LOGI NOTRACE
#define LOGW NOTRACE
#define LOGE NOTRACE

#else

typedef enum LOG_LEVEL {
    DLI_ENTRY,
    DLI_EXIT,
    DLI_INFO,
    DLI_WARN,
    DLI_ERROR,
} LOG_LEVEL;

void dbg_log_file(FILE *file);
void dbg_log_level(LOG_LEVEL level);
void dbg_log_nesting(int nesting);

void dbg_printf(LOG_LEVEL level, const char *channel, const char *func, const char *file,
                int line, const char *format, ...);

#define LENTRY LOG_PRINTF(DLI_ENTRY, LOG_TAG)
#define LEXIT LOG_PRINTF(DLI_EXIT, LOG_TAG)

#define LOGT LOG_PRINTF(DLI_TRACE, LOG_TAG)
#define LOGI LOG_PRINTF(DLI_INFO, LOG_TAG)
#define LOGW LOG_PRINTF(DLI_WARN, LOG_TAG)
#define LOGE LOG_PRINTF(DLI_ERROR, LOG_TAG)

#define LOG_PRINTF(level, channel)       \
    do {                                 \
        const char *__channel = channel; \
        enum LOG_LEVEL __level = level;  \
    LOG_PRINTF_IMPL

#define LOG_PRINTF_IMPL(...)                                                       \
    dbg_printf(__level, __channel, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    \
}                                                                             \
    while (0)

#endif // ENABLE_DEBUG_MESSAGES

#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */

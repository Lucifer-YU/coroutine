/*
 * dbgmsg.c
 *
 *  Created on: Jun 7, 2017
 *      Author: Lucifer
 */

#include <config.h>

#ifndef NDEBUG

#include <spinlock.h>
#include <dbg/dbgmsg.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char *__log_level_names[] = {
    "ENTRY", ///
    "EXIT",  ///
    "INFO",  ///
    "WARN",  ///
    "ERROR"  ///
};

/* size of output buffer (arbitrary) */
#ifndef BUFFER_SIZE
#define DBG_BUF_SIZE 20000
#endif

static FILE *__log_file = NULL;
static enum LOG_LEVEL __log_threshold_level = DLI_ENTRY;
static spinlock_t __log_spinlock = 0;

/* per-thread storage for ENTRY tracing level */
#ifndef MAX_NESTING
#define DBG_MAX_NESTING 50
#endif

#ifdef _MSC_VER
#ifndef __thread
#define __thread __declspec(thread)
#endif
#endif

static __thread int __log_nesting = 0;
static int __log_enable_nesting = 0;
#ifndef INDENT_CHAR
#define DBG_INDENT_CHAR '.'
#endif

#ifdef _WIN32
#include <Windows.h>
#define snprintf _snprintf
#else
#include <pthread.h>
#endif

void dbg_log_file(FILE *file) {
    spin_lock(&__log_spinlock);

    if (file == NULL)
        __log_file = stdout;
    else
        __log_file = file;

    spin_unlock(&__log_spinlock);
}

void dbg_log_level(LOG_LEVEL level) {
    if (level > DLI_ERROR)
        level = DLI_ERROR;
    else if (level < DLI_ENTRY)
        level = DLI_ENTRY;
    __log_threshold_level = level;
}

void dbg_log_nesting(int nesting) {
    __log_enable_nesting = nesting;
}

/**
 * generate an indentation string to be used for message output
 *
 * @param level
 *  	level of message (DLI_ENTRY, etc)
 * @param indent_str
 *  	destination for indentation string
 * @return 0 if output can proceed, -1 otherwise
 */
int dbg_get_indent(LOG_LEVEL level, char *indent_str) {
    // determine whether to output an ENTRY line
    if ((DLI_ENTRY == level || DLI_EXIT == level) && __log_enable_nesting) {
        // determine if this is an entry or an exit
        int nesting;
        if (DLI_EXIT == level)
            nesting = __log_nesting--;
        else
            nesting = ++__log_nesting;

        //see if we're past the level threshold
        if (nesting >= DBG_MAX_NESTING)
            nesting = DBG_MAX_NESTING;
        else if (nesting < 0)
            nesting = 0;

        memset(indent_str, DBG_INDENT_CHAR, nesting);
        indent_str[nesting] = '\0';
    } else {
        indent_str[0] = '\0';
    }
    return 0;
}

void dbg_printf(LOG_LEVEL level, const char *channel, const char *func, const char *file, int line, const char *format, ...) {

    char buffer[DBG_BUF_SIZE];
    char indent[DBG_MAX_NESTING + 1];
    char *buffer_ptr;
    size_t output_size;
    va_list args;
    int thread_id;
    int errno_reserved = 0;

    dbg_get_indent(level, indent);

    if (level < __log_threshold_level)
        return;

    errno_reserved = errno;
    thread_id =
#ifdef _WIN32
        (int) GetCurrentThreadId();
#else
        (int) (long) pthread_self();
#endif

    // format log message.
    if (DLI_ENTRY == level || DLI_EXIT == level) {
        output_size = snprintf(buffer, sizeof(buffer), "{%08X} %-5s [%-7s] at %s.%d: ", thread_id, __log_level_names[level], channel, func, line);
    } else {
        output_size = snprintf(buffer, sizeof(buffer), "{%08X} %-5s [%-7s]: ", thread_id, __log_level_names[level], channel);
    }
    if (output_size + 1 > sizeof(buffer)) {
        fprintf(stderr, "ERROR : buffer overflow in dbg_printf");
        return;
    }
    buffer_ptr = buffer + output_size;
    va_start(args, format);
    output_size += vsnprintf(buffer_ptr, DBG_BUF_SIZE - output_size, format, args);
    va_end(args);
    if (output_size > DBG_BUF_SIZE) {
        fprintf(stderr, "ERROR : buffer overflow in dbg_printf");
    }

    // write log file.
    spin_lock(&__log_spinlock);
    if (!__log_file)
        __log_file = stdout;
    fprintf(__log_file, "%s%s\n", indent, buffer);
    spin_unlock(&__log_spinlock);

    // flush the output to file
    fflush(__log_file);

    errno = errno_reserved;
}

#endif // NDEBUG

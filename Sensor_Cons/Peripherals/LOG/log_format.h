/**
 * @file    log_format.h
 * @brief   Turns a log record into text
 * @details Pure formatting: it performs no I/O, so it can be reused by any
 *          sink and tested without hardware.
 */

#ifndef LOG_FORMAT_H
#define LOG_FORMAT_H

#include <stdarg.h>
#include "log_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Render one log line as "[LEVEL] (file:line) message\r\n"
 * @param  out    Destination buffer, always NUL-terminated on return
 * @param  size   Size of @p out in bytes
 * @param  level  Severity tag to prefix
 * @param  file   Source file, or NULL to omit the location
 * @param  line   Source line, ignored when @p file is NULL
 * @param  format printf-style format for the message body
 * @param  args   Arguments for @p format
 * @retval Number of characters written, excluding the terminator
 */
size_t LogFormat_Line(char *out, size_t size, log_level_t level, const char *file, int line,
                      const char *format, va_list args);

#ifdef __cplusplus
}
#endif

#endif /* LOG_FORMAT_H */

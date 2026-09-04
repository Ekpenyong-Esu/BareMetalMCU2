/**
 * @file    log.h
 * @brief   Logging API: levels, call-site macros and backend selection
 * @details Aggregator over the LOG modules. Include this and nothing else:
 *          `log_types.h` defines the vocabulary, `log_format.*` renders lines
 *          and each `log_sink_*` writes them somewhere.
 */

#ifndef LOG_H
#define LOG_H

#include "log_types.h"
#include "log_sink_printf.h"
#include "log_sink_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Choose where log lines go
 * @details Logging goes to printf until this is called, e.g.
 *          @code log_set_sink(log_sink_uart(&huart1)); @endcode
 * @param  sink Backend to use, or NULL to silence logging
 */
void log_set_sink(const log_sink_t *sink);

/**
 * @brief  Backend currently receiving log lines
 * @retval Active sink, or NULL if logging was silenced
 */
const log_sink_t *log_get_sink(void);

// Core logging function that accepts caller location
// Not reentrant: the line buffer is shared, so logging from an interrupt while
// the main loop logs will interleave the two lines.
void log_logf(log_level_t level, const char *file, int line, const char *format, ...);

// Convenience macros that automatically pass __FILE__ and __LINE__
#define log_debug(...) log_logf(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_logf(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warning(...) log_logf(LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_logf(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // LOG_H

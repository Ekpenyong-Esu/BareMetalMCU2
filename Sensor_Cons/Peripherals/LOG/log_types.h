/**
 * @file    log_types.h
 * @brief   Vocabulary shared by the logging core, the formatter and the sinks
 * @details Holds no behaviour, so every LOG module can include it without
 *          depending on any other LOG module.
 */

#ifndef LOG_TYPES_H
#define LOG_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Longest log line, including the level tag, the location and the newline. */
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 128
#endif

/** 1 to show the full source path in logs, 0 to show only the file name. */
#ifndef LOG_SHOW_FULLPATH
#define LOG_SHOW_FULLPATH 0
#endif

/**
 * @brief Severity of a log line
 */
typedef enum { LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARNING, LOG_LEVEL_ERROR } log_level_t;

/**
 * @brief Where finished log lines are written
 *
 * The core formats a line and hands it to the active sink; a sink only moves
 * bytes and never looks at levels or formats. Adding a backend therefore means
 * adding one file, not editing the core.
 */
typedef struct {
    const char *name;                               /*!< Backend name, for diagnostics */
    void (*write)(const char *text, size_t length); /*!< Emit one NUL-terminated line */
} log_sink_t;

#ifdef __cplusplus
}
#endif

#endif /* LOG_TYPES_H */

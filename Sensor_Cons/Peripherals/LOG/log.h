#ifndef LOG_H
#define LOG_H

#include "stm32f4xx_hal.h"

// ----------------------------------------------------------------------------
// Configuration
// Each option can be overridden from the build (e.g. -DLOG_USE_UART=1) or by
// defining it before including this header.
// ----------------------------------------------------------------------------

// Select the output backend. Set to 1 to enable, 0 to disable.
// If both are enabled, LOG_USE_PRINTF takes precedence.
#ifndef LOG_USE_PRINTF
#define LOG_USE_PRINTF 1  // Use printf for logging
#endif
#ifndef LOG_USE_UART
#define LOG_USE_UART 0    // Use UART for logging (requires log_init)
#endif

// Size of the internal line buffer used when LOG_USE_UART is enabled
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 128
#endif

// Show full file path or only filename in logs.
// Set to 1 to include full path (e.g., /home/.../file.c), 0 to include only the basename (file.c)
#ifndef LOG_SHOW_FULLPATH
#define LOG_SHOW_FULLPATH 0
#endif

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} log_level_t;

// Function to initialize logging (required for UART)
void log_init(UART_HandleTypeDef *huart);

// Core logging function that accepts caller location
// Not reentrant with LOG_USE_UART: the line buffer is shared, so logging from
// an interrupt while the main loop logs will interleave the two lines.
void log_logf(log_level_t level, const char *file, int line, const char *format, ...);

// Convenience macros that automatically pass __FILE__ and __LINE__
#define log_debug(format, ...)   log_logf(LOG_LEVEL_DEBUG,   __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_info(format, ...)    log_logf(LOG_LEVEL_INFO,    __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_warning(format, ...) log_logf(LOG_LEVEL_WARNING, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_error(format, ...)   log_logf(LOG_LEVEL_ERROR,   __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // LOG_H

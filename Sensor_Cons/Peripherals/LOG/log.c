/**
 * @file    log.c
 * @brief   Lightweight logging driver (printf and/or UART backends)
 * @details Formats log lines with level + optional source location and emits
 *          them via the selected backend (eyalroz printf or UART transmit).
 * @version 1.0
 */

#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <printf/printf.h>

// UART handle for logging output
static UART_HandleTypeDef *log_uart = NULL;

#if LOG_USE_UART
// Scratch buffer used to assemble the full log line before UART transmission
static char log_buffer[LOG_BUFFER_SIZE];
#endif

// Initialize logging with UART handle
void log_init(UART_HandleTypeDef *huart) {
    log_uart = huart;
}

// Return the display form of a source file path (basename unless LOG_SHOW_FULLPATH)
static const char *log_display_name(const char *file) {
#if LOG_SHOW_FULLPATH
    return file;
#else
    const char *base = strrchr(file, '/');
#ifdef _WIN32
    if (!base) base = strrchr(file, '\\');
#endif
    return base ? base + 1 : file;
#endif
}

// Internal function to log messages with optional file/line
static void log_message_loc(log_level_t level, const char *file, int line, const char *format, va_list args) {
    const char *level_str = NULL;
    switch (level) {
        case LOG_LEVEL_DEBUG:   level_str = "[DEBUG] "; break;
        case LOG_LEVEL_INFO:    level_str = "[INFO] "; break;
        case LOG_LEVEL_WARNING: level_str = "[WARNING] "; break;
        case LOG_LEVEL_ERROR:   level_str = "[ERROR] "; break;
        default:                level_str = "[UNKNOWN] "; break;
    }

#if LOG_USE_PRINTF
    // Print the level and optional location, then the message
    printf("%s", level_str);
    if (file) {
        printf("(%s:%d) ", log_display_name(file), line);
    }
    vprintf(format, args);
    printf("\r\n");
#elif LOG_USE_UART
    // Use UART for logging
    if (log_uart == NULL) return;

    /* Build the log line in the scratch buffer. snprintf() returns the length
       the text WOULD have had, so clamp `pos` after every append to keep it
       in range and prevent a buffer overrun when a field is truncated. */
    int pos = snprintf(log_buffer, LOG_BUFFER_SIZE, "%s", level_str);
    if (pos > LOG_BUFFER_SIZE) pos = LOG_BUFFER_SIZE;

    // Optional source location
    if (file && pos < LOG_BUFFER_SIZE) {
        int n = snprintf(log_buffer + pos, LOG_BUFFER_SIZE - pos, "(%s:%d) ", log_display_name(file), line);
        if (n > 0) {
            pos += n;
            if (pos > LOG_BUFFER_SIZE) pos = LOG_BUFFER_SIZE;
        }
    }

    // Message body
    vsnprintf(log_buffer + pos, LOG_BUFFER_SIZE - pos, format, args);

    // Terminating newline (bounded append at the end of the assembled line)
    size_t len = strlen(log_buffer);
    snprintf(log_buffer + len, LOG_BUFFER_SIZE - len, "\r\n");

    // Transmit via UART
    HAL_UART_Transmit(log_uart, (uint8_t *)log_buffer, strlen(log_buffer), HAL_MAX_DELAY);
#else
    (void)level;
    (void)file;
    (void)line;
    (void)format;
    (void)args;
#endif
}

// Public varargs logging function used by macros
void log_logf(log_level_t level, const char *file, int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message_loc(level, file, line, format, args);
    va_end(args);
}

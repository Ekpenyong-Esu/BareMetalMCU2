/**
 * @file    log.c
 * @brief   Logging core: renders a record, then hands it to the active sink
 * @details Knows nothing about printf or UART; swapping backends is a call to
 *          log_set_sink(), not an edit here.
 * @version 2.0
 */

#include "log.h"
#include "log_format.h"

#include <stdarg.h>

// Backend log lines are written to; printf until the application says otherwise
static const log_sink_t *log_sink = NULL;

// Scratch buffer used to assemble one complete line before it is written out
static char log_line[LOG_BUFFER_SIZE];

void log_set_sink(const log_sink_t *sink) {
    log_sink = sink;
}

const log_sink_t *log_get_sink(void) {
    return (log_sink != NULL) ? log_sink : log_sink_printf();
}

void log_logf(log_level_t level, const char *file, int line, const char *format, ...) {
    const log_sink_t *sink = log_get_sink();
    if (sink == NULL || sink->write == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    size_t length = LogFormat_Line(log_line, sizeof(log_line), level, file, line, format, args);
    va_end(args);

    sink->write(log_line, length);
}

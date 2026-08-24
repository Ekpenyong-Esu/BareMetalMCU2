/**
 * @file    log_format.c
 * @brief   Log line rendering
 */

#include "log_format.h"

#include <stdio.h>
#include <string.h>
#include <printf/printf.h>

// Text that introduces every line of a given severity
static const char *LogFormat_LevelTag(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:   return "[DEBUG] ";
        case LOG_LEVEL_INFO:    return "[INFO] ";
        case LOG_LEVEL_WARNING: return "[WARNING] ";
        case LOG_LEVEL_ERROR:   return "[ERROR] ";
        default:                return "[UNKNOWN] ";
    }
}

// Source path as it should appear in the log
static const char *LogFormat_DisplayName(const char *file) {
#if LOG_SHOW_FULLPATH
    return file;
#else
    const char *base = strrchr(file, '/');
    return base ? base + 1 : file;
#endif
}

/* snprintf() returns the length the text WOULD have had, so a truncated field
   must not be added to the cursor unclamped or the next append would run past
   the buffer. */
static size_t LogFormat_Advance(size_t pos, size_t size, int written) {
    if (written < 0) {
        return pos;
    }
    pos += (size_t)written;
    return (pos > size - 1u) ? (size - 1u) : pos;
}

size_t LogFormat_Line(char *out, size_t size, log_level_t level,
                      const char *file, int line,
                      const char *format, va_list args) {
    if (out == NULL || size == 0u) {
        return 0u;
    }

    const char *tag = LogFormat_LevelTag(level);
    const char *name = (file != NULL) ? LogFormat_DisplayName(file) : NULL;

    size_t pos = 0u;
    int written = 0;

    written = snprintf(out, size, "%s", tag);
    pos = LogFormat_Advance(pos, size, written);

    if (file != NULL) {
        written = snprintf(out + pos, size - pos, "(%s:%d) ", name, line);
        pos = LogFormat_Advance(pos, size, written);
    }

    written = vsnprintf(out + pos, size - pos, format, args);
    pos = LogFormat_Advance(pos, size, written);

    written = snprintf(out + pos, size - pos, "\r\n");
    pos = LogFormat_Advance(pos, size, written);

    return pos;
}

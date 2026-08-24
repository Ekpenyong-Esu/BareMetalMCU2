/**
 * @file    log_sink_printf.c
 * @brief   printf log backend
 */

#include "log_sink_printf.h"

#include <stdio.h>
#include <printf/printf.h>

static void LogSinkPrintf_Write(const char *text, size_t length) {
    printf("%.*s", (int)length, text);
}

static const log_sink_t LogSinkPrintf = {
    .name  = "printf",
    .write = LogSinkPrintf_Write,
};

const log_sink_t *log_sink_printf(void) {
    return &LogSinkPrintf;
}

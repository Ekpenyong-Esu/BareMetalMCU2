/**
 * @file    log_sink_printf.h
 * @brief   Log backend writing through printf (SWO / ITM on this board)
 */

#ifndef LOG_SINK_PRINTF_H
#define LOG_SINK_PRINTF_H

#include "log_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Get the printf backend
 * @retval Sink to hand to log_set_sink()
 */
const log_sink_t *log_sink_printf(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_SINK_PRINTF_H */

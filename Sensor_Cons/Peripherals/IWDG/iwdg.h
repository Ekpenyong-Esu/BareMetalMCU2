/**
 * @file iwdg.h
 * @brief Independent watchdog driver, aggregate header
 *
 * @details The driver is split into single responsibility modules:
 *          - iwdg_types.h   data types and constants
 *          - iwdg_core.h    initialization and state
 *          - iwdg_refresh.h counter refresh
 *          - iwdg_timing.h  prescaler, reload and timeout arithmetic
 *          - iwdg_reset.h   reset source reporting
 *
 * @note The independent watchdog runs from the LSI, so it keeps counting in
 *       every low power mode except standby, and it cannot be stopped once
 *       started. All timeouts are nominal: the LSI is specified from 17 kHz
 *       to 47 kHz, so refresh well inside the configured period.
 */

#ifndef IWDG_H
#define IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iwdg_types.h"
#include "iwdg_core.h"
#include "iwdg_refresh.h"
#include "iwdg_timing.h"
#include "iwdg_reset.h"

#ifdef __cplusplus
}
#endif

#endif /* IWDG_H */

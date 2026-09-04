/**
 * @file iwdg_types.h
 * @brief Types for the watchdog timer (IWDG)
 * @details The watchdog is like a guard dog. It watches your program.
 *          You must "feed" it (refresh) on time. If your program hangs
 *          and forgets to feed it, the dog barks and resets the board.
 *          This file holds the settings and numbers for that guard dog.
 *
 * How it works (in simple words):
 *  - You set a timeout (how long the dog waits).
 *  - Your code must refresh before time runs out.
 *  - If not refreshed, the board restarts automatically.
 */

#ifndef IWDG_TYPES_H
#define IWDG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_iwdg.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Driver status codes
 */
typedef enum {
    IWDG_OK = 0,        /**< Operation completed successfully */
    IWDG_ERROR,         /**< General error */
    IWDG_TIMEOUT,       /**< Operation timed out */
    IWDG_INVALID_PARAM, /**< Invalid parameter */
    IWDG_NOT_READY,     /**< The watchdog has not been initialized */
    IWDG_NOT_SUPPORTED  /**< The hardware cannot honour the request */
} IWDG_StatusTypeDef;

/**
 * @brief Watchdog configuration
 * @note  Prescaler takes one of the HAL IWDG_PRESCALER_x values.
 */
typedef struct {
    uint32_t Prescaler; /**< IWDG_PRESCALER_4 through IWDG_PRESCALER_256 */
    uint32_t Reload;    /**< Reload value, 0 to IWDG_RELOAD_MAX */
} IWDG_ConfigTypeDef;

/**
 * @brief Nominal LSI frequency
 * @note  The datasheet spread is 17 kHz to 47 kHz, so every timeout computed
 *        from this figure is an estimate; size the refresh interval well
 *        inside the nominal period.
 */
#define IWDG_LSI_FREQ 32000U

#define IWDG_RELOAD_MAX 4095U
#define IWDG_RELOAD_MIN 0U
#define IWDG_MSEC_PER_SEC 1000U

/** @brief Defaults giving roughly one second */
#define IWDG_DEFAULT_PRESCALER IWDG_PRESCALER_32
#define IWDG_DEFAULT_RELOAD 999U

/**
 * @brief Range reachable at the nominal LSI frequency
 * @note  The counter reloads with RLR and resets on underflow, so a period
 *        lasts (RLR + 1) ticks: 4096 x 256 / 32000 Hz = 32768 ms.
 */
#define IWDG_TIMEOUT_MIN_US 125U
#define IWDG_TIMEOUT_MAX_MS 32768U

/** @brief Convenience timeout values, in milliseconds */
#define IWDG_TIMEOUT_100MS 100U
#define IWDG_TIMEOUT_500MS 500U
#define IWDG_TIMEOUT_1S 1000U
#define IWDG_TIMEOUT_2S 2000U
#define IWDG_TIMEOUT_4S 4000U
#define IWDG_TIMEOUT_8S 8000U
#define IWDG_TIMEOUT_16S 16000U
#define IWDG_TIMEOUT_32S 32000U

#ifdef __cplusplus
}
#endif

#endif /* IWDG_TYPES_H */

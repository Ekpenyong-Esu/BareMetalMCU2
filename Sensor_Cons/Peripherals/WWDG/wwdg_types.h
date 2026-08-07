/**
 * @file wwdg_types.h
 * @brief Data types and constants for the Window Watchdog driver
 */

#ifndef WWDG_TYPES_H
#define WWDG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief WWDG operation status
 */
typedef enum {
    WWDG_OK = 0,           /**< Operation completed successfully */
    WWDG_ERROR,            /**< General error occurred */
    WWDG_TIMEOUT,          /**< Operation timed out */
    WWDG_INVALID_PARAM,    /**< Invalid parameter provided */
    WWDG_WINDOW_ERROR,     /**< Refresh attempted outside the valid window */
    WWDG_NOT_READY,        /**< Driver has not been initialized */
    WWDG_NOT_SUPPORTED     /**< Hardware cannot perform the request */
} WWDG_StatusTypeDef;

/**
 * @brief WWDG configuration
 */
typedef struct {
    uint32_t Prescaler;    /**< WWDG_PRESCALER_1/2/4/8 */
    uint32_t Window;       /**< Window value, WWDG_WINDOW_MIN..Counter */
    uint32_t Counter;      /**< Counter value, WWDG_COUNTER_MIN..WWDG_COUNTER_MAX */
    uint32_t EWIMode;      /**< WWDG_EWI_ENABLE or WWDG_EWI_DISABLE */
} WWDG_ConfigTypeDef;

/**
 * @brief Early wakeup interrupt callback
 */
typedef void (*WWDG_EWI_Callback_t)(void);

/** @brief Counter/window register limits (T6 and W6 must stay set) */
#define WWDG_COUNTER_MIN            0x40U
#define WWDG_COUNTER_MAX            0x7FU
#define WWDG_WINDOW_MIN             0x40U
#define WWDG_WINDOW_MAX             0x7FU

/** @brief Fixed WWDG clock divider ahead of the prescaler */
#define WWDG_TICKS_PER_COUNT        4096U

/** @brief Default configuration used by WWDG_Init() */
#define WWDG_DEFAULT_PRESCALER      WWDG_PRESCALER_8
#define WWDG_DEFAULT_WINDOW         0x50U
#define WWDG_DEFAULT_COUNTER        0x7FU

/** @brief NVIC priority applied to the early wakeup interrupt */
#define WWDG_EWI_IRQ_PRIORITY       5U
#define WWDG_EWI_IRQ_SUBPRIORITY    0U

/**
 * @brief Reachable timeout range on this board (PCLK1 = 42 MHz)
 * @note  Informational only; WWDG_CalculateTimeout() derives the real values
 *        from the live PCLK1 frequency.
 */
#define WWDG_TIMEOUT_MIN_US         98U     /**< One tick at prescaler 1 */
#define WWDG_TIMEOUT_MAX_MS         49U     /**< 64 ticks at prescaler 8 */

#ifdef __cplusplus
}
#endif

#endif /* WWDG_TYPES_H */

/**
 * @file led_blink.h
 * @brief Fixed-rate blink engine built on top of the LED state driver.
 * @details The caller supplies the current time, so this module owns blink
 *          scheduling only - it has no opinion about the time source.
 */

#ifndef LED_BLINK_H
#define LED_BLINK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Blink engine state
 */
typedef struct {
    LedHandle_t *led;      /**< LED being driven */
    uint32_t periodMs;     /**< Full on+off period */
    uint32_t lastToggleMs; /**< Time of the last toggle */
    bool running;          /**< Blinking active flag */
} LedBlink_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Bind a blink engine to an initialized LED
 * @param   blink Pointer to blink engine
 * @param   led Pointer to an initialized LED handle
 * @retval  true if successful, false otherwise
 */
bool LedBlink_Init(LedBlink_t *blink, LedHandle_t *led);

/**
 * @brief   Start blinking
 * @param   blink Pointer to blink engine
 * @param   periodMs Full on+off period in milliseconds (must be non-zero)
 * @param   nowMs Current time in milliseconds
 * @retval  true if successful, false otherwise
 */
bool LedBlink_Start(LedBlink_t *blink, uint32_t periodMs, uint32_t nowMs);

/**
 * @brief   Stop blinking and switch the LED off
 * @param   blink Pointer to blink engine
 * @retval  true if successful, false otherwise
 */
bool LedBlink_Stop(LedBlink_t *blink);

/**
 * @brief   Toggle the LED when the current half-period has elapsed
 * @param   blink Pointer to blink engine
 * @param   nowMs Current time in milliseconds
 * @retval  true if successful, false otherwise
 */
bool LedBlink_Update(LedBlink_t *blink, uint32_t nowMs);

/**
 * @brief   Check whether the engine is blinking
 * @param   blink Pointer to blink engine
 * @retval  true if blinking, false otherwise
 */
bool LedBlink_IsRunning(const LedBlink_t *blink);

#ifdef __cplusplus
}
#endif

#endif /* LED_BLINK_H */

/**
 * @file blink_steady.c
 * @brief Behaviour: the auxiliary external LED blinks on and off at a fixed rate.
 */

/* Includes ------------------------------------------------------------------*/
#include "blink_steady.h"

#include "board.h"
#include "led_blink.h"

/* Private constants ---------------------------------------------------------*/
#define BLINK_PERIOD_MS   500u

/* Private variables ---------------------------------------------------------*/
static LedHandle_t s_led;
static LedBlink_t  s_blink;

/* Exported functions --------------------------------------------------------*/

bool BlinkSteady_Init(void)
{
    const LedConfig_t config = {
        .port = BOARD_LED_AUX_PORT,
        .pin = BOARD_LED_AUX_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    if (!Led_InitCustom(&s_led, &config)) {
        return false;
    }

    if (!LedBlink_Init(&s_blink, &s_led)) {
        return false;
    }

    return LedBlink_Start(&s_blink, BLINK_PERIOD_MS, 0u);
}

void BlinkSteady_Task(uint32_t nowMs)
{
    LedBlink_Update(&s_blink, nowMs);
}

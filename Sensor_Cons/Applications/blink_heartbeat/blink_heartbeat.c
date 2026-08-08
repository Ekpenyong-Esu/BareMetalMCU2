/**
 * @file    blink_heartbeat.c
 * @brief   Application 1 - Blink & Heartbeat composition.
 *
 * Starts the four LED behaviours and ticks them. Every decision about pins,
 * rates and patterns belongs to the behaviour modules, not to this file.
 */

#include "blink_heartbeat.h"

#include "blink_steady.h"
#include "led_heartbeat.h"
#include "pwm_led_hardware.h"
#include "pwm_led_software.h"
#include "sys.h"

static bool BlinkHeartbeat_Init(void)
{
    return (BlinkSteady_Init() &&
           LedHeartbeat_Init() &&
           PwmLedSoftware_Init() &&
           PwmLedHardware_Init()) != 0;
}

static void BlinkHeartbeat_Task(void)
{
    uint32_t nowMs = HAL_GetTick();

    BlinkSteady_Task(nowMs);
    LedHeartbeat_Task(nowMs);
    PwmLedSoftware_Task(nowMs);
    PwmLedHardware_Task(nowMs);
}

void BlinkHeartbeat_Run(void)
{
    if (!BlinkHeartbeat_Init()) {
        Error_Handler();
    }

    for (;;) {
        BlinkHeartbeat_Task();
    }
}

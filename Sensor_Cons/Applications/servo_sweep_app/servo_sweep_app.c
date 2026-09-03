/**
 * @file servo_sweep_app.c
 * @brief Application 7, "Servo Sweep": drive a hobby servo through its range
 *
 * This is the only file that knows anything about the board. Each layer below
 * it depends only on the next one down, and none of them depend back up:
 *
 *   servo_sweep_app  which pin, which timer, which sweep to run
 *   servo_console    saying over the serial port what is sweeping
 *   servo_sweep      angles and timing turned into servo motion
 *   SERVO / TIM      the drivers
 *
 * Progress is reported on USART1 at 115200 8N1 (PA9 TX, PA10 RX).
 *
 * Wiring:
 *
 *   servo signal (orange/yellow) to PB4, driven by TIM3_CH1
 *   servo VCC (red) to a separate 5-6 V supply, never to the MCU pin
 *   servo GND (brown/black) to supply GND and board GND together
 */

#include "servo_sweep_app.h"

#include "servo_console.h"
#include "servo_sweep.h"

/* PB4 is free on this board (no FMC/LTDC/I2C claim in project.ioc) and
   carries TIM3_CH1, which the SERVO driver maps via GPIO_AF2_TIM3. */
#define SERVO_SWEEP_APP_PORT    GPIOB
#define SERVO_SWEEP_APP_PIN     GPIO_PIN_4
#define SERVO_SWEEP_APP_TIMER   TIM3
#define SERVO_SWEEP_APP_CHANNEL TIM_CHANNEL_1

/* The console prints this verbatim: which pin is wired up is board knowledge,
   and board knowledge lives in this file. */
#define SERVO_SWEEP_APP_DESCRIPTION "servo signal on PB4, driven by TIM3_CH1"

/** Pause between full sweeps, so the turnaround point is visible on UART. */
#define SERVO_SWEEP_APP_PAUSE_MS 1000U

/* Static so they outlive ServoSweepApp_Run's stack frame: the timer keeps
   running off this handle for as long as the servo moves. */
static TIM_HandleTypeDef s_servoTimer;
static SERVO_Handle_t s_servo;

void ServoSweepApp_Run(void)
{
    /* Opened first so that a servo that will not start can say so. */
    ServoConsole_Init();

    if (SERVO_Init(&s_servo, &s_servoTimer, SERVO_SWEEP_APP_CHANNEL,
                   SERVO_SWEEP_APP_PORT, SERVO_SWEEP_APP_PIN) != SERVO_OK) {
        ServoConsole_ReportError("the servo output could not be started");
        return; /* the caller decides what to do */
    }

    ServoConsole_ReportReady(SERVO_SWEEP_APP_DESCRIPTION);

    ServoSweep_Config_t sweep = ServoSweep_GetDefaultConfig();

    for (;;) {
        SERVO_StatusTypeDef status =
            ServoSweep_RunOnce(&s_servo, &sweep, ServoConsole_ReportAngle);
        if (status != SERVO_OK) {
            ServoConsole_ReportError("a step was rejected by the servo driver");
            return;
        }
        HAL_Delay(SERVO_SWEEP_APP_PAUSE_MS);
    }
}

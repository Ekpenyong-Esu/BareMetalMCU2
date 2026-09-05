/**
 * @file servo_sweep_app.c
 * @brief Application 7, "Servo Sweep": drive a hobby servo through its range
 *
 * This is the only file that knows anything about the board. It sits on top
 * of two modules that do not know about each other:
 *
 *   servo_sweep_app   which pin, which timer, which UART, which sweep to run
 *     |            \
 *   servo_sweep     servo_console
 *   (angles and     (saying over the serial
 *    timing turned   port what is sweeping)
 *    into motion)         |
 *     |                 UART driver
 *   SERVO / TIM drivers
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
#define SERVO_SWEEP_APP_PORT GPIOB
#define SERVO_SWEEP_APP_PIN GPIO_PIN_4
#define SERVO_SWEEP_APP_TIMER TIM3
#define SERVO_SWEEP_APP_CHANNEL TIM_CHANNEL_1

/* The ST-LINK virtual COM port on this board. */
#define SERVO_SWEEP_APP_UART USART1

/* The console prints this verbatim: which pin is wired up is board knowledge,
   and board knowledge lives in this file. */
#define SERVO_SWEEP_APP_DESCRIPTION "servo signal on PB4, driven by TIM3_CH1"

/** Pause between full sweeps, so the turnaround point is visible on UART. */
#define SERVO_SWEEP_APP_PAUSE_MS 1000U

/* Static so they outlive ServoSweepApp_Run's stack frame: the timer keeps
   running off this handle for as long as the servo moves. */
static TIM_HandleTypeDef s_servoTimer;
static SERVO_Handle_t s_servo;

/** @brief Turn a driver status into words the console can print */
static const char *ServoSweepApp_StatusText(SERVO_StatusTypeDef status) {
    switch (status) {
        case SERVO_OK:
            return "ok";
        case SERVO_ERROR:
            return "driver error";
        case SERVO_BUSY:
            return "servo busy";
        case SERVO_TIMEOUT:
            return "servo timed out";
        case SERVO_INVALID_PARAM:
            return "invalid parameter";
        case SERVO_NOT_INITIALIZED:
            return "servo not initialised";
        case SERVO_OUT_OF_RANGE:
            return "angle out of range";
        default:
            return "unknown status";
    }
}

void ServoSweepApp_Run(void) {
    /* Opened first so that a servo that will not start can say so. */
    ServoConsole_Init(SERVO_SWEEP_APP_UART);

    /* SERVO_Init reads the peripheral off the handle; it never picks one itself. */
    s_servoTimer.Instance = SERVO_SWEEP_APP_TIMER;

    SERVO_StatusTypeDef status = SERVO_Init(&s_servo, &s_servoTimer, SERVO_SWEEP_APP_CHANNEL,
                                            SERVO_SWEEP_APP_PORT, SERVO_SWEEP_APP_PIN);
    if (status != SERVO_OK) {
        ServoConsole_ReportError(ServoSweepApp_StatusText(status));
        return; /* the caller decides what to do */
    }

    ServoConsole_ReportReady(SERVO_SWEEP_APP_DESCRIPTION);

    const ServoSweep_Config_t sweep = ServoSweep_GetDefaultConfig(&s_servo);

    for (;;) {
        status = ServoSweep_RunOnce(&s_servo, &sweep, ServoConsole_ReportAngle);
        if (status != SERVO_OK) {
            ServoConsole_ReportError(ServoSweepApp_StatusText(status));
            return;
        }
        HAL_Delay(SERVO_SWEEP_APP_PAUSE_MS);
    }
}
